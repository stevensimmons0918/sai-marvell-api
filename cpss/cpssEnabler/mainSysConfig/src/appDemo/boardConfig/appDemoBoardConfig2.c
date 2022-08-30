/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoBoardConfig2.c
*
* @brief file #2 : Includes board specific initialization definitions and data-structures.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#endif /*CHX_FAMILY*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
/* implement STUB */
GT_STATUS gtAppDemoXPhyFwDownload
(
    IN  GT_U8   devNum
)
{
    GT_UNUSED_PARAM(devNum);
    return GT_NOT_IMPLEMENTED;
}
GT_STATUS mtdParallelEraseFlashImage
(
    IN void* contextPtr,
    IN GT_U16 ports[],
    IN GT_U32 appSize,
    IN GT_U8 slaveData[],
    IN GT_U32 slaveSize,
    IN GT_U16 numPorts,
    OUT GT_U16 erroredPorts[],
    OUT GT_U16 *errCode
)
{
    GT_UNUSED_PARAM(contextPtr);
    GT_UNUSED_PARAM(ports);
    GT_UNUSED_PARAM(appSize);
    GT_UNUSED_PARAM(slaveData);
    GT_UNUSED_PARAM(slaveSize);
    GT_UNUSED_PARAM(numPorts);
    GT_UNUSED_PARAM(erroredPorts);
    GT_UNUSED_PARAM(errCode);
    return GT_NOT_IMPLEMENTED;
}
#endif

#if (!(defined ASIC_SIMULATION_ENV_FORBIDDEN) && defined ASIC_SIMULATION)
    /* need to run with simulation lib exists */
    #define WM_IMPLEMENTED
#endif


appDemo_cpssDxChTrunkMemberRemove_CB_FUNC appDemo_cpssDxChTrunkMemberRemove = NULL;
appDemo_cpssDxChTrunkMemberAdd_CB_FUNC    appDemo_cpssDxChTrunkMemberAdd    = NULL;
appDemo_cpssDxChTrunkMembersSet_CB_FUNC   appDemo_cpssDxChTrunkMembersSet   = NULL;

#ifdef CHX_FAMILY
/*number of DQ ports in each of the 6 DQ units in TXQ */
#define BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS     SIP_5_20_DQ_NUM_PORTS_CNS
/*macro to convert local port and data path index to TXQ port */
#define BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS)
/* build TXQ_port from global mac port */
#define BC3_TXQ_PORT(globalMacPort)    BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC((globalMacPort)%12,(globalMacPort)/12)

static GT_U32   bc3ConvertDmaNumToTxqNum(IN GT_U32 dmaNum)
{
    GT_U32  txqNum;
    if(dmaNum < 72)
    {
        txqNum = BC3_TXQ_PORT(dmaNum);
    }
    else
    if(dmaNum < 78)
    {
        GT_U32  dpCpuPorts[6] = {2,5,0,1,3,4};

        txqNum = BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(12 , dpCpuPorts[dmaNum-72]);
    }
    else
    {
        txqNum = GT_NA;
    }

    return txqNum;
}

/**
* @internal appDemoDxChFillDbForCpssPortMappingInfo function
* @endinternal
*
* @brief   This function converts 'appDemo port mapping' style to
*           'cpss DXCH port mapping' style .
*
* @param[in] devNum                   - The device number.
* @param[in] appDemoInfoPtr           - array of port mapping in AppDemo style
* @param[in]  appDemoInfoPtr          - array of port mapping in CPSS style
*                                       array allocated array by the caller !!!
* @param[out] appDemoInfoPtr          - (filled) array of port mapping in CPSS style
* @param[out] numOfMappedPortsPtr     - (pointer to) the number of entries filled in appDemoInfoPtr[]
* @param[out] numCpuSdmasPtr          - (pointer to) the number of CPU SDMAs
*                                       can be NULL
*
* @retval GT_OK      - on success
* @retval GT_FULL    - asking for too many physical ports.
* @retval GT_NOT_IMPLEMENTED - for device that code not implemented
*
*/
GT_STATUS appDemoDxChFillDbForCpssPortMappingInfo(
    IN GT_SW_DEV_NUM                devNum,
    IN APP_DEMO_PORT_MAP_STC        *appDemoInfoPtr,
    INOUT CPSS_DXCH_PORT_MAP_STC    *cpssInfoPtr,
    OUT   GT_U32                    *numOfMappedPortsPtr,
    OUT   GT_U32                    *numCpuSdmasPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    GT_U32  dmaPortOffset;
    GT_U32 maxPhyPorts;
    GT_U32 numOfMappedPorts = 0;
    GT_U32 numCpuSdmas = 0;
    GT_U32 bc3used = 0;

    /*************************************************************/
    /* implementation based on fillDbForCpssPortMappingInfo(...) */
    /*************************************************************/

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    numOfMappedPorts = 0;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        bc3used = 1;
    }
    else
    {
        return GT_NOT_IMPLEMENTED;
    }

    for (ii = 0 ; appDemoInfoPtr->startPhysicalPortNumber != GT_NA ; ii++ , appDemoInfoPtr++)
    {
        if ((appDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)&&
            (numOfMappedPorts < maxPhyPorts))
        {
            /* first generate line that map between the mac number to physical port in the device.
            for remote ports the physical port number ( cascade number) is in field of "dma step"*/
            cpssInfoPtr->physicalPortNumber = appDemoInfoPtr->jumpDmaPorts;
            cpssInfoPtr->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            cpssInfoPtr->interfaceNum       = appDemoInfoPtr->startGlobalDmaNumber;

            /* the cascade port not need TXQ queue */
            cpssInfoPtr->txqPortNumber  = bc3used ? GT_NA : 0;

            numOfMappedPorts++;
            cpssInfoPtr++;
            /* second, generate lines that map between remote physical port to the mac number*/
        }

        dmaPortOffset = 0;

        for(jj = 0 ; jj < appDemoInfoPtr->numOfPorts; jj++ , cpssInfoPtr++)
        {
            if(numOfMappedPorts >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_falcon_defaultMap is FULL, maxPhy port is %d ", maxPhyPorts);
                return rc;
            }

            cpssInfoPtr->physicalPortNumber = appDemoInfoPtr->startPhysicalPortNumber + jj;
            cpssInfoPtr->mappingType        = appDemoInfoPtr->mappingType;
            cpssInfoPtr->interfaceNum       = appDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;

            if(appDemoInfoPtr->startTxqNumber == GT_NA)
            {
                cpssInfoPtr->txqPortNumber  = GT_NA;
            }
            else
            if(appDemoInfoPtr->startTxqNumber == TXQ_BY_DMA_CNS)
            {
                /* 'use startGlobalDmaNumber' for the definition */
                cpssInfoPtr->txqPortNumber  = bc3used ? bc3ConvertDmaNumToTxqNum(cpssInfoPtr->interfaceNum) : 0;
            }
            else
            {
                /* explicit value */
                cpssInfoPtr->txqPortNumber  = appDemoInfoPtr->startTxqNumber + jj;
            }

            if(appDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                numCpuSdmas++;
            }

            numOfMappedPorts++;

            if((appDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(appDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
            {
                if(appDemoInfoPtr->jumpDmaPorts >= 2)
                {
                    dmaPortOffset += appDemoInfoPtr->jumpDmaPorts;
                }
                else
                {
                    dmaPortOffset ++;
                }
            }
        }
    }

    *numOfMappedPortsPtr = numOfMappedPorts;
    if(numCpuSdmasPtr)
    {
        *numCpuSdmasPtr       = numCpuSdmas;
    }

    return GT_OK;
}

#endif /*CHX_FAMILY*/

extern GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(
    IN GT_U8    devNum
);
extern GT_STATUS prvNoKmDrv_configure_dma_over_the_pci(
    IN GT_U8    devNum
);
extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);


/*******************************************************************************
* appDemo_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*  NOTE: implemented for SIP6 (and AC5)
*******************************************************************************/
GT_STATUS   appDemo_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_BOOL  dmaInsidePp /* is DMA over the pci (GT_FALSE) or inside the PP (GT_TRUE) */
)
{
    GT_STATUS rc;
#ifdef WM_IMPLEMENTED
    if(!cpssHwDriverGenWmInPexModeGet())
    {
        return GT_OK;
    }
#endif
    /* also for WM for 'pci mode' */
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            if(dmaInsidePp == GT_TRUE)
            {
                rc = prvNoKmDrv_configure_dma_internal_cpu(devNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_dma_internal_cpu", rc);
            }
            else
            {
                rc = prvNoKmDrv_configure_dma_over_the_pci(devNum);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_dma_over_the_pci", rc);
            }
            break;
        default:/* Falcon , Hawk(AC5P) ,Harrier */
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                rc = GT_NOT_SUPPORTED;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemo_configure_dma_per_devNum unknown SIP6 device", rc);
            }
            else
            {
                rc = prvNoKmDrv_configure_dma_per_devNum(devNum,0/*portGroup*/);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_dma_per_devNum", rc);
            }
            break;
    }

    return rc;
}



