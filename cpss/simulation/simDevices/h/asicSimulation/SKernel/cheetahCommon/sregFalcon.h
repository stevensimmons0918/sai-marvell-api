/******************************************************************************
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
* @file sregFalcon.h
*
* @brief Defines for Falcon memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregFalconh
#define __sregFalconh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <asicSimulation/SKernel/smem/smemFalcon.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>

/* Ingress/Egress Policer QOS attribute table */
#define SMEM_SIP6_POLICER_QOS_ATTRIBUTE_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerQosAttributes, index, cycle)

/* IPvX SIP6 - Router Extra Management Counters Set 0,1,2,3*/
#define SMEM_SIP6_IPVX_ROUTER_EXTRA_MANAGEMENT_COUNTER_CONFIGURATION_REG(dev,index)    \
    SIP_6_REG(dev,IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig2[(index)])

/* IPVX ECMP  Entry */
#define SMEM_SIP6_IPVX_ECMP_ENTRY_TBL_MEM(dev,entry_indx)          \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipvxEcmp, entry_indx)

/* IPVX - IPv4 FDB lookup mask */
#define SMEM_SIP6_IPVX_FDB_IPV4_LOOKUP_MASK_REG(dev)        \
    SIP_6_REG(dev,IPvX.FdbIpLookup.FdbIpv4RouteLookupMask)

/* IPVX - IPv6 FDB lookup mask */
#define SMEM_SIP6_IPVX_FDB_IPV6_LOOKUP_MASK_REG(dev, index)        \
    SIP_6_REG(dev,IPvX.FdbIpLookup.FdbIpv6RouteLookupMask[(index)])

/* IPVX - IPv4 FDB host lookup register */
#define SMEM_SIP6_IPVX_FDB_HOST_LOOKUP_REG(dev)        \
    SIP_6_REG(dev,IPvX.FdbRouteLookup.FdbHostLookup)

/*L2I - global EPort Min Value register */
#define SMEM_SIP6_L2I_BRIDGE_GLOBAL_E_PORT_MIN_VALUE_REG(dev) \
    SIP_6_REG(dev,L2I.globalEportConifguration.globalEportMinValue)

/*L2I - global EPort Max Value register */
#define SMEM_SIP6_L2I_BRIDGE_GLOBAL_E_PORT_MAX_VALUE_REG(dev) \
    SIP_6_REG(dev,L2I.globalEportConifguration.globalEportMaxValue)

/* FDB - Global Configuration 2 Register  */
#define SMEM_SIP6_FDB_GLOBAL_CONFIG_2_REG(dev)                \
    SIP_6_REG(dev,FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig2)

/* PREQ - Target Physical Port Table */
#define SMEM_SIP6_PREQ_TARGET_PHYSICAL_PORT_TBL_MEM(dev, phyPortNum) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqTargetPhyPort, phyPortNum)

/* PREQ - Queue Port Mapping Table */
#define SMEM_SIP6_PREQ_QUEUE_PORT_MAPPING_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqQueuePortMapping, index)


/* TXQ-PDX - Queue Group Map table */
#define SMEM_SIP6_TXQ_PDX_QUEUE_GROUP_MAP_TBL_MEM(dev, queue_index, instance) \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, txqPdxQueueGroupMap, queue_index, instance)

/* TXQ-SDQ - port range 'low' */
#define SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_LOW_REG(dev,sdqIndex,index) \
    SIP_6_REG(dev,SIP6_TXQ_SDQ[sdqIndex].PortConfig.PortRangeLow[index])

/* TXQ-SDQ - port range 'high' */
#define SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_HIGH_REG(dev,sdqIndex,index) \
    SIP_6_REG(dev,SIP6_TXQ_SDQ[sdqIndex].PortConfig.PortRangeHigh[index])

/* TXQ-SDQ - global port enable */
#define SMEM_SIP6_TXQ_SDQ_GLOBAL_PORT_ENABLE_REG(dev,sdqIndex,index) \
    SIP_6_REG(dev,SIP6_TXQ_SDQ[sdqIndex].PortConfig.PortConfig[index])

/* PREQ Global Configuration Register */
#define SMEM_SIP6_PREQ_GLB_CONF_REG(dev) \
    SIP_6_REG(dev, PREQ.globalConfig)

/* EGF eft evlan CNC config*/
#define SMEM_SIP6_EGF_EVLAN_PASS_DROP_REG(dev) \
    SIP_6_REG(dev, EGF_eft.cncEvlanConfigs.vlanPassDropConfigs)

/* EQ - Ingress Statistic Mirroring to Analyzer <%i> Port Enable Configuration*/
#define SMEM_SIP6_EQ_INGRESS_STATISTIC_MIRRORING_TO_ANALYZER_PORT_ENABLE_REG(dev,analyzerIndex) \
    SIP_6_REG(dev,EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortEnableConfig[analyzerIndex])

/* EQ - Ingress Statistic Mirroring to Analyzer <%i> Port Ratio Configuration */
#define SMEM_SIP6_EQ_INGRESS_STATISTIC_MIRRORING_TO_ANALYZER_PORT_RATIO_REG(dev,analyzerIndex) \
    SIP_6_REG(dev,EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortRatioConfig[analyzerIndex])

/* EQ - ePort ECMP ePort Minimum */
#define SMEM_SIP6_EQ_L2MLL_GLOBAL_E_PORT_MIN_VALUE_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.ePortECMPEPortMinimum)

/* EQ - ePort ECMP ePort Maximum */
#define SMEM_SIP6_EQ_L2MLL_GLOBAL_E_PORT_MAX_VALUE_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.ePortECMPEPortMaximum)


/* L2MLL - global ePort Minimum */
#define SMEM_SIP6_L2_MLL_GLOBAL_E_PORT_MIN_VALUE_REG(dev) \
    SIP_6_REG(dev,MLL.MLLGlobalCtrl.L2mllGlobalEPortMinValue)

/* L2MLL - global ePort Maximum */
#define SMEM_SIP6_L2_MLL_GLOBAL_E_PORT_MAX_VALUE_REG(dev) \
    SIP_6_REG(dev,MLL.MLLGlobalCtrl.L2mllGlobalEPortMaxValue)

/* MLL - multi-target - global ePort Minimum */
#define SMEM_SIP6_MLL_MULTI_TARGET_GLOBAL_E_PORT_MIN_VALUE_REG(dev) \
    SIP_6_REG(dev,MLL.multiTargetEPortMap.multiTargetGlobalEPortMinValue)

/* MLL - multi-target - global ePort Maximum */
#define SMEM_SIP6_MLL_MULTI_TARGET_GLOBAL_E_PORT_MAX_VALUE_REG(dev) \
    SIP_6_REG(dev,MLL.multiTargetEPortMap.multiTargetGlobalEPortMaxValue)

/*  TTI - Remote Physical Port Assignment Global Config Ext*/
#define SMEM_SIP6_TTI_REMOTE_PHY_PORT_ASSIGN_GLB_CONF_EXT_REG(dev) \
    SIP_6_REG(dev,TTI.phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfigExt)

/* TTI - vrf-id evlan mapping */
#define SMEM_SIP6_TTI_VRFID_EVLAN_MAPPING_TBL_MEM(dev, vlanId) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ttiVrfidEvlanMapping, vlanId)

/* TTI Unit Global Configuration Ext 2 */
#define SMEM_SIP6_TTI_TTI_UNIT_GLB_CONF_EXT_2_REG(dev) \
    SIP_6_REG(dev,TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt2)

#define TXDMA_PIZZA_SLICE_PER_DP_REG_MAC(dev,port,sliceNum) \
    SIP_6_REG(dev,sip6_txDMA[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[sliceNum])

#define TXFIFO_PIZZA_SLICE_PER_DP_REG_MAC(dev,port,sliceNum) \
    SIP_6_REG(dev,sip6_txFIFO[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[sliceNum])

#define SMEM_SIP6_TXFIFO_GLOBAL_CONFIG_1_REG_MAC(dev,dpIndex) \
    SIP_6_REG(dev,sip6_txFIFO[dpIndex].configs.globalConfigs.globalConfig1)

#define RXDMA_PER_CHANNEL_REG_MAC(dev,port,regName) \
    SIP_6_REG(dev,rxDMA[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].regName[SMEM_DATA_PATH_RELATIVE_PORT_GET(dev,port)])

#define SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,regName) \
    SIP_6_REG(dev,sip6_rxDMA[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].regName[SMEM_DATA_PATH_RELATIVE_PORT_GET(dev,port)])

/* RXDMA - per channel : PIP : configuration */
#define SMEM_SIP6_RXDMA_PIP_SCDMA_N_PIP_CONFIG_REG(dev,port) \
    RXDMA_PER_CHANNEL_REG_MAC(dev,port,globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg)

/* RXDMA - per channel : cascade port configuration */
#define SMEM_SIP6_RXDMA_CHANNEL_CASCADE_PORT_REG(dev,port) \
    SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,configs.channelConfig.channelCascadePort)

/* RXDMA - per channel : <loca dev source port> configuration */
#define SMEM_SIP6_RXDMA_CHANNEL_TO_LOCAL_DEV_SOURCE_PORT_REG(dev,port) \
    SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,configs.channelConfig.channelToLocalDevSourcePort)

/* RXDMA - per channel : cut through port configuration */
#define SMEM_SIP6_RXDMA_CHANNEL_CUT_THROUGH_CONFIG_REG(dev,port) \
    SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,configs.cutThrough.channelCTConfig)

/* RXDMA - per channel : <PCH Configuration>  */
#define SMEM_SIP6_RXDMA_CHANNEL_PCH_CONFIG_REG(dev,port) \
    SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,configs.channelConfig.PCHConfig)

/* RXDMA - per channel : <channelGeneralConfigs>  */
#define SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(dev,port) \
    SIP6_RXDMA_PER_CHANNEL_REG_MAC(dev,port,configs.channelConfig.channelGeneralConfigs)

/* RXDMA - global registers */
#define SIP6_RXDMA_REG_MAC(dev, unitIndex, regName) \
    SIP_6_REG(dev, sip6_rxDMA[unitIndex].regName)

#define SIP6_OLD_RXDMA_REG_MAC(dev, unitIndex, regName) \
    SIP_6_REG(dev, rxDMA[unitIndex].regName)

/* RXDMA - User Priority cut through enable */
#define SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_UP_ENABLE_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, configs.cutThrough.CTUPEnable)

/* RXDMA - cut through general configuration */
#define SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_GENERAL_CONFIG_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, configs.cutThrough.CTGeneralConfig)

/* RXDMA - cut through generic ethernet type configuration */
#define SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_GENERIC_ETH_CONFIG_REG_MAC(dev, index, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, configs.cutThrough.CTGenericConfigs[index])

/* RxDma: context index PIP priority thresholds0 register */
#define SMEM_SIP6_RXDMA_CONTEXT_INDEX_PIP_PRIO_THRESH0_REG(dev, unitIndex) \
    SIP6_OLD_RXDMA_REG_MAC(dev, unitIndex ,globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds0)

/* RxDma: context index PIP priority thresholds1 register */
#define SMEM_SIP6_RXDMA_CONTEXT_INDEX_PIP_PRIO_THRESH1_REG(dev, unitIndex) \
    SIP6_OLD_RXDMA_REG_MAC(dev, unitIndex, globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds1)


/* RXDMA - dp rx enable debug counters */
#define SMEM_SIP6_RXDMA_DP_RX_ENABLE_DEBUG_COUNTERS_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.dp_rx_enable_debug_counters)

/* RXDMA - rx ingress drop counter */
#define SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNTER_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxIngressDropCounter.rx_ingress_drop_count)

/* RXDMA - rx ingress drop counter config 'pattern' */
#define SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_REF_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxIngressDropCounter.rx_ingress_drop_count_type_ref)

/* RXDMA - rx ingress drop counter config 'mask' */
#define SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_MASK_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxIngressDropCounter.rx_ingress_drop_count_type_mask)

/* RXDMA - RX to CP counter */
#define SMEM_SIP6_RXDMA_RX_TO_CP_COUNTER_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxToCpCounter.rx_to_cp_count)

/* RXDMA - RX to CP counter config 'pattern' */
#define SMEM_SIP6_RXDMA_RX_TO_CP_COUNT_TYPE_REF_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxToCpCounter.rx_to_cp_count_type_ref)

/* RXDMA - RX to CP counter config 'mask' */
#define SMEM_SIP6_RXDMA_RX_TO_CP_COUNT_TYPE_MASK_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxToCpCounter.rx_to_cp_count_type_mask)

/* RXDMA - CP to RX counter */
#define SMEM_SIP6_RXDMA_CP_TO_RX_COUNTER_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxCpToRxCounter.cp_to_rx_count)

/* RXDMA - CP to RX counter config 'pattern' */
#define SMEM_SIP6_RXDMA_CP_TO_RX_COUNT_TYPE_REF_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxCpToRxCounter.cp_to_rx_count_type_ref)

/* RXDMA - CP to RX counter config 'mask' */
#define SMEM_SIP6_RXDMA_CP_TO_RX_COUNT_TYPE_MASK_REG_MAC(dev, unitIndex) \
    SIP6_RXDMA_REG_MAC(dev, unitIndex, debug.rxCpToRxCounter.cp_to_rx_count_type_mask)

/* RXDMA - rx ingress profile TPID */
#define SMEM_SIP6_RXDMA_PIP_PROFILE_TPID_REG(dev,index,unitIndex) \
    SIP6_OLD_RXDMA_REG_MAC(dev, unitIndex , globalRxDMAConfigs.preIngrPrioritizationConfStatus.profileTPID[index])

/* RXDMA - rx latency Sensitive Ether Type register */
#define SMEM_SIP6_RXDMA_PIP_LATENCY_SENSITIVE_ETHER_TYPE_REG(dev, index, unitIndex) \
    SIP6_OLD_RXDMA_REG_MAC(dev, unitIndex , globalRxDMAConfigs.preIngrPrioritizationConfStatus.latencySensitiveEtherTypeConf[index])


/* IA: Ingress Aggregator PIP latency Sensitive Threshold register */
#define SMEM_SIP6_IA_PIP_LATENCY_SENSITIVE_THRESHOLD_REG(dev, unitIndex) \
    SIP_5_20_REG(dev, ingressAggregator[unitIndex].PIPLsThreshold)




/* TTI - global registers */
#define SIP6_TTI_REG_MAC(dev, regName) \
    SIP_6_REG(dev, TTI.regName)

/* TTI - Cut through TTI configuration */
#define SMEM_SIP6_TTI_CUT_THROUGH_CONFIG_REG_MAC(dev) \
    SIP6_TTI_REG_MAC(dev, cutThroughTtiConfig)

/* TTI - Cut through Byte Count Extraction fail counter */
#define SMEM_SIP6_TTI_CUT_THROUGH_BC_EXTRACTION_FAIL_COUNTER_REG_MAC(dev) \
    SIP6_TTI_REG_MAC(dev, cutThroughBcExtactionFailCntr)

/* TTI - Cut through Packet Header Integrity exception configuration1 */
#define SMEM_SIP6_TTI_CUT_THROUGH_HEADER_INTEGRITY_EXCEPTION_CONFIG1_REG_MAC(dev) \
    SIP6_TTI_REG_MAC(dev, cutThroughHeaderIntegrityExceptionConfig1)

/* TTI - Cut through Packet Header Integrity exception configuration2 */
#define SMEM_SIP6_TTI_CUT_THROUGH_HEADER_INTEGRITY_EXCEPTION_CONFIG2_REG_MAC(dev) \
    SIP6_TTI_REG_MAC(dev, cutThroughHeaderIntegrityExceptionConfig2)

/* TTI - ECN Dropped configuration */
#define SMEM_SIP6_TTI_ECN_DROPPED_CONFIG_REG_MAC(dev) \
    SIP6_TTI_REG_MAC(dev, ECNDroppedConfiguration)

/* TTI - TTI Packet Type %p Exact Match profile ID register */
#define SMEM_SIP6_TTI_PACKET_TYPE_EXACT_MATCH_PROFILE_ID_REG(dev, index)     \
    SIP_6_REG(dev,TTI.TTIEngine.TTIPacketTypeExactMatchProfileId[index])

/* TTI - TTI Unit Global Configurations Ext2 */
#define SMEM_SIP6_TTI_UNIT_GLOBAL_CONFIG_EXT2_REG(dev)        \
        SIP6_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt2)

/* TTI - TTI Unit Global Configurations Ext3 */
#define SMEM_SIP6_TTI_UNIT_GLOBAL_CONFIG_EXT3_REG(dev)        \
        SIP6_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt3)

/* D2D Eagle mac Rx TDM calendar*/
#define D2D_EAGLE_MAC_CAL_REG_INDEX_MAC(dev,port,regNum)\
    SIP_6_REG(dev,eagle_d2d[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].d2d.macRx.rxTdmSchdule[regNum])

/* D2D Eagle PCS Rx calendar*/
#define D2D_EAGLE_PCS_RX_CAL_REG_INDEX_MAC(dev,port,regNum)\
    SIP_6_REG(dev,eagle_d2d[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].d2d.pcs.receiveCalSlotConfig[regNum])

/* D2D Eagle PCS Tx calendar*/
#define D2D_EAGLE_PCS_TX_CAL_REG_INDEX_MAC(dev,port,regNum)\
    SIP_6_REG(dev,eagle_d2d[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].d2d.pcs.transmitCalSlotConfig[regNum])

/* D2D Raven PCS Rx calendar*/
#define D2D_RAVEN_PCS_RX_CAL_REG_INDEX_MAC(dev,port,regNum) \
     SIP_6_REG(dev,raven_d2d_pair[smemRavenGlobalPortToRavenIndexGet(dev,port)].d2d[smemRavenGlobalPortToChannelGroupGet(dev, port)].pcs.receiveCalSlotConfig[regNum])

/* D2D Raven PCS Tx calendar*/
#define D2D_RAVEN_PCS_TX_CAL_REG_INDEX_MAC(dev,port,regNum) \
     SIP_6_REG(dev,raven_d2d_pair[smemRavenGlobalPortToRavenIndexGet(dev,port)].d2d[smemRavenGlobalPortToChannelGroupGet(dev, port)].pcs.transmitCalSlotConfig[regNum])

/* D2D Eagle mac Rx TDM calendar*/
#define D2D_RAVEN_MAC_CAL_REG_INDEX_MAC(dev,port,regNum)\
    SIP_6_REG(dev,raven_d2d_pair[smemRavenGlobalPortToRavenIndexGet(dev,port)].d2d[smemRavenGlobalPortToChannelGroupGet(dev, port)].macRx.rxTdmSchdule[regNum])

/* D2D pcs caendar*/
#define D2D_PCS_CAL_REG_INDEX_MAC(dev,port,regNum, isRx,isEagle) \
    (isEagle == GT_TRUE ?( isRx == GT_TRUE ? D2D_EAGLE_PCS_RX_CAL_REG_INDEX_MAC(dev,port,regNum) : D2D_EAGLE_PCS_TX_CAL_REG_INDEX_MAC(dev,port,regNum)): \
    /* Reven*/ \
    (isRx == GT_TRUE ? D2D_RAVEN_PCS_RX_CAL_REG_INDEX_MAC(dev,port,regNum) : D2D_RAVEN_PCS_TX_CAL_REG_INDEX_MAC(dev,port,regNum)))

/* D2D mac caendar*/
#define D2D_MAC_CAL_REG_INDEX_MAC(dev,port,regNum, isEagle) \
    (isEagle == GT_TRUE ? D2D_EAGLE_MAC_CAL_REG_INDEX_MAC(dev,port,regNum): D2D_RAVEN_MAC_CAL_REG_INDEX_MAC(dev,port,regNum))

/* FDB : get pointer to the memory of the FDB entry .
    the FDB memory is NOT in the 'CIDER' address space of the device.
    meaning it is unreachable by direct read/write from the CPU.
    (it was also in sip5 devices , but there we had enough 'fake' address space
    to support it as regular memory with fake 'CIDER' address space)

   NOTE: it replace the use of SMEM_CHT_MAC_TBL_MEM that return 'address'
*/
#define SMEM_SIP6_HIDDEN_FDB_PTR(dev, entry_index) \
    smemGenericHiddenFindMem(dev,SMEM_GENERIC_HIDDEN_MEM_FDB_E,entry_index,NULL)

#define SMEM_FALCON_LPM_MEMORY_TBL_MEM(dev, entry) \
    /* the LPM build of 20 rams of up to 32k lines in each */ \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, lpmMemory, \
        ((entry)%(32*1024))/*line index*/ , ((entry)/(32*1024))/* ram index 0..19*/)
/* lpm aging memory */
#define SMEM_FALCON_LPM_AGING_MEMORY_ENTRY_TBL_MEM(dev,entry_indx)          \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, lpmAgingMemory, entry_indx)

/* Device IDS status */
#define SMEM_FALCON_DFX_SERVER_DEVICE_ID_STATUS_REG(dev)   \
    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(dev)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.deviceIDStatus

/* HA - PTP Exception CPU Code Configuration */
#define SMEM_SIP6_HA_PTP_EXCEPTION_CPU_CODE_CONFIG_REG(dev)      \
        SIP_6_REG(dev, HA.PTPExceptionCPUCodeConfig)


/* TXDMA - global registers */
#define SIP6_TXDMA_REG_MAC(dev, unitIndex, regName) \
    SIP_6_REG(dev, sip6_txDMA[unitIndex].regName)

/* TXDMA - global register : txDMAGlobalConfig */
#define SMEM_SIP6_TXDMA_CUT_TXDMA_GLOBAL_CONFIG_REG_MAC(dev,unitIndex) \
    SIP6_TXDMA_REG_MAC(dev,unitIndex,configs.globalConfigs.txDMAGlobalConfig)
/*EPCL -  Target Physical Port Mapping Table */
#define SMEM_SIP6_EPCL_SRC_PHYSICAL_PORT_MAP_TBL_MEM(dev, entry_index)             \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, epclSourcePhysicalPortMapping, entry_index)

/*EPCL -  Source Physical Port Mapping Table */
#define SMEM_SIP6_EPCL_TRG_PHYSICAL_PORT_MAP_TBL_MEM(dev, entry_index)             \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, epclTargetPhysicalPortMapping, entry_index)

/* EPCL - Exact Match Profile ID Map Entry %n */
#define SMEM_SIP6_EPCL_EXACT_MATCH_PROFILE_ID_MAP_TBL_MEM(dev, entry_index)     \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, epclExactMatchProfileIdMapping, entry_index)

/* PCL : Exact Match Profile ID Map Entry %n */
#define SMEM_SIP6_PCL_EXACT_MATCH_PROFILE_ID_MAP_REG(dev, index)     \
    SIP_6_REG(dev,PCL.exactMatchProfileIdMapEntry[index])

/* Policer Metering Analyzer Indexes */
#define SMEM_SIP6_PLR_ANALYZER_INDEXES_REG(dev, stage)         \
    (SMEM_CHT_IS_SIP6_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[stage].globalConfig.plrAnalyzerIndexes : \
    SMAIN_NOT_VALID_CNS)

/*EPCL -  Port Latency Monitoring Table */
#define SMEM_SIP6_EPCL_PORT_LATENCY_MONITORING_TBL_MEM(dev, entry_index)           \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, epclPortLatencyMonitoring, entry_index)

#define SMEM_SIP6_EPCL_LM_SAMPLING_CFG_REG(_dev,_index) \
    SIP_6_REG(_dev,EPCL.latencyMonitoringSamplingConfig[_index])

#define SMEM_SIP6_EPCL_LM_CONTROL_REG(_dev) \
    SIP_6_REG(_dev,EPCL.latencyMonitoringControl)

#define SMEM_SIP6_EPCL_LM_PROFILE2SAMPLING_PROFILE_REG(_dev,_index) \
    SIP_6_REG(_dev,EPCL.latencyMonitoringProfile2SamplingProfile[_index])

