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
* @file prvCpssDxChInfoEnhanced.h
*
* @brief This file includes the declaration of the structure to hold the
* enhanced parameters that "fine tuning" the PP's capabilities in CPSS
* point of view.
*
* @version   50
********************************************************************************
*/
#ifndef __prvCpssDxChInfoEnhancedh
#define __prvCpssDxChInfoEnhancedh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>

/**
* @struct PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC
 *
 * @brief A Structure including PP's tables sizes and parameters that
 * define the "fine tuning" , of the specific device.
 * Notes:
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{
    void*       enhancedInfoPtr;

    struct{
        /* bridge section */
        GT_U32  fdb;
        GT_U32  vidxNum;
        GT_U32  stgNum;

        /* routing and TTI section */
        GT_U32 router;
        GT_U32 tunnelTerm;

        /* tunnel section*/
        GT_U32  tunnelStart;
        GT_U32  routerArp;

        /* ip section */
        GT_U32  routerNextHop;
        GT_U32  ecmpQos;
        GT_U32  mllPairs;

        /* PCL section */
        GT_U32 policyTcamRaws;

        /* CNC section */
        GT_U32 cncBlocks;
        GT_U32 cncBlockNumEntries;

        /* Policer Section */
        GT_U32  policersNum;
        GT_U32  egressPolicersNum;

        /* trunk Section */
        GT_U32  trunksNum;

        /* Transmit Descriptors */
        GT_U32 transmitDescr;

        /* Buffer Memory */
        GT_U32 bufferMemory;

        /*  Transmit Queues */
        GT_U32      txQueuesNum;

        /* LPM RAM */
        GT_U32 lpmRam;

        /* pipe BW capacity for Rx-DMA Tx-DMA TxQ Tx-FIFO Eth-TxFIFO units */
        GT_U32 pipeBWCapacityInGbps;

        /* 802.1br BPE Point to Point E-Channels */
        GT_U32 bpePointToPointEChannels;
        /* sip6 : exact match number of entries */
        GT_U32  emNum;

        /* Max number of VRF-IDs */
        GT_U32 vrfIdNum;

        /* Max number of OAM Table entries (per direction) */
        GT_U32 oamEntriesNum;

        /****************************************************/
        /* add here other table sizes that are "parametric" */
        /****************************************************/
    }tableSize;

    struct
    {
        /* TR101 Feature support */
        GT_BOOL tr101Supported;

        /* VLAN translation support */
        GT_BOOL vlanTranslationSupported;

        /* Policer Ingress second stage support flag */
        GT_BOOL iplrSecondStageSupported;

        /* trunk CRC hash support flag */
        GT_BOOL trunkCrcHashSupported;

        /* TM support flag */
        GT_BOOL TmSupported;
    }featureInfo;

}PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC;

/* check policer Id , when used as 'key' we return GT_BAD_PARAM on error */
/* note : index is in the range: 0..maxNum-1 */
#define PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(_devNum,_stage,_index)              \
    if((_index) >= PRV_CPSS_DXCH_PP_MAC(_devNum)->policer.memSize[(_stage)])     \
    {                                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "policer index[%d]>=[%d] (for stage[%d])",\
            _index,PRV_CPSS_DXCH_PP_MAC(_devNum)->policer.memSize[(_stage)],_stage);\
    }

/* check policer counter index , when used as 'key' we return GT_BAD_PARAM on error */
/* note : counter index is in the range: 0..maxNum-1 */
#define PRV_CPSS_DXCH_POLICERS_COUNTER_INDEX_NUM_CHECK_MAC(_devNum,_stage,_index)              \
    if((_index) >= PRV_CPSS_DXCH_PP_MAC(_devNum)->policer.countingMemSize[(_stage)])     \
    {                                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "policer counter index[%d]>=[%d] (for stage[%d])",\
            _index,PRV_CPSS_DXCH_PP_MAC(_devNum)->policer.countingMemSize[(_stage)],_stage);\
    }

/* VIDX that represents flood in vlan */
#define FLOOD_VIDX_CNS 0xFFF


/**
* @enum PRV_CPSS_DXCH_PORT_TYPE_ENT
 *
 * @brief port type used in mapping functions
*/
typedef enum{

    /** invalide source */
    PRV_CPSS_DXCH_PORT_TYPE_INVALID_E = -1,

    /** physical port */
    PRV_CPSS_DXCH_PORT_TYPE_PHYS_E = 0,

    /** rxDma port */
    PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,

    /** txDma port */
    PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,

    /** mac number */
    PRV_CPSS_DXCH_PORT_TYPE_MAC_E,

    /** txq port */
    PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,

    /** @brief Interlaken channel, relevant
     *  only when port mapping is CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E
     */
    PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E,

    /** traffic manager port idx , relavant if tmEnable == GT_TRUE */
    PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E,

    /** max port , used in array declaration */
    PRV_CPSS_DXCH_PORT_TYPE_MAX_E

} PRV_CPSS_DXCH_PORT_TYPE_ENT;

/*******************************************************************************
* PRV_CPSS_DXCH_PP_CONFIG_ENHANCED_INIT_FUNC
*
* DESCRIPTION:
*       do enhanced initialization , on devices that need to.
*       this function sets the fine tuning parameters needed by the CPSS SW.
*       Function may also do HW operations.
*
* APPLICABLE DEVICES:
*       NA
*
* INPUTS:
*       devNum      - The device number
*
* OUTPUTS:
*       none.

* RETURNS:
*       GT_OK         - on success
*       GT_HW_ERROR   - on hardware error.
*       GT_BAD_PARAM  - on bad devNum
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DXCH_PP_CONFIG_ENHANCED_INIT_FUNC)
(
    IN      GT_U8                           devNum
);

/* pointer to a function used for DXCH devices that will do enhanced
   initialization of SW and HW parameters --  for FUTURE compatibility */
GT_STATUS prvCpssDxChPpConfigEnhancedInitFuncPtrBind(PRV_CPSS_DXCH_PP_CONFIG_ENHANCED_INIT_FUNC funcPtr);

/**
* @struct PRV_CPSS_DXCH_PP_HW_INFO_DATA_PATH_INFO_STC
 *
 * @brief A Structure with info about data path (rxdma,txdma,txfifo).
 * Notes:
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{

    /** PFC Response First Port In Range. */
    GT_U32 pfcResponseFirstPortInRange;

    /** PFC Response Last Port In Range. */
    GT_U32 pfcResponseLastPortInRange;

    /** first port used in the data path. */
    GT_U32 dataPathFirstPort;

    /** @brief number of ports in the DP .
     *  if value is ZERO ... meaning 'not valid' DP .. ignored
     *  localDmaMapArr  - local DMA numbers per relative global numbers.
     *  May be NULL if map is 1:1.
     *  Used for Aldrin device.
     */
    GT_U32 dataPathNumOfPorts;

    const GT_U32 *localDmaMapArr;

    /** if 0 this DP not support the CPU DMA */
    GT_U32 cpuPortDmaNum;
    /** if 0 this DP not support the 'second' CPU DMA */
    GT_U32 cpuPortDmaNum1; /*AC5X support it */

    /** @brief (egress only) if 0 this DP not support the TM DMA (traffic manager)
     *  NOTE: for bobk (global are also local):
     *  global DMA IDs 0..47  are 'DP 0' local dma 0..47
     *  global DMA IDs 56..71 are 'DP 1' local dma 56..71
     *  NOTE: for bobcat3 (global are not local):
     *  global DMA IDs 0..11  are 'DP 0' local dma 0..11
     *  global DMA IDs 12..23 are 'DP 1' local dma 0..11
     *  global DMA IDs 24..35 are 'DP 2' local dma 0..11
     *  global DMA IDs 36..47 are 'DP 3' local dma 0..11
     *  global DMA IDs 48..59 are 'DP 4' local dma 0..11
     *  global DMA IDs 60..71 are 'DP 5' local dma 0..11
     *  global DMA ID 72   is 'DP 0' local dma 12
     *  currently there is NO representation of :
     *  'DP 1' local dma 12
     *  'DP 2' local dma 12
     *  'DP 3' local dma 12
     *  'DP 4' local dma 12
     *  'DP 5' local dma 12
     *  NOTE: for aldrin (global are not local):
     *  global DMA IDs 0..11  are 'DP 0' local dma 56..59, 64..71
     *  global DMA IDs 12..23 are 'DP 1' local dma 56..59, 64..71
     *  global DMA IDs 24..32 are 'DP 2' local dma 56..59, 64..67, 62
     *  global DMA ID 72   is 'DP 2' local dma 72
     */
    GT_U32 tmDmaNum;

} PRV_CPSS_DXCH_PP_HW_INFO_DATA_PATH_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_INFO_STC
 *
 * @brief A Structure with info for each cpu port (mg)
 * Notes:
 *      index to this array is the 'MG' unit id
 *
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{

    /** if this cpu port is valid. */
    GT_BOOL valid;

    /** the global dma number of the cpu port. */
    GT_U32 dmaNum;

    /** the dq num of the cpu port */
    GT_U32 dqNum;

    /** The first SW queue index that serves this MG */
    /* NOTE: this parameter is set during 'networkIf init' when ALL CPU SDMA ports
        already bound to their MG units and we can see 'unused MG units' */
    GT_U8   firstSwQueueIndex;
    /** indication that this MG is used as 'CPU SDMA port' (bind during 'port mapping') */
    GT_BOOL usedAsCpuPort;

    /** the local dma number of the cpu port. (supporting multiple CPU DMAs in the same DP unit)
        sip 6.15 and above */
    GT_U32 localDmaNum;

    /** indication to use cpuPortDmaNum1 instead of cpuPortDmaNum in hwInfo.multiDataPath.info[]  */
    GT_BOOL use_cpuPortDmaNum1;

} PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_NETIF_INFO_STC
 *
 * @brief A Structure with info for each cpu port 'netIf'
 * Notes:
 *      1. SIP6 info (not init/used for previous devices !)
 *      2. index to this array is the netIfNum
*/
typedef struct{

    /** if this netIfNum is valid. */
    GT_BOOL valid;

    /** the MG unit index (the MG to serve the SDMA operation) */
    GT_U32 mgUnitId;

} PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_NETIF_INFO_STC;

/* indication for the last device in devices lists */
#define LAST_DEV_IN_LIST_CNS   0xFFFFFFFF
/* auto calc field */
#define AUTO_CALC_FIELD_CNS 0xFFFFFFFF

/* indication that the number of CNC blocks and size of block should be
    'auto calculated' from the 'total number of cnc counters ...*/
#define CNC_AUTO_CALC                       AUTO_CALC_FIELD_CNS
/* auto calc the pcl rules from the tti number of rules (shared tcam)*/
#define PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC             AUTO_CALC_FIELD_CNS

/* 8 TX Queues number */
#define TX_QUEUE_NUM_8_CNS                8

/* not applicable (NA) table */
#define NA_TABLE_CNS    0

/* indication that the number of trunks that the device supports are according
   to number of bits that the device hold for trunk
   meaning no extra limitation on the number of trunks
   */
#define TRUNK_NUM_MAX_AUTO_CALC             AUTO_CALC_FIELD_CNS
/* indication that the value is not 'number of trunks' but number of 'l2ecmp table entries'*/
#define TRUNK_L2_ECMP_NUM_MEMBERS_INDICATION_CNS          (BIT_31)
/* indicates that 'number of trunks' is equal to 'l2ecmp table entries' */
#define TRUNK_EQUAL_L2_ECMP_NUM_MEMBERS_INDICATION_CNS    (BIT_30)
/* macro to indication that the value is not 'number of trunks' but number of 'l2ecmp table entries'
    + the number of 'l2ecmp table entries' */
#define L2_ECMP_TRUNK_ENTRIES_MAC(numEntries)   (TRUNK_L2_ECMP_NUM_MEMBERS_INDICATION_CNS | (numEntries))

/**
* @struct FINE_TUNING_STC
 *
 * @brief structure for the devices flavors, about table sizes
*/
typedef struct{

    const CPSS_PP_DEVICE_TYPE *devTypePtr;

    /** @brief device parameters being object of fine tuning
     *  Comment:
     */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC config;

} FINE_TUNING_STC;

typedef void GT_VOID_UNIT_IDS_STC;


/**
* @struct PRV_CPSS_DXCH_PP_HW_INFO_STC
 *
 * @brief A Structure including PP's HW major differences between devices.
 * Notes:
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{
    PRV_CPSS_SIP6_UNIT_ID_SIZE_IN_BYTE_GET_FUNC                 prvCpssUnitIdSizeInByteGetFunc;
    PRV_CPSS_SIP6_OFFSET_FROM_FIRST_INSTANCE_GET_FUNC           prvCpssOffsetFromFirstInstanceGetFunc;
    PRV_CPSS_SIP6_DMA_LOCAL_NUM_IN_DP_TO_GLOBAL_NUM_CONVERT_GET_NEXT_FUNC prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc;
    PRV_CPSS_SIP6_HW_REG_ADDR_TO_UNIT_ID_CONRVERT_FUNC          prvCpssHwRegAddrToUnitIdConvertFunc;

    struct{
        GT_BOOL supportIngressRateLimiter;
    }l2i;
    struct{
        GT_U32 cncUnits;
        GT_U32 cncBlocks;
        GT_U32 cncBlockNumEntries;
        GT_BOOL cncUploadUseDfxUnit;
    }cnc;
    struct{
        GT_U32  revision;
        GT_U32  txqUnitsNum;
    }txq;

    struct{
        GT_U32  cpuPortBitIndex;
        /* @brief : if GT_TRUE indication that the device not support tunnel start operations */
        GT_BOOL noTunnelStartSupport;
    }ha;

    struct{
        GT_U32  confiProcessorMemorySize;

        /* number of MG units */
        GT_U32  sip6MgNumOfUnits;

        /* bitmap of port groups that supports the AUQ */
        GT_U32  sip6AuqPortGroupBmp;
        /* bitmap of port groups that supports the FUQ */
        GT_U32  sip6FuqPortGroupBmp;
        /* Falcon (sip6) use separated clock for MG Time Stamp
           the value is in HRZ */
        GT_U32  sip6MgCoreClock;
    }mg;

    struct {
        GT_BOOL  supported;
    } eArch;

    struct{
        GT_U32 numDefaultEports;
        GT_U32 metalFix;
        /* @brief : if not 0 , meaning that the number is limiting the TTI TCAM
            parallel lookups */
        GT_U32 limitedNumOfParrallelLookups;
        /* @brief : if 1 , meaning that the device hold NO TTI TCAM operations ! */
        GT_U32 ttiTcamBypassed;
        /* @brief : if != 0 , hold the number of entries in 'myPhysicalPortAttributes' table */
        GT_U32 TTI_myPhysicalPortAttributes_numEntries;
        /* @brief : if != 0 , hold the number of entries in 'mac2me' table */
        GT_U32 TTI_mac2me_numEntries;
    }tti;

    struct
    {
        GT_U32 iPcl0Bypass;
        /* @brief : if not 0 , meaning that the number is limiting the IPCL0/1/2 TCAM, EPCL TCAM
            parallel lookups */
        GT_U32 limitedNumOfParrallelLookups;
    } pcl;

    struct{
        GT_BOOL supported;
        GT_BOOL enabled;
    }legacyVplsMode;

    struct{
        GT_U32      phyPort;
        GT_U32      trunkId;
        GT_U32      ePort;
        GT_U32      vid;
        GT_U32      vidx;
        GT_U32      eVidx;
        GT_U32      hwDevNum;
        GT_U32      sstId;
        GT_U32      stgId;
        GT_U32      fid;
        GT_U32      fdbUserDefined;
        GT_U32      portGroupId;
    }flexFieldNumBitsSupport;

    struct{
        GT_BOOL     useNonTrunkMembers2Table;
        GT_BOOL     useNonTrunkMembers1TableAsCascade;
        GT_U32      nonTrunkTable1NumEntries;
    }trunk;

    struct{
        GT_BOOL     isNotSupported;
    }dedicatedCpuMac;

    struct{
        GT_BOOL     isUsedPortMappingCheck;
    }portMapping;

    struct{
        GT_U32      maxFloors;
        GT_U32      numBanksForHitNumGranularity;
        GT_U32      maxClientGroups;
        GT_U32      sip6maxTcamGroupId;
        /* @brief : if not 0 , meaning that the number is limiting the all TCAM clients
            parallel lookups */
        GT_U32      limitedNumOfParrallelLookups;
    }tcam;

    struct{
        GT_BOOL notSupportIngressOam;
        GT_BOOL notSupportEgressOam;
    }oam;

    struct{
        PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_INFO_STC info[CPSS_MAX_SDMA_CPU_PORTS_CNS];
        PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_NETIF_INFO_STC netIfInfo[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    }cpuPortInfo;

    struct{
        GT_BOOL     notSupported;
    }gop_lms;

    struct {
        GT_U32  numberOfSmiIf; /* mu,ber of smi interfaces */
        GT_U32  activeSMIList[12];
        GT_BOOL activeSMIBmp [12];
        GT_U32  smiAutoPollingList[12]; /* whether there any out-of-band outpolling ports , ex: BC2 ports 0-47 1G */
        PRV_CPSS_DXCH_UNIT_ENT  sip6_smiUnitIdArr[12];/* SIP6 : explicit unitId of the SMI interface. */
    }smi_support;

    struct{
        GT_BOOL     supportSingleInstance;
        GT_U32      numOfTaiUnits;
        const PRV_CPSS_DXCH_UNIT_ENT  *taiUnitsPtr;/* (if not NULL) pointer to array of TAI units */
    }gop_tai;

    struct{
        GT_BOOL     supported;
    }gop_ilkn;

    struct{
        GT_U32  sip6LanesNumInDev;/* number of serdeses in the device (relevant to sip6 devices.) */
    }serdes;

    struct{
        GT_U32  sip6LedUnitNum;/* number of led units in the device (relevant to sip6 devices.) */
        GT_U32  sip6LedPorts;       /* number of led ports      in the device (relevant to sip6 devices.) */
    }ledInfo;

    struct{/* next are used when not 0 corresponding to the same resource by the
            same name in flexFieldNumBitsSupport */
        GT_U32      phyPort;/* currently for compilation purposes , may be 0 in bobk too */
                            /* (256 ports)BC3: support only 256 (not 512) */
        GT_U32      ePort;/* bobk supports 6k   although 'flex' 13 bits (that imply 8k) */
        GT_U32      vid;  /* bobk supports 4.5k although 'flex' 13 bits (that imply 8k) */
                          /* (512 ports)BC3: support only 4K (not 8K) */
        GT_U32      l2LttMll;/* bobk supports 8k , bc2 32k */
        GT_U32      sstId;/* (512 ports)BC3: support only 2K (not 4K) */

        GT_U32      stgId;  /* sip6: depend on remote physical ports mode (4K:2K:1K:512:256)*/
        GT_U32      vidx;   /* sip6: depend on remote physical ports mode (4K:2K:1K:512:256)*/
        GT_U32      trunkId;/* sip6: depend on remote physical ports mode (4K:2K:1K:512:256)*/
    }limitedResources;/* limitations when flexFieldNumBitsSupport is not enough */

    struct{
        GT_U32 numEntriesEportLttEcmp; /* number of entries in L2/Eport Ltt L2Ecmp */
        GT_U32 numEntriesL2Ecmp;    /* number of entries in L2Ecmp */
        GT_U32 numEntriesL3Ecmp;    /* number of entries in L3Ecmp */
        GT_U32 numEntriesIpNh;      /* number of entries in IP NH */
        GT_U32 numEntriesMllLtt;    /* number of entries in MLL LTT */
        GT_U32 numEntriesMllPairs;  /* number of lines in MLL table (each line hold 2 entries) */
        GT_U32 numEntriesIplrMetering;  /* number of entries in Iplr Metering */
        GT_U32 numEntriesEplrMetering;  /* number of entries in Eplr Metering */
        GT_U32 numEntriesIplrIpfix;     /* number of entries in Iplr Ipfix */
        GT_U32 numEntriesEplrIpfix;     /* number of entries in Eplr Ipfix */
        GT_U32 numEntriesOam;       /* number of entries in the OAM table */
        GT_U32 numEntriesTxqQueue;  /* number of TXQ queues . Not relevant to sip6 devices !!! */
        GT_U32 numEntriesProtectionLoc;/* number of entries in the protection LOC table */
        GT_U32 numEntriesFdb;/* number of entries in the FDB table */
        GT_U32 numEntriesEm ;/* number of entries in the EM  table */
        GT_U32 numEntriesTunnelStart ;/* number of entries in the tunnel table (the 'entry size' is regular TS = (4 ARPs))*/
        GT_U32 numEntriesPortIsolation; /* number of entries in L2/L3 port isolation table.*/
        GT_U32 numEntriesVidx;          /* number of entries in VIDX table.*/
        GT_U32 numEntriesLpmVrfId;      /* number of entries in LPM-VRFID table. (ipv4/6/fcoe)*/

        GT_U32 numEntriesVlanMembers ;/* The HW of Falcon supports 8K entries in vlan members table.
                                   but in 'physical ports mode' 256/512/1024 the number of
                                   ' application vlans' define in limitedResources.vid as 2K (512 mode)
                                   but for the 'tables manager' we need to know to allow access all 8K entries ,
                                   because the 'application vlan' is spread on 4 lines.
                                   */
        GT_U32 numEntriesStgMembers;   /* similar to numEntriesVlanMembers. */
        GT_U32 numEntriesSrcIdMembers; /* number of entries in Source ID Members table.
                                          Calculated according to 'physical ports mode' similar to numEntriesVlanMembers. */
        GT_U32 numEntriesNonTrunkMembers; /* number of entries in Non Trunk Members table.
                                          Calculated according to 'physical ports mode' similar to numEntriesVlanMembers. */
        GT_U32 numEntriesStream ;   /* number of entries in the stream table (smu,preq)*/
        GT_U32 numEntriesStreamSrf; /* number of entries in the stream SRF table (preq) */
        GT_U32 numEntriesStreamSrfHist;/* number of entries in the stream SRF Hist table (preq) */
    }parametericTables;/* tables with different number of entries between the devices */

    struct{
        GT_U32  numOfBlocks ;/* number of memory blocks that LPM uses . bc2 = 20 , bobk = 20 */
        GT_U32  numOfLinesInBlock;/* number of entries in each block memory . bc2 = 16k , bobk = 6k */
        GT_U32  totalNumOfLinesInBlockIncludingGap;/* The gap in lines between the start of 2 consecutive blocks: bc2 = 16K,bobk = 16K */
        GT_U32  indexForPbr;/* index of entry of start of PBR */
        PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT lpmMemMode; /*For bc3 only - LPM memory mode 0h:full mem size or 1h:half mem size*/

        GT_U32  sip6_numLinesPerBlockNonShared;  /* falcon : 640 lines , in non shared block */
        GT_U32  sip6_maxBlocksNonShared; /* falcon : up to 20 blocks from non shared (to fill to total of 30) */
        GT_U32  sip6_maxBlocksWithShared;/* falcon : 30 blocks supported */
    }lpm;

    struct{
        GT_U32  supportMultiDataPath;/* bobk support split RXDMA,Txdam,txFifo.. :
                                    rxdma0 - ports 0..47
                                    rxdma1 - ports 56..59 , 64..71 , 72 (CPU) , 73(TM)*/
        GT_U32  rxNumPortsPerDp;/* per DP : rxdma max number of port supported (size of array)
                                    bobk : 73
                                    bc3 : 13
                                    */
        GT_U32  txNumPortsPerDp;/* per DP : txdma/txfifo max number of port supported (size of array)
                                    bobk : 74
                                    bc3 : 13
                                    */
        PRV_CPSS_DXCH_PP_HW_INFO_DATA_PATH_INFO_STC info[MAX_DP_CNS];
        GT_U32  maxDp;/* number of indexes valid in info[] */
        GT_U32  numTxqDq;/* bobk hold single DQ , but 2 rxdma,txdma,txfifo
                            bobcat3 hold (per pipe) 3 DQ,rxdma,txdma,txfifo */
        GT_U32  supportRelativePortNum;/* Bobcat3 hold relative port number as
                                        index to the DP units.

                                        Bobk - hold GLOBAL port number !
                                        */
        GT_U32  txqDqNumPortsPerDp;/* per DP : number of 'TXQ_ports' that TXQ_DQ supports */
        GT_U32  mapPhyPortToTxqDqUnitIndex[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];/* index to the array is 'Physical port number'
                                    the value is 'TQX-DQ' unit index */
        GT_U32  pdqNumPorts;/* per Tile : number of scheduler port nodes */
        GT_U32  maxDpNumOfQueues;/* per data path : number of queues per dp */
        GT_U32  pdxNumQueueGroups;/* number of entries in 'DX Queue Group Map 0' in PDX unit (CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E) */

        GT_U32  sip6TxPizzaSize;/* sip6 = 33 , sip6.10 = 92 */
        GT_U32  sip6TxNumOfSpeedProfiles;/* sip6 = 8 , sip6.10 = 12 */
    }multiDataPath;

    struct{
        GT_U32  numOfRavens; /* number of Ravens , This device supports some Ravens .*/
        GT_U32  numRavensPerTile;/* number of Ravens per tile .*/
    }ravensInfo;


    struct{
        GT_BOOL     supported;
    }trafficManager;

    GT_U32      maxPhysicalPorts;
    GT_U32      bmpOfTablesInHalfSizeModeArr[(CPSS_DXCH_TABLE_LAST_E/32) + 1];

    struct{
        GT_U32  numOfPpg;/* num of PPGs per PHA/PPA unit */
        GT_U32  numOfPpn;/* num of PPNs per PPG unit */
    }phaInfo;

    GT_U32      tableSizeMode; /* table mode according to maxPhysicalPorts
                                  bobcat3 - halfSizeMode : 1,  otherwise : 0.
                                  falcon  - maxPhysicalPorts = 64  : 0
                                                               128 : 1
                                                               256 : 2
                                                               512 : 3
                                                               1024: 4
                                   */

    /* info about the SIP6 shared memory : SBM */
    struct{
        GT_U32  sharedMemoryNotUsed;/* Falcon , AC5P uses 'shared memory' , AC5X not use it (like sip4,sip5 devices) */
        GT_U32  numOfSuperSbm;     /* in Falcon 9 , In AC5P 10 */
        GT_U32  numOfSbmInSuperSbm;/* in Falcon 4 , In AC5P  4 */
        GT_U32  bmpSbmSupportMultipleArp;/* in Falcon multiple ARP supported in SSBM 8 , in AC5P supported in 8,9 */
        GT_U32  numOfArpsPerSbm;   /* in Falcon 24K , in AC5P 16K */
    }sip6_sbmInfo;

    /* info about the LMU units */
    struct{
        GT_U32  numLmuUnits;/* Falcon : 32 ; AC5P : 4 ; */
    }lmuInfo;

    /* info about Policer Units */
    struct{
        /* overall number of counting/IPFIX entries in all PLR stages
           (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)*/
        GT_U32 countingEntriesNum;

        GT_BOOL notSupportIngressPlr0;
        GT_BOOL notSupportIngressPlr1;
        GT_BOOL notSupportEgressPlr;
    }plrInfo;

    /* info about MACSec Units */
    struct{
        /* Bitmap of Global Data Path with MACSec units
           (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)*/
        GT_U32 macSecDpBmp;
    }macSecInfo;

    struct{
        GT_U32  isIaUnitNotSupported;/* for device above sip 5_10 , this is indication
            that the device NOT support the PIP as not have the IA unit .
            In Ironman the IA unit not exists , and there is no 'ingress oversubscribe' in the device */
    }pipInfo;

    /* pointer to the fine tuning info of the full family */
    const FINE_TUNING_STC *myFamilyFineTuningPtr;
    GT_U32           myFamilyFineTuningSize;

    /* pointer to the units info of the device */
    /* each device cast it according to it's format of 'UNIT_IDS_STC' */
    const GT_VOID_UNIT_IDS_STC    *devUnitsInfoPtr;


    /* info about PPU */
    struct{
        GT_BOOL     notSupported;  /* devices above 6.10 may declare that ppu not supported.
                                      like : Ironman  */
    }ppuInfo;

    /* info about preq */
    struct{
        GT_U32  portMappingNum;/* number of registers for : PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).portMapping[] */
    }preqInfo;

    /* info about ptp */
    struct{
        GT_U32  maxNumOfTais; /* max number of TAIs per device */
        GT_U32  ptpClkInKhz;  /* PTP clock in KHz */
        GT_U32  *frameLatencyArr;
        GT_U32  *frameLatencyFracArr;
    }ptpInfo;

}PRV_CPSS_DXCH_PP_HW_INFO_STC;

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* macro to get pointer to the hwInfo.multiDataPath.maxDp in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_SIP_5_20_MAX_DP_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.multiDataPath.maxDp

/* macro to get pointer to the hwInfo.txq in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_TXQ_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.txq

/* macro to get pointer to the hwInfo.ha in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_HA_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.ha

/* macro to get pointer to the hwInfo.cnc in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.cnc

/* macro to get pointer to the hwInfo.trunk in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.trunk


/* check if txq revision is 1 or above */
#define PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(_devNum)    \
    ((PRV_CPSS_DXCH_PP_HW_INFO_TXQ_MAC(_devNum).revision) ? 1 : 0)

/* txq units number in device  */
#define PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(_devNum)    \
    (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_MAC(_devNum).txqUnitsNum)

/* check if txq units number is 2  */
#define PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(_devNum)    \
    ((PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(_devNum) == 2) ? 1 : 0)

/* convert global port to hemisphere local port  */
#define PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(_devNum,_portNum)    \
    ((PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(_devNum)) ? ((_portNum) & (~BIT_6)) : (_portNum))

/* convert global port to hemisphere number port  */
#define PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(_devNum,_portNum)    \
    (((_portNum) & BIT_6) >> 6)

/* convert hemisphere local port to global port */
#define PRV_CPSS_DXCH_HEM_LOCAL_TO_GLOBAL_PORT(_devNum,_portGroupId,_portNum)    \
    ((PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(_devNum)) ? ((((_portGroupId) & 0x4) << 4) | ((_portNum) & 0x3F)) : (_portNum))

/* get the HA egress cpu port bit index */
#define PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(_devNum) \
    PRV_CPSS_DXCH_PP_HW_INFO_HA_MAC(_devNum).cpuPortBitIndex

/*  ch1 special macro support : cpu port in bit 27 and not 31 in the HA register */
#define PRV_CPSS_DXCH_CH1_SPECIAL_BIT_HA_CPU_PORT_BIT_INDEX_MAC(_devNum , ch1CpuPort) \
    (PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(_devNum))

/* macro to get pointer to the hwInfo.l2i in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_L2I_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.l2i

/* macro to get pointer to the hwInfo.mg in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.mg

/* macro to get pointer to the hwInfo.serdes in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.serdes

/* macro to get pointer to the hwInfo.ledInfo in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.ledInfo

/* macro to get pointer to the hwInfo.ravensInfo in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.ravensInfo

/* macro to check if eArch is supported, return GT_NOT_APPLICABLE_DEVICE if not */
#define PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(_devNum)           \
    if(GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(_devNum))     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not support 'sip5' features")

/* macro to check if SIP6 is supported, return GT_NOT_APPLICABLE_DEVICE if not */
#define PRV_CPSS_DXCH_PP_HW_INFO_SIP6_SUPPORTED_CHECK_MAC(_devNum)           \
    if(GT_FALSE == PRV_CPSS_SIP_6_CHECK_MAC(_devNum))     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not support 'sip6' features")

/* macro to check if 'eArch' supported/enabled. (feature generation 5) */
#define PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC    \
    PRV_CPSS_SIP_5_CHECK_MAC

/* macro to get to the hwInfo.flexFieldNumBitsSupport in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum)    \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.flexFieldNumBitsSupport)

/* macro to get to the hwInfo.limitedResources in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(_devNum)    \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.limitedResources)

/* macro to get max value of resource .
    it selects value from hwInfo.limitedResources if non zero for that resource.
    otherwise selects from hwInfo.flexFieldNumBitsSupport
    NOTE: return 'the last valid value'
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(_devNum,_resource)       \
    ((PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(_devNum)._resource) ?      \
     (PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(_devNum)._resource - 1) :  \
     ((BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum)._resource))))

/* macro to get the last valid index in vlan table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,vid)

/* macro to get the last valid index in eport table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_EPORT_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,eport)

/* macro to get the last valid index in src-id (sstId) table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,sstId)

/* macro to get the last valid index in vidx table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,vidx)

/* macro to get the last valid index in trunkId table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_TRUNK_ID_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,trunkId)

/* macro to get the last valid index in stgId table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_STG_ID_MAC(devNum)  \
    PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,stgId)

/* macro to get maxValue of phyPort */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).phyPort))

/* macro to get maxValue of trunkId */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).trunkId))

/* macro to get maxValue of ePort */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).ePort))

/* macro to get maxValue of vid */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).vid))

/* macro to get maxValue of vidx */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).vidx))

/* macro to get maxValue of eVidx */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).eVidx))

/* macro to get maxValue of hwDevNum */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).hwDevNum))

/* macro to get maxValue of sstId */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).sstId))

/* macro to get maxValue of stgId */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).stgId))

/* macro to get maxValue of fid */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).fid))

/* macro to get maxValue of fdbUserDefined */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FDB_USER_DEFINED_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).fdbUserDefined))

/* macro to get maxValue of portGroupId */
#define PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(_devNum)    \
    (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).portGroupId))

/* macro to get number of bits needed for vid */
#define PRV_CPSS_DXCH_PP_HW_NUM_BITS_VID_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).vid

/* macro to get the last valid index in L2 LTT MLL table.
    NOTE: it may be lower than the number returned by
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum)
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MLL_MAC(_devNum)  \
    ((PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(_devNum).l2LttMll) ?      \
     (PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(_devNum).l2LttMll - 1) :  \
     ((BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum).eVidx))))


/* macro to get the last valid index in eport (L2) LTT ECMP table.
    NOTE: it may be lower than the number of eports
*/
#define PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_ECMP_MAC(_devNum)  \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesEportLttEcmp) ?      \
     (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesEportLttEcmp - 1) :  \
     (PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(_devNum,ePort)))


/* macro to get pointer to the hwInfo.tti in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_TTI_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tti

/* macro to get pointer to the hwInfo.legacyVplsMode in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_MAC(_devNum)    \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.legacyVplsMode

/* macro to get value of hwInfo.legacyVplsMode.supported in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(_devNum)    \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.legacyVplsMode.supported)

/* macro to check if VPLS mode is supported, return GT_NOT_APPLICABLE_DEVICE if not */
#define PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(_devNum)           \
    if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(_devNum))     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "the device not support 'VPLS mode'")

/* macro to get value of hwInfo.legacyVplsMode.enabled in PRV_CPSS_DXCH_PP_HW_INFO_STC */
#define PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_ENABLED_MAC(_devNum)    \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.legacyVplsMode.enabled)

/* internal macro not to be used by any C file */
/* macro to return max port number by number of supported bits.
    the macro takes into account that 'dual device id' actually support 2 times
    the number that the bits support (due to split on 2 deviceIds)
*/
#define ____MAX_PORT_MAC(_devNum,_portType)      \
    ((PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_HW_DEV_NUM_MAC(_devNum))) ? \
        (2 * (BIT_MASK_MAC(PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(_devNum)._portType) + 1)) : \
        (PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(_devNum,_portType) + 1))

#define PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(_devNum) \
    ____MAX_PORT_MAC(_devNum,ePort)

#define PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(_devNum) \
    ____MAX_PORT_MAC(_devNum,phyPort)

/* max number of default ePorts in the device */
#define PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(_devNum) \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tti.numDefaultEports)


/* check that the port number is valid return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_PORT_CHECK_MAC(_devNum,_portNum)                \
    if ((_portNum) >= PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(_devNum))     \
    {                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portNum[%d]>=[%d]",\
            _portNum,PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(_devNum));\
    }

/* check that the physical port number is valid return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(_devNum,_portNum)        \
    if ((_portNum) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(_devNum) || \
        ((_portNum) >= CPSS_MAX_PORTS_NUM_CNS)) /* fake check for klockwork */ \
    {                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physical portNum[%d]>=[%d]",\
            _portNum,PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(_devNum));\
    }

/* check that the physical port number is NOT reserved, return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_PHY_PORT_RESERVED_CHECK_MAC(_devNum,_portNum)         \
    if (prvCpssDxChPortReservedPortCheck(_devNum, _portNum) == GT_TRUE)     \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Reserved port[%d], can not be used!", _portNum); \
    }

/* check that the port number is valid for 'default ePort' . return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(_devNum,_portNum)                \
    if ((_portNum) >= PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(_devNum))     \
    {                                                                          \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "default ePortNum[%d]>=[%d]",\
            _portNum,PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(_devNum));     \
    }


/* check that port mapping access check is used */
#define PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.portMapping.isUsedPortMappingCheck)


/* For legacy devices: regular physical port check.
   For E_ARCH devices: check and get MAC number for given physical port */
#define PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum)  \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &_portMacNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum,_portNum);                             \
        _portMacNum = _portNum;                                                         \
    }

/* For legacy devices: regular physical port check include CPU port.
   For E_ARCH devices: check and get MAC number for given physical port, NOT include
                       CPU port since it has no MAC */
#define PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum)  \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &_portMacNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum,_portNum);                 \
        _portMacNum = _portNum;                                                         \
    }

/* For legacy devices: regular physical port check.
   For E_ARCH devices: check and get TXQ number for given physical port */
#define PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(_devNum,_portNum,_portTxqNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &_portTxqNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum,_portNum);                             \
        _portTxqNum = _portNum;                                                         \
    }

/* For legacy devices: regular physical port check include CPU port.
   For E_ARCH devices: check and get TXQ number for given physical port, include CPU port */
#define PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXQ_PORT_GET_MAC(_devNum,_portNum,_portTxqNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &_portTxqNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum,_portNum);                 \
        _portTxqNum = _portNum;                                                         \
    }

/* For legacy devices: regular physical port check.
   For E_ARCH devices: check and get RX_DMA number for given physical port */
#define PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(_devNum,_portNum,_portRxdmaNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, &_portRxdmaNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum,_portNum);                             \
        _portRxdmaNum = _portNum;                                                       \
    }

/* For legacy devices: regular physical port check include CPU port.
   For E_ARCH devices: check and get RX_DMA number for given physical port, include CPU port */
#define PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(_devNum,_portNum,_portRxdmaNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, &_portRxdmaNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum,_portNum);                 \
        _portRxdmaNum = _portNum;                                                       \
    }

/* For legacy devices: regular physical port check.
   For E_ARCH devices: check and get TX_DMA number for given physical port */
#define PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(_devNum,_portNum,_portTxdmaNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E, &_portTxdmaNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(_devNum,_portNum);                             \
        _portTxdmaNum = _portNum;                                                       \
    }

/* For legacy devices: regular physical port check include CPU port.
   For E_ARCH devices: check and get TX_DMA number for given physical port, include CPU port */
#define PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(_devNum,_portNum,_portTxdmaNum) \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) == GT_TRUE)       \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E, &_portTxdmaNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(_devNum,_portNum);                 \
        _portTxdmaNum = _portNum;                                                       \
    }


/* For 'remote' port  : check and get LOCAL MAC number for the physical port.
   For non-remote port: check and get LOCAL MAC number for the physical port.
*/
#define PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(_devNum,_portNum,_portMacNum)  \
    if(prvCpssDxChPortRemotePortCheck(_devNum,_portNum))                                \
    { /*only sip5 devices can get here */                                               \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &_portMacNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum);\
    }

/* For 'remote' port  : check and get LOCAL MAC number for the physical port.
   For non-remote port: check and get LOCAL MAC number for the physical port.
*/
#define PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(_devNum,_portNum,_portMacNum)  \
    if(prvCpssDxChPortRemotePortCheck(_devNum,_portNum))                                \
    { /*only sip5 devices can get here */                                               \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(_devNum, _portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &_portMacNum);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum);\
    }

/* MACRO for 'MAC-in-PHY' APIs that need to get the MAC number for configuring */
#define PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum)  \
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(_devNum,_portNum,_portMacNum)

/* MACRO for 'MAC-in-PHY' APIs that need to get the MAC number for configuring */
#define PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum)  \
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(_devNum,_portNum,_portMacNum)


/* check if tableId is in half size mode */
#define PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(_devNum,tableId)  \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.bmpOfTablesInHalfSizeModeArr[(tableId)>>5] & 1<<((tableId)&0x1f)) ? 1 : 0)

/* set tableId to in half size mode */
#define PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_SET_MAC(_devNum,tableId)  \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.bmpOfTablesInHalfSizeModeArr[(tableId)>>5] |= 1<<((tableId)&0x1f)

/* clear tableId from half size mode */
#define PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_CLEAR_MAC(_devNum,tableId)  \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.bmpOfTablesInHalfSizeModeArr[(tableId)>>5] &= ~(1<<((tableId)&0x1f))


/* set tables size mode */
#define PRV_CPSS_DXCH_TABLES_SIZE_MODE_SET_MAC(_devNum,sizeMode)  \
   (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tableSizeMode = sizeMode)


/* Get tables size mode */
#define PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum)  \
    (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tableSizeMode)

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
);

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
);

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
);
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
);

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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapReverseMappingGet
(
    IN  GT_U8                           devNum,
    IN  PRV_CPSS_DXCH_PORT_TYPE_ENT     origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
);

/**
* @internal prvCpssDxChPortEventPortMapConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping for port per event type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
);

/**
* @internal prvCpssDxChPortMappingCPUPortGet function
* @endinternal
*
* @brief   Function checks and returns the number of SDMA CPU ports
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuPortNumPtr            -  array with the physical port bumbers of the SDMA CPU ports
* @param[out] numOfCpuPorts            - (pointer to) the number of SDMA CPU ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMappingCPUPortGet
(
     IN  GT_U8                devNum,
     OUT GT_PHYSICAL_PORT_NUM *cpuPortNumPtr, /*array of size CPSS_MAX_SDMA_CPU_PORTS_CNS*/
     OUT GT_U32               *numOfCpuPorts
);

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
);
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
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChInfoEnhancedh */


