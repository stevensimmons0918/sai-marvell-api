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
* @file prvCpssDxChPortMappingShadowDB.c
*
* @brief interface to port mapping shadow DB
*
* @version   9
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_STATUS prvCpssDxChPortPhysicalPortMapShadowDBClear
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physPortNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    if (physPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].valid = GT_FALSE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.trafficManagerEn       = GT_FALSE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType            = CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.portGroup              = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.macNum                 = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.rxDmaNum               = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txDmaNum               = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txqNum                 = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.ilknChannel            = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.tmPortIdx              = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txFifoPortNum          = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.isExtendedCascadePort  = GT_FALSE;
    return GT_OK;
}


GT_STATUS prvCpssDxChPortPhysicalPortMapShadowDBSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    physPortNum,
    IN CPSS_DXCH_PORT_MAP_STC *portMapPtr,
    IN GT_U32                  portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAX_E],
    IN GT_BOOL                 isExtendedCascade

)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    if (physPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDxChPortPhysicalPortMapShadowDBClear(devNum,physPortNum);
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].valid = GT_TRUE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.trafficManagerEn = portMapPtr->tmEnable;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType      = portMapPtr->mappingType;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.portGroup        = portMapPtr->portGroup;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.macNum           = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.rxDmaNum         = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txDmaNum         = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txFifoPortNum    = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.txqNum           = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.ilknChannel      = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E];
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.isExtendedCascadePort = isExtendedCascade;
    if (GT_TRUE == portMapPtr->tmEnable)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.tmPortIdx        = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E];
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].extPortMap.preemptionChannelAllocated        = portMapPtr->reservePreemptiveChannel;

    if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E]] = physPortNum;
    }

    if(PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum) &&
       (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ||
        portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        /* CPSS-13730 : portGroupId info should be added in cpssDxChPortPhysicalPortDetailedMapGet*/
        rc = prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(devNum,physPortNum,
            &PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.portGroup);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortPhysicalPortMapShadowDBGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum,
    OUT  CPSS_DXCH_DETAILED_PORT_MAP_STC **portMapShadowPtrPtr
)
{
    GT_U32                                  i;
    GT_STATUS       rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE    *aNodePtr = NULL;
    GT_U32                            pNodeIndex;
    GT_U8                             firstQueue;
    GT_U32                            globalDma;
    GT_BOOL                           treeReady;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    if (physPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(portMapShadowPtrPtr);
    *portMapShadowPtrPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum];
    (*portMapShadowPtrPtr)->extPortMap.chipletIndex = 0xFFFFFFFF;
    (*portMapShadowPtrPtr)->extPortMap.mgUnitIndex=0xFFFFFFFF;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)&&(*portMapShadowPtrPtr)->valid)
    {
        rc = prvCpssTxqUtilsTreeIsReadyGet(devNum,&treeReady);
        if(rc!=GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqUtilsTreeIsReadyGet  failed ");
        }
        /*check that mapping is already done*/
        if(treeReady == GT_TRUE)
        {
            (*portMapShadowPtrPtr)->extPortMap.tileId =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[(*portMapShadowPtrPtr)->portMap.txDmaNum].tileInd;
            (*portMapShadowPtrPtr)->extPortMap.txqInfo.pNodeIndex =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.dmaToPnode[(*portMapShadowPtrPtr)->portMap.txDmaNum].pNodeInd;

            pNodeIndex = (*portMapShadowPtrPtr)->extPortMap.txqInfo.pNodeIndex;
            PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,(*portMapShadowPtrPtr)->extPortMap.tileId,pNodeIndex)

            if(GT_FALSE == (*portMapShadowPtrPtr)->portMap.isExtendedCascadePort)
            {
               (*portMapShadowPtrPtr)->extPortMap.txqInfo.aNodeIndex =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physPortNum].aNodeInd;

                for(i=0;i<pNodePtr->aNodeListSize;i++)
                {
                    if(pNodePtr->aNodelist[i].aNodeIndex
                        == (*portMapShadowPtrPtr)->extPortMap.txqInfo.aNodeIndex )
                    {
                        aNodePtr = &(pNodePtr->aNodelist[i]);
                        break;
                    }
                }

                if(aNodePtr!=NULL)
                {
                    (*portMapShadowPtrPtr)->extPortMap.txqInfo.numberOfQueues = aNodePtr->queuesData.pdqQueueLast -
                        aNodePtr->queuesData.pdqQueueFirst+1;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Failed to find a node indexed   %d  ",
                    (*portMapShadowPtrPtr)->extPortMap.txqInfo.aNodeIndex);
                }
             }
             else
             {
                (*portMapShadowPtrPtr)->extPortMap.txqInfo.numberOfQueues = pNodePtr->sdqQueueLast -
                    pNodePtr->sdqQueueFirst+1;
                aNodePtr = &(pNodePtr->aNodelist[0]);

             }

             (*portMapShadowPtrPtr)->extPortMap.localDpInTile = aNodePtr->queuesData.dp;
             (*portMapShadowPtrPtr)->extPortMap.txqInfo.primarySchFirstQueueIndex = aNodePtr->queuesData.pdqQueueFirst;
             (*portMapShadowPtrPtr)->extPortMap.txqInfo.secondarySchFirstQueueIndex = aNodePtr->queuesData.queueBase;

             rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,(*portMapShadowPtrPtr)->portMap.txDmaNum,
                &((*portMapShadowPtrPtr)->extPortMap.globalDp),&((*portMapShadowPtrPtr)->extPortMap.localPortInDp));

             if(rc!=GT_OK)
             {
                  CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconDmaGlobalNumToLocalNumInDpConvert  failed for dma  %d  ",
                    (*portMapShadowPtrPtr)->portMap.txDmaNum);
             }

             if((*portMapShadowPtrPtr)->portMap.mappingType!= CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
             {
                 if(GT_FALSE==PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                 {
                     if((*portMapShadowPtrPtr)->extPortMap.localPortInDp!=
                         PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts-1)
                     {
                         (*portMapShadowPtrPtr)->extPortMap.chipletIndex = (*portMapShadowPtrPtr)->portMap.macNum/16;
                     }
                     else
                     {
                         (*portMapShadowPtrPtr)->extPortMap.chipletIndex = (*portMapShadowPtrPtr)->portMap.macNum - PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
                     }
                 }

               }
              else
              {
                    globalDma = (*portMapShadowPtrPtr)->portMap.txDmaNum;
                    /* look for the DMA of this port in the used CPU ports */
                    for (i = 0; i < CPSS_MAX_SDMA_CPU_PORTS_CNS; i++)
                    {
                        PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum,i);

                        if(globalDma == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[i].dmaNum)
                        {
                            firstQueue= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[i].firstSwQueueIndex;

                            if((firstQueue>>3) < CPSS_MAX_SDMA_CPU_PORTS_CNS &&
                                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.netIfInfo[(firstQueue>>3)].valid)
                            {
                              (*portMapShadowPtrPtr)->extPortMap.mgUnitIndex= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.netIfInfo[(firstQueue>>3)].mgUnitId;
                            }
                            break;
                        }
                    }
              }
        }
    }



    return GT_OK;
}