#define SMEM_SIP6_EPCL_LM_SAMPLING_COUNTER_REG(_dev,_index) \
    SIP_6_REG(_dev,EPCL.latencyMonitoringSamplingCounter[_index])

/* EPCL - PHA metadata mask register per lookup */
#define SMEM_SIP6_EPCL_PHA_METADATA_MASK_REG(_dev,_subLookup)   \
    (((_subLookup) == 0) ? SIP_6_REG(_dev,EPCL.PHAMetadataMask1) : \
     ((_subLookup) == 1) ? SIP_6_REG(_dev,EPCL.PHAMetadataMask2) : \
     ((_subLookup) == 2) ? SIP_6_REG(_dev,EPCL.PHAMetadataMask3) : \
     ((_subLookup) == 3) ? SIP_6_REG(_dev,EPCL.PHAMetadataMask4) : \
     SMAIN_NOT_VALID_CNS)

/* EPCL - PHA thread id mask register per lookup */
#define SMEM_SIP6_EPCL_PHA_THREAD_ID_MASK_REG(_dev)   \
    SIP_6_REG(_dev,EPCL.PHAThreadNumMask)

/* internal LMU table address*/
#define __SMEM_SIP6_LMU_TABLE(_dev,_raven,_lmu,_index,tableName) \
    ((_dev->numOfRavens == 0) ?/* not use _raven , and use only LMU[0] + memUnitBaseAddrInfo.lmu[] */ \
        (SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(_dev, tableName, _index, _lmu)) : \
    /* use Raven and LMU[0..max]*/                \
    (SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(_dev, tableName, _index, (_lmu + ((_raven & 1)*2))/*calc global LMU 0..3*/)))

#define SMEM_SIP6_LATENCY_MONITORING_STAT_TBL_MEM(_dev,_raven,_lmu,_index)         \
    __SMEM_SIP6_LMU_TABLE(_dev,_raven,_lmu,_index,lmuStatTable)

#define SMEM_SIP6_LATENCY_MONITORING_CFG_TBL_MEM(_dev,_raven,_lmu,_index)          \
    __SMEM_SIP6_LMU_TABLE(_dev,_raven,_lmu,_index,lmuCfgTable)

/* internal LMU register address*/
#define __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,regName) \
    ((_dev->numOfRavens == 0)?/* not use _raven , and use only LMU[0] + memUnitBaseAddrInfo.lmu[] */ \
        ((SMEM_CHT_MAC_REG_DB_SIP5_GET(_dev)->LMU[0].regName - _dev->memUnitBaseAddrInfo.lmu[0])/* relative to LMU0 */ \
         + _dev->memUnitBaseAddrInfo.lmu[_lmu]) : \
    /* use Raven and LMU[0..max]*/                \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(_dev)->LMU[((_raven)%2) * 2 + _lmu].regName)

#define SMEM_SIP6_LMU_AVERAGE_COEFFICIENT_REG(_dev,_raven,_lmu) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,averageCoefficient)

#define SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(_dev,_raven,_lmu,_num) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,profileStatisticsReadData[_num])

#define SMEM_SIP6_LMU_SUMMARY_INTR_CAUSE_REG(_dev,_raven,_lmu) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,summary_cause)

#define SMEM_SIP6_LMU_SUMMARY_INTR_MASK_REG(_dev,_raven,_lmu) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,summary_mask)

#define SMEM_SIP6_LMU_CHANNEL_ENABLE_REG(_dev,_raven,_lmu) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,channelEnable)

#define SMEM_SIP6_LMU_LATENCY_OVER_THRESHOLD_INTR_CAUSE_REG(_dev,_raven,_lmu,_num) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,latencyOverThreshold_cause[_num])

#define SMEM_SIP6_LMU_LATENCY_OVER_THRESHOLD_INTR_MASK_REG(_dev,_raven,_lmu,_num) \
    __SMEM_SIP6_LMU_REG(_dev,_raven,_lmu,latencyOverThreshold_mask[_num])

/* TCAM - Exact Match Profiles Table : Port maping , pipeNem  is relative to the tile (0..1) */
#define SMEM_FALCON_EXACT_MATCH_PORT_MAPPING_REG(dev, pipeNum) \
    SIP_6_REG(dev,TCAM.exactMatchProfilesTable.exactMatchPortMapping[pipeNum])

/* TCAM - Exact Match Profiles Table : Profile Table %n Control , entryIndex (1..15) */
#define SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_CONTROL_REG(dev, entryIndex) \
    SIP_6_REG(dev,TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[entryIndex])

/* TCAM - Exact Match Profiles Table : Profile Table %n Mask Data %i , entryIndex (1..15) wordNum (0..11) */
#define SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_MASK_DATA_REG(dev, entryIndex,wordNum) \
    SIP_6_REG(dev,TCAM.exactMatchProfilesTable.exactMatchProfileTableMaskData[entryIndex][wordNum])

/* TCAM - Exact Match Profiles Table : Profile Table %n Default Action Data %i , entryIndex (1..15) wordNum (0..7) */
#define SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_DEFAULT_ACTION_DATA_REG(dev, entryIndex,wordNum) \
    SIP_6_REG(dev,TCAM.exactMatchProfilesTable.exactMatchProfileTableDefaultActionData[entryIndex][wordNum])

/* TCAM - Exact Match Action Assignment Type %n Byte %i , entryIndex (0..15) byte (0..30) for sip6, (0...31) for sip6_10 */
#define SMEM_FALCON_EXACT_MATCH_ACTION_ASSIGNMENT_REG(dev, entryIndex,byte) \
    SIP_6_REG(dev,TCAM.exactMatchActionAssignment[entryIndex][byte])

/* EM : get pointer to the memory of the EXACT_MATCH entry .
    the EXACT_MATCH memory is NOT in the 'CIDER' address space of the device.
    meaning it is unreachable by direct read/write from the CPU.
    (it was also in sip5 devices , but there we had enough 'fake' address space
    to support it as regular memory with fake 'CIDER' address space)

   NOTE: it replace the use of SMEM_CHT_MAC_TBL_MEM that return 'address'
*/
#define SMEM_SIP6_HIDDEN_EXACT_MATCH_PTR(dev, entry_index) \
    smemGenericHiddenFindMem(dev,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E,entry_index,NULL)

/* EM - EM Global Configuration , regName values (emGlobalConfiguration1 ,emGlobalConfiguration2
                                  emCRCHashInitConfiguration )
*/
#define SMEM_FALCON_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(dev,regName) \
    SIP_6_REG(dev,EXACT_MATCH.emGlobalConfiguration.regName)

/* EREP - QCN Outgoing Counter Register */
#define SMEM_SIP6_EREP_QCN_CNTR_REG(dev)        \
        SIP_6_REG(dev,EREP.counters.qcnOutgoingCounter)

/* EREP - SNIFF Outgoing Counter Register */
#define SMEM_SIP6_EREP_SNIFF_CNTR_REG(dev)        \
        SIP_6_REG(dev,EREP.counters.sniffOutgoingCounter)

/* EREP - TRAP Outgoing Counter Register */
#define SMEM_SIP6_EREP_TRAP_CNTR_REG(dev)        \
        SIP_6_REG(dev,EREP.counters.trapOutgoingCounter)

/* EREP - MIRROR  Outgoing Counter Register */
#define SMEM_SIP6_EREP_MIRROR_CNTR_REG(dev)        \
        SIP_6_REG(dev,EREP.counters.mirrorOutgoingCounter)

/* EREP - HBU Packets Outgoing Forward Counter Register */
#define SMEM_SIP6_EREP_HBU_PACKET_FORWARD_CNTR_REG(dev)        \
        SIP_6_REG(dev,EREP.counters.hbuPacketsOutgoingForwardCounter)

/* EREP - Egress Statistic Mirroring to Analyzer <%i> Port Ratio Configuration */
#define SMEM_SIP6_EREP_EGRESS_ANALYZER_CNTR_REG(dev,analyzerIndex) \
    SIP_6_REG(dev,EREP.counters.egressAnalyzerCounter[analyzerIndex])

/* PHA - control Register */
#define SMEM_SIP6_PHA_CTRL_REG(dev) \
    SIP_6_REG(dev,PHA.pha_regs.generalRegs.PHACtrl)

/* PHA - PPA Threads Conf1 */
#define SMEM_SIP6_PHA_PPA_THREADS_CONF1_MEM(dev, pha_threadId) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, PHA_PPAThreadsConf1, pha_threadId)

/* PHA - PPA Threads Conf2 */
#define SMEM_SIP6_PHA_PPA_THREADS_CONF2_MEM(dev, pha_threadId) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, PHA_PPAThreadsConf2, pha_threadId)

/* PHA - Target Port Data (physical port) */
#define SMEM_SIP6_PHA_TARGET_PORT_DATA_MEM(dev, trgPort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, PHA_targetPortData, trgPort)

/* PHA - Source Port Data (physical port) */
#define SMEM_SIP6_PHA_SRC_PORT_DATA_MEM(dev, srcPort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, PHA_sourcePortData, srcPort)

/* PHA - PPN Packet header access offset register */
#define SMEM_SIP6_PHA_PPN_PKT_HEADER_ACCESS_OFFSET_REG(dev) \
    SIP_6_REG(dev,PHA.PPG[0].PPN[0].ppn_regs.pktHeaderAccessOffset)

/* EQ Probe Packet ingress drop code register */
#define SMEM_SIP6_EQ_INGRESS_PROBE_PACKET_DROP_CODE_CFG_REG(dev) \
    SIP_6_REG(dev,EQ.ingrDropCodeLatching.ingrDropCodeLatching)

#define SMEM_SIP6_EQ_INGRESS_PROBE_PACKET_DROP_CODE_REG(dev) \
    SIP_6_REG(dev,EQ.ingrDropCodeLatching.lastDropCode)

/* EREP Probe Packet egress drop code register */
#define SMEM_SIP6_EREP_EGRESS_PROBE_PACKET_DROP_CODE_CFG_REG(dev) \
    SIP_6_REG(dev,EREP.configurations.dropCodeConfigs)

#define SMEM_SIP6_EREP_EGRESS_PROBE_PACKET_DROP_CODE_REG(dev) \
    SIP_6_REG(dev,EREP.configurations.dropCodeStatus)

#define SMEM_SIP6_EREP_GLB_CFG_REG(dev) \
    SIP_6_REG(dev,EREP.configurations.erepGlobalConfigs)

#define SMEM_SIP6_EREP_EGRESS_STATISTIC_MIRRORING_TO_ANALYZER_PORT_RATIO_REG(dev, analyzerIndex) \
    SIP_6_REG(dev,EREP.configurations.egressStatisticalMirroringToAnalyzerRatio[analyzerIndex])

/* Packet Type Hash Mode Table */
#define SMEM_SIP6_PCL_HASH_MODE_TBL_MEM(dev, entry_index)\
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, crcHashMode, entry_index)

/* IPCL packet Type CRC Hash Mode register */
#define SMEM_SIP6_PCL_HASH_MODE_REG(dev) \
        (SMEM_CHT_IS_SIP6_GET(dev)? SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.CRCHashModes : \
            SMAIN_NOT_VALID_CNS)

/* EQ -  DLB global ePort config */
#define SMEM_SIP6_EQ_GLOBAL_E_PORT_DLB_CONFIG_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.globalEportDLBConfig)

/* EQ - DLB ePort Minimum */
#define SMEM_SIP6_EQ_DLB_E_PORT_MIN_VALUE_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.dlbEportMinValue)

/* EQ - DLB ePort Maximum */
#define SMEM_SIP6_EQ_DLB_E_PORT_MAX_VALUE_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.dlbEportMaxValue)

/* EQ - DLB LTT Index Base ePort */
#define SMEM_SIP6_EQ_DLB_LTT_INDEX_BASE_E_PORT_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.dlbLTTIndexBaseEport)

/* EQ - DLB LTT Index Base ePort */
#define SMEM_SIP6_EQ_DLB_MINIMUM_FLOWLET_IPG_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.dlbMinimumFlowletIpg)

/* EQ - DLB Path Utilization Table Entry */
#define SMEM_SIP6_EQ_L2_DLB_PATH_UTILIZATION_TBL_MEM(dev, _tableId, index) \
    (((_tableId) ==0 )?SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pathUtilization0, index): \
     ((_tableId) ==1 )?SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pathUtilization1, index): \
     ((_tableId) ==2 )?SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pathUtilization2, index): \
     ((_tableId) ==3 )?SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pathUtilization3, index): \
     SMAIN_NOT_VALID_CNS)

/* EQ - Pre-Egress Engine Global Configuration 2 */
#define SMEM_SIP6_EQ_PRE_EGR_GLB_CONF_2_REG(dev) \
    SIP_6_REG(dev,EQ.preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2)

/* EQ - Pre-Egress Engine scalable SGT support */
#define SMEM_SIP6_10_EQ_PRE_EGR_SGT_SUP_REG(dev) \
    SIP_6_REG(dev,EQ.preEgrEngineGlobalConfig.scalableSgtSupport)

/* EQ - Coarse Grain Detector Hash Bit Selection Configuration */
#define SMEM_SIP6_EQ_COARSE_GRAIN_DETECTOR_HASH_BIT_SELECT_CONFIG_REG(dev) \
    SIP_6_REG(dev,EQ.L2ECMP.coarseGrainDetectorHashBitSelectionConfig)

/* D2D eagle side - PCS global control register */
#define SMEM_SIP6_D2D_EAGLE_PCS_GLOBAL_CONTROL_REG(dev,port) \
    SIP_6_REG(dev,eagle_d2d[SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(dev,port)].d2d.pcs.globalControl)

/* D2D Raven PMA Operation Control */
#define SMEM_SIP6_D2D_RAVEN_PMA_OPERATION_CONTROL_INDEX_MAC(dev,port) \
     SIP_6_REG(dev,raven_d2d_pair[smemRavenGlobalPortToRavenIndexGet(dev,port)].d2d[smemRavenGlobalPortToChannelGroupGet(dev, port)].pma.pmaOperationControl)

/* L2I Bridge Global Configuration Register3 */
#define SMEM_SIP6_L2I_BRDG_GLB_CONF3_REG(dev)                                 \
    SIP_6_REG(dev,L2I.bridgeEngineConfig.bridgeGlobalConfig3)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregFalconh */


