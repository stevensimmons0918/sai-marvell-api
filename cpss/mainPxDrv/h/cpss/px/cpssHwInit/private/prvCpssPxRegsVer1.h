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
* @file prvCpssPxRegsVer1.h
*
* @brief This file includes the declaration of the structure to hold the
* addresses of Pipe PP registers for each different PP type.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxRegsVer1h
#define __prvCpssPxRegsVer1h

#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Check if 'offsetof' is defined by compiler and if not, define it */
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ( (GT_U32) ( (GT_UINTPTR) (&((TYPE *)0)->MEMBER) ) )
#endif

/* 16 MAC ports */
#define PRV_CPSS_PX_GOP_PORTS_NUM_CNS       16
/* 17 DMA ports */
#define PRV_CPSS_PX_DMA_PORTS_NUM_CNS       17
/* 17 ports */
#define PRV_CPSS_PX_PORTS_NUM_CNS           17
/* 16 SERDESes */
#define PRV_CPSS_PX_SERDES_NUM_CNS          16
/* CPU DMA port number */
#define PRV_CPSS_PX_CPU_DMA_NUM_CNS         16

/* number of lanes in the XPCS */
#define PRV_CPSS_PX_NUM_LANES_CNS           6

/* 8 shared pools */
#define PRV_CPSS_PX_SHARED_POOL_NUM_CNS     8


/* number of registers that needed for 'x' ports in register */
#define PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(numPortsInReg) ((PRV_CPSS_PX_PORTS_NUM_CNS + ((numPortsInReg)-1))/(numPortsInReg))

/* 2 CNC blocks in single CNC unit */
#define PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS  2

/* 2 DQ2 each 10 ports */
#define PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS        10
#define PRV_CPSS_PX_NUM_DQ_CNS                  2
#define PRV_CPSS_PX_MAX_DQ_PORTS_CNS        (PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS * PRV_CPSS_PX_NUM_DQ_CNS)

typedef struct PRV_CPSS_PX_PP_DP_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[85];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_PX_PP_DP_PIZZA_ARBITER_STC;


/*--------------------------------------
 * /Cider/EBU/BobK/BobK {Current}/Switching Core/MPPM/Units/<MPPM_IP> MPPM IP Units/<Pizza_arb> SIP_COMMON_MODULES_IP Units/Pizza Arbiter/Pizza Arbiter Configuration Register %n
 *    Register CPU 0x47000200 Pizza Arbiter Control Register
 *    Register CPU 0x47000204 Pizza Arbiter Status
 *    Register CPU 0x47000208 Pizza Arbiter Configuration Register %n (0x47000208 + n*0x4 : where n (0-92) represents reg number)
 *    Register CPU 0x47000400 Pizza Arbiter debug register (Internal)
 *-----------------------------------------*/
typedef struct PRV_CPSS_PX_PP_MPPM_PIZZA_ARBITER_STCT
{
    GT_U32    pizzaArbiterConfigReg[93];
    GT_U32    pizzaArbiterCtrlReg;
    GT_U32    pizzaArbiterStatusReg;
}PRV_CPSS_PX_PP_MPPM_PIZZA_ARBITER_STC;

typedef struct
{
    GT_U32    LLFCDatabase[8]/*reg*/;
    GT_U32    portSpeedTimer[8]/*reg*/;
    GT_U32    lastSentTimer[8]/*reg*/;
    GT_U32    FCACtrl;
    GT_U32    DBReg0;
    GT_U32    DBReg1;
    GT_U32    LLFCFlowCtrlWindow0;
    GT_U32    LLFCFlowCtrlWindow1;
    GT_U32    MACDA0To15;
    GT_U32    MACDA16To31;
    GT_U32    MACDA32To47;
    GT_U32    MACSA0To15;
    GT_U32    MACSA16To31;
    GT_U32    MACSA32To47;
    GT_U32    L2CtrlReg0;
    GT_U32    L2CtrlReg1;
    GT_U32    DSATag0To15;
    GT_U32    DSATag16To31;
    GT_U32    DSATag32To47;
    GT_U32    DSATag48To63;
    GT_U32    PFCPriorityMask;
    GT_U32    sentPktsCntrLSb;
    GT_U32    sentPktsCntrMSb;
    GT_U32    periodicCntrLSB;
    GT_U32    periodicCntrMSB;

}PRV_CPSS_PX_PORT_FCA_REGS_ADDR_STC;


typedef struct
{
    GT_U32 serdesExternalReg1; /* Serdes External Configuration 1   */
    GT_U32 serdesExternalReg2; /* Serdes External Configuration 2   */
    GT_U32 serdesExternalReg3; /* Serdes External Configuration 3   */
}PRV_CPSS_PX_SERDES_REGS_ADDR_STC;



/**
* @struct PRV_CPSS_PX_PORT_MAC_REGS_ADDR_STC
 *
 * @brief A struct including usefull mac configuration registers
 * addresses - used for cases when port has multiple MAC
 * units, like Tri-Speed, XG, XLG etc.
*/
typedef struct{

    /** MAC Control Register or Register0 */
    GT_U32 macCtrl;

    /** MAC Control Register1 */
    GT_U32 macCtrl1;

    /** MAC Control Register2 */
    GT_U32 macCtrl2;

    /** MAC Control Register3 */
    GT_U32 macCtrl3;

    /** MAC Control Register4 */
    GT_U32 macCtrl4;

    GT_U32 macCtrl5;

    /** Misc Configuration Register */
    GT_U32 miscConfig;

    /** FC DSA tag Register2 */
    GT_U32 fcDsaTag2;

    /** @brief FC DSA tag Register3
     *  Notes:
     */
    GT_U32 fcDsaTag3;

    GT_U32 macIntMask;

} PRV_CPSS_PX_PORT_MAC_REGS_ADDR_STC;

typedef struct
{
    GT_U32 macStatus;           /* MAC Status Register      */
    GT_U32 macCounters;         /* MAC counters register.   */
    GT_U32 macCaptureCounters;   /* MAC capture port counters register.   */
    GT_U32 autoNegCtrl;         /* AutoNeg Control Register */
    GT_U32 serdesCnfg;          /* Serdes Configuration Register 0 */
    GT_U32 serdesCnfg2;         /* Serdes Configuration Register 2 */
    GT_U32 serdesCnfg3;         /* Serdes Configuration Register 3 */
    GT_U32 xgGlobalConfReg0;    /* XG Global Configuration Register 0 */
    GT_U32 xgGlobalConfReg1;    /* XG Global Configuration Register 1 */
    GT_U32 xgXpcsGlobalStatus;  /* XG XPCS Global Status Register */
    GT_U32 xpcsGlobalMaxIdleCounter;    /* XPCS Global Max Idle Counter Register */
    GT_U32 metalFix;            /* Metal Fix Register */
    GT_U32 miscConfig;          /* Misc Configuration Register */
    GT_U32 xgMibCountersCtrl;   /* XG MIB Counters Control Register - Cheetah3 only */
    GT_U32 mibCountersCtrl;     /* MIB Counters Control Register */
    GT_U32 serialParameters;    /* Serial Parameters Register */
    GT_U32 serialParameters1;   /* Serial Parameters Register 1 - Xcat2 only */
    GT_U32 prbsCheckStatus;     /* PRBS Check Status register */
    GT_U32 prbsErrorCounter;    /* PRBS Error Counter Register */

    PRV_CPSS_PX_PORT_MAC_REGS_ADDR_STC macRegsPerType[PRV_CPSS_PORT_NOT_APPLICABLE_E]; /* array of mac registers
                                                                                addresses per available port types */
    GT_U32 xlgMibCountersCtrl; /* XLG MIB Counters Control Register */

    GT_U32 mpcs40GCommonStatus;

    GT_U32  xlgPortFIFOsThresholdsConfig; /* Port FIFOs Thresholds Configuration */
    GT_U32  gePortFIFOConfigReg0; /* Port FIFO Configuration Register 0 */
    GT_U32  gePortFIFOConfigReg1; /* Port FIFO Configuration Register 1 */

    /* EEE support */
    GT_U32 eeeLpiControl[3];/*0,1,2*/
    GT_U32 eeeLpiStatus;
    GT_U32 eeeLpiCounter;

    GT_U32 xlgDicPpmIpgReduce; /* XLG MAC IP - DIC PPM IPG Reduce Register */

    /* lane registers */
    GT_U32 laneConfig0[PRV_CPSS_PX_NUM_LANES_CNS];      /* Lane Configuration Register0 */
    GT_U32 laneConfig1[PRV_CPSS_PX_NUM_LANES_CNS];      /* Lane Configuration Register1 */
    GT_U32 disparityErrorCounter[PRV_CPSS_PX_NUM_LANES_CNS];/* Disparity Error Counter Register */
    GT_U32 prbsErrorCounterLane[PRV_CPSS_PX_NUM_LANES_CNS];/* PRBS Error Counter Register */
    GT_U32 laneStatus[PRV_CPSS_PX_NUM_LANES_CNS];       /* Lane Status Register */
    GT_U32 cyclicData[PRV_CPSS_PX_NUM_LANES_CNS];       /* Cyclic Data First Register*/

}PRV_CPSS_PX_PP_REGS_PER_PORT_REGS;

/*
 * Typedef: struct PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC
 *
 * Description: A struct including PP registers addresses for register access
 *              by different types of PPs.
 *
 * Fields: The struct includes fields by the names of PP registers, each
 *         field represents the respective register address.
 *
 * Notes:
 *          format of structure is similar to PRV_CPSS_PX_PP_REGS_ADDR_STC
 *
 */
/* This registers DB struct is used in several generic algorithm like registers
   dump. These algorithms require that this struct and PRV_CPSS_BOOKMARK_STC
   struct have packed members.
   Use 4 byte alignment to guaranty that nextPtr will be right after size in
   both this structure and in the PRV_CPSS_BOOKMARK_STC.
*/
#pragma pack(push, 4)
typedef struct /*PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC*/
{
    struct /*PCP*/{
        struct /*PTP*/{

            GT_U32    PCPPTPTypeKeyTableUDPAndPort_profileEntry[32]/*entry_num*/;
            GT_U32    PCPPortPTPTypeKeyUDBPConfigPort[2][17]/*UDB_num*//*port_num*/;
            GT_U32    PCPPortPTPTypeKeyUDBConfigPort[6][17]/*UDB_num*//*port_num*/;
            GT_U32    PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[32]/*entry_num*/;
            GT_U32    PCPPTPTypeKeyTableUDBPEntryMask[2][32]/*udbp_num*//*entry_num*/;
            GT_U32    PCPPTPTypeKeyTableUDBPEntry[2][32]/*udbp_num*//*entry_num*/;
            GT_U32    PCPPTPTypeKeyTableUDBEntryMask[6][32]/*udb_num*//*entry_num*/;
            GT_U32    PCPPTPTypeKeyTableUDBEntry[6][32]/*udb_num*//*entry_num*/;
            GT_U32    PCPPTPTypeKeyTableEntriesEnable;

        }PTP;

        struct /*PRS*/{

            GT_U32    PCPTPIDTable[4]/*reg_num*/;
            GT_U32    PCPPortPktTypeKeyTableExt[17]/*reg_num*/;
            GT_U32    PCPPortPktTypeKeyTable[17]/*reg_num*/;
            GT_U32    PCPPortDaOffsetConfig[17]/*reg_num*/;
            GT_U32    PCPPktTypeKeyUDBPMissValue[4]/*udbp_num*/;
            GT_U32    PCPPktTypeKeyTPIDEtherTypeMissValue;
            GT_U32    PCPPktTypeKeyTableUDBPEntryMask[32][4]/*reg_num*//*udbp_num*/;
            GT_U32    PCPPktTypeKeyTableUDBPEntry[32][4]/*entry_num*//*udbp_num*/;
            GT_U32    PCPPktTypeKeyTableTPIDEtherTypeEntryMask[32]/*reg_num*/;
            GT_U32    PCPPktTypeKeyTableTPIDEtherTypeEntry[32]/*reg_num*/;
            GT_U32    PCPPktTypeKeyTableSRCPortProfileEntry[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableMACDA4MSBEntryMask[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableMACDA4MSBEntry[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableMACDA2LSBEntryMask[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableMACDA2LSBEntry[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableIsLLCNonSnapEntry[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableEntriesEnable;
            GT_U32    PCPPktTypeKeyTableSRCPortProfileEntryMask[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeyTableIsLLCNonSnapEntryMask[32]/*entry_num*/;
            GT_U32    PCPPktTypeKeySRCPortProfileMissValue;
            GT_U32    PCPPktTypeKeyMACDA4MSBMissValue;
            GT_U32    PCPPktTypeKeyMACDA2LSBMissValue;
            GT_U32    PCPPktTypeKeyLookUpMissDetected;
            GT_U32    PCPPktTypeKeyLookUpMissCntr;
            GT_U32    PCPPktTypeKeyIsLLCNonSnapMissValue;
            GT_U32    PCPGlobalEtherTypeConfig4;
            GT_U32    PCPGlobalEtherTypeConfig3;
            GT_U32    PCPGlobalEtherTypeConfig2;
            GT_U32    PCPGlobalEtherTypeConfig1;
            GT_U32    PCPGlobalEtherTypeConfig0;
            GT_U32    PCPECNEnableConfig;
            GT_U32    PCPPacketTypeKeyTableIP2MEMatchIndexEntry[32]/*entry_num*/;
            GT_U32    PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[32]/*entry_num*/;

        }PRS;

        struct /*PORTMAP*/{

            GT_U32    PCPPktTypeDestinationFormat0Entry[32]/*reg_num*/;
            GT_U32    PCPPktTypeDestinationFormat1Entry[32]/*reg_num*/;
            GT_U32    PCPPortsEnableConfig;
            GT_U32    PCPPktTypeSourceFormat2Entry[32]/*reg_num*/;
            GT_U32    PCPPktTypeSourceFormat1Entry[32]/*reg_num*/;
            GT_U32    PCPPktTypeSourceFormat0Entry[32]/*reg_num*/;
            GT_U32    PCPPktTypeDestinationFormat2Entry[32]/*reg_num*/;
            GT_U32    PCPDstIdxExceptionCntr;
            GT_U32    PCPSrcIdxExceptionCntr;
            GT_U32    PCPLagDesignatedPortEntry[256]/*entry_num*/;
            GT_U32    PCPSRCExceptionForwardingPortMap;
            GT_U32    PCPDSTExceptionForwardingPortMap;
            GT_U32    PCPPacketTypeEnableEgressPortFiltering;
            GT_U32    PCPPacketTypeEnableIngressPortFiltering;

        }PORTMAP;

        struct /*HASH*/{

            GT_U32    PCPPktTypeHashMode2Entry[9]/*reg_num*/;
            GT_U32    PCPLFSRSeed[2]/*reg_num*/;
            GT_U32    PCPPktTypeHashMode1Entry[9]/*reg_num*/;
            GT_U32    PCPPktTypeHashMode0Entry[9]/*reg_num*/;
            GT_U32    PCPPktTypeHashConfigUDBPEntry[21][9]/*UDBP_num*//*entry_num*/;
            GT_U32    PCPPktTypeHashConfigIngrPort[9]/*entry_num*/;
            GT_U32    PCPHashPktTypeUDE1EtherType;
            GT_U32    PCPHashPktTypeUDE2EtherType;
            GT_U32    PCPHashCRC32Seed;
            GT_U32    PCPLagTableIndexMode;
            GT_U32    PCPPrngCtrl;
            GT_U32    PCPPacketTypeLagTableNumber;

        }HASH;

        struct /*general*/{

            GT_U32    PCPInterruptsCause;
            GT_U32    PCPInterruptsMask;
            GT_U32    PCPDebug;
            GT_U32    PCPGlobalConfiguration;

        }general;

        struct /*COS*/{

            GT_U32    PCPPortL2CoSMapEntry[17][16]/*port num*//*entry num*/;
            GT_U32    PCPPortCoSAttributes[17]/*port_num*/;
            GT_U32    PCPMPLSCoSMapEntry[8]/*entry num*/;
            GT_U32    PCPL3CoSMapEntry[64]/*entry num*/;
            GT_U32    PCPDsaCoSMapEntry[128]/*entry num*/;
            GT_U32    PCPCoSFormatTableEntry[32]/*entry_num*/;

        }COS;

        struct /*IP2ME*/{

            GT_U32    PCPIP2MEIpPart[4][7]/*word index*//*entry_num*/;
            GT_U32    PCPIP2MEControlBitsEntry[7]/*entry_num*/;

        }IP2ME;

    }PCP;

    struct /*MCFC*/{
        struct /*interrupts*/{

            GT_U32    MCFCInterruptsCause;
            GT_U32    MCFCInterruptsMask;

        }interrupts;

        struct /*cntrs*/{

            GT_U32    PFCReceivedCntr;
            GT_U32    MCReceivedCntr;
            GT_U32    UCReceivedCntr;
            GT_U32    QCNReceivedCntr;
            GT_U32    MCQCNReceivedCntr;
            GT_U32    outgoingQCNPktsCntr;
            GT_U32    outgoingMcPktsCntr;
            GT_U32    outgoingQCN2PFCMsgsCntr;
            GT_U32    outgoingMCQCNPktsCntr;
            GT_U32    outgoingPFCPktsCntr;
            GT_U32    outgoingUcPktsCntr;
            GT_U32    ingrDropCntr;

        }cntrs;

        struct /*config*/{

            GT_U32    QCNMCPortmapForPort[17]/*reg_num*/;
            GT_U32    PFCConfig;
            GT_U32    QCNGlobalConfig;
            GT_U32    portsQCN2PFCEnableConfig;
            GT_U32    portsQCNEnableConfig;
            GT_U32    UCMCArbiterConfig;
            GT_U32    ingrTrafficTrigQCNArbiterConfig;

        }config;

    }MCFC;

    struct /*PHA*/{
        struct /*pha_regs*/{

            GT_U32    PHACtrl;
            GT_U32    PHAStatus;
            GT_U32    portInvalidCRCMode;
            GT_U32    egrTimestampConfig;
            GT_U32    PHAGeneralConfig;
            GT_U32    PHATablesSERCtrl;
            GT_U32    PHAInterruptSumCause;
            GT_U32    PHASERErrorCause;
            GT_U32    PHASERErrorMask;
            GT_U32    PHAInternalErrorCause;
            GT_U32    PHAInternalErrorMask;
            GT_U32    headerSERCtrl;
            GT_U32    PHAInterruptSumMask;

        }pha_regs;

        struct /*PPA*/{

            struct /*ppa_regs*/{

                GT_U32    PPACtrl;
                GT_U32    PPAInterruptSumCause;
                GT_U32    PPAInterruptSumMask;
                GT_U32    PPAInternalErrorCause;
                GT_U32    PPAInternalErrorMask;

            }ppa_regs;

        }PPA;

        struct /*PPG*/{

            struct /*ppg_regs*/{

                GT_U32    PPGInterruptSumMask;
                GT_U32    PPGInterruptSumCause;
                GT_U32    PPGInternalErrorMask;
                GT_U32    PPGInternalErrorCause;

            }ppg_regs;

            /* manually added */
            GT_U32      PPG_IMEM_base_addr;/* base address of this memory (4 words per entry) */

        }PPG[4]/*PPG*/;

        struct /*PPN*/{

            struct /*ppn_regs*/{

                GT_U32    PPNDoorbell;
                GT_U32    pktHeaderAccessOffset;
                GT_U32    PPNInternalErrorCause;
                GT_U32    PPNInternalErrorMask;

            }ppn_regs;

        }PPN[8][4]/*PPN*//*PPG*/;

    }PHA;

    struct /*rxDMA*/{

        struct /*singleChannelDMAConfigs*/{

            GT_U32    SCDMAConfig0[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*Port*/;
            GT_U32    SCDMAConfig1[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*Port*/;
            GT_U32    SCDMACTPktIndentification[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*port*/;

        }singleChannelDMAConfigs;

        struct /*globalRxDMAConfigs*/{

            struct /*rejectConfigs*/{

                GT_U32    rejectPktCommand;
                GT_U32    rejectCPUCode;

            }rejectConfigs;

            struct /*globalConfigCutThrough*/{

                GT_U32    CTEtherTypeIdentification;
                GT_U32    CTUPEnable;

            }globalConfigCutThrough;

            struct /*buffersLimitProfiles*/{

                GT_U32    portsBuffersLimitProfileXONXOFFConfig[8];
                GT_U32    portsBuffersLimitProfileRxlimitConfig[8];
            }buffersLimitProfiles;

            struct /*preIngrPrioritizationConfStatus*/{

                GT_U32    VLANUPPriorityArrayProfile[4]/*profile*/;
                GT_U32    MPLSEtherTypeConf[2]/*MPLS*/;
                GT_U32    VIDConf[4]/*index*/;
                GT_U32    genericEtherType[4]/*index*/;
                GT_U32    qoSDSAPriorityArrayProfileSegment[4][8]/*profile*//*segment*/;
                GT_U32    PIPPriorityDropGlobalCntrsClear;
                GT_U32    PIPVeryHighPriorityDropGlobalCntrLsb;
                GT_U32    PIPVeryHighPriorityDropGlobalCntrMsb;
                GT_U32    PIPMedPriorityDropGlobalCntrMsb;
                GT_U32    PIPMedPriorityDropGlobalCntrLsb;
                GT_U32    PIPLowPriorityDropGlobalCntrMsb;
                GT_U32    VLANEtherTypeConf[4]/*index*/;
                GT_U32    MACDA2MsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPHighPriorityDropGlobalCntrLsb;
                GT_U32    MPLSEXPPriorityArrayProfile[4]/*profile*/;
                GT_U32    UPDSATagArrayProfile[4]/*profile*/;
                GT_U32    MACDAPriorityArrayProfile[4]/*profile*/;
                GT_U32    MACDAMask4LsbConf[4]/*mask*/;
                GT_U32    MACDAMask2MsbConf[4]/*mask*/;
                GT_U32    MACDA4LsbBytesConf[4]/*MAC DA conf*/;
                GT_U32    PIPLowPriorityDropGlobalCntrLSb;
                GT_U32    IPv6TCPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv6EtherTypeConf;
                GT_U32    IPv4ToSPriorityArrayProfileSegment[4][16]/*profile*//*segment*/;
                GT_U32    IPv4EtherTypeConf;
                GT_U32    PIPHighPriorityDropGlobalCntrMsb;
                GT_U32    PIPDropGlobalCounterTrigger;
                GT_U32    etherTypePriorityArrayProfile[4]/*profile*/;

            }preIngrPrioritizationConfStatus;

        }globalRxDMAConfigs;

        struct /* SCDMA Debug*/{
            GT_U32 portBuffAllocCnt[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*Port*/;
        }scdmaDebug;

        struct /*SIP_COMMON_MODULES*/{
            PRV_CPSS_PX_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }SIP_COMMON_MODULES;

        struct /* debug*/{

            struct /* counters_and_counters_statuses */{

                GT_U32    currentNumOfPacketsStoredInMppm;

            }countersAndCountersStatuses;

        }debug;

        struct /*singleChannelDMAPip*/ {
            GT_U32    SCDMA_n_PIP_Config      [PRV_CPSS_PX_DMA_PORTS_NUM_CNS];
            GT_U32    SCDMA_n_Drop_Pkt_Counter[PRV_CPSS_PX_DMA_PORTS_NUM_CNS];
        }singleChannelDMAPip;

    }rxDMA;

    struct /*txDMA*/{

        struct /*txDMAThresholdsConfigs*/{

            GT_U32    RXDMAUpdatesFIFOsThresholds[1]/*rxdma*/;
            GT_U32    clearFIFOThreshold;
            GT_U32    headerReorderFifoThreshold;
            GT_U32    payloadReorderFifoThreshold;

        }txDMAThresholdsConfigs;

        struct /*txDMAPerSCDMAConfigs*/{

            GT_U32    burstLimiterSCDMA              [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    FIFOsThresholdsConfigsSCDMAReg1[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOCntrsConfigsSCDMA        [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs                   [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs1                  [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMAConfigs2                  [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA reg2*/;
            GT_U32    SCDMAMetalFixReg               [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    SCDMARateLimitResidueVectorThr [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
        }txDMAPerSCDMAConfigs;

        struct /*txDMAGlobalConfigs*/{

            GT_U32    txDMAGlobalConfig1;
            GT_U32    txDMAGlobalConfig3;
            GT_U32    ingrCoreIDToMPPMMap;
            GT_U32    TXDMAMetalFixReg;
            GT_U32    srcSCDMASpeedTbl[3];
            GT_U32    localDevSrcPort2DMANumberMap[8];
        }txDMAGlobalConfigs;

        struct /*txDMADescCriticalFieldsECCConfigs*/{

            GT_U32    dataECCConfig;
            GT_U32    dataECCStatus;

        }txDMADescCriticalFieldsECCConfigs;

        struct /*txDMADebug*/{

            struct /*informativeDebug*/{

                GT_U32    nextBCFIFOFillLevel[1]/*core*/;
                GT_U32    nextBCFIFOMaxFillLevel[1]/*core*/;
                GT_U32    SCDMAStatusReg1          [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2          [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOFillLevel   [PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAPrefFIFOMaxFillLevel[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    descOverrunReg1;
                GT_U32    descOverrunReg2;
                GT_U32    descOverrunReg3;
                GT_U32    clearFIFOFillLevel;
                GT_U32    clearFIFOMaxFillLevel;
                GT_U32    headerReorderFIFOFillLevel;
                GT_U32    headerReorderFIFOMaxFillLevel;
                GT_U32    payloadReorderFIFOFillLevel;
                GT_U32    payloadReorderFIFOMaxFillLevel;
                GT_U32    descIDFIFOFillLevel;
                GT_U32    descIDFIFOMaxFillLevel;
                GT_U32    debugBusCtrlReg;
                GT_U32    txDMADebugBus;
                GT_U32    incomingDescsCntrLsb;
                GT_U32    incomingDescsCntrMsb;
                GT_U32    outgoingDescsCntrLsb;
                GT_U32    outgoingDescsCntrMsb;
                GT_U32    outgoingClearRequestsCntrLsb;
                GT_U32    outgoingClearRequestsCntrMsb;
                GT_U32    MPPMReadRequestsCntr;
                GT_U32    MPPMReadAnswersCntrLsb;
                GT_U32    MPPMReadAnswersCntrMsb;
                GT_U32    outgoingPayloadsCntrLsb;
                GT_U32    outgoingPayloadsCntrMsb;
                GT_U32    outgoingPayloadsEOPCntrLsb;
                GT_U32    outgoingPayloadsEOPCntrMsb;
                GT_U32    outgoingHeadersEOPCntrLsb;
                GT_U32    outgoingHeadersEOPCntrMsb;
                GT_U32    outgoingCTBCUpdatesCntr;
                GT_U32    HADescIDDBFreeIDFIFOFillLevel;
                GT_U32    internalDescIDDBFreeIDFIFOFillLevel;
                GT_U32    headerReorderMemIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadReorderMemIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txDMADebug;

        struct /*txDMAPizzaArb*/{
            PRV_CPSS_PX_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txDMAPizzaArb;

    }txDMA;

    struct /*txFIFO*/{

        struct /*txFIFOShiftersConfig*/{

            GT_U32    SCDMAShiftersConf[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;

        }txFIFOShiftersConfig;

        struct /*txFIFOLinkedLists*/{

            struct /*payloadLinkedList*/{

                struct /*payloadLinkListRAM*/{

                    GT_U32    payloadLinkListAddr;

                }payloadLinkListRAM;

            }payloadLinkedList;

            struct /*headerLinkedList*/{

                struct /*headerLinkListRAM*/{

                    GT_U32    headerLinkListAddr;

                }headerLinkListRAM;

            }headerLinkedList;

        }txFIFOLinkedLists;

        struct /*txFIFOInterrupts*/{

            GT_U32    txFIFOGeneralCauseReg1;
            GT_U32    txFIFOGeneralMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg1;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg2;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptCauseReg3;
            GT_U32    shifterSOPEOPFIFONotReadyInterruptMaskReg3;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg1;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg1;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg2;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg2;
            GT_U32    CTByteCountArrivedLateInterruptCauseReg3;
            GT_U32    CTByteCountArrivedLateInterruptMaskReg3;
            GT_U32    txFIFOSummaryCauseReg;
            GT_U32    txFIFOSummaryMaskReg;

        }txFIFOInterrupts;

        struct /*txFIFOGlobalConfig*/{

            GT_U32    SCDMAPayloadThreshold[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
            GT_U32    txFIFOMetalFixReg;

        }txFIFOGlobalConfig;

        struct /*txFIFOEnd2EndECCConfigs*/{

            GT_U32    dataECCConfig[2]/*ecc*/;
            GT_U32    dataECCStatus[2]/*ecc*/;

        }txFIFOEnd2EndECCConfigs;

        struct /*txFIFODebug*/{

            struct /*informativeDebug*/{

                GT_U32    SCDMAStatusReg6[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg1[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg2[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg3[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg4[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    SCDMAStatusReg5[PRV_CPSS_PX_DMA_PORTS_NUM_CNS]/*SCDMA*/;
                GT_U32    debugBusCtrlReg;
                GT_U32    txFIFODebugBus;
                GT_U32    LLEOPCntrLsb;
                GT_U32    LLEOPCntrMsb;
                GT_U32    incomingHeadersCntrLsb;
                GT_U32    incomingHeadersCntrMsb;
                GT_U32    headerIDDBFreeIDFIFOFillLevel;
                GT_U32    payloadIDDBFreeIDFIFOFillLevel;
                GT_U32    CTBCIDDBFreeIDFIFOFillLevel;

            }informativeDebug;

        }txFIFODebug;

        struct /*txFIFOPizzaArb*/{
            PRV_CPSS_PX_PP_DP_PIZZA_ARBITER_STC pizzaArbiter;
        }txFIFOPizzaArb;

    }txFIFO;

    struct /*BM*/{

        struct /*BMInterrupts*/{

            GT_U32    BMGeneralCauseReg1;
            GT_U32    BMGeneralMaskReg1;
            GT_U32    BMGeneralCauseReg2;
            GT_U32    BMGeneralMaskReg2;
            GT_U32    BMSummaryCauseReg;
            GT_U32    BMSummaryMaskReg;

        }BMInterrupts;

        struct /*BMGlobalConfigs*/{

            GT_U32    BMBufferTermination[5]/*reg*/;
            GT_U32    BMGlobalConfig1;
            GT_U32    BMGlobalConfig2;
            GT_U32    BMGlobalConfig3;
            GT_U32    BMPauseConfig;
            GT_U32    BMBufferLimitConfig1;
            GT_U32    BMBufferLimitConfig2;
            GT_U32    BMMetalFixReg;

        }BMGlobalConfigs;

        struct /*BMDebug*/{

            GT_U32    rxDMANextUpdatesFIFOMaxFillLevel[6]/*core*/;
            GT_U32    rxDMANextUpdatesFIFOFillLevel[6]/*core*/;
            GT_U32    SCDMAInProgRegs[148]/*scdma*/;/*148 in bobk , 114 in bc2*/

        }BMDebug;

        struct /*BMCntrs*/{

            GT_U32    BMGlobalBufferCntr;
            GT_U32    rxDMAsAllocationsCntr;
            GT_U32    BMFreeBuffersInsideBMCntr;
            GT_U32    BMRxDMAPendingAllocationCntr;
            GT_U32    BMCTClearsInsideBMCntr;
            GT_U32    incomingCTClearsCntr;
            GT_U32    incomingRegularClearsCntr;
            GT_U32    nextTableAccessCntr;
            GT_U32    nextTableWriteAccessCntr;
            GT_U32    BMTerminatedBuffersCntr;
            GT_U32    BMLostClearRequestsCntr;

        }BMCntrs;

    }BM;


    struct /*BMA*/{

        GT_U32  BmaMCCNTParityErrorCounter1;

    }BMA;  /*BMA*/



    struct /*GOP*/{

        struct {/*TAI*/

            GT_U32    TAIInterruptCause;
            GT_U32    TAIInterruptMask;
            GT_U32    TAICtrlReg0;
            GT_U32    TAICtrlReg1;
            GT_U32    timeCntrFunctionConfig0;
            GT_U32    timeCntrFunctionConfig1;
            GT_U32    timeCntrFunctionConfig2;
            GT_U32    frequencyAdjustTimeWindow;
            GT_U32    TODStepNanoConfigLow;
            GT_U32    TODStepFracConfigHigh;
            GT_U32    TODStepFracConfigLow;
            GT_U32    timeAdjustmentPropagationDelayConfigHigh;
            GT_U32    timeAdjustmentPropagationDelayConfigLow;
            GT_U32    triggerGenerationTODSecHigh;
            GT_U32    triggerGenerationTODSecMed;
            GT_U32    triggerGenerationTODSecLow;
            GT_U32    triggerGenerationTODNanoHigh;
            GT_U32    triggerGenerationTODNanoLow;
            GT_U32    triggerGenerationTODFracHigh;
            GT_U32    triggerGenerationTODFracLow;
            GT_U32    timeLoadValueSecHigh;
            GT_U32    timeLoadValueSecMed;
            GT_U32    timeLoadValueSecLow;
            GT_U32    timeLoadValueNanoHigh;
            GT_U32    timeLoadValueNanoLow;
            GT_U32    timeLoadValueFracHigh;
            GT_U32    timeLoadValueFracLow;
            GT_U32    timeCaptureValue0SecHigh;
            GT_U32    timeCaptureValue0SecMed;
            GT_U32    timeCaptureValue0SecLow;
            GT_U32    timeCaptureValue0NanoHigh;
            GT_U32    timeCaptureValue0NanoLow;
            GT_U32    timeCaptureValue0FracHigh;
            GT_U32    timeCaptureValue0FracLow;
            GT_U32    timeCaptureValue1SecHigh;
            GT_U32    timeCaptureValue1SecMed;
            GT_U32    timeCaptureValue1SecLow;
            GT_U32    timeCaptureValue1NanoHigh;
            GT_U32    timeCaptureValue1NanoLow;
            GT_U32    timeCaptureValue1FracHigh;
            GT_U32    timeCaptureValue1FracLow;
            GT_U32    timeCaptureStatus;
            GT_U32    timeUpdateCntrLow;
            GT_U32    timeUpdateCntrHigh;
            GT_U32    PClkClockCycleConfigLow;
            GT_U32    PClkClockCycleConfigHigh;
            GT_U32    incomingClockInCountingConfigLow;
            GT_U32    incomingClockInCountingEnable;
            GT_U32    generateFunctionMaskSecMed;
            GT_U32    generateFunctionMaskSecLow;
            GT_U32    generateFunctionMaskSecHigh;
            GT_U32    generateFunctionMaskNanoLow;
            GT_U32    generateFunctionMaskNanoHigh;
            GT_U32    generateFunctionMaskFracLow;
            GT_U32    generateFunctionMaskFracHigh;
            GT_U32    externalClockPropagationDelayConfigLow;
            GT_U32    externalClockPropagationDelayConfigHigh;
            GT_U32    driftThresholdConfigLow;
            GT_U32    driftThresholdConfigHigh;
            GT_U32    driftAdjustmentConfigLow;
            GT_U32    driftAdjustmentConfigHigh;
            GT_U32    clockCycleConfigLow;
            GT_U32    clockCycleConfigHigh;
            GT_U32    captureTriggerCntr;

        } TAI;

        struct /*PTP*/{

            GT_U32    PTPInterruptCause;
            GT_U32    PTPInterruptMask;
            GT_U32    PTPGeneralCtrl;
            GT_U32    PTPTXTimestampQueue0Reg0;
            GT_U32    PTPTXTimestampQueue0Reg1;
            GT_U32    PTPTXTimestampQueue0Reg2;
            GT_U32    PTPTXTimestampQueue1Reg0;
            GT_U32    PTPTXTimestampQueue1Reg1;
            GT_U32    PTPTXTimestampQueue1Reg2;
            GT_U32    totalPTPPktsCntr;
            GT_U32    PTPv1PktCntr;
            GT_U32    PTPv2PktCntr;
            GT_U32    Y1731PktCntr;
            GT_U32    NTPTsPktCntr;
            GT_U32    NTPReceivePktCntr;
            GT_U32    NTPTransmitPktCntr;
            GT_U32    WAMPPktCntr;
            GT_U32    addCorrectedTimeActionPktCntr;
            GT_U32    NTPPTPOffsetHigh;
            GT_U32    noneActionPktCntr;
            GT_U32    forwardActionPktCntr;
            GT_U32    dropActionPktCntr;
            GT_U32    captureIngrTimeActionPktCntr;
            GT_U32    captureAddTimeActionPktCntr;
            GT_U32    captureAddIngrTimeActionPktCntr;
            GT_U32    captureAddCorrectedTimeActionPktCntr;
            GT_U32    captureActionPktCntr;
            GT_U32    addTimeActionPktCntr;
            GT_U32    addIngrTimeActionPktCntr;
            GT_U32    NTPPTPOffsetLow;
            GT_U32    txPipeStatusDelay;

        }PTP[PRV_CPSS_PX_GOP_PORTS_NUM_CNS];

        struct /*CG*/{
            struct /*CG_CONVERTERS*/{
                GT_U32 CGMAControl0;
                GT_U32 CGMAControl1;
                GT_U32 CGMAConvertersStatus;
                GT_U32 CGMAConvertersResets;
                GT_U32 CGMAConvertersIpStatus;
                GT_U32 CGMAConvertersIpStatus2;
                GT_U32 CGMAConvertersFcControl0;
            }CG_CONVERTERS;
            struct /*CG_PORT*/{
                struct /*CG_PORT_MAC*/{
                    GT_U32 CGPORTMACCommandConfig;
                    GT_U32 CGPORTMACaddress0;
                    GT_U32 CGPORTMACaddress1;
                    GT_U32 CGPORTMACFrmLength;
                    GT_U32 CGPORTMACRxCrcOpt;
                    GT_U32 CGPORTMACStatus;
                    GT_U32 CGPORTMACStatNConfig;
                    GT_U32 CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_LAST_E];
                    GT_U32 CGPORTMACTxIpgLength;
                }CG_PORT_MAC;
            }CG_PORT;

        }CG[PRV_CPSS_PX_GOP_PORTS_NUM_CNS];/* only port 12 supported */

        struct
        {
            GT_U32 LEDControl;
            GT_U32 blinkGlobalControl;
            GT_U32 classGlobalControl;
            GT_U32 classesAndGroupConfig;
            GT_U32 classForcedData[6];
            GT_U32 portIndicationSelect[12];
            GT_U32 portTypeConfig[1];
            GT_U32 portIndicationPolarity[3];
            GT_U32 classRawData[6];
        }LED;
        struct
        {
            GT_U32 SMIManagement;
            GT_U32 SMIMiscConfiguration;
            GT_U32 PHYAutoNegotiationConfig;
            GT_U32 PHYAddress[16];
        }SMI;

        PRV_CPSS_PX_PORT_FCA_REGS_ADDR_STC FCA[PRV_CPSS_PX_GOP_PORTS_NUM_CNS];

        PRV_CPSS_PX_PP_REGS_PER_PORT_REGS   perPortRegs[PRV_CPSS_PX_GOP_PORTS_NUM_CNS];

    }GOP;

    PRV_CPSS_PX_SERDES_REGS_ADDR_STC    serdesConfig[PRV_CPSS_PX_SERDES_NUM_CNS];

    struct /*CNC*/
    {
        struct /*perBlockRegs*/
        {
            struct /*wraparound*/
            {
                GT_U32    CNCBlockWraparoundStatusReg[PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS][4]/*Block*//*Entry*/;
            }wraparound;

            struct /*rangeCount*/
            {
                GT_U32    CNCBlockRangeCountEnable[2][PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS]/*group of ranges*//*block*/;
            }rangeCount;

            struct /*clientEnable*/
            {
                GT_U32    CNCBlockConfigReg0[PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS]/*Block*/;
            }clientEnable;

            struct /*entryMode*/
            {
                GT_U32    CNCBlocksCounterEntryModeRegister/*Block/8*/;
            }entryMode;

            struct /*memory*/
            {
                GT_U32    baseAddress[PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS]/*Block memory base address*/;
            }memory;
        }perBlockRegs;

        struct /*globalRegs*/
        {
            GT_U32    CNCGlobalConfigReg;
            GT_U32    CNCFastDumpTriggerReg;
            GT_U32    CNCClearByReadValueRegWord0;
            GT_U32    CNCClearByReadValueRegWord1;
            GT_U32    CNCInterruptSummaryCauseReg;
            GT_U32    CNCInterruptSummaryMaskReg;
            GT_U32    wraparoundFunctionalInterruptCauseReg;
            GT_U32    wraparoundFunctionalInterruptMaskReg;
            GT_U32    rateLimitFunctionalInterruptCauseReg;
            GT_U32    rateLimitFunctionalInterruptMaskReg;
            GT_U32    miscFunctionalInterruptCauseReg;
            GT_U32    miscFunctionalInterruptMaskReg;
        }globalRegs;

    }CNC;

    struct /*TXQ*/
    {
        struct /*dq*/
        {
            struct /*shaper*/
            {

                GT_U32    portTokenBucketMode[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                GT_U32    portDequeueEnable[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                GT_U32    tokenBucketUpdateRate;
                GT_U32    tokenBucketBaseLine;
                GT_U32    CPUTokenBucketMTU;
                GT_U32    portsTokenBucketMTU;

            }shaper;

            struct /*scheduler*/
            {

                struct /*schedulerConfig*/
                {

                    GT_U32    conditionalMaskForPort[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                    GT_U32    schedulerConfig;
                    GT_U32    pktLengthForTailDropDequeue;
                    GT_U32    highSpeedPorts[2];

                }schedulerConfig;

                struct /*priorityArbiterWeights*/
                {

                    GT_U32    profileSDWRRGroup[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg0[16]/*Profile*/;
                    GT_U32    profileSDWRRWeightsConfigReg1[16]/*Profile*/;
                    GT_U32    profileSDWRREnable[16]/*Profile*/;
                    GT_U32    portSchedulerProfile[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;

                }priorityArbiterWeights;

                struct /*portShaper*/
                {

                    GT_U32    portRequestMaskSelector[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                    GT_U32    portRequestMask;
                    GT_U32    fastPortShaperThreshold;

                }portShaper;

                struct /*portArbiterConfig*/
                {
                    /* BC2 / BobK --- 85 registers, */
                    /* Aldrin     --- 96 registres  */
                    GT_U32    portsArbiterPortWorkConservingEnable[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-port*/;
                    GT_U32    portsArbiterMap[96]/*Slice_Group*/;
                    GT_U32    portsArbiterConfig;
                    GT_U32    portsArbiterStatus;

                }portArbiterConfig;

            }scheduler;

            struct /*global*/
            {

                struct /*memoryParityError*/
                {

                    GT_U32    tokenBucketPriorityParityErrorCntr;
                    GT_U32    parityErrorBadAddr;

                }memoryParityError;

                struct /*globalDQConfig*/
                {

                    GT_U32    portToDMAMapTable[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                    GT_U32    profileByteCountModification[16]/*Profile*/;
                    GT_U32    portBCConstantValue[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;
                    GT_U32    globalDequeueConfig;
                    GT_U32    BCForCutThrough;
                    GT_U32    creditCountersReset;

                }globalDQConfig;

                struct /*flushTrig*/
                {

                    GT_U32    portTxQFlushTrigger[PRV_CPSS_PX_MAX_DQ_PORTS_CNS]/*DQ-Port*/;

                }flushTrig;

                struct /*ECCCntrs*/
                {

                    GT_U32    DQIncECCErrorCntr;
                    GT_U32    DQClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/
                {

                    GT_U32    egrMirroredDroppedCntr;
                    GT_U32    STCDroppedCntr;

                }dropCntrs;

                struct /*Credit_Counters*/
                {
                    GT_U32    txdmaPortCreditCounter[PRV_CPSS_PX_DMA_PORTS_NUM_CNS];
                }creditCounters;

            }global;

            struct /*flowCtrlResponse*/
            {

                GT_U32    schedulerProfilePFCTimerToPriorityQueueMapEnable[16]/*profile*/;
                GT_U32    schedulerProfilePriorityQueuePFCFactor[16][8]/*Profile*//*TC*/;
                GT_U32    schedulerProfileLLFCXOFFValue[16]/*profile*/;
                GT_U32    portToTxQPortMap[256]/*Source Port*/;
                GT_U32    PFCTimerToPriorityQueueMap[8]/*TC*/;
                GT_U32    flowCtrlResponseConfig;

            }flowCtrlResponse;

        }dq[PRV_CPSS_PX_NUM_DQ_CNS];

        struct /*ll*/
        {

            struct /*global*/{

                struct /*globalLLConfig*/{

                    GT_U32    Global_Configuration;

                }globalLLConfig;

            }global;

        }ll;

        struct /*pfc*/
        {
            GT_U32    LLFCTargetPortRemap[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(4)]/*Port div 4*/;
            GT_U32    portFCMode[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(32)]/*Port div 32*/;
            GT_U32    PFCSourcePortToPFCIndexMap[32]/*Global Port*/;
            GT_U32    PFCPortProfile[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(8)]/*Port div 8*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[1]/*port group*/;
            GT_U32    PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsStatus[1]/*port group*/;
            GT_U32    PFCPortGroupCntrsParityErrorsCntr[1]/*port group*/;
            GT_U32    PFCGroupOfPortsTCXonThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCXoffThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCDropThresholds[8][1]/*tc*//*pipe*/;
            GT_U32    PFCGroupOfPortsTCCntr[1][8]/*port group*//*tc*/;
            GT_U32    PFCTriggerGlobalConfig;
            GT_U32    FCModeProfileTCXOffThresholds[8][8] /*profile*//*TC*/;
            GT_U32    FCModeProfileTCXOnThresholds[8][8] /*profile*//*TC*/;
            struct /*DBA - dynamic buffer allocation*/
            {
                GT_U32 PFCProfileTCAlphaThresholds[8][8]/*profile*//*TC*/;
                GT_U32 PFCAvailableBuffers;
                GT_U32 PFCDynamicBufferAllocationDisable;
                GT_U32 PFCXON_XOFFstatus[17];
            }dba;
        }pfc;

        struct /*qcn*/
        {
            GT_U32    CCFCSpeedIndex[72]/*Reg offset*/;
            GT_U32    CNSampleTbl[8]/*qFb 3 msb*/;
            GT_U32    profilePriorityQueueCNThreshold[16][8]/*profile*//*priority*/;
            GT_U32    feedbackMIN;
            GT_U32    feedbackMAX;
            GT_U32    CNGlobalConfig;
            GT_U32    enFCTriggerByCNFrameOnPort[16]/*Port div 32*/;
            GT_U32    enCNFrameTxOnPort[16]/*Port div 32*/;
            GT_U32    feedbackCalcConfigs;
            GT_U32    txqECCConf;
            GT_U32    descECCSingleErrorCntr;
            GT_U32    descECCDoubleErrorCntr;
            GT_U32    CNTargetAssignment;
            GT_U32    CNDropCntr;
            GT_U32    QCNInterruptCause;
            GT_U32    QCNInterruptMask;
            GT_U32    CNDescAttributes;
            GT_U32    CNBufferFIFOParityErrorsCntr;
            GT_U32    CNBufferFIFOOverrunsCntr;
            GT_U32    ECCStatus;
            GT_U32    cnAvailableBuffers;

        }qcn;

        struct /*queue*/
        {
            struct /*tailDrop*/{

                struct /*tailDropLimits*/{

                    GT_U32    poolBufferLimits[8]/*Priority*/;
                    GT_U32    poolDescLimits[8]/*Priority*/;
                    GT_U32    profilePortDescLimits[16]/*profile*/;
                    GT_U32    profilePortBufferLimits[16]/*profile*/;
                    GT_U32    globalDescsLimit;
                    GT_U32    globalBufferLimit;

                }tailDropLimits;

                struct /*tailDropCntrs*/{

                    GT_U32    portBuffersCntr[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(1)]/*Port*/; /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portDescCntr[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(1)]/*Port*/;    /* for sip5.20 and above - 576, else 72 */
                    GT_U32    priorityDescCntr[8]/*TC*/;
                    GT_U32    priorityBuffersCntr[8]/*TC*/;
                    GT_U32    totalDescCntr;
                    GT_U32    totalBuffersCntr;

                }tailDropCntrs;

                struct /*tailDropConfig*/{

                    GT_U32    portTailDropCNProfile[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(1)]/*Port*/;   /* for sip5.20 and above - 576, else 72 */
                    GT_U32    portEnqueueEnable[PRV_CPSS_PX_NUM_PORT_IN_REG_MAC(1)]/*Port*/;       /* for sip5.20 and above - 576, else 72 */
                    GT_U32    profileTailDropConfig[16]/*Profile*/;
                    GT_U32    profilePriorityQueueToPoolAssociation[16]/*profile*/;
                    GT_U32    profilePriorityQueueEnablePoolUsage[8]/*Priority*/;
                    GT_U32    profilePoolWRTDEn[16]/*profile*/;
                    GT_U32    profileQueueWRTDEn[16]/*profile*/;
                    GT_U32    profilePortWRTDEn[16]/*profile*/;
                    GT_U32    profileMaxQueueWRTDEn[16]/*profile*/;
                    GT_U32    profileECNMarkEn[16]/*Profile*/;
                    GT_U32    profileCNMTriggeringEnable[16]/*Profile*/;
                    GT_U32    globalTailDropConfig;
                    GT_U32    pktLengthForTailDropEnqueue;
                    GT_U32    maxQueueWRTDMasks;
                    GT_U32    dynamicAvailableBuffers;
                    GT_U32    portWRTDMasks;
                    GT_U32    queueWRTDMasks;
                    GT_U32    poolWRTDMasks;

                }tailDropConfig;

                struct /*resourceHistogram*/{

                    struct /*resourceHistogramLimits*/{

                        GT_U32    resourceHistogramLimitReg1;
                        GT_U32    resourceHistogramLimitReg2;

                    }resourceHistogramLimits;

                    struct /*resourceHistogramCntrs*/{

                        GT_U32    resourceHistogramCntr[4]/*counter*/;

                    }resourceHistogramCntrs;

                }resourceHistogram;

                struct /*mcFilterLimits*/{

                    GT_U32    mirroredPktsToAnalyzerPortDescsLimit;
                    GT_U32    ingrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    egrMirroredPktsToAnalyzerPortBuffersLimit;
                    GT_U32    mcDescsLimit;
                    GT_U32    mcBuffersLimit;

                }mcFilterLimits;

                struct /*muliticastFilterCntrs*/{

                    GT_U32    snifferDescsCntr;
                    GT_U32    ingrSnifferBuffersCntr;
                    GT_U32    egrSnifferBuffersCntr;
                    GT_U32    mcDescsCntr;
                    GT_U32    mcBuffersCntr;

                }muliticastFilterCntrs;

                struct /*FCUAndQueueStatisticsLimits*/{

                    GT_U32    XONLimit[8]/*SharedLimit*/;
                    GT_U32    XOFFLimit[8]/*SharedLimit*/;
                    GT_U32    queueXONLimitProfile[16]/*Profile*/;
                    GT_U32    queueXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    portXONLimitProfile[16]/*Profile*/;
                    GT_U32    portXOFFLimitProfile[16]/*Profile*/;
                    GT_U32    globalXONLimit;
                    GT_U32    globalXOFFLimit;
                    GT_U32    FCUMode;

                }FCUAndQueueStatisticsLimits;

            }tailDrop;

            struct /*global*/{

                struct /*ECCCntrs*/{

                    GT_U32    TDClrECCErrorCntr;

                }ECCCntrs;

                struct /*dropCntrs*/{

                    GT_U32    clearPktsDroppedCntrPipe[4]/*Pipe*/;

                }dropCntrs;

            }global;

            struct /*peripheralAccess*/{

                struct /*peripheralAccessMisc*/{

                    GT_U32    portGroupDescToEQCntr[4]/*port group*/;
                    GT_U32    peripheralAccessMisc;
                    GT_U32    QCNIncArbiterCtrl;

                }peripheralAccessMisc;

                struct /*egrMIBCntrs*/{

                    GT_U32    txQMIBCntrsPortSetConfig[2]/*Set*/;
                    GT_U32    txQMIBCntrsSetConfig[2]/*Set*/;
                    GT_U32    setTailDroppedPktCntr[2]/*Set*/;
                    GT_U32    setOutgoingPktCntr[2]/*Set*/;

                }egrMIBCntrs;

                struct /*CNCModes*/{

                    GT_U32    CNCModes;

                }CNCModes;

            }peripheralAccess;

        }queue;

        struct /*bmx*/
        {

            GT_U32    fillLevelDebugRegister;   /*Fill Level Debug Register*/

        }bmx;

    }TXQ;

    /* Added manually */
    struct /* MG */{

        struct /*XSMI*/{

            GT_U32 XSMIManagement;
            GT_U32 XSMIAddress;
            GT_U32 XSMIConfiguration;

        }XSMI;

        struct px_global
        {
            GT_U32          globalControl;  /* Global Control                      */
            GT_U32          extendedGlobalControl;  /* Extended Global Control */
            GT_U32          extendedGlobalControl2; /* Extended Global Control2 */
            GT_U32          generalConfigurations;  /* General Configurations   */
            GT_U32          addrCompletion; /* PCI to memory regions map  */
            GT_U32          fuQBaseAddr;    /* Fu Addr Update Block Base address.  */
            GT_U32          fuQControl;     /* Fu Addr Update Queue Control.       */
            GT_U32          interruptCoalescing; /* Interrupt Coalesing Conf. Register  */
            GT_U32          lastReadTimeStampReg; /* Last Read Time Stamp Register */
            GT_U32          sampledAtResetReg;    /* Sampled at Reset Register */

            GT_U32          metalFix;  /* Metal Fix */

            struct /*globalInterrupt*/
            {
                GT_U32    dfxInterruptCause;
                GT_U32    dfxInterruptMask;

                GT_U32    dfx1InterruptCause; /* sip5 only */
                GT_U32    dfx1InterruptMask;  /* sip5 only */
            }globalInterrupt;

            GT_U32          genxsRateConfig;    /* General Access Engine (Genxs) Rate Configuration */

            struct
            {
                GT_U32  serInitCtrl;
            }twsiReg;
        }globalRegs;

        struct
        {
            GT_U32  sdmaCfgReg;         /* SDMA configuration register.         */
            GT_U32  rxQCmdReg;          /* Receive Queue Command Reg.           */
            GT_U32  rxDmaCdp[8];        /* Rx SDMA Current Rx Desc. Pointer Reg */
            GT_U32  txQCmdReg;          /* Transmit Queue Command Reg.          */
            GT_U32  txDmaCdp[8];        /* Tx Current Desc Pointer Reg.         */
            GT_U32  rxDmaResErrCnt[2];  /* Rx SDMA resource error count regs0/1.*/
            GT_U32  rxDmaPcktCnt[8];    /* Rx SDMA packet count.                */
            GT_U32  rxDmaByteCnt[8];    /* Rx SDMA byte count.                  */
            GT_U32  txQWrrPrioConfig[8];/* Tx Queue WRR Priority Configuration. */
            GT_U32  txQFixedPrioConfig; /* Tx Queue Fixed Prio Config Register. */
            GT_U32  txSdmaTokenBucketQueueCnt[8];   /* Tx SDMA Token-Bucket Queue Counter */
            GT_U32  txSdmaTokenBucketQueueConfig[8];/* Tx SDMA Token Bucket Queue Configuration */
            GT_U32  txSdmaWrrTokenParameters; /* Transmit SDMA WRR Token Parameters */
            GT_U32  txSdmaPacketGeneratorConfigQueue[8]; /* Tx SDMA Packet Generator Config Queue */
            GT_U32  txSdmaPacketCountConfigQueue[8];     /* Tx SDMA Packet Count Config Queue */
            GT_U32  rxSdmaResourceErrorCountAndMode[8];  /* Rx SDMA Resource Error Count and Mode */
        }sdmaRegs;

    }MG;

    struct PRV_CPSS_PX_MAC_PACKET_GEN_VER1_CONFIG_STC
    {
        GT_U32 macDa[3];
        GT_U32 macSa[3];
        GT_U32 etherType;
        GT_U32 vlanTag;
        GT_U32 packetLength;
        GT_U32 packetCount;
        GT_U32 ifg;
        GT_U32 macDaIncrementLimit;
        GT_U32 controlReg0;
        GT_U32 readAndWriteData;
        GT_U32 readAndWriteControl;
        GT_U32 dataPattern[32];
    } packGenConfig[4]; /* one packet generator per 4 ports */

    struct
    {
        PRV_CPSS_PX_PP_MPPM_PIZZA_ARBITER_STC pizzaArbiter;
        struct {
            GT_U32 statusFailedSyndrome;
            GT_U32 lastFailingBuffer;
            GT_U32 lastFailingSegment;
        } dataIntegrity;

    }MPPM;


    struct px_addrSpace
    {
        GT_U32  buffMemWriteControl;    /* BufMem write control        */
        GT_U32  buffMemBank0Write;      /* bank0 write                 */
        GT_U32  buffMemBank1Write;      /* bank1 write                 */
        GT_U32  buffMemClearType;       /* Buffer Memory Clear Type Register */
    }addrSpace;


}PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC;
/* restore alignment setting */
#pragma pack(pop)

/* short name to RXDMA unit registers */
#define PRV_PX_REG1_UNIT_RXDMA_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->rxDMA

/* short name to TXDMA unit registers */
#define PRV_PX_REG1_UNIT_TXDMA_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->txDMA

/* short name to TX_FIFO unit registers */
#define PRV_PX_REG1_UNIT_TX_FIFO_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->txFIFO

/* TAI subunit */
#define PRV_PX_REG1_UNIT_GOP_TAI_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.TAI

/* LED subunit */
#define PRV_PX_REG1_UNIT_GOP_LED_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.LED

/* PTP (TSU) subunit   */
/* _port - port number */
#define PRV_PX_REG1_UNIT_GOP_PTP_MAC(_devNum, _port)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.PTP[_port]

/* CG subunit */
/* CG Port */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC(_devNum, _port)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT

/* CG Port MAC */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(_devNum, _port)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC

/* CG Port MAC MIBS */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_REGS_ADDR_ARR_MAC(_devNum, _port)   \
    &(PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[0])

/* CG Converters */
#define PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(_devNum, _port)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.CG[_port].CG_CONVERTERS

/* CG_PORT wrapper - get the register address - cg port mac address0 register */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACaddress0;

/* CG_PORT wrapper - get the register address - cg port mac frame length */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACFrmLength;

/* CG_PORT wrapper - get the register address - cg port mac command config */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACCommandConfig;

/* CG_PORT wrapper - get the register address - cg port mac Rx crc opt */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACRxCrcOpt;

/* CG_PORT wrapper - get the register address - cg port mac status */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatus;

/* CG_PORT wrapper - get the register address - cg port mac StatN Config */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_STATN_CONFIG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACStatNConfig;

/* CG_PORT wrapper - get the register address - cg port mac MIBS */
#define PRV_CPSS_PX_REG1_CG_PORT_MAC_MIB_ENTRIES_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_MAC(devNum, portNum);

/* CG_PORT wrapper - get the cg MIBS struct size */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_STC_SIZE_MAC(devNum,portNum)\
    sizeof(PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_MAC(devNum, portNum)) / sizeof(GT_U64);

/* CG_PORT wrapper - get the cg MIBS struct as an array */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_STC_TO_STRUCT_MAC(devNum,portNum)\
    (GT_U64 *) (&(PRV_PX_REG1_UNIT_CG_PORT_MAC_MIBS_MAC(devNum, portNum)));

/* CG_PORT wrapper - get the offset of a MIB counter field within the CG MIBS struct */
#define PRV_PX_REG1_UNIT_CG_PORT_MAC_MIB_OFFSET_MAC(field)\
    offsetof(CPSS_PORT_MAC_CG_COUNTER_SET_STC,field) / sizeof(GT_U64)

/* CG_CONVERTERS wrapper - get the register address - cg port mac control register0 */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAControl0;

/* CG_CONVERTERS wrapper - get the register address - cg port mac control register1 */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL1_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAControl1;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters status */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersStatus;

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters resets */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_RESETS_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersResets

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus

/* CG_CONVERTERS wrapper - get the register address - cg port mac converters ip status2 */
#define PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC(devNum,portNum,regAddrPtr)\
        *(regAddrPtr) = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portNum).CGMAConvertersIpStatus2

/* CG port applicable check */
#define PRV_CPSS_PX_REG1_CG_APPLICABLE_PORT_MAC(portNum,cgApplicablePtr)\
    ( (portNum) == 12 ) ? ( *(cgApplicablePtr) = GT_TRUE ) : ( *(cgApplicablePtr) = GT_FALSE )

/* FCA subunit   */
/* _port - port/mac number */
#define PRV_PX_REG1_UNIT_GOP_FCA_MAC(_devNum, _port)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP.FCA[_port]

#define PRV_PX_REG_UNIT_GOP_CPU_FCA_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_MAC(_devNum)->macRegs.cpuPortRegs.FCA

/* short name to CNC unit registers */
#define PRV_PX_REG1_UNIT_CNC_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->CNC

/* short name to TXQ_DQ unit registers */
#define PRV_PX_REG1_UNIT_TXQ_DQ_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[0]

/* short name to TXQ_DQ specific unit registers */
#define PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(_devNum,index)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.dq[index]

/* short name to TXQ_LL unit registers */
#define PRV_PX_REG1_UNIT_TXQ_LL_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.ll

/* short name to TXQ_PFC unit registers */
#define PRV_PX_REG1_UNIT_TXQ_PFC_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.pfc

/* short name to TXQ_QCN unit registers */
#define PRV_PX_REG1_UNIT_TXQ_QCN_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.qcn

/* short name to TXQ_BMX unit registers */
#define PRV_PX_REG1_UNIT_TXQ_BMX_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.bmx

/* short name to TXQ_Q unit registers */
#define PRV_PX_REG1_UNIT_TXQ_Q_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->TXQ.queue

/* short name to MG unit registers */
#define PRV_PX_REG1_UNIT_MG_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->MG

/* short name to BM unit registers */
#define PRV_PX_REG1_UNIT_BM_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->BM

/* short name to GOP unit registers */
#define PRV_PX_REG1_UNIT_GOP_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->GOP

/* short name to PCP unit registers */
#define PRV_PX_REG1_UNIT_PCP_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->PCP

/* short name to PCP HASH calculation registers */
#define PRV_PX_REG1_UNIT_PCP_HASH_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->PCP.HASH

/* short name to PCP PTP registers */
#define PRV_PX_REG1_UNIT_PCP_PTP_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->PCP.PTP

/* short name to PCP IP2ME registers */
#define PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->PCP.IP2ME

/* short name to PHA unit registers */
#define PRV_PX_REG1_UNIT_PHA_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->PHA

/* short name to MCFC unit registers */
#define PRV_PX_REG1_UNIT_MCFC_MAC(_devNum)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->MCFC

/* short name to SERDES unit registers */
#define PRV_PX_REG1_UNIT_SERDES_MAC(_devNum,index)   \
    PRV_CPSS_PX_DEV_REGS_VER1_MAC(_devNum)->serdesConfig[index]

/**
* @internal prvCpssPxHwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrVer1Init
(
    IN  GT_SW_DEV_NUM devNum
);

/**
* @internal prvCpssPxHwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller registers
*         struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwResetAndInitControllerRegAddrInit
(
    IN  GT_SW_DEV_NUM devNum
);


/**
* @internal prvCpssPxHwRegAddrVer1Remove function
* @endinternal
*
* @brief   This function free allocated memories relate to the registers struct of the devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrVer1Remove
(
    IN  GT_SW_DEV_NUM devNum
);

/**
* @internal prvCpssPxHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssPxHwRegAddrToUnitIdConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_U32                   regAddr
);

/**
* @internal prvCpssPxCgPortDbInvalidate function
* @endinternal
*
* @brief   Invalidate or init CG port register database
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - CG MAC port number
* @param[in] cgUnitInvalidate         - invalidate status
*                                      GT_TRUE - invalidate DB for given MAC port
*                                      GT_FALSE - init DB for given MAC port
*                                       None
*/
GT_STATUS prvCpssPxCgPortDbInvalidate
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32 portNum,
    IN  GT_BOOL cgUnitInvalidate
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxRegsVer1h */

