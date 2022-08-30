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
* @file sregIronman.h
*
* @brief Defines for Ironman memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregIronmanh
#define __sregIronmanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>

/* IPCL - IPCL UDB Replacement Entry Table (separated from the IPCL UDB Select table)*/
/* used where SMEM_LION3_PCL_UDB_SELECT_TBL_MEM(...) is used */
#define SMEM_SIP6_30_IPCL_UDB_REPLACEMENT_TBL_MEM(dev, entry_index , _cycle)            \
    ((_cycle == 0) ? SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipcl0UdbReplacement, entry_index) : \
     (_cycle == 1) ? SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipcl1UdbReplacement, entry_index) : \
     (_cycle == 2) ? SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipcl2UdbReplacement, entry_index) : \
        SMAIN_NOT_VALID_CNS)

/* IPCL - IPCL Unit MAX SDU Size Profile */
#define SMEM_SIP6_30_IPCL_MAX_SDU_SIZE_PROFILE_REG(dev, profileNum)  \
        SIP_6_REG(dev, PCL.maxSduSizeProfile[profileNum])

/* IPCL - IPCL Unit MAX SDU Exception */
#define SMEM_SIP6_30_IPCL_MAX_SDU_EXCEPTION_REG(dev)           \
        SIP_6_REG(dev, PCL.maxSduException)

/* EPCL - EPCL Unit Egress MAX SDU Size Profile */
#define SMEM_SIP6_30_EPCL_MAX_SDU_SIZE_PROFILE_REG(dev, profileNum)  \
        SIP_6_REG(dev, EPCL.egressMaxSduSizeProfile[profileNum])

/* EPCL - EPCL Unit Egress MAX SDU Exception */
#define SMEM_SIP6_30_EPCL_MAX_SDU_EXCEPTION_REG(dev)           \
        SIP_6_REG(dev, EPCL.egressMaxSduException)

#define SMEM_SIP6_30_IPCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_REG(dev, _cycle)           \
        SIP_6_REG(dev, PCL.flowIdActionAssignmentModeConfig[_cycle])

#define SMEM_SIP6_30_IPCL_POLICY_GENERIC_ACTION_REG(dev, _cycle)           \
        SIP_6_REG(dev, PCL.policyGenericActionConfig[_cycle])

#define SMEM_SIP6_30_IPCL_POLICY_GLOBAL_CONFIG_REG(dev, _cycle)           \
        SIP_6_REG(dev, PCL.policyGlobalConfig[_cycle])

#define SMEM_SIP6_30_IPCL_GLOBAL_CONFIG_EXT_1_REG(dev)           \
        SIP_6_REG(dev, PCL.globalConfig_ext_1)

/* EPCL - EPCL UDB Replacement Entry Table (separated from the EPCL UDB Select table)*/
/* used where SMEM_LION3_EPCL_UDB_SELECT_TBL_MEM(...) is used */
#define SMEM_SIP6_30_EPCL_UDB_REPLACEMENT_TBL_MEM(dev)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, epclUdbReplacement, 0)

/* TTI - TTI Unit hsr Prp Global Configurations 0 */
#define SMEM_SIP6_30_TTI_UNIT_HSR_PRP_GLOBAL_CONFIG0_REG(dev)        \
        SIP6_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.hsrPrpGlobalConfig0)

/* TTI - TTI Unit global configurations Ext3 */
#define SMEM_SIP6_30_TTI_UNIT_GLOBAL_CONFIG_EXT3_REG(dev)        \
        SIP6_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt3)

/* L2i - L2i Unit hsr Prp Global Configurations 0 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_GLOBAL_CONFIG0_REG(dev)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpGlobalConfig0)

/* L2i - L2i Unit hsr Prp Global Configurations 1 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_GLOBAL_CONFIG1_REG(dev)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpGlobalConfig1)

/* L2i - L2i Unit hsr Prp Global Configurations 2 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_GLOBAL_CONFIG2_REG(dev)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpGlobalConfig2)

/* L2i - L2i Unit hsr Prp Global Configurations 3 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_GLOBAL_CONFIG3_REG(dev)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpGlobalConfig3)


/* L2i - L2i Unit hsr Prp Hport config0 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_HPORT_CONFIG0_REG(dev,hport)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpSourcePortConfig0[hport])

/* L2i - L2i Unit hsr Prp Hport config1 */
#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_HPORT_CONFIG1_REG(dev,hport)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpSourcePortConfig1[hport])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_READY_REG(dev,pntIndex)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntReady[pntIndex])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_VALID_REG(dev,pntIndex)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntValid[pntIndex])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_ENTRY_PART0_REG(dev,pntIndex)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntEntryPart0[pntIndex])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_ENTRY_PART1_REG(dev,pntIndex)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntEntryPart1[pntIndex])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_ENTRY_PART2_REG(dev,pntIndex)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntEntryPart2[pntIndex])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_LRE_A_COUNTER_REG(dev,lreInstance)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntDiscardPacketsLreACounter[lreInstance])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_LRE_B_COUNTER_REG(dev,lreInstance)        \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntDiscardPacketsLreBCounter[lreInstance])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_LRE_INSTANCE_TABLE_REG(dev,lreInstance)   \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpLreInstanceTable[lreInstance])

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_READY_COUNTER_REG(dev)   \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntCounter)

#define SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_READY_COUNTER_THRESHOLD_REG(dev)   \
        SIP_6_REG(dev, L2I.hsrPrp.hsrPrpPntCounterThreshold)


#define SMEM_SIP6_30_FDB_UNIT_HSR_PRP_GLOBAL_CONFIG_REG(dev)   \
        SIP_6_REG(dev, FDB.FDBCore.hsrPrp.hsrPrpGlobalConfig)

#define SMEM_SIP6_30_FDB_UNIT_HSR_PRP_AGING_REG(dev)   \
        SIP_6_REG(dev, FDB.FDBCore.hsrPrp.hsrPrpAging)

#define SMEM_SIP6_30_FDB_UNIT_HSR_PRP_COUNTER_FIRST_REG(dev,hport)   \
        SIP_6_REG(dev, FDB.FDBCore.hsrPrp.hsrPrpCountersFirst  [hport])

#define SMEM_SIP6_30_FDB_UNIT_HSR_PRP_COUNTER_SECOND_REG(dev,hport)   \
        SIP_6_REG(dev, FDB.FDBCore.hsrPrp.hsrPrpCountersSecond [hport])

#define SMEM_SIP6_30_FDB_UNIT_HSR_PRP_COUNTER_THIRD_REG(dev,hport)   \
        SIP_6_REG(dev, FDB.FDBCore.hsrPrp.hsrPrpCountersThird  [hport])

#define SMEM_SIP6_30_HA_EGRESS_TPID_CONFIG_EXT1_REG(dev,tpidIndex)      \
        SIP_6_REG(dev, HA.egrTPIDEnhClasificationExt1[tpidIndex])

#define SMEM_SIP6_30_HA_EGRESS_TPID_CONFIG_EXT2_REG(dev,tpidIndex)      \
        SIP_6_REG(dev, HA.egrTPIDEnhClasificationExt2[tpidIndex])

#define SMEM_SIP6_30_SMU_IRF_SNG_GLOBAL_CONFIG_REG(dev)      \
        SIP_6_REG(dev, SMU.irfSng.irfSngGlobalConfig)

#define SMEM_SIP6_30_SMU_IRF_SNG_IRF_AGE_BIT_REG(dev,_index)      \
        SIP_6_REG(dev, SMU.irfSng.irfAgeBit[_index])

#define SMEM_SIP6_30_SMU_SNG_IRF_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, smuSngIrf, index)

#define SMEM_SIP6_30_SMU_IRF_COUNTERS_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, smuIrfCounters, index)

#define SMEM_SIP6_30_EREP_PRP_GLOBAL_CONFIG_REG(dev) \
    SIP_6_REG(dev,EREP.configurations.PRP_Global_Configuration)

#define SMEM_SIP6_30_EPCL_GLOBAL_CONFIG_EXT_1_REG(dev)           \
        SIP_6_REG(dev, EPCL.egressPolicyGlobalConfig_ext_1)

#define SMEM_SIP6_30_EPCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_REG(dev)           \
        SIP_6_REG(dev, EPCL.egressPolicyFlowIdActionAssignmentModeConfig)

#define SMEM_SIP6_30_PREQ_FRE_GLOBAL_CONFIG_REG(dev)           \
        SIP_6_REG(dev, PREQ.FRE.freGlobalConfig)

#define SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_0_REG(dev)           \
        SIP_6_REG(dev, PREQ.FRE.freSrfGlobalConfig0)

#define SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_1_REG(dev)           \
        SIP_6_REG(dev, PREQ.FRE.freSrfGlobalConfig1)

#define SMEM_SIP6_30_PREQ_SRF_MAPPING_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqSrfMapping, index)

#define SMEM_SIP6_30_PREQ_SRF_CONFIG_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqSrfConfig, index)

#define SMEM_SIP6_30_PREQ_DAEMONS_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqDaemons, index)

#define SMEM_SIP6_30_PREQ_HISTROY_BUFFER_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqHistoryBuffer, index)

#define SMEM_SIP6_30_PREQ_SRC_COUNTERS_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqSrfCounters, index)

#define SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_0_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqZeroBitVector0, index)

#define SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_1_TBL_MEM(dev,index)              \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, preqZeroBitVector1, index)

#define SMEM_SIP6_30_PREQ_FRE_SRF_TAKE_ANY_REG(dev,index)           \
        SIP_6_REG(dev, PREQ.FRE.TakeAny[index])

#define SMEM_SIP6_30_PREQ_FRE_SRF_RESTART_PERIOD_REG(dev)           \
        SIP_6_REG(dev, PREQ.FRE.RestartPeriod)

#define SMEM_SIP6_30_PREQ_FRE_SRF_LATENT_ERROR_PERIOD_REG(dev)           \
        SIP_6_REG(dev, PREQ.FRE.LatentErrorPeriod)

#define SMEM_SIP6_30_PREQ_FRE_SRF_LATENT_ERROR_DETECTED_REG(dev,index)           \
        SIP_6_REG(dev, PREQ.FRE.ErrorDetected[index])


#define SMEM_SIP6_30_PREQ_INTERRUPT_CAUSE_REG(dev)           \
        SIP_6_REG(dev, PREQ.preqInterrupts.preqInterruptCause)

#define SMEM_SIP6_30_PREQ_INTERRUPT_MASK_REG(dev)           \
        SIP_6_REG(dev, PREQ.preqInterrupts.preqInterruptMask)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregIronmanh */

