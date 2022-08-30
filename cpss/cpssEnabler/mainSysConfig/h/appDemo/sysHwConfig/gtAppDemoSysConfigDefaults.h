/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoSysConfigDefaults.h
*
* @brief System configuration and initialization structres default values.
*
* @version   46
********************************************************************************
*/
#ifndef __gtAppDemoSysConfigDefaultsh
#define __gtAppDemoSysConfigDefaultsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/*******************************************************************************
 * Defualt values for the different configuration structs.
 ******************************************************************************/

/****************************************************************************/
/* GT_PP_PHASE1_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define CPSS_PP_PHASE1_DEFAULT                                  \
{                                                               \
    0,                              /* devNum           */      \
    {                                                           \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF,                                   \
        CPSS_HW_INFO_STC_DEF                                    \
    },                              /* hwInfo[]         */      \
    1,                              /* numOfPortGroups  */      \
    0,                              /* deviceId         */      \
    0,                              /* revision         */      \
    166,                            /* coreClk          */      \
    CPSS_CHANNEL_PCI_E,             /* mngInterfaceType */      \
    CPSS_BUF_1536_E,                /* maxBufSize           */  \
    CPSS_SYS_HA_MODE_ACTIVE_E,      /* sysHAState           */  \
    APPDEMO_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,        \
                                    /*serdesRefClock*/          \
    GT_FALSE,/*tcamParityCalcEnable*/                           \
    /* relevant only to 8-region address completion mechanism*/ \
    2,/*isrAddrCompletionRegionsBmp*/  /* region 1 */           \
    0xFC, /*appAddrCompletionRegionsBmp*/  /* regions 2-7 */    \
    GT_FALSE,                 /*enableLegacyVplsModeSupport */  \
    /*ha2phaseInitParams struct */                              \
    {                                                           \
        0,                                                      \
        0,                                                      \
        0                                                       \
    }                                                           \
}



/* Dummy for competability, for init process. */
#define GT_DUMMY_REG_VAL_INFO_LIST                                          \
{                                                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},                                \
    {0x00000001, 0x00000000, 0x00000000, 0},                                \
    {0x00000002, 0x00000000, 0x00000000, 0},                                \
    {0x00000003, 0x00000000, 0x00000000, 0},                                \
    {0x00000004, 0x00000000, 0x00000000, 0},                                \
    {0x00000005, 0x00000000, 0x00000000, 0},                                \
    {0x00000006, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter        */      \
}

/****************************************************************************/
/* CPSS_PP_PHASE2_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define RX_DESC_NUM_DEF         200
#define TX_DESC_NUM_DEF         1000
#define AU_DESC_NUM_DEF         2048
#define FALCON_AU_DESC_NUM_DEF  3072
#define RX_BUFF_SIZE_DEF        1548
#define RX_BUFF_ALLIGN_DEF      1
#define APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF  7
#define APP_DEMO_RX_BUFF_ALLIGN_DEF      (1 << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
#define APP_DEMO_RX_BUFF_SIZE_DEF        800

#define GT_PCL_ACTION_SIZE_16_BYTE 16

/* 7 trunks supported by : EXMXTC , EXMXSAM  */
#define APP_DEMO_7_TRUNKS_CNS      7

/* 31 trunks supported by : DXSAL , EXMXTD , EXMXTG */
#define APP_DEMO_31_TRUNKS_CNS      31

/* 127 trunks supported by : DXCH , DXCH2, DXCH3 */
#define APP_DEMO_127_TRUNKS_CNS     127

/* 127 trunks supported by : EXMXPM */
#define APP_DEMO_255_TRUNKS_CNS     255

#define PP_LOGICAL_CONFIG_FDB_DEFAULT                                         \
{                                                                             \
    0,/*fdbTableMode = INTERNAL_SPLIT */                                      \
    0,/*fdbToLutRatio -- don't care for internal memory */                    \
    0,/*fdbHashSize    -- don't care for internal memory */                   \
    0 /*extFdbHashMode -- don't care for internal memory */                   \
}

#define PP_LOGICAL_CONFIG_EXTERNAL_TCAM_DEFAULT                               \
{                                                                             \
    0,/* actionLinesInMemoryControl0 */                                       \
    0,/* actionLinesInMemoryControl1 */                                       \
    NULL,/*extTcamRuleWriteFuncPtr   */                                       \
    NULL,/*extTcamRuleReadFuncPtr    */                                       \
    NULL,/*extTcamActionWriteFuncPtr */                                       \
    NULL /*extTcamActionReadFuncPtr  */                                       \
}

#define APP_DEMO_CPSS_LPM_RAM_MEM_BLOCKS_DEFAULT                               \
{                                                                              \
    0,                                                             /* numOfBlocks*/   \
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /* blocksSizeArray*/ \
    CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E,/* blocksAllocationMethod */  \
    CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E,              /* lpmMemMode */         \
    {{0,CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E}},        /* lpmRamConfigInfo*/    \
    1,                                                       /* lpmRamConfigInfoNumOfElements */ \
    0                                                        /* maxNumOfPbrEntries */ \
}

/****************************************************************************/
/* GT_PP_CONFIG struct default                                              */
/****************************************************************************/
#define PP_LOGICAL_CONFIG_DEFAULT                                            \
{                                                                    \
    500,                            /* maxNumOfLifs              */  \
    APP_DEMO_CPSS_INLIF_PORT_MODE_PORT_E, /* inlifPortMode       */  \
    50,                             /* maxNumOfPolicerEntries    */  \
    GT_TRUE,                        /* policerConformCountEn     */  \
    1000,                           /* maxNumOfNhlfe             */  \
    100,                            /* maxNumOfMplsIfs           */  \
    APP_DEMO_CPSS_MEMORY_LOCATION_EXTERNAL_E,/*ipMplsMemLocation */  \
    1024,                           /* maxNumOfPclAction         */  \
    GT_PCL_ACTION_SIZE_16_BYTE,     /* pclActionSize             */  \
    0,                              /* maxNumOfPceForIpPrefixes  */  \
    GT_FALSE,                       /* usePolicyBasedRouting     */  \
    GT_FALSE,                       /* usePolicyBasedDefaultMc   */  \
    0,                              /* maxNumOfPbrEntries        */  \
    4095,                           /* maxVid                    */  \
    1024,                           /* maxMcGroups               */  \
    5,                              /* maxNumOfVirtualRouters    */  \
    100,                            /* maxNumOfIpNextHop         */  \
    1000,                           /* maxNumOfIpv4Prefixes      */  \
    100,                            /* maxNumOfIpv4McEntries     */  \
    500,                            /* maxNumOfMll               */  \
    100,                            /* maxNumOfIpv6Prefixes      */  \
    50,                             /* maxNumOfIpv6McGroups      */  \
    500,                            /* maxNumOfTunnelEntries     */  \
    8,                              /* maxNumOfIpv4TunnelTerms   */  \
    100,                            /* maxNumOfTunnelTerm        */  \
    0/*GT_VLAN_INLIF_ENTRY_TYPE_REGULAR_E*/, /* vlanInlifEntryType  */  \
    GT_FALSE,                       /* ipMemDivisionOn           */  \
    APP_DEMO_CPSS_DXCH_TCAM_ROUTER_BASED_E,  /* routingMode      */  \
    0,                              /* ipv6MemShare              */  \
    APP_DEMO_31_TRUNKS_CNS,         /* numOfTrunks               */  \
    0,                              /* defIpv6McPclRuleIndex     */  \
    0,                              /* vrIpv6McPclId             */  \
    GT_TRUE,                        /* lpmDbPartitionEnable      */  \
    100,                            /* lpmDbFirstTcamLine        */  \
    1003,                           /* lpmDbLastTcamLine         */  \
    GT_TRUE,                        /* lpmDbSupportIpv4          */  \
    GT_TRUE,                        /* lpmDbSupportIpv6          */  \
    APP_DEMO_CPSS_LPM_RAM_MEM_BLOCKS_DEFAULT,/*lpmRamMemoryBlocksCfg*/\
    0,                              /* miiNumOfTxDesc            */  \
    0,                              /* miiTxInternalBufBlockSize */  \
    {13, 13, 13, 13, 12, 12, 12, 12},/* miiBufferPercentage      */  \
    0,                              /* miiRxBufSize              */  \
    0,                              /* miiHeaderOffset           */  \
    0,                              /* miiRxBufBlockSize         */  \
    GT_FALSE                        /* pha_packetOrderChangeEnable*/ \
}

/****************************************************************************/
/* APP_DEMO_LIB_INIT_PARAMS struct default                                  */
/****************************************************************************/
#define LIB_INIT_PARAMS_DEFAULT                                 \
{                                                               \
    GT_TRUE,                        /* initBridge       */      \
    GT_TRUE,                        /* initClassifier   */      \
    GT_TRUE,                        /* initCos          */      \
    GT_TRUE,                        /* initIpv4         */      \
    GT_FALSE,                       /* initIpv6         */      \
    GT_TRUE,                        /* initIpv4Filter   */      \
    GT_TRUE,                        /* initIpv4Tunnel   */      \
    GT_TRUE,                        /* initLif          */      \
    GT_TRUE,                        /* initMirror       */      \
    GT_TRUE,                        /* initMpls         */      \
    GT_TRUE,                        /* initMplsTunnel   */      \
    GT_TRUE,                        /* initNetworkIf    */      \
    GT_FALSE,                       /* initI2c          */      \
    GT_TRUE,                        /* initPhy          */      \
    GT_TRUE,                        /* initPort         */      \
    GT_TRUE,                        /* initTc           */      \
    GT_TRUE,                        /* initTrunk        */      \
    GT_TRUE,                        /* initPcl          */      \
    GT_FALSE,                       /* initTcam         */      \
    GT_TRUE,                        /* initPolicer      */      \
    GT_TRUE,                        /* Init action      */      \
    GT_FALSE,                 /* sFlow, only for D1 and D-XG*/  \
    GT_FALSE                        /* initPha */               \
}

/**
* @internal appDemoPhase2DefaultInit function
* @endinternal
*
* @brief   Sets default value for CPSS_PP_PHASE2_INIT_PARAMS struct
* @param[out] ppPh2ConfigPtr           - pointer to struct
*/
GT_VOID appDemoPhase2DefaultInit
(
    OUT CPSS_PP_PHASE2_INIT_PARAMS  *ppPh2ConfigPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtAppDemoSysConfigDefaultsh */



