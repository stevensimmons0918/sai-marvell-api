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
* @file gtDbDxPxBc3PipesSmi.c
*
* @brief Initialization functions for the the BC3 (PEX) and 2 pipes (SMI)
*                                     (multiple boards).
*       NOTE: in WM this is single board.
*       NOTE: this file compiled only when both CHX_FAMILY and PX_FAMILY defined.
*
* @version   1
********************************************************************************
*/
#ifdef COW_INTERNAL_COMPILE
    /* define only the tested one */
    #define PX_FAMILY
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/userExit/userEventHandler.h>

/* DXCH includes */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>

/* PX includes */
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID   0xFF /*dummy*/

static GT_U8    saved_boardRevId = 0;
static GT_U8    bc3_devNum = 0;
static GT_BOOL  isAfterEventsInit = GT_FALSE;
static GT_U8   genericBoardRevision = 1;
extern GT_BOOL portMgr;

/* genericBoardConfigFun will hold the generic board configuration */
/* we will use those pointers as "set default" values that we can override */
static GT_BOARD_CONFIG_FUNCS genericBoardConfigFun;
/* hold PIPE board functions ... maybe used ! */
static GT_BOARD_CONFIG_FUNCS pipe_genericBoardConfigFun;
extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern boardRevId2PortsInitList_STC *appDemo_boardRevId2PortsInitListPtr;
/* number of elements in appDemo_boardRevId2PortsInitListPtr */
extern GT_U32                  appDemo_boardRevId2PortsSize;

extern APPDEMO_BC2_SERDES_POLARITY_STC *appDemoPolarityArrayPtr;
extern GT_U32                           appDemoPolarityArraySize;

extern GT_STATUS   px_force_Ports_12_10G_1_100G(void);
extern GT_BOOL     px_forwardingTablesStage_enabled;

extern void DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#define NA_PG                        0
/* the BC3 connected to the 2 PIPEs via ports 16,20 (each 100G port) */
#define BC3_MAC_PORT_TO_PIPE_0_CNS   16  /* SERDES 16,17,18,19 */
#define BC3_MAC_PORT_TO_PIPE_1_CNS   20  /* SERDES 20,21,22,23 */

/* physical port number for the cascade ports */
#define BC3_CASCADE_TO_PIPE_0_CNS   40/*40..43*/
#define BC3_CASCADE_TO_PIPE_1_CNS   44/*44..47*/

/* physical port number for the cascade ports */
#define PIPE_0_FIRST_PHYSICAL_PORT_NUMBER_CNS   16
#define PIPE_1_FIRST_PHYSICAL_PORT_NUMBER_CNS   28

#define PIPE_0_NUM_PORTS_CNS    12
#define PIPE_1_NUM_PORTS_CNS    12

/* TxQ number for the remote ports .
since the cascade ports relate to DP[1] (dma ports 12..23) we need TxQ numbers :
    92..191
*/
#define BC3_FIRST_TXQ_TO_PIPE_0_CNS   150
#define BC3_FIRST_TXQ_TO_PIPE_1_CNS   162

/* the BC3 connected to the 2 PIPEs via ports 16,20 (each 100G port) */
#define PIPE_0_FIRST_MAC_PORT_CNS   0  /* SERDES 0 */
#define PIPE_1_FIRST_MAC_PORT_CNS   0  /* SERDES 0 */

/* ports for BC3 (to fill cpssApi_bc3_mapPorts[] that used at cpssDxChPortPhysicalPortMapSet(...)) . */
/*
    supporting next PIPE configuration:
    1. PIPE #1 :
        a. 12 ports 10G
        c. uplink 100G
    2. PIPE #2 :
        a. 2 ports 40G
        b. 2 ports 10G
        c. uplink 100G
        (unused 2 SERDESes)
*/
static APP_DEMO_PORT_MAP_STC bc3_with_remote_ports_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               , 16   ,   0       , TXQ_BY_DMA_CNS/*startTxqNumber*/ ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP,GT_NA}

    /* PIPE#1 : remote physical ports 16..27 (Txq : 150..161), mapped to MAC 16 , with physical port 40 */
    ,{PIPE_0_FIRST_PHYSICAL_PORT_NUMBER_CNS , PIPE_0_NUM_PORTS_CNS   ,  BC3_MAC_PORT_TO_PIPE_0_CNS , BC3_FIRST_TXQ_TO_PIPE_0_CNS/*startTxqNumber*/  , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,BC3_CASCADE_TO_PIPE_0_CNS,PIPE_0_FIRST_MAC_PORT_CNS}

    /* physical ports 41..43 , mapped to MAC 17..19 */
/*unused*/,{41               , 3   ,   17       , TXQ_BY_DMA_CNS/*startTxqNumber*/ ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP,GT_NA}

    /* PIPE#2 : remote physical ports 28..39 (Txq : 162..173) , mapped to MAC 20 , with physical port 44 */
    ,{PIPE_1_FIRST_PHYSICAL_PORT_NUMBER_CNS , PIPE_1_NUM_PORTS_CNS   ,  BC3_MAC_PORT_TO_PIPE_1_CNS     , BC3_FIRST_TXQ_TO_PIPE_1_CNS/*startTxqNumber*/  ,CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,BC3_CASCADE_TO_PIPE_1_CNS,PIPE_1_FIRST_MAC_PORT_CNS}

    /* physical ports 45..47 , mapped to MAC 21..23 */
/*unused*/,{45               , 3   ,   21       , TXQ_BY_DMA_CNS/*startTxqNumber*/ ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP,GT_NA}

    /* physical ports 48..58 , mapped to MAC 24..34 */
    ,{48               , 59-48/*11*/   ,   24   , TXQ_BY_DMA_CNS/*startTxqNumber*/ ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP,GT_NA}

     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1    , 74/*DMA*/, TXQ_BY_DMA_CNS/*startTxqNumber*/ , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP ,GT_NA}

    /* physical ports 64..100 , mapped to MAC 35..71 */
    ,{64               , 37   ,   35   , TXQ_BY_DMA_CNS/*startTxqNumber*/ ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP,GT_NA}

    /* 'CPU ports' : physical ports 101..102 , mapped to MAC 72..73 */
    ,{101              , 2   ,  72       , TXQ_BY_DMA_CNS/*startTxqNumber*/ , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP ,GT_NA}
    /* map 'CPU SDMA ports' 103..105 (no MAC/SERDES) DMA 75..77 */
    ,{103              , 3   ,  75/*DMA*/, TXQ_BY_DMA_CNS/*startTxqNumber*/ , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP ,GT_NA}


    ,{GT_NA,0,0,0,0,0,GT_NA}/* must be last */
};

/* based on: portInitlist_armstrong_rd[] , state speeds for the ports */
static PortInitList_STC portInitlist_bc3_speeds[] =
{
   /* the cascade ports */
    { PORT_LIST_TYPE_LIST,      {BC3_CASCADE_TO_PIPE_0_CNS, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   ,{ PORT_LIST_TYPE_LIST,      {BC3_CASCADE_TO_PIPE_1_CNS, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   /* native BC3 ports : 100G   (mac 0,4,8) */
   ,{ PORT_LIST_TYPE_LIST,      {0,4,8,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,   CPSS_PORT_INTERFACE_MODE_KR4_E  }
   /* native BC3 ports : 25G   (mac 12..15) but only 2 of them can be used because each pipe gets 120G ! */
   ,{ PORT_LIST_TYPE_LIST,      {12,13,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  }
   /* the remote ports */
   ,{ PORT_LIST_TYPE_INTERVAL,  {PIPE_0_FIRST_PHYSICAL_PORT_NUMBER_CNS,PIPE_0_FIRST_PHYSICAL_PORT_NUMBER_CNS+PIPE_0_NUM_PORTS_CNS-1,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   ,{ PORT_LIST_TYPE_INTERVAL,  {PIPE_1_FIRST_PHYSICAL_PORT_NUMBER_CNS,PIPE_1_FIRST_PHYSICAL_PORT_NUMBER_CNS+PIPE_1_NUM_PORTS_CNS-1,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   /* native BC3 ports : 48 ports of 10G    (mac 24..71)*/
   ,{ PORT_LIST_TYPE_INTERVAL,  {48, 58,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
                    /* jump non-exists physical port numbers [59..62] and the 'CPU port' 63 */
                    /* fix JIRA : CPSS-8930 : Five 10G ports of BC3 are not configured/initialized */
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,100,1,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
    /* the 2 'CPU' ports        (MAC 72..73)*/
   ,{ PORT_LIST_TYPE_LIST,      {101,102,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E   }
   /* must be last */
   ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_QSGMII_E }
};
/*based on : armstrong_rd_prv_boardRevId2PortsInitList[] */
static boardRevId2PortsInitList_STC portInitlist_bc3_speeds_InitList[]=
{
    /* clock, revid,  port list */
     {  525,   1,  CPSS_PP_SUB_FAMILY_NONE_E,     (BIT_6-1),  &portInitlist_bc3_speeds[0] }
     /* must be last */
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,       0x0,  NULL                            }
};

/* the table hold shadow of the BC3 HW configurations   */
/* the table hold shadow of the BC3 HW configurations   */
/* during function gtDbDxPxBc3PipesPipeTpidUpdate(...)  */
/* 1. info filled by appDemoGlobalTpidInfoGet(...)      */
/* 2. used in function pipeTpidUpdate(...)              */
/*                                                      */
/* also used by pipeFrontPanelPortsSet(...)  with the 'defaults' of 0x8100 */

static GT_U16   tpidTable[8/* TPID index 0..7 */] = {
     0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
};

/* the table hold shadow of the BC3 HW configurations   */
/* during function gtDbDxPxBc3PipesPipeTpidUpdate(...)  */
/* 1. info filled by appDemoGlobalTpidInfoGet(...)      */
/* 2. used in function pipeTpidUpdate_portProfiles(...) */
static GT_U8   tpidProfileBmp[2/*ETHERTYPE0 , ETHERTYPE1*/][8/* profile index 0..7 */] = {
       {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}/*CPSS_VLAN_ETHERTYPE0_E*/
      ,{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}/*CPSS_VLAN_ETHERTYPE1_E*/
};

/* per port info */
typedef struct{
    GT_BOOL     isPortMappedAsRemote;/* indication that the port is used as remote on DX device */
    GT_U32      remotePortNumberOnDx;/* valid when isPortMappedAsRemote == GT_TRUE */
    GT_U32      tpidProfile[2];/* index into tpidProfileBmp[0 or 1][x] */
} PIPE_PORT_INFO_STC;

/* (based on) similar to CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC */
typedef struct{
    GT_BOOL isExists; /* indication set in runtime according to actual device recognition on the 'SMI bus' */
    GT_UINTPTR busBaseAddr;/*SMI address (on the bus) : 0..31.*/
    GT_U32 numOfRemotePorts;
    GT_U32 startPhysicalPortNumber;
    GT_U32 startRemoteMacPortNum;
    GT_SW_DEV_NUM  cpssDevNum;/* NOTE: set by default like appDemoDevIndex , but may be changed after 'removel of device' and re-insert to new deviceNumber */
    GT_U32 dxCascadePortNum;/* physical port number of the cascade port from the DX device side */
    GT_U8  appDemoDevIndex;/* index in appDemoPpConfigList */
    PIPE_PORT_INFO_STC  portInfo[16];
} PIPE_SMI_REMOTE_DEV_INFO_STC;

static PIPE_SMI_REMOTE_DEV_INFO_STC pipe0_Info =
{
    GT_FALSE/*isExists*/
    ,4  /* SMI address */
    ,PIPE_0_NUM_PORTS_CNS/*numOfRemotePorts*/
    ,PIPE_0_FIRST_PHYSICAL_PORT_NUMBER_CNS/*startPhysicalPortNumber*/
    ,0/*startRemoteMacPortNum*/
    ,1/* cpssDevNum */
    ,BC3_CASCADE_TO_PIPE_0_CNS/*dxCascadePortNum*/
    ,1/*appDemoDevIndex*/
    ,{{GT_FALSE, 0, {0,0}}}
};
static PIPE_SMI_REMOTE_DEV_INFO_STC pipe1_Info =
{
    GT_FALSE/*isExists*/
    ,5 /* SMI address */
    ,PIPE_1_NUM_PORTS_CNS/*numOfRemotePorts*/
    ,PIPE_1_FIRST_PHYSICAL_PORT_NUMBER_CNS/*startPhysicalPortNumber*/
    ,0/*startRemoteMacPortNum*/
    ,2/* cpssDevNum */
    ,BC3_CASCADE_TO_PIPE_1_CNS/*dxCascadePortNum*/
    ,2/*appDemoDevIndex*/
    ,{{GT_FALSE, 0, {0,0}}}
};

static PIPE_SMI_REMOTE_DEV_INFO_STC* pipesInfo[] = {
     &pipe0_Info
    ,&pipe1_Info
    ,NULL
};

#define MAX_MAPPED_PORTS_CNS    128
static  GT_U32  actualNum_cpssApi_bc3_mapPorts = 0;/*actual number of valid entries in cpssApi_bc3_mapPorts[] */
/* cpssApi_bc3_mapPorts used at cpssDxChPortPhysicalPortMapSet(...) */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_bc3_mapPorts[MAX_MAPPED_PORTS_CNS] =
{
    /* filled at run time , by info from :
        bc3_with_remote_ports_defaultMap[]
    */
    {0, 0, 0, 0, 0, GT_FALSE, 0,GT_FALSE}
};

static GT_U32  cascadePorts[] = {/* physical port numbers of the cascade ports */
    BC3_CASCADE_TO_PIPE_0_CNS,
    BC3_CASCADE_TO_PIPE_1_CNS,
    GT_NA};

static GT_STATUS initConnectionToPipe(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
);
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_byDxCascadePort(IN GT_U32 dxCascadePortNum);
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_byCpssDevNum(IN GT_U8 cpssDevNum);
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_bySmiAddr(IN GT_U32 smiAddr);
static GT_U32 gtDbDxPxBc3PipesSmiBoard_rescan_smi(void);
static GT_STATUS gtDbDxPxBc3PipesSmiBoard_newPipeDevice(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr,
    IN GT_SW_DEV_NUM        newDevNum
);
static GT_STATUS gtDbDxPxBc3PipesPipeTpidUpdate_singlePipe(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * specific_internal_infoPtr/* ignored if NULL*/
);
static GT_STATUS pipe_treatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
    OUT GT_BOOL     *stopLogicPtr
);
static GT_STATUS    pipe_devInitFailed_doCleanUp(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
);

/**
* @internal getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] isB2bSystem              - GT_TRUE, the system is a B2B system.
* @param[out] numOfPp                  - Number of Packet processors in system.
* @param[out] numOfFa                  - Number of Fabric Adaptors in system.
* @param[out] numOfXbar                - Number of Crossbar devices in system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getBoardInfo
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT GT_U8   *numOfFa,
    OUT GT_U8   *numOfXbar,
    OUT GT_BOOL *isB2bSystem
)
{
    GT_STATUS rc;
    GT_U32  ii;
    GT_U32  devIndex;

    boardRevId = boardRevId;

    /* speed up the initialization do not hold SW shadow */
    appDemoDbEntryAdd("noDataIntegrityShadowSupport",1);

    rc = genericBoardConfigFun.boardGetInfo(genericBoardRevision,numOfPp,numOfFa,
                                            numOfXbar,isB2bSystem);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardGetInfo from BC3", rc);
        return rc;
    }

    /* extra/override info needed */

    /* do SMI scan and find the PIPE SMI devices */
    rc = appDemoSysGetSmiInfo();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoSysGetSmiInfo", rc);
        return rc;
    }

    for(devIndex = 1 ; devIndex <= 2 ; devIndex++)/* we expect 2 PIPEdevices at DB indexes 1,2 */
    {
        /* we expect to find 2 PIPE devices , at indexes 1,2 */
        for(ii = 0 ; pipesInfo[ii] ;ii++)
        {
            if(pipesInfo[ii]->isExists == GT_TRUE)
            {
                /* the DB already used */
                continue;
            }

            if(GT_FALSE == appDemoPpConfigList[devIndex].valid)
            {
                /* support case that the device is not on the SMI bus */
                continue;
            }
            else
            if(CPSS_CHANNEL_SMI_E != appDemoPpConfigList[devIndex].channel)
            {
                /* on this board the device expected to be SMI connected to the CPU */
                rc = GT_BAD_STATE;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("PIPE device not stated as 'SMI' interface in AppDemo DB", rc);
                return rc;
            }
            else
            if(pipesInfo[ii]->busBaseAddr != appDemoPpConfigList[devIndex].smiInfo.smiIdSel)
            {
                /* this is not our device */
                continue;
            }

            /* 'bind the device' to the info DB */
            pipesInfo[ii]->appDemoDevIndex = (GT_U8)devIndex;
            pipesInfo[ii]->isExists        = GT_TRUE;
        }
    }

    return GT_OK;
}
/**
* @internal getPpPhase1Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase1Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devIdx                   - The Pp Index to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase1Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS    *phase1Params
)
{
    GT_STATUS rc;
    GT_U32 value;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpPh1Params(genericBoardRevision,devIdx,phase1Params);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardGetPpPh1Params from BC3", rc);
        return rc;
    }

    bc3_devNum = appDemoPpConfigList[0].devNum;

    if (appDemoDbEntryGet("portMgr", &value) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    /* no extra/override info needed */

    return GT_OK;
}

/**
* @internal configBoardAfterPhase1 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase1.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase1
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS rc;
    GT_U32  ii;
    GT_U8  devNum = bc3_devNum;

    boardRevId = boardRevId;

    rc = appDemoDxChFillDbForCpssPortMappingInfo(
        bc3_devNum,
        &bc3_with_remote_ports_defaultMap[0],
        &cpssApi_bc3_mapPorts[0],
        &actualNum_cpssApi_bc3_mapPorts,
        NULL /*numCpuSdmasPtr*/);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChFillDbForCpssPortMappingInfo", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    appDemoPortMapSize = actualNum_cpssApi_bc3_mapPorts;
    appDemoPortMapPtr  = &cpssApi_bc3_mapPorts[0];

    appDemo_boardRevId2PortsInitListPtr = &portInitlist_bc3_speeds_InitList[0];
    appDemo_boardRevId2PortsSize = SIZE_OF_MAC(portInitlist_bc3_speeds_InitList);

    rc = genericBoardConfigFun.boardAfterPhase1Config(genericBoardRevision);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAfterPhase1Config from BC3", rc);
        return rc;
    }

    /* extra/override info needed */

    rc = px_force_Ports_12_10G_1_100G();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_force_Ports_12_10G_1_100G", rc);
        return rc;
    }

    for(ii = 0 ; cascadePorts[ii] != GT_NA ; ii++)
    {
        /* based on : initConnectionTo88e1690(...) */
        rc = initConnectionToPipe(devNum,cascadePorts[ii]);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("initConnectionToPipe", rc);
            return rc;
        }
    }

    /* enable port isolation for the src-filter for trunks in stacking WA */
    rc = cpssDxChNstPortIsolationEnableSet(devNum,GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIsolationEnableSet", rc);
        return rc;
    }


    /* state that port isolation work according to eport/trunkId from DSA/local
        and not according to src port/trunk on the local device */
    rc = cpssDxChNstPortIsolationOnEportsEnableSet(devNum,GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIsolationEnableSet", rc);
        return rc;
    }


    /* disable the <OrigSrcPortFilter> due to wrong <Orig Src Phy Port Or Trunk ID> */
    /* this adds no limitation on the application/system !  */
    rc = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum,GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdOrigSrcPortFilterEnableSet", rc);
        return rc;
    }


    return GT_OK;
}

/**
* @internal getPpPhase2Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase2Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] oldDevNum                - The old Pp device number to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    OUT CPSS_PP_PHASE2_INIT_PARAMS    *phase2Params
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpPh2Params(genericBoardRevision,oldDevNum,phase2Params);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardGetPpPh2Params from BC3", rc);
        return rc;
    }

    /* no extra/override info needed */

    return GT_OK;
}

/**
* @internal configBoardAfterPhase2 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase2.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase2
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardAfterPhase2Config(genericBoardRevision);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAfterPhase2Config from BC3", rc);
        return rc;
    }

    /* no extra/override info needed */

    return GT_OK;
}

/**
* @internal getPpLogicalInitParams function
* @endinternal
*
* @brief   Returns the parameters needed for sysPpLogicalInit() function.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ppLogInitParams          - Pp logical init parameters struct.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpLogicalInitParams
(
    IN  GT_U8           boardRevId,
    IN  GT_U8           devNum,
    OUT CPSS_PP_CONFIG_INIT_STC    *ppLogInitParams
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpLogInitParams(genericBoardRevision,devNum,ppLogInitParams);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardGetPpLogInitParams from BC3", rc);
        return rc;
    }

    /* no extra/override info needed */

    /* at this stage the DX device already triggered the 'event handles:
        1. appDemoEventHandlerPreInit
        2. appDemoEventRequestDrvnModeInit

       now we need to call it for the Pipe devices
    */
    isAfterEventsInit = GT_TRUE;

    return GT_OK;
}

/**
* @internal getTapiLibInitParams function
* @endinternal
*
* @brief   Returns Tapi library initialization parameters.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] libInitParams            - Library initialization parameters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getTapiLibInitParams
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT APP_DEMO_LIB_INIT_PARAMS    *libInitParams
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetLibInitParams(genericBoardRevision,devNum,libInitParams);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardGetLibInitParams from BC3", rc);
        return rc;
    }

    /* no extra/override info needed */

    return GT_OK;
}

/**
* @internal afterInitBoardConfig function
* @endinternal
*
* @brief   PSS Board specific configurations that should be done after board
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS afterInitBoardConfig
(
    IN  GT_U8                       boardRevId
)
{
    GT_STATUS   rc;
    GT_U8  devNum = bc3_devNum;

    boardRevId = boardRevId;

    /* let the generic board do it's things */
    rc = genericBoardConfigFun.boardAfterInitConfig(genericBoardRevision);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardAfterInitConfig from BC3", rc);
        return rc;
    }

    /* extra/override info needed */

    /* enable port isolation per vlan for the src-filter for trunks in stacking WA */
    rc = cpssDxChBrgVlanPortIsolationCmdSet(devNum,
        1/* default vlanId */,
        CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortIsolationCmdSet", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal gtDbDxPxBc3PipesSmiBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbDxPxBc3PipesSmiBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS   rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardCleanDbDuringSystemReset(genericBoardRevision);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("boardCleanDbDuringSystemReset from BC3", rc);
        return rc;
    }

    isAfterEventsInit = GT_FALSE;

    return GT_OK;
}


/*
 * pxDeviceRemoved_impactDxDevice
 *
 * Description: remove PX device hold impact of the DX device.
 *              NOTE: at this point the 'cpss' is not valid any more for this
 *              'removed' PX device !!!
 *              appDemoPpConfigList[] and internal_infoPtr are still valid for
 *              this device.
 *
 * Fields:
 *      internal_infoPtr - pointer to pipe device info (in this file)
 *
 */
static GT_STATUS pxDeviceRemoved_impactDxDevice
(
    IN  PIPE_SMI_REMOTE_DEV_INFO_STC *internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_BOOL stopLogic;
    GT_U32  portNum;
    GT_U8   devNum = CAST_SW_DEVNUM(internal_infoPtr->cpssDevNum);

    /* make sure that the BC3 will know that the remote physical ports need to
       be considered 'link down' */
    /* NOTE: the function pipe_treatLinkStatusChange will call DXCH API !!!
             but not to PX APIs that device already removed !!!
    */
    for(portNum = 0 ; portNum < 16 ; portNum++)
    {
        if (internal_infoPtr->portInfo[portNum].isPortMappedAsRemote == GT_FALSE)
        {
            continue;
        }

        rc = pipe_treatLinkStatusChange(devNum,portNum,
            GT_FALSE,/* indicate that the port is link down */
            APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E,
            &stopLogic);
        if(rc != GT_OK)
        {
            osPrintf("pipe_treatLinkStatusChange : failed on pipe[%d] port[%d] \n",
                devNum,
                portNum);
            /* do not return / break ! .... continue to other ports */
        }
    }

    /* NOTE: application may want to also remove those ports from
        trunks / vlans / FDB ....

        but the 'appDemo' not going that 'far'
    */

    if(!portMgr)
    {
        CPSS_PORTS_BMP_STC portBmp;
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);
        /* link down BC3 cascade port when PIPE is removed */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, internal_infoPtr->dxCascadePortNum);
        cpssDxChPortModeSpeedSet(bc3_devNum, &portBmp, GT_FALSE,
                                 /*don't care*/CPSS_PORT_INTERFACE_MODE_NA_E,
                                 /*don't care*/CPSS_PORT_SPEED_NA_E);
        cpssOsPrintf("BC3 casade port %d powered down\n", internal_infoPtr->dxCascadePortNum);
    }

    return GT_OK;
}


/*
 * gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb
 *
 * Description: remove device from the DB of this 'board config'
 *          NOTE: the device was already removed from the CPSS ,
 *               but still exists in appDemoPpConfigList[]
 *              this function is called from cpssPpRemove
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
static GT_STATUS gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb
(
    IN  GT_U8                       devIdx,
    IN  GT_U8                       boardRevId
)
{
    PIPE_SMI_REMOTE_DEV_INFO_STC *internal_infoPtr;
    GT_U8   devNum = appDemoPpConfigList[devIdx].devNum;

    boardRevId = boardRevId;

    internal_infoPtr = getPipeDevInfo_byCpssDevNum(devNum);
    if(internal_infoPtr == NULL)
    {
        /* support missing device */
        return GT_OK;
    }

    /* clean-up the PX info from the DX device */
    (void) pxDeviceRemoved_impactDxDevice(internal_infoPtr);

    /* state that the device not exists */
    internal_infoPtr->isExists = GT_FALSE;

    return GT_OK;
}
/**
* @internal gtDbDxPxBc3PipesSmiBoardReg_DeviceSimpleCpssPpInsert function
* @endinternal
*
* @brief   'board specific' support for Insert packet processor.
*       NOTE: called by cpssPpInsert(...)
*       and if such function exists , the cpssPpInsert(...) only 'count the time'
*
* @param[in] busType        - PP interface type connected to the CPU.
* @param[in] busNum         - PCI/SMI bus number.
* @param[in] busDevNum      - PCI/SMI Device number.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] parentDevNum   - Device number of the device to which the
*                             current device is connected. A value 0xff
*                             means PP is directly connected to cpu.
* @param[in] insertionType  - place holder for future options to initialize PP.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong devNum(>31). Already used busNum/busDevNum.
* @retval GT_FAIL           - otherwise.
*/
static GT_STATUS gtDbDxPxBc3PipesSmiBoardReg_DeviceSimpleCpssPpInsert
(
    IN  CPSS_PP_INTERFACE_CHANNEL_ENT busType,
    IN  GT_U8  busNum,
    IN  GT_U8  busDevNum,
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_SW_DEV_NUM  parentDevNum,
    IN  APP_DEMO_CPSS_HIR_INSERTION_TYPE_ENT insertionType
)
{
    GT_STATUS   rc;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr;
    GT_U8       devIdx;/* index into appDemoPpConfigList[]*/

    insertionType = insertionType;/* not used */
    parentDevNum  = parentDevNum; /* not used */

    if(busType != CPSS_CHANNEL_SMI_E)
    {
        rc = GT_NOT_SUPPORTED;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("only PIPE SMI devices supported", rc);
        return rc;
    }

    if(busNum != 0)
    {
        rc = GT_BAD_PARAM;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("busNum must be ZERO", rc);
        return rc;
    }

    internal_infoPtr = getPipeDevInfo_bySmiAddr(busDevNum);
    if(internal_infoPtr == NULL)
    {
        rc = GT_NOT_FOUND;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("not valid SMI addr for this board", rc);
        return rc;
    }

    if(internal_infoPtr->isExists == GT_TRUE)
    {
        osPrintf("The device already exists as devNum [%d] \n",
            internal_infoPtr->cpssDevNum);
        rc = GT_ALREADY_EXIST;

        CPSS_ENABLER_DBG_TRACE_RC_MAC("the device already exists", rc);

        return rc;
    }

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        rc = GT_BAD_PARAM;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("devNum must be less than 128", rc);
        return rc;
    }

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        rc = GT_ALREADY_EXIST;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("other device already use this devNum \n", rc);
        return rc;
    }

    /* rescan the SMI to add valid new devices into appDemoPpConfigList[] */
    /* the function return BMP of new SMI addresses , but we not use it */
    (void)gtDbDxPxBc3PipesSmiBoard_rescan_smi();

    devIdx = internal_infoPtr->appDemoDevIndex;
    if(appDemoPpConfigList[devIdx].valid == GT_FALSE ||
       appDemoPpConfigList[devIdx].smiInfo.smiIdSel != internal_infoPtr->busBaseAddr )
    {
        /* trying to add device while not connected to the SMI */
        /* the function gtDbDxPxBc3PipesSmiBoard_rescan_smi() did not recognized and 'validated' it */
        rc = GT_NOT_FOUND;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("The device not found on the SMI addr ", rc);
        return rc;
    }

    rc = gtDbDxPxBc3PipesSmiBoard_newPipeDevice(internal_infoPtr,devNum);
    if(rc != GT_OK)
    {
        (void)pipe_devInitFailed_doCleanUp(internal_infoPtr);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtDbDxPxBc3PipesSmiBoard_newDevice \n", rc);
        return rc;
    }

    rc = gtDbDxPxBc3PipesPipeTpidUpdate_singlePipe(internal_infoPtr);
    if(rc != GT_OK)
    {
        osPrintf("Device [%d] SMI[%d] successfully added . but FAILED(rc = %d) to synch 'tpid' configurations to it \n",
            devNum,
            internal_infoPtr->busBaseAddr,
            rc);
        /* fail here not require to remove the device !!! */
        /* the device was added , we only failed to synch 'tpid' configurations to it !!! */
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtDbDxPxBc3PipesPipeTpidUpdate_singlePipe \n", rc);
        return rc;
    }

    if(!portMgr)
    {
        CPSS_PORTS_BMP_STC portBmp;
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);
        /* link up BC3 cascade port */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, internal_infoPtr->dxCascadePortNum);
        cpssDxChPortModeSpeedSet(bc3_devNum, &portBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_100G_E);
        cpssOsPrintf("BC3 casade port %d powered up\n", internal_infoPtr->dxCascadePortNum);
    }

    return GT_OK;
}

/* replacing : prvCpssDxChCfgPort88e1690CascadePortToInfoGet(...) */
static APP_DEMO_PORT_MAP_STC* getCascadePortInfo(IN GT_U32  cascadePortNum)
{
    GT_U32  ii;

    for(ii = 0 ;bc3_with_remote_ports_defaultMap[ii].startPhysicalPortNumber != GT_NA ;ii++)
    {
        /* the physical port of the cascade of the DX is in filed of 'jumpDmaPorts'
           when the mappingType is 'remote physical port' */

        if(bc3_with_remote_ports_defaultMap[ii].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            continue;
        }

        if(bc3_with_remote_ports_defaultMap[ii].jumpDmaPorts != cascadePortNum)
        {
            continue;
        }

        return &bc3_with_remote_ports_defaultMap[ii];
    }

    return NULL;
}

/* based on : internal_initConnectionTo88e1690_dxch_remote_port(...) */
static GT_STATUS internal_initConnectionToPipe_dxch_remote_port(
    IN GT_U8   devNum,
    IN GT_U32  cascadePortNum,
    IN GT_U32  physicalPortNumber,
    IN GT_U32  remoteMacNumber
)
{
    GT_STATUS rc;
    GT_U32  portNum = physicalPortNumber;
    GT_U32  dsa_HwDevNum , dsa_PortNum = remoteMacNumber;
    GT_BOOL  value = GT_TRUE;

    {
        PIPE_SMI_REMOTE_DEV_INFO_STC * pipe_internal_infoPtr = getPipeDevInfo_byDxCascadePort(cascadePortNum);

        if(pipe_internal_infoPtr)
        {
            pipe_internal_infoPtr->portInfo[remoteMacNumber].isPortMappedAsRemote = GT_TRUE;
            pipe_internal_infoPtr->portInfo[remoteMacNumber].remotePortNumberOnDx = physicalPortNumber;
        }
    }

    dsa_HwDevNum = 0;/* the bits from the DSA are ignored anyway ! by the PIPE */

    /* state that the port egress packets with DSA (single word) */
    rc = cpssDxChCscdPortTypeSet(devNum, portNum ,
        CPSS_PORT_DIRECTION_TX_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
         The following configuration should be applied in the Header Alteration Physical Port Table 2:
             <Transmit FORWARD packets with Forced 4B FROM_CPU bit> = 1
             <Transmit TO_ANALZER packets with Forced 4B FROM_CPU bit> = 1
             <Transmit FROM_CPU packets with Forced 4B FROM_CPU bit> = 1
    */

    rc = cpssDxChCscdPortForce4BfromCpuDsaEnableSet(devNum, portNum, value);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet (devNum, portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* set info what the egress DSA should hold for target port , target device */
    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet (devNum, portNum,
        dsa_HwDevNum , dsa_PortNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* force the 'localDevSrcPort' to do 'briging' , because by default the bridge
       is bypassed for 'eDSA' packets , that comes from the PIPE , but the PIPE
       not aware to the bridging */
    rc = cpssDxChCscdPortBridgeBypassEnableSet(devNum,portNum,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* based on : internal_initConnectionTo88e1690_dxch(...) */
static GT_STATUS internal_initConnectionToPipe_dxch(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS rc;
    GT_U32  srcDevLsbAmount = 0 , srcPortLsbAmount = 4;
    GT_U32    physicalPortBase;
    APP_DEMO_PORT_MAP_STC * internal_infoPtr = getCascadePortInfo(cascadePortNum);
    GT_U32  ii;

    if(internal_infoPtr == NULL)
    {
        rc = GT_BAD_PARAM;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("NULL pointer", rc);
        return rc;
    }

    /***********************/
    /* global - per device */
    /***********************/
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        devNum, srcDevLsbAmount,srcPortLsbAmount);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***********************/
    /* per cascade port    */
    /***********************/

    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, cascadePortNum  ,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* actually BC3 gets (from PIPE) eDSA , but BC3 sends (to PIPE) '1 word 'from cpu' DSA' */
    rc = cpssDxChCscdPortTypeSet(devNum, cascadePortNum ,
        CPSS_PORT_DIRECTION_BOTH_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, cascadePortNum ,
         CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    physicalPortBase = internal_infoPtr->startPhysicalPortNumber /*remotePhysicalPortNum*/ -
                       internal_infoPtr->startRemoteMacNumber    /*remoteMacPortNum*/;

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, cascadePortNum ,
        physicalPortBase);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum,
        cascadePortNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 88e1690 supports Tx for max 10K , we add 16 ... to not be bottleneck in any case */
    rc = cpssDxChPortMruSet(devNum, cascadePortNum , _10K + 16);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************/
    /* per remote physical port    */
    /*******************************/
    for(ii = 0 ; ii < internal_infoPtr->numOfPorts ; ii++)
    {
        rc = internal_initConnectionToPipe_dxch_remote_port(devNum,cascadePortNum,
                internal_infoPtr->startPhysicalPortNumber + ii,
                internal_infoPtr->startRemoteMacNumber    + ii);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_dxch_remote_port", rc);
            return rc;
        }
    }

    return GT_OK;
}

/* get info about the PIPE device according to the SMI address */
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_bySmiAddr(IN GT_U32 smiAddr)
{
    GT_U32  ii;
    for(ii = 0 ; pipesInfo[ii] ;ii++)
    {
        if(pipesInfo[ii]->busBaseAddr == smiAddr)
        {
            return pipesInfo[ii];
        }
    }

    return NULL;
}

/* get info about the PIPE device according to the cascade port on the DX device */
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_byDxCascadePort(IN GT_U32 dxCascadePortNum)
{
    GT_U32  ii;
    for(ii = 0 ; pipesInfo[ii] ;ii++)
    {
        if(pipesInfo[ii]->isExists == GT_FALSE)
        {
            continue;
        }

        if(pipesInfo[ii]->dxCascadePortNum == dxCascadePortNum)
        {
            return pipesInfo[ii];
        }
    }

    return NULL;
}
/* get info about the PIPE device according to the cpssDevNum of the PIPE device */
static PIPE_SMI_REMOTE_DEV_INFO_STC * getPipeDevInfo_byCpssDevNum(
    IN GT_U8 cpssDevNum
)
{
    GT_U32  ii;
    for(ii = 0 ; pipesInfo[ii] ;ii++)
    {
        if(pipesInfo[ii]->isExists == GT_FALSE)
        {
            continue;
        }

        if(pipesInfo[ii]->cpssDevNum == cpssDevNum)
        {
            return pipesInfo[ii];
        }
    }

    return NULL;
}

#define PORT_SKIP_CHECK(dev,port)                                             \
    {                                                                         \
        GT_BOOL     isValid;                                                  \
        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum , &isValid); \
        if(rc != GT_OK || isValid == GT_FALSE)                                \
        {                                                                     \
            continue;                                                         \
        }                                                                     \
    }

typedef enum{
    SRC_PORT_PROFILE_FRONT_UPLINK_E = 0,
    SRC_PORT_PROFILE_FRONT_PANEL_E
}SRC_PORT_PROFILE_E;

typedef enum{
    /************************************/
    /* profiles for 'front panel' ports */
    /************************************/
    /* BIT 0 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_E                                  = 1 << 0   ,
    /* BIT 1 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E               = 1 << 1   ,
    /* BIT 2 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E               = 1 << 2   ,
    /* BIT 3 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E     = 1 << 3   ,
    /* BIT 4 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E     = 1 << 4   ,
    /* BIT 5 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E               = 1 << 5   ,
    /* BIT 6 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E               = 1 << 6   ,
}SRC_PORT_PROFILE_BMP_E;

typedef enum{
            /* uplink */
/* 0*/    PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E,
/* 1*/    PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_INVALID_E,        /* 'fallback' if not matched in 'uplink' */

            /* front panel */
/* 2*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_2_TAG1_E,
/* 3*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_2_TAG1_E,

/* 4*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_3_TAG1_E,
/* 5*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_3_TAG1_E,

/* 6*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_E,
/* 7*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_E,

/* 8*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_2_TAG1_E,
/* 9*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_3_TAG1_E,

/*10*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_UNTAGGED_E, /* 'fallback' if not matched in any of the tag0/1 */

}PIPE_INGRESS_TRAFFIC_TYPE_ENT;

typedef struct{
    GT_U32                                      tpidIndex;
    GT_U32                                      tpid1Index;  /* relevant for tag1 or tag0+tag1*/
    PIPE_INGRESS_TRAFFIC_TYPE_ENT               appDemoType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   cpssType;
    GT_U32                                      srcProfileId;/* combination of values from SRC_PORT_PROFILE_E */
}APPDEMO_TO_CPSS_TRAFFIC_TYPE_STC;

static APPDEMO_TO_CPSS_TRAFFIC_TYPE_STC frontPanel_appDemoToCpssTrafficType[] =
{    /*tpidIndex*//*tpid1Index*/     /*appDemoType*/                                                  /*cpssType*/                                                                    /*srcProfileId*/
     {0          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_2_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E}
    ,{1          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_2_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E}

    ,{0          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_3_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E}
    ,{1          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_3_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E}

    ,{0          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E        ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E}
    ,{1          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E        ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E}

    ,{2          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_2_TAG1_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E   ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E}
    ,{3          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_3_TAG1_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E   ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E}

    ,{0          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_UNTAGGED_E                , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E      ,0/* no extra*/}

    /* must be last */
    ,{GT_NA,0,0,0,0}
};

static CPSS_PX_PORTS_BMP    uplinkPortsBmp       = 0x1000;/* single 100G port */
static CPSS_PX_PORTS_BMP    frontPannelPortsBmp  = 0x0FFF;/* ports  0..11 */
/* base index in the table of 'destination' table for the traffic ingress from the 'front panel' */
#define PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX 0
/* base index in the table of 'destination' table for the traffic ingress from the 'uplink' */
#define PIPE_UPLINK_DESTINATION_BASE_INDEX      0x100


static GT_STATUS    pipeTpidUpdate_portProfiles(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS               rc;
    GT_SW_DEV_NUM   devNum = internal_infoPtr->cpssDevNum;
    GT_U32                  tpidIndex;
    GT_PORT_NUM             portNum;
    CPSS_ETHER_MODE_ENT     ethMode;
    GT_U32                  tpidProfile;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    GT_U32                  srcPortProfile;

    for (portNum = 0; portNum < 12; portNum++)
    {
        srcPortProfile = 0;

        for(ethMode  = CPSS_VLAN_ETHERTYPE0_E ;
            ethMode <= CPSS_VLAN_ETHERTYPE1_E ;
            ethMode++)
        {
            tpidProfile = internal_infoPtr->portInfo[portNum].tpidProfile[ethMode-1];

            for(tpidIndex = 0 ; tpidIndex < 4; tpidIndex++)
            {
                if(tpidProfileBmp[ethMode-1][tpidProfile] & (1 << tpidIndex))
                {
                    if(ethMode == CPSS_VLAN_ETHERTYPE0_E)
                    {
                        if(tpidIndex == 0)
                        {
                            /* we recognize TPID 0,1 */
                            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E;
                        }
                        else
                        if(tpidIndex == 1)
                        {
                            /* we recognize TPID 0,1 */
                            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E;
                        }
                    }
                    else
                    {
                        if(tpidIndex == 2)
                        {
                            /* we recognize TPID 2,3 */
                            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E;
                        }
                        else
                        if(tpidIndex == 3)
                        {
                            /* we recognize TPID 2,3 */
                            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E;
                        }
                    }
                }
            }
        }

        if((srcPortProfile & (SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E)) &&
           (srcPortProfile & (SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E | SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E)) )
        {
            /* both tags can be from this port , so allow also 'tag0+tag1' */
            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E;
        }

        if((srcPortProfile & (SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E)) &&
           (srcPortProfile & (SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E | SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E)) )
        {
            /* both tags can be from this port , so allow also 'tag0+tag1' */
            srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E;
        }

        /* those ports are 'front panel' ports  */
        srcPortProfile |= SRC_PORT_PROFILE_BMP_FRONT_PANEL_E;

        rc = cpssPxIngressPortPacketTypeKeyGet(devNum,portNum,&portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;

        portKey.srcPortProfile = srcPortProfile;

        rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}


static GT_STATUS pipeTpidUpdate(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM   devNum = internal_infoPtr->cpssDevNum;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    GT_U32                  entryIndex;
    GT_U32                  tpidIndex;
    GT_U16                  tpid0,tpid1;
    CPSS_PX_INGRESS_TPID_ENTRY_STC  tpidEntry;
    CPSS_PX_PACKET_TYPE     packetType;

    tpidEntry.size = 2; /* BUG in CPSS (CPSS-8765 : PIPE : cpssPxIngressTpidEntrySet use 'size' as '2 bytes' resolution instead of '1 byte')
                           TPID size resolution is 2B. */
    tpidEntry.valid = GT_TRUE;

    /* NOTE: the 'masks' where already set during 'init' in pipeFrontPanelPortsSet(...) */

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        tpidIndex = frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex;
        tpid0 = tpidTable[tpidIndex];
        tpid1 = 0;/* yet TBD */

        if(tpidIndex < 4)
        {
            tpidEntry.val = tpid0;
            rc = cpssPxIngressTpidEntrySet(devNum,tpidIndex,&tpidEntry);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;

        rc = cpssPxIngressPacketTypeKeyEntryGet(devNum, packetType, &keyData, &keyMask);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryGet", rc);
        if (rc != GT_OK)
            return rc;

        keyMask.profileIndex = SRC_PORT_PROFILE_BMP_FRONT_PANEL_E |
                               frontPanel_appDemoToCpssTrafficType[entryIndex].srcProfileId;
        keyData.profileIndex = keyMask.profileIndex;

        tpid1 = tpidTable[frontPanel_appDemoToCpssTrafficType[entryIndex].tpid1Index];

        keyData.udbPairsArr[0].udb[0] = 0;
        keyData.udbPairsArr[0].udb[1] = 0;
        switch(frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType)
        {
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
                keyData.etherType = tpid0;
                keyData.udbPairsArr[0].udb[0] = (GT_U8)(tpid1 >> 8);
                keyData.udbPairsArr[0].udb[1] = (GT_U8)(tpid1 >> 0);
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
                keyData.etherType = tpid0;
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E:
                keyData.etherType = tpid1;
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            default:
                break;
        }

        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/* update :
PIPE_PORT_INFO_STC::tpid0profile
PIPE_PORT_INFO_STC::tpid1profile
*/
static GT_STATUS appDemoPortTpidProfileUpdate(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_U8   devNum = bc3_devNum;
    GT_PORT_NUM portNum;
    CPSS_ETHER_MODE_ENT  ethMode;

    for(ethMode = CPSS_VLAN_ETHERTYPE0_E ;
        ethMode <= CPSS_VLAN_ETHERTYPE1_E ;
        ethMode++)
    {
        for (portNum = 0; portNum < 12; portNum++)
        {
            rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum,
                    portNum + internal_infoPtr->startPhysicalPortNumber,
                    ethMode,
                    GT_TRUE,/*isDefaultProfile*/
                    &internal_infoPtr->portInfo[portNum].tpidProfile[ethMode-1]);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanPortIngressTpidProfileGet", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/* update :
static GT_U16   tpidTable[8] = {
static GT_U8   tpidProfileBmp[2][8]
*/
static GT_STATUS appDemoGlobalTpidInfoGet(void)
{
    GT_STATUS   rc;
    GT_U8       devNum = bc3_devNum;
    CPSS_ETHER_MODE_ENT  ethMode;
    GT_U32      ii;
    GT_U32      tpidBmp;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr;

    /* update the DB according to the DX configurations */
    for(ethMode = CPSS_VLAN_ETHERTYPE0_E ;
        ethMode <= CPSS_VLAN_ETHERTYPE1_E ;
        ethMode++)
    {
        for(ii = 0 ; ii < 8 ; ii++)
        {
            rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum,
                ii,ethMode,&tpidBmp);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanIngressTpidProfileGet", rc);
                return rc;
            }
            tpidProfileBmp[ethMode-1][ii] = (GT_U8)tpidBmp;
        }
    }

    for(ii = 0 ; ii < 8 ; ii++)
    {
        rc = cpssDxChBrgVlanTpidEntryGet(devNum,
            CPSS_DIRECTION_INGRESS_E,ii,&tpidTable[ii]);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgVlanTpidEntryGet", rc);
            return rc;
        }
    }


    for(ii = 0 ; cascadePorts[ii] != GT_NA ; ii++)
    {
        internal_infoPtr = getPipeDevInfo_byDxCascadePort(cascadePorts[ii]);
        if(internal_infoPtr == NULL)
        {
            /* support missing device */
            continue;
        }

        rc = appDemoPortTpidProfileUpdate(internal_infoPtr);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortTpidProfileUpdate", rc);
            return rc;
        }
    }

    return GT_OK;
}

/* update the pipe configurations due to change in the 'TPID' configurations in the DX .*/
static GT_STATUS gtDbDxPxBc3PipesPipeTpidUpdate_singlePipe(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * specific_internal_infoPtr/* ignored if NULL*/
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr;

/* update
    static GT_U16   tpidTable[8]
    static GT_U8   tpidProfileBmp[2][8]
*/
    rc = appDemoGlobalTpidInfoGet();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoGlobalTpidInfoGet", rc);
        return rc;
    }

    for(ii = 0 ; cascadePorts[ii] != GT_NA ; ii++)
    {
        internal_infoPtr = getPipeDevInfo_byDxCascadePort(cascadePorts[ii]);
        if(internal_infoPtr == NULL)
        {
            /* support missing device */
            continue;
        }

        if(specific_internal_infoPtr &&
           specific_internal_infoPtr != internal_infoPtr)
        {
            /* support insertion of specific device */
            continue;
        }

        rc = pipeTpidUpdate_portProfiles(internal_infoPtr);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeTpidUpdate_portProfiles", rc);
            return rc;
        }

        rc = pipeTpidUpdate(internal_infoPtr);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeTpidUpdate", rc);
            return rc;
        }
    }

    return GT_OK;
}

/* update the pipe configurations due to change in the 'TPID' configurations in the DX .*/
GT_STATUS gtDbDxPxBc3PipesPipeTpidUpdate(void)
{
    return gtDbDxPxBc3PipesPipeTpidUpdate_singlePipe(NULL);
}


/* set the ingress and PHA tables to forward packets from 'front panel' to 'uplink'
   + adding 'FRW' eDSA (4 words) with 'src port' info.*/
static GT_STATUS pipeFrontPanelPortsSet(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM   devNum = internal_infoPtr->cpssDevNum;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                entryIndex;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        srcPortInfo;
    GT_U32  dsaPortNum;
    GT_U32  tpidIndex;
    GT_U16  tpid;
    CPSS_PX_PACKET_TYPE     packetType;
    CPSS_PX_INGRESS_TPID_ENTRY_STC  tpidEntry;

    tpidEntry.size = 2; /* BUG in CPSS (CPSS-8765 : PIPE : cpssPxIngressTpidEntrySet use 'size' as '2 bytes' resolution instead of '1 byte')
                           TPID size resolution is 2B. */
    tpidEntry.valid = GT_TRUE;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    portKey.srcPortProfile = SRC_PORT_PROFILE_FRONT_PANEL_E;
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    portKey.portUdbPairArr[0].udbByteOffset = 16;/* start of Tag1 after Tag0 (relevant to double tagging) */

    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /**************************************/
    /* define and enable the traffic type */
    /**************************************/

    /* define 'traffic type' will match the src port profile */
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    keyMask.profileIndex = SRC_PORT_PROFILE_FRONT_PANEL_E;/* bit 0 */
    keyData.profileIndex = SRC_PORT_PROFILE_FRONT_PANEL_E;

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        tpidIndex = frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex;
        tpid = tpidTable[tpidIndex];

        if(tpidIndex < 4)
        {
            tpidEntry.val = tpid;
            rc = cpssPxIngressTpidEntrySet(devNum,tpidIndex,&tpidEntry);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        keyMask.etherType = 0xFFFF;
        keyData.etherType = tpid;

        keyMask.udbPairsArr[0].udb[0] = 0;
        keyMask.udbPairsArr[0].udb[1] = 0;

        keyData.udbPairsArr[0].udb[0] = 0;
        keyData.udbPairsArr[0].udb[1] = 0;

        switch(frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType)
        {
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
                keyMask.udbPairsArr[0].udb[0] = 0xFF;
                keyMask.udbPairsArr[0].udb[1] = 0xFF;

                keyData.udbPairsArr[0].udb[0] = (GT_U8)(tpid>>8);
                keyData.udbPairsArr[0].udb[1] = (GT_U8)(tpid>>0);
                break;

            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
                keyMask.etherType = 0;
                keyData.etherType = 0;
                break;

            default:
                break;
        }

        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
        if (rc != GT_OK)
            return rc;


        /* validate the entry to allow match ... otherwise we get DROP */
        rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*************************************************************/

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* state that 'front panel' can forward packet only to the uplink ports */
    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E;
    entryIndex = PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX;

    rc = cpssPxIngressPortMapEntrySet(devNum, tableType, entryIndex, uplinkPortsBmp, GT_FALSE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Destination table)", rc);
    if (rc != GT_OK)
        return rc;

    packetTypeFormat.indexMax = 1;/* 0 bits */
    packetTypeFormat.indexConst = PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX;

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType,packetType,&packetTypeFormat);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;
    }

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        /* disable SRC filtering ! ... not needed */
        rc = cpssPxIngressFilteringEnableSet(devNum, CPSS_DIRECTION_EGRESS_E,
            packetType, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressFilteringEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*********************************/
    /* set the egress PHA processing */
    /*********************************/

    /* the traffic ingress from the 'front panel' ports :
       state that this traffic that egress the uplink ports treated by the :
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
    */

    cpssOsMemSet(&operationInfo,0,sizeof(operationInfo));
    operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE;/* will be set in next logic */
    operationInfo.info_edsa_E2U.eDsaForward.hwSrcDev = 0;/* ignored any way by the DX device */
    operationInfo.info_edsa_E2U.eDsaForward.tpIdIndex = 0;/* TBD */
    operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE;/* will be set in next logic */

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        operationInfo.info_edsa_E2U.eDsaForward.tpIdIndex = frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex;

        switch(frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType)
        {
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_TRUE;  /*tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_TRUE;  /*tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_TRUE;  /*tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE; /*no tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE; /*no tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_TRUE;  /*tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            default:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE; /*no tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE; /*no tag1*/
                break;
        }

        for (portNum = 12; portNum < 16; portNum++)
        {
            PORT_SKIP_CHECK(devNum,portNum);
            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,
                portNum,
                frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType,/*packetType*/
                frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType,   /*operationType*/
                &operationInfo);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressHeaderAlterationEntrySet", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    cpssOsMemSet(&srcPortInfo,0,sizeof(srcPortInfo));

    dsaPortNum = 0;
    /* state that traffic ingress from the 'front pannel' , need next src info :
       The source ePort number is configured for each Extended Port.
       It is used to assign the source port number into the eDSA tag */
    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        srcPortInfo.info_dsa.srcPortNum = dsaPortNum;

        rc = cpssPxEgressSourcePortEntrySet(devNum,
            portNum,
            CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E,
            &srcPortInfo
        );
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressSourcePortEntrySet", rc);
        if (rc != GT_OK)
            return rc;

        /* the DXCH device not like 'holes' in the port numbers */
        dsaPortNum++;
    }

    return GT_OK;
}

/* get the 4 bits trg_port from the DSA tag bits 23:19 <Trg Phy Port[4:0]> */
static CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC bitFieldArr[] = {
    /* byteOffset , startBit , numBits */
    {13           , 3        ,  4     }
};


/* set the ingress and PHA tables to forward packets from uplink to specific port
    in 'front panel' and removing the 'FROM_CPU' dsa (single word) */
static GT_STATUS pipeUplinksPortsSet(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM   devNum = internal_infoPtr->cpssDevNum;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    GT_PHYSICAL_PORT_NUM  portNum;
    CPSS_PX_PORTS_BMP     trgPortsBmp;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_PACKET_TYPE     packetType;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    portKey.srcPortProfile = SRC_PORT_PROFILE_FRONT_UPLINK_E;
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    portKey.portUdbPairArr[0].udbByteOffset = 12;/* start of the DSA */

    for (portNum = 12; portNum < 16; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;

        /* state that the port is 'cascade port' that ingress with DSA tag */
        rc = cpssPxIngressPortDsaTagEnableSet(devNum,portNum,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortDsaTagEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /**************************************/
    /* define and enable the traffic type */
    /**************************************/

    /* define 'traffic type' will match the src port profile */
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    keyMask.profileIndex = 0x7F;
    keyMask.udbPairsArr[0].udb[0] = 0xc0;/* the 2 bits of '31:30 Tag Command' */

    packetType = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    keyData.profileIndex = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    keyData.udbPairsArr[0].udb[0] = 0x40;/* the 2 bits of '31:30 Tag Command' = 'FROM_CPU' */

    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
    if (rc != GT_OK)
        return rc;


    /* validate the entry to allow match ... otherwise we get DROP */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_TRUE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
    if (rc != GT_OK)
        return rc;

    /* if the traffic from the uplink will not be matched by the above 'PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E' */
    /* cause it to be dropped by the next index */
    packetType           = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_INVALID_E;
    keyData.profileIndex = BIT_0;

    keyMask.udbPairsArr[0].udb[0] = 0;/* dont care */
    keyData.udbPairsArr[0].udb[0] = 0;/* dont care */

    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
    if (rc != GT_OK)
        return rc;

    /* invalidate the entry to cause DROP */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_FALSE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
    if (rc != GT_OK)
        return rc;

    /*************************************************************/
    /* state that 'uplink' can forward packet to any of the 'front panel' ports */
    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E;

    for(portNum = 0; portNum < 12 ; portNum++)
    {
        trgPortsBmp = 1 << portNum;

        rc = cpssPxIngressPortMapEntrySet(devNum, tableType, portNum + PIPE_UPLINK_DESTINATION_BASE_INDEX, trgPortsBmp, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;
    }

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));
    packetTypeFormat.indexMax   = PIPE_UPLINK_DESTINATION_BASE_INDEX + 12;/*12 ports*/
    packetTypeFormat.indexConst = PIPE_UPLINK_DESTINATION_BASE_INDEX;
    packetTypeFormat.bitFieldArr[0] = bitFieldArr[0];

    packetType = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, packetType, &packetTypeFormat);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Destination table)", rc);
    if (rc != GT_OK)
        return rc;

    /*************************************************************/
    {
        /* disable SRC filtering ! ... not needed */
        rc = cpssPxIngressFilteringEnableSet(devNum, CPSS_DIRECTION_EGRESS_E,
            packetType, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressFilteringEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*********************************/
    /* set the egress PHA processing */
    /*********************************/

    /* the traffic ingress from the uplink ports :
       state that this traffic that egress the 'front panel' ports treated by the :
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E */

    cpssOsMemSet(&operationInfo,0,sizeof(operationInfo));
    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);
        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,
            portNum,
            PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E,
            CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E,
            &operationInfo);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressHeaderAlterationEntrySet", rc);
        if (rc != GT_OK)
            return rc;

    }
#if 0 /* no info needed for traffic ingress/egress from the uplink */
    {
        CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        srcPortInfo;
        CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        trgPortInfo;

        cpssOsMemSet(&srcPortInfo,0,sizeof(srcPortInfo));
        /* state that traffic ingress from the uplink , not need any src info */
        for (portNum = 12; portNum < 16; portNum++)
        {
            PORT_SKIP_CHECK(devNum,portNum);
            rc = cpssPxEgressSourcePortEntrySet(devNum,
                portNum,
                CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E,
                &srcPortInfo
            );
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressSourcePortEntrySet", rc);
            if (rc != GT_OK)
                return rc;
        }

        cpssOsMemSet(&trgPortInfo,0,sizeof(trgPortInfo));
        /* state that traffic egress from the uplink , (not really need info from) trg port info*/
        for (portNum = 12; portNum < 16; portNum++)
        {
            PORT_SKIP_CHECK(devNum,portNum);
            rc = cpssPxEgressTargetPortEntrySet(devNum,
                portNum,
                CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E,
                &trgPortInfo
            );
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressTargetPortEntrySet", rc);
            if (rc != GT_OK)
                return rc;
        }
    }
#endif


    return GT_OK;
}

/* set the ingress and PHA tables to forward packets from 'front panel' to 'uplink'
   + adding 'FRW' eDSA (4 words) with 'src port' info.
   and from uplink to specific port in 'front panel' and removing the 'FROM_CPU' dsa (single word) */
static GT_STATUS pipeDsaSystemSet(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_STATUS   rc;
    GT_SW_DEV_NUM   devNum = internal_infoPtr->cpssDevNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PORTS_BMP   fullPortsBmp;
    GT_U32    entryIndex;

    /* set the ingress ports into 2 categories:
       1. 'front panel'
       2. 'uplink'
    */

    rc = pipeFrontPanelPortsSet(internal_infoPtr);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeFrontPanelPortsSet", rc);
        return rc;
    }

    rc = pipeUplinksPortsSet(internal_infoPtr);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeUplinksPortsSet", rc);
        return rc;
    }

    fullPortsBmp = uplinkPortsBmp | frontPannelPortsBmp;

    /* designated port : no trunk members */
    for(entryIndex = 0 ; entryIndex < 128 ; entryIndex++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,fullPortsBmp);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressHashDesignatedPortsEntrySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* disable forwarding to all ports. Event handler will enable forwarding for
       connected ports with link UP. */
    for (portNum = 0; portNum < PRV_CPSS_PX_CPU_DMA_NUM_CNS; portNum++)
    {
        rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortTargetEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/* do init PIPE configurations */
static GT_STATUS internal_initConnectionToPipe_px(
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS rc;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr = getPipeDevInfo_byDxCascadePort(cascadePortNum);
    GT_SW_DEV_NUM   devNum;
    GT_U8   devIndex;

    if(internal_infoPtr == NULL)
    {
        /* support missing device */
        return GT_OK;
    }

    devNum =  internal_infoPtr->cpssDevNum;

    /* sanity check */
    devIndex = internal_infoPtr->appDemoDevIndex;
    if(GT_FALSE == appDemoPpConfigList[devIndex].valid)
    {
        rc = GT_NOT_FOUND;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("PIPE device not found in AppDemo DB", rc);
        return rc;
    }
    else
    if(CPSS_CHANNEL_SMI_E != appDemoPpConfigList[devIndex].channel)
    {
        /* on this board the device expected to be SMI connected to the CPU */
        rc = GT_BAD_STATE;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("PIPE device not stated as 'SMI' interface in AppDemo DB", rc);
        return rc;
    }

    /* skip calling to px_forwardingTablesStage(...)
       from gtDbPxPipeBoardReg_DeviceSimpleInit(...) */
    px_forwardingTablesStage_enabled = GT_FALSE;

    /* must update the 'to be used' devNum in the PIPE initialization. */
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);

    /* actually calling gtDbPxPipeBoardReg_DeviceSimpleInit(...) */
    rc = pipe_genericBoardConfigFun.deviceSimpleInit(
        internal_infoPtr->appDemoDevIndex,
        PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("deviceSimpleInit on PIPE", rc);
        return rc;
    }

    if(!PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        /* some configurations not hold the A0 options.
           see use of PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC() in file gtDbPxPipe.c
           that not copied to current file */
        return GT_NOT_SUPPORTED;
    }

    rc = pipeDsaSystemSet(internal_infoPtr);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeDsaSystemSet", rc);
        return rc;
    }

    if(isAfterEventsInit == GT_TRUE)
    {
        rc = appDemoDeviceEventHandlerPreInit(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventHandlerPreInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* spawn the user event handler processes */
        rc = appDemoDeviceEventRequestDrvnModeInit(internal_infoPtr->appDemoDevIndex);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventRequestDrvnModeInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* the next functions that called during cpssInitSystem for the DX device
           will also init this PIPE device
            1. appDemoEventHandlerPreInit
            2. appDemoEventRequestDrvnModeInit
        */
    }

    /* the appDemo use this for devices iterator */
    appDemoPpConfigDevAmount++;

    return GT_OK;
}

/* do clean-up of PIPE devices that failed to be initialized properly
   (probably due to SMI connection).

   for robustness of initialization , we need to 'recover' this state , and allow
   the system to continue to initialized.

   the 'failed SMI device' can be later 'cpssPpInsert' maybe in other SMI frequency ,
   or ignored because hold bad/unstable SMI connection.
*/
static GT_STATUS    pipe_devInitFailed_doCleanUp(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U8  devIdx;

    devNum = internal_infoPtr->cpssDevNum;
    devIdx = internal_infoPtr->appDemoDevIndex;

    osPrintf("failed to init PIPE devNum [%d] on SMI [%d] ... doing 'remove device' and clean the DB \n",
        devNum,
        internal_infoPtr->busBaseAddr);

    /* clean-up the fail : from CPSS */
    (void)cpssPxCfgDevRemove(devNum);

    /* clean-up the fail : from the DX device */
    /* clean-up the fail : from this file DB */
    (void)gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb(devIdx,saved_boardRevId);

    /* clean-up the fail : from appDemoPpConfigList[] */
    appDemoPpConfigList[devIdx].valid = GT_FALSE;

    /* allow the initialization of BC3 to continue as there is no fail !! */
    /* allow the device to be inserted later (maybe in other SMI frequency) ,
        or just ignoring this device ... due to bad SMI connection to it ! */
    return GT_OK;
}

/* based on : initConnectionTo88e1690(...) */
static GT_STATUS initConnectionToPipe(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum
)
{
    GT_STATUS rc;

    rc = internal_initConnectionToPipe_dxch(devNum,cascadePortNum);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_dxch", rc);
        return rc;
    }

    rc = internal_initConnectionToPipe_px(cascadePortNum);
    if(rc != GT_OK)
    {
        PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr = getPipeDevInfo_byDxCascadePort(cascadePortNum);

        (void)pipe_devInitFailed_doCleanUp(internal_infoPtr);

        /* allow the initialization of BC3 to continue as there is no fail !! */
        /* allow the device to be inserted later (maybe in other SMI frequency) ,
            or just ignoring this device ... due to bad SMI connection to it ! */
    }

    return GT_OK;
}

/*******************************************************************************
* pipe_treatLinkStatusChange
*
* DESCRIPTION:
*       function called for 'link status change'
*       the Pipe device need to notify the DX device that the MAC of the port
*       hold link status change.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port
*       linkUp     - indication that the port is currently 'up'/'down'
*                   relevant only to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E'
*       partIndex  - indication of calling before main logic or after main logic.
*
* OUTPUTS:
*       stopLogicPtr - (pointer to) indication to stop the logic after CB ended.
*                      NOTE: relevant to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E'
*                       GT_TRUE - stop the logic after CB ended.
*                            meaning caller will not call the main logic
*                       GT_TRUE - continue the logic after CB ended.
*                            meaning caller call the main logic
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS pipe_treatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
    OUT GT_BOOL     *stopLogicPtr
)
{
    GT_STATUS   rc;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr;
    GT_U32                  dxPortNum;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT old_portLinkStatusState;

    *stopLogicPtr = GT_FALSE;
    if(!PRV_CPSS_PP_MAC(devNum))
    {
        /* support calling from gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb(...)
           after the PX device was removed ! */

        /* the function getPipeDevInfo_byCpssDevNum() will check that this was valid PX device */
    }
    else
    if(!PRV_CPSS_PX_FAMILY_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    if(partIndex == APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E)
    {
        return GT_OK;
    }

    internal_infoPtr = getPipeDevInfo_byCpssDevNum(devNum);
    if(internal_infoPtr == NULL)
    {
        /* support missing device */
        return GT_OK;
    }

    if(portNum >= 16)
    {
        rc = GT_NO_SUCH;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("unexpected portNumber", rc);
        return rc;
    }

    if(GT_FALSE == internal_infoPtr->portInfo[portNum].isPortMappedAsRemote)
    {
        /* the port is not mapped (like the cascade port #12..15) */
        return GT_OK;
    }
    dxPortNum = internal_infoPtr->portInfo[portNum].remotePortNumberOnDx;

    if(portMgr)
    {
        CPSS_PORT_MANAGER_STATUS_STC portStage;
        CPSS_PORT_MANAGER_STC portEventStc;

        rc = cpssDxChPortManagerStatusGet(bc3_devNum, dxPortNum, &portStage);

        if(rc != GT_OK)
        {
            cpssOsPrintf("calling PortManagerStatusGet returned=%d, portNum=%d", rc, dxPortNum);
            return rc;
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
            portStage.portUnderOperDisable == GT_TRUE)
        {
            cpssOsPrintf("port is in HW failure state or in disable so abort link change notification");
            return GT_OK;
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E )
        {
            return GT_OK;
        }

        portEventStc.portEvent = linkUp? CPSS_PORT_MANAGER_EVENT_ENABLE_E:CPSS_PORT_MANAGER_EVENT_DISABLE_E;

        rc = cpssDxChPortManagerEventSet(bc3_devNum, dxPortNum, &portEventStc);
        if(rc != GT_OK)
        {
            cpssOsPrintf("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
            return rc;
        }
        return GT_OK;
    }
    /* the remote physical port number on the DX that represents the PIPE port */

    rc = cpssDxChBrgEgrFltPortLinkEnableGet(bc3_devNum, dxPortNum, &old_portLinkStatusState);
    if(rc != GT_OK)
    {
        return rc;
    }

    portLinkStatusState =  linkUp ?
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    if(old_portLinkStatusState == portLinkStatusState)
    {
        /* avoid 'printings' of ports when no change in value */
        return GT_OK;
    }

    /* the port on the PIPE changed status , we need to notify the relevant DX device */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(bc3_devNum, dxPortNum, portLinkStatusState);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(appDemoPrintLinkChangeFlagGet())
    {
        cpssOsPrintSync("LINK %s : devNum[%ld] (remote physical port[%2ld]) \n",
                            (linkUp == GT_FALSE) ? "DOWN" : "UP  ",
                                        (GT_U32)bc3_devNum, dxPortNum);
    }
    return GT_OK;
}

/**
* @internal gtDbDxPxBc3Pipes_cpssDxChTrunkMemberRemove_WA_srcFilter function
* @endinternal
*
* @brief   part of trunk WA due to no src-filter of stacking
*           using the port isolation for the src-filter of trunk members
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunkId
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtDbDxPxBc3Pipes_cpssDxChTrunkMemberRemove_WA_srcFilter
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC srcInterface;
    GT_HW_DEV_NUM   localDev_hwDevNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    if(memberPtr->hwDevice == localDev_hwDevNum)
    {
        srcInterface.type = CPSS_INTERFACE_TRUNK_E;
        srcInterface.trunkId = trunkId;
        if(memberPtr->hwDevice != localDev_hwDevNum)
        {
            rc = cpssDxChNstPortIsolationPortAdd(devNum,trunkId,
                &srcInterface,
                memberPtr->port);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* NOTE: application may not want to disable the port isolation */
            rc = cpssDxChNstPortIsolationModeSet(devNum,
                memberPtr->port,
                CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return cpssDxChTrunkMemberRemove(devNum,trunkId,memberPtr);

}
/**
* @internal appDemo_cpssDxChTrunkMemberAdd_WA_srcFilter function
* @endinternal
*
* @brief   part of trunk WA due to no src-filter of stacking
*           using the port isolation for the src-filter of trunk members
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunkId
* @param[in] memberPtr                - (pointer to) the trunk member
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtDbDxPxBc3Pipes_cpssDxChTrunkMemberAdd_WA_srcFilter
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC srcInterface;
    GT_HW_DEV_NUM   localDev_hwDevNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    localDev_hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    if(memberPtr->hwDevice == localDev_hwDevNum)
    {
        srcInterface.type = CPSS_INTERFACE_TRUNK_E;
        srcInterface.trunkId = trunkId;
        if(memberPtr->hwDevice != localDev_hwDevNum)
        {
            rc = cpssDxChNstPortIsolationPortDelete(devNum,trunkId,
                &srcInterface,
                memberPtr->port);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChNstPortIsolationModeSet(devNum,
                memberPtr->port,
                CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return cpssDxChTrunkMemberAdd(devNum,trunkId,memberPtr);

}

/**
* @internal gtDbDxPxBc3Pipes_cpssDxChTrunkMembersSet_WA_srcFilter function
* @endinternal
*
* @brief   part of trunk WA due to no src-filter of stacking
*           using the port isolation for the src-filter of trunk members
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtDbDxPxBc3Pipes_cpssDxChTrunkMembersSet_WA_srcFilter
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC      old_TrunkPortsBmp,new_TrunkPortsBmp;
    CPSS_PORTS_BMP_STC      old_nonTrunkPorts,new_nonTrunkPorts;
    CPSS_PORTS_BMP_STC      old_portIsolationPorts,new_portIsolationPorts;
    CPSS_PORTS_BMP_STC      fullLocalPortsBmp;
    CPSS_PORTS_BMP_STC      removedLocalPortsBmp;
    CPSS_INTERFACE_INFO_STC srcInterface;
    GT_BOOL                 cpuPortMember;
    GT_U32                  ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* get the 'old' 'non-ports' of the trunk */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&old_nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChTrunkMembersSet(devNum,trunkId,
        numOfEnabledMembers,enabledMembersArray,
        numOfDisabledMembers,disabledMembersArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the 'new' 'non-ports' of the trunk */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&new_nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(CPSS_PORTS_BMP_ARE_EQUAL_MAC(&old_nonTrunkPorts,&new_nonTrunkPorts))
    {
        /* nothing to update */
        return GT_OK;
    }


    CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&fullLocalPortsBmp);
    /* adjusted physical ports of BMP to hold only bmp of existing ports*/
    rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , &fullLocalPortsBmp , &fullLocalPortsBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    srcInterface.type = CPSS_INTERFACE_TRUNK_E;
    srcInterface.trunkId = trunkId;

    rc = cpssDxChNstPortIsolationTableEntryGet(devNum,
        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,
        &srcInterface,
        &cpuPortMember,
        &old_portIsolationPorts); /* block the trunk members */
    if(rc != GT_OK)
    {
        return rc;
    }


    /* new_TrunkPortsBmp = fullLocalPortsBmp & ~(new_nonTrunkPorts) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&new_TrunkPortsBmp,
                                       &fullLocalPortsBmp,
                                       &new_nonTrunkPorts);
    /* old_TrunkPortsBmp = fullLocalPortsBmp & ~(new_nonTrunkPorts) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&old_TrunkPortsBmp,
                                       &fullLocalPortsBmp,
                                       &old_nonTrunkPorts);



    /* new_portIsolationPorts = (old_portIsolationPorts | old_TrunkPortsBmp) & ~(new_TrunkPortsBmp); */

    /* restore old members to the BMP */
    CPSS_PORTS_BMP_BITWISE_OR_MAC(&new_portIsolationPorts,&old_portIsolationPorts,&old_TrunkPortsBmp);
    /* remove new members from the BMP */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&new_portIsolationPorts,&new_portIsolationPorts,&new_TrunkPortsBmp);

    rc = cpssDxChNstPortIsolationTableEntrySet(devNum,
        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,
        &srcInterface,
        cpuPortMember,
        &new_portIsolationPorts); /* block the trunk members */
    if(rc != GT_OK)
    {
        return rc;
    }


    /* removedLocalPortsBmp = old_TrunkPortsBmp & ~(new_TrunkPortsBmp) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&removedLocalPortsBmp,
                                       &old_TrunkPortsBmp,
                                       &new_TrunkPortsBmp);

    for(ii = 0 ; ii < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); ii++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&removedLocalPortsBmp,ii))
        {
            rc = cpssDxChNstPortIsolationModeSet(devNum,
                ii,
                CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&new_TrunkPortsBmp,ii))
        {
            rc = cpssDxChNstPortIsolationModeSet(devNum,
                ii,
                CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal gtDbDxPxBc3Pipes_cpssDxChTrunkCatchUp_WA_srcFilter function
* @endinternal
*
* @brief   function to allow to update the port isolation with the members of the trunk.
*           (after the trunk is already set by the configuration)
*           this is debug function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - the trunkId
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtDbDxPxBc3Pipes_cpssDxChTrunkCatchUp_WA_srcFilter
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC      nonTrunkPorts;
    CPSS_PORTS_BMP_STC      portIsolationPorts;
    CPSS_INTERFACE_INFO_STC srcInterface;
    GT_BOOL                 cpuPortMember;
    GT_U32                  ii;

    /* get the 'non-ports' of the trunk */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    srcInterface.type = CPSS_INTERFACE_TRUNK_E;
    srcInterface.trunkId = trunkId;

    rc = cpssDxChNstPortIsolationTableEntryGet(devNum,
        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,
        &srcInterface,
        &cpuPortMember,
        &portIsolationPorts);
    if(rc != GT_OK)
    {
        return rc;
    }


    rc = cpssDxChNstPortIsolationTableEntrySet(devNum,
        CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,
        &srcInterface,
        cpuPortMember,
        &nonTrunkPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0 ; ii < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); ii++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&nonTrunkPorts,ii))
        {
            rc = cpssDxChNstPortIsolationModeSet(devNum,
                ii,
                CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }



    return GT_OK;
}

/* rescan the SMI and update the appDemoPpConfigList[] with new PIPEs */
/* return BMP of new SMI addresses used by the devices */
static GT_U32 gtDbDxPxBc3PipesSmiBoard_rescan_smi(void)
{
    GT_STATUS rc = GT_OK;
    GT_U8   devIdx;
    GT_BOOL isFirstSmiDev = GT_TRUE;
    GT_SMI_INFO smiInfo;
    PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr;
    GT_U32  numNewDevices = 0;
    GT_U32  smiNewDevicesBmp = 0;
    GT_BOOL deviceAlreadyExists;

    while(rc == GT_OK)
    {
        rc = gtPresteraGetSmiDev(isFirstSmiDev, &smiInfo);
        if(rc != GT_OK)
        {
            /* no more devices on the SMI */
            break;
        }

        isFirstSmiDev = GT_FALSE;

        internal_infoPtr = getPipeDevInfo_bySmiAddr(smiInfo.smiIdSel);
        if(internal_infoPtr == NULL)
        {
            /* this SMI device is not relevant */
            continue;
        }

        if(internal_infoPtr->isExists == GT_TRUE)
        {
            /* this device is already configured */
            continue;
        }

        devIdx = internal_infoPtr->appDemoDevIndex;
        deviceAlreadyExists = GT_FALSE;

        for( ; appDemoPpConfigList[devIdx].valid == GT_TRUE &&
               devIdx < APP_DEMO_PP_CONFIG_SIZE_CNS;
             devIdx++)
        {
            /* the index is in use .... is it 'me' ? */
            if(smiInfo.smiIdSel == appDemoPpConfigList[devIdx].smiInfo.smiIdSel)
            {
                deviceAlreadyExists = GT_TRUE;
                break;
            }
        }

        if(deviceAlreadyExists == GT_TRUE)
        {
            continue;
        }

        osPrintf("Recognized 'new' device on SMI[%d] (added to appDemoPpConfigList[%d])\n",
            smiInfo.smiIdSel,
            devIdx);

        internal_infoPtr->appDemoDevIndex = devIdx;

        appDemoPpConfigList[devIdx].devFamily = CPSS_PX_FAMILY_PIPE_E;
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_SMI_E;
        appDemoPpConfigList[devIdx].devNum  = CAST_SW_DEVNUM(internal_infoPtr->cpssDevNum);
        appDemoPpConfigList[devIdx].valid = GT_TRUE;
        osMemCpy(&(appDemoPpConfigList[devIdx].smiInfo),
                 &smiInfo,
                 sizeof(GT_SMI_INFO));

        smiNewDevicesBmp |= 1 << smiInfo.smiIdSel;
        numNewDevices++;
    }

    if(numNewDevices == 0)
    {
        osPrintf("Not Recognized 'new' devices on SMI \n");
    }
    else
    if(numNewDevices == 1)
    {
        osPrintf("Totally Recognized single 'new' device on SMI \n");
    }
    else
    {
        osPrintf("Totally Recognized 'new' [%d] devices on SMI \n",
            numNewDevices);
    }

    return smiNewDevicesBmp;
}

static GT_STATUS gtDbDxPxBc3PipesSmiBoard_newPipeDevice(
    IN PIPE_SMI_REMOTE_DEV_INFO_STC * internal_infoPtr,
    IN GT_SW_DEV_NUM        newDevNum
)
{
    GT_STATUS   rc;
    GT_U32  physicalPortNumber,remoteMacNumber;
    GT_U32  ii;
    GT_U32  dxCascadePortNum = internal_infoPtr->dxCascadePortNum;
    APP_DEMO_PORT_MAP_STC * dx_internal_infoPtr = getCascadePortInfo(dxCascadePortNum);

    internal_infoPtr->isExists = GT_TRUE;
    if(newDevNum != 0xFFFFFFFF)
    {
        internal_infoPtr->cpssDevNum = newDevNum;
        /* update also the appDemoPpConfigList[] */
        appDemoPpConfigList[internal_infoPtr->appDemoDevIndex].devNum  = CAST_SW_DEVNUM(newDevNum);
    }

    for(ii = 0 ; ii < dx_internal_infoPtr->numOfPorts ; ii++)
    {
        physicalPortNumber = dx_internal_infoPtr->startPhysicalPortNumber + ii;
        remoteMacNumber    = dx_internal_infoPtr->startRemoteMacNumber    + ii;
        internal_infoPtr->portInfo[remoteMacNumber].isPortMappedAsRemote = GT_TRUE;
        internal_infoPtr->portInfo[remoteMacNumber].remotePortNumberOnDx = physicalPortNumber;
    }

    rc = internal_initConnectionToPipe_px(dxCascadePortNum);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_px", rc);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* gtDbDxPxBc3PipesSmiBoard_convertDxPortToPxPort
*
* DESCRIPTION:
*       function called by 'test' to check if for this port may need to use
*       'PX' API , with 'PX device' and 'PX portNum'.
*       for function like : cpssDxChPortLinkStatusGet() on remote port should
*       actually replaced with call to --> cpssPxPortLinkStatusGet()
*
* INPUTS:
*       devNum     - DXCH physical device number
*       portNum    - DXCH physical port (remote physical port)
*
* OUTPUTS:
*       isConvertedPtr - (pointer to)indication that the 'conversion' needed or not.
*       isPxApiPtr     - (pointer to)indication that the 'conversion' is to PX or DX.
*       convertedDevNumPtr - (pointer to)the converted devNum
*       convertedPortNumPtr - (pointer to)the converted portNum

*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS gtDbDxPxBc3PipesSmiBoard_convertDxPortToPxPort
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *isConvertedPtr,
    OUT GT_BOOL                 *isPxApiPtr,
    OUT GT_U8                   *convertedDevNumPtr,
    OUT GT_PHYSICAL_PORT_NUM    *convertedPortNumPtr
)
{
    GT_U32  ii,jj;

    if(isConvertedPtr)
    {
        *isConvertedPtr = GT_FALSE;
    }
    if(isPxApiPtr)
    {
        *isPxApiPtr     = GT_FALSE;
    }
    if(convertedDevNumPtr)
    {
        *convertedDevNumPtr  = devNum;
    }
    if(convertedPortNumPtr)
    {
        *convertedPortNumPtr = portNum;
    }

    if(devNum != bc3_devNum)
    {
        return GT_OK;
    }

    for(ii = 0 ; pipesInfo[ii] ;ii++)
    {
        if(pipesInfo[ii]->isExists == GT_FALSE)
        {
            continue;
        }
        if(portNum < pipesInfo[ii]->startPhysicalPortNumber)
        {
            continue;
        }
        if(portNum >= (pipesInfo[ii]->startPhysicalPortNumber + pipesInfo[ii]->numOfRemotePorts))
        {
            continue;
        }

        for(jj = 0 ; jj < 12; jj++)
        {
            if(pipesInfo[ii]->portInfo[jj].isPortMappedAsRemote == GT_FALSE)
            {
                /* no conversion ! */
                continue;
            }

            if(pipesInfo[ii]->portInfo[jj].remotePortNumberOnDx == portNum)
            {
                if(isConvertedPtr)
                {
                    /* converted */
                    *isConvertedPtr = GT_TRUE;
                }
                if(isPxApiPtr)
                {
                    /* use PX APIs for 'L1' functions */
                    *isPxApiPtr     = GT_TRUE;
                }
                if(convertedDevNumPtr)
                {
                    *convertedDevNumPtr  = CAST_SW_DEVNUM(pipesInfo[ii]->cpssDevNum);
                }
                if(convertedPortNumPtr)
                {
                    *convertedPortNumPtr = jj;
                }
                /* converted */
                return GT_OK;
            }
        }
    }

    /* no conversion */
    return GT_OK;
}

/**
* @internal gtDbDxPxBc3PipesSmiBoardReg function
* @endinternal
*
* @brief   Registration function for : the BC3 (PEX) and 2 pipes (SMI)
*                                     (multiple boards).
*       NOTE: in WM this is single board.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxPxBc3PipesSmiBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    GT_STATUS   rc;
    APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT mode;

    saved_boardRevId = boardRevId;

    if(boardCfgFuncs == NULL)
    {
        rc = GT_FAIL;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("NULL pointer", rc);
        return rc;
    }

    /* the board that we work with requires normal (slow) SMI frequency
       (not fast and not accelerated)

       board of BC3 on PEX + 2*PIPEs on SMI

       we 'save' here value to be used by gtPresteraGetSmiDev(...)
    */
    mode = appDemoSoCMDCFrequency_getFromDb();
    if(mode > APP_DEMO_SOC_MDC_FREQUENCY_MODE_ACCELERATED_E) /* was not stated before the cpssInitSystem(...) */
    {
        /* state the we need to use 'normal mode' -- this is what used in 'MRVL PTK' */
        appDemoSoCMDCFrequency_setInDb(APP_DEMO_SOC_MDC_FREQUENCY_MODE_NORMAL_E);
    }

    /* save BC3 callback functions */
    rc = gtDbDxBobcat2BoardReg(genericBoardRevision,&genericBoardConfigFun);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtDbDxBobcat2BoardReg", rc);
        return rc;
    }

    rc = gtDbPxPipeBoardReg(PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID, &pipe_genericBoardConfigFun);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtDbPxPipeBoardReg", rc);
        return rc;
    }

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardGetInfo                 = getBoardInfo;
    boardCfgFuncs->boardGetPpPh1Params          = getPpPhase1Config;
    boardCfgFuncs->boardAfterPhase1Config       = configBoardAfterPhase1;
    boardCfgFuncs->boardGetPpPh2Params          = getPpPhase2Config;
    boardCfgFuncs->boardAfterPhase2Config       = configBoardAfterPhase2;
    boardCfgFuncs->boardGetPpLogInitParams      = getPpLogicalInitParams;
    boardCfgFuncs->boardGetLibInitParams        = getTapiLibInitParams;
    boardCfgFuncs->boardAfterInitConfig         = afterInitBoardConfig;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbDxPxBc3PipesSmiBoardReg_BoardCleanDbDuringSystemReset;

    appDemo_cpssDxChTrunkMemberRemove = gtDbDxPxBc3Pipes_cpssDxChTrunkMemberRemove_WA_srcFilter;
    appDemo_cpssDxChTrunkMemberAdd    = gtDbDxPxBc3Pipes_cpssDxChTrunkMemberAdd_WA_srcFilter;
    appDemo_cpssDxChTrunkMembersSet   = gtDbDxPxBc3Pipes_cpssDxChTrunkMembersSet_WA_srcFilter;

    /********************************************/
    /* HIR support - add/remove of PIPE devices */
    /********************************************/
    isHirApp = GT_TRUE;/* little trick ! to allow to call : cpssPpInsert(...) , cpssPpRemove(...) */
    /* function for remove PIPE device (only set 'not exists' for this device in DB of this file) */
    boardCfgFuncs->deviceCleanup                 = gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb;
    /* function for add PIPE device (do full DB and HW insertion of this device) */
    boardCfgFuncs->DeviceSimpleCpssPpInsert      = gtDbDxPxBc3PipesSmiBoardReg_DeviceSimpleCpssPpInsert;

    /* special link status event from PX hold impact on the DX device */
    appDemoCallBackOnLinkStatusChaneFunc = pipe_treatLinkStatusChange;

    /* as we work over SMI , need to access to it as less that we can have
        support 5 times in a second.(every 200 millisecond)
        the 'default' is 50 times in a second.(every 20 millisecond)
    */
    DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(200);

    /* support for tests */
    appDemoConvertDxPortToPxPortFunc = gtDbDxPxBc3PipesSmiBoard_convertDxPortToPxPort;

    return GT_OK;
}

