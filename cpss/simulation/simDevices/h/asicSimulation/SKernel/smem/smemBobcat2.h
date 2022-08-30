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
* @file smemBobcat2.h
*
* @brief Bobcat2 memory mapping implementation.
*
* @version   7
********************************************************************************
*/
#ifndef __smemBobcat2h
#define __smemBobcat2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemLion3.h>

#define  REG_PTR_AND_SIZE_MAC(regArr) \
    (GT_U32*)(void*)((((SMEM_CHT_PP_REGS_UNIT_START_INFO_STC*)(void*)&regArr) + 1)) , ((sizeof(regArr) - sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC))/sizeof(GT_U32))

/* Falcon 4 tiles need 'more' */
#define MAX_INTERRUPT_NODES    (4*500)

/* concatenation of strings : "BOBK_UNIT_MG_E" with instance '2' to create string :
    "BOBK_UNIT_MG_E_INSTANCE_2"
 */
#define ADD_INSTANCE_OF_UNIT_TO_STR(unitNameStr , instance)    \
    unitNameStr "_INSTANCE_" #instance


ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat2ActiveWriteTmHostingUnit);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat2ActiveWriteRxDmaGlobalCntrsClear);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobcat2ActiveWritePortLpiCtrlReg);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemBobcat2ActiveReadPortLpiStatusReg);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemBobcat2ActiveReadTodTimeCaptureValueFracLow);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemBobkActiveWriteRxDmaPIPDropGlobalCounterTrigger);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobkActiveWritePolicerMeteringBaseAddr);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobkActiveWritePolicerCountingBaseAddr);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemBobKActiveWriteMacXlgCtrl4);

/* LMS active write functions for Bobcat2 */
#define BOBCAT2_LMS_ACTIVE_WRITE_MEMORY(offset, writeTableFunc)      \
    ACTIVE_WRITE_MEMORY_MAC(BOBCAT2_LMS_UNIT_BASE_ADDR_CNS + offset, writeTableFunc, 0), \
    ACTIVE_WRITE_MEMORY_MAC(BOBCAT2_LMS1_UNIT_BASE_ADDR_CNS + offset, writeTableFunc, 1), \
    ACTIVE_WRITE_MEMORY_MAC(BOBCAT2_LMS2_UNIT_BASE_ADDR_CNS + offset, writeTableFunc, 2)

#define BOBCAT2_MIB_PORT_56_START_OFFSET_CNS     0x00800000
#define BOBCAT2_MIB_OFFSET_CNS                  (0x400)
#define     SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS     (GT_U32)((~(BOBCAT2_MIB_OFFSET_CNS * 0xFF)) & (~(0xFF)))

#define BOBCAT2_PORT_OFFSET_CNS              0x1000
#define BOBCAT2_PORT_0_START_OFFSET_CNS      (BOBCAT2_PORT_OFFSET_CNS * 0)
#define BOBCAT2_PORT_33_START_OFFSET_CNS     (BOBCAT2_PORT_OFFSET_CNS * 33)
#define BOBCAT2_PORT_48_START_OFFSET_CNS     (BOBCAT2_PORT_OFFSET_CNS * 48)

#define BOBCAT2_PORT_56_START_OFFSET_CNS     0x200000
#define BOBCAT2_XPCS_ADDRESS_RANGE_CNS       0x0001E8
#define BOBCAT2_XLG_MAC_ADDRESS_RANGE_CNS    0x00009C

/* the base address for non exists 'dummy' units :

   NOTE:
   1. this is 'dummy address' for simulation purposes , to be able to access to it .
   2. the CPU can not access to it !
   3. we use 'unused' memory space --> we can change it to any other unit space.
*/
#define UNIT_BASE_ADDR_MAC(unit_index) ((unit_index) << SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS)

#define BOBCAT2_FDB_TABLE_BASE_ADDR_CNS(index)              DUMMY_UNITS_BASE_ADDR_CNS(0)

#define BOBCAT2_EGF_EFT_UNIT_BASE_ADDR_CNS(index)           0x35000000 + (UNIT_BASE_ADDR_MAC(index))
#define BOBCAT2_EGF_QAG_UNIT_BASE_ADDR_CNS(index)           0x3B000000 + (UNIT_BASE_ADDR_MAC(index))
#define BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(index)           0x48000000 + (UNIT_BASE_ADDR_MAC(index))

#define BOBCAT2_HA_UNIT_BASE_ADDR_CNS(index)                0x0F000000
#define BOBCAT2_IPCL_UNIT_BASE_ADDR_CNS                     0x02000000
#define BOBCAT2_EPCL_UNIT_BASE_ADDR_CNS                     0x14000000

#define BOBCAT2_LPM_UNIT_BASE_ADDR_CNS                      0x58000000
#define BOBCAT2_TCAM_UNIT_BASE_ADDR_CNS                     0x05000000
#define BOBCAT2_TM_HOSTING_UNIT_BASE_ADDR_CNS               0x0C000000
#define BOBCAT2_EQ_UNIT_BASE_ADDR_CNS(index)                0x0D000000 + UNIT_BASE_ADDR_MAC(index)
#define BOBCAT2_MLL_UNIT_BASE_ADDR_CNS                      0x1D000000

#define BOBCAT2_NEW_GOP_UNIT_BASE_ADDR_CNS      BOBCAT2_GOP_UNIT_BASE_ADDR_CNS

#define BOBCAT2_MG_UNIT_BASE_ADDR_CNS           0x00000000
#define BOBCAT2_TTI_UNIT_BASE_ADDR_CNS          0x01000000
#define BOBCAT2_IPLR1_UNIT_BASE_ADDR_CNS        0x20000000
#define BOBCAT2_L2I_UNIT_BASE_ADDR_CNS          0x03000000
#define BOBCAT2_IPVX_UNIT_BASE_ADDR_CNS         0x06000000
#define BOBCAT2_BM_UNIT_BASE_ADDR_CNS           0x16000000
#define BOBCAT2_BMA_UNIT_BASE_ADDR_CNS          0x2D000000
#define BOBCAT2_FDB_UNIT_BASE_ADDR_CNS          0x04000000
#define BOBCAT2_MPPM_UNIT_BASE_ADDR_CNS         0x47000000
#define BOBCAT2_EPLR_UNIT_BASE_ADDR_CNS         0x0E000000
#define BOBCAT2_CNC_0_UNIT_BASE_ADDR_CNS        0x2E000000
#define BOBCAT2_CNC_1_UNIT_BASE_ADDR_CNS        0x2F000000
#define BOBCAT2_GOP_UNIT_BASE_ADDR_CNS          0x10000000
#define BOBCAT2_MIB_UNIT_BASE_ADDR_CNS          0x12000000
#define BOBCAT2_SERDES_UNIT_BASE_ADDR_CNS       0x13000000
#define BOBCAT2_IPLR0_UNIT_BASE_ADDR_CNS        0x0B000000
#define BOBCAT2_RX_DMA_UNIT_BASE_ADDR_CNS       0x17000000
#define BOBCAT2_TX_DMA_UNIT_BASE_ADDR_CNS       0x26000000
#define BOBCAT2_TXQ_QUEUE_UNIT_BASE_ADDR_CNS    0x3C000000
#define BOBCAT2_TXQ_LL_UNIT_BASE_ADDR_CNS       0x3D000000
#define BOBCAT2_TXQ_DQ_UNIT_BASE_ADDR_CNS       0x40000000
#define BOBCAT2_CPFC_UNIT_BASE_ADDR_CNS         0x3E000000

#define BOBCAT2_IOAM_UNIT_BASE_ADDR_CNS         0x1C000000
#define BOBCAT2_EOAM_UNIT_BASE_ADDR_CNS         0x1E000000
#define BOBCAT2_ERMRK_UNIT_BASE_ADDR_CNS        0x15000000

#define BOBCAT2_LMS_UNIT_BASE_ADDR_CNS          0x21000000 /* LMS0_1,2,3 0x22000000, 0x23000000, 0x24000000 */
#define BOBCAT2_LMS1_UNIT_BASE_ADDR_CNS         0x50000000 /* LMS1_1,2,3 0x51000000, 0x52000000, 0x53000000 */
#define BOBCAT2_LMS2_UNIT_BASE_ADDR_CNS         0x54000000 /* LMS2_1,2,3 0x55000000, 0x56000000, 0x57000000 */

#define BOBCAT2_TXQ_QCN_UNIT_BASE_ADDR_CNS      0x3F000000

#define BOBCAT2_TX_FIFO_UNIT_BASE_ADDR_CNS      0x27000000
#define BOBCAT2_ETH_TX_FIFO_UNIT_BASE_ADDR_CNS  0x1A000000
#define BOBCAT2_ILKN_TX_FIFO_UNIT_BASE_ADDR_CNS 0x1B000000

#define BOBCAT2_TMDROP_UNIT_BASE_ADDR_CNS       0x09000000
#define BOBCAT2_TM_FCU_UNIT_BASE_ADDR_CNS       0x08000000
#define BOBCAT2_TM_INGRESS_GLUE_UNIT_BASE_ADDR_CNS      0x18000000
#define BOBCAT2_TM_EGRESS_GLUE_UNIT_BASE_ADDR_CNS       0x19000000
#define BOBCAT2_TMQMAP_GLUE_UNIT_BASE_ADDR_CNS  0x0A000000

/* do not INIT memory for core that is not the single instance */
#define SINGLE_INSTANCE_UNIT_MAC(dev,coreId)    \
    if(dev->portGroupId != coreId)          \
    {                                       \
        return;                             \
    }
extern SMEM_REGISTER_DEFAULT_VALUE_STC bobcat2_gop_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC sip5_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC sip5_10_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC bobcat2Additional_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC bobcat2B0_gop_registersDefaultValueArr[];

/* check if active memory is empty for the unit */
#define IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr) \
    ((currUnitChunkPtr->numOfChunks != 0 && (currUnitChunkPtr->unitActiveMemPtr[0].address == END_OF_TABLE)) ? 1 : 0)

#define ACTIVE_MEM_POLICER_COMMON_MAC(policerId)   \
    /* Policers Table Access Control Register */                                                            \
    {0x00000070, SMEM_FULL_MASK_CNS,  NULL,policerId, smemXCatActiveWritePolicerTbl, policerId},             \
                                                                                                            \
    /* Policer IPFIX memories */                                                                            \
    {0x00000014, SMEM_FULL_MASK_CNS, NULL, policerId, smemXCatActiveWriteIPFixTimeStamp,policerId},         \
    /*part of Policer Timer Memory */                                                                       \
    {0x00000318, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixNanoTimeStamp  , policerId, NULL,policerId},    \
    {0x0000031C, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSecLsbTimeStamp, policerId, NULL,policerId},    \
    {0x00000320, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSecMsbTimeStamp, policerId, NULL,policerId},    \
                                                                                                            \
    {0x00000048, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
    {0x0000004C, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
                                                                                                            \
    /* PLR interrupt cause register */                                                                      \
    {0x00000200, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, policerId, smemChtActiveWriteIntrCauseReg,policerId}, \
    {0x00000204, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

/* management counters policer active memory */
#define ACTIVE_MEM_POLICER_MANAGEMENT_COUNTERS_MAC(policerId)   \
    /* Policer : when Read a Data Unit counter part(Entry LSB) of Management Counters Entry.  */            \
    /*           then value of LSB and MSB copied to Shadow registers                         */            \
    /* management counters                                                                    */            \
    /* 0x40 between sets , 0x10 between counters                                              */            \
    /* 0x00 , 0x10 , 0x20 , 0x30                                                               */            \
    /* 0x40 , 0x50 , 0x60 , 0x70                                                              */            \
    /* 0x80 , 0x90 , 0xa0 , 0xb0                                                              */            \
    {POLICER_MANAGEMENT_COUNTER_ADDR_CNS, POLICER_MANAGEMENT_COUNTER_MASK_CNS, smemXCatActiveReadPolicerManagementCounters, policerId, NULL,policerId}

/* bc2 common policer active memory */
#define ACTIVE_MEM_POLICER_BC2_COMMON_MAC(policerId)   \
    ACTIVE_MEM_POLICER_COMMON_MAC(policerId),          \
    ACTIVE_MEM_POLICER_MANAGEMENT_COUNTERS_MAC(policerId)

#define ACTIVE_MEM_MG_COMMON_MAC                                                                \
    /*MG*/                                                                                      \
    /* Global interrupt cause register */                                                       \
    {0x00000030, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteIntrCauseReg, 0},              \
                                                                                                \
    /* Receive SDMA Interrupt Cause Register (RxSDMAInt) */                                     \
    {0x0000280C, SMEM_FULL_MASK_CNS,                                                            \
        smemChtActiveReadIntrCauseReg, 22, smemChtActiveWriteIntrCauseReg, 0},                  \
    /* Receive SDMA Interrupt Mask Register */                                                  \
    {0x00002814, SMEM_FULL_MASK_CNS,                                                            \
        NULL, 0, smemChtActiveWriteRcvSdmaInterruptsMaskReg, 0},                                \
                                                                                                \
    /* The SDMA packet count registers */                                                       \
    {0x00002820, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},                                \
    {0x00002830, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},                                \
                                                                                                \
     /* The SDMA byte count registers */                                                        \
    {0x00002840, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},                                \
    {0x00002850, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},                                \
                                                                                                \
    /* The SDMA Receive SDMA Resource Error Count_And_Mode 0x00002860, 0x00002864 */            \
    {0x00002860, 0xFFFFFFF8, smemLion3SdmaRxResourceErrorCountAndModeActiveRead, 0, NULL,0},    \
                                                                                                \
    /* The SDMA Receive SDMA Resource Error Count_And_Mode 0x00002878.., 0x0000288c*/           \
    {0x00002878, 0xFFFFFFF8, smemLion3SdmaRxResourceErrorCountAndModeActiveRead, 0, NULL,0},    \
    {0x00002880, 0xFFFFFFF0, smemLion3SdmaRxResourceErrorCountAndModeActiveRead, 0, NULL,0},    \
                                                                                                \
    /* Transmit SDMA Packet Generator Config Queue */                                           \
    {0x000028B0, 0xFFFFFFF0,                                                                    \
        NULL, 0, smemLion3ActiveWriteTransSdmaPacketGeneratorConfigQueueReg, 0},                \
                                                                                                \
    /* Transmit SDMA Packet Generator Config Queue */                                           \
    {0x000028C0, 0xFFFFFFF0,                                                                    \
        NULL, 0, smemLion3ActiveWriteTransSdmaPacketGeneratorConfigQueueReg, 4/*indicates that first queue is 4*/}, \
                                                                                                \
    /* Tx SDMA Packet Count Config Queue */                                                     \
    {0x000028D0, 0xFFFFFFF0,                                                                    \
        NULL, 0, smemLion3ActiveWriteTransSdmaPacketCountConfigQueueReg, 0},                    \
                                                                                                \
    /* Tx SDMA Packet Count Config Queue */                                                     \
    {0x000028E0, 0xFFFFFFF0,                                                                    \
        NULL, 0, smemLion3ActiveWriteTransSdmaPacketCountConfigQueueReg, 0},                    \
                                                                                                \
    /* Address Update Queue Base Address */                                                     \
    {0x000000C0, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveAuqBaseWrite, 0},                   \
                                                                                                \
    /* FDB Upload Queue Base Address */                                                         \
    {0x000000C8, SMEM_FULL_MASK_CNS, NULL, 0 , smemCht2ActiveFuqBaseWrite, 0},                  \
                                                                                                \
    /* AUQHostConfiguration */                                                                  \
    {0x000000D8, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveAuqHostCfgWrite, 0},              \
                                                                                                \
    /* Receive SDMA Queue Command */                                                            \
    {0x00002680, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteSdmaCommand,0},                \
    /* Transmit SDMA Queue Command Register */                                                  \
    {0x00002868, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteStartFromCpuDma, 0},           \
                                                                                                \
    /* Global control register */                                                               \
    {0x00000058, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteGlobalReg, 0},                 \
                                                                                                \
    /* Transmit SDMA Interrupt Cause Register */                                                \
    {0x00002810, SMEM_FULL_MASK_CNS,                                                            \
        smemChtActiveReadIntrCauseReg, 21 , smemChtActiveWriteIntrCauseReg,0},                  \
    /* Transmit SDMA Interrupt Mask Register */                                                 \
    {0x00002818, SMEM_FULL_MASK_CNS,                                                            \
        NULL, 0, smemChtActiveWriteTransSdmaInterruptsMaskReg, 0},                              \
                                                                                                \
    /* read interrupts cause registers Misc -- ROC register */                                  \
    {0x00000038, SMEM_FULL_MASK_CNS,                                                            \
        smemChtActiveReadIntrCauseReg, 11, smemChtActiveWriteIntrCauseReg, 0},                  \
                                                                                                \
    /* Write Interrupt Mask MIsc Register */                                                    \
    {0x0000003c, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteMiscInterruptsMaskReg, 0},     \
                                                                                                \
    /* Write Interrupt Mask FuncUnitsIntsSum Register */                                        \
    {0x000003Fc, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveGenericWriteInterruptsMaskReg, 0},  \
    /* Read/Write Interrupt cause FuncUnitsIntsSum Register */                                  \
    {0x000003F8, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0 , smemChtActiveWriteIntrCauseReg, 0},\
                                                                                                \
    /* Write Interrupt Mask FuncUnits1IntsSum Register */                                       \
    {0x000003F0, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveGenericWriteInterruptsMaskReg, 0},  \
    /* Read/Write Interrupt cause FuncUnits1IntsSum Register */                                 \
    {0x000003F4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0 , smemChtActiveWriteIntrCauseReg, 0},\
                                                                                                \
    /* SDMA configuration register - 0x00002800 */                                              \
    {0x00002800, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteSdmaConfigReg, 0},             \
                                                                                                \
    /* XSMI Interrupt Cause Register (n=0) - 0x00030010 */                                      \
    {0x00030010, SMEM_FULL_MASK_CNS,                                                            \
            smemChtActiveReadIntrCauseReg, 12 , smemChtActiveWriteIntrCauseReg, 0},             \
    /* XSMI Interrupt Cause Register (n=1) - 0x00032010 */                                      \
    {0x00032010, SMEM_FULL_MASK_CNS,                                                            \
            smemChtActiveReadIntrCauseReg, 13 , smemChtActiveWriteIntrCauseReg, 0},             \
    /* XSMI Interrupt Mask Register (n=0) - 0x00030014 */                                       \
    {0x00030014, SMEM_FULL_MASK_CNS,                                                            \
        NULL, 0, smemChtActiveWriteTransSdmaInterruptsMaskReg, 0},                              \
    /* XSMI Interrupt Mask Register (n=1) - 0x00032014 */                                       \
    {0x00032014, SMEM_FULL_MASK_CNS,                                                            \
        NULL, 0, smemChtActiveWriteTransSdmaInterruptsMaskReg, 0},                              \
    {0x000FFFFC, SMEM_FULL_MASK_CNS,                                                            \
        NULL, 0, smemChtActiveWriteIpcCpssToWm, 0}


#define BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(n) \
    {0x00010020+0x10*(n-1), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00010024+0x10*(n-1), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

/*  EQ unit active memories without Ingress STC Interrupt Cause <n> registers */
#define ACTIVE_MEM_EQ_WITHOUT_INT_STC_INT_COMMON_MAC                                                    \
/*EQ*/                                                                                                  \
        /* SMEM_BIND_TABLE_MAC(ingrStc)                             */                                  \
        /*   the table is not of type 'write' after 'whole' words : */                                  \
        /*   word 0 - RW                                            */                                  \
        /*   word 1 - RO                                            */                                  \
        /*   word 2 - RW                                            */                                  \
        /*                                                          */                                  \
        /*   the Write to the table word 0,2 is allowed via         */                                  \
        /*    <IngressSTCW0 WrEn> , <IngressSTCW2 WrEn> in SMEM_CHT_INGRESS_STC_CONF_REG(dev) */        \
        /*                                                          */                                  \
        /*    addresses are : 0x0B040000 .. 0x0B040FFC              */                                  \
                                                                                                        \
    {0x00040000, 0xFFFFF000 ,NULL,0,smemLion3ActiveWriteIngressStcTable,0},                             \
    /*Trunk LFSR Configuration*/                                                                        \
    {0x0000D000,SMEM_FULL_MASK_CNS,NULL,0,smemLion3ActiveWriteLfsrConfig,0/*trunk*/},                   \
    /*ePort ECMP LFSR Configuration*/                                                                   \
    {0x00005020,SMEM_FULL_MASK_CNS,NULL,0,smemLion3ActiveWriteLfsrConfig,1/*eport ECMP*/},              \
                                                                                                        \
    /* ROC (read only clear) counters */                                                                \
    /*Ingress Drop Counter*/                                                                            \
    {   0x40    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*CPU Code Rate Limiter Drop Counter*/                                                              \
    {   0x68    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*Drop Codes Counter*/                                                                              \
    {   0x70    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*BM Clear Critical ECC Error Counter*/                                                             \
    {   0x78    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*BM Inc Critical ECC Error Counter*/                                                               \
    {   0x88    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*Target ePort MTU Exceeded Counter*/                                                               \
    { 0xa024    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
    /*Ingress Forwarding Restrictions Dropped Packets Counter*/                                         \
    {0x2000c    , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                               \
                                                                                                        \
    /* read only counters -- without clear ! */                                                         \
    /*TO CPU Rate Limiter<%n+1> Packet Counter --> 255 registers , but for simple mask do '256' */      \
    {0x80000    , SMEM_FULL_MASK_CNS & (~(0xFF << 2)), NULL, 0, smemChtActiveWriteToReadOnlyReg,0},     \
                                                                                                        \
    /* oamProtectionLocStatusTable */                                                                   \
    {0x00B00000, 0xFFFFFF00, NULL, 0, smemLion3ActiveWriteProtectionLocStatusTable, 0},                 \
                                                                                                        \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(1),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(2),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(3),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(4),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(5),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(6),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(7),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(8),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(9),                                                            \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(10),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(11),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(12),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(13),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(14),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(15),                                                           \
    BIND_CPU_CODE_RATE_LIMITER_INTERRUPT(16),                                                           \
    /*BMCriticalECCInterruptCause*/                                                                     \
    {0x000000a0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0 , smemChtActiveWriteIntrCauseReg,0}, \
    {0x000000a4, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}


#define ACTIVE_MEM_EQ_COMMON_MAC                                                                       \
/*EQ*/                                                                                                  \
    /*ingress STC interrupt register*/                                                                  \
    /* need to support 0x0B000620 - 16 registers in steps of 0x10 */                                    \
    /*   and Summary Cause Register in  0x0B000600                */                                    \
    /*                                                            */                                    \
    /*   AND 0x0B000610 --> not exists in reg file                */                                    \
    /*                                                            */                                    \
    /*   so 0x0B000600 , 0x0B000620 ...0x0B000720                 */                                    \
                                                                                                        \
    /* 0x0B000600 .. 0x0B0006F0 , steps of 0x10 */                                                      \
    {0x00000600, 0xFFFFFF0F, smemChtActiveReadIntrCauseReg, 24 , smemChtActiveWriteIntrCauseReg,0},     \
    /* 0x0B000700 .. 0x0B000710 , steps of 0x10 */                                                      \
    {0x00000700, 0xFFFFFFEF, smemChtActiveReadIntrCauseReg, 24 , smemChtActiveWriteIntrCauseReg,0},     \
    /* rest of EQ active memories */                                                                    \
    ACTIVE_MEM_EQ_WITHOUT_INT_STC_INT_COMMON_MAC


#define ACTIVE_MEM_CNC_COMMON_MAC                                                                           \
    /* CNC Fast Dump Trigger Register Register */                                                           \
    {0x00000030,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncFastDumpTrigger, 0},                    \
                                                                                                            \
    /* CPU direct read from the counters */                                                                 \
    {0x00010000,0xFFFF0000, smemCht3ActiveCncBlockRead, 0x00010000/*base address of block 0*/, NULL, 0},    \
    {0x00020000,0xFFFF0000, smemCht3ActiveCncBlockRead, 0x00010000/*base address of block 0*/, NULL, 0},    \
                                                                                                            \
    /* CNC Block Wraparound Status Register */                                                              \
    {0x00001400,0xFFFFFE00, smemCht3ActiveCncWrapAroundStatusRead, 0, NULL, 0},                             \
                                                                                                            \
    /* read interrupts cause registers CNC -- ROC register */                                               \
    {0x00000100,SMEM_FULL_MASK_CNS,smemChtActiveReadIntrCauseReg, 3, smemChtActiveWriteIntrCauseReg, 0},    \
                                                                                                            \
    /* Write Interrupt Mask CNC Register */                                                                 \
    {0x00000104,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncInterruptsMaskReg, 0}

/* LMS */
#define ACTIVE_MEM_LMS_COMMON_MAC(lmsId)                                            \
    /* SMI0 Management Register */                                                  \
    {0x00004054    , SMEM_FULL_MASK_CNS , NULL, lmsId, smemChtActiveWriteSmi,lmsId},\
    /* SMI2 Management Register */                                                  \
    {0x02004054    , SMEM_FULL_MASK_CNS , NULL, lmsId, smemChtActiveWriteSmi,lmsId}


#define TXQ_QUEQUE_EGR_PACKET_COUNTERS_CLEAR_ON_READ_ACTIVE_MEM_MAC \
    {0x00093210, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093220, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093230, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093240, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093250, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093260, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093270, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093280, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},    \
    {0x00093290, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0}

#define TXQ_QUEQUE_ACTIVE_MEM_MAC \
    {0x00090008, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00090040, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00090048, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00090050, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0009000C, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}, \
    {0x00090044, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}, \
    {0x0009004C, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}, \
    {0x00090054, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define HA_ACTIVE_MEM_MAC                                                      \
/* HA */                                                                       \
    /* HA Interrupt Cause Register */                                          \
    {0x00000300, SMEM_FULL_MASK_CNS,                                           \
        smemChtActiveReadIntrCauseReg, 12, smemChtActiveWriteIntrCauseReg, 0}, \
    /* HA Interrupt Mask Register */                                           \
    {0x00000304, SMEM_FULL_MASK_CNS,                                           \
        NULL, 0, smemLion3ActiveWriteHaInterruptsMaskReg, 0},                  \
                                                                               \
    /* ROC (read only clear) counters */                                       \
    /*Nat Exception Drop Counter*/                                             \
    { 0x00000314, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}


#define  MLL_ACTIVE_MEM_MAC                                                    \
/*MLL*/                                                                        \
    /* MLL counters */                                                         \
    /*L2MLLValidProcessedEntriesCntr[0]*/                                      \
    {0xe20       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLValidProcessedEntriesCntr[1]*/                                      \
    {0xe24       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLValidProcessedEntriesCntr[2]*/                                      \
    {0xe28       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLSilentDropCntr*/                                                    \
    {0x00000e00  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLSkippedEntriesCntr*/                                                \
    {0x00000e04  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLTTLExceptionCntr*/                                                  \
    {0x00000e08  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLOutMcPktsCntr[0]*/                                                  \
    {0xc00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLOutMcPktsCntr[1]*/                                                  \
    {0xd00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLLSilentDropCntr*/                                                      \
    {0x00000800  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLLMCFIFODropCntr*/                                                      \
    {0x00000804  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLSkippedEntriesCntr*/                                                \
    {0x00000b04  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLOutMcPktsCntr[0]*/                                                  \
    {0x900       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLOutMcPktsCntr[1]*/                                                  \
    {0xa00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLL Interrupt Cause Registers*/                                          \
    {0x00000030, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00000034, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}


#define  L2I_ACTIVE_MEM_MAC                                                                 \
/*L2I*/                                                                                     \
    /* Bridge Interrupt Cause Register */                                                   \
    {0x00002100, SMEM_FULL_MASK_CNS,                                                        \
        smemChtActiveReadIntrCauseReg, 14, smemChtActiveWriteIntrCauseReg, 0},              \
    /* Bridge Interrupt Mask Register */                                                    \
    {0x00002104, SMEM_FULL_MASK_CNS,                                                        \
        NULL, 0, smemChtActiveWriteBridgeInterruptsMaskReg, 0},                             \
                                                                                            \
    /* Security Breach Status Register 3 */                                                 \
    {0x0000140C, SMEM_FULL_MASK_CNS, smemChtActiveReadBreachStatusReg, 0, NULL, 0},         \
                                                                                            \
    /*hostIncomingPktsCount;               */                                               \
    {0x00001510, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*hostOutgoingPktsCount;               */                                               \
    {0x00001514, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*hostOutgoingMcPktCount;              */                                               \
    {0x00001520, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*hostOutgoingBcPktCount;              */                                               \
    {0x00001524, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*matrixSourceDestinationPktCount;     */                                               \
    {0x00001528, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*setVLANIngrFilteredPktCount[n]*/                                                      \
    {0x0000153c, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    {0x0000155c, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*setIncomingPktCount[n]        */                                                      \
    {0x00001538, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    {0x00001558, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*setSecurityFilteredPktCount[n]*/                                                      \
    {0x00001540, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    {0x00001560, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    /*setBridgeFilteredPktCount[n]  */                                                      \
    {0x00001544, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},                    \
    {0x00001564, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}



GT_VOID smemBobcat2InterruptTreeAddNode(
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr,
    IN    SKERNEL_INTERRUPT_REG_INFO_STC * currNodePtr,
    IN    GT_U32 nodeCount
);

/* Macro increment index before nodes insertion and checks index range for current interrupt */
#define INTERRUPT_TREE_ADD_NODE_WITH_CHECK(index, dbPtrPtr, nodeArr) \
    INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, dbPtrPtr, nodeArr , sizeof(nodeArr)/sizeof(SKERNEL_INTERRUPT_REG_INFO_STC))

/* for specific number on elements :
   Macro increment index before nodes insertion and checks index range for current interrupt */
#define INTERRUPT_TREE_ADD_X_NODES_WITH_CHECK(index, dbPtrPtr, nodeArr , IN_numNodes) \
    { \
        GT_U32 nodesNum = sizeof(nodeArr)/sizeof(SKERNEL_INTERRUPT_REG_INFO_STC); \
        if((IN_numNodes) < nodesNum){nodesNum = (IN_numNodes);}                   \
        if(nodesNum == 0){nodesNum = 1;} /*support pointer to single node */      \
        index += nodesNum; \
        if (index >= MAX_INTERRUPT_NODES) \
            skernelFatalError("Interrupt index node is out of range: index[%d]", index); \
        smemBobcat2InterruptTreeAddNode(dbPtrPtr, nodeArr, nodesNum); \
    }

#define INTERRUPT_TREE_ADD_SINGLE_NODE_WITH_CHECK(index, dbPtrPtr, nodeArr) \
    { \
        index ++; \
        if (index >= MAX_INTERRUPT_NODES) \
            skernelFatalError("Interrupt index node is out of range: index[%d]", index); \
        smemBobcat2InterruptTreeAddNode(dbPtrPtr, nodeArr, 1); \
    }



GT_VOID smemBobcat2GlobalInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    GT_U32 *indexPtr,
    INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
);

/**
* @internal smemBobcat2Init function
* @endinternal
*
* @brief   Init memory module for a Bobcat device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemBobcat2Init
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemBobcat2Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobcat2Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobkInit function
* @endinternal
*
* @brief   Init memory module for a bobk device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemBobkInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemBobkAldrinInit function
* @endinternal
*
* @brief   Init memory module for a bobk-aldrin device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemBobkAldrinInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemBobkInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobkAldrinInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkAldrinInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

void smemBobcat2SpecificDeviceMemInitPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobcat2DoPortInterrupt function
* @endinternal
*
* @brief   Set port interrupt
*/
void smemBobcat2DoPortInterrupt(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   macPort,
    IN GT_U32                   intIndex
);


/**
* @internal smemBobcat2UnitEthTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobcat2 Eth TX Fifo unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
void smemBobcat2UnitEthTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
);


/**
* @internal smemBobkPolicerTablesSupport function
* @endinternal
*
* @brief   manage Policer tables:
*         1. iplr 0 - all tables
*         2. iplr 1 - none
*         3. eplr  - other names for meter+counting
* @param[in] numOfChunks              - pointer to device object.
*                                      unitPtr - pointer to the unit chunk
* @param[in] plrUnit                  - PLR unit
*/
void smemBobkPolicerTablesSupport
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  numOfChunks,
    INOUT SMEM_CHUNK_BASIC_STC  chunksMem[],
    IN SMEM_SIP5_PP_PLR_UNIT_ENT   plrUnit
);

/**
* @internal smemBobcat2ErrataCleanUp function
* @endinternal
*
* @brief   Clean up erratum for Bobcat2 device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobcat2ErrataCleanUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobcat2RxDmaUnitIndexFromAddressGet function
* @endinternal
*
* @brief   get the RxDma unit index from an address within this unit.
*
* @param[in] devObjPtr                - pointer to device object.
*/
GT_U32 smemBobcat2RxDmaUnitIndexFromAddressGet (
    IN  SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN  GT_U32                 address
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemBobcat2h */

