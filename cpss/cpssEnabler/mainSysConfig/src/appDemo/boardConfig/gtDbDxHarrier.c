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
* @file gtDbDxHarrier.c
*
* @brief Initialization functions for the Harrier - SIP6.20 - board.
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
#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/init/cpssInit.h>/* needed for : cpssPpWmDeviceInAsimEnvSet() */
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <gtOs/gtOsSharedPp.h>

/* indication for the ASIM project */
static GT_BOOL  appDemoInAsimEnv = GT_FALSE;
static GT_STATUS appDemoPostInitAsimEnv(void);
static GT_STATUS appDemoRunningInAsimEnv(void);
static GT_U8       last_boardRevId = 0;
static GT_BOOL  isUnderSecondDeviceCatchup = GT_FALSE;
static GT_BOOL  isSmmuNeededFor2Devices = GT_FALSE;

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

extern GT_STATUS appDemoIgnoreAutoLearnNaMsgSet(IN GT_U32 ignore);
extern GT_STATUS appDemoTreatNaMsgOnConnectionPortForRemoteDeviceSet(IN GT_U32 enable);

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS

#define DEFAULT_QUEUE_NUM         8
#define REMOTE_PORT_GROUP_OF_5_START_FROM_0(_startPrefix,_dma,_numberOfQueues)    {{ _startPrefix##0,_startPrefix##1, _startPrefix##2,_startPrefix##3,\
            _startPrefix##4,GT_NA} ,\
        { _dma,_dma,_dma,_dma,_dma ,GT_NA} ,\
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,_numberOfQueues,GT_FALSE}

#define REMOTE_PORT_GROUP_OF_5_START_FROM_5(_startPrefix,_dma,_numberOfQueues)    {{ _startPrefix##5,_startPrefix##6, _startPrefix##7,_startPrefix##8,\
            _startPrefix##9,GT_NA} ,\
        { _dma,_dma,_dma,_dma,_dma ,GT_NA} ,\
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,_numberOfQueues,GT_FALSE}


#define REMOTE_PORT_GROUP_OF_4_START_FROM_0(_startPrefix,_dma,_numberOfQueues)    {{ _startPrefix##0,_startPrefix##1, _startPrefix##2,_startPrefix##3,\
         GT_NA} ,\
        { _dma,_dma,_dma,_dma,GT_NA} ,\
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,_numberOfQueues,GT_FALSE}

#define REMOTE_PORT_GROUP_OF_4_START_FROM_5(_startPrefix,_dma,_numberOfQueues)    {{ _startPrefix##5,_startPrefix##6, _startPrefix##7,_startPrefix##8,\
            GT_NA} ,\
        { _dma,_dma,_dma,_dma,GT_NA} ,\
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,_numberOfQueues,GT_FALSE}


#define START_LOOP_ALL_DEVICES(_devNum) \
    for (devIndex = SYSTEM_DEV_NUM_MAC(0); \
          (devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)); devIndex++) \
    {                                                                            \
        _devNum =   appDemoPpConfigList[devIndex].devNum;                        \
        SYSTEM_SKIP_NON_ACTIVE_DEV((GT_U8)devIndex)

#define END_LOOP_ALL_DEVICES \
    }


static GT_U32   harrier_force_single_device = 0;
GT_STATUS   harrier_force_single_device_set(IN GT_U32   deviceToUse/*0 or 1*/)
{
    if(deviceToUse > 1)
    {
        osPrintf("harrier_force_single_device_set : ERROR : deviceToUse[%d] paramter MUST be 0 or 1 \n",
            deviceToUse);
        return GT_BAD_PARAM;
    }
    harrier_force_single_device = 1 + deviceToUse;
    return GT_OK;
}
/* flag to allow to set DSA cascade port on B2B
    by default not needed ... to be like customer
*/
static GT_U32   harrier_b2b_DSA = 0;
GT_STATUS   harrier_b2b_DSA_set(IN GT_U32   enable)
{
    harrier_b2b_DSA = enable;
    return GT_OK;
}


extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

#define DEV_TYPE_TO_2_FIELDS_MAC(devType)    {(GT_U16)(devType)/*vendorId*/ , (GT_U16)((devType) >> 16) /*devId*/}

/* define the ability to use limited list and not query the BUS for ALL historical / future devices */
/* if this pointer need to be used the caller must fill the vendorId with 0x11AB (GT_PCI_VENDOR_ID) */
/* and LAST entry MUST be 'termination' by vendorId = 0  !!*/
extern GT_PCI_DEV_VENDOR_ID    *limitedDevicesListPtr;

GT_PCI_DEV_VENDOR_ID    harrierLimitedDevicesList[] =
{
    /* list the devices that this appDemo expected to recognize and work with */

    /* NOTE:
       The devices that the ASIM recognize MUST be first in this list to reduce
       the time for the cpssInitSystem to run (on Asim) .
    */
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98CN106xxS_CNS),/*Harrier-MCM*/
    /* Aldrin3M devices */
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DX7321M_CNS),
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DXC720_CNS),
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DXC721_CNS),
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DX7320_CNS),
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DX7321_CNS),
    DEV_TYPE_TO_2_FIELDS_MAC(CPSS_98DX7320M_CNS),

    /* must be last -- termination */
    {0,0}
};

#define PORT_NUMBER_ARR_SIZE    18
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              numberOfQueuesPerPort;
    GT_BOOL                             reservePreemptiveChannel;
}HARRIER_PORT_MAP_STC;

/* mapping of Harrier physical ports to MAC ports (and TXQ ports). */
static HARRIER_PORT_MAP_STC aldrin3M_port_mode[] =
{
    /* DP[0] ports */
     {{ 12, 13, 14, 15, 16, 17, 18, 19 ,GT_NA} ,
      {  0,  2,  4,  6,  8, 10, 12, 14 ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}
    /* DP[1] ports */
    ,{{  4,  5,  6,  7,  8,  9, 10, 11 ,GT_NA} ,
      { 16, 18, 20, 22, 24, 26, 28, 30 ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}

    /* DP[2] ports - only 4 ports */
    ,{{  0,  1,  2,  3, GT_NA} ,
      { 32, 34, 36, 38, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}
     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 40/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE}
    ,{{60      /*60*/          ,GT_NA} , { 41/*DMA-MG1*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,DEFAULT_QUEUE_NUM,GT_FALSE}
};

static GT_U32   actualNum_aldrin3M_port_mode = sizeof(aldrin3M_port_mode)/sizeof(aldrin3M_port_mode[0]);

/* mapping of Harrier physical ports to MAC ports (and TXQ ports). */
static HARRIER_PORT_MAP_STC harrier_port_mode[] =
{
    /* DP[0] ports */
     {{ 0,2, 4, 6,8,10,12,14 ,GT_NA} ,
      { 0,2, 4, 6,8,10,12,14 ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}
    /* DP[1] ports */
    ,{{ 16,18, 20, 22,24,26,28,30 ,GT_NA} ,
      { 16,18, 20, 22,24,26,28,30 ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}

    /* DP[2] ports - only 4 ports */
    ,{{ 32,34, 36, 38,GT_NA} ,
      { 32,34, 36, 38,GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE}
     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 40/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE}
    ,{{60      /*60*/          ,GT_NA} , { 41/*DMA-MG1*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,DEFAULT_QUEUE_NUM,GT_FALSE}
};

static GT_U32   actualNum_harrier_port_mode = sizeof(harrier_port_mode)/sizeof(harrier_port_mode[0]);


/* DP 0 - 8 ports of 50G, each of the ports has 5 remote ports 10G each with 4Qs per remote port. */
/* DP 1   4 ports of 100G, each of the ports has 4 remote ports of 25G each with 8Qs per remote port*/
/* DP 2  4 ports of 50G, 8 Qs per port*/


static HARRIER_PORT_MAP_STC harrier_max_rpp_port_mode[] =
{
    /* DP[0] ports */
     {{ 0,2, 4, 6,8,10,12, 14,GT_NA} ,
      { 0,2, 4, 6,8,10,12, 14,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE/*do not reserve preemptive channels*/}

        /* Remote ports on DMA 0 with 4 queues (50 -54)*/
      ,REMOTE_PORT_GROUP_OF_5_START_FROM_0(5,0,4)
        /* Remote ports on DMA 2 with 4 queues (55 -59)*/
      ,REMOTE_PORT_GROUP_OF_5_START_FROM_5(5,2,4)
       /* Remote ports on DMA 4 with 4 queues (70 -74)*/
      ,REMOTE_PORT_GROUP_OF_5_START_FROM_0(7,4,4)
          /* Remote ports on DMA 6 with 4 queues (75-79)*/
      ,REMOTE_PORT_GROUP_OF_5_START_FROM_5(7,6,4)
       /* Remote ports on DMA 8 with 4 queues (80 -84)*/
      ,REMOTE_PORT_GROUP_OF_5_START_FROM_0(8,8,4)

       /* Remote ports on DMA 16 with 4 queues (85 -89)*/
       ,REMOTE_PORT_GROUP_OF_5_START_FROM_5(8,10,4)
        /* Remote ports on DMA 18 with 4 queues (90 -94)*/
       ,REMOTE_PORT_GROUP_OF_5_START_FROM_0(9,12,4)

       /* Remote ports on DMA 32 with 4 queues (95 -99)*/
       ,REMOTE_PORT_GROUP_OF_5_START_FROM_5(9,14,4)


        /* DP[1] ports */

      ,{{ 16,24,GT_NA} ,
       { 16, 24,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE}
        /* Remote ports on DMA 16 with 8 queues (100 -103)*/
       ,REMOTE_PORT_GROUP_OF_4_START_FROM_0(10,16,8)
        /* Remote ports on DMA 24 with 8 queues (105 -108)*/
       ,REMOTE_PORT_GROUP_OF_4_START_FROM_5(10,24,8)

        /* DP[2] ports */
        ,{{ 32, 34,36, 38,GT_NA} ,
          { 32, 34,36, 38,GT_NA} ,
          CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE}



     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 40/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE    }
    ,{{60      /*60*/          ,GT_NA} , { 41/*DMA-MG1*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,DEFAULT_QUEUE_NUM,GT_FALSE   }
};

static GT_U32   actualNum_harrier_max_rpp_port_mode = sizeof(harrier_max_rpp_port_mode)/sizeof(harrier_max_rpp_port_mode[0]);



static HARRIER_PORT_MAP_STC harrier_max_queue_port_mode[] =
{
        /* DP[0] ports */
       {{ 0,GT_NA} ,
       { 0,GT_NA} ,
         CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,248/*256-8( for cpu port)*/,GT_FALSE}

        /* DP[1] ports */

      ,{{ 16,GT_NA} ,
       { 16,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,220,GT_FALSE }

       /* DP[2] ports */

      ,{{ 32,34,36,38,GT_NA} ,
       { 32,34,36,38,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE}

       ,{{CPU_PORT/*63*/          ,GT_NA} , { 40/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM ,GT_FALSE   }

};

static GT_U32   actualNum_harrier_max_queue_port_mode = sizeof(harrier_max_queue_port_mode)/sizeof(harrier_max_queue_port_mode[0]);


static HARRIER_PORT_MAP_STC harrier_max_queue_port_mode_no_cpu[] =
{
        /* DP[0] ports */
       {{ 0,GT_NA} ,
       { 0,GT_NA} ,
         CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,256,GT_FALSE}

        /* DP[1] ports */

      ,{{ 16,GT_NA} ,
       { 16,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,256,GT_FALSE }

       /* no DP[2] ports */
};

static GT_U32   actualNum_harrier_max_queue_port_mode_no_cpu = sizeof(harrier_max_queue_port_mode_no_cpu)/sizeof(harrier_max_queue_port_mode_no_cpu[0]);


static HARRIER_PORT_MAP_STC harrier_max_queue_port_mode_distributed [] =
{
        /* DP[0] ports */
     {{ 0,2, 4, 6,8,10,12,14 ,GT_NA} ,
      { 0,2, 4, 6,8,10,12,14 ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,32,GT_FALSE}
    /* DP[1] ports */
    ,{{ 16, 20, 24,28, GT_NA} ,
      { 16, 20, 24,28, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,64,GT_FALSE}

       /* no DP[2] ports */
};

static GT_U32   actualNum_harrier_max_queue_port_mode_distributed = sizeof(harrier_max_queue_port_mode_distributed)/sizeof(harrier_max_queue_port_mode_distributed[0]);




/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

#define MAX_MAPPED_PORTS_CNS    128
static  GT_U32  actualNum_cpssApi_harrier_defaultMap = 0;/*actual number of valid entries in cpssApi_harrier_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_harrier_defaultMap[MAX_MAPPED_PORTS_CNS];
/* filled at run time , by info from harrier_defaultMap[] */

static PortInitList_STC *harrier_force_PortsInitListPtr = NULL;
static HARRIER_PORT_MAP_STC *harrier_forcePortMapPtr = NULL;
static GT_U32 * force_actualNum_harrier_port_modePtr = NULL;

#define HARRIER_BOARD_TYPE_DB_ALDRIN_3M         0
#define HARRIER_BOARD_TYPE_DB_106xxS            1
#define HARRIER_BOARD_TYPE_B2B_ALDRIN_106xxS    2
#define HARRIER_BOARD_TYPE_INVALID              3

static GT_U32 harrierBoardType = HARRIER_BOARD_TYPE_INVALID;


/* Aldrin3M ports in 50G mode */
static PortInitList_STC aldrin3M_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,19,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Harrier ports in 50G mode */
static PortInitList_STC harrier_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,38,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

#define DEV0_CONNECTION_PORT_200G   8 /* port on Harrier  (on dev 0 to dev 1) */ /*SD 4..7  */
#define DEV1_CONNECTION_PORT_200G  32 /* port on Aldrin3M (on dev 1 to dev 0) */ /*SD 16..19*/

static PortInitList_STC harrier_dev_0_b2b_portInitlist[] =
{
    /* 0..6 (in steps of 2)*/
     { PORT_LIST_TYPE_INTERVAL,  {0,6,2, APP_INV_PORT_CNS } , CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 (8,10,12,14) */
    ,{ PORT_LIST_TYPE_LIST,      {DEV0_CONNECTION_PORT_200G,APP_INV_PORT_CNS }      , CPSS_PORT_SPEED_200G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}
    /* 16..30 (in steps of 2) */
    ,{ PORT_LIST_TYPE_INTERVAL,  {16,30,2, APP_INV_PORT_CNS } , CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 32..36 (in steps of 2)*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {32,36,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};
static PortInitList_STC aldrin3m_dev_1_b2b_portInitlist[] =
{
    /* 0..30 (in steps of 2) */
     { PORT_LIST_TYPE_INTERVAL,  {0,30,2, APP_INV_PORT_CNS } , CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* 32 (32,34,36,38)     */
    ,{ PORT_LIST_TYPE_LIST,      {DEV1_CONNECTION_PORT_200G,APP_INV_PORT_CNS }      , CPSS_PORT_SPEED_200G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Harrier RPP ports */
static PortInitList_STC harrier_portInitlistRpp[] =
{
     /*DP0*/
     { PORT_LIST_TYPE_INTERVAL,  {0,14,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},
     { PORT_LIST_TYPE_INTERVAL,  {50,59,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     { PORT_LIST_TYPE_INTERVAL,  {70,79,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     { PORT_LIST_TYPE_INTERVAL,  {80,89,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     { PORT_LIST_TYPE_INTERVAL,  {90,99,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     /*DP1*/
     { PORT_LIST_TYPE_INTERVAL,  {16,24,4, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_200G_E ,  CPSS_PORT_INTERFACE_MODE_KR4_E},
     { PORT_LIST_TYPE_INTERVAL,  {100,103,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     { PORT_LIST_TYPE_INTERVAL,  {105,108,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     /*DP2*/
     { PORT_LIST_TYPE_INTERVAL,  {32,38,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},

     { PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Harrier Port Mode 4*/
static PortInitList_STC harrier_portInitlistMaxQ[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,28,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},
     { PORT_LIST_TYPE_INTERVAL,  {32,38,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},

     { PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/**
* @internal prvAppDemoHarrierSerdesConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES: Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Ironman; xCat3; AC5; AC5P.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoHarrierSerdesConfigSet
(
    IN GT_U8 devNum,
    IN GT_U8 boardRevId
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              polarityArraySize;
    APPDEMO_SERDES_LANE_POLARITY_STC    *currentPolarityArrayPtr;
    CPSS_PORT_MAC_TO_SERDES_STC         *currentMacToSerdesMapPtr;
    GT_U32                              harrier_MacToSerdesMap_arrSize;
    GT_U32                              portNum, portMacNum;
    CPSS_DXCH_PORT_MAP_STC              portMap;

    static APPDEMO_SERDES_LANE_POLARITY_STC  aldrin3M_DB_PolarityArray[] =
    {
        /* laneNum  invertTx    invertRx */
         { 0,   GT_FALSE,   GT_FALSE  }
        ,{ 1,   GT_FALSE,   GT_FALSE  }
        ,{ 2,   GT_FALSE,   GT_TRUE   }
        ,{ 3,   GT_FALSE,   GT_TRUE   }
        ,{ 4,   GT_FALSE,   GT_FALSE  }
        ,{ 5,   GT_FALSE,   GT_FALSE  }
        ,{ 6,   GT_FALSE,   GT_TRUE   }
        ,{ 7,   GT_FALSE,   GT_TRUE   }
        ,{ 8,   GT_FALSE,   GT_TRUE   }
        ,{ 9,   GT_FALSE,   GT_TRUE   }
        ,{ 10,  GT_FALSE,   GT_FALSE  }
        ,{ 11,  GT_FALSE,   GT_TRUE   }
        ,{ 12,  GT_FALSE,   GT_TRUE   }
        ,{ 13,  GT_FALSE,   GT_TRUE   }
        ,{ 14,  GT_FALSE,   GT_FALSE  }
        ,{ 15,  GT_FALSE,   GT_FALSE  }
        ,{ 16,  GT_FALSE,   GT_FALSE  }
        ,{ 17,  GT_FALSE,   GT_FALSE  }
        ,{ 18,  GT_FALSE,   GT_FALSE  }
        ,{ 19,  GT_FALSE,   GT_TRUE   }
    };

/*
    PHY_PORT_NUM   |  MAC |  SD
    _______________|___________
          0        |  32  |  19
          1        |  34  |  18
          2        |  36  |  17
          3        |  38  |  16
          4        |  16  |  12
          5        |  18  |  15
          6        |  20  |  13
          7        |  22  |  14
          8        |  24  |  11
          9        |  26  |  8
          10       |  28  |  10
          11       |  30  |  9
          12       |  0   |  4
          13       |  2   |  7
          14       |  4   |  5
          15       |  6   |  6
          16       |  8   |  3
          17       |  10  |  0
          18       |  12  |  2
          19       |  14  |  1
*/
    static CPSS_PORT_MAC_TO_SERDES_STC  aldrin3M_DB_MacToSerdesMap[] =
    {
        {{4,7,5,6,3,0,2,1}},    /* SD: 0-7    MAC: 0...14  */
        {{4,7,5,6,3,0,2,1}},    /* SD: 8-15   MAC: 16..30  */
        {{3,2,1,0,4,5,6,7}},    /* SD: 16-19  MAC: 32..38  */
    };

    /*
        PHY_PORT_NUM   |  MAC |  SD
        _______________|___________
              0        |  24  |  12
              1        |  30  |  15
              2        |  26  |  13
              3        |  28  |  14
              4        |  0   |  11
              5        |  22  |  8
              6        |  18  |  10
              7        |  20  |  9
              8        |  8   |  4
              9        |  14  |  7
              10       |  10  |  5
              11       |  12  |  6
              12       |  0   |  3
              13       |  6   |  0
              14       |  2   |  2
              15       |  4   |  1
    */
    static CPSS_PORT_MAC_TO_SERDES_STC  harrier_106xxS_DB_MacToSerdesMap[] =
    {
        {{3,2,1,0,4,5,6,7}},    /* SRDS: 0-7    MAC: 0...14  */
        {{3,2,1,0,4,5,6,7}},    /* SRDS: 8-15   MAC: 16..30  */
    };

    UNUSED_PARAM_MAC(boardRevId);

    switch(harrierBoardType)
    {
        case HARRIER_BOARD_TYPE_DB_ALDRIN_3M:
            currentPolarityArrayPtr = aldrin3M_DB_PolarityArray;
            polarityArraySize = sizeof(aldrin3M_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
            currentMacToSerdesMapPtr = aldrin3M_DB_MacToSerdesMap;
            harrier_MacToSerdesMap_arrSize = sizeof(aldrin3M_DB_MacToSerdesMap) / sizeof(CPSS_PORT_MAC_TO_SERDES_STC);
            break;

        case HARRIER_BOARD_TYPE_DB_106xxS:
            currentPolarityArrayPtr = NULL;
            polarityArraySize = 0;
            currentMacToSerdesMapPtr = harrier_106xxS_DB_MacToSerdesMap;
            harrier_MacToSerdesMap_arrSize = sizeof(harrier_106xxS_DB_MacToSerdesMap) / sizeof(CPSS_PORT_MAC_TO_SERDES_STC);
            break;

        case HARRIER_BOARD_TYPE_B2B_ALDRIN_106xxS:
        case HARRIER_BOARD_TYPE_INVALID:
        default:
            return GT_OK;
    }

    if(currentPolarityArrayPtr != NULL)
    {
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
    }

    /**************************/
    /* handle 'Serdes muxing' */
    /**************************/
    if(currentMacToSerdesMapPtr != NULL)
    {
        for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
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
            if(harrier_MacToSerdesMap_arrSize <= (portMacNum/16))
            {
                break;
            }
            if((portMacNum % 16) != 0)
            {
                continue;
            }
            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&currentMacToSerdesMapPtr[portMacNum/16]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/* fill array of cpssApi_harrier_defaultMap[] and update actualNum_cpssApi_harrier_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    HARRIER_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;

    if(actualNum_cpssApi_harrier_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    if(NULL == harrier_force_PortsInitListPtr||NULL == force_actualNum_harrier_port_modePtr)
    {
      currAppDemoInfoPtr = &aldrin3M_port_mode[0];
      numEntries = actualNum_aldrin3M_port_mode;
    }
    else
    {
        currAppDemoInfoPtr = harrier_forcePortMapPtr;
        numEntries = *force_actualNum_harrier_port_modePtr;
    }

    cpssOsMemSet(cpssApi_harrier_defaultMap,0,sizeof(cpssApi_harrier_defaultMap));

    currCpssInfoPtr    = &cpssApi_harrier_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_harrier_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_harrier_defaultMap is FULL, maxPhy port ", rc);
                return rc;
            }

            interfaceNum = currAppDemoInfoPtr->globalDmaNumber[jj];


            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->portNumberArr[jj];
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = interfaceNum;
            currCpssInfoPtr->txqPortNumber      = currAppDemoInfoPtr->numberOfQueuesPerPort;
            currCpssInfoPtr->reservePreemptiveChannel = currAppDemoInfoPtr->reservePreemptiveChannel;

            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && currCpssInfoPtr->physicalPortNumber != CPU_PORT)
                {
                    /* skip this SDMA - the GM currently supports single SDMA */
                    currCpssInfoPtr--;
                    continue;
                }

                numCpuSdmas++;
            }

            actualNum_cpssApi_harrier_defaultMap++;
        }
    }

    return GT_OK;
}

/**
* @internal harrier_getBoardInfo function
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
static GT_STATUS harrier_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(firstDev, pciInfo);
    if (rc != GT_OK)
    {
        if (firstDev != GT_FALSE)
        {
            osPrintf("Could not recognize any device on PCI bus!\n");
        }
        return GT_NOT_FOUND;
    }

    /*copy info for later use*/
    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)].pciInfo = *pciInfo;

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)].hwInfo = *hwInfoPtr;

    return rc;
}

/**
* @internal harrier_getBoardInfoSimple function
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
static GT_STATUS harrier_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo,
    IN GT_BOOL  firstDev
)
{
    GT_STATUS   rc;

    rc = harrier_getBoardInfo(firstDev, hwInfoPtr, pciInfo);
    return rc;
}

#ifdef ASIC_SIMULATION
extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
#endif

/**
* @internal harrier_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of harrier board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID harrier_boardTypePrint
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
    appDemoInAsimEnv = cpssPpWmDeviceInAsimEnvGet() ? GT_TRUE : GT_FALSE;

    if(appDemoInAsimEnv)
    {
        environment = "HW (ASIM + WM simulation)";
    }
    else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif

    if(appDemoInAsimEnv)
    {
        appDemoRunningInAsimEnv();
    }

    if(appDemoPpConfigDevAmount > 1)
    {
        cpssOsPrintf("%s Board Type: %s [%s] of [%d] devices \n", devName , boardName , environment, appDemoPpConfigDevAmount);
    }
    else
    {
        cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
    }
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

    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->maxNumOfPhyPortsToUse = 0;/* single mode of 128 physical ports ,
                                            so supports value 128 or 0 */

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98DXC720_CNS ,/* dummy to represent 'harrier' family (one of falcon values) */
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
    IN GT_U32               devIndex,
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

    appDemoCpssCurrentDevIndex = devIndex;

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

/**
* @internal harrier_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS harrier_initPortMappingStage
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType,
    IN GT_BOOL                  configure8bitQOffset
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen,i;
    CPSS_DXCH_PORT_MAP_STC *mapUpdatedArrPtr;
    GT_U32 mapArrUpdatedLen;

    switch(devType)
    {
        case CPSS_HARRIER_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_harrier_defaultMap, mapArrPtr, mapArrLen);
            mapArrLen = actualNum_cpssApi_harrier_defaultMap;
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }


    if(GT_FALSE==configure8bitQOffset)
    {
        for(i=0;i<mapArrLen;i++)
        {
            if(mapArrPtr[i].txqPortNumber>16)
            {
                configure8bitQOffset = GT_TRUE;
                break;
            }
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
    if(GT_TRUE ==configure8bitQOffset)
    {
        rc = cpssDxChPortTxQueueOffsetWidthSet(CAST_SW_DEVNUM(devNum),CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_8_BIT_ENT);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxQueueOffsetWidthSet", rc);
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

extern GT_STATUS appDemoBc2PortListInitForceShowOnlySet(IN GT_BOOL force);

/**
* @internal harrier_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS harrier_PortModeConfiguration
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    PortInitList_STC *portInitListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    UNUSED_PARAM_MAC(devType);

    if(appDemoPpConfigDevAmount == 2) /* hold priority over 'harrier_force_PortsInitListPtr' */
    {
        if(devIndex == SYSTEM_DEV_NUM_MAC(0))
        {
            /* use speed table with the needed speeds on the connection ports */
            portInitListPtr = harrier_dev_0_b2b_portInitlist;
        }
        else
        {
            /* use speed table with the needed speeds on the connection ports */
            portInitListPtr = aldrin3m_dev_1_b2b_portInitlist;
        }
    }
    else
    if(harrier_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = harrier_force_PortsInitListPtr;
    }
    else
    {
        portInitListPtr = aldrin3M_portInitlist;
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        /* force appDemoBc2PortListInit(...) to just print info , without any config */
        appDemoBc2PortListInitForceShowOnlySet(GT_TRUE);
    }

    rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum),portInitListPtr,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* indication for function appDemoBc2PortInterfaceInit(...) to
       not call appDemoBc2PortListInit(...) */
    appDemo_PortsInitList_already_done = 1;

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        appDemoBc2PortListInitForceShowOnlySet(GT_FALSE);
    }

    return GT_OK;
}

/**
* @internal harrier_appDemoDbUpdate function
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
static GT_STATUS harrier_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_HARRIER_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

    return GT_OK;
}

/**
* @internal utfDmaWindowGet function
* @endinternal
*
* @brief   The function support DMA multi-windows (each device may hold different window) .
*
* @param[in] devNum                - the devNum to get the windowId
* @param[out] windowIdPtr          - (pointer to) the windowId
*
* @retval GT_OK                    - on success .
* @retval GT_FAIL                  - on failure .
*
* @note none
*
*/
static  GT_STATUS utfDmaWindowGet(
    IN GT_SW_DEV_NUM   devNum,
    OUT GT_U32         *windowIdPtr
)
{
    GT_U32 devIndex;

    for(devIndex = 0 ; devIndex < appDemoPpConfigDevAmount; devIndex++)
    {
        if(appDemoPpConfigList[devIndex].devNum == CAST_SW_DEVNUM(devNum) &&
           appDemoPpConfigList[devIndex].valid)
        {
            *windowIdPtr = appDemoPpConfigList[devIndex].dmaWindow;
            return GT_OK;
        }
    }

    *windowIdPtr = 0;
    return GT_NOT_FOUND;
}


/**
* @internal harrier_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS harrier_initPhase1AndPhase2
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
    GT_HW_DEV_NUM           hwDevNum,hsHwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };
    GT_U32                 tmpData;
    GT_BOOL                configure8bitQOffset = GT_FALSE;

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));


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

    /* check if the device did HW reset (soft/hard) */
    appDemoDxChCheckIsHwDidHwReset(devNum);

    /* update the appDemo DB */
    rc = harrier_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    if(isSmmuNeededFor2Devices)
    {
        /* next needed for SMMU system , so the DMA need to be allocated per device */
        /* this will set the device on specific 'window' .
           appDemo_configure_dma_per_devNum()
           internal_prvNoKmDrv_configure_dma_per_devNum
                extDrvGetDmaBase() --> will get 'base' of this DMA window
                (that allocated by shrDmaByWindowInit)
        */
        rc = osGlobalDbDmaActiveWindowByPcieParamsSet(
            appDemoPpConfigList[devIndex].pciInfo.pciBusNum >> 8,/*pciDomain (see logic in prvExtDrvSysfsFindDev)*/
            appDemoPpConfigList[devIndex].pciInfo.pciBusNum & 0xFF,/*pciBus (see logic in prvExtDrvSysfsFindDev)*/
            appDemoPpConfigList[devIndex].pciInfo.pciIdSel,/*pciDev*/
            appDemoPpConfigList[devIndex].pciInfo.funcNo,/*pciFunc*/
            &appDemoPpConfigList[devIndex].dmaWindow/*OUT - get the window*/);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("osGlobalDbDmaActiveWindowByPcieParamsSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* the Harrier got it's allocation before the cpssInitSystem (with higher) */
        /* but the Aldrin3M did not get this initialization                        */
        if(appDemoPpConfigList[devIndex].dmaWindow != 0)
        {
            /* NOTE: next code taken from : shrMemSharedBufferInit
                that do it for window 0 , before cpssInitSystem
                shrDmaByWindowInit will call :
                extDrvGetDmaBase()
                    check_dma()
                        try_map_mvDmaDrv()
                            allocate DMA from the mvDmaDrv.ko (with the SMMU)
                */
            shrDmaByWindowInit(GT_TRUE,
                appDemoPpConfigList[devIndex].dmaWindow);
        }

        utfDmaWindowGet_func = utfDmaWindowGet;
    }

    osPrintf("devNum[%d] : set DMA windows (MG , oATU) \n",devNum);

    rc = appDemo_configure_dma_per_devNum(CAST_SW_DEVNUM(devNum) , GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(isSmmuNeededFor2Devices)
    {
        rc = osGlobalDbDmaActiveWindowSet(0);/* restore to window 0 (just a restore value) */
        CPSS_ENABLER_DBG_TRACE_RC_MAC("osGlobalDbDmaActiveWindowSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    waIndex = 0;
    while(falcon_WaList[waIndex] != CPSS_DXCH_IMPLEMENT_WA_LAST_E)
    {
        waIndex++;
    }

    if(waIndex)
    {
        rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),waIndex,falcon_WaList, NULL);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(boardRevId == 2)
    {
        harrier_force_PortsInitListPtr = harrier_portInitlistRpp;
        harrier_forcePortMapPtr = harrier_max_rpp_port_mode;
        force_actualNum_harrier_port_modePtr = &actualNum_harrier_max_rpp_port_mode;
    }
    else if(boardRevId == 3)
    {
        rc = appDemoDbEntryGet("maxQmode", &tmpData);
        harrier_force_PortsInitListPtr = harrier_portInitlistMaxQ;
        if(rc == GT_OK)
        {
            switch(tmpData)
            {
                case 1:
                 harrier_forcePortMapPtr = harrier_max_queue_port_mode_no_cpu;
                 force_actualNum_harrier_port_modePtr = &actualNum_harrier_max_queue_port_mode_no_cpu;
                 break;
               case 2:
                 harrier_forcePortMapPtr = harrier_max_queue_port_mode_distributed;
                 force_actualNum_harrier_port_modePtr = &actualNum_harrier_max_queue_port_mode_distributed;
                 break;
               default:
                 break;
            }
        }

        if(NULL == harrier_forcePortMapPtr)
        {
            harrier_forcePortMapPtr = harrier_max_queue_port_mode;
            force_actualNum_harrier_port_modePtr = &actualNum_harrier_max_queue_port_mode;
        }
        configure8bitQOffset = GT_TRUE;
    }

    /*************************************************/
    /* fill once the DB of 'port mapping' to be used */
    /*************************************************/
    rc = fillDbForCpssPortMappingInfo(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("fillDbForCpssPortMappingInfo", rc);
    if (rc != GT_OK)
        return rc;

    rc = harrier_initPortMappingStage(devNum,devType,configure8bitQOffset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devIndex , devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /*in case of Hitless Startup hwDevNum should be set by application */
    if (appDemoDbEntryGet("hitless_startup_HwDevNum",&hsHwDevNum) == GT_OK)
    {
        hwDevNum = (devNum + hsHwDevNum) & 0x3FF;
    }

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal harrier_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS harrier_getNeededLibs(
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

/**
* @internal harrier_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS harrier_initPpLogicalInit
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
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

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

#define HARRIER_LED_UNITS_MAX_NUM_CNS 3
#define HARRIER_DB_LED_INDICATIONS_NUM_CNS 20

/* Harrier DB board has following LEDs:
   QSFP cages have 4 LEDs per cage - one per SERDES
   SFP cages have 1 LED per cage
   First two LED Units should have 8 indications. Last one should have 4 indications.
   Use Class 2 indication in mode "Link and Activity" */
static const APP_DEMO_LED_UNIT_CONFIG_STC harrier_DB_led_indications[HARRIER_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 71,    GT_FALSE}, /* 8 LED ports (LED Unit 0 - DP0)     */
    {64, 71,    GT_FALSE}, /* 8 LED ports (LED Unit 1 - DP1)     */
    {64, 67,    GT_FALSE}  /* 4 LED ports (LED Unit 2 - DP2)     */
};

/* MAC to LED Position mapping */
static const APP_DEMO_LED_PORT_MAC_POSITION_STC harrier_DB_led_port_map[HARRIER_DB_LED_INDICATIONS_NUM_CNS] =
{
    { 2, 6}, { 4, 5}, { 0, 7}, { 6, 4}, {12, 1}, {10, 2}, {14, 0}, { 8, 3},
    {18, 6}, {20, 5}, {16, 7}, {22, 4}, {28, 1}, {26, 2}, {30, 0}, {24, 3},
    {32, 3}, {34, 2}, {36, 1}, {38, 0}
};

/**
* @internal harrierLedPortPositionGet
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] boardRevId            - board revision ID
* @param[in] portMacNum            - current MAC to check
*
* @retval GT_U32                   - the LED ports position for the current MAC port
*
*/
static GT_U32 harrierLedPortPositionGet
(
    IN  GT_U8  boardRevId,
    IN  GT_U32 portMacNum
)
{
    GT_U32 ledPosition = APPDEMO_BAD_VALUE;
    const APP_DEMO_LED_PORT_MAC_POSITION_STC * portsMapArrPtr;
    GT_U32 portsMapArrLength;
    GT_U32 ii;

    GT_UNUSED_PARAM(boardRevId);

    portsMapArrPtr = harrier_DB_led_port_map;
    portsMapArrLength = HARRIER_DB_LED_INDICATIONS_NUM_CNS;

    for (ii = 0; ii < portsMapArrLength; ii++)
    {
        if (portsMapArrPtr[ii].macPortNumber == portMacNum)
        {
            ledPosition = portsMapArrPtr[ii].ledPortPosition;
            break;
        }
    }

    return ledPosition;
}

/**
* @internal harrier_LedInit function
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
static GT_STATUS harrier_LedInit
(
    IN  GT_U8    devNum,
    IN  GT_U8    boardRevId
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
    const APP_DEMO_LED_UNIT_CONFIG_STC * ledStreamIndication;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    ledConfig.ledOrganize                       = CPSS_LED_ORDER_MODE_BY_CLASS_E;
    ledConfig.sip6LedConfig.ledClockFrequency   = 1000;
    ledConfig.pulseStretch                      = CPSS_LED_PULSE_STRETCH_1_E;

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    ledStreamIndication = harrier_DB_led_indications;

    for(ledUnit = 0; ledUnit < HARRIER_LED_UNITS_MAX_NUM_CNS; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;
        /* End of LED stream location: LED ports 0..7 */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;
        /* LED unit may be bypassed in the LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = ledStreamIndication[ledUnit].bypassLedUnit;
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    classNum = 2;
    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for SIP_5/6 */
    ledClassManip.blinkEnable             = GT_TRUE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;

    /* activate disableOnLinkDown as part of "Link and Activity" indication */
    ledClassManip.disableOnLinkDown       = GT_TRUE;

    rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                               CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Hawk */
                                               ,classNum ,&ledClassManip);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
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

        position = harrierLedPortPositionGet(boardRevId, portMap.interfaceNum);

        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal harrier_appDemoInitSequence_part1 function
* @endinternal
*
* @brief   Part 1 : This is the 'simple' board initialization function for the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS harrier_appDemoInitSequence_part1
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;

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

    rc = harrier_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;




    return GT_OK;
}

/**
* @internal harrier_appDemoInitSequence_part2 function
* @endinternal
*
* @brief   part2 : This is the 'simple' board initialization function for the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS harrier_appDemoInitSequence_part2
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     sysConfigParams;
    GT_BOOL supportSystemReset = GT_TRUE;
    GT_U32      tmpData;


    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    devType = appDemoPpConfigList[devIndex].deviceId;

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
        rc = prvAppDemoHarrierSerdesConfigSet(CAST_SW_DEVNUM(devNum),boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoHarrierSerdesConfigSet", rc);
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
    rc = harrier_PortModeConfiguration(devIndex,devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = harrier_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = harrier_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_initPpLogicalInit", rc);
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

    rc = harrier_LedInit(CAST_SW_DEVNUM(devNum), boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_LedInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    #endif /*GM_USED*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        CPSS_TBD_BOOKMARK_FALCON_EMULATOR
        /* until we make it happen ... do not try it on emulator ! */
        supportSystemReset = GT_FALSE;
    }

    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal harrier_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS harrier_EventHandlerInit
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32 value;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_U8                   devIndex,devNum;

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
        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E ||
            system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)
        {
            PRINT_SKIP_DUE_TO_DB_FLAG("appDemoEventRequestDrvnModeInit ","system recovery in HA/HS process");
            return GT_OK;
        }

        if(isUnderSecondDeviceCatchup == GT_TRUE)
        {
            START_LOOP_ALL_DEVICES(devNum)
            {
                appDemoDeviceEventRequestDrvnModeInit(devIndex);
                devNum = devNum;/* avoid gcc warning*/
            }
            END_LOOP_ALL_DEVICES
        }
        else
        {
            rc = appDemoEventRequestDrvnModeInit();
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
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
static GT_STATUS   appDemoHarrierMainUtForbidenTests(void)
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
         TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPortGroupTablePrint       )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {
         SUITE_NAME(cpssDxChHwInitDeviceMatrix )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_too_long_and_PASS [] =
    {
         TEST_NAME(cpssDxChDiagAllMemTest)                  /* : takes [313] sec */
        ,TEST_NAME(cpssDxChLatencyMonitoringPortCfgSet)     /* : takes [219] sec */
        ,TEST_NAME(cpssDxChPtpTaiTodStepSet)                /* : takes [119] sec */
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlSet)  /* : takes [422] sec*/
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlGet)  /* : takes [187] sec*/

        ,TEST_NAME(prvCpssDxChTestFdbHash_8K)               /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_16K)              /* : takes [92] sec */
        /*,TEST_NAME(prvCpssDxChTestFdbHash_32K) allow one of those to run */  /* : takes [92] sec */
        ,TEST_NAME(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)/* : takes [167] sec*/
        ,TEST_NAME(cpssDxChExactMatchRulesDump)             /* : takes [209] sec */
        ,TEST_NAME(cpssDxChPortLoopbackPktTypeToCpuSet)     /* : takes [295] sec */

        /* enhUT : */
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes      ) /*takes [215] sec*/
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck  ) /*takes [371] sec*/
        ,TEST_NAME(prvTgfPclMetadataPacketType         ) /*takes [578] sec*/
        ,TEST_NAME(tgfExactMatchManagerTtiPclFullPathHaCheckSingleEntry) /* take [170] seconds  */
        ,TEST_NAME(tgfExactMatchManagerHaFullCapacity)                   /* take [1014] seconds */
        ,TEST_NAME(tgfExactMatchManagerDeleteScanFullCapacity)           /* take [754] seconds  */
        ,TEST_NAME(tgfExactMatchManagerAgingScanFullCapacity)            /* take [914] seconds  */
        ,TEST_NAME(tgfTrunkCascadeTrunk)                                 /* takes [264] sec  */
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)                      /* takes [271] sec  */
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort) /* takes [1808] sec */
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)                       /* takes [446] sec  */
        ,TEST_NAME(prvTgfPclFwsMove)                                     /* takes [320] sec  */
        ,TEST_NAME(prvTgfPclFwsMove_virtTcam_80B)                        /* takes [486] sec  */

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
        ,TEST_NAME(/*.tgfIp*/prvTgfIpLpmHaCheck      ) /* crash */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_too_long_and_fail [] =
    {
         TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithoutIngressHdr) /* failed [20] times ,takes [618] sec  */
        ,TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithIngressHdr)    /* failed [20] times ,takes [534] sec  */

        ,TEST_NAME(tgfPtpAddCorrectionTimeTest)
        ,TEST_NAME(prvTgfGenInterruptCoalescingForLinkChangeEvent)

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_Stuck [] =
    {
        {NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_Stuck [] =
    {
        TEST_NAME(/*tgfEvent.*/prvTgfGenInterruptCoalescing)/* critical ERROR .. cause 'fatal error' */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES enhancedUt_forbidenSuites_Emulator_Killer [] =
    {
        {NULL}/* must be last */
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


    /* tests that defined forbidden by Hawk */

#ifdef IMPL_TGF
    appDemoForbidenTestsAdd (mainUt_forbidenTests_FatalError);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH_FAIL);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_FatalError);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_too_long_and_fail);

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("cpssDxChExactMatchManager","*","","");
        utfAddPreSkippedRule("cpssDxChVirtualTcam","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");
        /*utfAddPreSkippedRule("tgfExactMatchManager","*","","");*/

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
* @internal harrier_localUtfInit function
* @endinternal
*
*/
static GT_STATUS harrier_localUtfInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */

    if(isUnderSecondDeviceCatchup == GT_FALSE)
    {
        /* Initialize unit tests for CPSS */
        rc = utfPreInitPhase();
        if (rc != GT_OK)
        {
            utfPostInitPhase(rc);
            return rc;
        }
    }

    START_LOOP_ALL_DEVICES(devNum)
    {
        rc = utfInit(CAST_SW_DEVNUM(devNum));
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
            utfPostInitPhase(rc);
            return rc;
        }
    }
    END_LOOP_ALL_DEVICES

    if(isUnderSecondDeviceCatchup == GT_FALSE)
    {
        utfPostInitPhase(rc);
    }
#endif /* INCLUDE_UTF */

    if(isUnderSecondDeviceCatchup == GT_FALSE)
    {
        appDemoHarrierMainUtForbidenTests();
    }

    return rc;
}

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum, GT_BOOL tcamDaemonEnable);

typedef struct{
    GT_U32  portNum;
}LOCAL_CASCADE_INFO_STC;

typedef struct{
    LOCAL_CASCADE_INFO_STC          devPortInfo[2];
    CPSS_CSCD_PORT_TYPE_ENT         cascadePortType;
    CPSS_CSCD_LINK_TYPE_ENT         cascadeLinkType;
}LOCAL_CASCADE_CONNECTED_PORTS_INFO_STC;
static LOCAL_CASCADE_CONNECTED_PORTS_INFO_STC harrier_b2b_connectedPortsInfo[] =
{
    {
        {
        /*port on dev 0*/{DEV0_CONNECTION_PORT_200G/*portNum*/},
        /*port on dev 1*/{DEV1_CONNECTION_PORT_200G/*portNum*/}
        }
        , CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E/*cascadePortType*/
        , CPSS_CSCD_LINK_TYPE_PORT_E/*cascadeLinkType*/
    }
    /* must be last */
    ,{{{GT_NA},{GT_NA}}, GT_NA,GT_NA}
};
static GT_U32   dev0_connection_port = GT_NA;
static GT_U32   dev1_connection_port = GT_NA;
#define TRUNK_INDICATION (1<<31)
/* allow to set pre-cpssInitSystem the connection port number of device 0 (harrier)*/
/* if not using the default , the user must set the speeds of the connection ports */
GT_STATUS appDemo_harrier_dev0_connection_port(GT_U32   portNum , GT_U32    trunkId)
{
    if(trunkId)
    {
        cpssOsPrintf("Set device 0 connection trunk [%d]",trunkId);
        dev0_connection_port = trunkId | TRUNK_INDICATION;
    }
    else
    {
        cpssOsPrintf("Set device 0 connection portNum [%d]",portNum);
        dev0_connection_port = portNum;
    }
    return GT_OK;
}
/* allow to set pre-cpssInitSystem the connection port number of device 1 (aldrin3m)*/
GT_STATUS appDemo_aldrin3m_dev1_connection_port(GT_U32   portNum , GT_U32    trunkId)
{
    if(trunkId)
    {
        cpssOsPrintf("Set device 1 connection trunk [%d]",trunkId);
        dev1_connection_port = trunkId | TRUNK_INDICATION;
    }
    else
    {
        cpssOsPrintf("Set device 1 connection portNum [%d]",portNum);
        dev1_connection_port = portNum;
    }

    return GT_OK;
}
/**
* @internal harrier_b2b_cascadeInit function
* @endinternal
*
*/
/* do not 'static'
   to allow to be called even if was not called during cpssInitSystem */
GT_STATUS harrier_b2b_cascadeInit(GT_VOID)
{
    LOCAL_CASCADE_CONNECTED_PORTS_INFO_STC  *currConnectedPortsInfoPtr;
    /* assuming that the connections are :
        dev0_port24  = dev1_port24
        dev0_port26  = dev1_port26
        dev0_port28  = dev1_port28
        dev0_port30  = dev1_port30
    */
    GT_STATUS   rc;
    GT_U8   devNum;
    GT_U32  devIndex,portNum;
    GT_U32  index;
    GT_HW_DEV_NUM   hwDevNum[2];
    GT_HW_DEV_NUM   targetHwDevNum;
    CPSS_CSCD_LINK_TYPE_STC     cpssCascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  currentSrcPortTrunkHashEn;
    GT_BOOL      egressAttributesLocallyEn;

    START_LOOP_ALL_DEVICES(devNum)
    {
        index = devIndex - SYSTEM_DEV_NUM_MAC(0);

        /* set device map table to be in 'per target device' mode .
        to match our next cpssDxChCscdDevMapTableSet(...) settings */
        rc = cpssDxChCscdDevMapLookupModeSet(devNum,CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdDevMapLookupModeSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum[index]);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgHwDevNumGet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    END_LOOP_ALL_DEVICES

    for(currConnectedPortsInfoPtr = &harrier_b2b_connectedPortsInfo[0] ;
        currConnectedPortsInfoPtr->cascadePortType != GT_NA ;
        currConnectedPortsInfoPtr++)
    {
        START_LOOP_ALL_DEVICES(devNum)
        {
            /* devNum,devIndex we get from START_LOOP_ALL_DEVICES */
            index = devIndex - SYSTEM_DEV_NUM_MAC(0);
            portNum = currConnectedPortsInfoPtr->devPortInfo[index].portNum;

            if(currConnectedPortsInfoPtr->cascadePortType == CPSS_CSCD_PORT_NETWORK_E)
            {
                /* no cascade settings needed */
                continue;
            }

            rc = cpssDxChCscdPortTypeSet(devNum, portNum,CPSS_PORT_DIRECTION_BOTH_E,
                currConnectedPortsInfoPtr->cascadePortType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdPortTypeSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* trust DSA info */
            rc = cpssDxChCosTrustDsaTagQosModeSet(devNum, portNum, GT_TRUE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCosTrustDsaTagQosModeSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* disable policy */
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_FALSE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPclPortIngressPolicyEnable", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Cascade port's QoS profile default settings */
            {
                CPSS_QOS_ENTRY_STC          portQosCfg;
                portQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                portQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                portQosCfg.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                portQosCfg.qosProfileId     = 64; /*#define GT_CORE_CHEETAH_TC_FIRST_ENTRY 64*/
                rc = cpssDxChCosPortQosConfigSet(devNum, portNum, &portQosCfg);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCosPortQosConfigSet", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* disable "send to CPU AU" from this port */
            rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, GT_FALSE);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgFdbNaToCpuPerPortSet", rc);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* for the DSA compensation */
            /* set short IPG : 8 bytes (instead of 12) */
            rc  = cpssDxChPortIpgSet(devNum, portNum , 8);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortIpgSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* set short preamble to 4 (instead of 8) */
            rc = cpssDxChPortPreambleLengthSet(devNum, portNum ,
                                        CPSS_PORT_DIRECTION_BOTH_E, 4);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPreambleLengthSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(currConnectedPortsInfoPtr->cascadeLinkType != CPSS_CSCD_LINK_TYPE_PORT_E)
            {
                /* trunk was not implemented */
                continue;
            }

            /* use the hwDevNum of the 'other device' */
            targetHwDevNum = hwDevNum[1 - index];

            rc = cpssDxChCscdDevMapTableGet(devNum,targetHwDevNum, 0, 0, 0, &cpssCascadeLink,
                                            &currentSrcPortTrunkHashEn, &egressAttributesLocallyEn);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdDevMapTableGet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            cpssCascadeLink.linkNum  = portNum;
            cpssCascadeLink.linkType = currConnectedPortsInfoPtr->cascadeLinkType;

            rc = cpssDxChCscdDevMapTableSet(devNum,targetHwDevNum, 0, 0, 0, &cpssCascadeLink,
                                            currentSrcPortTrunkHashEn, egressAttributesLocallyEn);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdDevMapTableSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

        }
        END_LOOP_ALL_DEVICES
    }/* loop on currConnectedPortsInfoPtr */

    return GT_OK;
}

/**
* @internal gtDbHarrierBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Harrier device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbHarrierBoardReg_SimpleInit
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
    GT_BOOL     firstDev;
    GT_U16 devId;
    char*       namePtr = NULL;
    GT_U32      index;
    GT_U32      connection_port;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    if(isUnderSecondDeviceCatchup == GT_TRUE)
    {
        goto secondDeviceCatchUp_lbl;
    }

    last_boardRevId = boardRevId;

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

    appDemoPpConfigDevAmount = 0;

    firstDev = GT_TRUE;

    /* set only limited number of devices that the PCIe scan need to go over */
    /* this to reduce time on ASIM environment , and should not harm other platforms */
    limitedDevicesListPtr = harrierLimitedDevicesList;

    while(1)
    {
        /* this function finds a Prestera devices on PCI bus */
        rc = harrier_getBoardInfoSimple(
                &hwInfo[0],
                &pciInfo,
                firstDev);
        if (rc != GT_OK)
        {
            break;/* no more devices */
        }

        appDemoPpConfigDevAmount++;
        firstDev = GT_FALSE;
    }

    if(appDemoPpConfigDevAmount == 2)
    {
        namePtr = "2 devices";

        isSmmuNeededFor2Devices = GT_TRUE;/* The CPU working with SMMU */

        /* lets give priority to 106XXS-Integrated-Switch (Harrier-MCM) that connected to the CPU to be devNum = 0 */
        /* 9010 Aldrin3M                                */
        /* 2100 106XXS-Integrated-Switch : Harrier-MCM  */
        if((appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].pciInfo.pciDevVendorId.devId & 0xFF00) == 0x2100 &&
           (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo.pciDevVendorId.devId & 0xFF00) == 0x9000)
        {
            static APP_DEMO_PP_CONFIG tmpInfo;
            /* swap index 0 and 1 */
            tmpInfo = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)];
            appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)] = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)];
            appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)] = tmpInfo;

            namePtr = "106XXS-Integrated-Switch and Aldrin3M";
        }

        harrierBoardType = HARRIER_BOARD_TYPE_B2B_ALDRIN_106xxS;

        harrier_forcePortMapPtr = harrier_port_mode;
        force_actualNum_harrier_port_modePtr = &actualNum_harrier_port_mode;
        harrier_force_PortsInitListPtr = aldrin3m_dev_1_b2b_portInitlist;    /* will be overwritten in harrier_PortModeConfiguration() */

        devId = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo.pciDevVendorId.devId;
        switch(devId & 0xFF00)
        {
            case    0x2100:
                cpssOsPrintf("first Device [0x%4.4x] is : 106XXS-Integrated-Switch \n" ,
                    devId);
                break;
            default:
                cpssOsPrintf("first Device [0x%4.4x] \n" ,
                    devId);
                break;
        }

        devId = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)].pciInfo.pciDevVendorId.devId;
        switch(devId & 0xFF00)
        {
            case    0x9000:
                cpssOsPrintf("second Device [0x%4.4x] is : Aldrin3M \n" ,
                    devId);
                break;
            default:
                cpssOsPrintf("second Device [0x%4.4x] \n" ,
                    devId);
                break;
        }
    }
    else
    if(appDemoPpConfigDevAmount == 0)
    {
        /* failed to find device(s) */
        rc = GT_NOT_FOUND;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_getBoardInfoSimple: found no device", rc);
        return rc;
    }
    else /* single device */
    {
        devId = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo.pciDevVendorId.devId;
        switch(devId & 0xFF00)
        {
            case    0x2100:
                cpssOsPrintf("Device [0x%4.4x] is : 106XXS-Integrated-Switch \n" ,
                    devId);
                namePtr = "106XXS-Integrated-Switch";

                harrier_forcePortMapPtr = harrier_port_mode;
                force_actualNum_harrier_port_modePtr = &actualNum_harrier_port_mode;
                harrier_force_PortsInitListPtr = harrier_portInitlist;

                harrierBoardType = HARRIER_BOARD_TYPE_DB_106xxS;
                break;
            case    0x9000:
                cpssOsPrintf("Device [0x%4.4x] is : Aldrin3M \n" ,
                    devId);
                namePtr = "Aldrin3M";
                harrierBoardType = HARRIER_BOARD_TYPE_DB_ALDRIN_3M;
                break;
            default:
                cpssOsPrintf("Device [0x%4.4x] (not Aldrin3M and not 106XXS-Integrated-Switch) \n" ,
                    devId);
                namePtr = "unknown device (not Aldrin3M and not 106XXS-Integrated-Switch)";
                break;
        }
    }

    if(namePtr)
    {
        /* print board type before phase 1,2 */

        harrier_boardTypePrint("DB" /*boardName*/, namePtr /*devName*/);
    }

    secondDeviceCatchUp_lbl:


    for (devIndex = SYSTEM_DEV_NUM_MAC(0);
          (devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)); devIndex++)
    {
        devNum =   devIndex;

        if(harrier_force_single_device == 2)
        {
            if(devIndex == SYSTEM_DEV_NUM_MAC(0))
            {
                /* we ignore the first device */
                continue;
            }

            /* copy info from second to first */
            appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)] = appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(1)];

            /* we are at the second device , but we need to treat it as first */
            devIndex = SYSTEM_DEV_NUM_MAC(0);
            devNum =   devIndex;

            appDemoPpConfigDevAmount = 1;
        }

        SYSTEM_SKIP_NON_ACTIVE_DEV((GT_U8)devIndex);

        if(harrier_force_single_device)
        {
            /* print this info before phase 1,2 */
            if(harrier_force_single_device == 1)
            {
                /* we are done initializing the single device */
                osPrintf("NOTE: use 'first device' (out of 2) for the init (ignore the second one) \n");
            }
            else  /*harrier_force_single_device == 2*/
            {
                osPrintf("NOTE: use 'second device' (out of 2) for the init (ignore the first one) \n");
            }
        }

        rc = harrier_appDemoInitSequence_part1(boardRevId, devIndex, devNum, &appDemoPpConfigList[devIndex].hwInfo);
        if (rc != GT_OK)
            return rc;

        if(harrier_force_single_device)
        {
            appDemoPpConfigDevAmount = 1;
            break;
        }
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    START_LOOP_ALL_DEVICES(devNum)
    {
        rc = harrier_appDemoInitSequence_part2(boardRevId, devIndex , devNum);
        if (rc != GT_OK)
            return rc;
    }
    END_LOOP_ALL_DEVICES

    if(appDemoPpConfigDevAmount == 2)
    {
        /* auto learn is when value == 0 and rc == GT_OK :
           rc = appDemoDbEntryGet("ctrlMacLearn", &value)
        */
        /* ignore NA if running in auto-learn mode */
        rc = appDemoIgnoreAutoLearnNaMsgSet(1);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoIgnoreAutoLearnNaMsgSet", rc);
        if (rc != GT_OK)
            return rc;

        /* in case of controlled learning each device learn on it's local ports only */
        rc = appDemoTreatNaMsgOnConnectionPortForRemoteDeviceSet(1);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTreatNaMsgOnConnectionPortForRemoteDeviceSet", rc);
        if (rc != GT_OK)
            return rc;

        START_LOOP_ALL_DEVICES(devNum)
        {
            index = devIndex - SYSTEM_DEV_NUM_MAC(0);

            if(index == 0)
            {
                connection_port = dev0_connection_port == GT_NA ?
                          harrier_b2b_connectedPortsInfo[0].devPortInfo[index].portNum :
                          dev0_connection_port;
            }
            else
            {
                connection_port = dev1_connection_port == GT_NA ?
                          harrier_b2b_connectedPortsInfo[0].devPortInfo[index].portNum :
                          dev1_connection_port;
            }

            if(TRUNK_INDICATION & connection_port)
            {
                appDemoPpConfigList[devIndex].cscdPortsArr[0].trunkId =  (GT_TRUNK_ID)(connection_port - TRUNK_INDICATION);
            }
            else
            {
                appDemoPpConfigList[devIndex].cscdPortsArr[0].portNum =  (GT_U8)connection_port;
            }

            if(TRUNK_INDICATION & connection_port)
            {
                cpssOsPrintf("NOTE: the USER must create the trunk[%d] on devNum[%d] with the 2 100G ports or the 4 50G ports \n",
                    connection_port - TRUNK_INDICATION,
                    devNum);
            }

            if(appDemoSysConfig.forceAutoLearn == GT_FALSE)
            {
                if((TRUNK_INDICATION & connection_port) == 0)
                {
                    /* Disable NA from the connection port - on controlled learning
                       Since the CPU need single NA from the original dev,port to set the
                       other device too .
                       see logic in prvDxChBrgFdbPortGroupMacEntrySet(...)
                    */
                    rc = cpssDxChBrgFdbNaToCpuPerPortSet(CAST_SW_DEVNUM(devNum),connection_port,GT_FALSE);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgFdbNaToCpuPerPortSet", rc);
                    if (rc != GT_OK)
                        return rc;
                }
                else
                {
                    cpssOsPrintf("NOTE: the USER must call for the connection trunk[%d] ports : cpssDxChBrgFdbNaToCpuPerPortSet(%d,port,GT_FALSE)\n",
                        connection_port - TRUNK_INDICATION,
                        devNum);
                }
            }
        }
        END_LOOP_ALL_DEVICES

        /* gather info that for AC5 for example done in several places:
            1. configBoardAfterPhase2
            2. xcat3CascadePortConfig
            3. prvCscdPortsInit
        */
        if(harrier_b2b_DSA)
        {
            rc = harrier_b2b_cascadeInit();
            CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_b2b_cascadeInit", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* init the event handlers */
    rc = harrier_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        START_LOOP_ALL_DEVICES(devNum)
        {
            /* Data Integrity module initialization. It should be done after events init. */
            rc = appDemoDataIntegrityInit(devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
        END_LOOP_ALL_DEVICES
    }

    rc = harrier_localUtfInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;

    if(GT_TRUE == appDemoInAsimEnv)
    {
        appDemoPostInitAsimEnv();
    }

    if(isUnderSecondDeviceCatchup == GT_TRUE)
    {
        return GT_OK;
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

    cpssOsPrintf("Time processing the cpssInitSystem (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;


    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal harrier_appDemoPreFirstDevice function
* @endinternal
*
* @brief   indicate that the init will activate only on the first device (Harrier)
*          as this device needs the 'HS' operations after did MI 'supper image' settings
*          that are global in the CPSS and the other device (Aldrin3M) must not
*          be as it is not doing MI 'supper image' settings.
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS harrier_appDemoPreFirstDevice(GT_VOID)
{
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", 0x1);

    return GT_OK;
}


/**
* @internal harrier_appDemoCatchUpSecondDevice function
* @endinternal
*
* @brief   the complementary operations that need to be done after the first device
*          finished the 'HS' , and second device can start its initialization.
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS harrier_CatchUpSecondDevice(GT_VOID)
{
    GT_STATUS rc;

    if(appDemoPpConfigDevAmount != 2)
    {
        rc = GT_NOT_SUPPORTED;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_CatchUpSecondDevice : second device not exists !", rc);
        return rc;
    }

    appDemoSysConfig.appDemoActiveDeviceBmp = 0x2;/* The first device will not be re-configured */
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);
    isUnderSecondDeviceCatchup = GT_TRUE;


    rc = gtDbHarrierBoardReg_SimpleInit(last_boardRevId);

    appDemoSysConfig.appDemoActiveDeviceBmp = 0x0;/* reset the indications */
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);
    isUnderSecondDeviceCatchup = GT_FALSE;

    return rc;
}


/**
* @internal harrier_appDemoCatchUpSecondDevice function
* @endinternal
*
* @brief   the complementary operations that need to be done after the first device
*          finished the 'HS' , and second device can start its initialization.
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS harrier_appDemoCatchUpSecondDevice(GT_VOID)
{
    GT_STATUS   rc;

    GT_U32  start_sec  = 0;
    GT_U32  start_nsec = 0;
    GT_U32  end_sec  = 0;
    GT_U32  end_nsec = 0;
    GT_U32  diff_sec;
    GT_U32  diff_nsec;

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    rc = harrier_CatchUpSecondDevice();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("harrier_CatchUpSecondDevice failed ", rc);
        return rc;
    }

    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the harrier_appDemoCatchUpSecondDevice took [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return GT_OK;
}

/**
* @internal gtDbHarrierBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbHarrierBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;

    return GT_OK;
}
/**
* @internal gtDbDxHarrierBoardReg function
* @endinternal
*
* @brief   Registration function for the Hawk board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxHarrierBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbHarrierBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbHarrierBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS internal_onEmulator(GT_BOOL doInitSystem);
static GT_U32 harrier_emulator_deviceId = CPSS_98DXC720_CNS;/* for to use value recognized by the CPSS ! */
/* function to be called before phoenix_onEmulator_prepare to allow other/ZERO the 'phoenix_emulator_deviceId' */
GT_STATUS harrier_emulator_deviceId_set(GT_U32   deviceId)
{
    harrier_emulator_deviceId = deviceId;
    return GT_OK;
}
extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

extern GT_STATUS cnc_onEmulator_extremely_slow_emulator_set(GT_U32   timeToSleep);

extern GT_STATUS debugEmulatorTimeOutSet
(
    GT_U32 timeOutType,
    GT_U32 timeOutValue
);

GT_STATUS harrier_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc =  internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 'NO KN' is doing polling in this interrupt global cause register */
    /* but this polling should not be seen during 'trace'                   */
    appDemoTrace_skipTrace_onAddress(PRV_CPSS_HARRIER_MG0_BASE_ADDRESS_CNS + 0x30 /*0x7F900030*//*address*/,0/*index*/);

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

    if(harrier_emulator_deviceId)/* assign only if not ZERO */
    {
        appDemoDebugDeviceIdSet(0, harrier_emulator_deviceId);
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

    {
        GT_U32 debug_sip6_TO_txq_polling_interval = 10;/*orig 200*/
        GT_U32 debug_sip6_TO_after_txq_drain      = 10;/*orig 500*/
        GT_U32 debug_sip6_TO_after_mac_disabled   = 10;/*orig 500*/

        /* use values that used before Phoenix/Hawk added USX ports */
        debugEmulatorTimeOutSet(1,debug_sip6_TO_txq_polling_interval);
        debugEmulatorTimeOutSet(3,debug_sip6_TO_after_txq_drain);
        debugEmulatorTimeOutSet(4,debug_sip6_TO_after_mac_disabled);
    }

    return GT_OK;
}

GT_STATUS  harrier_clearMappingDb
(
    GT_VOID
)
{
    actualNum_cpssApi_harrier_defaultMap =0;
    harrier_force_PortsInitListPtr = NULL;
    harrier_forcePortMapPtr = NULL;
    force_actualNum_harrier_port_modePtr = NULL;

    return GT_OK;
}

/* Example how to work with trunks as 'Connection trunks' between
    device 0 (Harrier) using trunkId 222 and device 1 (Aldrin3m) trunkId 111

    instead of the default 'Connection ports' :
    device 0 (Harrier) using port 8(+10,12,14) and device 1 (Aldrin3m) port 32(+34,36,38)

    just 'copy paste' into the LUA CLI:

//preinit info to use trunk 222 as connection trunk on device 0
do shell-execute appDemo_harrier_dev0_connection_port    0,111
//preinit info to use trunk 111 as connection trunk on device 0
do shell-execute appDemo_aldrin3m_dev1_connection_port   0,222

//init the system
cpssInitSystem 40,1,0

// you need to see next comments during the cpssInitSystem :
//NOTE: the USER must create the trunk[111] on devNum[0] with the 2 100G ports or the 4 50G ports
//NOTE: the USER must call for the connection trunk[111] ports : cpssDxChBrgFdbNaToCpuPerPortSet(0,port,GT_FALSE)
//NOTE: the USER must create the trunk[222] on devNum[1] with the 2 100G ports or the 4 50G ports
//NOTE: the USER must call for the connection trunk[222] ports : cpssDxChBrgFdbNaToCpuPerPortSet(1,port,GT_FALSE)


//remove the 200G from the connection ports
config
interface range ethernet 0/8,1/32
no speed
exit

//define 50G on the connection ports
interface range ethernet 0/8,10,12,14,1/32,34,36,38
speed 50000 mode KR
no shutdown
exit
//add connection trunk members on device 0
do cpss-api call cpssdxchtrunkmemberadd devNum 0 trunkId 111
8
16
do cpss-api call cpssdxchtrunkmemberadd devNum 0 trunkId 111
10
16
do cpss-api call cpssdxchtrunkmemberadd devNum 0 trunkId 111
12
16
do cpss-api call cpssdxchtrunkmemberadd devNum 0 trunkId 111
14
16

//add connection trunk members on device 1
do cpss-api call cpssdxchtrunkmemberadd devNum 1 trunkId 222
32
17
do cpss-api call cpssdxchtrunkmemberadd devNum 1 trunkId 222
34
17
do cpss-api call cpssdxchtrunkmemberadd devNum 1 trunkId 222
36
17
do cpss-api call cpssdxchtrunkmemberadd devNum 1 trunkId 222
38
17


//disable NA messages on connection trunk members on device 0
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 0 portNum 8  enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 0 portNum 10 enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 0 portNum 12 enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 0 portNum 14 enable false

//disable NA messages on connection trunk members on device 1
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 1 portNum 32 enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 1 portNum 34 enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 1 portNum 36 enable false
do cpss-api call cpssDxChBrgFdbNaToCpuPerPortSet devNum 1 portNum 38 enable false



*/

GT_STATUS cmdEventRunTimeoutSet(IN GT_U32 cmdTimeout);
/* this function called before cpssInitSystem*/
static GT_STATUS   appDemoRunningInAsimEnv(void)
{
    #define CMD_PROC_TIMOUT 30000
    /*cmdSysTimeout = (CMD_PROC_TIMOUT * 6);*/
    cpssOsPrintf("set timeout for commands 10 times more than the regular timeout , as the ASIM runs slowly \n");
    /* set 10 times more than the regular timeout , as the ASIM runs slowly */
    cmdEventRunTimeoutSet(6*CMD_PROC_TIMOUT*10);

    appDemoDbEntryAdd("appDemoInAsimEnv", 1);

    return GT_OK;
}

GT_STATUS ASIC_SIMULATION_firmwareInit(IN GT_U8    devNum, IN GT_U32   phaFwVersionId);
static GT_STATUS appDemoPostInitAsimEnv(void)
{
    GT_U8   devNum;
    GT_U32  devIndex;

    cpssOsPrintf("ASIM env : manually connect the WM to the PHA threads (per device) \n");

    START_LOOP_ALL_DEVICES(devNum)
    {
        ASIC_SIMULATION_firmwareInit(devNum, 0/*default fw version ID*/);
    }
    END_LOOP_ALL_DEVICES

    return GT_OK;
}

extern void tgfTrafficGenerator_debug_captureOnTime_set(IN GT_U32  sleepTime);
extern void      debug_captureOnTime_set(IN GT_U32  sleepTime);
extern GT_STATUS simDisconnectAllOuterPorts(void);
extern GT_STATUS simDisconnectAllOuterPorts(void);

/* run this before starting running tests in LUA*/
GT_STATUS   luaTestsRunningInAsimEnv(void)
{
#ifdef CMD_LUA_CLI
    cpssOsPrintf("Notify Embedded (LUA/enhUT) tests to wait more on 'captured' packet \n");
    debug_captureOnTime_set(100);
    /* the LUA test 'mac_pdu' uses the TGF timeout for captured packet */
    tgfTrafficGenerator_debug_captureOnTime_set(100);
    cpssOsPrintf("Notify WM to disconnect from outer port connection so traffic from OCT/tuns not enter the switch \n");
    simDisconnectAllOuterPorts();
#else  /*CMD_LUA_CLI*/
    cpssOsPrintf("WARINING : LUA CLI not part of the compilation , so function do nothing \n");
#endif /*CMD_LUA_CLI*/
    return GT_OK;
}


