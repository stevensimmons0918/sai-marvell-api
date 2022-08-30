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
* @file cpssPxPortMapping.c
*
* @brief CPSS PX physical port mapping
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define IS_IN_BMP(index,bmp) ((bmp & (1<<(index))) ? 1 : 0)
#define SET_IN_BMP(index,bmp) bmp |= 1<<(index)

/**
* @internal physicalPortMapCheck function
* @endinternal
*
* @brief   do validity check on ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS physicalPortMapCheck
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_PX_PORT_MAP_STC       *portMapArrayPtr
)
{
    CPSS_PX_PORT_MAP_STC *currPtr;
    GT_U32  ii;
    GT_U32  phyPortsBmp = 0; /* BMP of physical ports used */
    GT_U32  interfaceNumberBmp = 0;/* BMP of interfaces number used */

    currPtr = &portMapArrayPtr[0];

    /*********************************/
    /* validity checks on parameters */
    /*********************************/
    for(ii = 0 ; ii < portMapArraySize; ii++,currPtr++)
    {
        PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,currPtr->physicalPortNumber);

        if(IS_IN_BMP(currPtr->physicalPortNumber,phyPortsBmp))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The <physicalPortNumber> [%d] number exists more than once",
                currPtr->physicalPortNumber);
        }
        SET_IN_BMP(currPtr->physicalPortNumber,phyPortsBmp);


        if(currPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            /* the interface number is the 'MAC number' and must be in the BMP
               of existing 'ports' */
            PRV_CPSS_PORT_MAC_CHECK_MAC(devNum,currPtr->interfaceNum);

            /* do check for 'out of range' for 'allocations of arrays' that uses
               size PRV_CPSS_PX_GOP_PORTS_NUM_CNS */
            CPSS_PARAM_CHECK_MAX_MAC(currPtr->interfaceNum,PRV_CPSS_PX_GOP_PORTS_NUM_CNS);

            if(IS_IN_BMP(currPtr->interfaceNum,interfaceNumberBmp))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "The <interfaceNum> [%d] number exists more than once",
                    currPtr->interfaceNum);
            }
            SET_IN_BMP(currPtr->interfaceNum,interfaceNumberBmp);

        }
        else
        if(currPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* no checking of the <interfaceNum> because hold no 'MAC' */
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "unknown mappingType[%d] (supporting only 'ETHERNET/CPU_SDMA')",
                currPtr->mappingType);
        }
    }


    return GT_OK;
}

/**
* @internal rxDMA2LocalPhysSet function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS rxDMA2LocalPhysSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN GT_U32 rxDmaNum,
    IN GT_PHYSICAL_PORT_NUM physPort
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(rxDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->rxDMA.
        singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum];

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,9,physPort);

    return rc;
}

/*----------------------------------------------------------------
 *    TxDMA 0 :  128*4 = 512 entries (local phys port)   [x] = 5
 *    TxDMA 1 :  128*4 = 512                             [x] = 5
 *    TxDMA 2 :                                          [x] = 5
 *    TxDMA 3 :                                          [x] = 5
 *    TxDMA 4 :                                          [x] = 5
 *    TxDMA 5 :                                          [x] = 5
 *  example:
 *    local-phys-port         globalDma,         dp-index,  localDma
 *        x               ,{     29,       {         2,         5 }    }
 *
 *----------------------------------------------------------------*/
static GT_STATUS txDMALocalPhys2RxDMASet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    IN  GT_U32 rxDmaNum

)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register's address */
    GT_U32      fldOffs;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(rxDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    /* 4 ports in register */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->txDMA.txDMAGlobalConfigs.
        localDevSrcPort2DMANumberMap[physPort/4];

    fldOffs = (physPort % 4) * 8;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,fldOffs,8,rxDmaNum);

    return rc;
}

/**
* @internal BMAMapOfLocalPhys2RxDMASet function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - physical port
* @param[in] rxDmaNum                 - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS BMAMapOfLocalPhys2RxDMASet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32 rxDmaNum
)
{
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(rxDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    /* need to write the 'local' DMA number */
    rc = prvCpssPxWriteTableEntry(devNum,
                                    CPSS_PX_TABLE_BMA_PORT_MAPPING_E,
                                    physPort,
                                    &rxDmaNum);
    return rc;
}

/*******************************************************************************
* TxQPort2TxDMAMapSet
*
* DESCRIPTION:
*       configure TxQ port 2 TxDMA  mapping
*
* APPLICABLE DEVICES:
*        Pipe
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - device number
*       portGroupId - port group id
*       txqNum     - txq port
*       txDmaNum   - txDMA port
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum
*       GT_BAD_PTR      - on bad pointer
*
* COMMENTS:
*
*******************************************************************************/
/*static*/ GT_STATUS TxQPort2TxDMAMapSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(txDmaNum,PRV_CPSS_PX_DMA_PORTS_NUM_CNS);
    CPSS_PARAM_CHECK_MAX_MAC(txqNum,PRV_CPSS_PX_MAX_DQ_PORTS_CNS);

    /* need to write the 'local' DMA number */
    /* NOTE: the DB of portToDMAMapTable[txqNum] hold addresses from DQ[0] and from DQ[1] !!! */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[txqNum];
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,0,8,txDmaNum);
    return rc;
}

/**
* @internal prvCpssPxPFCResponsePhysPort2TxQMapSet function
* @endinternal
*
* @brief   configure PFC response Phys port mapping to txq
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - phys port
* @param[in] txqNum                   - txq port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssPxPFCResponsePhysPort2TxQMapSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32 txqNum
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);
    CPSS_PARAM_CHECK_MAX_MAC(txqNum,PRV_CPSS_PX_MAX_DQ_PORTS_CNS);


    return(prvCpssPxWriteTableEntry(devNum,
                                    CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E,
                                    physPort,
                                    &txqNum));

}

/**
* @internal PFCTriggerPhysPort2MACMapSet function
* @endinternal
*
* @brief   configure PFC trigger Phys port mapping to mac number
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - phys port
* @param[in] macNum                   - mac port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS PFCTriggerPhysPort2MACMapSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32 macNum
)
{
    GT_STATUS rc;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCSourcePortToPFCIndexMap[physPort];

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 8, macNum);

    return rc;
}

/**
* @internal CPU_2_TxQPortSet function
* @endinternal
*
* @brief   configure CPU txq number
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - txq number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS CPU_2_TxQPortSet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32 txqNum
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    dpIdx,localTxQNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    dpIdx       = txqNum / PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS;
    localTxQNum = txqNum % PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS;

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).global.globalDQConfig.globalDequeueConfig;

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1,7,localTxQNum);

    return rc;
}

/**
* @internal physicalPortMapSet function
* @endinternal
*
* @brief   set (single) port mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMapPtr               - pointer to port mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS physicalPortMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_PORT_MAP_STC       *portMapPtr
)
{
    GT_STATUS   rc;
    CPSS_PX_DETAILED_PORT_MAP_STC       *detailedShadowPtr;
    CPSS_PX_SHADOW_PORT_MAP_STC         *shadowPtr;
    GT_U32  dmaNumber;
    GT_U32  rxDmaNum,txDmaNum;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    detailedShadowPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portMapPtr->physicalPortNumber];

    shadowPtr = &detailedShadowPtr->portMap;
    shadowPtr->mappingType = portMapPtr->mappingType;

    if(portMapPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        dmaNumber = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
        shadowPtr->macNum = GT_PX_NA;
    }
    else
    {
        dmaNumber = portMapPtr->interfaceNum;
        shadowPtr->macNum = portMapPtr->interfaceNum;
    }

    shadowPtr->dmaNum   = dmaNumber;

    rxDmaNum = dmaNumber;
    txDmaNum = dmaNumber;

    /* The <txqPortNumber> is equal to <physicalPortNumber> */
    /* HARD WIRED in the device */
    shadowPtr->txqNum = portMapPtr->physicalPortNumber;

    if ((system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E ) &&
        (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) )
    {
        /* 1. RX DMA to Local Device Physical Source Port */
        rc = rxDMA2LocalPhysSet(devNum,
                rxDmaNum,
                portMapPtr->physicalPortNumber);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* 2. TXDMA Local Device Physical Port to RX DMA number */
        rc = txDMALocalPhys2RxDMASet(devNum,
                portMapPtr->physicalPortNumber,
                rxDmaNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* 3. BMA map of Local Device Physical Port to RX DMA number */
        rc = BMAMapOfLocalPhys2RxDMASet(devNum,
                portMapPtr->physicalPortNumber,
                rxDmaNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* 5. TxQ port to TX DMA map */
        rc = TxQPort2TxDMAMapSet(devNum,
                shadowPtr->txqNum,
                txDmaNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(shadowPtr->macNum != GT_PX_NA)
        {
            /* 7. PFC Trigger Local Device Source Port to MAC map */
            rc = PFCTriggerPhysPort2MACMapSet(devNum,
                    portMapPtr->physicalPortNumber,
                    shadowPtr->macNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(portMapPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* 9. CPU TxQ port number  */
            rc = CPU_2_TxQPortSet(devNum,
                    shadowPtr->txqNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    detailedShadowPtr->valid = GT_TRUE;

    /*-------------------------------*
     * inverse MAC--> Physical port  *
     *-------------------------------*/
    if(shadowPtr->macNum != GT_PX_NA)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[shadowPtr->macNum] = portMapPtr->physicalPortNumber;
    }



    return GT_OK;
}


/**
* @internal internal_cpssPxPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       CPSS_PX_PORT_MAP_STC exampleMap[] =
*       (
*       -- the list for 'network ports' : 10G each
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E,   0, 0),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E,   1, 1),
*       ...
*       ( 11, _INTRFACE_TYPE_ETHERNET_MAC_E,   11, 11),
*       -- the 'uplink port' : 100G
*       ( 12, _INTRFACE_TYPE_ETHERNET_MAC_E,   12, 12),
*       -- the SDMA CPU
*       ( 16, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, 16),
*       );
*       rc = cpssPxCfgPhysicalPortMapSet(devNum, sizeof(exampleMap)/sizeof(exampleMap[0]), &exampleMap[0]);
*       if(rc != GT_OK)
*       (
*       fatal error.
*       )
*
*/
static GT_STATUS internal_cpssPxPortPhysicalPortMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_PX_PORT_MAP_STC       *portMapArrayPtr
)
{
    GT_STATUS   rc;
    CPSS_PX_PORT_MAP_STC *currPtr;
    GT_U32  ii;
    GT_U32  bmpOldMapping = 0;
    GT_U32  bmpOldReverseMapping = 0;
    CPSS_PX_DETAILED_PORT_MAP_STC       *detailedShadowPtr;
    GT_U32  bmpNewMapping = 0;
    GT_U32  bmpNewReverseMapping = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portMapArrayPtr);

    /*********************************/
    /* validity checks on parameters */
    /*********************************/
    rc = physicalPortMapCheck(devNum,portMapArraySize,portMapArrayPtr);
    if(rc != GT_OK)
    {
        return rc;
    }


    detailedShadowPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[0];
    /* get mapping of current info */
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++,detailedShadowPtr++)
    {
        if(detailedShadowPtr->valid)
        {
            SET_IN_BMP(ii,bmpOldMapping);
            if(detailedShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                SET_IN_BMP(detailedShadowPtr->portMap.macNum,bmpOldReverseMapping);
            }
        }
    }

    currPtr = &portMapArrayPtr[0];

    /* set mapping of current info */
    for(ii = 0 ; ii < portMapArraySize; ii++,currPtr++)
    {
        rc = physicalPortMapSet(devNum,currPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        SET_IN_BMP(currPtr->physicalPortNumber,bmpNewMapping);
        if(currPtr->mappingType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            SET_IN_BMP(currPtr->interfaceNum,bmpNewReverseMapping);
        }
    }

    /* remove mapping of OLD info */
    detailedShadowPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[0];
    /* get mapping of new info */
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++,detailedShadowPtr++)
    {
        if(IS_IN_BMP(ii,bmpNewMapping))
        {
            /* already valid in new setting */
            continue;
        }

        if(0 == IS_IN_BMP(ii,bmpOldMapping))
        {
            /* was not valid in old setting */
            continue;
        }

        /* was valid in old setting , but not valid in new setting */
        detailedShadowPtr->valid = GT_FALSE;
        detailedShadowPtr->portMap.mappingType = GT_PX_NA;
        detailedShadowPtr->portMap.macNum      = GT_PX_NA;
        detailedShadowPtr->portMap.dmaNum      = GT_PX_NA;
        detailedShadowPtr->portMap.txqNum      = GT_PX_NA;
    }

    for(ii = 0 ; ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; ii++)
    {
        if(IS_IN_BMP(ii,bmpNewReverseMapping))
        {
            /* already valid in new setting */
            continue;
        }

        if(0 == IS_IN_BMP(ii,bmpOldReverseMapping))
        {
            /* was not valid in old setting */
            continue;
        }

        /* was valid in old setting , but not valid in new setting */
        PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[ii] = GT_PX_NA;
    }

    /* Initialize data structure for port resource allocation */
    rc = prvCpssPxPortResourcesInit(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortResourcesInit");
    }

    return GT_OK;
}

/**
* @internal cpssPxPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       CPSS_PX_PORT_MAP_STC exampleMap[] =
*       (
*       -- the list for 'network ports' : 10G each
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E,   0, 0),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E,   1, 1),
*       ...
*       ( 11, _INTRFACE_TYPE_ETHERNET_MAC_E,   11, 11),
*       -- the 'uplink port' : 100G
*       ( 12, _INTRFACE_TYPE_ETHERNET_MAC_E,   12, 12),
*       -- the SDMA CPU
*       ( 16, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, 16),
*       );
*       rc = cpssPxCfgPhysicalPortMapSet(devNum, sizeof(exampleMap)/sizeof(exampleMap[0]), &exampleMap[0]);
*       if(rc != GT_OK)
*       (
*       fatal error.
*       )
*
*/
GT_STATUS cpssPxPortPhysicalPortMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_PX_PORT_MAP_STC       *portMapArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portMapArraySize,portMapArrayPtr));

    rc = internal_cpssPxPortPhysicalPortMapSet(devNum,portMapArraySize,portMapArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portMapArraySize,portMapArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal BMAMapOfLocalPhys2RxDMAGet function
* @endinternal
*
* @brief   get mapping of BMA local physical port 2 RxDMA port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - physical port
*
* @param[out] rxDmaPortPtr             - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS BMAMapOfLocalPhys2RxDMAGet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32 *rxDmaPortPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxDmaPortPtr);

    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,physPort);

    rc = prvCpssPxReadTableEntry(devNum,
                                    CPSS_PX_TABLE_BMA_PORT_MAPPING_E,
                                    physPort,
                                    rxDmaPortPtr);

    return rc;
}

/**
* @internal PFCTriggerPhysPort2MACMapGet function
* @endinternal
*
* @brief   configure PFC trigger Phys port mapping to mac number
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physPort                 - phys port
*
* @param[out] macNumPtr                - mac port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS PFCTriggerPhysPort2MACMapGet
(
    IN GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32 *macNumPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macNumPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCSourcePortToPFCIndexMap[physPort];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 8, macNumPtr);

    return rc;
}


/**
* @internal RxDMA2LocalPhysGet function
* @endinternal
*
* @brief   RxDMA 2 phys port mapping get
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxDmaNum                 - rxDMA port
*
* @param[out] physPortPtr              - (pointer to) physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS RxDMA2LocalPhysGet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32 rxDmaNum,
    OUT GT_PHYSICAL_PORT_NUM *physPortPtr
)
{
    GT_STATUS rc;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(physPortPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->rxDMA.singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum];
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr,0,9,physPortPtr);

    return rc;
}

/**
* @internal TxQPort2TxDMAMapGet function
* @endinternal
*
* @brief   get mapping of TxQ port 2 TxDMA
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - txq port
*
* @param[out] txDmaNumPtr              - txDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS TxQPort2TxDMAMapGet
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_U32  txqNum,
    OUT GT_U32 *txDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(txqNum,PRV_CPSS_PX_MAX_DQ_PORTS_CNS);

    /* NOTE: the DB of portToDMAMapTable[txqNum] hold addresses from DQ[0] and from DQ[1] !!! */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[txqNum];

    /* reading the 'local' DMA number */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, txDmaNumPtr);

    return rc;
}


/**
* @internal physicalPortMapGet function
* @endinternal
*
* @brief   get (single) port mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port
*
* @param[out] portMapPtr               - pointer to (single) port mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS physicalPortMapGet
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_PHYSICAL_PORT_NUM     physicalPortNum,
    OUT CPSS_PX_PORT_MAP_STC  *portMapPtr
)
{
    GT_STATUS   rc;
    GT_U32  rxDmaNum;
    GT_U32  txDmaNum;
    GT_U32  macNum;
    GT_U32  txqNum;
    GT_PHYSICAL_PORT_NUM rxDmaToPhysicalPortNum;

    rc = BMAMapOfLocalPhys2RxDMAGet(devNum,physicalPortNum,/*OUT*/&rxDmaNum); /* 3  Phys --> RxDma */
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = PFCTriggerPhysPort2MACMapGet(devNum,physicalPortNum,/*OUT*/&macNum);  /* 7 Phys --> Mac */
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = RxDMA2LocalPhysGet(devNum,rxDmaNum,/*OUT*/&rxDmaToPhysicalPortNum);  /* 1 RxDma --> Phys */
    if (rc != GT_OK)
    {
        return rc;
    }

    if(rxDmaToPhysicalPortNum != physicalPortNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "mismatch : The Physical port[%d] bound to RxDMA[%d] which bound to other Physical port[%d]",
            physicalPortNum,
            rxDmaNum,
            rxDmaToPhysicalPortNum);
    }

    /* the logic is HARD CODED in the device */
    txqNum = physicalPortNum;

    rc = TxQPort2TxDMAMapGet(devNum,txqNum,/*OUT*/&txDmaNum);             /* 5 TxQ --> txDma */
    if (rc != GT_OK)
    {
        return rc;
    }

    if(txDmaNum != rxDmaNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "mismatch : The Physical port[%d] hold RxDMA[%d] != TxDMA[%d]",
            physicalPortNum,
            rxDmaNum,
            txDmaNum);
    }

    portMapPtr->physicalPortNumber = physicalPortNum;

    if(txDmaNum == PRV_CPSS_PX_CPU_DMA_NUM_CNS)
    {
        portMapPtr->mappingType = CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E;
        portMapPtr->interfaceNum = GT_PX_NA;
    }
    else
    {
        portMapPtr->mappingType = CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        portMapPtr->interfaceNum = macNum;

        if(macNum != txDmaNum)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "mismatch : The Physical port[%d] hold macNum[%d] != TxDMA[%d]",
                physicalPortNum,
                macNum,
                txDmaNum);
        }
    }


    return GT_OK;
}

/**
* @internal internal_cpssPxPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPhysicalPortMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_PX_PORT_MAP_STC      *portMapArrayPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      ii;
    GT_PHYSICAL_PORT_NUM        physPort;
    CPSS_PX_PORT_MAP_STC      *portMapPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portMapArrayPtr);

    if(portMapArraySize == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portMapArraySize ZERO value is not allowed");
    }
    else
    if ((firstPhysicalPortNumber + portMapArraySize) > PRV_CPSS_PX_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "firstPhysicalPortNumber[%d]+portMapArraySize[%d] must be less than[%d]",
            firstPhysicalPortNumber,
            portMapArraySize,
            PRV_CPSS_PX_PORTS_NUM_CNS);
    }

    portMapPtr = &portMapArrayPtr[0];
    physPort   = firstPhysicalPortNumber;

    for ( ii = 0; ii < portMapArraySize ; ii++, portMapPtr++, physPort++)
    {
        rc = physicalPortMapGet(devNum,physPort,portMapPtr);
        if (GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;

}


/**
* @internal cpssPxPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_PX_PORT_MAP_STC      *portMapArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr));

    rc = internal_cpssPxPortPhysicalPortMapGet(devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPhysicalPortMapIsValidGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);

    *isValidPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid;

    return GT_OK;
}

/**
* @internal cpssPxPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapIsValidGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortMapIsValidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isValidPtr));

    rc = internal_cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum, isValidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isValidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS prvCpssPxPortPhysicalPortMapReverseMappingGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  PRV_CPSS_PX_PORT_TYPE_ENT     origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               currentPortValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(physicalPortNumPtr);

    if(origPortType == PRV_CPSS_PX_PORT_TYPE_PHYS_E)
    {
        *physicalPortNumPtr = origPortNum;
        return GT_OK;
    }
    /*----------------------------------------------------------------------------------------------*
     * for MAC --> physical port conversion there exists separate table, that allows direct access. *
     *----------------------------------------------------------------------------------------------*/
    if (origPortType == PRV_CPSS_PX_PORT_TYPE_MAC_E)
    {
        GT_U32 macNum = origPortNum;
        /* the 'MAC number' must be in the BMP of existing 'ports' */
        PRV_CPSS_PORT_MAC_CHECK_MAC(devNum,macNum);

        *physicalPortNumPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[macNum];
        if (*physicalPortNumPtr == GT_PX_NA)
        {
            return/* it's not error for LOG*/GT_NOT_FOUND;
        }

        return GT_OK;
    }

    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        if(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid != GT_TRUE)
        {
            continue;
        }

        switch(origPortType)
        {
            case PRV_CPSS_PX_PORT_TYPE_TxDMA_E:
            case PRV_CPSS_PX_PORT_TYPE_RxDMA_E:
                currentPortValue = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.dmaNum;
                break;
            case PRV_CPSS_PX_PORT_TYPE_TXQ_E:
                currentPortValue = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.txqNum;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(origPortType);
        }

        if(currentPortValue == origPortNum)
        {
            *physicalPortNumPtr = portNum;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "origPortType[%d] origPortNum[%d] : not found matched physical port number",
        origPortType,origPortNum);
}

/**
* @internal internal_cpssPxPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
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
static GT_STATUS internal_cpssPxPortPhysicalPortMapReverseMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    if(origPortType == CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        return prvCpssPxPortPhysicalPortMapReverseMappingGet(devNum,
            PRV_CPSS_PX_PORT_TYPE_MAC_E,origPortNum,physicalPortNumPtr);
    }

    if(origPortType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        /* get the physical port number of the CPU port */
        return prvCpssPxPortPhysicalPortMapReverseMappingGet(devNum,
            PRV_CPSS_PX_PORT_TYPE_RxDMA_E,
            PRV_CPSS_PX_CPU_DMA_NUM_CNS,/* use the DMA of the CPU */
            physicalPortNumPtr);
    }

    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(origPortType);
}

/**
* @internal cpssPxPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssPxPortPhysicalPortMapReverseMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortMapReverseMappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, origPortType, origPortNum, physicalPortNumPtr));

    rc = internal_cpssPxPortPhysicalPortMapReverseMappingGet(devNum, origPortType,
        origPortNum, physicalPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, origPortType, origPortNum, physicalPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPhysicalPortDetailedMapGet function
* @endinternal
*
* @brief   Function returns physical port detailed mapping
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMapShadowPtr         - (pointer to) detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPhysicalPortDetailedMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr
)
{

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portMapShadowPtr);

    /*---------------------------------*
     *   copy data                     *
     *---------------------------------*/
    *portMapShadowPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum];

    return GT_OK;
}

/**
* @internal cpssPxPortPhysicalPortDetailedMapGet function
* @endinternal
*
* @brief   Function returns physical port detailed mapping
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMapShadowPtr         - (pointer to) detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortDetailedMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortDetailedMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMapShadowPtr));

    rc = internal_cpssPxPortPhysicalPortDetailedMapGet(devNum,portNum,portMapShadowPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMapShadowPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortPhysicalPortMapCheckAndConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS prvCpssPxPortPhysicalPortMapCheckAndConvert
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    IN  PRV_CPSS_PX_PORT_TYPE_ENT     convertedPortType,
    OUT GT_U32                          *convertedPortNumPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, physicalPortNum);
    CPSS_NULL_PTR_CHECK_MAC(convertedPortNumPtr);

    /* check that valid mapping exists (except PHYS to PHYS check and convert) */
    if((convertedPortType != PRV_CPSS_PX_PORT_TYPE_PHYS_E) &&
       (PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].valid != GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : convertedPortType[%d] not valid in shadow",
            physicalPortNum,convertedPortType);
    }

    switch(convertedPortType)
    {
        case PRV_CPSS_PX_PORT_TYPE_PHYS_E:
            *convertedPortNumPtr = physicalPortNum;
            break;

        case PRV_CPSS_PX_PORT_TYPE_RxDMA_E:
        case PRV_CPSS_PX_PORT_TYPE_TxDMA_E:
            *convertedPortNumPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.dmaNum;
            break;

        case PRV_CPSS_PX_PORT_TYPE_MAC_E:
            if(PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                /* CPU port has no MAC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physicalPortNum[%d] : is 'CPU SDMA' so hold no 'MAC'",
                    physicalPortNum);
            }

            *convertedPortNumPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.macNum;
            break;

        case PRV_CPSS_PX_PORT_TYPE_TXQ_E:
            *convertedPortNumPtr = PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[physicalPortNum].portMap.txqNum;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(convertedPortType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortEventPortMapConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping for port per event type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS prvCpssPxPortEventPortMapConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          portNumFrom,
    OUT GT_U32                          *portNumToPtr
)
{
    GT_STATUS   rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portNumToPtr);

    switch (evConvertType)
    {
        case PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E:
            /* convert MAC-to-PHY */
            rc = cpssPxPortPhysicalPortMapReverseMappingGet((GT_U8)devNum,
                                                              CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E,
                                                              portNumFrom, /*OUT*/portNumToPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E:
            /* convert PHY-to-MAC */
            rc = prvCpssPxPortPhysicalPortMapCheckAndConvert((GT_U8)devNum, portNumFrom,
                                                               PRV_CPSS_PX_PORT_TYPE_MAC_E,
                                                               portNumToPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(evConvertType);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPpResourcesTxqGlobal2LocalConvert function
* @endinternal
*
* @brief   Function converts global TXQ to dq idx and local txq-dq port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - global TXQ port number
*
* @param[out] dqIdxPtr                 - (pointer to) dq idx
* @param[out] localTxqNumPtr           - (pointer to) local txq number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPpResourcesTxqGlobal2LocalConvert
(

    IN GT_SW_DEV_NUM devNum,
    IN GT_U32 txqNum,
    OUT GT_U32 *dqIdxPtr,
    OUT GT_U32 *localTxqNumPtr
)
{
    GT_U32 txqPorts;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(dqIdxPtr);
    CPSS_NULL_PTR_CHECK_MAC(localTxqNumPtr);

    txqPorts = PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq *
               PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
    if (txqNum >= txqPorts)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *dqIdxPtr       = txqNum / PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
    *localTxqNumPtr = txqNum % PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
    return GT_OK;
}

/**
* @internal internal_cpssPxPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPhysicalPortMapIsCpuGet
(
    IN GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(isCpuPtr);

    if ((PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid == GT_TRUE)&&
        (PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.mappingType
                                                    == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E))
    {
        *isCpuPtr = GT_TRUE;
    }
    else
    {
        *isCpuPtr = GT_FALSE;
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapIsCpuGet
(
    IN    GT_SW_DEV_NUM           devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPhysicalPortMapIsCpuGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isCpuPtr));

    rc = internal_cpssPxPortPhysicalPortMapIsCpuGet(devNum, portNum, isCpuPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isCpuPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



