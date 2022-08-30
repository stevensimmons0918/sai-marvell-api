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
* @file cpssAppPlatformSip6.c
*
* @brief Initialization functions for the Falcon - SIP6 - board.
*
* @version   2
********************************************************************************
*/
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Belly-to-Belly board boolean variable */
static GT_BOOL belly2belly = GT_FALSE;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */


extern GT_STATUS appPlatformDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

#define _2_TILES_PORTS_OFFSET   56
#define TESTS_PORTS_OFFSET(devNum)  (_2_TILES_PORTS_OFFSET)

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define FALCON_TXQ_PER_DQ   (8 * (FALCON_MAC_PER_DP + 1))
#define FALCON_MAC_PER_DP   8 /* -- without CPU port -- first ports are those with SERDES 28/56 */
#define FALCON_NUM_DP       8 /* DP port groups */

/* build TXQ_port from macPort, dpIndex */
#define FALCON_TXQ_PORT(macPort, dpIndex)   \
    (FALCON_TXQ_PER_DQ*(dpIndex) + (macPort))

/* build MAC_port from macPort, dpIndex */
#define FALCON_MAC_PORT(macPort, dpIndex)   \
    (FALCON_MAC_PER_DP*(dpIndex) + (macPort))

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS
/* value to not collide with TXQ of other port */
#define TXQ_CPU_PORT    300

typedef struct
{
    GT_PHYSICAL_PORT_NUM                startPhysicalPortNumber;
    GT_U32                              numOfPorts;
    GT_U32                              startGlobalDmaNumber;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              jumpDmaPorts;/* allow to skip DMA ports (from startGlobalDmaNumber).
                                                        in case that mappingType is CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E
                                                        this field Contains the cascade port number*/
}FALCON_PORT_MAP_STC;

#define IGNORE_DMA_STEP 0 /*value 0 or 1 means 'ignore' */
/* jump DMA in steps of 2 because we currently limited to '128 physical ports' mode */
/* so the 12.8T device with the 256+ MACs ... need to use only limited MACs */
#define DMA_STEP_OF_2   2
#define DMA_STEP_OF_3   3
#define DMA_STEP_OF_4   4


/*stay with same DMA - needed for remote ports */
#define DMA_NO_STEP   0xFFFF
#define NO_REMOTE_PORT 0xFFFF

#if 0
just like falcon_onEmulator_prepare_tiles(IN GT_U32    numOfTiles) but for 'HW'

/* function needed to hint us for the phase1Params->maxNumOfPhyPortsToUse */
GT_STATUS falcon_prepare_tiles(IN GT_U32    numOfTiles)
{
    appDemoDbEntryAdd("numOfTiles", numOfTiles);
    return GT_OK;
}

extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

#endif

/*  in sip5_20 was 2 ports modes:
     256, 512.
    sip6 have 5 ports modes:
    64, 128, 256, 512, 1024.
    Note that falcon have 3.2 device, 6.4 device and 12.8 device, each of these devices can have the ports modes above so in total it's (3 * 5) = 15 different modes
    */

/* 64 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports).*/
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_64_port_mode[] =
{
    /* physical ports 0..49 , mapped to MAC 0..32 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
     /* map 'CPU SDMA ports' 56..58 (no MAC/SERDES) DMA 66..68 */
    ,{56              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap_64_port_mode = sizeof(falcon_3_2_defaultMap_64_port_mode)/sizeof(falcon_3_2_defaultMap_64_port_mode[0]);


/* 128 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP }
    /* physical ports 64..68 , mapped to MAC 59..63 */
    ,{64              , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP }
    /* map 'Ethernet CPU ports' (non-SDMA) */

    /* physical port 69 , mapped to MAC 64 */
/* on DP[2] : CPU network port on DMA 64 (NOT muxed with any SDMAs) */
    ,{69              , 1           ,  64       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
/* on DP[4] : CPU network port on DMA 65 is muxed with DMA 66 used by SDMA0 */

    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 67..69 */
    ,{80              , 3           ,  67/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap = sizeof(falcon_3_2_defaultMap)/sizeof(falcon_3_2_defaultMap[0]);

/* 128 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports) + One Network CPU port */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_TwoNetCpuPorts[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 67 (DMA 66 muxed with DMA 65 ... so cant be used)
       meaning using MG[1] (and MG[0] is unused) */
    ,{CPU_PORT/*63*/  , 1           ,  67/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP }
    /* physical ports 64..68 , mapped to MAC 59..63 */
    ,{64              , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP }
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 64 */
    ,{69              , 1           ,  64       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* muxed with MG[0] on DP[4] */
    ,{70              , 1           ,  65       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    ,{80              , 1           ,  68/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{81              , 1           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}


};
static GT_U32   actualNum_falcon_3_2_defaultMap_TwoNetCpuPorts = sizeof(falcon_3_2_defaultMap_TwoNetCpuPorts)/sizeof(falcon_3_2_defaultMap_TwoNetCpuPorts[0]);


/* currently 256 and 512 modes are using 128 mode port map to avoid failed tests that not support other ports selection*/

/* 256 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_256_port_mode[] =
{
    /* physical ports 128..186 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 192..196 , mapped to MAC 59..63 */
    ,{128             , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap_256_port_mode = sizeof(falcon_3_2_defaultMap_256_port_mode)/sizeof(falcon_3_2_defaultMap_256_port_mode[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: not remote ports .physical ports 128..186 , mapped to MAC 0..58 */
     {(0 /*+ 128*/)              , 59/*56*/          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP }
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  65/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* remote physical ports 590..599 , mapped to MAC 59 , with phisical port 64. this is comment out to enable the enh-UT ports  58 12 36 0 */
   /* ,{590             , 10   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,DMA_NO_STEP , 64}*/
    /* remote physical ports 600..609 , mapped to MAC 60 , with phisical port 65 */
    ,{600             , 10   ,  60       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,65}
    /* remote physical ports 610..619 , mapped to MAC 61 , with phisical port 66 */
    ,{610             , 10   ,  61       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,66}
    /* remote physical ports 620..629 , mapped to MAC 62 , with phisical port 67 */
    ,{620             , 9   ,  62       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 67}
    /* remote physical ports 630..639 , mapped to MAC 63 , with phisical port 68 */
    ,{630             , 9   ,  63       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 68}
#if 0 /* muxed with MG[2] on DP[6] */
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 64 */
    ,{69              , 1           ,  64       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif /*0*/
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  66/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

};
static GT_U32   actualNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports[0]);


/* we have PHYSICAL ports 0..59,63,64..119*/
/* this macro hold the number of ports from 64..119 */
#define NUM_PORTS_64_TO_119  ((119-64)+1)

/* 64 port mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_64_port_mode[] =
{
    /* physical ports 0..52 , mapped to MAC 0..52 */
    /* leave physical port 53 to be used by the 'port delete WA' */
     {0               , 53          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 54 , mapped to MAC 54 */
    ,{54              ,  1          ,   54       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E   ,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' 55 - 61 to DMA 137 - 143 */
    ,{55              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap_64_port_mode = sizeof(falcon_6_4_defaultMap_64_port_mode)/sizeof(falcon_6_4_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..132 , mapped to MAC 59..127 */
    ,{64              ,(127 - 59 + 1),  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

#if 0  /* the 2 ports muxed with SDMAs */
    /* CPU#0 port */
    ,{133             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port */
    ,{134             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif
    /* map 'CPU SDMA ports' 135 - 141 to DMA 137 - 143 */
    ,{135              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap_256_port_mode = sizeof(falcon_6_4_defaultMap_256_port_mode)/sizeof(falcon_6_4_defaultMap_256_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..119 , mapped to MAC 59..116 */
    ,{64              ,NUM_PORTS_64_TO_119,  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 121 - 127 to DMA 137 - 143 */
    ,{121              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap = sizeof(falcon_6_4_defaultMap)/sizeof(falcon_6_4_defaultMap[0]);

static FALCON_PORT_MAP_STC falcon_6_4_100G_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..116 with step 2 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

    ,{64              , 5,             118,         CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}

    /* map 'CPU SDMA ports' 121 - 127 to DMA 137 - 143 */
    ,{121              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_100G_defaultMap = sizeof(falcon_6_4_100G_defaultMap)/sizeof(falcon_6_4_100G_defaultMap[0]);


/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/
static GT_U32   modeMaxMac = 0;
static GT_U32   mode100G   = 0;
static GT_U32   cpuPortMux2UseSdma = 1; /* default mode - SDMA (for CPU should be used '0' by calling appDemoDbEntrySet) */

static FALCON_PORT_MAP_STC falcon_6_4_maxMAC[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..123 , mapped to MAC 59..118 */
    ,{64              ,60,             59 ,        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* NOTE: the 'mux' are removed in run time !!! depend on :
            use_falcon_6_4_cpu_ports_group1
            use_falcon_6_4_cpu_ports_group2
    */
    /* CPU#0 port Raven 0 - dp[0]*/
    ,{124             ,1,               128 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port Raven 3 - dp[6]*/
    ,{125             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#2 port Raven 4 - dp[8]*/
    ,{126             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#3 port Raven 7 - dp[14]*/
    ,{127             ,1,               135 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_maxMAC = sizeof(falcon_6_4_maxMAC)/sizeof(falcon_6_4_maxMAC[0]);

static FALCON_PORT_MAP_STC falcon_6_4_256_port_maxMAC[] =
{
    /* first 59 MACs : physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* next 128-59 MACs : physical ports 64..132 , mapped to MAC 59..127 */
    ,{64              ,(128-59)     ,   59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* NOTE: the 'mux' are removed in run time !!! depend on :
            use_falcon_6_4_cpu_ports_group1
            use_falcon_6_4_cpu_ports_group2
    */
    /* CPU#0 port Raven 0 - dp[0]*/
    ,{133             ,1,               128 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port Raven 3 - dp[6]*/
    ,{134             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#2 port Raven 4 - dp[8]*/
    ,{135             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#3 port Raven 7 - dp[14]*/
    ,{136             ,1,               135 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' 137 - 143 to DMA 137 - 143 ,
       NOTE: the 'mux' of DMA 138 (muxed with MAC 131) and DMA 140 (muxed with MAC 132)
            is removed in runtime if
    */
    ,{137              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_256_port_maxMAC = sizeof(falcon_6_4_256_port_maxMAC)/sizeof(falcon_6_4_256_port_maxMAC[0]);

/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
     /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
       /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  66       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' */
    ,{133             , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports[0]);

/*
NOTE: in 12.8 we select only 8 CPU SDMA ports although 16 available , because:
1. we work in 128 ports mode ... so using all 16 will reduce dramatically the number of regular ports.
2. we want to test 'non-consecutive' MG units so we use (MGs:1,3..5,10..13)
3. we test CPU_PORT (63) to work with MG1 instead of MG0 , because DP[4] (connected to MG0)
    may serve 'CPU network port'
*/

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_64_port_mode[] =
{
    /* physical ports 0..32 , mapped to MAC 0..58 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{50              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{54              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap_64_port_mode = sizeof(falcon_12_8_defaultMap_64_port_mode)/sizeof(falcon_12_8_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  128  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
   /* ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4,NO_REMOTE_PORT}*/

    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap_256_port_mode = sizeof(falcon_12_8_defaultMap_256_port_mode)/sizeof(falcon_12_8_defaultMap_256_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  64  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
    ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4}

    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap = sizeof(falcon_12_8_defaultMap)/sizeof(falcon_12_8_defaultMap[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
      /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  160       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports[0]);

/* 512 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_512_port_mode[] =
{
    /* physical ports 0..61 , mapped to MAC 0..61 */
     {0               , 61          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 15 CPU ports , the existence of next ports : in DP[8..11].*/
    /* physical ports 64..257 , mapped to MAC 59..255 (in steps of 1) */
    ,{64              ,(255 - 61 + 1),  61  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 258 , mapped to MAC 257 */
    ,{259             , 1           ,  257    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 260..273 (no MAC/SERDES) DMA 274..287 (MG[2..15])*/
    ,{261              , 14          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 274 (no MAC/SERDES) DMA 272 (MG[0])*/
    ,{275              , 1           ,  272/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap_512_port_mode =
    sizeof(falcon_12_8_defaultMap_512_port_mode)
    /sizeof(falcon_12_8_defaultMap_512_port_mode[0]);

/* 512 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[] =
{
    /* physical ports 0..61 , mapped to MAC 0..61 */
     {0               , 61          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 15 CPU ports , the existence of next ports : in DP[8..11].*/
    /* physical ports 64..257 , mapped to MAC 59..255 (in steps of 1) */
    ,{64              ,(255 - 61 + 1),  61  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 258 , mapped to MAC 257 */
    ,{259             , 1           ,  257    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    , {260             , 1           ,  258    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 260..273 (no MAC/SERDES) DMA 274..287 (MG[2..15])*/
    ,{261              , 14          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
#if 0
        /* muxed with DMA 257 : map 'CPU SDMA ports' 280 (no MAC/SERDES) DMA 258 (MG[0])*/
    ,{274              , 1           ,  272/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
#endif
};

static GT_U32   actualNum_falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts =
    sizeof(falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts)
    /sizeof(falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[0]);


#define MAX_MAPPED_PORTS_CNS    1024
static  GT_U32  actualNum_cpssApi_falcon_defaultMap = 0;/*actual number of valid entries in cpssApi_falcon_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_falcon_defaultMap[MAX_MAPPED_PORTS_CNS] =
{
    {0}
    /* filled at run time , by info from falcon_3_2_defaultMap[] */
};


/*******************AC5X PortMaps********************/

#define PORT_NUMBER_ARR_SIZE    9
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
}PHOENIX_PORT_MAP_STC;

/* the physical port numbers are restricted to 0..74 due to 'my physical ports' table (TTI unit) */
/* mapping of Phoenix physical ports to MAC ports (and TXQ ports). */
static PHOENIX_PORT_MAP_STC phoenix_defaultMap_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , { 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , { 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , { 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_phoenix_defaultMap_port_mode = sizeof(phoenix_defaultMap_port_mode)/sizeof(phoenix_defaultMap_port_mode[0]);

static PHOENIX_PORT_MAP_STC phoenix_defaultMap_port_mode_rpp[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  GT_NA} , { 0,  1,  2,  3,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, GT_NA} , { 8,  9,  10, 11, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 16, 17, 18, 19, GT_NA} , { 16, 17, 18, 19, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* RPP */
    ,{{ 24, 25, 26, 27, GT_NA} ,{ 16, 16, 16, 16, 16, 16, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E}
    ,{{ 32, 33, 34, 35, GT_NA} , { 32, 33, 34, 35, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 40, 41, 42, 43, GT_NA} , { 40, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_phoenix_defaultMap_port_mode_rpp = sizeof(phoenix_defaultMap_port_mode_rpp)/sizeof(phoenix_defaultMap_port_mode_rpp[0]);


/* mapping of 24 Port Mode Phoenix physical ports to MAC ports (and TXQ ports). */
static PHOENIX_PORT_MAP_STC phoenix_defaultMap_24_6_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_phoenix_defaultMap_24_6_port_mode = sizeof(phoenix_defaultMap_24_6_port_mode)/sizeof(phoenix_defaultMap_24_6_port_mode[0]);

/* mapping of 16 Port Mode Phoenix physical ports to MAC ports (and TXQ ports). */
static PHOENIX_PORT_MAP_STC phoenix_defaultMap_16_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_phoenix_defaultMap_16_port_mode = sizeof(phoenix_defaultMap_16_port_mode)/sizeof(phoenix_defaultMap_16_port_mode[0]);

/* mapping of Aldrin3S 24 Port Mode physical ports to MAC ports (and TXQ ports). */
static PHOENIX_PORT_MAP_STC aldrin_3s_defaultMap_12_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  GT_NA} , { 0,  1,  2,  3,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, GT_NA} , { 8,  9,  10, 11, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 16, 17, 18, 19, GT_NA} , { 16, 17, 18, 19, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 24, 25, 26, 27, GT_NA} , { 24, 25, 26, 27, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 32, 33, 34, 35, GT_NA} , { 32, 33, 34, 35, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 40, 41, 42, 43, GT_NA} , { 40, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_aldrin_3s_defaultMap_12_port_mode = sizeof(aldrin_3s_defaultMap_12_port_mode)/sizeof(aldrin_3s_defaultMap_12_port_mode[0]);

/* mapping of Aldrin3S 8 physical ports to MAC ports (and TXQ ports). */
static PHOENIX_PORT_MAP_STC aldrin_3s_defaultMap_8_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  GT_NA} , { 0,  1,  2,  3,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, GT_NA} , { 8,  9,  10, 11, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 'CPU' mac */
    ,{{48,49,                   GT_NA} , {48,49,                   GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* ports from 100G mac */
    ,{{50,51,52,53,             GT_NA} , {50,51,52,53,             GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
static GT_U32   actualNum_aldrin_3s_defaultMap_8_port_mode = sizeof(aldrin_3s_defaultMap_8_port_mode)/sizeof(aldrin_3s_defaultMap_8_port_mode[0]);

static GT_U32   phoenix_only_ports_of_100G_mac = 0; /* the default is 0 as the CPSS supports USX ports */
GT_STATUS phoenix_only_ports_of_100G_mac_set(IN GT_U32   use)
{
    phoenix_only_ports_of_100G_mac = use;
    return GT_OK;
}

static GT_U32   phoenix_only_ports_of_25G_mac = 1; /* the default is 1 as the CPSS not supports ALL USX ports , but 'first port' of each 8 USX ports */
GT_STATUS phoenix_only_ports_of_25G_mac_set(IN GT_U32   use)
{
    phoenix_only_ports_of_25G_mac = use;
    return GT_OK;
}

/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/

#define MAX_MAPPED_PORTS_AC5X_CNS    128
static  GT_U32  actualNum_cpssApi_phoenix_defaultMap = 0;/*actual number of valid entries in cpssApi_phoenix_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_phoenix_defaultMap[MAX_MAPPED_PORTS_AC5X_CNS] =
{
    {0}
    /* filled at run time */
};
/*******************AC5X PortMaps********************/

#ifdef GM_USED
GT_U32  useSinglePipeSingleDp = 1;
#else /*!GM_USED*/
GT_U32  useSinglePipeSingleDp = 0;
#endif /*!GM_USED*/
/* allow to modify the flag from the terminal */
GT_STATUS falcon_useSinglePipeSingleDp_Set(IN GT_BOOL useSingleDp)
{
    useSinglePipeSingleDp = useSingleDp;
    return GT_OK;
}

/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

GT_VOID cpssAppPlatformBelly2BellyEnable(IN GT_BOOL enable)
{
    belly2belly = enable;
}

/* fill array of cpssApi_falcon_defaultMap[] and update actualNum_cpssApi_falcon_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    FALCON_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32  dmaPortOffset;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;
    GT_U32 maxEthMacs;
    GT_U32 tmpData;

    if(appPlatformDbEntryGet("useCpuPort2", &tmpData) == GT_OK)
    {
        cpuPortMux2UseSdma = 0;
    }

    if(useSinglePipeSingleDp == 1)
    {
        /*************************************************************************************/
        /* the array for the GM is already in 'CPSS format' , and not relate to below arrays */
        /* see singlePipe_singleDp_cpssApi_falcon_defaultMap[]                               */
        /*************************************************************************************/
        numCpuSdmas = 1;
        return GT_OK;
    }

    if(actualNum_cpssApi_falcon_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        default:
        case 0:
        case 1:
            if (maxPhyPorts == 64)
            {
                 currAppDemoInfoPtr = &falcon_3_2_defaultMap_64_port_mode[0];
                 numEntries = actualNum_falcon_3_2_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_3_2_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512 */
            {
                if(cpuPortMux2UseSdma == 0)
                {
                    currAppDemoInfoPtr = &falcon_3_2_defaultMap_TwoNetCpuPorts[0];
                    numEntries = actualNum_falcon_3_2_defaultMap_TwoNetCpuPorts;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_3_2_defaultMap[0];
                    numEntries = actualNum_falcon_3_2_defaultMap;
                }
            }
            break;
        case 2:
             if(mode100G)
            {
                currAppDemoInfoPtr = &falcon_6_4_100G_defaultMap[0];
                numEntries = actualNum_falcon_6_4_100G_defaultMap;
            }
            else if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_64_port_mode[0];
                numEntries = actualNum_falcon_6_4_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_6_4_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512*/
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap[0];
                numEntries = actualNum_falcon_6_4_defaultMap;
            }

            if(modeMaxMac)
            {
                if (maxPhyPorts == 256)
                {
                    currAppDemoInfoPtr = &falcon_6_4_256_port_maxMAC[0];
                    numEntries = actualNum_falcon_6_4_256_port_maxMAC;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_6_4_maxMAC[0];
                    numEntries = actualNum_falcon_6_4_maxMAC;
                }
            }
            break;
    case 4:
            if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_64_port_mode[0];
                numEntries = actualNum_falcon_12_8_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_12_8_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 512)
            {
                if(cpuPortMux2UseSdma == 0)
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[0];
                    numEntries = actualNum_falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap_512_port_mode[0];
                    numEntries = actualNum_falcon_12_8_defaultMap_512_port_mode;
                }
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128 */
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap[0];
                numEntries = actualNum_falcon_12_8_defaultMap;
            }
            break;
    }

    cpssOsMemSet(cpssApi_falcon_defaultMap,0,sizeof(cpssApi_falcon_defaultMap));

    currCpssInfoPtr    = &cpssApi_falcon_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
         if ((currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)&&(actualNum_cpssApi_falcon_defaultMap < maxPhyPorts))
            {
                /* first generate line that map between the mac number to physical port in the device.
                for remote ports the physical port number ( cascade number) is in field of "dma step"*/
                 currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->jumpDmaPorts;
                 currCpssInfoPtr->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                 currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber;
                 currCpssInfoPtr->txqPortNumber      = 0;

                 actualNum_cpssApi_falcon_defaultMap++;
                 currCpssInfoPtr++;
                /* second, generate lines that map between remote physical port to the mac number*/
            }
        dmaPortOffset = 0;
        for(jj = 0 ; jj < currAppDemoInfoPtr->numOfPorts; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_falcon_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                return rc;
            }

            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->startPhysicalPortNumber + jj;
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;
            /* convert the DMA to TXQ port number , because those need to be in consecutive range and in 8 TC steps */
            currCpssInfoPtr->txqPortNumber      = 0;


            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                numCpuSdmas++;
            }

            actualNum_cpssApi_falcon_defaultMap++;

             if((currAppDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
             {
                   if(currAppDemoInfoPtr->jumpDmaPorts >= 2)
                    {
                        dmaPortOffset += currAppDemoInfoPtr->jumpDmaPorts;
                    }
                    else
                    {
                        dmaPortOffset ++;
                    }
               }
        }
    }

    if(belly2belly)
    { /* In Belly-to-Belly board's front-panel - the Ravens order is 2-15,0-1. Shift of 32 ports is needed */
        maxEthMacs = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        for(ii = 0 ; ii < actualNum_cpssApi_falcon_defaultMap ; ii++)
        {
             if(cpssApi_falcon_defaultMap[ii].interfaceNum < maxEthMacs)
             {
                 cpssApi_falcon_defaultMap[ii].interfaceNum = (cpssApi_falcon_defaultMap[ii].interfaceNum + 32) % maxEthMacs;
             }
        }
     }

    return GT_OK;
}

/*******************AC5X PortMaps********************/
/* fill array of cpssApi_phoenix_defaultMap[] and update actualNum_cpssApi_phoenix_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo_ac5x(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    PHOENIX_PORT_MAP_STC *currAppDemoInfoPtr = NULL;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr  = NULL;
    GT_U32  ii,jj;
    GT_U32 numEntries = 0;
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32 value;

    if(actualNum_cpssApi_phoenix_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    if((appPlatformDbEntryGet("rppMap", &value) == GT_OK) && (value != 0))
    {
        currAppDemoInfoPtr = &phoenix_defaultMap_port_mode_rpp[0];
        numEntries = actualNum_phoenix_defaultMap_port_mode_rpp;
    }
    else
    {
        switch (devType)
        {
            case CPSS_98DX3510_CNS:
            case CPSS_98DX3510_H_CNS:
            case CPSS_98DX3510M_H_CNS:
            case CPSS_98DX3530_CNS:
            case CPSS_98DX3530_H_CNS:
            case CPSS_98DX3530M_H_CNS:
            case CPSS_98DX3510M_CNS:
            case CPSS_98DX3530M_CNS:
            case CPSS_98DX3550_CNS:
            case CPSS_98DX3550M_CNS:
                /* AC5X 48 physical ports */
                currAppDemoInfoPtr = &phoenix_defaultMap_port_mode[0];
                numEntries = actualNum_phoenix_defaultMap_port_mode;
                break;
            case CPSS_98DX3500_CNS:
            case CPSS_98DX3520_CNS:
            case CPSS_98DX3500M_CNS:
            case CPSS_98DX3520M_CNS:
            case CPSS_98DX3540_CNS:
            case CPSS_98DX3540M_CNS:
                /* AC5X 24 physical ports */
                currAppDemoInfoPtr = &phoenix_defaultMap_24_6_port_mode[0];
                numEntries = actualNum_phoenix_defaultMap_24_6_port_mode;
                break;
            case CPSS_98DX3501_CNS:
            case CPSS_98DX3501M_CNS:
                /* AC5X 16 physical ports */
                currAppDemoInfoPtr = &phoenix_defaultMap_16_port_mode[0];
                numEntries = actualNum_phoenix_defaultMap_16_port_mode;
                break;
            case CPSS_98DX7312_CNS:
            case CPSS_98DX7312M_CNS:
            case CPSS_98DXC712_CNS:
                /* Aldrin 3S 24 physical ports */
                currAppDemoInfoPtr = &aldrin_3s_defaultMap_12_port_mode[0];
                numEntries = actualNum_aldrin_3s_defaultMap_12_port_mode;
                break;
            case CPSS_98DX7308_CNS:
            case CPSS_98DX7308M_CNS:
            case CPSS_98DXC708_CNS:
                /* Aldrin 3S 8 physical ports */
                currAppDemoInfoPtr = &aldrin_3s_defaultMap_8_port_mode[0];
                numEntries = actualNum_aldrin_3s_defaultMap_8_port_mode;
                break;
            default:
                break;
        }
    }

    cpssOsMemSet(cpssApi_phoenix_defaultMap,0,sizeof(cpssApi_phoenix_defaultMap));

    currCpssInfoPtr    = &cpssApi_phoenix_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_phoenix_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                return rc;
            }

            interfaceNum = currAppDemoInfoPtr->globalDmaNumber[jj];

            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->portNumberArr[jj];
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = interfaceNum;
            currCpssInfoPtr->txqPortNumber      = 0;/* CPSS will give us 8 TC queues per port */

            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && currCpssInfoPtr->physicalPortNumber != CPU_PORT)
                {
                    /* skip this SDMA - the GM currently supports single SDMA */
                    currCpssInfoPtr--;
                    continue;
                }
            }

            actualNum_cpssApi_phoenix_defaultMap++;
        }
    }

    return GT_OK;
}
/*******************AC5X PortMaps********************/

/**
* @internal falcon_initStaticSerdesMuxingValidation function
* @endinternal
*
* @brief   init static serdes muxing
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

/**
* @internal falcon_configPolaritySwap function
* @endinternal
*
* @brief   Config polarity swap
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/

/* CAP - CPSS App Platform */
GT_STATUS
cpssAppPlatformPpFalconPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
)
{
    GT_STATUS rc;

    rc = fillDbForCpssPortMappingInfo(devNum);
    if (rc != GT_OK)
        return rc;

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
        default:
            if(useSinglePipeSingleDp == 0)
            {
                ARR_PTR_AND_SIZE_MAC(cpssApi_falcon_defaultMap, *mapArrPtr, *mapArrLen);
                *mapArrLen = actualNum_cpssApi_falcon_defaultMap;
            }
            break;
    }

    return GT_OK;
}


/*******************AC5X PortMaps********************/
GT_STATUS cpssAppPlatformPpAc5xPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
)
{
    GT_STATUS rc;

    rc = fillDbForCpssPortMappingInfo_ac5x(devNum);
    if (rc != GT_OK)
        return rc;

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_phoenix_defaultMap, *mapArrPtr, *mapArrLen);
            *mapArrLen = actualNum_cpssApi_phoenix_defaultMap;
            break;
    }

    return GT_OK;
}
/*******************AC5X PortMaps********************/

