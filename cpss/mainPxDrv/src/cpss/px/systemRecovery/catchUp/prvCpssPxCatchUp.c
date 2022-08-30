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
* @file prvCpssPxCatchUp.c
*
* @brief CPSS Px CatchUp functions.
*
* @version   1
********************************************************************************
*/

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/systemRecovery/catchUp/private/prvCpssPxCatchUp.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/px/port/cpssPxPortManager.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


static prvCpssCatchUpFuncPtr  catchUpFuncPtrArray[] = {
                                                prvCpssPxHwDevNumCatchUp,prvCpssPxCpuPortModeCatchUp,
                                                prvCpssPxPortModeParamsCatchUp,prvCpssPxPortResourcesCatchUp,
                                                NULL
                                                };

/**
* @internal prvCpssPxCatchUpEnableInterrupts function
* @endinternal
*
* @brief   enable interrupts for all devices
*
*
* @param[in] devNum                - The device number.
* @param[in] enable                - the enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvCpssPxCatchUpEnableInterrupts
(
    GT_VOID
)
{
    GT_STATUS           rc = GT_OK;
    CPSS_HW_INFO_STC    *hwInfo;
    GT_U32              ii;

    for (ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        hwInfo = cpssDrvHwPpHwInfoStcPtrGet(ii,CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        if (hwInfo != NULL)
        {
            if ((hwInfo->irq.switching) == CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS)
            {
                rc = prvCpssDrvPPinitPollingEnableSet(ii,GT_TRUE);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
            else
            {
                rc = prvCpssDrvInterruptEnable(hwInfo->intMask.switching);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssPxHwDevNumCatchUp function
* @endinternal
*
* @brief   Synchronize hw device number in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwDevNumCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_HW_DEV_NUM hwDevNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc =  cpssPxCfgHwDevNumGet(devNum,&hwDevNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* save actual HW devNum to the DB */
    PRV_CPSS_HW_DEV_NUM_MAC(devNum) = hwDevNum;

    return rc;
}

/**
* @internal prvCpssPxCpuPortModeCatchUp function
* @endinternal
*
* @brief   Synchronize cpu port mode in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxCpuPortModeCatchUp
(
    IN GT_U8    devNum
)
{
    CPSS_HW_INFO_STC    *hwInfo;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    hwInfo = cpssDrvHwPpHwInfoStcPtrGet(devNum,0);
    if (hwInfo != NULL)
    {
        if (hwInfo->busType  == CPSS_HW_INFO_BUS_TYPE_SMI_E)
        {
             PRV_CPSS_PP_MAC(devNum)->cpuPortMode = CPSS_NET_CPU_PORT_MODE_NONE_E;
        }
        else
        {
            PRV_CPSS_PP_MAC(devNum)->cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortModeParamsCatchUp function
* @endinternal
*
* @brief   Synchronize Port Mode parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortModeParamsCatchUp
(
    IN GT_U8    devNum
)
{
    GT_PHYSICAL_PORT_NUM port;
    GT_STATUS rc = GT_OK;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT            speed;
    GT_BOOL isValid;
    GT_U32 portMac;
    GT_U32 portSpeedInMBit;
    GT_BOOL linkDownState = GT_FALSE;
    GT_BOOL cpuPortIsMapped = GT_FALSE;
    GT_BOOL apEnable = GT_FALSE;
    CPSS_PX_PORT_AP_PARAMS_STC apParams;
    GT_U32 maxNumberOfPorts = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_HA_E == tempSystemRecovery_Info.systemRecoveryProcess)
    {
        /*port mode params reconstructs by replay */
        return GT_OK;
    }

    maxNumberOfPorts = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

    /* loop over all GE and FE ports */
    for (port = 0; port <= maxNumberOfPorts; port++)
    {
        if ( port == PRV_CPSS_PX_CPU_DMA_NUM_CNS )
        {
            /* cpu port */
            cpuPortIsMapped = GT_TRUE;
            continue;
        }

        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, port, &isValid);
        if((rc != GT_OK) || (isValid != GT_TRUE))
        {
            continue;
        }
        else
        {
            PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum, port);
        }

        PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMac);

        rc =  prvCpssPxPortInterfaceModeHwGet(devNum, port, &ifMode);
        if (rc == GT_NOT_INITIALIZED)
        {
            continue;
        }
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssPxPortSpeedHwGet(devNum, port, &speed);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;

        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = speed;
        if((CPSS_PORT_SPEED_2500_E == speed) &&
            (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode))
        {
            /* SGMII 2.5G in HW implemented as 1000BaseX */
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode =
                                            CPSS_PORT_INTERFACE_MODE_SGMII_E;
        }

        /* recover speed for pizza*/
        if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
        {
            /* check if port is link down. Only one port of 4 that is not force link down  */
            /* get QSGMII configuration others - NA                                        */
            rc =  cpssPxPortForceLinkDownEnableGet(devNum,port,&linkDownState);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (linkDownState == GT_TRUE)
            {
                ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = ifMode;
                speed = CPSS_PORT_SPEED_NA_E;
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = speed;
            }
        }

        if ( (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) && (speed != CPSS_PORT_SPEED_NA_E) )
        {
            rc = prvCpssPxPortDynamicPizzaArbiterSpeedConv(devNum,port,speed,/*OUT*/&portSpeedInMBit);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* update pizza DB */
            PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB.prv_portDB[port].isInitilized = GT_TRUE;
            PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB.prv_portDB[port].portSpeedInMBit = portSpeedInMBit;
            PRV_CPSS_PX_PP_MAC(devNum)->paData.paPortDB.numInitPorts++;
        }

        if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            prvCpssPxPortTypeSet(devNum, port, ifMode, speed);
        }

        /* check if port is AP */
        if ( (speed == CPSS_PORT_SPEED_NA_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) )
        {
            cpssOsMemSet(&apParams, 0, sizeof(CPSS_PX_PORT_AP_PARAMS_STC));
            rc = cpssPxPortApPortConfigGet(devNum,port,&apEnable,&apParams);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (apEnable == GT_TRUE)
            {
                 PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portIfMode = apParams.modesAdvertiseArr[0].ifMode;
                 PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed = apParams.modesAdvertiseArr[0].speed;
            }
        }
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E == tempSystemRecovery_Info.systemRecoveryProcess)
    {
        if ( cpuPortIsMapped == GT_TRUE )
        {
            rc = prvCpssPxPortPizzaArbiterIfConfigure(devNum, PRV_CPSS_PX_CPU_DMA_NUM_CNS, CPSS_PORT_SPEED_1000_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortResourcesCatchUp function
* @endinternal
*
* @brief   Synchronize Port resources parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesCatchUp
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    #ifdef GM_USED
      devNum = devNum;
    #else
        GT_PHYSICAL_PORT_NUM port;
        CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC resource;
        GT_U32  *txQDescrCreditsPtr = NULL;
        GT_U32  *txFifoHeaderCreditsPtr = NULL;
        GT_U32  *txFifoPayloadCreditsPtr = NULL;
        GT_U32  dpIndex;
        GT_U32  localTxqNum;
        GT_U32  maxDp = PRV_CPSS_PX_MAX_DP_CNS;
        CPSS_PORT_SPEED_ENT speed;
        GT_U32 portMac;
        GT_U32 speedValueMbps;
        GT_U32 speedIndex;
        CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
        PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_TRUE)
        {
            /* tm is not supported */
            return GT_OK;
        }

        for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
        {
            /* zero overall summary speed */
            PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus.coreOverallSpeedSummary[dpIndex] = 0;
            /* zero the credits in shadow */
            rc = prvCpssPxPortResourcesConfigDbDelete(devNum, dpIndex,
                                                      PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus.usedDescCredits[dpIndex],
                                                      PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus.usedHeaderCredits[dpIndex],
                                                      PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus.usedPayloadCredits[dpIndex]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        txQDescrCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txQDescrCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        txFifoHeaderCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txFifoHeaderCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        txFifoPayloadCreditsPtr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxDp);
        if(txFifoPayloadCreditsPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(txQDescrCreditsPtr, 0, sizeof(GT_U32)*maxDp);
        cpssOsMemSet(txFifoHeaderCreditsPtr, 0, sizeof(GT_U32)*maxDp);
        cpssOsMemSet(txFifoPayloadCreditsPtr, 0, sizeof(GT_U32)*maxDp);

        /* loop over all GE and FE ports */
        for (port = 0; port < PRV_CPSS_PX_PORTS_NUM_CNS; port++)
        {
            /* Get Port Mapping DataBase */
            rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, port, &portMapShadowPtr);
            if(rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
            if ((portMapShadowPtr->valid == GT_FALSE) || (portMapShadowPtr->portMap.mappingType >= CPSS_PX_PORT_MAPPING_TYPE_MAX_E) )
            {
                continue;
            }

            rc = prvCpssPxPpResourcesTxqGlobal2LocalConvert(devNum, portMapShadowPtr->portMap.txqNum, &dpIndex, &localTxqNum);
            if(GT_OK != rc)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }

            if (portMapShadowPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if (CPSS_SYSTEM_RECOVERY_PROCESS_HA_E == tempSystemRecovery_Info.systemRecoveryProcess)
                {
                    /* In this case reconstruction is done by replay */
                    continue;
                }

                portMac = portMapShadowPtr->portMap.macNum;
                    /* get the port speed */
                speed = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMac].portSpeed;
            }
            else
            {
                speed = CPSS_PORT_SPEED_1000_E;
            }
            if (speed == CPSS_PORT_SPEED_NA_E)
            {
                /* port is not used */
                continue;
            }
            /* convert speed to Mbps value */
            rc = prvCpssPxPortResourcesSpeedIndexGet(speed,&speedIndex,&speedValueMbps);
            if (rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
            PRV_CPSS_PX_PP_MAC(devNum)->port.resourcesStatus.coreOverallSpeedSummary[dpIndex] += speedValueMbps;

            rc = prvCpssPxPipePortResourceConfigGet(devNum,port,&resource);
            if (rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
            txQDescrCreditsPtr[dpIndex]      += resource.txdmaDescCreditScdma;
            txFifoHeaderCreditsPtr[dpIndex]  += resource.txdmaScdmaHeaderTxfifoThrshold;
            txFifoPayloadCreditsPtr[dpIndex] += resource.txdmaScdmaPayloadTxfifoThrshold;

        }

        for (dpIndex=0; dpIndex < maxDp; dpIndex++)
        {
            /* update credits in shadow */
            rc = prvCpssPxPortResourcesConfigDbAdd(devNum, dpIndex, txQDescrCreditsPtr[dpIndex],
                                                     txFifoHeaderCreditsPtr[dpIndex], txFifoPayloadCreditsPtr[dpIndex]);
            if (rc != GT_OK)
            {
                cpssOsFree(txQDescrCreditsPtr);
                cpssOsFree(txFifoHeaderCreditsPtr);
                cpssOsFree(txFifoPayloadCreditsPtr);
                return rc;
            }
        }
        cpssOsFree(txQDescrCreditsPtr);
        cpssOsFree(txFifoHeaderCreditsPtr);
        cpssOsFree(txFifoPayloadCreditsPtr);

    #endif

    return rc;
}

/**
* @internal prvCpssPxSystemRecoveryCatchUpHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxSystemRecoveryCatchUpHandle
(
   GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8 devNum;
    GT_U32 i = 0;

    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
#ifdef    CPSS_LOG_ENABLE
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
#endif

    /* take time from the CatchUp start */
    cpssOsTimeRT(&start_sec, &start_nsec);

    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if ( PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] == NULL )
        {
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            continue;
        }
        while (catchUpFuncPtrArray[i] != NULL)
        {
            /* perform catch up*/
            rc = (*catchUpFuncPtrArray[i])(devNum);
            if (rc != GT_OK)
            {
                CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                return rc;
            }
            i++;
        }

        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* prepare iterator for next device*/
        i = 0;

    }

    /* Calculate time from CatchUp start */
    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
#ifdef    CPSS_LOG_ENABLE
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;
#endif

    CPSS_LOG_INFORMATION_MAC("Catchup time processing is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}

/**
* @internal prvCpssPxSystemRecoveryCompletionHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  none.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxSystemRecoveryCompletionHandle
(
   GT_VOID
)
{
    GT_STATUS rc =GT_OK;
    /*enable Interrupts for all devices */
    rc = prvCpssPxCatchUpEnableInterrupts();

    return rc;
}