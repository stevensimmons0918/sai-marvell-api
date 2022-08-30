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
* @file PP-WM-AC5X.c
*
* @brief PP-WM-AC5X Packet Processor information.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

/* List of Tx queues to work in generator mode */
static CPSS_APP_PLATFORM_SDMA_QUEUE_STC txGenQueues[] = { {0,3,256,144}, {0,6,256,144} };

static CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC ac5x_DB_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
        { 11,   GT_FALSE,    GT_TRUE }
};

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE \
    _SM_(apEnable                   ) GT_FALSE, /* currently AP mode is not supported in cpss for AC5x - will be enabled once support is added */ \
    _SM_(numOfDataIntegrityElements ) 0x0

/** Macro for PP Phase 2 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS \
    _SM_(newDevNum                  ) 0, /* to be modified */ \
    _SM_(auDescNum                  ) 2048,\
    _SM_(auMessageLength            ) CPSS_AU_MESSAGE_LENGTH_8_WORDS_E,\
    _SM_(useDoubleAuq               ) GT_FALSE,\
    _SM_(useSecondaryAuq            ) GT_FALSE,\
    _SM_(fuDescNum                  ) 2048,\
    _SM_(fuqUseSeparate             ) GT_TRUE,\
    _SM_(noTraffic2CPU              ) GT_FALSE,\
    _SM_(cpuPortMode                ) CPSS_NET_CPU_PORT_MODE_SDMA_E,\
    _SM_(useMultiNetIfSdma          ) GT_TRUE,\
    _SM_(netifSdmaPortGroupId       ) 0,\
    _SM_(txDescNum                  ) 1000,\
    _SM_(rxDescNum                  ) 200,\
    _SM_(rxAllocMethod              ) CPSS_RX_BUFF_STATIC_ALLOC_E,\
    _SM_(rxBuffersInCachedMem       ) GT_FALSE,\
    _SM_(rxHeaderOffset             ) 0,\
    _SM_(rxBufferPercentage         ) {13,13,13,13,12,12,12,12},\
    _SM_(txGenQueueList             ) txGenQueues,\
    _SM_(txGenQueueNum              ) sizeof(txGenQueues)/sizeof(txGenQueues[0]),\
    /* NetIf Lib Init Params*/\
    _SM_(miiTxDescNum               ) 0,\
    _SM_(miiTxBufBlockSize          ) 0,\
    _SM_(miiRxBufSize               ) 0,\
    _SM_(miiRxBufBlockSize          ) 0,\
    _SM_(miiRxHeaderOffset          ) 0,\
    _SM_(miiRxBufferPercentage      ) {13,13,13,13,12,12,12,12}

/** Macro for PP after Phase 2 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS \
    /* Led Init*/\
    _SM_(ledOrganize                ) CPSS_LED_ORDER_MODE_BY_CLASS_E,\
    _SM_(ledClockFrequency          ) CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E,\
    _SM_(invertEnable               ) GT_TRUE,\
    _SM_(pulseStretch               ) CPSS_LED_PULSE_STRETCH_1_E,\
    _SM_(blink0Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink0DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(blink1Duration             ) CPSS_LED_BLINK_DURATION_1_E,\
    _SM_(blink1DutyCycle            ) CPSS_LED_BLINK_DUTY_CYCLE_1_E,\
    _SM_(disableOnLinkDown          ) GT_FALSE,\
    _SM_(clkInvert                  ) GT_FALSE,\
    _SM_(class5select               ) CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E,\
    _SM_(class13select              ) CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E,\
    _SM_(ledStart                   ) 64,\
    _SM_(ledEnd                     ) 64,\
    _SM_(ledPositionTable           ) NULL,\
    /*Led Class manipulation config*/\
    _SM_(invertionEnable            ) GT_FALSE,\
    _SM_(blinkSelect                ) CPSS_LED_BLINK_SELECT_0_E,\
    _SM_(forceEnable                ) GT_FALSE,\
    _SM_(forceData                  ) 0,\
    _SM_(pulseStretchEnable         ) GT_FALSE

/** Macro for PP Lib Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS \
    _SM_(initBridge                 ) GT_TRUE,\
    _SM_(initIp                     ) GT_TRUE,\
    _SM_(initMirror                 ) GT_TRUE,\
    _SM_(initNetIf                  ) GT_TRUE,\
    _SM_(initPhy                    ) GT_TRUE,\
    _SM_(initPort                   ) GT_TRUE,\
    _SM_(initTrunk                  ) GT_TRUE,\
    _SM_(initPcl                    ) GT_TRUE,\
    _SM_(initTcam                   ) GT_TRUE,\
    _SM_(initPolicer                ) GT_TRUE,\
    _SM_(initPha                    ) GT_TRUE,\
    /* Ip Lib Init Params*/\
    _SM_(blocksAllocationMethod     ) CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E,\
    _SM_(usePolicyBasedRouting      ) GT_FALSE,\
    _SM_(maxNumOfIpv4Prefixes       ) 1000,\
    _SM_(maxNumOfIpv4McEntries      ) 100,\
    _SM_(maxNumOfIpv6Prefixes       ) 100,\
    _SM_(lpmDbFirstTcamLine         ) 100,\
    _SM_(lpmDbLastTcamLine          ) 1003,\
    _SM_(lpmDbSupportIpv4           ) GT_TRUE,\
    _SM_(lpmDbSupportIpv6           ) GT_TRUE,\
    _SM_(lpmDbPartitionEnable       ) GT_TRUE,\
    /* Trunk Lib Init Params*/\
    _SM_(numOfTrunks                ) 0x1000,\
    _SM_(trunkMembersMode           ) CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E,\
    /* Pha Lib Init Params*/\
    _SM_(pha_packetOrderChangeEnable) GT_FALSE,\
    /* Bridge Lib Init Params*/\
    _SM_(policerMruSupported        ) GT_FALSE

/** Macro for PP Phase 1 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x0,\
    _SM_(appAddrCompletionRegionsBmp) 0x0,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 128,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0

/** Macro for PP Logical Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS \
    /*Logical Init*/\
    _SM_(routingMode                ) CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E, \
    _SM_(maxNumOfPbrEntries         ) 5120,\
    _SM_(lpmMemoryMode              ) CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,\
    _SM_(sharedTableMode            ) CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E,\
    /*General Init*/\
    _SM_(ctrlMacLearn               ) GT_TRUE,\
    _SM_(flowControlDisable         ) GT_TRUE,\
    _SM_(modeFcHol                  ) GT_FALSE,\
    _SM_(mirrorAnalyzerPortNum      ) 0,\
    /*Cascade Init*/\
    _SM_(cascadePtr                 ) NULL

/** Macro for PP serdes polarity Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(polarityPtr                ) ac5x_DB_PolarityArray, \
    _SM_(polarityArrSize            ) sizeof(ac5x_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC)

/** Macro for PP serdes mux Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(serdesMapPtr               ) NULL, \
    _SM_(serdesMapArrSize           ) 0


/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_NO_INTERNAL_CPU \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_FALSE

/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_TRUE

/** Macro for MPD init **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS \
    /*Serdes Params*/\
    _SM_(initMpd                ) GT_FALSE

/** Macro for belly2belly handling disable **/
#define CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS \
    _SM_(belly2belly                ) GT_FALSE

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_AC5X_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_NO_INTERNAL_CPU,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};

CPSS_APP_PLATFORM_PP_PROFILE_STC WM_AC5X_INT_CPU_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_128_PHY,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_AP_ENABLE,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};
