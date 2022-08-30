/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformPpProfile.h
*
* @brief This file contains Packet processor profile definitions.
*
* @version   1
********************************************************************************
*/
#ifndef __CPSS_APP_PLATFORM_PP_PROFILE_H
#define __CPSS_APP_PLATFORM_PP_PROFILE_H

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#endif

#define CAP_BAD_VALUE       (GT_U32)(~0)

#define CPSS_APP_PLATFORM_MAX_TRUNKS_CNS      3
#define CPSS_APP_PLATFORM_MAX_TRUNK_PORTS_CNS 128

typedef struct _CPSS_APP_PLATFORM_SDMA_QUEUE_STC
{
    GT_U32 sdmaPortNum;
    GT_U8  queueNum;
    /* Number of Tx buffers for a tx queue */
    /* Note: num of Tx Buffers == num of Tx descriptors */
    GT_U32 numOfTxBuff;
    GT_U32 txBuffSize;
}CPSS_APP_PLATFORM_SDMA_QUEUE_STC;

typedef struct
{
    GT_U32  portMac;
    GT_U32  ledIf;
    GT_U32  ledPosition;
}CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC;

typedef struct
{
    GT_U32      trunkId;
    GT_U32      numCscdPorts;
    CPSS_CSCD_PORT_TYPE_ENT     cscdPortType;                    /* Enum of cascade port or network port */
    GT_U32      portList[CPSS_APP_PLATFORM_MAX_TRUNK_PORTS_CNS];
} CPSS_APP_PLATFORM_CSCD_TRUNK_INFO_STC;

/*
 * @struct _CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC.
 * @brief: Casacade Port info.
 */
typedef struct _CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC {

    GT_U32  numCscdTrunks;
    GT_U32  srcDevNum;
    GT_U32  trgtDevNum;
    CPSS_APP_PLATFORM_CSCD_TRUNK_INFO_STC   trunkInfo[CPSS_APP_PLATFORM_MAX_TRUNKS_CNS];
} CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC;

/*
 * @struct: CPSS_APP_PLATFORM_PP_PROFILE_STC
 * @brief: This hold the Phase-1, Phase-2 and post Phase2 related information which
 *         is used to initialize packet processor in a board.
 */

/**
* @struct CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC
 *
 * @brief Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*/
typedef struct{

    /** number of SERDES lane */
    GT_U32 laneNum;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertTx;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertRx;

} CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC;

typedef struct _CPSS_APP_PLATFORM_PP_PROFILE_STC
{
    /*** PP Phase-1 Parameters ***/
    GT_U32                                         coreClock;
    CPSS_SYS_HA_MODE_ENT                           ppHAState;
    CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT              serdesRefClock;
    GT_U32                                         isrAddrCompletionRegionsBmp;
    GT_U32                                         appAddrCompletionRegionsBmp;
    GT_U32                                         numOfPortGroups;
    GT_U32                                         maxNumOfPhyPortsToUse;
    GT_U32                                         tcamParityCalcEnable;
    CPSS_DXCH_IMPLEMENT_WA_ENT                     *cpssWaList;
    GT_U32                                         cpssWaNum;
    GT_BOOL                                        apEnable;
    GT_U32                                         numOfDataIntegrityElements;

    /*** PP Phase-2 Parameters ***/
    GT_U32                                         newDevNum;
    /*** AU queue params ***/
    GT_U32                                         auDescNum;
    CPSS_AU_MESSAGE_LENGTH_ENT                     auMessageLength;
    GT_BOOL                                        useDoubleAuq;
    GT_BOOL                                        useSecondaryAuq;
    GT_U32                                         fuDescNum;
    GT_BOOL                                        fuqUseSeparate;

    GT_BOOL                                        noTraffic2CPU;
    CPSS_NET_CPU_PORT_MODE_ENT                     cpuPortMode;

    /*** SDMA params ***/
    GT_BOOL                                        useMultiNetIfSdma;
    GT_U32                                         netifSdmaPortGroupId;
    GT_U32                                         txDescNum;
    GT_U32                                         rxDescNum;
    CPSS_RX_BUFF_ALLOC_METHOD_ENT                  rxAllocMethod;
    GT_BOOL                                        rxBuffersInCachedMem;
    GT_U32                                         rxHeaderOffset;
    GT_U32                                         rxBufferPercentage[CPSS_MAX_RX_QUEUE_CNS]; /* Applicable only if non multiNetIfSdma mode */
    CPSS_APP_PLATFORM_SDMA_QUEUE_STC               *txGenQueueList; /* List of Tx queues to work in generator mode */
    GT_U32                                         txGenQueueNum;

    /*** MII Params ***/
    GT_U32                                         miiTxDescNum;
    GT_U32                                         miiTxBufBlockSize;
    GT_U32                                         miiRxBufSize;
    GT_U32                                         miiRxBufBlockSize;
    GT_U32                                         miiRxHeaderOffset;
    GT_U32                                         miiRxBufferPercentage[CPSS_MAX_RX_QUEUE_CNS];

    /*** Post Phase-2 Parameters ***/
    /* Led Init*/
    CPSS_LED_ORDER_MODE_ENT                        ledOrganize;
    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT               ledClockFrequency;
    GT_BOOL                                        invertEnable;
    CPSS_LED_PULSE_STRETCH_ENT                     pulseStretch;
    CPSS_LED_BLINK_DURATION_ENT                    blink0Duration;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT                  blink0DutyCycle;
    CPSS_LED_BLINK_DURATION_ENT                    blink1Duration;
    CPSS_LED_BLINK_DUTY_CYCLE_ENT                  blink1DutyCycle;
    GT_BOOL                                        disableOnLinkDown;
    GT_BOOL                                        clkInvert;
    CPSS_LED_CLASS_5_SELECT_ENT                    class5select;
    CPSS_LED_CLASS_13_SELECT_ENT                   class13select;
    GT_U32                                         ledStart;
    GT_U32                                         ledEnd;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC          *ledPositionTable;

    /*Led Class manipulation config*/
    GT_BOOL                                        invertionEnable;
    CPSS_LED_BLINK_SELECT_ENT                      blinkSelect;
    GT_BOOL                                        forceEnable;
    GT_U32                                         forceData;
    GT_BOOL                                        pulseStretchEnable;

    /*Logical Init*/
    CPSS_DXCH_CFG_ROUTING_MODE_ENT                 routingMode;
    GT_U32                                         maxNumOfPbrEntries;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT                 lpmMemoryMode;
    CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT            sharedTableMode;

    /*General Init*/
    GT_BOOL                                        ctrlMacLearn;
    GT_BOOL                                        flowControlDisable;
    GT_BOOL                                        modeFcHol;
    GT_U32                                         mirrorAnalyzerPortNum;

    /*Cascade Init*/
    CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC         *cascadePtr;

    /*Lib Init*/
    GT_BOOL                                        initBridge;
    GT_BOOL                                        initIp;
    GT_BOOL                                        initMirror;
    GT_BOOL                                        initNetIf;
    GT_BOOL                                        initPhy;
    GT_BOOL                                        initPort;
    GT_BOOL                                        initTrunk;
    GT_BOOL                                        initPcl;
    GT_BOOL                                        initTcam;
    GT_BOOL                                        initPolicer;
    GT_BOOL                                        initPha;

    /** Lib Init Parameters **/
    /* IP Lib Params **/
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT blocksAllocationMethod;
    GT_BOOL                                        usePolicyBasedRouting;
    GT_U32                                         maxNumOfIpv4Prefixes;
    GT_U32                                         maxNumOfIpv4McEntries;
    GT_U32                                         maxNumOfIpv6Prefixes;
    GT_U32                                         lpmDbFirstTcamLine;
    GT_U32                                         lpmDbLastTcamLine;
    GT_BOOL                                        lpmDbSupportIpv4;
    GT_BOOL                                        lpmDbSupportIpv6;
    GT_BOOL                                        lpmDbPartitionEnable;
    /* Trunk Lib Params **/
    GT_U32                                         numOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT               trunkMembersMode;
    /* Pha Lib Params **/
    GT_BOOL                                        pha_packetOrderChangeEnable;

    /* Bridge Lib Params **/
    GT_BOOL                                        policerMruSupported;

    /* Serdes Polarity Params **/
    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC     *polarityPtr;
    GT_U32                                         polarityArrSize;

    /* Serdes Mux Params **/
    CPSS_PORT_MAC_TO_SERDES_STC                    *serdesMapPtr;
    GT_U32                                         serdesMapArrSize;

    /*Internal Cpu*/
    GT_BOOL                                        internalCpu;
    /*MPD Init*/
    GT_BOOL                                        initMpd;

    /*belly2belly enable*/
    GT_BOOL                                         belly2belly;
} CPSS_APP_PLATFORM_PP_PROFILE_STC;

#endif /* __CPSS_APP_PLATFORM_PP_PROFILE_H */
