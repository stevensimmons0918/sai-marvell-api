/*******************************************************************************
*              (c                   ), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file PP-98EX5520.c
*
* @brief PP-98EX5520 Packet Processor information.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

/* List of Tx queues to work in generator mode */
static CPSS_APP_PLATFORM_SDMA_QUEUE_STC txGenQueues[] = { {0,3,256,144}, {0,6,256,144} };

static CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC aldrin2_mac_ledPos_Arr[] =
{
      /* portMac           ledIF,    ledPosition,    */
     /* DP 0*/
     {       0           , 0,    0              }
    ,{       4           , 0,    1              }
    ,{       8           , 0,    2              }
     /* DP 1*/
    ,{      12           , 1,    0              }
    ,{      16           , 1,    1              }
    ,{      20           , 1,    2              }
    /* DP 2 */
    ,{      24           , 3,    0              }
    ,{      25           , 3,    1              }
    ,{      26           , 3,    2              }
    ,{      27           , 3,    3              }
    ,{      28           , 3,    4              }
    ,{      29           , 3,    5              }
    ,{      30           , 3,    6              }
    ,{      31           , 3,    7              }
    ,{      32           , 3,    8              }
    ,{      33           , 3,    9              }
    ,{      34           , 3,   10              }
    ,{      35           , 3,   11              }
    ,{      36           , 3,   12              }
    ,{      37           , 3,   13              }
    ,{      38           , 3,   14              }
    ,{      39           , 3,   15              }
    ,{      40           , 3,   16              }
    ,{      41           , 3,   17              }
    ,{      42           , 3,   18              }
    ,{      43           , 3,   19              }
    ,{      44           , 3,   20              }
    ,{      45           , 3,   21              }
    ,{      46           , 3,   22              }
    ,{      47           , 3,   23              }
    /* CPU */
    ,{      72           , 3,   24              }
    /* DP-3 */
    ,{      48           , 2,    0              }
    ,{      49           , 2,    1              }
    ,{      50           , 2,    2              }
    ,{      51           , 2,    3              }
    ,{      52           , 2,    4              }
    ,{      53           , 2,    5              }
    ,{      54           , 2,    6              }
    ,{      55           , 2,    7              }
    ,{      56           , 2,    8              }
    ,{      57           , 2,    9              }
    ,{      58           , 2,   10              }
    ,{      59           , 2,   11              }
    ,{      60           , 2,   12              }
    ,{      61           , 2,   13              }
    ,{      62           , 2,   14              }
    ,{      63           , 2,   15              }
    ,{      64           , 2,   16              }
    ,{      65           , 2,   17              }
    ,{      66           , 2,   18              }
    ,{      67           , 2,   19              }
    ,{      68           , 2,   20              }
    ,{      69           , 2,   21              }
    ,{      70           , 2,   22              }
    ,{      71           , 2,   23              }
    ,{CAP_BAD_VALUE  , CAP_BAD_VALUE, CAP_BAD_VALUE  }
};

static CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC  aldrin2_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_FALSE },
    { 1,    GT_TRUE,    GT_TRUE  },
    { 2,    GT_TRUE,    GT_FALSE },
    { 3,    GT_TRUE,    GT_FALSE },
    { 4,    GT_FALSE,   GT_TRUE  },
    { 5,    GT_FALSE,   GT_TRUE  },
    { 6,    GT_FALSE,   GT_TRUE  },
    { 7,    GT_TRUE,    GT_FALSE },
    { 8,    GT_TRUE,    GT_TRUE  },
    { 9,    GT_FALSE,   GT_TRUE  },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_TRUE,    GT_FALSE },
    { 12,   GT_TRUE,    GT_TRUE  },
    { 13,   GT_FALSE,   GT_TRUE  },
    { 14,   GT_FALSE,   GT_TRUE  },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_TRUE,    GT_TRUE  },
    { 17,   GT_TRUE,    GT_TRUE  },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_TRUE,    GT_TRUE  },
    { 21,   GT_TRUE,    GT_TRUE  },
    { 22,   GT_TRUE,    GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_FALSE },
    { 26,   GT_TRUE,    GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_TRUE,    GT_FALSE },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_TRUE,    GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_FALSE },
    { 33,   GT_TRUE,    GT_FALSE },
    { 34,   GT_TRUE,    GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_TRUE,    GT_FALSE },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_FALSE },
    { 42,   GT_TRUE,    GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_TRUE,    GT_FALSE },
    { 45,   GT_TRUE,    GT_FALSE },
    { 46,   GT_FALSE,   GT_TRUE  },
    { 47,   GT_FALSE,   GT_FALSE },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_TRUE,    GT_TRUE  },
    { 50,   GT_TRUE,    GT_FALSE },
    { 51,   GT_TRUE,    GT_TRUE  },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_TRUE,    GT_TRUE  },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_TRUE  },
    { 56,   GT_TRUE,    GT_FALSE },
    { 57,   GT_FALSE,   GT_TRUE  },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_TRUE,    GT_TRUE  },
    { 60,   GT_TRUE,    GT_FALSE },
    { 61,   GT_FALSE,   GT_TRUE  },
    { 62,   GT_TRUE,    GT_FALSE },
    { 63,   GT_TRUE,    GT_TRUE  },
    { 64,   GT_FALSE,   GT_FALSE },
    { 65,   GT_TRUE,    GT_TRUE  },
    { 66,   GT_FALSE,   GT_FALSE },
    { 67,   GT_TRUE,    GT_TRUE  },
    { 68,   GT_TRUE,    GT_FALSE },
    { 69,   GT_TRUE,    GT_TRUE  },
    { 70,   GT_FALSE,   GT_FALSE },
    { 71,   GT_TRUE,    GT_FALSE },
    { 72,   GT_FALSE,   GT_FALSE }
};

/** Macro for PP Phase 1 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_AP \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x02,\
    _SM_(appAddrCompletionRegionsBmp) 0x3C,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 0,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0,\
    _SM_(apEnable                   ) GT_TRUE,\
    _SM_(numOfDataIntegrityElements ) 0xFFFFFFFF

#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_NON_AP \
    _SM_(coreClock                  ) CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS,\
    _SM_(ppHAState                  ) CPSS_SYS_HA_MODE_ACTIVE_E,\
    _SM_(serdesRefClock             ) CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,\
    _SM_(isrAddrCompletionRegionsBmp) 0x02,\
    _SM_(appAddrCompletionRegionsBmp) 0x3C,\
    _SM_(numOfPortGroups            ) 1,\
    _SM_(maxNumOfPhyPortsToUse      ) 0,\
    _SM_(tcamParityCalcEnable       ) 0,\
    _SM_(cpssWaList                 ) NULL,\
    _SM_(cpssWaNum                  ) 0,\
    _SM_(apEnable                   ) GT_FALSE,\
    _SM_(numOfDataIntegrityElements ) 0xFFFFFFFF


/** Macro for PP Phase 2 Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS \
    _SM_(newDevNum                  ) 32, /* to be modified */ \
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
    _SM_(ledPositionTable           ) aldrin2_mac_ledPos_Arr,\
    /*Led Class manipulation config*/\
    _SM_(invertionEnable            ) GT_FALSE,\
    _SM_(blinkSelect                ) CPSS_LED_BLINK_SELECT_0_E,\
    _SM_(forceEnable                ) GT_FALSE,\
    _SM_(forceData                  ) 0,\
    _SM_(pulseStretchEnable         ) GT_FALSE

/** Macro for PP Logical Init Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS \
    /*Logical Init*/\
    _SM_(routingMode                ) CPSS_DXCH_TCAM_ROUTER_BASED_E,\
    _SM_(maxNumOfPbrEntries         ) 8192,\
    _SM_(lpmMemoryMode              ) CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,\
    _SM_(sharedTableMode            ) CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E,\
    /*General Init*/\
    _SM_(ctrlMacLearn               ) GT_TRUE,\
    _SM_(flowControlDisable         ) GT_TRUE,\
    _SM_(modeFcHol                  ) GT_TRUE,\
    _SM_(mirrorAnalyzerPortNum      ) 0,\
    /*Cascade Init*/\
    _SM_(cascadePtr                 ) NULL

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
    _SM_(initPolicer                ) GT_FALSE,\
    _SM_(initPha                    ) GT_FALSE,\
    /* Ip Lib Init Params*/\
    _SM_(blocksAllocationMethod     ) 0,\
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
    _SM_(numOfTrunks                ) 8191,\
    _SM_(trunkMembersMode           ) CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E,\
    /* Pha Lib Init Params*/\
    _SM_(pha_packetOrderChangeEnable) GT_FALSE,\
    /* Bridge Lib Init Params*/\
    _SM_(policerMruSupported        ) GT_TRUE

/** Macro for PP serdes polarity Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(polarityPtr                ) aldrin2_DB_PolarityArray, \
    _SM_(polarityArrSize            ) sizeof(aldrin2_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC)

/** Macro for PP serdes mux Params **/
#define CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS \
    /*Serdes Params*/\
    _SM_(serdesMapPtr               ) NULL, \
    _SM_(serdesMapArrSize           ) 0


/** Macro for Internal Cpu **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS \
    /*Serdes Params*/\
    _SM_(internalCpu                ) GT_FALSE

/** Macro for MPD init **/
#define CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS \
    /*Serdes Params*/\
    _SM_(initMpd                ) GT_FALSE

/** Macro for belly2belly handling disable **/
#define CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS \
    _SM_(belly2belly                ) GT_FALSE

CPSS_APP_PLATFORM_PP_PROFILE_STC DB_98EX5520_PP_INFO = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_NON_AP,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};

CPSS_APP_PLATFORM_PP_PROFILE_STC DB_98EX5520_PP_INFO_AP = {
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE1_DEFAULTS_AP,
    CPSS_APP_PLATFORM_PP_PROFILE_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_AFTER_PHASE2_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LOGICAL_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_LIB_INIT_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_SERDES_MUX_PARAMS_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INTERNAL_CPU_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_INIT_MPD_DEFAULTS,
    CPSS_APP_PLATFORM_PP_PROFILE_BELLY2BELLY_DISABLE_DEFAULTS,
};