GT_STATUS prvCpssDxChPortPhysicalPortIsCpu
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    physPortNum,
    OUT  GT_BOOL                 *isCpuPortPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (physPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physical portNum[%d]>=[%d]",
            physPortNum,PRV_CPSS_MAX_PP_PORTS_NUM_CNS);
    }
    if (!PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
    {
        if(physPortNum == CPSS_CPU_PORT_NUM_CNS)
        {
            *isCpuPortPtr = GT_TRUE;
        }
        else
        {
            *isCpuPortPtr = GT_FALSE;
        }
    }
    else
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPortNum].portMap.mappingType ==
            CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            *isCpuPortPtr = GT_TRUE;
        }
        else
        {
            *isCpuPortPtr = GT_FALSE;
        }
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChPortPhysicalPortMapShadowDBInit
(
    IN GT_U8 devNum
)
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               macNum;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);

    for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
    {
        prvCpssDxChPortPhysicalPortMapShadowDBClear(devNum,portNum);
    }

    /*-----------------------------------*
     * inverse MAC--> Physical port init *
     *-----------------------------------*/
    for (macNum = 0; macNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; macNum++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[macNum] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    }
    return GT_OK;
}

/**
* @internal portMapCheckAndConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to convert
* @param[in] convertedPortType        - converted port type
* @param[in] allowRemotePhyPort       - indication to support 'remote physical port'
*
* @param[out] convertedPortNumPtr      - (pointer to) converted port, according to port type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS portMapCheckAndConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    IN  PRV_CPSS_DXCH_PORT_TYPE_ENT     convertedPortType,
    OUT GT_U32                          *convertedPortNumPtr,
    IN  GT_BOOL                         allowRemotePhyPort
)
{
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, physicalPortNum);

    /* check that valid mapping exists (except PHYS to PHYS check and convert) */
    if((convertedPortType != PRV_CPSS_DXCH_PORT_TYPE_PHYS_E) &&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].valid != GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : convertedPortType[%d] not valid in shadow",
            physicalPortNum,convertedPortType);
    }

    if( (convertedPortType != PRV_CPSS_DXCH_PORT_TYPE_PHYS_E) &&
        allowRemotePhyPort == GT_TRUE &&
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType !=
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : MUST be 'REMOTE physical port' ",
            physicalPortNum);
    }


    switch(convertedPortType)
    {
        case PRV_CPSS_DXCH_PORT_TYPE_PHYS_E:
            *convertedPortNumPtr = physicalPortNum;
            break;

        case PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E:
            *convertedPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.rxDmaNum;
            break;

        case PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E:
            *convertedPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.txDmaNum;
            break;

        case PRV_CPSS_DXCH_PORT_TYPE_MAC_E:

            if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                /* CPU port has no MAC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : is 'CPU SDMA' so hold no 'MAC'",
                    physicalPortNum);
            }
            else
            if( allowRemotePhyPort == GT_FALSE &&
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
            {
                /* this is remote physical port and should not be used for setting 'local' configurations */
                /* the caller need to use */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : is 'REMOTE physical port' so hold no LOCAL 'MAC'",
                    physicalPortNum);
            }

            *convertedPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.macNum;
            break;

        case PRV_CPSS_DXCH_PORT_TYPE_TXQ_E:
            *convertedPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.txqNum;
            if(CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS == (*convertedPortNumPtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : is 'cascade' so hold no 'TXQ'",
                    physicalPortNum);
            }
            break;

        case PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E:
            *convertedPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.ilknChannel;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPhysicalPortMapCheckAndConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to convert
* @param[in] convertedPortType        - converted port type
*
* @param[out] convertedPortNumPtr      - (pointer to) converted port, according to port type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapCheckAndConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    IN  PRV_CPSS_DXCH_PORT_TYPE_ENT     convertedPortType,
    OUT GT_U32                          *convertedPortNumPtr
)
{
    return portMapCheckAndConvert(devNum,physicalPortNum,convertedPortType,convertedPortNumPtr,
        GT_FALSE);/*allowRemotePhyPort --> GT_FALSE*/
}

/**
* @internal prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to convert
*
* @param[out] macSecUnitNumPtr       - (pointer to) MACSec unit of a port
* @param[out] macSecChannelNumPtr    - (pointer to) MACSec unit's channel of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    OUT GT_U32                          *macSecUnitNumPtr,
    OUT GT_U32                          *macSecChannelNumPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, physicalPortNum);

    /* check that valid mapping exists */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].valid != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : not valid in shadow",
            physicalPortNum);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType !=
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : MUST be 'ETHERNET_MAC' ",
            physicalPortNum);
    }


    /* Get MAC Sec Unit that is Data path ID and local channel */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.rxDmaNum,
                                                          /*OUT*/macSecUnitNumPtr, /*OUT*/macSecChannelNumPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check DPs connected to MACSec */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp & (1 << (*macSecUnitNumPtr)))
    {
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d]: does not have MACSec unit",
            physicalPortNum);
}


/**
* @internal prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB - ONLY for remote physical port.
*         the returned value is the LOCAL port value
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to convert (must be of type 'remote physical port')
* @param[in] convertedPortType        - converted port type
*
* @param[out] convertedPortNumPtr      - (pointer to) converted port, according to port type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum (physicalPortNum not 'remote')
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    IN  PRV_CPSS_DXCH_PORT_TYPE_ENT     convertedPortType,
    OUT GT_U32                          *convertedPortNumPtr
)
{
    return portMapCheckAndConvert(devNum,physicalPortNum,convertedPortType,convertedPortNumPtr,
        GT_TRUE);/*allowRemotePhyPort --> GT_TRUE*/
}

/**
* @internal prvCpssDxChPortRemotePortCheck function
* @endinternal
*
* @brief   Function returns indication if the port is 'remote' physical port or not.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port
*                                       GT_TRUE : The port is   'remote' physical port.
*                                       GT_FALSE : The port is NOT 'remote' physical port.
*/
GT_BOOL prvCpssDxChPortRemotePortCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum
)
{
    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return GT_FALSE;
    }
    else /* check size of portsMapInfoShadowArr. */
    if(physicalPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        return GT_FALSE;
    }
    else /* portsMapInfoShadowArr array is zeroed on creation.
           only configured entries has non zero data. */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].valid != GT_TRUE)
    {
        /* take care of non configured port as NOT 'remote' */
        return GT_FALSE;
    }
    else
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType !=
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal prvCpssDxChPortReservedPortCheck function
* @endinternal
*
* @brief   Function returns 'true' if the port is 'reserved' physical port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @retval GT_TRUE                  - the port is 'reserved' physical port.
* @retval GT_FALSE                 - the port is NOT 'reserved' physical port
*                                       or not applicable device.
*/
GT_BOOL prvCpssDxChPortReservedPortCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_U32                          i;
    const GT_PHYSICAL_PORT_NUM            *portsArrPtr;


    /* Get reserved ports array */
    portsArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.reservedPortsPtr;
    if (portsArrPtr == NULL)
    {
        /* Do nothing - not supported for this device type */
        return GT_FALSE;
    }

    /* Iterate all ports in array */
    i = 0;
    while (portsArrPtr[i] != GT_NA)
    {
        if (portNum == portsArrPtr[i])
        {
            /* Reserved port found in data base */
            return GT_TRUE;
        }
        i++;
    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] origPortType             - original port type
* @param[in] origPortNum              - MAC/TXQ/DMA port number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapReverseMappingGet
(
    IN  GT_U8                           devNum,
    IN  PRV_CPSS_DXCH_PORT_TYPE_ENT     origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               currentPortValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(physicalPortNumPtr);

    if(origPortType == PRV_CPSS_DXCH_PORT_TYPE_PHYS_E)
    {
        *physicalPortNumPtr = origPortNum;
        return GT_OK;
    }
    /*----------------------------------------------------------------------------------------------*
     * for MAC --> physical port conversion there exists separate table, that allows direct access. *
     * so the search is excessive                                                                   *
     *----------------------------------------------------------------------------------------------*/
    if (origPortType == PRV_CPSS_DXCH_PORT_TYPE_MAC_E)
    {
        GT_U32 macNum = origPortNum;
        if (macNum >= PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *physicalPortNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[macNum];
        if (*physicalPortNumPtr == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            return/* it's not error for LOG*/GT_NOT_FOUND;
        }
        return GT_OK;
    }

    for(portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid != GT_TRUE)
        {
            continue;
        }

        switch(origPortType)
        {
            case PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E:
                currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.rxDmaNum;
                break;

            case PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E:
                currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.txDmaNum;
                break;

         /*
          *  case PRV_CPSS_DXCH_PORT_TYPE_MAC_E:
          *      currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.macNum;
          *      break;
          */

            case PRV_CPSS_DXCH_PORT_TYPE_TXQ_E:
                currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.txqNum;
                break;

            case PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E:
                currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.ilknChannel;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(currentPortValue == origPortNum)
        {
            *physicalPortNumPtr = portNum;
            return GT_OK;
        }
    }

    return/* it's not error for LOG*/GT_NOT_FOUND;
}

/**
* @internal prvCpssDxChPortEventPortMapConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping for port per event type
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] evConvertType            - event convert type
* @param[in] portNumFrom              - port to convert
*
* @param[out] portNumToPtr             - (pointer to) converted port, according to convert event type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEventPortMapConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          portNumFrom,
    OUT GT_U32                          *portNumToPtr
)
{
    GT_STATUS   rc;

    CPSS_NULL_PTR_CHECK_MAC(portNumToPtr);

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        *portNumToPtr = portNumFrom;
        return GT_OK;
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch (evConvertType)
    {
        case PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E:
            if (PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc)
            {
                rc = PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc(
                    devNum,evConvertType, &portNumFrom);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            /* convert MAC-to-PHY */
            rc = cpssDxChPortPhysicalPortMapReverseMappingGet((GT_U8)devNum,
                                                              CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,
                                                              portNumFrom, /*OUT*/portNumToPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E:
            /* convert PHY-to-MAC */
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert((GT_U8)devNum, portNumFrom,
                                                               PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                                                               portNumToPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
            if (PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc)
            {
                rc = PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc(
                    devNum,evConvertType, portNumToPtr);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "evConvertType[%d] : illegal event's convert type", evConvertType);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet function
* @endinternal
 *
* @brief    Function checks and returns parray of physical ports that have same dma
 *
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   - device number
* @param[in] dmaNum                  -  dma index
* @param[out] physicalPortNumArrSize              - number of ports that have same dma
* @param[out] physicalPortNumArr             - array of ports that have same dma
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet
(
    IN  GT_U8                                                         devNum,
    IN  GT_U32                                                       dmaNum,
    OUT GT_U32                                                  *physicalPortNumArrSize,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumArr
)
    {
        GT_PHYSICAL_PORT_NUM portNum;
        GT_U32               currentPortValue;

        PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
        CPSS_NULL_PTR_CHECK_MAC(physicalPortNumArrSize);
        CPSS_NULL_PTR_CHECK_MAC(physicalPortNumArr);

        *physicalPortNumArrSize =0 ;

        for(portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid != GT_TRUE)
            {
                continue;
            }

           currentPortValue = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.rxDmaNum;

            if(currentPortValue == dmaNum)
            {
                physicalPortNumArr[*physicalPortNumArrSize] = portNum;
                (*physicalPortNumArrSize)++;
            }
        }

        if(*physicalPortNumArrSize == 0)
        {
            return /* not error for the CPSS LOG */ GT_NOT_FOUND;
        }

        return GT_OK;
    }


