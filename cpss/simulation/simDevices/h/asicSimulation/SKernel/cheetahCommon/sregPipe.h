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
* @file sregPipe.h
*
* @brief Defines for PIPE memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregPipeh
#define __sregPipeh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*SMEM_PIPE_PP_REGS_ADDR_STC*/

/*interrupt bit index in PCP cause register */
#define PCP_Packet_Type_Key_Look_Up_Miss 2

/* Transmit SDMA Queue Command Register */
#define SMEM_PIPE_TX_SDMA_QUE_CMD_REG(dev)                                \
    SMEM_CHT_MAC_REG_DB_GET(dev)->MG.SDMARegs.transmitSDMAQueueCommand

#define SMEM_PIPE_PCP_PORT_TPID_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPTPIDTable[index]

#define SMEM_PIPE_PCP_PORT_DA_OFFSET_CONFIG_REG(dev,srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPortDaOffsetConfig[srcPort]

#define SMEM_PIPE_PCP_ECN_ENABLE_CONFIG_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPECNEnableConfig

#define SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_0_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPGlobalEtherTypeConfig0

#define SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_1_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPGlobalEtherTypeConfig1

#define SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_2_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPGlobalEtherTypeConfig2

#define SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_3_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPGlobalEtherTypeConfig3

#define SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_4_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPGlobalEtherTypeConfig4

#define SMEM_PIPE_PCP_PORT_PACKET_TYPE_KEY_TABLE_REG(dev,srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPortPktTypeKeyTable[srcPort]

#define SMEM_PIPE_PCP_PORT_PACKET_TYPE_KEY_TABLE_EXT_REG(dev,srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPortPktTypeKeyTableExt[srcPort]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_ENTRIES_ENABLE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableEntriesEnable

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_4MSB_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_4MSB_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_2LSB_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_2LSB_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_TPID_ETHER_TYPE_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_TPID_ETHER_TYPE_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IS_LLC_NON_SNAP_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IS_LLC_NON_SNAP_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_SRC_PORT_PROFILE_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_SRC_PORT_PROFILE_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_UDBP_ENTRY_REG(dev,index,udbIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableUDBPEntry[index][udbIndex]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_UDBP_ENTRY_MASK_REG(dev,index,udbIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTableUDBPEntryMask[index][udbIndex]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IP2ME_IDX_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntry[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IP2ME_IDX_ENTRY_MASK_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[index]

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyLookUpMissDetected

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_COUNTER_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyLookUpMissCntr

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_MACDA_4MSB_MISS_VALUE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyMACDA4MSBMissValue

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_MACDA_2LSB_MISS_VALUE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyMACDA2LSBMissValue

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_TPID_ETHERTYPE_MISS_VALUE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyTPIDEtherTypeMissValue

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_IS_LLC_NON_SNAP_MISS_VALUE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyIsLLCNonSnapMissValue

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_SRC_PORT_PROFILE_MISS_VALUE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeySRCPortProfileMissValue

#define SMEM_PIPE_PCP_PACKET_TYPE_KEY_UDBP_MISS_VALUE_REG(dev,udbIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PRS.PCPPktTypeKeyUDBPMissValue[udbIndex]

#define SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT0_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypeDestinationFormat0Entry[index]

#define SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT1_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypeDestinationFormat1Entry[index]

#define SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT2_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypeDestinationFormat2Entry[index]

#define SMEM_PIPE_PCP_DST_EXCEPTION_FORWARDING_PORT_MAP_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPDSTExceptionForwardingPortMap

#define SMEM_PIPE_PCP_DST_IDX_EXCEPTION_CNTR_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPDstIdxExceptionCntr

#define SMEM_PIPE_PCP_DST_PORT_MAPTABLE_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pipe_PCP_dstPortMapTable, index)

#define SMEM_PIPE_PCP_PORT_FILTER_TABLE_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pipe_PCP_portFilterTable, index)


#define SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT0_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypePortFilterFormat0Entry[index]

#define SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT1_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypePortFilterFormat1Entry[index]

#define SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT2_ENTRY_REG(dev,index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPktTypePortFilterFormat2Entry[index]

#define SMEM_PIPE_PCP_PORT_FILTER_EXCEPTION_FORWARDING_PORT_MAP_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPortFilterExceptionForwardingPortMap

#define SMEM_PIPE_PCP_PORT_FILTER_IDX_EXCEPTION_CNTR_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPortFilterIdxExceptionCntr

#define SMEM_PIPE_PCP_HASH_PACKET_TYPE_UDE1_ETHER_TYPE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPHashPktTypeUDE1EtherType

#define SMEM_PIPE_PCP_HASH_PACKET_TYPE_UDE2_ETHER_TYPE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPHashPktTypeUDE2EtherType

#define SMEM_PIPE_PCP_PACKET_TYPE_HASH_CONFIG_UDBP_REG(dev,_hashType,_udbpIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPktTypeHashConfigUDBPEntry[_udbpIndex][_hashType]

#define SMEM_PIPE_PCP_PACKET_TYPE_HASH_CONFIG_INGRESS_PORT_REG(dev,_hashType) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPktTypeHashConfigIngrPort[_hashType]

#define SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_0_ENTRY_REG(dev,_hashType) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPktTypeHashMode0Entry[_hashType]

#define SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_1_ENTRY_REG(dev,_hashType) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPktTypeHashMode1Entry[_hashType]

#define SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_2_ENTRY_REG(dev,_hashType) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPktTypeHashMode2Entry[_hashType]

#define SMEM_PIPE_PCP_HASH_CRC32_SEED_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPHashCRC32Seed

#define SMEM_PIPE_PCP_PACKET_TYPE_LAG_TABLE_NUMBER_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPPacketTypeLagTableNumber

#define SMEM_PIPE_PCP_LAG_DESIGNATED_PORT_ENTRY_REG(dev,_lagIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPLagDesignatedPortEntry[_lagIndex]

#define SMEM_PIPE_PCP_LAG_INDEX_MODE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.HASH.PCPLagTableIndexMode

#define SMEM_PIPE_PCP_PORTS_ENABLE_CONFIG_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPortsEnableConfig

#define SMEM_PIPE_PCP_ENABLE_PACKET_TYPE_EGRESS_FILTERING_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPacketTypeEnableEgressPortFiltering

#define SMEM_PIPE_PCP_ENABLE_PACKET_TYPE_INGRESS_FILTERING_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PORTMAP.PCPPacketTypeEnableIngressPortFiltering

#define SMEM_PIPE_PCP_COS_FORMAT_TABLE_ENTRY_REG(dev,_index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPCoSFormatTableEntry[_index]

#define SMEM_PIPE_PCP_PORT_COS_ATTRIBUTES_REG(dev,_srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPPortCoSAttributes[_srcPort]

#define SMEM_PIPE_PCP_PORT_L2_COS_MAPPING_ENTRY_REG(dev,_srcPort,_cosIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPPortL2CoSMapEntry[_srcPort][_cosIndex]

#define SMEM_PIPE_PCP_DSA_COS_MAPPING_ENTRY_REG(dev,_cosIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPDsaCoSMapEntry[_cosIndex]

#define SMEM_PIPE_PCP_L3_COS_MAPPING_ENTRY_REG(dev,_cosIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPL3CoSMapEntry[_cosIndex]

#define SMEM_PIPE_PCP_MPLS_COS_MAPPING_ENTRY_REG(dev,_cosIndex) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.COS.PCPMPLSCoSMapEntry[_cosIndex]

#define SMEM_PIPE_PCP_INTERRUPTS_CAUSE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.general.PCPInterruptsCause

#define SMEM_PIPE_PCP_INTERRUPTS_MASK_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.general.PCPInterruptsMask

#define SMEM_PIPE_PCP_GLOBAL_CONFIG_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.general.PCPGlobalConfiguration

#define SMEM_PIPE_PHA_HA_TABLE_MEM(dev, trgPort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pipe_PHA_haTable, trgPort)

#define SMEM_PIPE_PHA_TARGET_PORT_DATA_MEM(dev, trgPort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pipe_PHA_targetPortData, trgPort)

#define SMEM_PIPE_PHA_SRC_PORT_DATA_MEM(dev, srcPort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, pipe_PHA_srcPortData, srcPort)

#define SMEM_PIPE_PHA_CTRL_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PHA.pha_regs.PHACtrl

#define SMEM_PIPE_PHA_GENERAL_CONFIG_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PHA.pha_regs.PHAGeneralConfig

#define SMEM_PIPE_PCP_IP2ME_CTLBITS_REG(dev,_index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.IP2ME.PCPIP2MEControlBitsEntry[_index]

#define SMEM_PIPE_PCP_IP2ME_IPADDR_REG(dev,_index,_part) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.IP2ME.PCPIP2MEIpPart[_part][_index]

/* PHA - PPN Packet header access offset register */
#define SMEM_PIPE_PHA_PPN_PKT_HEADER_ACCESS_OFFSET_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PHA.PPN[0][0].ppn_regs.pktHeaderAccessOffset

/* PTP */
#define SMEM_PIPE_PCP_PORT_PTP_TYPE_UDBP_CONFIG_REG(dev, udbp, srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPortPTPTypeKeyUDBPConfigPort[udbp][srcPort]

#define SMEM_PIPE_PCP_PORT_PTP_TYPE_UDB_CONFIG_REG(dev, udb, srcPort) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPortPTPTypeKeyUDBConfigPort[udb][srcPort]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDBP_TABLE_REG(dev, udbp, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDBPEntry[udbp][index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDBP_MASK_TABLE_REG(dev, udbp, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDBPEntryMask[udbp][index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_TABLE_REG(dev, udb, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDBEntry[udb][index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_MASK_TABLE_REG(dev, udb, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDBEntryMask[udb][index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_PORT_PROFILE_TABLE_REG(dev, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDPAndPort_profileEntry[index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_PORT_PROFILE_MASK_TABLE_REG(dev, index) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[index]

#define SMEM_PIPE_PCP_PTP_TYPE_KEY_TABLE_ENTRIES_ENABLE_REG(dev) \
    SMEM_PIPE_MAC_REG_DB_GET(dev)->PCP.PTP.PCPPTPTypeKeyTableEntriesEnable


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregPipeh */


