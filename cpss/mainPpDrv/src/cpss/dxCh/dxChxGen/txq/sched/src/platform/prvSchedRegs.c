/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief set TM register addresses - generated file
 *
* @file tm_regs.c
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/** Synced to Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.10} \TXQ_PSI \TXQ_PDQ
 */

#define SET_REG_ADDRESS(alias, hiaddr, loaddr)          alias.l[0]=loaddr;  alias.l[1]=hiaddr;



static void prvSchedInitAdressStruct_Sip_6_30
(
    GT_U32 schedulerOffset,
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr
)
{

    struct prvCpssDxChTxqSchedAddressSpace addressSpace;

    SET_REG_ADDRESS(addressSpace.Sched.PPerCtlConf_Addr, 0x00000000, 0x00000800+schedulerOffset)    /* Port Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.PPerRateShap_Addr, 0x00000000, 0x00000808+schedulerOffset)    /* Port Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.PPerRateShapInt_Addr, 0x00000000, 0x00020a10+schedulerOffset)    /* Port Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.Port_Pizza_Last_Slice, 0x00000000, 0x00000820+schedulerOffset)    /* Port Pizza Last Slice */
    SET_REG_ADDRESS(addressSpace.Sched.QPerCtlConf_Addr, 0x00000000, 0x0000b400+schedulerOffset)    /* Queue Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.QPerRateShap_Addr, 0x00000000, 0x0000b408+schedulerOffset)    /* Queue Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.QPerRateShapInt_Addr, 0x00000000, 0x00029c88+schedulerOffset)    /* Queue Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.APerCtlConf_Addr, 0x00000000, 0x00008a00+schedulerOffset)    /* A-Level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.APerRateShap_Addr, 0x00000000, 0x00008a08+schedulerOffset)    /* A Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.APerRateShapInt_Addr, 0x00000000, 0x00026940+schedulerOffset)    /* A-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.BPerCtlConf_Addr, 0x00000000, 0x00006000+schedulerOffset)    /* B-Level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.BPerRateShap_Addr, 0x00000000, 0x00006008+schedulerOffset)    /* B level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.BPerRateShapInt_Addr, 0x00000000, 0x00024900+schedulerOffset)    /* B-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.CPerCtlConf_Addr, 0x00000000, 0x00003600+schedulerOffset)    /* C-level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.CPerRateShap_Addr, 0x00000000, 0x00003608+schedulerOffset)    /* C Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.CPerRateShapInt_Addr, 0x00000000, 0x00022740+schedulerOffset)    /* C-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.Identity_Addr, 0x00000000, 0x00000028+schedulerOffset)    /* Identity */
    SET_REG_ADDRESS(addressSpace.Sched.ScrubSlots_Addr, 0x00000000, 0x00000038+schedulerOffset)    /* Scrubbing Slots Allocation */
    SET_REG_ADDRESS(addressSpace.Sched.TreeDeqEn_Addr, 0x00000000, 0x00000048+schedulerOffset)    /* Tree Dequeue Enable */
    SET_REG_ADDRESS(addressSpace.Sched.PDWRREnReg, 0x00000000, 0x00000050+schedulerOffset)    /* Tree DWRR Priority Enable for Port Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.Global_Cfg, 0x00000000, 0x0001f008+schedulerOffset)    /* Global Config */
    SET_REG_ADDRESS(addressSpace.Sched.ErrorStatus_Addr, 0x00000000, 0x00000200+schedulerOffset)    /* QM Scheduler Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.FirstExcp_Addr, 0x00000000, 0x00000208+schedulerOffset)    /* QM Scheduler First Exception */
    SET_REG_ADDRESS(addressSpace.Sched.ErrCnt_Addr, 0x00000000, 0x00000210+schedulerOffset)    /* QM Scheduler Error Counter */
    SET_REG_ADDRESS(addressSpace.Sched.ExcpCnt_Addr, 0x00000000, 0x00000218+schedulerOffset)    /* QM Scheduler Exception Counter */
    SET_REG_ADDRESS(addressSpace.Sched.ExcpMask_Addr, 0x00000000, 0x00000220+schedulerOffset)    /* QM Scheduler Exception Mask */
    SET_REG_ADDRESS(addressSpace.Sched.Plast_Addr, 0x00000000, 0x0001f010+schedulerOffset)    /* Tree Status */
    SET_REG_ADDRESS(addressSpace.Sched.PPerStatus_Addr, 0x00000000, 0x00020d10+schedulerOffset)    /* Port Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.CPerStatus_Addr, 0x00000000, 0x00022c00+schedulerOffset)    /* C Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.BPerStatus_Addr, 0x00000000, 0x00024930+schedulerOffset)    /* B Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.APerStatus_Addr, 0x00000000, 0x00026970+schedulerOffset)    /* A Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFunc_Entry, 0x00000000, 0x00000a00+schedulerOffset)    /* Port Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFuncPtr, 0x00000000, 0x00001a00+schedulerOffset)    /* Port Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketTokenEnDiv, 0x00000000, 0x00001e00+schedulerOffset)    /* Port Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketBurstSize, 0x00000000, 0x00002200+schedulerOffset)    /* Port Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.PortDWRRPrioEn, 0x00000000, 0x00002600+schedulerOffset)    /* Port DWRR Priority Enable for C-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosLo, 0x00000000, 0x00002c00+schedulerOffset)    /* Port DWRR State Or Pizza Slices */
    SET_REG_ADDRESS(addressSpace.Sched.PortRangeMap, 0x00000000, 0x00003200+schedulerOffset)    /* Port Mapping to C-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFunc, 0x00000000, 0x0000b600+schedulerOffset)    /* Queue Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFuncPtr, 0x00000000, 0x0000c000+schedulerOffset)    /* Queue Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketTokenEnDiv, 0x00000000, 0x0000e000+schedulerOffset)    /* Queue Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketBurstSize, 0x00000000, 0x00010000+schedulerOffset)    /* Queue Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.QueueQuantum, 0x00000000, 0x00012000+schedulerOffset)    /* Queue DWRR State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueAMap, 0x00000000, 0x00014000+schedulerOffset)    /* Queue Mapping to A-Level Node */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFunc_Entry, 0x00000000, 0x00008c00+schedulerOffset)    /* A-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFuncPtr, 0x00000000, 0x00009c00+schedulerOffset)    /* A-Level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, 0x00000000, 0x0000a000+schedulerOffset)    /* A-Level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketBurstSize, 0x00000000, 0x0000a400+schedulerOffset)    /* A-Level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlDWRRPrioEn, 0x00000000, 0x0000a800+schedulerOffset)    /* A-Level DWRR Priority Enable for Queue level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlQuantum, 0x00000000, 0x0000ac00+schedulerOffset)    /* A-Level DWRR State */
    SET_REG_ADDRESS(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap, 0x00000000, 0x0000b000+schedulerOffset)    /* A-Level Mapping: A-Level to B-Level Node And A-Level to Queue Range */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFunc_Entry, 0x00000000, 0x00006200+schedulerOffset)    /* B-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFuncPtr, 0x00000000, 0x00007200+schedulerOffset)    /* B-Level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, 0x00000000, 0x00007600+schedulerOffset)    /* B-Level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketBurstSize, 0x00000000, 0x00007a00+schedulerOffset)    /* B-Level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlDWRRPrioEn, 0x00000000, 0x00007e00+schedulerOffset)    /* B-Level DWRR Priority Enable for A-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlQuantum, 0x00000000, 0x00008200+schedulerOffset)    /* B-Level DWRR State */
    SET_REG_ADDRESS(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap, 0x00000000, 0x00008600+schedulerOffset)    /* B-Level Mapping: B-Level to C-Level Node and B-Level to A-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFunc_Entry, 0x00000000, 0x00003800+schedulerOffset)    /* C-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFuncPtr, 0x00000000, 0x00004800+schedulerOffset)    /* C-level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, 0x00000000, 0x00004c00+schedulerOffset)    /* C-level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketBurstSize, 0x00000000, 0x00005000+schedulerOffset)    /* C-level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlDWRRPrioEn, 0x00000000, 0x00005400+schedulerOffset)    /* C-level DWRR Priority Enable for B-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlQuantum, 0x00000000, 0x00005800+schedulerOffset)    /* C-Level DWRR State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvltoPortAndBlvlRangeMap, 0x00000000, 0x00005c00+schedulerOffset)    /* C-Level Mapping: C-Level to Port and C-Level to B-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.PortShpBucketLvls, 0x00000000, 0x00016000+schedulerOffset)    /* Port Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.PortShaperBucketNeg, 0x00000000, 0x00016400+schedulerOffset)    /* Port Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlShpBucketLvls, 0x00000000, 0x00018600+schedulerOffset)    /* C-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.CLevelShaperBucketNeg, 0x00000000, 0x00018a00+schedulerOffset)    /* C-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlShpBucketLvls, 0x00000000, 0x00019000+schedulerOffset)    /* B-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.BLevelShaperBucketNeg, 0x00000000, 0x00019400+schedulerOffset)    /* B-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlShpBucketLvls, 0x00000000, 0x00019a00+schedulerOffset)    /* A-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.ALevelShaperBucketNeg, 0x00000000, 0x00019e00+schedulerOffset)    /* A-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.QueueShpBucketLvls, 0x00000000, 0x0001b000+schedulerOffset)    /* Queue Shaper Bucket Level */
    SET_REG_ADDRESS(addressSpace.Sched.QueueShaperBucketNeg, 0x00000000, 0x0001d000+schedulerOffset)    /* Queue Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.PortNodeState, 0x00000000, 0x0001f200+schedulerOffset)    /* Port Node State */
    SET_REG_ADDRESS(addressSpace.Sched.PortMyQ, 0x00000000, 0x0001f600+schedulerOffset)    /* Port  MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus01, 0x00000000, 0x0001fa00+schedulerOffset)    /* Port RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus23, 0x00000000, 0x0001fe00+schedulerOffset)    /* Port RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus45, 0x00000000, 0x00020200+schedulerOffset)    /* Port RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus67, 0x00000000, 0x00020600+schedulerOffset)    /* Port RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortWFS, 0x00000000, 0x00020a00+schedulerOffset)    /* Port Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.PortBPFromQMgr, 0x00000000, 0x00020c00+schedulerOffset)    /* Port Back-Pressure From Queue Manager */
    SET_REG_ADDRESS(addressSpace.Sched.QueueNodeState, 0x00000000, 0x00027000+schedulerOffset)    /* Queue Node State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueWFS, 0x00000000, 0x00029000+schedulerOffset)    /* Queue Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateLo, 0x00000000, 0x00029200+schedulerOffset)    /* Queue L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateHi, 0x00000000, 0x00029600+schedulerOffset)    /* Queue L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateLo, 0x00000000, 0x00029a00+schedulerOffset)    /* Queue L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateHi, 0x00000000, 0x00029b00+schedulerOffset)    /* Queue L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateLo, 0x00000000, 0x00029c00+schedulerOffset)    /* Queue L2 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateHi, 0x00000000, 0x00029c40+schedulerOffset)    /* Queue L2 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.QueuePerStatus, 0x00000000, 0x0002c000+schedulerOffset)    /* Queue Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlNodeState, 0x00000000, 0x00020e00+schedulerOffset)    /* C Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlMyQ, 0x00000000, 0x00021200+schedulerOffset)    /* C Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus01, 0x00000000, 0x00021600+schedulerOffset)    /* C Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus23, 0x00000000, 0x00021a00+schedulerOffset)    /* C Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus45, 0x00000000, 0x00021e00+schedulerOffset)    /* C Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus67, 0x00000000, 0x00022200+schedulerOffset)    /* C Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlWFS, 0x00000000, 0x00022600+schedulerOffset)    /* C Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateLo, 0x00000000, 0x00022640+schedulerOffset)    /* C Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateHi, 0x00000000, 0x000226c0+schedulerOffset)    /* C Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlNodeState, 0x00000000, 0x00022e00+schedulerOffset)    /* B Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlMyQ, 0x00000000, 0x00023200+schedulerOffset)    /* B Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus01, 0x00000000, 0x00023600+schedulerOffset)    /* B Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus23, 0x00000000, 0x00023a00+schedulerOffset)    /* B Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus45, 0x00000000, 0x00023e00+schedulerOffset)    /* B Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus67, 0x00000000, 0x00024200+schedulerOffset)    /* B Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlWFS, 0x00000000, 0x00024600+schedulerOffset)    /* B Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateLo, 0x00000000, 0x00024680+schedulerOffset)    /* B Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateHi, 0x00000000, 0x00024780+schedulerOffset)    /* B Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateLo, 0x00000000, 0x00024880+schedulerOffset)    /* B Level L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateHi, 0x00000000, 0x000248c0+schedulerOffset)    /* B Level L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlNodeState, 0x00000000, 0x00024e00+schedulerOffset)    /* A Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlMyQ, 0x00000000, 0x00025200+schedulerOffset)    /* A Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus01, 0x00000000, 0x00025600+schedulerOffset)    /* A Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus23, 0x00000000, 0x00025a00+schedulerOffset)    /* A Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus45, 0x00000000, 0x00025e00+schedulerOffset)    /* A Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus67, 0x00000000, 0x00026200+schedulerOffset)    /* A Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlWFS, 0x00000000, 0x00026600+schedulerOffset)    /* A Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateLo, 0x00000000, 0x00026680+schedulerOffset)    /* A Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateHi, 0x00000000, 0x00026780+schedulerOffset)    /* A Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateLo, 0x00000000, 0x00026880+schedulerOffset)    /* A Level L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateHi, 0x00000000, 0x000268c0+schedulerOffset)    /* A Level L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateLo, 0x00000000, 0x00026900+schedulerOffset)    /* A Level L2 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateHi, 0x00000000, 0x00026920+schedulerOffset)    /* A Level L2 Cluster Hi State */
    cpssOsMemCpy(addressSpacePtr,&addressSpace,sizeof(struct prvCpssDxChTxqSchedAddressSpace));
}


static void prvSchedInitAdressStruct_Sip_6_15
(
    GT_U32 schedulerOffset,
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr
)
{

    struct prvCpssDxChTxqSchedAddressSpace addressSpace;

    SET_REG_ADDRESS(addressSpace.Sched.ErrorStatus_Addr, 0x00000000, 0x00000000+schedulerOffset)    /* QM Scheduler Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.FirstExcp_Addr, 0x00000000, 0x00000008+schedulerOffset)    /* QM Scheduler First Exception */
    SET_REG_ADDRESS(addressSpace.Sched.ErrCnt_Addr, 0x00000000, 0x00000010+schedulerOffset)    /* QM Scheduler Error Counter */
    SET_REG_ADDRESS(addressSpace.Sched.ExcpCnt_Addr, 0x00000000, 0x00000018+schedulerOffset)    /* QM Scheduler Exception Counter */
    SET_REG_ADDRESS(addressSpace.Sched.ExcpMask_Addr, 0x00000000, 0x00000020+schedulerOffset)    /* QM Scheduler Exception Mask */
    SET_REG_ADDRESS(addressSpace.Sched.Identity_Addr, 0x00000000, 0x00000028+schedulerOffset)    /* Identity */
    SET_REG_ADDRESS(addressSpace.Sched.ForceErr_Addr, 0x00000000, 0x00000030+schedulerOffset)    /* QM Scheduler Force Error */
    SET_REG_ADDRESS(addressSpace.Sched.ScrubSlots_Addr, 0x00000000, 0x00000038+schedulerOffset)    /* Scrubbing Slots Allocation */
    SET_REG_ADDRESS(addressSpace.Sched.BPMsgFIFO_Addr, 0x00000000, 0x00000040+schedulerOffset)    /* TM to TM Backpressure FIFO */
    SET_REG_ADDRESS(addressSpace.Sched.TreeDeqEn_Addr, 0x00000000, 0x00000048+schedulerOffset)    /* Tree Dequeue Enable */
    SET_REG_ADDRESS(addressSpace.Sched.PDWRREnReg, 0x00000000, 0x00000050+schedulerOffset)    /* Tree DWRR Priority Enable for Port Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.PPerCtlConf_Addr, 0x00000000, 0x00000800+schedulerOffset)    /* Port Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.PPerRateShap_Addr, 0x00000000, 0x00000808+schedulerOffset)    /* Port Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.PortExtBPEn_Addr, 0x00000000, 0x00000810+schedulerOffset)    /* Port External backpressure Enable */
    SET_REG_ADDRESS(addressSpace.Sched.PBytePerBurstLimit_Addr, 0x00000000, 0x00000818+schedulerOffset)    /* Port DWRR Byte Per Burst Limit */
    SET_REG_ADDRESS(addressSpace.Sched.CPerCtlConf_Addr, 0x00000000, 0x00003600+schedulerOffset)    /* C-level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.CPerRateShap_Addr, 0x00000000, 0x00003608+schedulerOffset)    /* C Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.BPerCtlConf_Addr, 0x00000000, 0x00006000+schedulerOffset)    /* B-Level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.BPerRateShap_Addr, 0x00000000, 0x00006008+schedulerOffset)    /* B level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.APerCtlConf_Addr, 0x00000000, 0x00008a00+schedulerOffset)    /* A-Level Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.APerRateShap_Addr, 0x00000000, 0x00008a08+schedulerOffset)    /* A Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.QPerCtlConf_Addr, 0x00000000, 0x0000b400+schedulerOffset)    /* Queue Periodic Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.QPerRateShap_Addr, 0x00000000, 0x0000b408+schedulerOffset)    /* Queue Level Periodic Rate Shaping Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.EccErrStatus_Addr, 0x00000000, 0x0001f000+schedulerOffset)    /* QM Scheduler ECC Error Log - General */
    SET_REG_ADDRESS(addressSpace.Sched.ScrubDis_Addr, 0x00000000, 0x0001f008+schedulerOffset)    /* Scrubbing Disable */
    SET_REG_ADDRESS(addressSpace.Sched.Plast_Addr, 0x00000000, 0x0001f010+schedulerOffset)    /* Tree Status */
    SET_REG_ADDRESS(addressSpace.Sched.PPerRateShapInt_Addr, 0x00000000, 0x00020a10+schedulerOffset)    /* Port Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.PMemsEccErrStatus_Addr, 0x00000000, 0x00020a18+schedulerOffset)    /* Port ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.PBnkEccErrStatus_Addr, 0x00000000, 0x00020a20+schedulerOffset)    /* Port Bank ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.PPerStatus_Addr, 0x00000000, 0x00020d10+schedulerOffset)    /* Port Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.CPerRateShapInt_Addr, 0x00000000, 0x00022740+schedulerOffset)    /* C-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.CMemsEccErrStatus_Addr, 0x00000000, 0x00022748+schedulerOffset)    /* C Level ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.CMyQEccErrStatus_Addr, 0x00000000, 0x00022750+schedulerOffset)    /* C Level MyQ ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.CBnkEccErrStatus_Addr, 0x00000000, 0x00022760+schedulerOffset)    /* C Level Bank ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.CPerStatus_Addr, 0x00000000, 0x00022c00+schedulerOffset)    /* C Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.BPerRateShapInt_Addr, 0x00000000, 0x00024900+schedulerOffset)    /* B-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.BMemsEccErrStatus_Addr, 0x00000000, 0x00024908+schedulerOffset)    /* B Level ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.BMyQEccErrStatus_Addr, 0x00000000, 0x00024910+schedulerOffset)    /* B Level MyQ ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.BBnkEccErrStatus_Addr, 0x00000000, 0x00024920+schedulerOffset)    /* B Level Bank ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.BPerStatus_Addr, 0x00000000, 0x00024930+schedulerOffset)    /* B Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.APerRateShapInt_Addr, 0x00000000, 0x00026940+schedulerOffset)    /* A-level Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.AMemsEccErrStatus_Addr, 0x00000000, 0x00026948+schedulerOffset)    /* A Level ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.AMyQEccErrStatus_Addr, 0x00000000, 0x00026950+schedulerOffset)    /* A Level MyQ ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.ABnkEccErrStatus_Addr, 0x00000000, 0x00026960+schedulerOffset)    /* A Level Bank ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.APerStatus_Addr, 0x00000000, 0x00026970+schedulerOffset)    /* A Level Periodic Status */
    SET_REG_ADDRESS(addressSpace.Sched.QMemsEccErrStatus_Addr, 0x00000000, 0x00029c80+schedulerOffset)    /* Queue ECC Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.QPerRateShapInt_Addr, 0x00000000, 0x00029c88+schedulerOffset)    /* Queue Periodic Rate Shaping Internal Parameters */
    SET_REG_ADDRESS(addressSpace.Sched.QBnkEccErrStatus_StartAddr, 0x00000000, 0x00029ca0+schedulerOffset)    /* Queue ECC Bank 0 Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.QueueBank1EccErrStatus, 0x00000000, 0x00029ca8+schedulerOffset)    /* Queue ECC Bank 1 Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.QueueBank2EccErrStatus, 0x00000000, 0x00029cb0+schedulerOffset)    /* Queue ECC Bank 2 Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.QueueBank3EccErrStatus, 0x00000000, 0x00029cb8+schedulerOffset)    /* Queue ECC Bank 3 Error Status */
    SET_REG_ADDRESS(addressSpace.Sched.EccConfig_Addr, 0x00000000, 0x0002c048+schedulerOffset)    /* ECC Configuration */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters0, 0x00000000, 0x0002c050+schedulerOffset)    /* ECC Memory Parameters0 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters1, 0x00000000, 0x0002c058+schedulerOffset)    /* ECC Memory Parameters1 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters2, 0x00000000, 0x0002c060+schedulerOffset)    /* ECC Memory Parameters2 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters3, 0x00000000, 0x0002c068+schedulerOffset)    /* ECC Memory Parameters3 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters4, 0x00000000, 0x0002c070+schedulerOffset)    /* ECC Memory Parameters4 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters5, 0x00000000, 0x0002c078+schedulerOffset)    /* ECC Memory Parameters5 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters6, 0x00000000, 0x0002c080+schedulerOffset)    /* ECC Memory Parameters6 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters7, 0x00000000, 0x0002c088+schedulerOffset)    /* ECC Memory Parameters7 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters8, 0x00000000, 0x0002c090+schedulerOffset)    /* ECC Memory Parameters8 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters9, 0x00000000, 0x0002c098+schedulerOffset)    /* ECC Memory Parameters9 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters10, 0x00000000, 0x0002c0a0+schedulerOffset)    /* ECC Memory Parameters10 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters11, 0x00000000, 0x0002c0a8+schedulerOffset)    /* ECC Memory Parameters11 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters12, 0x00000000, 0x0002c0b0+schedulerOffset)    /* ECC Memory Parameters12 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters13, 0x00000000, 0x0002c0b8+schedulerOffset)    /* ECC Memory Parameters13 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters14, 0x00000000, 0x0002c0c0+schedulerOffset)    /* ECC Memory Parameters14 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters15, 0x00000000, 0x0002c0c8+schedulerOffset)    /* ECC Memory Parameters15 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters16, 0x00000000, 0x0002c0d0+schedulerOffset)    /* ECC Memory Parameters16 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters17, 0x00000000, 0x0002c0d8+schedulerOffset)    /* ECC Memory Parameters17 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters18, 0x00000000, 0x0002c0e0+schedulerOffset)    /* ECC Memory Parameters18 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters19, 0x00000000, 0x0002c0e8+schedulerOffset)    /* ECC Memory Parameters19 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters20, 0x00000000, 0x0002c0f0+schedulerOffset)    /* ECC Memory Parameters20 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters21, 0x00000000, 0x0002c0f8+schedulerOffset)    /* ECC Memory Parameters21 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters22, 0x00000000, 0x0002c100+schedulerOffset)    /* ECC Memory Parameters22 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters23, 0x00000000, 0x0002c108+schedulerOffset)    /* ECC Memory Parameters23 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters24, 0x00000000, 0x0002c110+schedulerOffset)    /* ECC Memory Parameters24 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters25, 0x00000000, 0x0002c118+schedulerOffset)    /* ECC Memory Parameters25 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters26, 0x00000000, 0x0002c120+schedulerOffset)    /* ECC Memory Parameters26 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters27, 0x00000000, 0x0002c128+schedulerOffset)    /* ECC Memory Parameters27 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters28, 0x00000000, 0x0002c130+schedulerOffset)    /* ECC Memory Parameters28 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters29, 0x00000000, 0x0002c138+schedulerOffset)    /* ECC Memory Parameters29 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters30, 0x00000000, 0x0002c140+schedulerOffset)    /* ECC Memory Parameters30 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters31, 0x00000000, 0x0002c148+schedulerOffset)    /* ECC Memory Parameters31 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters32, 0x00000000, 0x0002c150+schedulerOffset)    /* ECC Memory Parameters32 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters33, 0x00000000, 0x0002c158+schedulerOffset)    /* ECC Memory Parameters33 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters34, 0x00000000, 0x0002c160+schedulerOffset)    /* ECC Memory Parameters34 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters35, 0x00000000, 0x0002c168+schedulerOffset)    /* ECC Memory Parameters35 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters36, 0x00000000, 0x0002c170+schedulerOffset)    /* ECC Memory Parameters36 */
    SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters37, 0x00000000, 0x0002c178+schedulerOffset)    /* ECC Memory Parameters37 */
    SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFunc_Entry, 0x00000000, 0x00000a00+schedulerOffset)    /* Port Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFuncPtr, 0x00000000, 0x00001a00+schedulerOffset)    /* Port Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketTokenEnDiv, 0x00000000, 0x00001e00+schedulerOffset)    /* Port Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketBurstSize, 0x00000000, 0x00002200+schedulerOffset)    /* Port Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.PortDWRRPrioEn, 0x00000000, 0x00002600+schedulerOffset)    /* Port DWRR Priority Enable for C-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosLo, 0x00000000, 0x00002a00+schedulerOffset)    /* Port DWRR Quantum - Low Priorities (0-3) */
    SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosHi, 0x00000000, 0x00002e00+schedulerOffset)    /* Port DWRR Quantum - High Priorities (4-7) */
    SET_REG_ADDRESS(addressSpace.Sched.PortRangeMap, 0x00000000, 0x00003200+schedulerOffset)    /* Port Mapping to C-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFunc_Entry, 0x00000000, 0x00003800+schedulerOffset)    /* C-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFuncPtr, 0x00000000, 0x00004800+schedulerOffset)    /* C-level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, 0x00000000, 0x00004c00+schedulerOffset)    /* C-level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketBurstSize, 0x00000000, 0x00005000+schedulerOffset)    /* C-level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlDWRRPrioEn, 0x00000000, 0x00005400+schedulerOffset)    /* C-level DWRR Priority Enable for B-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlQuantum, 0x00000000, 0x00005800+schedulerOffset)    /* C-Level DWRR Quantum */
    SET_REG_ADDRESS(addressSpace.Sched.ClvltoPortAndBlvlRangeMap, 0x00000000, 0x00005c00+schedulerOffset)    /* C-Level Mapping: C-Level to Port and C-Level to B-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFunc_Entry, 0x00000000, 0x00006200+schedulerOffset)    /* B-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFuncPtr, 0x00000000, 0x00007200+schedulerOffset)    /* B-Level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, 0x00000000, 0x00007600+schedulerOffset)    /* B-Level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketBurstSize, 0x00000000, 0x00007a00+schedulerOffset)    /* B-Level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlDWRRPrioEn, 0x00000000, 0x00007e00+schedulerOffset)    /* B-Level DWRR Priority Enable for A-Level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlQuantum, 0x00000000, 0x00008200+schedulerOffset)    /* B-Level DWRR Quantum */
    SET_REG_ADDRESS(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap, 0x00000000, 0x00008600+schedulerOffset)    /* B-Level Mapping: B-Level to C-Level Node and B-Level to A-Level Range */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFunc_Entry, 0x00000000, 0x00008c00+schedulerOffset)    /* A-Level Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFuncPtr, 0x00000000, 0x00009c00+schedulerOffset)    /* A-Level Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, 0x00000000, 0x0000a000+schedulerOffset)    /* A-Level Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketBurstSize, 0x00000000, 0x0000a400+schedulerOffset)    /* A-Level Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlDWRRPrioEn, 0x00000000, 0x0000a800+schedulerOffset)    /* A-Level DWRR Priority Enable for Queue level Scheduling */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlQuantum, 0x00000000, 0x0000ac00+schedulerOffset)    /* A-Level DWRR Quantum */
    SET_REG_ADDRESS(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap, 0x00000000, 0x0000b000+schedulerOffset)    /* A-Level Mapping: A-Level to B-Level Node And A-Level to Queue Range */
    SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFunc, 0x00000000, 0x0000b600+schedulerOffset)    /* Queue Elig Prio Function Table */
    SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFuncPtr, 0x00000000, 0x0000c000+schedulerOffset)    /* Queue Elig Prio Function Pointer */
    SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketTokenEnDiv, 0x00000000, 0x0000e000+schedulerOffset)    /* Queue Token Bucket - Tokens Enable and Divider */
    SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketBurstSize, 0x00000000, 0x00010000+schedulerOffset)    /* Queue Token Bucket - Burst Size */
    SET_REG_ADDRESS(addressSpace.Sched.QueueQuantum, 0x00000000, 0x00012000+schedulerOffset)    /* Queue DWRR Quantum */
    SET_REG_ADDRESS(addressSpace.Sched.QueueAMap, 0x00000000, 0x00014000+schedulerOffset)    /* Queue Mapping to A-Level Node */
    SET_REG_ADDRESS(addressSpace.Sched.PortShpBucketLvls, 0x00000000, 0x00016000+schedulerOffset)    /* Port Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.PortShaperBucketNeg, 0x00000000, 0x00016400+schedulerOffset)    /* Port Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio0, 0x00000000, 0x00016600+schedulerOffset)    /* Port DWRR Deficit - Priority 0 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio1, 0x00000000, 0x00016a00+schedulerOffset)    /* Port DWRR Deficit - Priority 1 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio2, 0x00000000, 0x00016e00+schedulerOffset)    /* Port DWRR Deficit - Priority 2 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio3, 0x00000000, 0x00017200+schedulerOffset)    /* Port DWRR Deficit - Priority 3 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio4, 0x00000000, 0x00017600+schedulerOffset)    /* Port DWRR Deficit - Priority 4 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio5, 0x00000000, 0x00017a00+schedulerOffset)    /* Port DWRR Deficit - Priority 5 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio6, 0x00000000, 0x00017e00+schedulerOffset)    /* Port DWRR Deficit - Priority 6 */
    SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio7, 0x00000000, 0x00018200+schedulerOffset)    /* Port DWRR Deficit - Priority 7 */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlShpBucketLvls, 0x00000000, 0x00018600+schedulerOffset)    /* C-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.CLevelShaperBucketNeg, 0x00000000, 0x00018a00+schedulerOffset)    /* C-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.CLvlDef, 0x00000000, 0x00018c00+schedulerOffset)    /* C-Level DWRR Deficit */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlShpBucketLvls, 0x00000000, 0x00019000+schedulerOffset)    /* B-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.BLevelShaperBucketNeg, 0x00000000, 0x00019400+schedulerOffset)    /* B-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlDef, 0x00000000, 0x00019600+schedulerOffset)    /* B-Level DWRR Deficit */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlShpBucketLvls, 0x00000000, 0x00019a00+schedulerOffset)    /* A-Level Shaper Bucket Levels */
    SET_REG_ADDRESS(addressSpace.Sched.ALevelShaperBucketNeg, 0x00000000, 0x00019e00+schedulerOffset)    /* A-Level Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlDef, 0x00000000, 0x0001a000+schedulerOffset)    /* A-Level DWRR Deficit */
    SET_REG_ADDRESS(addressSpace.Sched.QueueShpBucketLvls, 0x00000000, 0x0001b000+schedulerOffset)    /* Queue Shaper Bucket Level */
    SET_REG_ADDRESS(addressSpace.Sched.QueueShaperBucketNeg, 0x00000000, 0x0001d000+schedulerOffset)    /* Queue Shaper Bucket Negative */
    SET_REG_ADDRESS(addressSpace.Sched.QueueDef, 0x00000000, 0x0001e000+schedulerOffset)    /* Queue DWRR Deficit */
    SET_REG_ADDRESS(addressSpace.Sched.PortNodeState, 0x00000000, 0x0001f200+schedulerOffset)    /* Port Node State */
    SET_REG_ADDRESS(addressSpace.Sched.PortMyQ, 0x00000000, 0x0001f600+schedulerOffset)    /* Port  MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus01, 0x00000000, 0x0001fa00+schedulerOffset)    /* Port RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus23, 0x00000000, 0x0001fe00+schedulerOffset)    /* Port RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus45, 0x00000000, 0x00020200+schedulerOffset)    /* Port RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus67, 0x00000000, 0x00020600+schedulerOffset)    /* Port RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.PortWFS, 0x00000000, 0x00020a00+schedulerOffset)    /* Port Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.PortBPFromSTF, 0x00000000, 0x00020b00+schedulerOffset)    /* Port Back-Pressure From FCU */
    SET_REG_ADDRESS(addressSpace.Sched.PortBPFromQMgr, 0x00000000, 0x00020c00+schedulerOffset)    /* Port Back-Pressure From Queue Manager */
    SET_REG_ADDRESS(addressSpace.Sched.TMtoTMPortBPState, 0x00000000, 0x00020d00+schedulerOffset)    /* Port Back-Pressure From TM-to-TM */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlNodeState, 0x00000000, 0x00020e00+schedulerOffset)    /* C Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlMyQ, 0x00000000, 0x00021200+schedulerOffset)    /* C Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus01, 0x00000000, 0x00021600+schedulerOffset)    /* C Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus23, 0x00000000, 0x00021a00+schedulerOffset)    /* C Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus45, 0x00000000, 0x00021e00+schedulerOffset)    /* C Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus67, 0x00000000, 0x00022200+schedulerOffset)    /* C Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlWFS, 0x00000000, 0x00022600+schedulerOffset)    /* C Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateLo, 0x00000000, 0x00022640+schedulerOffset)    /* C Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateHi, 0x00000000, 0x000226c0+schedulerOffset)    /* C Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.ClvlBPFromSTF, 0x00000000, 0x00022770+schedulerOffset)    /* C Level Back-Pressure From Status Formatter */
    SET_REG_ADDRESS(addressSpace.Sched.TMtoTMClvlBPState, 0x00000000, 0x00022800+schedulerOffset)    /* C Level TM to TM Back-Pressure State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlNodeState, 0x00000000, 0x00022e00+schedulerOffset)    /* B Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlMyQ, 0x00000000, 0x00023200+schedulerOffset)    /* B Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus01, 0x00000000, 0x00023600+schedulerOffset)    /* B Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus23, 0x00000000, 0x00023a00+schedulerOffset)    /* B Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus45, 0x00000000, 0x00023e00+schedulerOffset)    /* B Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus67, 0x00000000, 0x00024200+schedulerOffset)    /* B Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlWFS, 0x00000000, 0x00024600+schedulerOffset)    /* B Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateLo, 0x00000000, 0x00024680+schedulerOffset)    /* B Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateHi, 0x00000000, 0x00024780+schedulerOffset)    /* B Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateLo, 0x00000000, 0x00024880+schedulerOffset)    /* B Level L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateHi, 0x00000000, 0x000248c0+schedulerOffset)    /* B Level L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.TMtoTMBlvlBPState, 0x00000000, 0x00024a00+schedulerOffset)    /* B Level TM to TM Back-Pressure State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlNodeState, 0x00000000, 0x00024e00+schedulerOffset)    /* A Level Node State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlMyQ, 0x00000000, 0x00025200+schedulerOffset)    /* A Level MyQ */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus01, 0x00000000, 0x00025600+schedulerOffset)    /* A Level RR/DWRR 01 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus23, 0x00000000, 0x00025a00+schedulerOffset)    /* A Level RR/DWRR 23 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus45, 0x00000000, 0x00025e00+schedulerOffset)    /* A Level RR/DWRR 45 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus67, 0x00000000, 0x00026200+schedulerOffset)    /* A Level RR/DWRR 67 Status */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlWFS, 0x00000000, 0x00026600+schedulerOffset)    /* A Level Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateLo, 0x00000000, 0x00026680+schedulerOffset)    /* A Level L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateHi, 0x00000000, 0x00026780+schedulerOffset)    /* A Level L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateLo, 0x00000000, 0x00026880+schedulerOffset)    /* A Level L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateHi, 0x00000000, 0x000268c0+schedulerOffset)    /* A Level L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateLo, 0x00000000, 0x00026900+schedulerOffset)    /* A Level L2 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateHi, 0x00000000, 0x00026920+schedulerOffset)    /* A Level L2 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.TMtoTMAlvlBPState, 0x00000000, 0x00026a00+schedulerOffset)    /* A Level TM to TM Back-Pressure State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueNodeState, 0x00000000, 0x00027000+schedulerOffset)    /* Queue Node State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueWFS, 0x00000000, 0x00029000+schedulerOffset)    /* Queue Wait For Scrub */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateLo, 0x00000000, 0x00029200+schedulerOffset)    /* Queue L0 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateHi, 0x00000000, 0x00029600+schedulerOffset)    /* Queue L0 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateLo, 0x00000000, 0x00029a00+schedulerOffset)    /* Queue L1 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateHi, 0x00000000, 0x00029b00+schedulerOffset)    /* Queue L1 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateLo, 0x00000000, 0x00029c00+schedulerOffset)    /* Queue L2 Cluster Lo State */
    SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateHi, 0x00000000, 0x00029c40+schedulerOffset)    /* Queue L2 Cluster Hi State */
    SET_REG_ADDRESS(addressSpace.Sched.TMtoTMQueueBPState, 0x00000000, 0x0002a000+schedulerOffset)    /* Queue TM to TM Back-Pressure State */
    SET_REG_ADDRESS(addressSpace.Sched.QueuePerStatus, 0x00000000, 0x0002c000+schedulerOffset)    /* Queue Periodic Status */

    cpssOsMemCpy(addressSpacePtr,&addressSpace,sizeof(struct prvCpssDxChTxqSchedAddressSpace));
}


void prvSchedInitAdressStruct
(
    GT_U32 devNum,
    GT_U32 tileNum,
    GT_U32 schedulerOffset,
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr
)
{

    struct prvCpssDxChTxqSchedAddressSpace addressSpace;

    /*initialize only for tile 0*/
    if(tileNum!=0)
    {
        return;
    }

    if(GT_TRUE ==PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        prvSchedInitAdressStruct_Sip_6_30(schedulerOffset,addressSpacePtr);
        return;
    }

    if(GT_TRUE ==PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        prvSchedInitAdressStruct_Sip_6_15(schedulerOffset,addressSpacePtr);
        return;
    }

    {
        SET_REG_ADDRESS(addressSpace.Sched.EccErrStatus_Addr, 0x00000000, 0x00064000+schedulerOffset)    /* QM Scheduler ECC Error Log - General */
        SET_REG_ADDRESS(addressSpace.Sched.ScrubDis_Addr, 0x00000000, 0x00064008+schedulerOffset)    /* Scrubbing Disable */
        SET_REG_ADDRESS(addressSpace.Sched.Plast_Addr, 0x00000000, 0x00064010+schedulerOffset)    /* Tree Status */
        SET_REG_ADDRESS(addressSpace.Sched.PPerRateShapInt_Addr, 0x00000000, 0x00067420+schedulerOffset)    /* Port Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.PMemsEccErrStatus_Addr, 0x00000000, 0x00067428+schedulerOffset)    /* Port ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.PBnkEccErrStatus_Addr, 0x00000000, 0x00067430+schedulerOffset)    /* Port Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.PPerStatus_Addr, 0x00000000, 0x00067720+schedulerOffset)    /* Port Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.CPerRateShapInt_Addr, 0x00000000, 0x0006aa80+schedulerOffset)    /* C-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.CMemsEccErrStatus_Addr, 0x00000000, 0x0006aa88+schedulerOffset)    /* C Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CMyQEccErrStatus_Addr, 0x00000000, 0x0006aa90+schedulerOffset)    /* C Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CBnkEccErrStatus_Addr, 0x00000000, 0x0006aaa0+schedulerOffset)    /* C Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.CPerStatus_Addr, 0x00000000, 0x0006b400+schedulerOffset)    /* C Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.BPerRateShapInt_Addr, 0x00000000, 0x0006eac0+schedulerOffset)    /* B-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.BMemsEccErrStatus_Addr, 0x00000000, 0x0006eac8+schedulerOffset)    /* B Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BMyQEccErrStatus_Addr, 0x00000000, 0x0006ead0+schedulerOffset)    /* B Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BBnkEccErrStatus_Addr, 0x00000000, 0x0006eae0+schedulerOffset)    /* B Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.BPerStatus_Addr, 0x00000000, 0x0006eaf0+schedulerOffset)    /* B Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.APerRateShapInt_Addr, 0x00000000, 0x0007cc80+schedulerOffset)    /* A-level Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.AMemsEccErrStatus_Addr, 0x00000000, 0x0007cc88+schedulerOffset)    /* A Level ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.AMyQEccErrStatus_Addr, 0x00000000, 0x0007cc90+schedulerOffset)    /* A Level MyQ ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.ABnkEccErrStatus_Addr, 0x00000000, 0x0007cca0+schedulerOffset)    /* A Level Bank ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.APerStatus_Addr, 0x00000000, 0x0007ccb0+schedulerOffset)    /* A Level Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.QMemsEccErrStatus_Addr, 0x00000000, 0x0008ad00+schedulerOffset)    /* Queue ECC Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QPerRateShapInt_Addr, 0x00000000, 0x0008ad08+schedulerOffset)    /* Queue Periodic Rate Shaping Internal Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.QBnkEccErrStatus_StartAddr, 0x00000000, 0x0008ad20+schedulerOffset)    /* Queue ECC Bank 0 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank1EccErrStatus, 0x00000000, 0x0008ad28+schedulerOffset)    /* Queue ECC Bank 1 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank2EccErrStatus, 0x00000000, 0x0008ad30+schedulerOffset)    /* Queue ECC Bank 2 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.QueueBank3EccErrStatus, 0x00000000, 0x0008ad38+schedulerOffset)    /* Queue ECC Bank 3 Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.ErrorStatus_Addr, 0x00000000, 0x00000000+schedulerOffset)    /* QM Scheduler Error Status */
        SET_REG_ADDRESS(addressSpace.Sched.FirstExcp_Addr, 0x00000000, 0x00000008+schedulerOffset)    /* QM Scheduler First Exception */
        SET_REG_ADDRESS(addressSpace.Sched.ErrCnt_Addr, 0x00000000, 0x00000010+schedulerOffset)    /* QM Scheduler Error Counter */
        SET_REG_ADDRESS(addressSpace.Sched.ExcpCnt_Addr, 0x00000000, 0x00000018+schedulerOffset)    /* QM Scheduler Exception Counter */
        SET_REG_ADDRESS(addressSpace.Sched.ExcpMask_Addr, 0x00000000, 0x00000020+schedulerOffset)    /* QM Scheduler Exception Mask */
        SET_REG_ADDRESS(addressSpace.Sched.Identity_Addr, 0x00000000, 0x00000028+schedulerOffset)    /* Identity */
        SET_REG_ADDRESS(addressSpace.Sched.ForceErr_Addr, 0x00000000, 0x00000030+schedulerOffset)    /* QM Scheduler Force Error */
        SET_REG_ADDRESS(addressSpace.Sched.ScrubSlots_Addr, 0x00000000, 0x00000038+schedulerOffset)    /* Scrubbing Slots Allocation */
        SET_REG_ADDRESS(addressSpace.Sched.BPMsgFIFO_Addr, 0x00000000, 0x00000040+schedulerOffset)    /* TM to TM Backpressure FIFO */
        SET_REG_ADDRESS(addressSpace.Sched.TreeDeqEn_Addr, 0x00000000, 0x00000048+schedulerOffset)    /* Tree Dequeue Enable */
        SET_REG_ADDRESS(addressSpace.Sched.PDWRREnReg, 0x00000000, 0x00000050+schedulerOffset)    /* Tree DWRR Priority Enable for Port Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.PPerCtlConf_Addr, 0x00000000, 0x00000800+schedulerOffset)    /* Port Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.PPerRateShap_Addr, 0x00000000, 0x00000808+schedulerOffset)    /* Port Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.PortExtBPEn_Addr, 0x00000000, 0x00000810+schedulerOffset)    /* Port External backpressure Enable */
        SET_REG_ADDRESS(addressSpace.Sched.PBytePerBurstLimit_Addr, 0x00000000, 0x00000818+schedulerOffset)    /* Port DWRR Byte Per Burst Limit */
        SET_REG_ADDRESS(addressSpace.Sched.CPerCtlConf_Addr, 0x00000000, 0x00005400+schedulerOffset)    /* C-level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.CPerRateShap_Addr, 0x00000000, 0x00005408+schedulerOffset)    /* C Level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.BPerCtlConf_Addr, 0x00000000, 0x00009800+schedulerOffset)    /* B-Level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.BPerRateShap_Addr, 0x00000000, 0x00009808+schedulerOffset)    /* B level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.APerCtlConf_Addr, 0x00000000, 0x0000dc00+schedulerOffset)    /* A-Level Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.APerRateShap_Addr, 0x00000000, 0x0000dc08+schedulerOffset)    /* A Level Periodic Rate Shaping Parameters */
        SET_REG_ADDRESS(addressSpace.Sched.QPerCtlConf_Addr, 0x00000000, 0x0001b000+schedulerOffset)    /* Queue Periodic Configuration */
        SET_REG_ADDRESS(addressSpace.Sched.QPerRateShap_Addr, 0x00000000, 0x0001b008+schedulerOffset)    /* Queue Level Periodic Rate Shaping Parameters */

        if(GT_FALSE ==PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            SET_REG_ADDRESS(addressSpace.Sched.EccConfig_Addr, 0x00000000, 0x00094040+schedulerOffset)    /* ECC Configuration */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters0, 0x00000000, 0x00094048+schedulerOffset)    /* ECC Memory Parameters0 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters1, 0x00000000, 0x00094050+schedulerOffset)    /* ECC Memory Parameters1 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters2, 0x00000000, 0x00094058+schedulerOffset)    /* ECC Memory Parameters2 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters3, 0x00000000, 0x00094060+schedulerOffset)    /* ECC Memory Parameters3 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters4, 0x00000000, 0x00094068+schedulerOffset)    /* ECC Memory Parameters4 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters5, 0x00000000, 0x00094070+schedulerOffset)    /* ECC Memory Parameters5 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters6, 0x00000000, 0x00094078+schedulerOffset)    /* ECC Memory Parameters6 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters7, 0x00000000, 0x00094080+schedulerOffset)    /* ECC Memory Parameters7 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters8, 0x00000000, 0x00094088+schedulerOffset)    /* ECC Memory Parameters8 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters9, 0x00000000, 0x00094090+schedulerOffset)    /* ECC Memory Parameters9 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters10, 0x00000000, 0x00094098+schedulerOffset)    /* ECC Memory Parameters10 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters11, 0x00000000, 0x000940a0+schedulerOffset)    /* ECC Memory Parameters11 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters12, 0x00000000, 0x000940a8+schedulerOffset)    /* ECC Memory Parameters12 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters13, 0x00000000, 0x000940b0+schedulerOffset)    /* ECC Memory Parameters13 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters14, 0x00000000, 0x000940b8+schedulerOffset)    /* ECC Memory Parameters14 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters15, 0x00000000, 0x000940c0+schedulerOffset)    /* ECC Memory Parameters15 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters16, 0x00000000, 0x000940c8+schedulerOffset)    /* ECC Memory Parameters16 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters17, 0x00000000, 0x000940d0+schedulerOffset)    /* ECC Memory Parameters17 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters18, 0x00000000, 0x000940d8+schedulerOffset)    /* ECC Memory Parameters18 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters19, 0x00000000, 0x000940e0+schedulerOffset)    /* ECC Memory Parameters19 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters20, 0x00000000, 0x000940e8+schedulerOffset)    /* ECC Memory Parameters20 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters21, 0x00000000, 0x000940f0+schedulerOffset)    /* ECC Memory Parameters21 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters22, 0x00000000, 0x000940f8+schedulerOffset)    /* ECC Memory Parameters22 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters23, 0x00000000, 0x00094100+schedulerOffset)    /* ECC Memory Parameters23 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters24, 0x00000000, 0x00094108+schedulerOffset)    /* ECC Memory Parameters24 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters25, 0x00000000, 0x00094110+schedulerOffset)    /* ECC Memory Parameters25 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters26, 0x00000000, 0x00094118+schedulerOffset)    /* ECC Memory Parameters26 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters27, 0x00000000, 0x00094120+schedulerOffset)    /* ECC Memory Parameters27 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters28, 0x00000000, 0x00094128+schedulerOffset)    /* ECC Memory Parameters28 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters29, 0x00000000, 0x00094130+schedulerOffset)    /* ECC Memory Parameters29 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters30, 0x00000000, 0x00094138+schedulerOffset)    /* ECC Memory Parameters30 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters31, 0x00000000, 0x00094140+schedulerOffset)    /* ECC Memory Parameters31 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters32, 0x00000000, 0x00094148+schedulerOffset)    /* ECC Memory Parameters32 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters33, 0x00000000, 0x00094150+schedulerOffset)    /* ECC Memory Parameters33 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters34, 0x00000000, 0x00094158+schedulerOffset)    /* ECC Memory Parameters34 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters35, 0x00000000, 0x00094160+schedulerOffset)    /* ECC Memory Parameters35 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters36, 0x00000000, 0x00094168+schedulerOffset)    /* ECC Memory Parameters36 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters37, 0x00000000, 0x00094170+schedulerOffset)    /* ECC Memory Parameters37 */
        }
        else
        {
            SET_REG_ADDRESS(addressSpace.Sched.EccConfig_Addr, 0x00000000, 0x00094048+schedulerOffset)    /* ECC Configuration */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters0, 0x00000000, 0x00094050+schedulerOffset)    /* ECC Memory Parameters0 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters1, 0x00000000, 0x00094058+schedulerOffset)    /* ECC Memory Parameters1 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters2, 0x00000000, 0x00094060+schedulerOffset)    /* ECC Memory Parameters2 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters3, 0x00000000, 0x00094068+schedulerOffset)    /* ECC Memory Parameters3 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters4, 0x00000000, 0x00094070+schedulerOffset)    /* ECC Memory Parameters4 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters5, 0x00000000, 0x00094078+schedulerOffset)    /* ECC Memory Parameters5 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters6, 0x00000000, 0x00094080+schedulerOffset)    /* ECC Memory Parameters6 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters7, 0x00000000, 0x00094088+schedulerOffset)    /* ECC Memory Parameters7 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters8, 0x00000000, 0x00094090+schedulerOffset)    /* ECC Memory Parameters8 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters9, 0x00000000, 0x00094098+schedulerOffset)    /* ECC Memory Parameters9 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters10, 0x00000000, 0x000940a0+schedulerOffset)    /* ECC Memory Parameters10 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters11, 0x00000000, 0x000940a8+schedulerOffset)    /* ECC Memory Parameters11 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters12, 0x00000000, 0x000940b0+schedulerOffset)    /* ECC Memory Parameters12 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters13, 0x00000000, 0x000940b8+schedulerOffset)    /* ECC Memory Parameters13 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters14, 0x00000000, 0x000940c0+schedulerOffset)    /* ECC Memory Parameters14 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters15, 0x00000000, 0x000940c8+schedulerOffset)    /* ECC Memory Parameters15 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters16, 0x00000000, 0x000940d0+schedulerOffset)    /* ECC Memory Parameters16 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters17, 0x00000000, 0x000940d8+schedulerOffset)    /* ECC Memory Parameters17 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters18, 0x00000000, 0x000940e0+schedulerOffset)    /* ECC Memory Parameters18 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters19, 0x00000000, 0x000940e8+schedulerOffset)    /* ECC Memory Parameters19 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters20, 0x00000000, 0x000940f0+schedulerOffset)    /* ECC Memory Parameters20 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters21, 0x00000000, 0x000940f8+schedulerOffset)    /* ECC Memory Parameters21 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters22, 0x00000000, 0x00094100+schedulerOffset)    /* ECC Memory Parameters22 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters23, 0x00000000, 0x00094108+schedulerOffset)    /* ECC Memory Parameters23 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters24, 0x00000000, 0x00094110+schedulerOffset)    /* ECC Memory Parameters24 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters25, 0x00000000, 0x00094118+schedulerOffset)    /* ECC Memory Parameters25 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters26, 0x00000000, 0x00094120+schedulerOffset)    /* ECC Memory Parameters26 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters27, 0x00000000, 0x00094128+schedulerOffset)    /* ECC Memory Parameters27 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters28, 0x00000000, 0x00094130+schedulerOffset)    /* ECC Memory Parameters28 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters29, 0x00000000, 0x00094138+schedulerOffset)    /* ECC Memory Parameters29 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters30, 0x00000000, 0x00094140+schedulerOffset)    /* ECC Memory Parameters30 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters31, 0x00000000, 0x00094148+schedulerOffset)    /* ECC Memory Parameters31 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters32, 0x00000000, 0x00094150+schedulerOffset)    /* ECC Memory Parameters32 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters33, 0x00000000, 0x00094158+schedulerOffset)    /* ECC Memory Parameters33 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters34, 0x00000000, 0x00094160+schedulerOffset)    /* ECC Memory Parameters34 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters35, 0x00000000, 0x00094168+schedulerOffset)    /* ECC Memory Parameters35 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters36, 0x00000000, 0x00094170+schedulerOffset)    /* ECC Memory Parameters36 */
            SET_REG_ADDRESS(addressSpace.Sched.ECC_Memory_Parameters37, 0x00000000, 0x00094178+schedulerOffset)    /* ECC Memory Parameters37 */
        }
        SET_REG_ADDRESS(addressSpace.Sched.PortNodeState, 0x00000000, 0x00064400+schedulerOffset)    /* Port Node State */
        SET_REG_ADDRESS(addressSpace.Sched.PortMyQ, 0x00000000, 0x00064c00+schedulerOffset)    /* Port  MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.PortWFS, 0x00000000, 0x00067400+schedulerOffset)    /* Port Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.PortBPFromSTF, 0x00000000, 0x00067500+schedulerOffset)    /* Port Back-Pressure From FCU */
        SET_REG_ADDRESS(addressSpace.Sched.PortBPFromQMgr, 0x00000000, 0x00067600+schedulerOffset)    /* Port Back-Pressure From Queue Manager */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlNodeState, 0x00000000, 0x00067800+schedulerOffset)    /* C Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlMyQ, 0x00000000, 0x00068000+schedulerOffset)    /* C Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus01, 0x00000000, 0x00068800+schedulerOffset)    /* C Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus23, 0x00000000, 0x00069000+schedulerOffset)    /* C Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlWFS, 0x00000000, 0x0006a800+schedulerOffset)    /* C Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateLo, 0x00000000, 0x0006a880+schedulerOffset)    /* C Level L0 Cluster State Lo */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlBPFromSTF, 0x00000000, 0x0006aab0+schedulerOffset)    /* C Level Back-Pressure From Status Formatter */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMClvlBPState, 0x00000000, 0x0006ac00+schedulerOffset)    /* C Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlNodeState, 0x00000000, 0x0006b800+schedulerOffset)    /* B Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlMyQ, 0x00000000, 0x0006c000+schedulerOffset)    /* B Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus01, 0x00000000, 0x0006c800+schedulerOffset)    /* B Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus23, 0x00000000, 0x0006d000+schedulerOffset)    /* B Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlWFS, 0x00000000, 0x0006e800+schedulerOffset)    /* B Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateLo, 0x00000000, 0x0006e880+schedulerOffset)    /* B Level L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateLo, 0x00000000, 0x0006ea80+schedulerOffset)    /* B Level L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL1ClusterStateHi, 0x00000000, 0x0006eaa0+schedulerOffset)    /* B Level L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlNodeState, 0x00000000, 0x00070000+schedulerOffset)    /* A Level Node State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlMyQ, 0x00000000, 0x00072000+schedulerOffset)    /* A Level MyQ */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus01, 0x00000000, 0x00074000+schedulerOffset)    /* A Level RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlWFS, 0x00000000, 0x0007c000+schedulerOffset)    /* A Level Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateLo, 0x00000000, 0x0007c200+schedulerOffset)    /* A Level L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateLo, 0x00000000, 0x0007ca00+schedulerOffset)    /* A Level L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL1ClusterStateHi, 0x00000000, 0x0007cb00+schedulerOffset)    /* A Level L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateLo, 0x00000000, 0x0007cc00+schedulerOffset)    /* A Level L2 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL2ClusterStateHi, 0x00000000, 0x0007cc40+schedulerOffset)    /* A Level L2 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueNodeState, 0x00000000, 0x00080000+schedulerOffset)    /* Queue Node State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueWFS, 0x00000000, 0x00088000+schedulerOffset)    /* Queue Wait For Scrub */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateLo, 0x00000000, 0x00088800+schedulerOffset)    /* Queue L0 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateLo, 0x00000000, 0x0008a800+schedulerOffset)    /* Queue L1 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL1ClusterStateHi, 0x00000000, 0x0008aa00+schedulerOffset)    /* Queue L1 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateLo, 0x00000000, 0x0008ac00+schedulerOffset)    /* Queue L2 Cluster Lo State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL2ClusterStateHi, 0x00000000, 0x0008ac80+schedulerOffset)    /* Queue L2 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMQueueBPState, 0x00000000, 0x0008c000+schedulerOffset)    /* Queue TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.QueuePerStatus, 0x00000000, 0x00094000+schedulerOffset)    /* Queue Periodic Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFunc_Entry, 0x00000000, 0x00000a00+schedulerOffset)    /* Port Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.PortEligPrioFuncPtr, 0x00000000, 0x00001c00+schedulerOffset)    /* Port Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketTokenEnDiv, 0x00000000, 0x00002400+schedulerOffset)    /* Port Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.PortTokenBucketBurstSize, 0x00000000, 0x00002c00+schedulerOffset)    /* Port Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.PortDWRRPrioEn, 0x00000000, 0x00003400+schedulerOffset)    /* Port DWRR Priority Enable for C-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosLo, 0x00000000, 0x00003c00+schedulerOffset)    /* Port DWRR Quantum - Low Priorities (0-3) */
        SET_REG_ADDRESS(addressSpace.Sched.PortQuantumsPriosHi, 0x00000000, 0x00004400+schedulerOffset)    /* Port DWRR Quantum - High Priorities (4-7) */
        SET_REG_ADDRESS(addressSpace.Sched.PortRangeMap, 0x00000000, 0x00004c00+schedulerOffset)    /* Port Mapping to C-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFunc_Entry, 0x00000000, 0x00005600+schedulerOffset)    /* C-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlEligPrioFuncPtr, 0x00000000, 0x00006800+schedulerOffset)    /* C-level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketTokenEnDiv, 0x00000000, 0x00007000+schedulerOffset)    /* C-level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlTokenBucketBurstSize, 0x00000000, 0x00007800+schedulerOffset)    /* C-level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlDWRRPrioEn, 0x00000000, 0x00008000+schedulerOffset)    /* C-level DWRR Priority Enable for B-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlQuantum, 0x00000000, 0x00008800+schedulerOffset)    /* C-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.ClvltoPortAndBlvlRangeMap, 0x00000000, 0x00009000+schedulerOffset)    /* C-Level Mapping: C-Level to Port and C-Level to B-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFunc_Entry, 0x00000000, 0x00009a00+schedulerOffset)    /* B-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlEligPrioFuncPtr, 0x00000000, 0x0000ac00+schedulerOffset)    /* B-Level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketTokenEnDiv, 0x00000000, 0x0000b400+schedulerOffset)    /* B-Level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlTokenBucketBurstSize, 0x00000000, 0x0000bc00+schedulerOffset)    /* B-Level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlDWRRPrioEn, 0x00000000, 0x0000c400+schedulerOffset)    /* B-Level DWRR Priority Enable for A-Level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlQuantum, 0x00000000, 0x0000cc00+schedulerOffset)    /* B-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap, 0x00000000, 0x0000d400+schedulerOffset)    /* B-Level Mapping: B-Level to C-Level Node and B-Level to A-Level Range */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFunc_Entry, 0x00000000, 0x0000de00+schedulerOffset)    /* A-Level Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlEligPrioFuncPtr, 0x00000000, 0x0000f000+schedulerOffset)    /* A-Level Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketTokenEnDiv, 0x00000000, 0x00011000+schedulerOffset)    /* A-Level Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlTokenBucketBurstSize, 0x00000000, 0x00013000+schedulerOffset)    /* A-Level Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlDWRRPrioEn, 0x00000000, 0x00015000+schedulerOffset)    /* A-Level DWRR Priority Enable for Queue level Scheduling */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlQuantum, 0x00000000, 0x00017000+schedulerOffset)    /* A-Level DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap, 0x00000000, 0x00019000+schedulerOffset)    /* A-Level Mapping: A-Level to B-Level Node And A-Level to Queue Range */
        SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFunc, 0x00000000, 0x0001b200+schedulerOffset)    /* Queue Elig Prio Function Table */
        SET_REG_ADDRESS(addressSpace.Sched.QueueEligPrioFuncPtr, 0x00000000, 0x0001c000+schedulerOffset)    /* Queue Elig Prio Function Pointer */
        SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketTokenEnDiv, 0x00000000, 0x00024000+schedulerOffset)    /* Queue Token Bucket - Tokens Enable and Divider */
        SET_REG_ADDRESS(addressSpace.Sched.QueueTokenBucketBurstSize, 0x00000000, 0x0002c000+schedulerOffset)    /* Queue Token Bucket - Burst Size */
        SET_REG_ADDRESS(addressSpace.Sched.QueueQuantum, 0x00000000, 0x00034000+schedulerOffset)    /* Queue DWRR Quantum */
        SET_REG_ADDRESS(addressSpace.Sched.QueueAMap, 0x00000000, 0x0003c000+schedulerOffset)    /* Queue Mapping to A-Level Node */
        SET_REG_ADDRESS(addressSpace.Sched.PortShpBucketLvls, 0x00000000, 0x00044000+schedulerOffset)    /* Port Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlShpBucketLvls, 0x00000000, 0x00048c00+schedulerOffset)    /* C-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.CLvlDef, 0x00000000, 0x00049800+schedulerOffset)    /* C-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlShpBucketLvls, 0x00000000, 0x0004a000+schedulerOffset)    /* B-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlDef, 0x00000000, 0x0004ac00+schedulerOffset)    /* B-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlShpBucketLvls, 0x00000000, 0x0004c000+schedulerOffset)    /* A-Level Shaper Bucket Levels */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlDef, 0x00000000, 0x0004f000+schedulerOffset)    /* A-Level DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.QueueShpBucketLvls, 0x00000000, 0x00054000+schedulerOffset)    /* Queue Shaper Bucket Level */
        SET_REG_ADDRESS(addressSpace.Sched.QueueDef, 0x00000000, 0x00060000+schedulerOffset)    /* Queue DWRR Deficit */
        SET_REG_ADDRESS(addressSpace.Sched.PortShaperBucketNeg, 0x00000000, 0x00044800+schedulerOffset)    /* Port Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.CLevelShaperBucketNeg, 0x00000000, 0x00049400+schedulerOffset)    /* C-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.BLevelShaperBucketNeg, 0x00000000, 0x0004a800+schedulerOffset)    /* B-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.ALevelShaperBucketNeg, 0x00000000, 0x0004e000+schedulerOffset)    /* A-Level Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.QueueShaperBucketNeg, 0x00000000, 0x0005c000+schedulerOffset)    /* Queue Shaper Bucket Negative */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus67, 0x00000000, 0x0006a000+schedulerOffset)    /* C Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlRRDWRRStatus45, 0x00000000, 0x00069800+schedulerOffset)    /* C Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.ClvlL0ClusterStateHi, 0x00000000, 0x0006a980+schedulerOffset)    /* C Level L0 Cluster State Hi */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus67, 0x00000000, 0x0006e000+schedulerOffset)    /* B Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlRRDWRRStatus45, 0x00000000, 0x0006d800+schedulerOffset)    /* B Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.BlvlL0ClusterStateHi, 0x00000000, 0x0006e980+schedulerOffset)    /* B Level L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus67, 0x00000000, 0x0007a000+schedulerOffset)    /* A Level RR/DWRR 67 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus45, 0x00000000, 0x00078000+schedulerOffset)    /* A Level RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlL0ClusterStateHi, 0x00000000, 0x0007c600+schedulerOffset)    /* A Level L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.QueueL0ClusterStateHi, 0x00000000, 0x00089800+schedulerOffset)    /* Queue L0 Cluster Hi State */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMPortBPState, 0x00000000, 0x00067700+schedulerOffset)    /* Port Back-Pressure From TM-to-TM */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMBlvlBPState, 0x00000000, 0x0006ec00+schedulerOffset)    /* B Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.AlvlRRDWRRStatus23, 0x00000000, 0x00076000+schedulerOffset)    /* A Level RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.TMtoTMAlvlBPState, 0x00000000, 0x0007d000+schedulerOffset)    /* A Level TM to TM Back-Pressure State */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio0, 0x00000000, 0x00044c00+schedulerOffset)    /* Port DWRR Deficit - Priority 0 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio1, 0x00000000, 0x00045400+schedulerOffset)    /* Port DWRR Deficit - Priority 1 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio2, 0x00000000, 0x00045c00+schedulerOffset)    /* Port DWRR Deficit - Priority 2 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio3, 0x00000000, 0x00046400+schedulerOffset)    /* Port DWRR Deficit - Priority 3 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio4, 0x00000000, 0x00046c00+schedulerOffset)    /* Port DWRR Deficit - Priority 4 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio5, 0x00000000, 0x00047400+schedulerOffset)    /* Port DWRR Deficit - Priority 5 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio6, 0x00000000, 0x00047c00+schedulerOffset)    /* Port DWRR Deficit - Priority 6 */
        SET_REG_ADDRESS(addressSpace.Sched.PortDefPrio7, 0x00000000, 0x00048400+schedulerOffset)    /* Port DWRR Deficit - Priority 7 */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus01, 0x00000000, 0x00065400+schedulerOffset)    /* Port RR/DWRR 01 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus23, 0x00000000, 0x00065c00+schedulerOffset)    /* Port RR/DWRR 23 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus45, 0x00000000, 0x00066400+schedulerOffset)    /* Port RR/DWRR 45 Status */
        SET_REG_ADDRESS(addressSpace.Sched.PortRRDWRRStatus67, 0x00000000, 0x00066c00+schedulerOffset)    /* Port RR/DWRR 67 Status */
    }

    cpssOsMemCpy(addressSpacePtr,&addressSpace,sizeof(struct prvCpssDxChTxqSchedAddressSpace));
}


