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
* @file sregHawk.h
*
* @brief Defines for Hawk memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregHawkh
#define __sregHawkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>

/* PPU - Global config */
#define SMEM_SIP6_10_PPU_GLOBAL_CONFIG_REG(dev) \
    SIP_6_10_REG(dev,PPU.generalRegs.ppuGlobalConfig)

/* TTI - PPU Profile */
#define SMEM_SIP6_10_TTI_PPU_PROFILE_REG(dev, profileIdx) \
    SIP_6_10_REG(dev,TTI.ppuProfile[profileIdx])

/* TTI - PPU Profile */
#define SMEM_SIP6_10_TTI_TUNNEL_HEADER_LENGTH_PROFILE_REG(dev, profileIdx) \
    SIP_6_10_REG(dev,TTI.tunnelHeaderLengthProfile[profileIdx])

/*TTI -  Em ProfileId 1 Mapping Table */
#define SMEM_SIP6_10_TTI_EM_PROFILE_ID_1_MAPPING_TBL_MEM(dev, entry_index)             \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ttiEmProfileId1Mapping, entry_index)
/*TTI -  Em ProfileId 2 Mapping Table */
#define SMEM_SIP6_10_TTI_EM_PROFILE_ID_2_MAPPING_TBL_MEM(dev, entry_index)             \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ttiEmProfileId2Mapping, entry_index)

/* PPU - SP Bus default profile word */
#define SMEM_SIP6_10_PPU_SP_BUS_DEFAULT_PROFILE_WORD_REG(dev, profileIdx, wordIdx) \
    SIP_6_10_REG(dev,PPU.spBusDefaultProfile[profileIdx].word[wordIdx])

/* PPU - K_stg key generation profile byte field */
#define SMEM_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_REG(dev, kstgNum, profileIdx, byteIdx) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].keyGenProfile[profileIdx].byte[byteIdx])

/* PPU - Tcam key LS Bytes */
#define SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_LSB_REG(dev, kstgNum, tcamIndex) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].tcamKey[tcamIndex].keyLsb)

/* PPU - Tcam key MS Bytes */
#define SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MSB_REG(dev, kstgNum, tcamIndex) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].tcamKey[tcamIndex].keyMsb)

/* PPU - Tcam key mask LS Bytes */
#define SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MASK_LSB_REG(dev, kstgNum, tcamIndex) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].tcamKey[tcamIndex].maskLsb)

/* PPU - Tcam key mask MS Bytes */
#define SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MASK_MSB_REG(dev, kstgNum, tcamIndex) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].tcamKey[tcamIndex].maskMsb)

/* PPU - Tcam key valid */
#define SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_VALID_REG(dev, kstgNum, tcamIndex) \
    SIP_6_10_REG(dev,PPU.kstg[kstgNum].tcamKey[tcamIndex].validate)

/* PPU - Action table 0 */
#define SMEM_SIP6_10_PPU_ACTION_TBL_0_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ppuActionTable0, index)

/* PPU - Action table 1 */
#define SMEM_SIP6_10_PPU_ACTION_TBL_1_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ppuActionTable1, index)

/* PPU - Action table 2 */
#define SMEM_SIP6_10_PPU_ACTION_TBL_2_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ppuActionTable2, index)

/* PPU - DAU Profile Table */
#define SMEM_SIP6_10_PPU_DAU_PROFILE_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ppuDauProfileTable, index)

/* PPU - DAU Protected Window */
#define SMEM_SIP6_10_PPU_DAU_PROTECTED_WINDOW_REG(dev, index) \
    SIP_6_10_REG(dev,PPU.dau.dauProtWin[index])

/* PPU - SMEM_SIP6_10_PPU_INT_ */
#define SMEM_SIP6_10_PPU_INT_CAUSE_REG(dev) \
    SIP_6_10_REG(dev,PPU.generalRegs.ppuInterruptCause)

/* PPU - Interrupt counter */
#define SMEM_SIP6_10_PPU_INT_COUNTER_REG(dev, index) \
    SIP_6_10_REG(dev,PPU.interruptCounter[index])

/* PPU - Error profile */
#define SMEM_SIP6_10_PPU_ERROR_PROFILE_REG(dev, index, fld) \
    SIP_6_10_REG(dev,PPU.errProfile[index].field[fld])

/* EQ - Hash Bit Selection Profile [16] */
#define SMEM_SIP6_10_EQ_L2ECMP_HASH_BIT_SELECTION_REG(dev,index) \
    SIP_6_10_REG(dev,EQ.L2ECMP.L2ECMP_Hash_Bit_Selection[index])

/* EQ - L2ECMP config  */
#define SMEM_SIP6_10_EQ_L2ECMP_CONFIG_REG(dev) \
    SIP_6_10_REG(dev,EQ.L2ECMP.L2ECMP_config)

/* EQ - source port hash Table Entry */
#define SMEM_SIP6_10_EQ_SOURCE_PORT_HASH_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, sourcePortHash, index)

/*EPCL -  Queue Group Latency Profile Configuration Table */
#define SMEM_SIP6_10_EPCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_TBL_MEM(dev, entry_index)             \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, queueGroupLatencyProfileConfigTable, entry_index)

/* EPCL - Queue Offset Latency Profile Configuration */
#define SMEM_SIP6_10_EPCL_QUEUE_OFFSET_LATENCY_PROFILE_CONFIG_REG(dev, index) \
    SIP_6_10_REG(dev,EPCL.queueOffsetLatencyProfileConfig[index])

/* Ingress/Egress Policer IPFIX First N Packets Configuration0 */
#define SMEM_SIP6_10_POLICER_IPFIX_FIRST_N_PACKETS_CONFIG0_REG(dev, index) \
    SIP_6_10_REG(dev,PLR[index].IPFIXFirstNPacketsConfig0)

/* Ingress/Egress Policer IPFIX First N Packets Configuration1 */
#define SMEM_SIP6_10_POLICER_IPFIX_FIRST_N_PACKETS_CONFIG1_REG(dev, index) \
    SIP_6_10_REG(dev,PLR[index].IPFIXFirstNPacketsConfig1)

/* Ingress/Egress IPFIX First N packets table */
#define SMEM_SIP6_10_POLICER_IPFIX_FIRST_N_PACKETS_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerIpfix1StNPackets, index, cycle)

/* TXQ-PDX - Preemption Enable' */
#define SMEM_SIP6_TXQ_SDQ_PREEMPTION_ENABLE_REG(dev,index) \
    SIP_6_REG(dev,SIP6_TXQ_PDX.PreemptionEnable[index])

/* Ingress TPID Enhanced classification */
#define SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_REG(dev, index)       \
    SIP_6_10_REG(dev, TTI.VLANAssignment.ingrTPIDEnhClasification[index])

/* Ingress TPID Enhanced classification Ext */
#define SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT_REG(dev, index)       \
    SIP_6_10_REG(dev, TTI.VLANAssignment.ingrTPIDEnhClasificationExt[index])

/* Ingress TPID Enhanced classification Ext2 */
#define SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT2_REG(dev, index)       \
    SIP_6_10_REG(dev, TTI.VLANAssignment.ingrTPIDEnhClasificationExt2[index])

/* Ingress TPID Enhanced classification Ext3 */
#define SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT3_REG(dev, index)       \
    SIP_6_10_REG(dev, TTI.VLANAssignment.ingrTPIDEnhClasificationExt3[index])


/* Exact Match profile configuration */
#define SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_1_REG(dev, profileIndex) \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emProfileConfiguration1[profileIndex])

#define SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_2_REG(dev, profileIndex) \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emProfileConfiguration2[profileIndex])

    #define SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_3_REG(dev, profileIndex) \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emProfileConfiguration3[profileIndex])

#define SMEM_SIP6_10_EXACT_MATCH_AUTO_LEARN_PROFILE_CONFIG_4_REG(dev, profileIndex) \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emProfileConfiguration4[profileIndex])

/* EM Flow ID configuration */
#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_1_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdAllocationConfiguration1)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_CONFIG_2_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdAllocationConfiguration2)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_1_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus1)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_2_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus2)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_ALLOCATION_STATUS_3_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus3)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_CAUSE_REGISTER_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emInterrupt.emInterruptCauseReg)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_INTERRUPT_MASK_REGISTER_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emInterrupt.emInterruptMaskReg)

#define SMEM_SIP6_10_EXACT_MATCH_FLOW_ID_FAIL_COUNTER_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emFlowIdFailCounter)

#define SMEM_SIP6_10_EXACT_MATCH_INDEX_FAIL_COUNTER_REG(dev) \
     SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emIndexFailCounter)

/* Auto Learned EM Entry Index Table */
#define SMEM_SIP6_10_AUTO_LEARN_EXACT_MATCH_ENTRY_TBL_MEM(dev, index)   \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, exactMatchAutoLearnedEntryIndexTable, index)

/* Auto Learned Collision Status */
#define SMEM_SIP6_10_AUTO_LEARN_COLLISION_STATUS_1_REGISTER_REG(dev)   \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emAutoLearningCollisionStatus1)

#define SMEM_SIP6_10_AUTO_LEARN_COLLISION_STATUS_N_REGISTER_REG(dev,index)   \
    SIP_6_10_REG(dev, EXACT_MATCH.emAutoLearning.emAutoLearningCollisionStatus_n[index])

/* EGF QAG -VLAN Q offset Table */
#define SMEM_SIP6_20_EGF_QAG_VLAN_Q_OFFSET_TBL_MEM(dev, vlanId) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfQagVlanQOffsetMappingTable, vlanId)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregHawkh */
