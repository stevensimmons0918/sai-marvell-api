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
* @file gtDbDxPhoenix.c
*
* @brief Initialization functions for the Phoenix - SIP6.15 - board.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdAndPp.h>

#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef INCLUDE_MPD
#include <appDemo/phy/gtAppDemoPhyConfig.h>
/*static GT_U32 mpdIfIndexArr[512];
CPSS_OS_MUTEX  xsmi_extn_mtx;*/
#endif

extern GT_U32 bc2BoardType;

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;
/* port manager boolean variable */
extern GT_BOOL portMgr;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
extern GT_U32  appDemo_PortsInitList_already_done;

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);
extern GT_STATUS appDemoDxLibrariesInit(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);
extern GT_STATUS appDemoDxPpGeneralInit(GT_U8 dev);
extern GT_STATUS appDemoDxTrafficEnable(GT_U8 dev);
extern GT_STATUS boardAutoPollingConfigure(IN  GT_U8   dev,IN  GT_U32  boardType);
extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);
extern GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);
extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);
GT_STATUS appDemoBc2PIPEngineInit
(
    IN  GT_U8       boardRevId
);
GT_STATUS appDemoB2PtpConfig
(
    IN  GT_U8  devNum
);
extern GT_STATUS appDemoDxHwPpPhase1DbEntryInit
(
    IN  GT_U8                        dev,
    INOUT  CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *cpssPpPhase1ParamsPtr,
    IN  CPSS_PP_DEVICE_TYPE         deviceId,
    OUT GT_U32                      *waIndexPtr,
    OUT CPSS_DXCH_IMPLEMENT_WA_ENT  *waFromCpss,
    OUT GT_BOOL                     *trunkToCpuWaEnabledPtr
);

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS

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

#define MAX_MAPPED_PORTS_CNS    128
static  GT_U32  actualNum_cpssApi_phoenix_defaultMap = 0;/*actual number of valid entries in cpssApi_phoenix_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_phoenix_defaultMap[MAX_MAPPED_PORTS_CNS];
    /* filled at run time , by info from falcon_3_2_defaultMap[] */

static PortInitList_STC *phoenix_force_PortsInitListPtr = NULL;

static PortInitList_STC phoenix_portInitlist__only_ports_of_100G_mac[] =
{    /* ports in 25G */
     { PORT_LIST_TYPE_LIST,  {50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


static PortInitList_STC phoenix_AC5X_RD_portInitlist[] =
{
    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL, { 0,31,1,               APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
    /* ports in 2.5G */
    ,{ PORT_LIST_TYPE_INTERVAL, { 32,47,1,              APP_INV_PORT_CNS },  CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E  }
    /* ports in 25G */
    ,{ PORT_LIST_TYPE_LIST,     {48,49,50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC phoenix_48G_6x25g_portInitlist[] =
{
    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL, { 0,47,1,               APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
    /* ports in 25G */
    ,{ PORT_LIST_TYPE_LIST,     {48,49,50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC phoenix_24G_6x25g_portInitlist[] =
{
    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL, { 0,23,1,               APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
    /* ports in 25G */
    ,{ PORT_LIST_TYPE_LIST,     {48,49,50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC phoenix_16G_6x25g_portInitlist[] =
{
    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL, { 0,15,1,               APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
    /* ports in 25G */
    ,{ PORT_LIST_TYPE_LIST,     {48,49,50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC phoenix_12x25g_portInitlist[] =
{    /* ports in 25G */
    { PORT_LIST_TYPE_LIST,  {0,8,16,24,32,40, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
   ,{ PORT_LIST_TYPE_LIST,  {48,49,           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
   ,{ PORT_LIST_TYPE_LIST,  {50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

   ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC phoenix_8x25g_portInitlist[] =
{    /* ports in 25G */
    { PORT_LIST_TYPE_LIST,  {0,8,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
   ,{ PORT_LIST_TYPE_LIST,  {48,49,           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
   ,{ PORT_LIST_TYPE_LIST,  {50,51,52,53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

   ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


static PortInitList_STC phoenix_rpp_portInitlist[] =
{
    /* ports in 25G */
    { PORT_LIST_TYPE_LIST,  {0,8,16, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* RPP*/
    ,{PORT_LIST_TYPE_LIST,{24,25,26,27,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* fill array of cpssApi_phoenix_defaultMap[] and update actualNum_cpssApi_phoenix_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    PHOENIX_PORT_MAP_STC *currAppDemoInfoPtr = NULL;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32 numEntries = 0;/* number of entries in currAppDemoInfoPtr */
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  value;
    GT_U32                  currBoardType;

    if(actualNum_cpssApi_phoenix_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ( currBoardType ==  APP_DEMO_XCAT5X_A0_BOARD_RD_CNS)
    {
        currAppDemoInfoPtr = &phoenix_defaultMap_port_mode[0];
        numEntries = actualNum_phoenix_defaultMap_port_mode;
        phoenix_force_PortsInitListPtr = phoenix_AC5X_RD_portInitlist;
    }
    else if((appDemoDbEntryGet("rppMap", &value) == GT_OK) && (value != 0))
    {
        currAppDemoInfoPtr = &phoenix_defaultMap_port_mode_rpp[0];
        numEntries = actualNum_phoenix_defaultMap_port_mode_rpp;
        phoenix_force_PortsInitListPtr = phoenix_rpp_portInitlist;
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
    cpssOsMemSet(cpssApi_phoenix_defaultMap, 0, sizeof(cpssApi_phoenix_defaultMap));

    currCpssInfoPtr    = &cpssApi_phoenix_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_phoenix_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_phoenix_defaultMap is FULL, maxPhy port ", rc);
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

static GT_BOOL  ac5xInternalCpu = GT_FALSE;

extern void DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
void NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#endif/*NOKM_DRV_EMULATE_INTERRUPTS*/


/**
* @internal phoenix_getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] firstDev                 - GT_TRUE - first device, GT_FALSE - next device
*
* @param[out] hwInfoPtr                - HW info of found device.
* @param[out] pciInfo                  - PCI info of found device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS phoenix_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;
    GT_U32 i = SYSTEM_DEV_NUM_MAC(0);
    GT_U32 flag_mbus_ac5x = 0;

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(firstDev, pciInfo);
    if (rc != GT_OK)
    {

        /* currently the 'pci scan' by the appDemo not supported , so this is hint that reunning AC5 on internal CPU */
        ac5xInternalCpu = GT_TRUE;
        flag_mbus_ac5x = MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS;

        appDemoPpConfigList[i].valid = 1;
        appDemoPpConfigList[i].deviceId  = CPSS_98DX7312M_CNS;
        appDemoPpConfigList[i].devFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;

        appDemoPpConfigList[i].channel = CPSS_CHANNEL_PCI_E;
        appDemoPpConfigList[i].devNum  = i;
        appDemoPpConfigList[i].pciInfo.pciBusNum = 0xFFFF;  /*to match IS_SIP6_MBUS_ADDR(...) */
        appDemoPpConfigList[i].pciInfo.pciIdSel  = 0xFF;    /*to match IS_SIP6_MBUS_ADDR(...) */
        appDemoPpConfigList[i].pciInfo.funcNo    = 0xFF;    /*to match IS_SIP6_MBUS_ADDR(...) */
        appDemoPpConfigList[i].pciInfo.pciDevVendorId.devId    = appDemoPpConfigList[i].deviceId >> 16;
        appDemoPpConfigList[i].pciInfo.pciDevVendorId.vendorId = appDemoPpConfigList[i].deviceId & 0xFFFF ;

        /* set a supported ac5 device */
        /*appDemoDebugDeviceIdSet(0, appDemoPpConfigList[i].deviceId);*/

        /* needed for cpssHwDriverSip6MbusCreateDrv(...) */
        prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_AC5X_E);

        cpssOsMemCpy(pciInfo, &appDemoPpConfigList[i].pciInfo, sizeof(GT_PCI_INFO));
    }
    else
    {
        /*copy info for later use*/
        appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo = *pciInfo;
    }

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E | (flag_mbus_ac5x << MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS), /*flag_mbus_ac5=1,flag_mbus_ac5x=2*/
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].hwInfo = *hwInfoPtr;

    if(ac5xInternalCpu == GT_TRUE && !cpssDeviceRunCheck_onEmulator())
    {
        /* NOTE:
           the first 'polling time' is if we work in polling mode of :
           hwInfoPtr->irq.switching     = CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS;
           the CPSS is opening a task for polling.

           the second 'polling time' is if we work in polling mode of :
           hwInfoPtr->irq.switching = 0x11ab0000 | (noKmMappingsNum & 0xffff);
           the exTDrv is opening a task for polling.
        */

        if(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].hwInfo.irq.switching ==
            CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS)
        {
            /* the default of 20 mili is not good enough for the LUA tests */
            /* NOTE : relevant only if working in polling mode             */
            DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(1/*millisec*/);
            osPrintf("NOTE : interrupts in polling mode by CPSS \n");
        }
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
        if((appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].hwInfo.irq.switching & 0xFFFF0000) ==
            0x11ab0000)
        {
            /* the default of 20 mili is not good enough for the LUA tests */
            /* NOTE : relevant only if working in polling mode             */
            NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(1/*millisec*/);
            osPrintf("NOTE : interrupts in polling mode by extDrv \n");
        }
#endif /*NOKM_DRV_EMULATE_INTERRUPTS*/
    }

    return rc;
}

/**
* @internal phoenix_getBoardInfoSimple function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS phoenix_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    rc = phoenix_getBoardInfo(GT_TRUE/*firstDev*/, hwInfoPtr, pciInfo);
    if (rc != GT_OK) {return rc;}

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
    if(rc == GT_OK)
    {
        osPrintf("More than one recognized device found on PCI bus!\n");
        return GT_INIT_ERROR;
    }

    return GT_OK;
}

/**
* @internal phoenix_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of Phoenix board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID phoenix_boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;

#ifdef GM_USED
    environment = "GM (Golden Model) - simulation";
#elif defined ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "WM (White Model) - simulation (special EMULATOR Mode)";
    }
    else
    {
        environment = "WM (White Model) - simulation";
    }
#else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif
    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
/*    cpssOsPrintf("Using Cider Version [%s] \n", ciderVersionName);*/

}

/**
* @internal getPpPhase1ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* @param[in] devNum                   - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS getPpPhase1ConfigSimple
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U8                               boardRevId,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params
)
{
    GT_STATUS rc;

    UNUSED_PARAM_MAC(boardRevId);

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->maxNumOfPhyPortsToUse = 128;/* single mode of 128 physical ports ,
                                            so supports value 128 or 0 */

    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E;

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98DX3500M_CNS,
        NULL,NULL,NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwPpPhase1DbEntryInit", rc);
    if(rc != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal getPpPhase2ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase2Init().
*
* @param[in] devNum                   - The device number to get the parameters for.
* @param[in] devType                  - The Pp device type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2ConfigSimple
(
    IN GT_SW_DEV_NUM       devNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  appDemo_ppPhase2Params;
    GT_U32                      tmpData;
    GT_U32                      auDescNum,fuDescNum;
    GT_U32 rxDescNum = RX_DESC_NUM_DEF;
    GT_U32 txDescNum = TX_DESC_NUM_DEF;

    appDemoPhase2DefaultInit(&appDemo_ppPhase2Params);
    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

    if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
    {
        appDemo_ppPhase2Params.useMultiNetIfSdma = GT_TRUE;
        if( appDemoDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) == GT_NO_SUCH )
        {

            /* Enable Tx queue 3 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_3", GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Enable Tx queue 6 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_6", GT_TRUE);
            if(rc != GT_OK)
            {
                 return rc;
            }
         }
    }

    /* allow each tile with it's own number of AUQ size */
    auDescNum = AU_DESC_NUM_DEF;
    /* allow each pipe with it's own number of FUQ size (for CNC upload) */
    /* NOTE: Fdb Upload still happen only on 'per tile' base (like AUQ) ...
       it is not per pipe triggered */
    fuDescNum = AU_DESC_NUM_DEF;

    phase2Params->newDevNum                  = CAST_SW_DEVNUM(devNum);
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

    rc = appDemoDbEntryAdd("fuqUseSeparate", GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(appDemoDbEntryGet("fuDescNum", &tmpData) == GT_OK)
    {
        /* Someone already stated the number of fuDescNum, so we NOT override it ! */
    }
    else
    {
        /* Add the fuDescNum to the DB, to allow appDemoAllocateDmaMem(...) to use it ! */
        rc = appDemoDbEntryAdd("fuDescNum", fuDescNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    appDemoCpssCurrentDevIndex = devNum;

    rc = appDemoAllocateDmaMem(devType,
                               /* Rx */
                               rxDescNum,RX_BUFF_SIZE_DEF,RX_BUFF_ALLIGN_DEF,
                               /* Tx */
                               txDescNum,
                               /* AUQ */
                               auDescNum,
                               &appDemo_ppPhase2Params);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAllocateDmaMem", rc);
    if(rc != GT_OK)
        return rc;

    /* convert CPSS_PP_PHASE2_INIT_PARAMS to CPSS_DXCH_PP_PHASE2_INIT_INFO_STC */
    phase2Params->netIfCfg          = appDemo_ppPhase2Params.netIfCfg         ;
    phase2Params->auqCfg            = appDemo_ppPhase2Params.auqCfg           ;
    phase2Params->fuqUseSeparate    = appDemo_ppPhase2Params.fuqUseSeparate   ;
    phase2Params->fuqCfg            = appDemo_ppPhase2Params.fuqCfg           ;
    phase2Params->useSecondaryAuq   = appDemo_ppPhase2Params.useSecondaryAuq  ;
    phase2Params->useDoubleAuq      = appDemo_ppPhase2Params.useDoubleAuq     ;
    phase2Params->useMultiNetIfSdma = appDemo_ppPhase2Params.useMultiNetIfSdma;
    phase2Params->multiNetIfCfg     = appDemo_ppPhase2Params.multiNetIfCfg    ;

    return rc;
}

#if 0
/**
 *  * mydHwCtrlRWMutexCreate
 *   * @return MYD_STATUS
 *    */
MTD_STATUS appDemo_phy_RWMutexCreate()
{
    char buf[64]={0};

    cpssOsMutexCreate(buf, &xsmi_extn_mtx);
    return MTD_OK;
}

/**
 *  * mydHwCtrlRWMutexDestroy
 *   * @return MYD_STATUS
 *    */
MTD_STATUS appDemo_phy_RWMutexDestroy()
{
    cpssOsMutexDelete(xsmi_extn_mtx);
    return MTD_OK;
}

/**
 * * @internal falcon_initPhys function
 * * @endinternal
 * */
GT_STATUS ac5x_phy_init
(
    IN GT_SW_DEV_NUM      devNum,
    IN GT_BOOL            loadImage;
)
{
   if(loadImage){
           rc = gtAppDemoPhyMpdInit(devNum);
   if (rc != GT_OK)
   return rc;
   }

       return GT_OK;
}

#endif


/**
* @internal phoenix_initPortMappingStage_fromEzbXml function
* @endinternal
*
*/
static GT_STATUS phoenix_initPortMappingStage_fromEzbXml
(
    IN GT_U32                   devIndex,
    OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtrPtr,
    OUT GT_U32                 *mapArrLenPtr
)
{
    GT_STATUS   rc;
    #define PORT_MAP_MAX_PORTS_CNS  CPSS_MAX_PORTS_NUM_CNS/* max physical ports */
    static CPSS_DXCH_PORT_MAP_STC portMapArr[PORT_MAP_MAX_PORTS_CNS];
    CPSS_DXCH_PORT_MAP_STC *currCpuSdmaPortInfoPtr;
    GT_U32 numPorts,numCpuSdmaPorts;
    CPSS_DXCH_PORT_MAP_STC cpuSdmaArr[2];

    cpssOsBzero((GT_CHAR *)cpuSdmaArr,sizeof(cpuSdmaArr));

    numCpuSdmaPorts = 0;
    /* the EZ_BRINGUP not supports the CPU SDMA ports that need to be added */
    currCpuSdmaPortInfoPtr = &cpuSdmaArr[numCpuSdmaPorts++];
    currCpuSdmaPortInfoPtr->physicalPortNumber = CPU_PORT;
    currCpuSdmaPortInfoPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
    currCpuSdmaPortInfoPtr->interfaceNum = 54; /*DMA - bound to MG0*/

    currCpuSdmaPortInfoPtr = &cpuSdmaArr[numCpuSdmaPorts++];
    currCpuSdmaPortInfoPtr->physicalPortNumber = GT_NA;/* let the logic find non-used number (reverse from 60 after skip 63-61) */
    currCpuSdmaPortInfoPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
    currCpuSdmaPortInfoPtr->interfaceNum = 55; /*DMA - bound to MG1*/

    rc = appDemoEzbSip6PortMappingGet(devIndex,
            PORT_MAP_MAX_PORTS_CNS,
            portMapArr,
            &numPorts,
            numCpuSdmaPorts,
            cpuSdmaArr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbSip6PortMappingGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsPrintf(" -- EZ_BRINGUP : The port mapping from XML file [%s] \n",
        ezbXmlName());

    *mapArrPtrPtr = portMapArr;
    *mapArrLenPtr = numPorts;
    return GT_OK;
}

/**
* @internal phoenix_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS phoenix_initPortMappingStage
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen;
    CPSS_DXCH_PORT_MAP_STC *mapUpdatedArrPtr;
    GT_U32 mapArrUpdatedLen;

    switch(devType)
    {
        case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_phoenix_defaultMap, mapArrPtr, mapArrLen);
            mapArrLen = actualNum_cpssApi_phoenix_defaultMap;
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }
    else
    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = phoenix_initPortMappingStage_fromEzbXml(devIndex,&mapArrPtr,&mapArrLen);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initPortMappingStage_fromEzbXml", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    rc = appDemoDxChPortPhysicalPortUpdatedMapCreate(
        mapArrLen, mapArrPtr, &mapArrUpdatedLen, &mapUpdatedArrPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (mapUpdatedArrPtr != NULL)
    {
        mapArrPtr = mapUpdatedArrPtr;
        mapArrLen = mapArrUpdatedLen;
    }

    rc = cpssDxChPortPhysicalPortMapSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        if (mapUpdatedArrPtr != NULL)
        {
            cpssOsFree(mapUpdatedArrPtr);
        }
        return rc;
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);

    if (mapUpdatedArrPtr != NULL)
    {
        cpssOsFree(mapUpdatedArrPtr);
    }
    return rc;
}

/**
* @internal phoenix_PortModeConfiguration_fromEzbXml function
* @endinternal
*
*/
static GT_STATUS phoenix_PortModeConfiguration_fromEzbXml
(
    IN GT_U32                   devIndex,
    OUT PortInitList_STC        **portInitListPtrPtr
)
{
    GT_STATUS   rc;
    /* each port will get it's own line according to info from the XML */
    /* function appDemoEzbPortsSpeedGet will fill it                   */
    static PortInitList_STC phoenix_portInitlist__from_xml[PORT_MAP_MAX_PORTS_CNS];
    GT_U32  actualNumEntries;

    rc = appDemoEzbPortsSpeedGet(devIndex,
            PORT_MAP_MAX_PORTS_CNS,
            phoenix_portInitlist__from_xml,
            &actualNumEntries);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbPortsSpeedGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* the XML not supports and also 'PDL' not support next: CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E */
    /* so convert CPSS_PORT_INTERFACE_MODE_XHGS_E to CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E         */
    {
        static EZB_XML_SPEED_IF_MODE_REPLACE_STC replaceInfo[] =
        {
            /* ability to state 'no speed' and 'no interface' by using '10M' speed from the XML,
                , as PDL not supports 'no speed' and 'no interface'  */
            {
                /*old*/{GT_NA                                    ,CPSS_PORT_SPEED_10_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_NA_E            ,CPSS_PORT_SPEED_NA_E}
            },
            /* allow to set {USX_20G_OXGMII , *} , as PDL not supports USX_20G_OXGMII */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_XHGS_E          ,GT_NA/*all speeds*/},
                /*new*/{CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E   ,GT_NA}
            },
            /* allow to set {KR , 100G} to become {KR4 , 100G} , as PDL not supports KR4 */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_KR_E            ,CPSS_PORT_SPEED_100G_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_KR4_E           ,CPSS_PORT_SPEED_100G_E}
            },

            /* must be last */
            EZB_XML_SPEED_IF_MODE_REPLACE___MUAST_BE_LAST__CNS
        };
        rc = appDemoEzbPortsSpeedAndModeUpdate(phoenix_portInitlist__from_xml,replaceInfo);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbPortsSpeedGet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    cpssOsPrintf(" -- EZ_BRINGUP : The default port speed from XML file [%s] \n",
        ezbXmlName());
    *portInitListPtrPtr = phoenix_portInitlist__from_xml;
    return GT_OK;
}

/**
* @internal phoenix_initSerdesMuxingAndPolaritySwap function
* @endinternal
*
*/
static GT_STATUS phoenix_initSerdesMuxingAndPolaritySwap_fromEzbXml
(
    IN  GT_U32                              devIndex,
    OUT EZB_GEN_MAC_SERDES_INFO_STC   **macToSerdesArrPtr,
    OUT GT_U32                              *macToSerdesArr_arrSizePtr,
    OUT EZB_GEN_SERDES_INFO_STC       **serdesArrPtr,
    OUT GT_U32                              *serdesArr_arrSizePtr
)
{
    static EZB_GEN_MAC_SERDES_INFO_STC macToSerdesArr_fromEzbXml[PORT_MAP_MAX_PORTS_CNS];
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    GT_U32  ii,jj,numOfMuxingSerdeses;

#if EZ_BRINGUP
    ezbPpConfigPtr = &ezbPpConfigList[devIndex];
#else
    (void)devIndex;
    ezbPpConfigPtr = NULL;
#endif
    CPSS_NULL_PTR_CHECK_MAC(ezbPpConfigPtr);

    macGenInfoPtr = ezbPpConfigPtr->macGenInfoPtr;

    cpssOsPrintf("XML : mac-to-serdes info : \n");
    cpssOsPrintf("      mac      |      local SERDESes  \n");
    cpssOsPrintf("======================================\n");

    numOfMuxingSerdeses = 0;
    for(ii = 0 ; ii < ezbPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(macGenInfoPtr->isMacUsed == GT_FALSE ||
           macGenInfoPtr->isSerdesInfoValid == GT_FALSE ||
           macGenInfoPtr->macSerdesInfo.holdSerdesMuxingInfo == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("      [%3.3d]   |   [%d]",
            macGenInfoPtr->macNum,
            macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[0]);

        for(jj = 1 ; jj < macGenInfoPtr->macSerdesInfo.numOfSerdeses; jj++)
        {
            cpssOsPrintf(",[%d]",
                macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[jj]);
        }
        cpssOsPrintf("\n");

        if(macGenInfoPtr->macSerdesInfo.firstMacPtr == NULL)/* should not be NULL ... but just in case */
        {
            macGenInfoPtr->macSerdesInfo.firstMacPtr = macGenInfoPtr;
        }

        cpssOsMemCpy(&macToSerdesArr_fromEzbXml[numOfMuxingSerdeses],
            &macGenInfoPtr->macSerdesInfo,
            sizeof(EZB_GEN_MAC_SERDES_INFO_STC));

        numOfMuxingSerdeses++;
    }

    *macToSerdesArr_arrSizePtr =  numOfMuxingSerdeses;
    *macToSerdesArrPtr         =  macToSerdesArr_fromEzbXml;

    if(numOfMuxingSerdeses)
    {
        cpssOsPrintf(" -- EZ_BRINGUP : The 'MAC-SERDES muxing' from XML file [%s] \n",
            ezbXmlName());
    }
    else
    {
        cpssOsPrintf(" -- EZ_BRINGUP : no MAC-SERDES muxing defined \n");
    }

    cpssOsPrintf("XML : serdes polarity info : \n");
    cpssOsPrintf("      SERDES   |  TX polarity |   RX polarity \n");
    cpssOsPrintf("==============================================\n");
    for(jj = 0 ; jj < ezbPpConfigPtr->numOfSerdeses ; jj++)
    {
        if(ezbPpConfigPtr->serdesGenInfoPtr[jj].polarityValid == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("        [%3.3d]   |   %s      |   %s  \n"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.laneNum
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertTx == GT_TRUE ? "true ":"false"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertRx == GT_TRUE ? "true ":"false"
            );
    }

    *serdesArr_arrSizePtr      =  ezbPpConfigPtr->numOfSerdeses;   /* can be ZERO */
    *serdesArrPtr              =  ezbPpConfigPtr->serdesGenInfoPtr;/* can be NULL */

    if(ezbPpConfigPtr->numOfSerdeses)
    {
        cpssOsPrintf(" -- EZ_BRINGUP : The 'SERDES lane swaping' from XML file [%s] \n",
            ezbXmlName());
    }
    else
    {
        cpssOsPrintf(" -- EZ_BRINGUP : no 'SERDES lane swaping' (polarity invert) defined \n");
    }

    return GT_OK;
}

/**
* @internal prvAppDemoPhoenixSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES: AC5X.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoPhoenixSerdesPolarityConfigSet
(
    IN GT_U8 devNum
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              polarityArraySize;
    APPDEMO_SERDES_LANE_POLARITY_STC    *currentPolarityArrayPtr;
    CPSS_PP_DEVICE_TYPE                 devType;
    CPSS_DXCH_CFG_DEV_INFO_STC          devInfo;

    static APPDEMO_SERDES_LANE_POLARITY_STC  ac5x_DB_PolarityArray[] =
    {
    /* laneNum  invertTx    invertRx */
        { 11,   GT_FALSE,    GT_TRUE }
    };

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    devType = devInfo.genDevInfo.devType;

    switch(devType)
    {
        case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
            currentPolarityArrayPtr = ac5x_DB_PolarityArray;
            polarityArraySize = sizeof(ac5x_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
            break;

        default:
            return GT_OK;
    }
    for (i = 0; i < polarityArraySize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                currentPolarityArrayPtr[i].laneNum,
                currentPolarityArrayPtr[i].invertTx,
                currentPolarityArrayPtr[i].invertRx);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvAppDemoPhoenixLaneMacToSerdesMuxSet function
* @endinternal
*
* @brief   Per DB/RD board configure the mac to serdes mux settings
*
* @note   APPLICABLE DEVICES: AC5X.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoPhoenixLaneMacToSerdesMuxSet
(
    IN GT_U8 devNum,
    IN GT_U8 boardRevId
)
{
    GT_STATUS                           rc;
    CPSS_PORT_MAC_TO_SERDES_STC         *phoenixMacToSerdesMapPtr;
    CPSS_PP_DEVICE_TYPE                 devType;
    CPSS_DXCH_CFG_DEV_INFO_STC          devInfo;
    CPSS_DXCH_PORT_MAP_STC              portMap;
    GT_U32                              portNum;

    static CPSS_PORT_MAC_TO_SERDES_STC  phoenix_DB_MacToSerdesMap[] =
    {
        {{2, 1, 3, 0}} /* port 50,51,52,53
                               10,9, 11, 8*/
    };

    static CPSS_PORT_MAC_TO_SERDES_STC  phoenix_RD_MacToSerdesMap[] =
    {
        {{0, 1, 2, 3}} /* port 50,51,52,53
                               8, 9, 10, 11*/
    };

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    devType = devInfo.genDevInfo.devType;

    switch(devType)
    {
        case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
            if (boardRevId == 2 || boardRevId == 3) /* RD board */
            {
                phoenixMacToSerdesMapPtr = &phoenix_RD_MacToSerdesMap[0];
            }
            else /* DB board */
            {
                phoenixMacToSerdesMapPtr = &phoenix_DB_MacToSerdesMap[0];
            }
            break;

        default:
            return GT_OK;
    }

    for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if((portMap.interfaceNum) != 50 || rc != GT_OK)
        {
            continue;
        }

        rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                                               &phoenixMacToSerdesMapPtr[0]);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLaneMacToSerdesMuxSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

static GT_U32                              phoenix_MacToSerdesMap_arrSize = 0;
static EZB_GEN_MAC_SERDES_INFO_STC*   phoenix_MacToSerdesMap = NULL;
static GT_U32                              phoenix_SerdesArray_arrSize = 0;
static EZB_GEN_SERDES_INFO_STC*       phoenix_SerdesArray = NULL;
/**
* @internal phoenix_initSerdesMuxingAndPolaritySwap function
* @endinternal
*
*/
static GT_STATUS phoenix_initSerdesMuxingAndPolaritySwap
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U8                    boardRevId
)
{
    GT_STATUS   rc;
    EZB_GEN_MAC_SERDES_INFO_STC *macSerdesPtr;
    EZB_GEN_SERDES_INFO_STC     *serdesPtr;
    GT_U32  ii;

    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = phoenix_initSerdesMuxingAndPolaritySwap_fromEzbXml(devIndex,
            &phoenix_MacToSerdesMap,
            &phoenix_MacToSerdesMap_arrSize,
            &phoenix_SerdesArray,
            &phoenix_SerdesArray_arrSize);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initSerdesMuxingAndPolaritySwap_fromEzbXml`", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /**************************/
    /* handle 'Serdes muxing' */
    /**************************/
    if(phoenix_MacToSerdesMap)
    {
        macSerdesPtr = &phoenix_MacToSerdesMap[0];

        for(ii = 0 ; ii < phoenix_MacToSerdesMap_arrSize ; ii++,macSerdesPtr++)
        {
            if(macSerdesPtr->holdSerdesMuxingInfo == GT_FALSE)
            {
                continue;
            }

            if(macSerdesPtr->firstMacPtr == NULL)
            {
                continue;
            }

            rc = cpssDxChPortLaneMacToSerdesMuxSet(CAST_SW_DEVNUM(devNum),
                    macSerdesPtr->firstMacPtr->physicalPort,/* The physical port that represent the 'group' */
                    &macSerdesPtr->macToSerdesMap);         /* The array of the lane swaps */
            if (rc != GT_OK &&
                rc != GT_NOT_SUPPORTED)/* HWS not supports the device yet 'mvHwsPortLaneMacToSerdesMuxSet' */
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLaneMacToSerdesMuxSet", rc);
                return rc;
            }
            else if(rc == GT_NOT_SUPPORTED)
            {
                cpssOsPrintf("WARNING : cpssDxChPortLaneMacToSerdesMuxSet not supported yet \n");
                /* no need for all ports to fail */
                break;
            }
        }
    }
    else
    {
        rc = prvAppDemoPhoenixLaneMacToSerdesMuxSet(devNum, boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoPhoenixLaneMacToSerdesMuxSet", rc);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /**************************/
    /* handle 'lane swaping'  */
    /* ('polarity invert')    */
    /**************************/
    if(phoenix_SerdesArray)
    {
        GT_BOOL wasCpssLaneInitDone = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr ? GT_TRUE : GT_FALSE;

        serdesPtr = &phoenix_SerdesArray[0];

        for(ii = 0 ; ii < phoenix_SerdesArray_arrSize ; ii++,serdesPtr++)
        {
            if(serdesPtr->polarityValid != GT_TRUE)
            {
                /* not hold info from the XML */
                continue;
            }

           rc = cpssDxChPortSerdesLanePolaritySet(CAST_SW_DEVNUM(devNum), 0/*portGroupNum*/,
                       serdesPtr->polarity.laneNum,
                       serdesPtr->polarity.invertTx,
                       serdesPtr->polarity.invertRx);
            if (rc != GT_OK &&
                rc != GT_NOT_IMPLEMENTED)/* HWS not supports the device yet: in function hwsHawkIfInit :
                    hwsDeviceSpecInfo[devNum].serdesType = SERDES_LAST;             CPSS_TBD_BOOKMARK_AC5P */
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
                return rc;
            }
            else if(rc == GT_NOT_IMPLEMENTED)
            {
                cpssOsPrintf("WARNING : cpssDxChPortSerdesLanePolaritySet not supported yet \n");
                if(wasCpssLaneInitDone == GT_FALSE)
                {
                    /* free the DB if not created before , as the DB still exists
                       even though the support for polarity not exists
                       this to prevent fails during cpssInitSystem
                    */
                    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr);
                }
                /* no need for all ports to fail */
                break;
            }
        }
    }
    else
    {
        if ((boardRevId != 2) && (boardRevId != 3)) /* Not RD board */
        {
            rc = prvAppDemoPhoenixSerdesPolarityConfigSet(devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoPhoenixSerdesPolarityConfigSet", rc);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal phoenix_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS phoenix_PortModeConfiguration
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    PortInitList_STC *portInitListPtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    UNUSED_PARAM_MAC(devType);

    if(phoenix_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = phoenix_force_PortsInitListPtr;
    }
    else if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = phoenix_PortModeConfiguration_fromEzbXml(devIndex,&portInitListPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_PortModeConfiguration_fromEzbXml", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    else if (phoenix_only_ports_of_100G_mac)
    {
        cpssOsPrintf("NOTE: cpss give speed of 25G only to the 4 ports of the 100G MAC ... \n"
                     "as this is the only MAC supported by the CPSS at the moment (USX,CPU ... TBD)  \n");

        portInitListPtr = phoenix_portInitlist__only_ports_of_100G_mac;

        /* state the enh-UT about this mode */
        appDemoDbEntryAdd("phoenix_only_ports_of_100G_mac", 1);
    }
    else
    {
        switch (devType)
        {
            case CPSS_98DX3550_CNS:
            case CPSS_98DX3550M_CNS:
            case CPSS_98DX3510_CNS:
            case CPSS_98DX3510_H_CNS:
            case CPSS_98DX3510M_H_CNS:
            case CPSS_98DX3530_H_CNS:
            case CPSS_98DX3530M_H_CNS:
            case CPSS_98DX3510M_CNS:
            case CPSS_98DX3530_CNS:
            case CPSS_98DX3530M_CNS:
                /* AC5X 48G + 6x25G physical ports */
                portInitListPtr = phoenix_48G_6x25g_portInitlist;
                break;
            case CPSS_98DX3500_CNS:
            case CPSS_98DX3520_CNS:
            case CPSS_98DX3540_CNS:
            case CPSS_98DX3500M_CNS:
            case CPSS_98DX3520M_CNS:
            case CPSS_98DX3540M_CNS:
                /* AC5X 24G + 6x25G physical ports */
                portInitListPtr = phoenix_24G_6x25g_portInitlist;
                break;
            case CPSS_98DX3501_CNS:
            case CPSS_98DX3501M_CNS:
                /* AC5X 16G + 6x10 physical ports */
                portInitListPtr = phoenix_16G_6x25g_portInitlist;
                break;
            case CPSS_98DX7312_CNS:
            case CPSS_98DX7312M_CNS:
            case CPSS_98DXC712_CNS:
                /* Aldrin 3S 12x25G physical ports */
                portInitListPtr = phoenix_12x25g_portInitlist;
                break;
            case CPSS_98DX7308_CNS:
            case CPSS_98DX7308M_CNS:
            case CPSS_98DXC708_CNS:
                /* Aldrin 3S 8x25G physical ports */
                portInitListPtr = phoenix_8x25g_portInitlist;
                break;
            default:
                break;
        }
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        if ( portInitListPtr == NULL )
        {
            return GT_FAIL;
        }
        rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum), portInitListPtr, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* indication for function appDemoBc2PortInterfaceInit(...) to
           not call appDemoBc2PortListInit(...) */
        appDemo_PortsInitList_already_done = 1;
    }

    return GT_OK;
}

/**
* @internal phoenix_appDemoDbUpdate function
* @endinternal
*
* @brief   Update the AppDemo DB (appDemoPpConfigList[devIndex]) about the device
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS phoenix_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;  CPSS_TBD_BOOKMARK_PHOENIX

    return GT_OK;
}

/**
* @internal phoenix_MPP_set function
* @endinternal
*
* @brief   Change miscellaneous MPP configurations
*
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS phoenix_MPP_set
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc;
    /* MPP 19 is used as OUT for DEV_INIT_DONE signal.
       This signal is connected to RESET input of external CPU cards on DB board.
       AC5X Device toggles DEV_INIT_DONE during Soft Reset and this resets external CPU.
       Following configuration changes MPP settings to connecte constant value '1' to MPP 19..
       And external CPU will not get reset. */

    /* set output for GPIO 19 to be 1 */
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x7F018130, 19, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable output for GPIO 19 */
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x7F01812C, 19, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set MPP 19 to be GPIO 19 */
    rc = prvCpssDrvHwPpSetRegField(devNum, 0x80020108, 12, 4, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal phoenix_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS phoenix_initPhase1AndPhase2
(
    IN GT_SW_DEV_NUM       devNum,
    IN GT_U8              boardRevId,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN GT_U32             devIndex,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC       cpssPpPhase1Info;     /* CPSS phase 1 PP params */
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       cpssPpPhase2Info;     /* CPSS phase 2 PP params */
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    GT_U32                  boardType;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));

    switch(boardRevId)
    {
        case 1:
            boardType = APP_DEMO_XCAT5X_A0_BOARD_DB_CNS;
            break;
        case 2:
        case 3:
            boardType = APP_DEMO_XCAT5X_A0_BOARD_RD_CNS;
            break;
        default:
            boardType = APP_DEMO_XCAT5X_A0_BOARD_DB_CNS;
            break;
    }

    userForceBoardType(boardType);

    rc = appDemoDxChBoardTypeGet(SYSTEM_DEV_NUM_MAC(0), boardRevId, &bc2BoardType);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = getPpPhase1ConfigSimple(devNum, boardRevId, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;

    cpssPpPhase1Info.hwInfo[0] = *hwInfoPtr;

    /* devType is retrieved in hwPpPhase1Part1*/
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if (rc != GT_OK)
        return rc;

    *devTypePtr = devType;

    /* configure MPPs */
    rc = phoenix_MPP_set(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_MPP_set", rc);
    if (rc != GT_OK)
        return rc;

    /* check if the device did HW reset (soft/hard) */
    appDemoDxChCheckIsHwDidHwReset(devNum);

    /* update the appDemo DB */
    rc = phoenix_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    rc = appDemo_configure_dma_per_devNum(CAST_SW_DEVNUM(devNum) , ac5xInternalCpu);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemo_configure_dma_per_devNum", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    waIndex = sizeof(falcon_WaList)/sizeof(falcon_WaList[0]) - 1;

    if(waIndex)
    {
        rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),waIndex,falcon_WaList, NULL);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*************************************************/
    /* fill once the DB of 'port mapping' to be used */
    /*************************************************/
    rc = fillDbForCpssPortMappingInfo(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("fillDbForCpssPortMappingInfo", rc);
    if (rc != GT_OK)
        return rc;

    rc = phoenix_initPortMappingStage(devIndex,devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase2Init", rc);
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal phoenix_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS phoenix_getNeededLibs(
    OUT  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    OUT  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
)
{
    osMemSet(libInitParamsPtr,0,sizeof(*libInitParamsPtr));
    osMemSet(sysConfigParamsPtr,0,sizeof(*sysConfigParamsPtr));

    libInitParamsPtr->initBridge = GT_TRUE;/*prvBridgeLibInit*/
    libInitParamsPtr->initIpv4   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initIpv6   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initPcl    = GT_TRUE;/*prvPclLibInit*/
    libInitParamsPtr->initTcam   = GT_TRUE;/*prvTcamLibInit*/
    libInitParamsPtr->initPort   = GT_TRUE;/*prvPortLibInit*/
    libInitParamsPtr->initPhy    = GT_TRUE;/*prvPhyLibInit*/
    libInitParamsPtr->initNetworkIf = GT_TRUE;/*prvNetIfLibInit*/
    libInitParamsPtr->initMirror = GT_TRUE;/*prvMirrorLibInit*/
    libInitParamsPtr->initTrunk  = GT_TRUE;/*prvTrunkLibInit*/
    libInitParamsPtr->initPha    = GT_TRUE;/*prvPhaLibInit*/

    sysConfigParamsPtr->numOfTrunks         = _4K;    /*used by prvTrunkLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv4    = GT_TRUE;/*used by prvIpLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv6    = GT_TRUE;/*used by prvIpLibInit*/
    /*NOTYE: info sysConfigParamsPtr->lpmRamMemoryBlocksCfg is set later : used by prvIpLibInit*/

    sysConfigParamsPtr->pha_packetOrderChangeEnable = GT_FALSE;/*used by prvPhaLibInit*/

    return GT_OK;
}

/* number of SIP6 PBR entries in lpm line */
#define PRV_CPSS_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS 5

/**
* @internal phoenix_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS phoenix_initPpLogicalInit
(
    IN GT_SW_DEV_NUM       devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC    *lpmRamMemoryBlocksCfgPtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    GT_U32 value = 0;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));

    /* wanted to use small block for PBR  = 3200 (640*5) instead of 8K , but this cause bad behavior */
    /* so wanted to use 8K but this cause to use more than single big bank (5k in bank = 1K lines)
       8K fatal error on UTF_TEST_CASE_MAC(cpssDxChLpmLeafEntryWrite) */
    /* so using 5K */
    /* from more info
       see JIRA : CPSS-10305 : Phoenix : using 8K PBR cause cpss to write to non-existing memory */
    ppLogInitParams.maxNumOfPbrEntries = _5K;
    ppLogInitParams.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;/* don't care value for this device as it have no shared tables ! */

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
    {
        ppLogInitParams.maxNumOfPbrEntries = value;
    }

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        ppLogInitParams.sharedTableMode = value;
    }

    rc = cpssDxChCfgPpLogicalInit(CAST_SW_DEVNUM(devNum), &ppLogInitParams);
    if(rc != GT_OK)
        return rc;

    /* function must be called after cpssDxChCfgPpLogicalInit that update the value of :
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam */
    /* calculate info needed by 'prvDxCh2Ch3IpLibInit' (init of LPM manager) */
    rc = appDemoFalconIpLpmRamDefaultConfigCalc(CAST_SW_DEVNUM(devNum),
                                                ppLogInitParams.sharedTableMode,
                                                ppLogInitParams.maxNumOfPbrEntries,
                                                lpmRamMemoryBlocksCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

extern void pexAccessTracePreWriteCallbackBind(void (*pCb)(GT_U32, GT_U32));

static void appDemoHwPpDrvRegTracingPhoenixDpTracingRegAddressesBoundsSet
(
    IN GT_U8  devNum,
    IN GT_U32 unitsBitmap
)
{
    GT_STATUS                rc;
    GT_U32                   baseArr[20];
    GT_U32                   sizeArr[20];
    GT_U32                   i;
    GT_U32                   unitsNum;
    PRV_CPSS_DXCH_UNIT_ENT   unitId;
    GT_BOOL                  error;

    cpssOsMemSet(baseArr, 0, sizeof(baseArr));
    cpssOsMemSet(sizeArr, 0, sizeof(sizeArr));

    unitsNum = 0;
    for (i = 0; (i < 20); i++)
    {
        if ((unitsBitmap & (1 << i)) == 0) continue;
        switch (i)
        {
            case  0: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E; break;
            case  1: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E; break;
            case  2: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E; break;
            case  3: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E; break;
            case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E; break;
            case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E; break;
            case  6: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E; break;
            case  7: unitId = PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E; break;
            case  8: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E; break;
            case  9: unitId = PRV_CPSS_DXCH_UNIT_TXDMA_E; break;
            case 10: unitId = PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E; break;
            case 11: unitId = PRV_CPSS_DXCH_UNIT_PB_COUNTER_E; break;
            case 12: unitId = PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E; break;
            case 13: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E; break;
            case 14: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E; break;
            case 15: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E; break;
            case 16: unitId = PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E; break;
            case 17: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E; break;
            case 18: unitId = PRV_CPSS_DXCH_UNIT_RXDMA_E; break;
            default:
                /* unused bits */
                continue;
        }
        error = GT_FALSE;
        baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
            devNum, unitId, &error);
        if (error != GT_FALSE)
        {
            cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
            return;
        }
        rc = prvCpssSip6UnitIdSizeInByteGet(
            devNum, unitId, &(sizeArr[unitsNum]));
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
            return;
        }
        unitsNum ++;
    }

    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsSet(
        baseArr[0], sizeArr[0], baseArr[1], sizeArr[1],
        baseArr[2], sizeArr[2], baseArr[3], sizeArr[3],
        baseArr[4], sizeArr[4], baseArr[5], sizeArr[5],
        baseArr[6], sizeArr[6], baseArr[7], sizeArr[7],
        baseArr[8], sizeArr[8], baseArr[9], sizeArr[9],
        baseArr[10], sizeArr[10], baseArr[11], sizeArr[11],
        baseArr[12], sizeArr[12], baseArr[13], sizeArr[13],
        baseArr[14], sizeArr[14], baseArr[15], sizeArr[15],
        baseArr[16], sizeArr[16], baseArr[17], sizeArr[17],
        baseArr[18], sizeArr[18], baseArr[19], sizeArr[19],
        0, 0);
}

static GT_BOOL phoenixPreWriteTrace_FirstCall = GT_TRUE;
static GT_U8   phoenixPreWriteTrace_devNum = 0;
static GT_U32  phoenixPreWriteTrace_UnitsBitmap = 0;

static void phoenixPreWriteTrace(GT_U32 address, GT_U32 data)
{
    if (phoenixPreWriteTrace_UnitsBitmap == 0)
    {
        /* no units bound - ready to new bind */
        phoenixPreWriteTrace_FirstCall = GT_TRUE;
        return;
    }
    if (phoenixPreWriteTrace_FirstCall != GT_FALSE)
    {
        if (PRV_CPSS_PP_MAC(phoenixPreWriteTrace_devNum) == 0)
        {
            /* no DB yet */
            return;
        }
        if (PRV_CPSS_DXCH_PP_MAC(phoenixPreWriteTrace_devNum)->accessTableInfoSize == 0)
        {
            /* no table DB yet */
            return;
        }
        /* bind at first call when unit addresses DB already configured */
        phoenixPreWriteTrace_FirstCall = GT_FALSE;
        appDemoHwPpDrvRegTracingPhoenixDpTracingRegAddressesBoundsSet(
            phoenixPreWriteTrace_devNum,
            phoenixPreWriteTrace_UnitsBitmap);
    }
    if (appDemoHwPpDrvRegTracingIsTraced(address) == 0) return;

    osPrintf("0x%08X 0x%08X\n", address, data);
}

void appDemoHwPpDrvRegTracingPhoenixDpPreWriteEnable
(
    IN GT_U8  devNum,
    IN GT_U32 unitsBitmap
)
{
    phoenixPreWriteTrace_devNum      = devNum;
    phoenixPreWriteTrace_UnitsBitmap = unitsBitmap;
    pexAccessTracePreWriteCallbackBind(&phoenixPreWriteTrace);
}


/**
* @internal phoenix_appDemoInitSequence function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS phoenix_appDemoInitSequence
(
    IN GT_U8              boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     sysConfigParams;
    GT_BOOL supportSystemReset = GT_TRUE;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;
    GT_U32      tmpData;


    /* Set debug devType if needed */
    rc = appDemoDebugDeviceIdGet(CAST_SW_DEVNUM(devNum), &devType);
    if (GT_OK == rc)
    {
        rc = prvCpssDrvDebugDeviceIdSet(CAST_SW_DEVNUM(devNum), devType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = phoenix_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }


    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    if(!cpssDeviceRunCheck_onEmulator())
    {
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
        if(boardRevId == 2 || boardRevId == 3) /* RD board */
        {
            rc = gtAppDemoPhyMpdInit(devNum,boardRevId);  /* pass 0 means switch chip #0  */
            CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5x_initPhys", rc);
        }
#endif
        rc = phoenix_initSerdesMuxingAndPolaritySwap(devIndex,devNum,boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initStaticSerdesMuxing", rc);
        if (rc != GT_OK)
            return rc;
    }

    if (portMgr)
    {
        rc = cpssDxChPortManagerInit(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMgrInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = appDemoPortManagerTaskCreate(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortManagerTaskCreate", rc);
        if (rc != GT_OK)
            return rc;
    }


    rc = phoenix_PortModeConfiguration(devIndex,devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = phoenix_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = phoenix_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_initPpLogicalInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxLibrariesInit(CAST_SW_DEVNUM(devNum),&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxLibrariesInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxPpGeneralInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxPpGeneralInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoBc2PIPEngineInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PIPEngineInit", rc);
    if(rc != GT_OK)
        return rc;

#if 0
    CPSS_TBD_BOOKMARK_PHOENIX /* cpss not ready yet */
    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;
#endif /*0*/


    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChCfgDevInfoGet(CAST_SW_DEVNUM(devNum), &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
        return rc;

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    #endif /*GM_USED*/
    if(cpssDeviceRunCheck_onEmulator() ||
       /* AC5X A0 does not support Soft Reset when internal CPU is used.
          The erratum fixed in A1 revision */
       ((ac5xInternalCpu == GT_TRUE) && (devInfo.genDevInfo.revision == 0)))
    {
        /* emulator and internal CPU do not support system reset */
        supportSystemReset = GT_FALSE;
    }

#ifndef ASIC_SIMULATION
    /* not support gtShutdownAndCoreRestart */
    appDemoPpConfigList[devIndex].devSupportSystemReset_forbid_gtShutdownAndCoreRestart = GT_TRUE;
    cpssOsPrintf("NOTE: the Phoenix HW crash the gtShutdownAndCoreRestart(...) \n");
#endif /* ASIC_SIMULATION */

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;
    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal phoenix_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS phoenix_EventHandlerInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    GT_U32 value;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    GT_UNUSED_PARAM(devNum);

    /* appDemoEventHandlerPreInit:
        needed to be called before:
        boardCfgFuncs.boardAfterInitConfig(...)
        and before:
        appDemoEventRequestDrvnModeInit()
        */
    rc = appDemoEventHandlerPreInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (GT_OK == appDemoDbEntryGet("initSystemWithoutInterrupts", &value) && value)
    {
        /* we not want interrupts handling */
        PRINT_SKIP_DUE_TO_DB_FLAG("event handling","initSystemWithoutInterrupts");
    }
    else
    {
        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            PRINT_SKIP_DUE_TO_DB_FLAG("appDemoEventRequestDrvnModeInit ","system recovery HA process");
            return GT_OK;
        }

        rc = appDemoEventRequestDrvnModeInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

extern GT_STATUS   appDemoFalconMainUtForbidenTests(void);
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appDemoPhoenixMainUtForbidenTests(void)
{
#ifdef IMPL_TGF
    static FORBIDEN_TESTS mainUt_serdes [] =
    {
         TEST_NAME(cpssDxChPortSerdesGroupGet)
        ,TEST_NAME(cpssDxChPortSerdesPowerStatusSet)
        ,TEST_NAME(cpssDxChPortSerdesResetStateSet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTune)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeSet)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeGet)
        ,TEST_NAME(cpssDxChPortSerdesPolaritySet)
        ,TEST_NAME(cpssDxChPortSerdesPolarityGet)
        ,TEST_NAME(cpssDxChPortSerdesTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesTuningGet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableSet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableGet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigGet)
        ,TEST_NAME(cpssDxChPortEomBaudRateGet)
        ,TEST_NAME(cpssDxChPortEomDfeResGet)
        ,TEST_NAME(cpssDxChPortEomMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTuneStatusGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigGet)
        ,TEST_NAME(cpssDxChPortSerdesEyeMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningGet)

        ,TEST_NAME(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet)/* also read SERDES non supported register */
        ,TEST_NAME(cpssDxChCatchUpValidityCheckTest)                          /* also read SERDES non supported register - for recovery */

        /* other non-serdes tests : */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterRead) /*SMI is not supported as we not init the PHY lib ... see appDemoDbEntryAdd ("initPhy",0); */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterWrite)/*SMI is not supported as we not init the PHY lib ... see appDemoDbEntryAdd ("initPhy",0); */

        /* PLL tests */
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableGet)
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableSet)


        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {
         SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES mainUt_forbidenSuites_Emulator_too_long_and_PASS [] =
    {
         SUITE_NAME(cpssDxChTrunkHighLevel_1) /* cpssDxChTrunkHighLevel is enough to run */
        ,SUITE_NAME(cpssDxChTrunkLowLevel_1)  /* cpssDxChTrunkLowLevel  is enough to run */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*tgfPtp.*/tgfPtpEgressTimestampQueueinterrupt   )
        ,TEST_NAME(/*tgfCutThrough.*/prvTgfCutThroughIpv4CtTermination_SlowToFast         ) /* port modes not supported yet */

#ifdef WIN32
        ,SUITE_NAME(prvTgfIpLpmHaCheck         )/* crash on NULL pointer only on WIN32 !!! */
#endif /*WIN32*/

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_too_long_and_PASS [] =
    {
         TEST_NAME(cpssDxChDiagAllMemTest)                  /* : takes [313] sec */
        ,TEST_NAME(cpssDxChLatencyMonitoringPortCfgSet)     /* : takes [219] sec */
        ,TEST_NAME(cpssDxChPtpTaiTodStepSet)                /* : takes [119] sec */
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlSet)  /* : takes [1322] sec*/

        ,TEST_NAME(prvCpssDxChTestFdbHash_8K)               /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_16K)              /* : takes [92] sec */
        /*,TEST_NAME(prvCpssDxChTestFdbHash_32K) allow one of those to run */  /* : takes [92] sec */
        ,TEST_NAME(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)/* : takes [167] sec*/
        ,TEST_NAME(cpssDxChExactMatchRulesDump)             /* : takes [209] sec */
        ,TEST_NAME(cpssDxChPortLoopbackPktTypeToCpuSet)     /* : takes [295] sec */

        /* enhUT : */
        ,TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection) /* : takes [252] sec */
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting    ) /*takes [147] sec*/
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes      ) /*takes [215] sec*/
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck  ) /*takes [371] sec*/
        ,TEST_NAME(prvTgfPclMetadataPacketType         ) /*takes [804] sec*/

        ,{NULL}/* must be last */
    };


    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdQosTcDpRemapTableAccessModeGet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTaiPtpPulseInterfaceSet       )
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTaiPtpPulseInterfaceGet       )

        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlSet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlGet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTsFrameCounterIndexSet)

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*tgfMpls.*/tgfMplsSrEntropyLabel) /* segmentation fail */
        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES enhancedUt_forbidenSuites_Emulator_Killer [] =
    {

        {NULL}/* must be last */
    };


    static FORBIDEN_TESTS enhUtFailOn4PortsLimit [] =
    {
         TEST_NAME(tgfPortTxSchedulerSp)
        ,TEST_NAME(/*tgfBridge.*/tgfBasicDynamicLearning_fromCascadePortDsaTagForward )
        ,TEST_NAME(/*tgfBridge.*/tgfVlanManipulationEthernetOverMpls                  )
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressMirrorSourceBased                        )
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressMirrorEndToEnd                           )
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressMirrorEportVsPhysicalPort                )
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressVlanMirroring                            )
        ,TEST_NAME(/*tgfMirror.*/prvTgfIngressMirrorAnalyzerIsEportIsVidx             )
        ,TEST_NAME(/*tgfMirror.*/prvTgfIngressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
        ,TEST_NAME(/*tgfMirror.*/prvTgfEgressMirrorToAnalyzerCounter                  )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkSaLearning                                    )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkSaLearning_globalEPortRepresentTrunk          )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkSimpleHashMacSaDa                             )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk   )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkCrcHashMaskPriority                           )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkCascadeTrunk                                  )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkDesignatedTableModes                          )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkHashMplsModeEnableSet                         )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkHashMaskSet                                   )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkHashIpShiftSet                                )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkCascadeTrunkWithWeights                       )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkSortMode                                      )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort  )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkIpclLoadBalancingHash                         )
        ,TEST_NAME(/*tgfTrunk.*/tgfTrunkPacketTypeHashModeTest                        )
        ,TEST_NAME(/*tgfL2Mll.*/prvTgfTtlException                                    )
        ,TEST_NAME(/*tgfL2Mll.*/prvTgfEgressInterface                                 )
        ,TEST_NAME(/*tgfL2Mll.*/prvTgfEgressInterfaceEntrySelector                    )

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_LongerThanOneMinute [] =
    {
         TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection)
        ,TEST_NAME(tgfPortTxSchedulerSp)
        ,TEST_NAME(tgfPortTxTailDropDbaPortResources)
        ,TEST_NAME(prvTgfBrgVplsBasicTest1)
        ,TEST_NAME(tgfBasicTrafficSanity)
        ,TEST_NAME(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
        ,TEST_NAME(tgfVlanManipulationEthernetOverMpls)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByIndex)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByMsg)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfBrgGenMtuCheck)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRouting)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingRandomEnable)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
        ,TEST_NAME(prvTgfIpv4NonExactMatchUcRouting)
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting)
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck)
        ,TEST_NAME(prvTgfPclMetadataPacketType)
        ,TEST_NAME(prvTgfPclMetadataPacketTypeIpOverMpls)
        ,TEST_NAME(prvTgfIngressMirrorEportVsPhysicalPort)
        ,TEST_NAME(prvTgfEgressMirrorEportVsPhysicalPort)
        ,TEST_NAME(tgfTrunkSaLearning)
        ,TEST_NAME(tgfTrunkSaLearning_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkWithRemoteMembers)
        ,TEST_NAME(tgfTrunkCrcHashMaskPriority)
        ,TEST_NAME(tgfTrunkCascadeTrunk)
        ,TEST_NAME(tgfTrunkDesignatedTableModes)
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)
        ,TEST_NAME(tgfTrunkSortMode)
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL)
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)
        ,TEST_NAME(tgfIpfixSamplingToCpuTest)
        ,TEST_NAME(tgfIpfixAlarmEventsTest)
        ,TEST_NAME(prvTgfUnucUnregmcBcFiltering)
        ,TEST_NAME(prvTgfEgressInterface)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
        ,TEST_NAME(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
        ,TEST_NAME(prvTgfTunnelTermPbrDualLookup)
        ,{NULL}/* must be last */
    };
#endif /*IMPL_TGF*/


    /* tests that defined forbidden by Phoenix */

#ifdef IMPL_TGF
    appDemoForbidenTestsAdd (mainUt_forbidenTests_FatalError);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH_FAIL);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_FatalError);

    if(phoenix_only_ports_of_100G_mac)
    {
        /* we run in limited environment that not supports more than 4 ports */
        /* so there are tests that expected to fail                          */
        appDemoForbidenTestsAdd (enhUtFailOn4PortsLimit);
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("cpssDxChExactMatchManager","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");

        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (enhancedUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (mainUt_serdes);
        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd(mainUt_forbidenSuites_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd (enhancedUt_forbidenSuites_Emulator_Killer);

        {
            GT_U32 skip = 0;
            appDemoDbEntryGet("onEmulatorSkipSlowEnhancedUt", &skip);
            if (skip)
            {
                appDemoForbidenTestsAdd(
                    enhancedUt_forbidenTests_Emulator_LongerThanOneMinute);
            }
        }
    }
#endif /*IMPL_TGF*/

    /* add tests that defined forbidden by Falcon */
    return appDemoFalconMainUtForbidenTests();
}

/**
* @internal phoenix_localUtfInit function
* @endinternal
*
*/
static GT_STATUS phoenix_localUtfInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF

    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    rc = utfInit(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(rc);
#else
    devNum = devNum;
#endif /* INCLUDE_UTF */

    appDemoPhoenixMainUtForbidenTests();

    return rc;
}

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum, GT_BOOL tcamDaemonEnable);
extern GT_U32 prvCpssDxChHwIsUnitUsed_Phoenix_Emulator_Bypass_TCAM_get(void);

/** @struct PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC
*
*   @brief Positions of LED bit indications in stream.
*
*/
typedef struct {
    /** The first bit in the LED stream indication to be driven in current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart;
    /** The last bit in the LED stream indication to be driven in the current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd;
} PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC;

static PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC ac5x_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 79},   /* LED Unit 0 - Ports  0-15 */
    {64, 79},   /* LED Unit 1 - Ports 16-31 */
    {64, 79},   /* LED Unit 2 - Ports 32-47 */
    {64, 65},   /* LED Unit 3 - Ports 48-49 */
    {64, 67}    /* LED Unit 4 - Ports 50-53 */
};

static PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC ac5x_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    { 0,  0},   /* LED Unit 0 - Excluded from LED stream */
    { 0,  0},   /* LED Unit 1 - Excluded from LED stream */
    { 0,  0},   /* LED Unit 2 - Excluded from LED stream */
    {64, 65},   /* LED Unit 3 - Ports 48-49 */
    {64, 67}    /* LED Unit 4 - Ports 50-53 */
};

/**
* @internal led_port_position_get_mac
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] portMacNum            - current MAC to check
* @param[in] boardRevId            - board revision ID
*
* @retval GT_U32                   - the LED ports position for the current MAC port.
*
*/
static GT_U32 led_port_position_get_mac
(
    IN GT_U32   portMacNum,
    IN GT_U8    boardRevId
)
{
    GT_U32 ledPosition;

    if (portMacNum < 48)
    {
        if(boardRevId == 2 || boardRevId == 3) /* RD board */
        {
            ledPosition = APPDEMO_BAD_VALUE;
        }
        else
        {
            ledPosition = portMacNum % 16;
        }
    }
    else
    {
        if (portMacNum < 50)
        {
            ledPosition = portMacNum - 48;
        }
        else
        {
            ledPosition = portMacNum - 50;
        }
    }

    return ledPosition;
}

/**
* @internal phoenixLedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - device number
* @param[in] boardRevId            - board revision ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS phoenix_LedInit
(
    IN GT_U8    devNum,
    IN GT_U8    boardRevId
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          ledUnit;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    GT_U32                          portMacNum;
    PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC * ledStreamIndication;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    ledConfig.ledOrganize                       = CPSS_LED_ORDER_MODE_BY_CLASS_E;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = 1627;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = CPSS_LED_PULSE_STRETCH_1_E;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if(boardRevId == 2 || boardRevId == 3) /* RD board */
    {
        ledStreamIndication = ac5x_RD_led_indications;
    }
    else
    {
        ledStreamIndication = ac5x_led_indications;
    }

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        if(ledStreamIndication[ledUnit].ledStart == 0 && ledStreamIndication[ledUnit].ledEnd == 0)
        {
            /* The LED unit is exluded from the LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_TRUE;
        }
        else
        {
            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0 */
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
            /* The LED unit is included into LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
        }
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for phoenix */
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = GT_FALSE;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = 0;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = GT_FALSE;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                                   CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for phoenix */
                                                   ,classNum ,&ledClassManip);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 0,  Position 0   Ports  0.. 7                                              *
     *           P1,    LED Server 0,  Position 8   Ports  8..15                                              *
     *           P2,    LED Server 1,  Position 0   Ports 16..23                                              *
     *           P3,    LED Server 1,  Position 8   Ports 24..32                                              *
     *           P4,    LED Server 2,  Position 0   Ports 32..39                                              *
     *           P5,    LED Server 2,  Position 8   Ports 40..47                                              *
     *           P6,    LED Server 3,  Position 0   Port  48                                                  *
     *           P7,    LED Server 3,  Position 1   Port  49                                                  *
     *           P8,    LED Server 4,  Position 0   Ports 50..53                                              *
     *                                                                                                        *
     *--------------------------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;
        position = led_port_position_get_mac(portMacNum, boardRevId);

        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, /*classNum*/2, /*invertEnable*/1);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortClassPolarityInvertEnableSet", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal phoenix_phyInit function
* @endinternal
*
* @brief   PHY related configurations
*
* @param[in] devNum                - device number
* @param[in] phySmiInterface       - PHY SMI interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS phoenix_phyInit
(
    IN  GT_U8  devNum,
    IN CPSS_PHY_SMI_INTERFACE_ENT phySmiInterface
)
{
    GT_STATUS                       rc;
    GT_U32                          smiAddr;

    /* SMI0 LED specific PHY configurations */
    for(smiAddr = 0; smiAddr < 32; smiAddr++)
    {
        /* Page 28 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, phySmiInterface, smiAddr, 22, 28);
        if( rc != GT_OK)
        {
            return rc;
        }
        /* LED matrix mode */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, phySmiInterface, smiAddr, 1, 0x400);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* Page 3 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, phySmiInterface, smiAddr, 22, 3);
        if( rc != GT_OK)
        {
            return rc;
        }
        /* LED Link/Activity */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, phySmiInterface, smiAddr, 16, 0x1181);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* Change to Page 0 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, phySmiInterface, smiAddr, 22, 3);
        if( rc != GT_OK)
        {
            return rc;
        }
    }

    /* XSMI1 LED specific PHY configurations */
    for (smiAddr = 0; smiAddr < 16; smiAddr++) {
        rc = cpssXsmiPortGroupRegisterWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_XSMI_INTERFACE_0_E, smiAddr, 0xf020, 31, 0x0138);
        if ( rc != GT_OK) {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal gtDbPhoenixBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Phoenix device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbPhoenixBoardReg_SimpleInit
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_PCI_INFO pciInfo;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
    /* allow setting clock after the SOFT-RESET during the 'system reset'
       and 'system init' */
    if(simCoreClockOverwrittenGet())
    {
        simCoreClockOverwrite(simCoreClockOverwrittenGet());
    }
#endif

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;

    /* this function finds all Prestera devices on PCI bus */
    rc = phoenix_getBoardInfoSimple(
            &hwInfo[0],
            &pciInfo);
    if (rc != GT_OK)
        return rc;

    /* give 'hint' to the CPSS that the device is of Phoenix type .
       so it will know to access the MG registers in proper offset ...
       to read register 0x4c to get the 'deviceId'
    */
    prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_AC5X_E);

    appDemoPpConfigDevAmount = 1;
    devIndex = SYSTEM_DEV_NUM_MAC(0);
    devNum =   devIndex;

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    if (boardRevId == 2 || boardRevId == 3)
    {
        phoenix_boardTypePrint("RD" /*boardName*/, "Phoenix" /*devName*/);
    }
    else
    {
        phoenix_boardTypePrint("DB" /*boardName*/, "Phoenix" /*devName*/);
    }

    rc = phoenix_appDemoInitSequence(boardRevId, devIndex, devNum, hwInfo);
    if (rc != GT_OK)
        return rc;
    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        /* check if we need to init the MPD to support phys */
        rc = appDemoEzbMpdPpPortInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPpPortInit", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* init the event handlers */
    rc = phoenix_EventHandlerInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        /* Data Integrity module initialization. It should be done after events init. */
        rc = appDemoDataIntegrityInit(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    rc = phoenix_localUtfInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = phoenix_LedInit(devNum, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_LedInit", rc);
    if (rc != GT_OK)
        return rc;

    if (boardRevId == 2 || boardRevId == 3)
    {
        rc = phoenix_phyInit(devNum, CPSS_PHY_SMI_INTERFACE_1_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_phyInit", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* CPSSinit time measurement */
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("cpssInitSystem time: %d sec., %d nanosec.\n", seconds, nanoSec);


    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the cpssInitSimple (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbPhoenixBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbPhoenixBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;
    actualNum_cpssApi_phoenix_defaultMap = 0;

    return GT_OK;
}

/**
* @internal gtDbDxPhoenixBoardReg function
* @endinternal
*
* @brief   Registration function for the Phoenix board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxPhoenixBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbPhoenixBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbPhoenixBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS internal_onEmulator(GT_BOOL doInitSystem);
static GT_U32 phoenix_emulator_deviceId = CPSS_98DX7312_CNS;/* for to use value recognized by the CPSS ! */
/* function to be called before phoenix_onEmulator_prepare to allow other/ZERO the 'phoenix_emulator_deviceId' */
GT_STATUS phoenix_emulator_deviceId_set(GT_U32   deviceId)
{
    phoenix_emulator_deviceId = deviceId;
    return GT_OK;
}
extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

extern GT_STATUS cnc_onEmulator_extremely_slow_emulator_set(GT_U32   timeToSleep);


GT_STATUS phoenix_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc =  internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 'NO KN' is doing polling in this interrupt global cause register */
    /* but this polling should not be seen during 'trace'                   */
    appDemoTrace_skipTrace_onAddress(PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS + 0x30 /*0x7F900030*//*address*/,0/*index*/);

    /*
        allow to see Addresses that CPSS not access because forbidden in the Emulator
    */
    trace_ADDRESS_NOT_SUPPORTED_MAC_set(1);

    /* CPSS for emulator is stable. LOG is not needed for common testing. */
#ifdef CPSS_LOG_ENABLE
    /* disable CPSS LOG */
    cpssLogEnableSet (0);
    /* disable LOG for ERRORs */
    cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,0);
#endif /*CPSS_LOG_ENABLE*/

    appDemoPrintLinkChangeFlagSet(GT_TRUE);

    if(phoenix_emulator_deviceId)/* assign only if not ZERO */
    {
        appDemoDebugDeviceIdSet(0, phoenix_emulator_deviceId);
    }

#ifdef IMPL_TGF
#ifdef INCLUDE_UTF
    /* in the cnc test with fdb upload :
       1. the device get to the fdb entry after some time. so we need to help it ... to wait more than usual
       2. the CNC upload takes longer than usual.
       influence the next 2 tests :
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_cncOverflowAndCombineWithFu",1,1
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_fullQueue",1,1
    */
    cnc_onEmulator_extremely_slow_emulator_set(3000);
#endif /* INCLUDE_UTF */
#endif /* IMPL_TGF */

    return GT_OK;
}

GT_STATUS  ac5x_clearMappingDb
(
    GT_VOID
)
{
    actualNum_cpssApi_phoenix_defaultMap =0;
    phoenix_force_PortsInitListPtr = NULL;
    return GT_OK;
}

