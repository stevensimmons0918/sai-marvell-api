/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitialize.c
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include "xpsCommon.h"
#include "openXpsLag.h"
#include <cpssHalProfile.h>

#include <cpssHalSys.h>
#include <cpssHalDev.h>
#include "cpssHalPort.h"
#include <cpssHalExt.h>
#include <cpssHalUtil.h>

#include <stdlib.h>
#include <stdio.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>

#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtOs/gtEnvDep.h>
#include "cpssHalLed.h"

#ifdef ASIC_SIMULATION
#include "cpssHalSim.h"
#include "cpssHalSimPrv.h"
#endif
#include <cpssHalInit.h>
#include "cpssHalDeviceStruct.h"
#include "cpssHalDevice.h"
#include "cpssHalPort.h"
#include "cpssHalConfig.h"
#include "cpssHalPhyConfig.h"

void (*cpssHalLinkStateNotify)(xpsDevice_t devId, uint32_t portNum,
                               int linkStatus);


extern uint32_t eventHandlerTid;
static uint32_t portEvntTid;
extern cpssHalDeviceCfg globalSwitchDb[MAX_GLOBAL_DEVICES];
extern int cpssHalCurrentSwitchId;
#define CPSSHAL_SWITCH(x)           globalSwitchDb[cpssHalCurrentSwitchId].x

#ifndef RESET_PP_EXCLUDE_PEX
extern GT_STATUS falcon_force_early_check_for_device_not_reset_set(void);
#endif

int gEnableCpssLog = 0;
extern GT_STATUS cpssHalEnableLog(int);
extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(
    IN CPSS_PP_FAMILY_TYPE_ENT devFamily);
extern GT_STATUS prvCpssErrorLogEnableSet(IN GT_BOOL enable);
extern GT_STATUS allow_phase1_fail_without_cleanup_set(GT_U32 allow);
static GT_U32   numCpuSdmas = 0;
void cpssHalResetPp(int devNum, bool resetInterrupt, XP_DEV_TYPE_T devType);
extern GT_CHAR * CPSS_SPEED_2_STR(CPSS_PORT_SPEED_ENT speed);
extern GT_CHAR * CPSS_IF_2_STR(CPSS_PORT_INTERFACE_MODE_ENT ifEnm);
extern GT_CHAR * CPSS_MAPPING_2_STR(CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm);
extern int GetSharedProfileIdx();
extern int GetPbrMaxNum();

static LED_PROFILE_TYPE_E ledProfileMode;

#define MRVL_HAL_API_TRACE(api_name, status) cpssOsPrintf("%s:%d - ERROR: %s returns status: 0x%x\n", __func__, __LINE__, api_name, status)
#define MRVL_HAL_API_NOTICE(api_name, status) cpssOsPrintf("%s:%d - NOTICE: %s returns status: 0x%x\n", __func__, __LINE__, api_name, status)

typedef struct
{
    GT_U32   firstDma;
    GT_U32   numPorts;
} SDMA_RANGE_INFO_STC;

typedef struct
{
    GT_U32  sdmaPort;
    GT_U32  macPort;
} MUX_DMA_STC;
static GT_U32   reservedCpuSdmaGlobalQueue[2] = {0};
static MUX_DMA_STC falcon_12_8_muxed_SDMAs[] = {{272, 258}, {274, 259}, {276, 260}, {278, 261}, {280, 266}, {282, 267}, {284, 268}, {286, 269}, {GT_NA, GT_NA}};
static MUX_DMA_STC falcon_3_2_muxed_SDMAs[]  = {{ 68, 66}, { 70, 67}, {GT_NA, GT_NA}};
static MUX_DMA_STC falcon_6_4_muxed_SDMAs[]  = {{136, 130}, {138, 131}, {140, 132}, {142, 133}, {GT_NA, GT_NA}};

#ifdef RETRY_PP_SOFT_RESET
extern GT_STATUS prvCpssDxChUnitBaseTableGet
(
    IN  GT_U8    devNum,
    OUT GT_U32 **devUnitsBaseAddrPtrPtr
);
#define SMI_MNG_ADDRESS_MARK_CNS 0xFFFFFFAA
typedef struct
{
    GT_U32  regAddr;
    GT_U32  value;
} regsToRestore_t;


regsToRestore_t dev0regsToRestore[] =
{
    /* Unit Default ID (UDID) Register */
    { 0x00000204, 0 },
    /* Window n registers (n=0..5)
     *    Base Address,
     *    Size,
     *    High Address Remap,
     *    Window Control
     */
    { 0x020c, 0 }, { 0x0210, 0 }, { 0x23c, 0}, { 0x0254, 0 },
    { 0x0214, 0 }, { 0x0218, 0 }, { 0x240, 0}, { 0x0258, 0 },
    { 0x021c, 0 }, { 0x0220, 0 }, { 0x244, 0}, { 0x025c, 0 },
    { 0x0224, 0 }, { 0x0228, 0 }, { 0x248, 0}, { 0x0260, 0 },
    { 0x022c, 0 }, { 0x0230, 0 }, { 0x24c, 0}, { 0x0264, 0 },
    { 0x0234, 0 }, { 0x0248, 0 }, { 0x250, 0}, { 0x0258, 0 },
    /* address completion are used by LSP of MSYS for
       eth port PHY managment */
    { 0x0124, 0 }, { 0x0128, 0 }, { 0x012c, 0}, { 0x0130, 0 },
    { 0x0134, 0 }, { 0x0138, 0 }, { 0x013c, 0},
    /* SMI Master controller configuration is used for BobK systems
       for MSYS eth port PHY managment */
    { SMI_MNG_ADDRESS_MARK_CNS, 0 }, /* must be last before {0,0} */
    { 0, 0 }/* END */
};

regsToRestore_t dev1regsToRestore[] =
{
    /* Unit Default ID (UDID) Register */
    { 0x00000204, 0 },
    /* Window n registers (n=0..5)
     *    Base Address,
     *    Size,
     *    High Address Remap,
     *    Window Control
     */
    { 0x020c, 0 }, { 0x0210, 0 }, { 0x23c, 0}, { 0x0254, 0 },
    { 0x0214, 0 }, { 0x0218, 0 }, { 0x240, 0}, { 0x0258, 0 },
    { 0x021c, 0 }, { 0x0220, 0 }, { 0x244, 0}, { 0x025c, 0 },
    { 0x0224, 0 }, { 0x0228, 0 }, { 0x248, 0}, { 0x0260, 0 },
    { 0x022c, 0 }, { 0x0230, 0 }, { 0x24c, 0}, { 0x0264, 0 },
    { 0x0234, 0 }, { 0x0248, 0 }, { 0x250, 0}, { 0x0258, 0 },
    /* address completion are used by LSP of MSYS for
       eth port PHY managment */
    { 0x0124, 0 }, { 0x0128, 0 }, { 0x012c, 0}, { 0x0130, 0 },
    { 0x0134, 0 }, { 0x0138, 0 }, { 0x013c, 0},
    /* SMI Master controller configuration is used for BobK systems
       for MSYS eth port PHY managment */
    { SMI_MNG_ADDRESS_MARK_CNS, 0 }, /* must be last before {0,0} */
    { 0, 0 }/* END */
};

CPSS_HW_INFO_STC dev0HwInfo, dev1HwInfo;

#endif
#define CPSS_HAL_GLOBAL_LAG_MAX_MEMBERS(devNum) (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers/2)
extern GT_STATUS prvCpssDrvEventSupportCheck
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData
);


GT_STATUS falcon_initPortDelete_WA
(
    IN GT_SW_DEV_NUM       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii, jj;
    CPSS_DXCH_PORT_MAP_STC      *currPtr = &portMapArrayPtr[0];
    CPSS_PORTS_BMP_STC          usedPhyPorts;
    MUX_DMA_STC                 *muxed_SDMAs_Ptr;
    SDMA_RANGE_INFO_STC         falcon_3_2_SDMAs_available  = { 64 +  4,  4};
    SDMA_RANGE_INFO_STC         falcon_6_4_SDMAs_available  = {128 +  8,  8};
    SDMA_RANGE_INFO_STC         falcon_12_8_SDMAs_available = {256 + 16, 16};
    SDMA_RANGE_INFO_STC         *SDMAs_available_Ptr;
    CPSS_PORTS_BMP_STC
    availableSDMAPorts;/* note : this is BMP of MAC/DMA ... not of physical ports */
    GT_U32                      maxPhyPorts;
    CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC    waInfo;

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&usedPhyPorts);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&availableSDMAPorts);

    muxed_SDMAs_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ?
                      falcon_12_8_muxed_SDMAs :
                      PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? falcon_6_4_muxed_SDMAs  :
                      /* single tile*/        falcon_3_2_muxed_SDMAs  ;

    SDMAs_available_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ?
                          &falcon_12_8_SDMAs_available :
                          PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ?
                          &falcon_6_4_SDMAs_available  :
                          /* single tile*/        &falcon_3_2_SDMAs_available  ;

    /**********************************************************************/
    /********** start by search for CPU SDMA to be used by the WA *********/
    /**********************************************************************/

    /* build BMP of available SDMA ports that can be used */
    for (ii = SDMAs_available_Ptr->firstDma ;
         ii < SDMAs_available_Ptr->firstDma + SDMAs_available_Ptr->numPorts ;
         ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&availableSDMAPorts, ii);
    }
    for (ii = 0 ; ii < portMapArraySize; ii++, currPtr++)
    {
        /* state that this physical port is used */
        CPSS_PORTS_BMP_PORT_SET_MAC(&usedPhyPorts, currPtr->physicalPortNumber);

        /* for CPU SDMA , find the highest 'physical port number' and still it's 'queue 7' */
        if (currPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* state that this DMA port is used as CPU SDMA */
            /* note : this is BMP of DMAs ... not of physical ports */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, currPtr->interfaceNum);
        }
        else
        {
            for (jj = 0 ; muxed_SDMAs_Ptr[jj].sdmaPort != GT_NA ; jj++)
            {
                if (currPtr->interfaceNum == muxed_SDMAs_Ptr[jj].macPort)
                {
                    /* since the application uses this MAC that is muxed with the SDMA ...
                       it is not free for the WA */
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts,
                                                  muxed_SDMAs_Ptr[jj].sdmaPort);
                    break;
                }
            }
        }
    }

    prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numCpuSdmas);

    if (CPSS_PORTS_BMP_IS_ZERO_MAC(&availableSDMAPorts))
    {
        /* the application uses ALL the SDMAs that the device can offer */
        /* so we will steal the 'queue 7,6' of the 'last one' */
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numCpuSdmas * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numCpuSdmas * 8) - 2;
    }
    else
    {
        /* indication for the CPSS to find SDMA that is not used ! */
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numCpuSdmas * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numCpuSdmas * 8) - 2;
        /*  next not supported (yet) by the CPSS
            waInfo.reservedCpuSdmaGlobalQueue[0]         = 0xFFFFFFFF;
            waInfo.reservedCpuSdmaGlobalQueue[1]         = 0xFFFFFFFF;
        */
    }

    if (waInfo.reservedCpuSdmaGlobalQueue[1] == 7) /* single CPU port */
    {
        waInfo.reservedCpuSdmaGlobalQueue[1] =
            6;/* the '7' is reserved for all 'from cpu' tests (LUA/enh-UT) */
        waInfo.reservedCpuSdmaGlobalQueue[0] = 5;
    }

    /* saved for later use */
    reservedCpuSdmaGlobalQueue[0] = waInfo.reservedCpuSdmaGlobalQueue[0];
    reservedCpuSdmaGlobalQueue[1] = waInfo.reservedCpuSdmaGlobalQueue[1];
    /**********************************************************************/
    /********** start the search for physical port to be used by the WA ***/
    /**********************************************************************/
    for (ii = maxPhyPorts-1 ; ii ; ii--)
    {
        if (ii == CPSS_CPU_PORT_NUM_CNS ||
            ii == CPSS_NULL_PORT_NUM_CNS)
        {
            /* not valid numbers to use ! */
            continue;
        }

        if (0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts, ii))
        {
            /* not used */
            break;
        }
    }

    if (ii == 0)
    {
        cpssOsPrintf((" falcon_initPortDelete_WA : not found free physical port number ?! \n"));
        return GT_FULL;
    }

    waInfo.reservedPortNum = ii;


    /*************************************************/
    /********** State the tail drop profile to use ***/
    /*************************************************/
    waInfo.reservedTailDropProfile = 15;


    /**********************************/
    /* call the CPSS to have the info */
    /**********************************/
    rc = cpssDxChHwPpImplementWaInit_FalconPortDelete(CAST_SW_DEVNUM(devNum),
                                                      &waInfo);
    MRVL_HAL_API_NOTICE("cpssDxChHwPpImplementWaInit_FalconPortDelete", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}




#define GT_NA ((GT_U32)~0)

/* hold database of information read from device that cannot be reread */
typedef struct
{
    GT_U8               devNum;
    CPSS_PP_DEVICE_TYPE devType;
    GT_PCI_INFO         pciInfo;
    GT_BOOL
    Ports[MAX_PORT]; /* port that are enable by cpssHalInitializePortApi */
    GT_U32              numPorts;
} mrvDevInfo;

#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0 {{7,1,6,0,5,3,4,2}}
#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1 {{0,6,1,7,2,4,3,5}}
#define FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE   16

/* Muxing settings are the same for all ports */
static CPSS_PORT_MAC_TO_SERDES_STC
falcon_6_4T_MacToSerdesMap[FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE] =
{
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /* 0-7*/    /*Raven 0 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /* 8-15*/   /*Raven 0 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*16-23*/   /*Raven 1 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*24-31*/   /*Raven 1 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*32-39*/   /*Raven 2 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*40-47*/   /*Raven 2 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*48-55*/   /*Raven 3 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*56-63*/   /*Raven 3 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*64-71*/   /*Raven 4 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*72-79*/   /*Raven 4 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*80-87*/   /*Raven 5 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*88-95*/   /*Raven 5 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*96-103*/  /*Raven 6 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*104-111*/ /*Raven 6 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*112-119*/ /*Raven 7 */
    APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1             /*120-127*/ /*Raven 7 */
};

static CPSS_PORT_MAC_TO_SERDES_STC  falcon_Belly2Belly_MacToSerdesMap[] =
{
    {{0, 7, 2, 4, 1, 5, 3, 6}},  /* 0-7*/ /*Raven 0 */
    {{0, 5, 1, 7, 3, 2, 4, 6}},  /* 8-15*/ /*Raven 0 */
    {{1, 5, 0, 7, 4, 2, 3, 6}},  /*16-23*/ /*Raven 1 */
    {{0, 6, 1, 7, 2, 3, 4, 5}},  /*24-31*/ /*Raven 1 */
    {{0, 6, 2, 7, 1, 5, 4, 3}},  /*32-39*/ /*Raven 2 */
    {{0, 5, 1, 6, 2, 7, 4, 3}},  /*40-47*/ /*Raven 2 */
    {{2, 6, 1, 3, 0, 4, 5, 7}},  /*48-55*/ /*Raven 3 */
    {{1, 6, 2, 7, 0, 4, 3, 5}},  /*56-63*/ /*Raven 3 */
    {{0, 7, 2, 4, 1, 5, 3, 6}},  /*64-71*/ /*Raven 4 */
    {{0, 5, 1, 7, 3, 2, 4, 6}},  /*72-79*/ /*Raven 4 */
    {{1, 5, 0, 7, 4, 2, 3, 6}},  /*80-87*/ /*Raven 5 */
    {{0, 6, 1, 7, 2, 3, 4, 5}},  /*88-95*/ /*Raven 5 */
    {{0, 6, 2, 7, 1, 5, 4, 3}},  /*96-103*/ /*Raven 6 */
    {{0, 5, 1, 6, 2, 7, 4, 3}},  /*104-111*/ /*Raven 6 */
    {{2, 6, 1, 3, 0, 4, 5, 7}},  /*112-119*/ /*Raven 7 */
    {{1, 6, 2, 7, 0, 4, 3, 5}},  /*120-127*/ /*Raven 7 */
    {{0, 7, 2, 4, 1, 5, 3, 6}},  /*128-135*/ /*Raven 8 */
    {{0, 5, 1, 7, 3, 2, 4, 6}},  /*136-143*/ /*Raven 8 */
    {{1, 5, 0, 7, 4, 2, 3, 6}},  /*144-151*/ /*Raven 9 */
    {{0, 6, 1, 7, 2, 3, 4, 5}},  /*152-159*/ /*Raven 9 */
    {{0, 6, 2, 7, 1, 5, 4, 3}},  /*160-167*/ /*Raven 10*/
    {{0, 5, 1, 6, 2, 7, 4, 3}},  /*168-175*/ /*Raven 10*/
    {{2, 6, 1, 3, 0, 4, 5, 7}},  /*176-183*/ /*Raven 11*/
    {{1, 6, 2, 7, 0, 4, 3, 5}},  /*184-191*/ /*Raven 11*/
    {{0, 7, 2, 4, 1, 5, 3, 6}},  /*192-199*/ /*Raven 12*/
    {{0, 5, 1, 7, 3, 2, 4, 6}},  /*200-207*/ /*Raven 12*/
    {{1, 5, 0, 7, 4, 2, 3, 6}},  /*208-215*/ /*Raven 13*/
    {{0, 6, 1, 7, 2, 3, 4, 5}},  /*216-223*/ /*Raven 13*/
    {{0, 6, 2, 7, 1, 5, 4, 3}},  /*224-231*/ /*Raven 14*/
    {{0, 5, 1, 6, 2, 7, 4, 3}},  /*232-239*/ /*Raven 14*/
    {{2, 6, 1, 3, 0, 4, 5, 7}},  /*240-247*/ /*Raven 15*/
    {{1, 6, 2, 7, 0, 4, 3, 5}},  /*248-255*/ /*Raven 15*/
};


#define APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_0 {{3,2,0,1,7,6,4,5}}
#define APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1 {{4,5,7,6,0,1,3,2}}
#define APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1 {{0,1,2,3,4,5,6,7}}

static CPSS_PORT_MAC_TO_SERDES_STC
falcon_3_2T_MacToSerdesMap[FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE] =
{
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_0,            /* 0-7*/    /*Raven 0 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1,            /* 8-15*/   /*Raven 0 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_0,            /*16-23*/   /*Raven 1 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1,            /*24-31*/   /*Raven 1 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_0,            /*32-39*/   /*Raven 2 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1,            /*40-47*/   /*Raven 2 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_0,            /*48-55*/   /*Raven 3 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*56-63*/   /*Raven 3 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*64-71*/   /*Raven 4 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*72-79*/   /*Raven 4 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*80-87*/   /*Raven 5 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*88-95*/   /*Raven 5 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*96-103*/  /*Raven 6 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*104-111*/ /*Raven 6 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1,            /*112-119*/ /*Raven 7 */
    APPDEMO_FALCON_3_2T_MAC_2_SD_MAP_1_to_1             /*120-127*/ /*Raven 7 */
};

#define APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0 {{0,1,2,3,4,5,6,7}}
#define APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS {{0,1,2,3,4,5,6,7}}
#define APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1 {{7,6,5,4,3,2,1,0}}

static CPSS_PORT_MAC_TO_SERDES_STC  falcon_2T4T_MacToSerdesMap[] =
{
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 0 MACs: 0-7      SDs: 0-7   */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 0 MACs: 8-15     SDs: 8-15  */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 2 MACs: 32-39    SDs: 16-23 */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 2 MACs: 40-47    SDs: 24-31 */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    {{0, 3, 1, 2, 5, 7, 4, 6}},         /* Raven 4 MACs: 64-71    SDs: 32-39 */
    {{7, 4, 6, 5, 2, 0, 3, 1}},         /* Raven 4 MACs: 72-79    SDs: 40-47 */
    {{0, 3, 1, 2, 5, 7, 4, 6}},         /* Raven 5 MACs: 80-87    SDs: 48-55 */
    {{7, 4, 6, 5, 2, 0, 3, 1}},         /* Raven 5 MACs: 88-95    SDs: 56-63 */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 7 MACs: 112-119  SDs: 64-71 */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 7 MACs: 120-127  SDs: 72-79 */
};

// TODO: Shud be  per device array
mrvDevInfo mrvDevInfodata = {};

#define MRVSave(_data) mrvDevInfodata._data = _data

#define MRVGet(_data) _data = mrvDevInfodata._data

#define MRVSetPort(_portNum) mrvDevInfodata.Ports[_portNum] = GT_TRUE

#define MRVGetPort(_portNum) mrvDevInfodata.Ports[_portNum]

#ifdef RETRY_PP_SOFT_RESET
GT_STATUS cpssHalCheckIsHwResetDone(uint32_t devNum);
GT_STATUS cpssHalWriteHwResetDone(uint32_t devNum);

GT_STATUS cpssHalReadWriteRestoreReg(GT_U8  devNum,
                                     regsToRestore_t regsToRestore[], GT_BOOL isRead)
{
    GT_STATUS rc = GT_OK;
    GT_U32          ii;
    GT_U32          supportMgWindowsRegisters = 1;
    GT_U32          mgOffset;
    GT_U32          *unitBasePtr;

    if (PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0)
    {
        return (GT_STATUS)GT_ERROR;
    }

    unitBasePtr = NULL;
    rc = prvCpssDxChUnitBaseTableGet(devNum, &unitBasePtr);
    if ((GT_NOT_APPLICABLE_DEVICE != rc) && (rc != GT_OK))
    {
        return rc;
    }
    mgOffset = (NULL == unitBasePtr)?0:unitBasePtr[PRV_CPSS_DXCH_UNIT_MG_E];

    for (ii = 0; ii < sizeof(regsToRestore)/sizeof(regsToRestore[0]); ii++)
    {
        if (0 == regsToRestore[ii].regAddr)
        {
            break;
        }
        regsToRestore[ii].regAddr += mgOffset;
    }


    if (supportMgWindowsRegisters)
    {
        /* fill-in registers restore DB by run time values */
        for (ii = 0 ; regsToRestore[ii].regAddr != 0; ii++)
        {
            if (SMI_MNG_ADDRESS_MARK_CNS == regsToRestore[ii].regAddr)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE)
                {
                    regsToRestore[ii].regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(
                                                    devNum)->GOP.SMI[0].SMIMiscConfiguration;
                }
                else
                {
                    /* BC2 A0 - MSYS does not use this register.
                       mark as last one and not used */
                    regsToRestore[ii].regAddr = 0;
                }
            }
        }
    }


    if (supportMgWindowsRegisters)
    {
        if (isRead)
        {
            /* read the registers */
            for (ii = 0 ; regsToRestore[ii].regAddr != 0; ii++)
            {
                rc = prvCpssDrvHwPpReadRegister(devNum, regsToRestore[ii].regAddr,
                                                &(regsToRestore[ii].value));
                cpssOsPrintf("prvCpssDrvHwPpReadRegister :%d \n", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            /* write the saved registers */
            /* restore the registers */
            for (ii = 0 ; regsToRestore[ii].regAddr != 0; ii++)
            {
                rc = prvCpssDrvHwPpWriteRegister(devNum, regsToRestore[ii].regAddr,
                                                 regsToRestore[ii].value);
                cpssOsPrintf("prvCpssDrvHwPpWriteRegister :%d \n", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return rc;
}
void printRestroreRegValues(GT_U8 devNum, regsToRestore_t regsToRestore[])
{
    uint32_t ii = 0;
    cpssOsPrintf("Dump Reg for Dev :%d \n", devNum);
    for (ii = 0 ; regsToRestore[ii].regAddr != 0; ii++)
    {
        cpssOsPrintf("regAddr: 0x%x   Val: 0x%x\n", regsToRestore[ii].regAddr,
                     regsToRestore[ii].value);
    }
    return;
}
#endif

/*******************************************************************************
* MRVDevTypeGetIndex
*
* DESCRIPTION: find location of device type in mrvSupportedDevData
*
* INPUTS:
*       devType  - device type (PP type)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       index of the PP in the mrvSupportedDevData
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 MRVDevTypeGetIndex(CPSS_PP_DEVICE_TYPE devType)
{
    GT_U32 i;
    for (i=0; i < mrvSupportedDevDataSize; i++)
    {
        if (mrvSupportedDevData[i].devType == devType)
        {
            return i;
        }
    }
    return 0;
}

/*******************************************************************************
* cpssHalInitializeValidateProfile
*
* DESCRIPTION:
* APPLICABLE DEVICES:
* NOT APPLICABLE DEVICES:
* INPUTS:
* OUTPUTS:
* RETURNS:
* COMMENTS:
*******************************************************************************/
GT_STATUS cpssHalInitializeValidateProfile
(
    XP_DEV_TYPE_T devType,
    PROFILE_STC                 profile[] /* can be null*/
)
{
    if (NULL==profile)
    {
        return GT_OK;
    }
    GT_U32 index;
    for (index = 0; index < 1000; index++)
    {
        if (profile[index].profileType == PROFILE_TYPE_INVALID_E)
        {
            /* invalid profile id - likely  uninitialized */
            return GT_ERROR;
        }
        else if (profile[index].profileType == PROFILE_TYPE_LAST_E)
        {
            /* last profile entry */

            // Process/Fill the Platform config in profile
            cpssHalReadPlatformConfig(cpssHalPlatformConfigFileNameGet(), profile, index);

            return GT_OK;
        }
        else if (profile[index].profileType == PROFILE_TYPE_HW_SKU_E)
        {
            CPSSHAL_SWITCH(halHwsku) = profile[index].profileValue.hwTableSizes;
            if (cpssHalSwitchUpdateSku(devType) != GT_OK)
            {
                return GT_ERROR;
            }
        }
        else if ((profile[index].profileType == PROFILE_TYPE_PORT_MAP_E) ||
                 (profile[index].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
        {
            profile[index].profileValue.portMap.isRxTxParamValid = 0;
        }
        else if (profile[index].profileType >= PROFILE_TYPE_MAX_E)
        {
            /* unknown profile type */
            return GT_ERROR;
        }

    }
    /* to many entries in profile */
    return GT_ERROR;
}

/*******************************************************************************
* cpssHalApiReturn
*
* DESCRIPTION:
* APPLICABLE DEVICES:
* NOT APPLICABLE DEVICES:
* INPUTS:
* OUTPUTS:
* RETURNS:
* COMMENTS:
*******************************************************************************/
GT_STATUS cpssHalApiReturn
(
    const char *func_name,
    int         line,
    const char *text,
    GT_STATUS   status
)
{
    GT_STATUS new_status;
    cpssOsPrintf("%s:%d - ERROR: %s returns status: %d\n", func_name, line, text,
                 status);
#ifdef DEMO
    new_status = GT_OK;
#else
    new_status = status;
#endif
    return new_status;
}


/*******************************************************************************
* cpssHalInitializeSystem
*
* DESCRIPTION:
*       initialize cpss functionality not related to device
*           initialize os
*           initialize extDrv
*           initialize interrupts
*           initialize Pci
*           initialize cpss PP init
*           disable interrupts
*           perform pci scan

*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalInitializeSystem
(
    void
)
{
    GT_STATUS                   rc;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_OS_FUNC_BIND_STC       osFuncBind;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;

    /* must be called before any Os function */
    rc = osWrapperOpen(NULL);
    if (rc != GT_OK)
    {
        osPrintf("osWrapper initialization failure!\n");
        return rc;
    }

    /* Initialize memory pool. It must be done before any memory allocations */
    /* must be before osWrapperOpen(...) that calls osStaticMalloc(...) */
    rc = osMemInit(cpssHalSys_param_default_mem_init_size(), GT_TRUE);
    if (rc != GT_OK)
    {
        osPrintf("osMemInit() failed, rc=%d\n", rc);
        return rc;
    }

    osPrintf("osMemInit() allocated %d\n",
             cpssHalSys_param_default_mem_init_size());

    rc = cpssHalInitServicesGetDefaultOsBindFuncs(&osFuncBind);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssHalInitServicesGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssHalInitServicesGetDefaultTraceFuncs(&traceFuncBindInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* 1'st step */
    rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBind, &traceFuncBindInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* 2'nd step */
    rc = cpssPpInit();
    if (rc != GT_OK)
    {
        return rc;
    }

    osMemSet(&mrvDevInfodata, 0, sizeof(mrvDevInfodata));

    return GT_OK;
}

/*******************************************************************************
* cpssHalInitializeDevice
*
* DESCRIPTION:
*       initialize the device
*           phase 1
*           implement wa
*           phase 2
*           logical init
*           map 'high' ports to 'low' ports starrting from 1..
*           initialize SMI
*           initilize cpss libs (using input devnum) in correcte order
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
extern GT_STATUS falcon_initStaticSerdesMuxing(GT_U8 devNum);
extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum(GT_U8    devNum,
                                                     GT_U32   portGroupId);
extern GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(GT_U8    devNum);

GT_TASK tuneStartTaskId;

/* by default the WA is not disabled */
static GT_U32   falcon_initPortDelete_WA_disabled = 0;

static GT_U32   falconLastSdmaQueue = 31;
static GT_U32   useIpfixFlowManager =
    0; /* 0 is used for disabling Ipfix Flow manager feature */
static GT_U8    IpfixFlow_disableTxSdma =
    6; /* Default tx queue used by fw in gtAppDemoIpFix.c file */
static GT_U8    IpfixFlow_disableRxSdma =
    6; /* Default rx queue used by fw in gtAppDemoIpFix.c file */
/* This variable will be set to GT_TRUE  by function appDemoRxBuffsCacheableSet in case of cacheable RX buffers */
static GT_BOOL rxBuffersInCachedMem = GT_FALSE;

#define RX_BUFF_ALLIGN_DEF      1

/* MACRO to allow loops on CPSS_MAX_SDMA_CPU_PORTS_CNS and to access only used interfaces (valid and used !!!)*/
#define PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum,cpuIndex)     \
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[cpuIndex].valid == GT_FALSE ||         \
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[cpuIndex].usedAsCpuPort == GT_FALSE)   \
        continue


/* support 2 queues for the Falcon WA */
static GT_U32   isSdmaQueueInPacketGeneratorMode(
    IN CPSS_PP_FAMILY_TYPE_ENT devFamily,
    IN  GT_U32  queue,
    OUT GT_U32  *buffSizePtr
    , /* in SDMA generator mode , the buffers size in the queue */
    OUT GT_U32
    *numOfDescPtr /* in SDMA generator mode , the number of descriptors in the queue */
)
{
    *buffSizePtr =0;
    *numOfDescPtr=0;

    if (devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return 0;/* (currently) no need for packet generator mode */
    }

    if (falcon_initPortDelete_WA_disabled)
    {
        /* if the WA disabled , we not need packet generator mode */
        return 0;
    }

    /* last 2 queues are served as packet generator , for the Falcon 'port delete WA' */
    if (queue == falconLastSdmaQueue)
    {
        /* set the second queue with 2 descriptors and (3000+16) buffer size */
        *buffSizePtr =(3000+16);
        /* need only single buffer but the appDemoAllocateDmaMem(...) divide it by 2 */
        *numOfDescPtr=2;
        return 1;
    }
    else if (queue == (falconLastSdmaQueue-1))
    {
        *buffSizePtr =0;/* keep 'queue defaults' */
        *numOfDescPtr=0;/* keep 'queue defaults' */
        return 1;
    }

    return 0;
}

/* code copied from appDemo appDemoAllocateDmaMem(...) and edited */
/**
* @internal appDemoAllocateDmaMem function
* @endinternal
*
* @brief   This function allocates memory for phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
*/

static GT_STATUS appDemoAllocateDmaMem_useMultiNetIfSdma(
    IN      GT_U8                       devNum,
    IN      GT_U32                      devType,
    IN      GT_U32                      rxDescNum,
    IN      GT_U32                      rxBufSize,
    IN      GT_U32                      rxBufAllign,
    IN      GT_U32                      txDescNum,
    IN      GT_U32                      txGenBuffSize,
    IN      GT_U32                      txGenNumOfDescBuff,
    INOUT   CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *ppPhase2Params
)
{
    GT_STATUS   rc;
    GT_U32  txGenDescSize, rxDescSize;
    GT_U32  ii, jj;
    GT_U32  txQue, rxQue;
    GT_U32                      *tmpPtr;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    GT_BOOL
    txGenQueue[NUM_OF_SDMA_QUEUES];/* Enable Tx queue to work in generator mode */
    GT_U32
    txGenQueueBuffSize[NUM_OF_SDMA_QUEUES];/* in SDMA generator mode , the buffers size in the queue */
    GT_U32
    txGenNumOfDescBuffArr[NUM_OF_SDMA_QUEUES];/* in SDMA generator mode , the number of descriptors in the queue */
    GT_U32 rxDescrNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS][8];

    CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC  * sdmaQueuesConfigPtr;
    CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC  * sdmaRxQueuesConfigPtr;

    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

    for (txQue = 0; txQue < NUM_OF_SDMA_QUEUES; txQue++)
    {
        txGenQueue[txQue] = GT_FALSE;
        txGenQueueBuffSize[txQue] = 0;
        txGenNumOfDescBuffArr[txQue] = 0;
        if (isSdmaQueueInPacketGeneratorMode(devFamily, txQue,
                                             &txGenQueueBuffSize[txQue],    /* explicit size for the buffers in this queue  */
                                             &txGenNumOfDescBuffArr[txQue]))/* explicit number of descriptors for this queue  */
        {
            /* Enable Tx queue to work in Tx queue generator mode */
            txGenQueue[txQue] = GT_TRUE;
        }
    }

    /* Tx block size calc & malloc  */
    rc = cpssDxChHwTxDescSizeGet(devType, &txGenDescSize);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwTxDescSizeGet", rc);
        return rc;
    }

    ii=0;
    for (jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
    {
        PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);

        if (ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
        {
            break;
        }
        for (txQue = 0; txQue < 8; txQue++)
        {
            sdmaQueuesConfigPtr =
                &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
            /* need to make this array per mg per q:*/
            if (txGenQueue[(ii*8) + txQue] == GT_FALSE)
            {
                /* Tx block size calc & malloc  */
                sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E;
                sdmaQueuesConfigPtr->numOfTxBuff =
                    0; /*not relevant in non traffic generator mode*/
                sdmaQueuesConfigPtr->buffSize =
                    0; /*not relevant in non traffic generator mode*/
                /* Block TxSdmaQueue for flow manager feature */
                if ((useIpfixFlowManager == 1) && (jj == 0)  &&
                    (IpfixFlow_disableTxSdma == txQue))
                {
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                    continue;
                }
                sdmaQueuesConfigPtr->numOfTxDesc = txDescNum ? (txDescNum/8) : 0;  /*125 */
                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                    (sdmaQueuesConfigPtr->numOfTxDesc * txGenDescSize);
                if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                {
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                        osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                    if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                    {
                        osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                        return GT_OUT_OF_CPU_MEM;
                    }

                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                }
            }
            else
            {
                /* Generator mode */
                GT_U32 tmp_txGenBuffSize = (txGenBuffSize == 0) ||
                                           (txGenQueueBuffSize[(ii*8) + txQue] == 0) ?
                                           txGenBuffSize :
                                           txGenQueueBuffSize[(ii*8) + txQue] + 16;

                sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                if (txGenNumOfDescBuffArr[(ii*8) + txQue])
                {
                    sdmaQueuesConfigPtr->numOfTxDesc = txGenNumOfDescBuffArr[(ii*8) + txQue];
                }
                else
                {
                    sdmaQueuesConfigPtr->numOfTxDesc = txGenNumOfDescBuff;
                }

                sdmaQueuesConfigPtr->numOfTxDesc /= 2;

                sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E;
                sdmaQueuesConfigPtr->numOfTxBuff = sdmaQueuesConfigPtr->numOfTxDesc;

                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                    (sdmaQueuesConfigPtr->numOfTxDesc + 1) * (txGenDescSize + tmp_txGenBuffSize);
                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                    osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                {
                    osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                    return GT_OUT_OF_CPU_MEM;
                }
                sdmaQueuesConfigPtr->buffSize = tmp_txGenBuffSize;
            }
        }

        ii++;
    }
    /* Rx block size calc & malloc  */

    for (jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
    {
        for (rxQue = 0; rxQue < 8; rxQue++)
        {
            rxDescrNumArr[jj][rxQue] = rxDescNum/8;
        }
    }

    rc = cpssDxChHwRxDescSizeGet(devType, &rxDescSize);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwRxDescSizeGet", rc);
        return rc;
    }

    ii = 0;
    for (jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
    {
        PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);

        if (ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
        {
            continue;
        }
        for (rxQue = 0; rxQue < 8; rxQue++)
        {
            sdmaRxQueuesConfigPtr =
                &ppPhase2Params->multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue];
            sdmaRxQueuesConfigPtr->buffAllocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
            sdmaRxQueuesConfigPtr->buffersInCachedMem = rxBuffersInCachedMem;
            sdmaRxQueuesConfigPtr->numOfRxDesc = rxDescrNumArr[ii][rxQue];
            /* Block RxSdmaQueue for flow manager feature */
            if ((useIpfixFlowManager == 1) && (jj == 0) &&
                (IpfixFlow_disableRxSdma == rxQue))
            {
                continue;
            }
            sdmaRxQueuesConfigPtr->descMemSize = sdmaRxQueuesConfigPtr->numOfRxDesc *
                                                 rxDescSize;
            sdmaRxQueuesConfigPtr->descMemPtr = osCacheDmaMalloc(
                                                    sdmaRxQueuesConfigPtr->descMemSize);
            if (sdmaRxQueuesConfigPtr->descMemPtr == NULL)
            {
                osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                for (txQue = 0; txQue < 8; txQue++)
                {
                    sdmaQueuesConfigPtr =
                        &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                    if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                    {
                        osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                    }
                }

                return GT_OUT_OF_CPU_MEM;
            }
            /* init the Rx buffer allocation method */
            /* Set the system's Rx buffer size.     */
            if ((rxBufSize % rxBufAllign) != 0)
            {
                rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
            }


            sdmaRxQueuesConfigPtr->buffHeaderOffset =
                0; /* give the same offset to all queues*/
            sdmaRxQueuesConfigPtr->buffSize = rxBufSize;
            sdmaRxQueuesConfigPtr->numOfRxBuff =
                rxDescrNumArr[ii][rxQue]; /* by default the number of buffers equel the number of descriptors*/
            sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize =
                (rxBufSize * sdmaRxQueuesConfigPtr->numOfRxBuff);
            if (sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize != 0)
            {
                /* If RX buffers should be cachable - allocate it from regular memory */
                if (GT_TRUE == rxBuffersInCachedMem)
                {
                    tmpPtr = osMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize +
                                       rxBufAllign-1));
                }
                else
                {
                    tmpPtr = osCacheDmaMalloc((
                                                  sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                }

                if (tmpPtr == NULL)
                {
                    osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                    for (txQue = 0; txQue < 8; txQue++)
                    {
                        sdmaQueuesConfigPtr =
                            &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                        if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                        {
                            osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                        }
                    }
                    osCacheDmaFree(sdmaRxQueuesConfigPtr->descMemPtr);
                    return GT_OUT_OF_CPU_MEM;
                }
            }
            else
            {
                tmpPtr = NULL;
            }

            if ((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
            {
                tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                   (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
            }
            sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr = tmpPtr;
        }

        ii++;
    }

    return GT_OK;
}

typedef struct
{
    GT_UINTPTR          evHndl;
    GT_U32              hndlrIndex;
} EV_HNDLR_PARAM;

GT_STATUS cpssHalGetPortByMac
(
    GT_U8 cpssDevId,
    GT_U32 cpssMacNum,
    GT_U32 *cpssPortNum
)
{
    int i;
    PROFILE_STC *profile = NULL;
    cpssHalGetProfile(cpssDevId, &profile);
    if (NULL == profile)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "PortManager dev %d port %d Profile failed {%x}\n", cpssDevId, cpssPortNum,
              profile);
        return GT_NO_MORE;
    }
    i = 0;
    while (profile[i].profileType != PROFILE_TYPE_LAST_E)
    {
        if ((profile[i].profileType == PROFILE_TYPE_PORT_MAP_E) ||
            (profile[i].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
        {
            if (cpssMacNum == profile[i].profileValue.portMap.macNum)
            {
                *cpssPortNum = profile[i].profileValue.portMap.portNum;
                return GT_OK;
            }
        }
        i++;
    }
    return GT_NO_MORE;
}

static GT_STATUS cpssHalPortMgrEventAction
(
    GT_U8                   devId,
    CPSS_UNI_EV_CAUSE_ENT   uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS   status = GT_OK;
    CPSS_PORT_MANAGER_STC portEventStc;
    GT_U32 cpssPortNum;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    GT_BOOL hcdFound;
    CPSS_PORT_SPEED_ENT apSpeed;
    CPSS_PORT_INTERFACE_MODE_ENT apIfMode;
    CPSS_DXCH_PORT_AP_STATUS_STC apStatusDx;

    switch (uniEv)
    {
        case CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E:
            cpssOsPrintf("cpssHalPortMgrEventAction evExtData %d CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E Unhandled\n ",
                         evExtData);
            break;
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            cpssPortNum = evExtData;
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
            {
#if 0
                status = cpssDxChPortManagerStatusGet(devId, cpssPortNum, &portStage);
                if (status != GT_OK)
                {
                    cpssOsPrintf("calling PortManagerStatusGet returned=%d, portNum=%d\n", status,
                                 cpssPortNum);
                    break;
                }
                if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E)
                {
                    cpssOsPrintf(" LinkStatus: port is in HW failure state or in disable abort mac_level_change notification %d\n",
                                 cpssPortNum);
                    return GT_OK;
                }
                if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
                {
                    cpssOsPrintf("port is in reset state so abort mac_level_change notification%d\n",
                                 cpssPortNum);
                    return GT_OK;
                }
                if (portStage.portUnderOperDisable == GT_FALSE)
                {
                    status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                         &portEventStc);
                    if (status != GT_OK)
                    {
                        cpssOsPrintf("calling cpssDxChPortManagerEventSet returned=%d, portNum=%d\n",
                                     status,
                                     cpssPortNum);
                        return status;
                    }
                }

                cpssOsPrintf("Port Event CPSS_PP_PORT_LINK_STATUS_CHANGED_E received %d Mac %d Port %d\n",
                             uniEv, evExtData, cpssPortNum);

                /* Port Manager is not updated immdeiately and still takes it own time to update the status.
                   Hence using PorLinkStatusGet to return the current status .*/
#endif //0
                status = cpssHalLinkstatuschangeEvent(devId, cpssPortNum);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: cpssHalLinkstatuschangeEvent returned=%d, portNum=%d\n",
                                 status, cpssPortNum);
                    //return status;
                }

                GT_BOOL newLinkStatus = GT_FALSE;
                status = cpssDxChPortLinkStatusGet(devId, cpssPortNum, &newLinkStatus);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: cpssDxChPortLinkStatusGet returned=%d, portNum=%d\n",
                                 status, cpssPortNum);
                    return status;
                }
                cpssOsPrintf("EVENT: Port: %d LinkStatus: %d \n",  cpssPortNum, newLinkStatus);

#if 0
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT brgLinkState;
                brgLinkState = (newLinkStatus) ?
                               CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                               CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

                status = cpssDxChBrgEgrFltPortLinkEnableSet(devId, cpssPortNum, brgLinkState);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: cpssDxChBrgEgrFltPortLinkEnableSet Failed port :%d state :%d \n",
                                 cpssPortNum, brgLinkState);
                }
#endif

                int xpsPort;
                if (newLinkStatus == GT_TRUE)
                {
                    cpssHalL2GPortNum(devId, cpssPortNum, &xpsPort);
                    cpssOsPrintf("EVENT: xps port number for link status changed to up  in CPSS_PP_PORT_LINK_STATUS_CHANGED_E %d \n",
                                 xpsPort);
                    if (cpssHalLinkStateNotify)
                    {
                        cpssHalLinkStateNotify(devId, xpsPort, 1);
                    }
                }
                else
                {
                    cpssHalL2GPortNum(devId, cpssPortNum, &xpsPort);
                    cpssOsPrintf("EVENT: xps port number for link status changed to down  in CPSS_PP_PORT_LINK_STATUS_CHANGED_E %d \n",
                                 xpsPort);
                    if (cpssHalLinkStateNotify)
                    {
                        cpssHalLinkStateNotify(devId, xpsPort, 0);
                    }
                }
            }
            break;
        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
        case CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E:
        case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
            cpssPortNum = evExtData;
            {
                status = cpssDxChPortManagerStatusGet(devId, cpssPortNum, &portStage);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: PortManagerStatusGet returned=%d, portNum=%d\n", status,
                                 cpssPortNum);
                    break;
                }
                if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E)
                {
                    cpssOsPrintf("EVENT: port %d is in HW FAILURE state abort mac_level_change notification\n",
                                 cpssPortNum);
                    // Proceed further to try Event set
                    // return GT_OK;
                }
                if (portStage.portUnderOperDisable == GT_TRUE)
                {
                    cpssOsPrintf("EVENT: port %d is disable state abort mac_level_change notification\n",
                                 cpssPortNum);
                }

                status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                     &portEventStc);
                cpssOsPrintf("EVENT: Port Event CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E  received %d Mac %d Port %d\n",
                             uniEv, evExtData, cpssPortNum);
            }
            break;

        case CPSS_PP_PORT_802_3_AP_E:
            //Not HANDLED
            cpssOsPrintf("EVENT: Port Event CPSS_PP_PORT_802_3_AP_E Unhandled received %d Port %d\n",
                         uniEv, evExtData);
            break;

        case CPSS_SRVCPU_PORT_AP_DISABLE_E:
            cpssOsPrintf("EVENT: cpssHalPortMgrEventAction evExtData %d ", evExtData);
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E;
            if (GT_OK == cpssHalGetPortByMac(devId, evExtData, &cpssPortNum))
            {
                status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                     &portEventStc);
                cpssOsPrintf("EVENT: Port Event CPSS_SRVCPU_PORT_AP_DISABLE_E received %d Mac %d Port %d\n",
                             uniEv, evExtData, cpssPortNum);
            }
            else
            {
                cpssOsPrintf("ERROR: Port Event CPSS_SRVCPU_PORT_AP_DISABLE_E FAILED to set %d Mac %d Port %d\n",
                             uniEv, evExtData, -1);
            }

            break;
        case CPSS_SRVCPU_PORT_802_3_AP_E:
            cpssOsPrintf("EVENT: cpssHalPortMgrEventAction CPSS_SRVCPU_PORT_802_3_AP_E evExtData %d ",
                         evExtData);
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E;
            if (GT_OK == cpssHalGetPortByMac(devId, evExtData, &cpssPortNum))
            {
                hcdFound = GT_FALSE;
                apSpeed = CPSS_PORT_SPEED_NA_E;
                apIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;

                status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                     &portEventStc);
                cpssOsPrintf("EVENT: Port Event CPSS_SRVCPU_PORT_802_3_AP_E received %d Mac %d Port %d\n",
                             uniEv, evExtData, cpssPortNum);
                status = cpssDxChPortApPortStatusGet(devId, cpssPortNum, &apStatusDx);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortApPortStatusGet:rc=%d,portNum=%d\n",
                                 status,  cpssPortNum);
                    return status;
                }

                hcdFound = apStatusDx.hcdFound;
                apSpeed = apStatusDx.portMode.speed;
                apIfMode = apStatusDx.portMode.ifMode;

                if (hcdFound)
                {
                    CPSS_PORT_SPEED_ENT speed;
                    CPSS_PORTS_BMP_STC portsBmp;
                    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, cpssPortNum);

                    /* Check current spped */
                    status = cpssDxChPortSpeedGet(devId,  cpssPortNum, &speed);
                    if (status != GT_OK)
                    {
                        cpssOsPrintf("ERROR: CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortSpeedGet:rc=%d,portNum=%d\n",
                                     status,  cpssPortNum);
                        return status;
                    }
                    else
                    {
                        /*if speed <= CPSS_PORT_SPEED_1000_E we get the value from HW; at this stage speed is not always update correctly;
                          assume pizza allocation can<E2><96><92>~Rt be less than 1G*/

                        if (speed <= CPSS_PORT_SPEED_1000_E)
                        {
                            speed = CPSS_PORT_SPEED_1000_E;
                        }
                    }

                    /* check if port was already deleted */
                    if (speed == CPSS_PORT_SPEED_NA_E)
                    {
                        return GT_OK;
                    }
                    if ((speed != CPSS_PORT_SPEED_NA_HCD_E) && ((speed != apSpeed) &&
                                                                (CPSS_PORT_SPEED_20000_E != speed)))
                    {
                        /* Release pizza resources */
                        status = cpssDxChPortModeSpeedSet(devId, &portsBmp, GT_FALSE,
                                                          apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                        if (status != GT_OK)
                        {
                            cpssOsPrintf("ERROR: CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n",
                                         status,  cpssPortNum);
                            return status;
                        }

                        speed = CPSS_PORT_SPEED_NA_HCD_E;
                    }

                    if (speed == CPSS_PORT_SPEED_NA_HCD_E)
                    {
                        status = cpssDxChPortModeSpeedSet(devId, &portsBmp, GT_TRUE,
                                                          apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                        if (status != GT_OK)
                        {
                            cpssOsPrintf("ERROR: CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n",
                                         status,  cpssPortNum);
                            return status;
                        }

                    }

                    if (speed == CPSS_PORT_SPEED_NA_HCD_E)
                    {
                        status = cpssDxChPortModeSpeedSet(devId, &portsBmp, GT_TRUE,
                                                          apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                        if (status != GT_OK)
                        {
                            cpssOsPrintf("ERROR: CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n",
                                         status,  cpssPortNum);
                            return status;
                        }
                    }
                }
                else
                {
                    cpssOsPrintf("EVENT: CPSS_SRVCPU_PORT_802_3_AP_E - portNum=%d, no resolution for hcd\n",
                                 cpssPortNum);
                }
            }
            else
            {
                cpssOsPrintf("ERROR: Port Event CPSS_SRVCPU_PORT_802_3_AP_E cpssHalGetPortByMac FAILED %d Mac %d Port %d\n",
                             uniEv, evExtData, -1);
            }

            break;
        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
            cpssPortNum = (GT_U16)evExtData;
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
            {
                cpssOsPrintf("EVENT: Port Event CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.  received %d Mac %d Port %d\n",
                             uniEv, evExtData, cpssPortNum);
                if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_TRUE)
                {
                    /* this event is not handled for SIP6 devices */
                    return GT_OK;
                }
                status = cpssDxChPortManagerStatusGet(devId, cpssPortNum, &portStage);
                if (status != GT_OK)
                {
                    cpssOsPrintf("ERROR: PortManagerStatusGet returned=%d, portNum=%d\n", status,
                                 cpssPortNum);
                    break;
                }
                if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
                    portStage.portUnderOperDisable == GT_TRUE)
                {
                    cpssOsPrintf("EVENT: port %d is in HW FAILURE state or in disable abort mac_level_change notification\n",
                                 cpssPortNum);
                    // Proceed further to try Event set
                    // return GT_OK;
                }
                status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                     &portEventStc);
            }

            /* training is done inside port manager, so no need to continue */


            break;
        case CPSS_PP_PORT_AN_HCD_FOUND_E:
            cpssOsPrintf("Event CPSS_PP_PORT_AN_HCD_FOUND_E\n");
            status = cpssDxChPortPhysicalPortMapReverseMappingGet(devId,
                                                                  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, evExtData, &cpssPortNum);
            if (status != GT_OK)
            {
                cpssOsPrintf("CPSS_PP_PORT_AN_HCD_FOUND_E - cpssDxChPortPhysicalPortMapReverseMappingGet:status=%d,portNum=%d\n",
                             status, evExtData);
                break;
            }

            status = cpssDxChPortManagerStatusGet(devId, cpssPortNum, &portStage);
            if (status != GT_OK)
            {
                cpssOsPrintf("ERROR: PortManagerStatusGet returned=%d, portNum=%d\n", status,
                             cpssPortNum);
                break;
            }

            if ((portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E) ||
                ((portStage.portUnderOperDisable == GT_TRUE) &&
                 (!PRV_CPSS_SIP_6_CHECK_MAC(devId))))
            {
                cpssOsPrintf("EVENT: port %d is in HW FAILURE state or in disable abort mac_level_change notification\n",
                             cpssPortNum);
                // Proceed further to try Event set
                // return GT_OK;
            }
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E;
            status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                 &portEventStc);
            if (status != GT_OK)
            {
                cpssOsPrintf("ERROR: PortManagerStatusSet returned=%d, portNum=%d\n", status,
                             cpssPortNum);
                break;
            }

            break;
        case CPSS_PP_PORT_AN_RESTART_E:
            cpssOsPrintf("Event CPSS_PP_PORT_AN_RESTART_E\n");
            status = cpssDxChPortPhysicalPortMapReverseMappingGet(devId,
                                                                  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, evExtData, &cpssPortNum);
            if (status != GT_OK)
            {
                cpssOsPrintf("CPSS_PP_PORT_AN_RESTART_E - cpssDxChPortPhysicalPortMapReverseMappingGet:status=%d,portNum=%d\n",
                             status, evExtData);
                break;
            }

            status = cpssDxChPortManagerStatusGet(devId, cpssPortNum, &portStage);
            if (status != GT_OK)
            {
                cpssOsPrintf("ERROR: PortManagerStatusGet returned=%d, portNum=%d\n", status,
                             cpssPortNum);
                break;
            }

            if ((portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E) ||
                ((portStage.portUnderOperDisable == GT_TRUE) &&
                 (!PRV_CPSS_SIP_6_CHECK_MAC(devId))))
            {
                cpssOsPrintf("EVENT: port %d is in HW FAILURE state or in disable abort mac_level_change notification\n",
                             cpssPortNum);
                // Proceed further to try Event set
                // return GT_OK;
            }
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E;
            status = cpssDxChPortManagerEventSet(devId, (GT_PHYSICAL_PORT_NUM) cpssPortNum,
                                                 &portEventStc);
            if (status != GT_OK)
            {
                cpssOsPrintf("ERROR: PortManagerStatusSet returned=%d, portNum=%d\n", status,
                             cpssPortNum);
                break;
            }
            break;
        default:
            MRVL_HAL_API_TRACE("Event type is not matched", uniEv);
            break;
    }

    return status;
}

static unsigned __TASKCONV cpssHalPortMgrEventHdlr(GT_VOID * param)
{
    xpDevice_t          devId =0;
    GT_U8               devNum = 0;
    GT_STATUS           status = GT_OK;
    GT_U32 i;
    GT_UINTPTR evHndl;
    GT_U32 evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];
    GT_U32 evBitmap;
    GT_U32 evExtData;
    CPSS_UNI_EV_CAUSE_ENT uniEv;
    GT_U32 evCauseIdx;
    EV_HNDLR_PARAM *hndlrParamPtr;

    hndlrParamPtr = (EV_HNDLR_PARAM*)param;
    evHndl        = hndlrParamPtr->evHndl;
    while (1)
    {
        status = cpssEventSelect(evHndl, NULL, evBitmapArr,
                                 (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if (status != GT_OK)
        {
            continue;
        }

        for (evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
        {
            if (evBitmapArr[evCauseIdx] == 0)
            {
                continue;
            }

            evBitmap = evBitmapArr[evCauseIdx];

            for (i = 0; evBitmap; evBitmap >>= 1, i++)
            {
                if ((evBitmap & 1) == 0)
                {
                    continue;
                }
                uniEv = (CPSS_UNI_EV_CAUSE_ENT)((evCauseIdx << 5) + i);

                if ((status=cpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                {
                    status = cpssHalPortMgrEventAction(devId, uniEv, evExtData);

                }
            }
        }
    }
    return (GT_TRUE);
}

GT_STATUS cpssHalPortMgrEventRegister
(
    GT_U8                   devId
)
{
    CPSS_UNI_EV_CAUSE_ENT   evHndlrCauseArr[] =
    {
        CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
        CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
        CPSS_PP_PORT_802_3_AP_E,
        CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
        CPSS_SRVCPU_PORT_802_3_AP_E,
        CPSS_SRVCPU_PORT_AP_DISABLE_E,
        CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
        CPSS_PP_PORT_AN_HCD_FOUND_E,
        CPSS_PP_PORT_AN_RESTART_E
    };

    GT_U32 evHndlrCauseArrSize = sizeof(evHndlrCauseArr)/sizeof(evHndlrCauseArr[0]);
    GT_U32 ii;
    static EV_HNDLR_PARAM taskParamArr;
    GT_STATUS status = GT_OK;
    char taskName[30] = {0};
    XP_DEV_TYPE_T devType;

    taskParamArr.hndlrIndex = 25;
    taskParamArr.evHndl = (GT_UINTPTR)(&cpssHalPortMgrEventHdlr);

    /* call CPSS to bind the events under single handler */
    status = cpssEventBind(evHndlrCauseArr,
                           evHndlrCauseArrSize,
                           &taskParamArr.evHndl);
    switch (status)
    {
        case GT_FAIL:
        case GT_BAD_PTR:
        case GT_OUT_OF_CPU_MEM:
        case GT_FULL:
        case GT_ALREADY_EXIST:
            break;
    }

    if (GT_OK != status)
    {
        MRVL_HAL_API_TRACE("cpss Event bind failed for port params", status);
        return status;
    }

    cpssOsSprintf(taskName, "portEventTask");
    status = cpssOsTaskCreate(taskName,
                              200,
                              _32KB,
                              cpssHalPortMgrEventHdlr,
                              &taskParamArr,
                              &portEvntTid);
    if (status != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssOsTaskCreate failed to creating Learning Task", status);
        return status;
    }

    osTimerWkAfter(1000);
    for (ii=0; ii<evHndlrCauseArrSize; ii++)
    {
        status = prvCpssDrvEventSupportCheck(devId, evHndlrCauseArr[ii],
                                             PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS);
        if (status != GT_OK)
        {
            /* the device not supports this event */
            /* do not call CPSS to avoid 'ERROR LOG' indications about those that are not supported */
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "prvCpssDrvEventSupportCheck Event %d is not supported", evHndlrCauseArr[ii]);
            continue;
        }

        status = cpssEventDeviceMaskSet(devId, evHndlrCauseArr[ii],
                                        CPSS_EVENT_UNMASK_E);
        if (status != GT_OK)
        {
            cpssOsPrintf("cpssEventDeviceMaskSet rc %d idx %d event %d\n", status, ii,
                         evHndlrCauseArr[ii]);
            MRVL_HAL_API_TRACE("cpss Event device mask set failed %d", status);
        }
    }

    extern unsigned __TASKCONV appDemoPortInitSeqSignalDetectedStage(
        GT_VOID * param);
    cpssOsSprintf(taskName, "portManagerTask");
    status = cpssOsTaskCreate(taskName,                     /* Task Name      */
                              500,                                   /* Task Priority  */
                              _64K,                                  /* Stack Size     */
                              appDemoPortInitSeqSignalDetectedStage, /* Starting Point */
                              (GT_VOID*)((GT_UINTPTR)devId),        /* Arguments list */
                              &tuneStartTaskId);                     /* task ID        */
    if (status != GT_OK)
    {
        MRVL_HAL_API_TRACE("calling cpssOsTaskCreate appDemoPortInitSeqSignalDetectedStage ",
                           status);
        return status;
    }

    cpssHalGetDeviceType(devId, &devType);
    if (IS_DEVICE_FALCON(devType))
    {
        // SONIC-1642 - mask CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E, as interrupt makes CPU busy
        status = cpssEventDeviceMaskSet(devId, CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
                                        CPSS_EVENT_MASK_E);
        if (status != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssEventDeviceMaskSet failed", status);
            return status;
        }
    }


    return GT_OK;
}

GT_STATUS cpssHalInitializeDevice
(
    GT_U8                   devNum,
    XP_DEV_TYPE_T           xpDevType,
    PROFILE_STC             profile[],
    GT_PCI_INFO             *pciInfo
)
{
    CPSS_PP_DEVICE_TYPE                 devType;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_NEW_STC   cpssPpPhase1Info;
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC   cpssPpPhase2Info;
    CPSS_DXCH_CFG_DEV_INFO_STC          cpssDevInfo;
    GT_STATUS                           rc = GT_OK;
    GT_U32                              txDescSize = 0;
    GT_U32                              rxDescSize = 0;
    GT_U32                              auDescSize = 0;
    GT_U32                              i = 0;
    GT_U32                              port_index = 0;
    GT_U32                              ASICDataIndex;
    CPSS_DXCH_PORT_MAP_STC              portMapArray[MAX_PORT];
    GT_32                               intKey;
    GT_BOOL                             useMultiNetIfSdma = GT_FALSE;
    GT_U32                              flags = 0;
#ifdef RETRY_PP_SOFT_RESET
    GT_STATUS                           devInitStatus = GT_OK;
#endif


    if (profile == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Bad params recieved");
        return GT_BAD_PARAM;
    }

    //MRVSave(devNum);

    /* PCI scan */

    devType = (pciInfo->pciDevVendorId.devId << 16) +
              (pciInfo->pciDevVendorId.vendorId & 0xffff) ;
    /*TODO make this a macro*/
    if ((devType == CPSS_98CX8520_CNS) ||
        (devType == CPSS_98CX8540_CNS) ||
        (devType == CPSS_98CX8580_CNS) ||
        (devType == CPSS_98CX8514_CNS))
    {
        useMultiNetIfSdma = GT_TRUE;
        prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_FALCON_E);
    }
    else if (devType == CPSS_98DX3550M_CNS)
    {
        useMultiNetIfSdma = GT_TRUE;
        prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_AC5X_E);
        /*to match IS_SIP6_MBUS_ADDR(...) */
        pciInfo->pciBusNum = 0xFFFF;
        pciInfo->pciIdSel  = 0xFF;
        pciInfo->funcNo    = 0xFF;
        flags = (MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS <<
                 MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS);
    }


    //MRVSave(pciInfo);

    /* Phase 1 */
    cpssOsMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    extDrvPexConfigure(pciInfo->pciBusNum, pciInfo->pciIdSel, pciInfo->funcNo,
                       flags, &(cpssPpPhase1Info.hwInfo[0]));

    cpssOsPrintf("pci bus %d, seld %d, func %d cpssPpPhase1Info.hwInfo[0].busType %d\n",
                 pciInfo->pciBusNum, pciInfo->pciIdSel, pciInfo->funcNo,
                 cpssPpPhase1Info.hwInfo[0].busType);
    cpssPpPhase1Info.devNum = devNum;

    /* get device index from device type */
    ASICDataIndex = MRVDevTypeGetIndex(devType);

    /* retrieve PP Core Clock from HW */
    cpssPpPhase1Info.coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;
    cpssPpPhase1Info.mngInterfaceType      = cpssHalSys_param_mngInterfaceType(
                                                 xpDevType);
    cpssPpPhase1Info.ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;
    cpssPpPhase1Info.serdesRefClock        =
        mrvSupportedDevData[ASICDataIndex].serdesRefClock;
    /* Address Completion Region 1 - for Interrupt Handling    */
    cpssPpPhase1Info.isrAddrCompletionRegionsBmp = 0x02;
    /* Address Completion Regions 2,3,4,5 - for Application    */
    cpssPpPhase1Info.appAddrCompletionRegionsBmp = 0x3C;
    /* Address Completion Regions 6,7 - reserved for other CPU */
    cpssPpPhase1Info.numOfPortGroups = 1;

#ifndef RESET_PP_EXCLUDE_PEX
    //reset fix
    falcon_force_early_check_for_device_not_reset_set();
#endif

    rc = cpssDxChPortApEnableSet(devNum, 0x1, GT_TRUE);

    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortApEnableSet", rc);
        return rc;
    }

    if (IS_DEVICE_FALCON(xpDevType) || IS_DEVICE_AC5X(xpDevType))
    {
        //Enable Log
        prvCpssErrorLogEnableSet(0);

        /*use 512 physical ports to support 256+2 MACs and CPU-SDMA port(s) */
        cpssPpPhase1Info.maxNumOfPhyPortsToUse = cpssHalGetSKUMaxPhyPorts(devNum);

        /* state the CPSS to not delete the DB about the device even though it failed the ‘phase 1 init’ function */
        allow_phase1_fail_without_cleanup_set(1);
        uint8_t retry = 0;

        /* devType is retrieved in hwPpPhase1Part1*/
        while ((rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info,
                                            &devType))  == GT_HW_ERROR_NEED_RESET)
        {
            if (retry++ >= 3)
            {
                cpssOsPrintf("cpssDxChHwPpPhase1Init retry exceeded for dev %d with rc %d\n",
                             devNum, rc);
                return rc;
            }
            cpssHalResetPp(devNum, FALSE, xpDevType);
            osTimerWkAfter(1000);
        }

        cpssOsPrintf("cpssDxChHwPpPhase1Init success on dev %d retry %d for rc %d\n",
                     devNum, retry,
                     rc);
    }
    else
    {

        /* devType is retrieved in hwPpPhase1Part1*/
        uint8_t retry = 0;
        while (retry < 3)
        {
            rc = cpssDxChHwPpPhase1Init_new(&cpssPpPhase1Info, &devType);

            /* HW reset status is determined from reading user defined register.
               Commented Soft-reset in init flow as the scenarios are handled by drv.
               Phase1 init failure handling with soft-reset is required and can be enhanced later.
               For now, just retry.*/
#ifdef RETRY_PP_SOFT_RESET
            devInitStatus = cpssHalCheckIsHwResetDone(devNum);
#endif
            if ((rc != GT_OK)
#ifdef RETRY_PP_SOFT_RESET
                || ((rc == GT_OK) && (devInitStatus != GT_OK))
#endif
               )
            {
#ifdef RETRY_PP_SOFT_RESET
                cpssOsPrintf("Trigger Soft Reset.rc :%d devInitStatus :%d \n", rc,
                             devInitStatus);

                /* Save Dma registers before soft reset */
                if (devNum == 0)
                {
                    rc = cpssHalReadWriteRestoreReg(devNum, dev0regsToRestore, GT_TRUE);
                    cpssOsPrintf(" Read Restore REG dev=%d  ret=%d \n", devNum, rc);

                    printRestroreRegValues(devNum, dev0regsToRestore);
                }
                else
                {
                    rc = cpssHalReadWriteRestoreReg(devNum, dev1regsToRestore, GT_TRUE);
                    cpssOsPrintf(" Read Restore REG dev=%d  ret=%d \n", devNum, rc);

                    printRestroreRegValues(devNum, dev1regsToRestore);
                }

                rc = cpssDxChHwPpSoftResetSkipParamSet(devNum,
                                                       CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
                cpssOsPrintf("cpssDxChHwPpSoftResetSkipParamSet ret=%d dev=%d\n",
                             rc, devNum);
                rc = cpssDxChHwPpSoftResetTrigger(devNum);
                cpssOsPrintf("cpssDxChHwPpSoftResetTrigger ret=%d dev=%d\n",
                             rc, devNum);
                osTimerWkAfter(1000);
                rc = cpssDxChCfgDevRemove(devNum);
                cpssOsPrintf("cpssDxChCfgDevRemove ret=%d dev=%d\n", rc, devNum);
#endif
            }
            else
            {
                break;
            }
            osTimerWkAfter(1000);
            retry++;
        }
        cpssOsPrintf("cpssDxChHwPpPhase1Init Sucess in retry %d for dev %d\n", retry,
                     devNum);
#ifdef RETRY_PP_SOFT_RESET
        /* Write back saved DMA reg values. */
        if (retry != 0)
        {
            if (devNum == 0)
            {
                rc = cpssHalReadWriteRestoreReg(devNum, dev0regsToRestore, GT_FALSE);
                cpssOsPrintf(" Write Restore REG dev=%d  ret=%d \n", devNum, rc);

                printRestroreRegValues(devNum, dev0regsToRestore);
            }
            else
            {
                rc = cpssHalReadWriteRestoreReg(devNum, dev1regsToRestore, GT_FALSE);
                cpssOsPrintf(" Write Restore REG dev=%d  ret=%d \n", devNum, rc);

                printRestroreRegValues(devNum, dev1regsToRestore);
            }
        }

        /* Set HW reset status in user defined register. */
        rc = cpssHalWriteHwResetDone(devNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalWriteHwResetDone ret=%d dev=%d\n", rc, devNum);
            return rc;
        }
#endif
    }
    /* comment due to latest cpss
     * initialize service cpu
    if (mrvSupportedDevData[ASICDataIndex].needServiceCpu){
        CPSS_RESOURCE_MAPPING_STC resourceMapping;
        osMemSet(&resourceMapping,0,sizeof(resourceMapping));

        resourceMapping.sramBaseAddress = pciInfo.resourceMapping.sram.base;
        resourceMapping.sramSize        = (GT_U32)pciInfo.resourceMapping.sram.size;
        rc = cpssHwInitResourceMappingSet(devNum, &resourceMapping);
        if (rc != GT_OK) {
            cpssOsTimerWkAfter(2000);
            return rc;
        }
    }
    */

    /* remap ports */
    if (mrvSupportedDevData[ASICDataIndex].needDevMapping)
    {
        /* Get device info */
        rc = cpssDxChCfgDevInfoGet(devNum, &cpssDevInfo);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChCfgDevInfoGet", rc);
            return rc;
        }
        cpssOsMemSet(portMapArray, 0, sizeof(portMapArray));
        i=0;
        port_index = 0;
        while (profile[i].profileType != PROFILE_TYPE_LAST_E)
        {
            if ((profile[i].profileType == PROFILE_TYPE_PORT_MAP_E) ||
                (profile[i].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
            {
                portMapArray[port_index].physicalPortNumber =
                    profile[i].profileValue.portMap.portNum;  /* new number*/
                portMapArray[port_index].interfaceNum =
                    profile[i].profileValue.portMap.macNum; /* mac*/
                portMapArray[port_index].txqPortNumber = profile[i].profileValue.portMap.txQNum;
                portMapArray[port_index].mappingType =
                    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
                portMapArray[port_index].portGroup = 0;
                portMapArray[port_index].tmEnable = GT_FALSE;
                portMapArray[port_index].tmPortInd = GT_NA;
                port_index++;
            }
            else if (profile[i].profileType == PROFILE_TYPE_CPU_PORT_MAP_E)
            {
                // CPSS_CPU_PORT_NUM_CNS
                portMapArray[port_index].physicalPortNumber =
                    profile[i].profileValue.portMap.portNum;
                if (IS_DEVICE_FALCON(xpDevType) || IS_DEVICE_AC5X(xpDevType))
                {
                    portMapArray[port_index].interfaceNum = profile[i].profileValue.portMap.macNum;
                }
                else
                {
                    portMapArray[port_index].interfaceNum = GT_NA;
                }
                portMapArray[port_index].txqPortNumber = profile[i].profileValue.portMap.txQNum;
                portMapArray[port_index].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
                portMapArray[port_index].portGroup = 0;
                portMapArray[port_index].tmEnable = GT_FALSE;
                portMapArray[port_index].tmPortInd = GT_NA;
                port_index++;
            }
            i++;
        }

        rc = cpssDxChPortPhysicalPortMapSet(devNum, port_index, portMapArray);
        if (GT_OK != rc)
        {
            MRVL_HAL_API_TRACE("cpssDxChPortPhysicalPortMapSet", rc);
            return rc;
        }
    }

    if (IS_DEVICE_FALCON(xpDevType))
    {

        rc = falcon_initPortDelete_WA(devNum, port_index, portMapArray);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("falcon_initPortDelete_WA", rc);
            return rc;
        }

    }
    rc = cpssDxChPortManagerInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortManagerInit", rc);
        return rc;
    }

    if (IS_DEVICE_FALCON(xpDevType))
    {
        CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC   globalParamsStc;
        cpssOsMemSet(&globalParamsStc, 0, sizeof(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC));
        globalParamsStc.rxTermination = 1; /* AVDD */
        globalParamsStc.globalParamsBitmapType =
            CPSS_PORT_MANAGER_GLOBAL_PARAMS_RX_TERM_E;
        rc = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParamsStc);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChPortManagerGlobalParamsOverride", rc);
            return rc;
        }
    }

    //Disable Log
    prvCpssErrorLogEnableSet(0);
    /* WA init */
    for (i=0; i < CPSS_DXCH_IMPLEMENT_WA_LAST_E; i++)
    {
        if (mrvSupportedDevData[ASICDataIndex].waList[i] ==
            CPSS_DXCH_IMPLEMENT_WA_LAST_E)
        {
            break;    /* count wa */
        }
    }
    if (i)
    {
        rc = cpssDxChHwPpImplementWaInit(devNum, i,
                                         mrvSupportedDevData[ASICDataIndex].waList, NULL);
        MRVL_HAL_API_TRACE("cpssDxChHwPpImplementWaInit", rc);
    }

#ifndef ASIC_SIMULATION
    if (IS_DEVICE_FALCON(xpDevType))
    {
        /* Configure DMA */

        rc = prvNoKmDrv_configure_dma_per_devNum(devNum, 0/*portGroup*/);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("prvNoKmDrv_configure_dma_per_devNum", rc);
        }
    }
    else if (IS_DEVICE_AC5X(xpDevType))
    {
        rc = prvNoKmDrv_configure_dma_internal_cpu(devNum);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("prvNoKmDrv_configure_dma_internal_cpu", rc);
        }
    }
#endif

    /* Phase 2 */
    cpssOsMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));
    cpssPpPhase2Info.auMessageLength =
        CPSS_AU_MESSAGE_LENGTH_8_WORDS_E; // prevent failure of cpssDxChHwPpPhase2Init
    cpssPpPhase2Info.noTraffic2CPU = GT_FALSE;
    cpssPpPhase2Info.useMultiNetIfSdma = useMultiNetIfSdma;
    cpssPpPhase2Info.newDevNum = devNum;
    cpssPpPhase2Info.useDoubleAuq = GT_FALSE;

    rc = cpssDxChHwTxDescSizeGet(devType, &txDescSize);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwTxDescSizeGet", rc);
        return rc;
    }
    rc = cpssDxChHwRxDescSizeGet(devType, &rxDescSize);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwRxDescSizeGet", rc);
        return rc;
    }

    rc = cpssDxChHwAuDescSizeGet(devType, &auDescSize);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwAuDescSizeGet", rc);
        return rc;
    }

    if (useMultiNetIfSdma == GT_TRUE)
    {
        /* packet generator mode : Default number of Tx buffer size */
        GT_U32  txGenBuffSize = 128 + 16; /* 16 bytes of eDsa tag */
        /* packet generator mode : Default number of descriptors and buffers */
        GT_U32  txGenNumOfDescBuff = 512;

        /* fill info into cpssPpPhase2Info.netIfCfg that relevant to this mode */
        rc = appDemoAllocateDmaMem_useMultiNetIfSdma(
                 devNum,
                 devType,
                 cpssHalSys_param_rxDescNum(xpDevType),
                 cpssHalSys_param_rxBuffSize(xpDevType),
                 RX_BUFF_ALLIGN_DEF,
                 cpssHalSys_param_txDescNum(xpDevType),/* non-packet-generator-mode */
                 txGenBuffSize,
                 txGenNumOfDescBuff,
                 &cpssPpPhase2Info);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("appDemoAllocateDmaMem_useMultiNetIfSdma", rc);
            return rc;
        }
    }
    else
    {
        cpssPpPhase2Info.netIfCfg.txDescBlockSize =
            cpssHalSys_param_txDescNum(xpDevType) * txDescSize;
        cpssPpPhase2Info.netIfCfg.txDescBlock = osCacheDmaMalloc(
                                                    cpssPpPhase2Info.netIfCfg.txDescBlockSize);
        if (cpssPpPhase2Info.netIfCfg.txDescBlock == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA allocate memory failed for tx descriptor block ");
            return XP_ERR_NULL_POINTER;
        }

        cpssPpPhase2Info.netIfCfg.rxDescBlockSize =
            cpssHalSys_param_rxDescNum(xpDevType) * rxDescSize;
        cpssPpPhase2Info.netIfCfg.rxDescBlock = osCacheDmaMalloc(
                                                    cpssPpPhase2Info.netIfCfg.rxDescBlockSize);
        if (cpssPpPhase2Info.netIfCfg.rxDescBlock == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA allocate memory failed for rx descriptor block");
            return XP_ERR_NULL_POINTER;
        }

        cpssPpPhase2Info.netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;

        cpssPpPhase2Info.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize =
            cpssHalSys_param_rxBuffSize(xpDevType) * cpssHalSys_param_rxBuffNum(xpDevType);
        cpssPpPhase2Info.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr =
            osCacheDmaMalloc(
                cpssPpPhase2Info.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize);
        if (cpssPpPhase2Info.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr ==
            NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA allocate memory failed for rx buffer block");
            return XP_ERR_NULL_POINTER;
        }
    }

    cpssPpPhase2Info.netIfCfg.rxBufInfo.buffersInCachedMem = GT_FALSE;

    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[0] =
        cpssHalSys_param_rxBufferPercentage_0(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[1] =
        cpssHalSys_param_rxBufferPercentage_1(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[2] =
        cpssHalSys_param_rxBufferPercentage_2(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[3] =
        cpssHalSys_param_rxBufferPercentage_3(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[4] =
        cpssHalSys_param_rxBufferPercentage_4(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[5] =
        cpssHalSys_param_rxBufferPercentage_5(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[6] =
        cpssHalSys_param_rxBufferPercentage_6(xpDevType);
    cpssPpPhase2Info.netIfCfg.rxBufInfo.bufferPercentage[7] =
        cpssHalSys_param_rxBufferPercentage_7(xpDevType);

    cpssPpPhase2Info.netIfCfg.rxBufInfo.headerOffset = 0;
    cpssPpPhase2Info.netIfCfg.rxBufInfo.rxBufSize    = cpssHalSys_param_rxBuffSize(
                                                           xpDevType);

    cpssPpPhase2Info.auqCfg.auDescBlockSize =
        cpssHalSys_param_auqDescNum(xpDevType) * auDescSize;
    cpssPpPhase2Info.auqCfg.auDescBlock = osCacheDmaMalloc(
                                              cpssPpPhase2Info.auqCfg.auDescBlockSize);
    if (cpssPpPhase2Info.auqCfg.auDescBlock == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " DMA allocate memory failed for AU descriptor block ");
        return XP_ERR_NULL_POINTER;
    }

    cpssPpPhase2Info.fuqUseSeparate = cpssHalSys_param_fuqUseSeparate(xpDevType);
    cpssPpPhase2Info.fuqCfg.auDescBlockSize = cpssHalSys_param_fuqDescBlockSize(
                                                  xpDevType);

    /* Lock the interrupts, this phase changes the interrupts nodes pool data  */
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey);

    rc = cpssDxChHwPpPhase2Init(devNum, &cpssPpPhase2Info);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChHwPpPhase2Init", rc);
        return rc;
    }

#if 0 // Not called in appDemo
    /* in order to avoid ERROR state when queue filled (the default state),
       we configure the SDMA to drop incoming frames when there is no resource */
    for (i = 0; i < cpssHalSys_param_maxNumOfQueues(xpDevType); i++)
    {
        rc = cpssDxChNetIfSdmaRxResourceErrorModeSet((GT_U8)devNum, (GT_U8) i,
                                                     CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChNetIfSdmaRxResourceErrorModeSet", rc);
            return rc;
        }
    }
#endif

    /* Lock the interrupts, this phase changes the interrupts nodes pool data  */
    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);

    /* logical init */
    CPSS_DXCH_PP_CONFIG_INIT_STC ppConfigInit;
    osMemSet(&ppConfigInit, 0,  sizeof(CPSS_DXCH_PP_CONFIG_INIT_STC));
    ppConfigInit.routingMode = cpssHalSys_param_ppRoutingMode(xpDevType);
    ppConfigInit.maxNumOfPbrEntries = GetPbrMaxNum();

    if (IS_DEVICE_FALCON(xpDevType))
    {
        /* next parameter relevant to sip6 devices (ignored by legacy devices) :
           we want to work with FDB 128K */
        switch (GetSharedProfileIdx())
        {
            case CPSS_HAL_MID_L3_MID_L2_NO_EM:
                {
                    ppConfigInit.sharedTableMode =
                        CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E;
                }
                break;
            case CPSS_HAL_MID_L3_MID_L2_MIN_EM:
                {
                    ppConfigInit.sharedTableMode =
                        CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
                }
                break;
            case CPSS_HAL_MAX_L3_MIN_L2_NO_EM:
                {
                    ppConfigInit.sharedTableMode =
                        CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
                }
                break;
            case CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM:
                {
                    ppConfigInit.sharedTableMode =
                        CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E;
                }
                break;
            default:
                MRVL_HAL_API_TRACE("Invalid Profile", GetSharedProfileIdx());
                return GT_FAIL;
        }
    }
    else if (IS_DEVICE_AC5X(xpDevType))
    {
#if defined(INCLUDE_MPD)
        /* PHY init using MPD */
        rc = cpssHalPhyMpdInit(devNum, xpDevType, profile);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalPhyMpdInit failed rc=%d\n", rc);
            MRVL_HAL_API_TRACE("cpssHalPhyMpdInit", rc);
            return rc;
        }
#endif
        ppConfigInit.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        /* don't care value for this device as it have no shared tables ! */
        ppConfigInit.sharedTableMode =
            CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
    }
    rc = cpssDxChCfgPpLogicalInit(devNum, &ppConfigInit);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChCfgPpLogicalInit", rc);
        return rc;
    }

    rc = cpssDxChPhyPortSmiInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPhyPortSmiInit", rc);
        return rc;
    }

    rc = cpssDxChBrgVlanInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgVlanInit", rc);
        return rc;
    }

    rc = cpssDxChBrgFdbInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgFdbInit", rc);
        return rc;
    }

    rc = cpssDxChBrgMcInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgMcInit", rc);
        return rc;
    }

    rc = cpssDxChBrgStpInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgStpInit", rc);
        return rc;
    }

    rc = cpssDxChPortStatInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortStatInit", rc);
        return rc;
    }

    /*dxChPortBufMgInit*/

    rc = cpssDxChPortTxInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortTxInit", rc);
        return rc;
    }
    rc = cpssDxChNetIfInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChNetIfInit", rc);
        return rc;
    }
    rc = cpssDxChPclInit(devNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPclInit", rc);
        return rc;
    }

    GT_U32 maxLag = cpssHalGetSKUMaxLAGrps(devNum);
    GT_U32 maxMemberPerLag = cpssHalGetSKUmaxLAGMbrPerGrp(devNum);

    /* We are fixing 8 members per trunk.
     * Device can have upto 16K members. Reserving first half for trunk and later for L2 ECMP.
     * Over-write, the maxLags read from port-profiles.
     */
    if ((maxLag * maxMemberPerLag) > (CPSS_HAL_GLOBAL_LAG_MAX_MEMBERS(devNum)-1))
    {
        maxLag = (CPSS_HAL_GLOBAL_LAG_MAX_MEMBERS(devNum)-1)/maxMemberPerLag;
    }

    /*
     * CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E  can have fixed 8 members
     * CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E can more than 8 members, but we have
     * to set the maxMember in API cpssDxChTrunkFlexInfoSet, before trunk create and member add.
     * With NATIVE, lag load balancing is not uniform.
     * For now, operate in FLEX mode, with 8 members for all trunks.
     * NOTE:
     *   At device level, max 16K members are supported in falcon.
     *   With more members in group, LAG scale will come down.
     */

    rc = cpssDxChTrunkInit(devNum, maxLag,
                           cpssHalSys_param_trunkMembersMode(xpDevType));
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChTrunkInit", rc);
        return rc;
    }

    /* Fix numMembers to 8.*/
    rc = cpssDxChTrunkFlexInfoSet(devNum, 0xFFFF/*hybrid mode indication*/,
                                  0, maxMemberPerLag);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChTrunkFlexInfoSet", rc);
        return rc;
    }

    /* Over write the CPSS max value, considering the supported max members per grp.*/

    if (cpssHalGetSKUMaxLAGrps(devNum) != maxLag)
    {
        if ((int)cpssHalSetSKUMaxLAGrps(maxLag) < 0)
        {
            MRVL_HAL_API_TRACE("cpssHalGetSKUMaxLAGrps", maxLag);
            return GT_FAIL;
        }
    }

    rc = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum, GT_TRUE, 1);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet", rc);
        return rc;
    }

    rc = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum, GT_TRUE, 0);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet", rc);
        return rc;
    }

    /* diable pp interrups */
    /*      rc = prvCpssDrvHwPpPortGroupWriteRegister(
            devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0x34 ,0);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("prvCpssDrvHwPpPortGroupWriteRegister reg 0x34 value 0", rc);
            return rc;
        }
    */

    return rc;
}
/*******************************************************************************
* cpssHalInitializeLanesSwapApi
*
* DESCRIPTION:
*       initialize Lanes Swap
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*       lanesSwapProfilePtr - Lanes Swap Profile structure
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
GT_STATUS cpssHalInitializeLanesSwap
(
    GT_U8               devNum,
    XP_DEV_TYPE_T       devType,
    PROFILE_STC        *lanesSwapProfilePtr
)
{
    GT_U32              laneItr;
    GT_STATUS           rc=GT_OK;

    if (IS_DEVICE_FALCON(devType))
    {
        laneItr=0;
        PROFILE_STC falcon_2T_lane_polarity[] =
#include "falcon_2T_lane_polarity.h"
            PROFILE_STC falcon_3_2T_lane_polarity_ebof[] =
#include "falcon_3_2T_lane_polarity_ebof.h"
                PROFILE_STC falcon_6_4T_lane_polarity[] =
#include "falcon_6_4T_lane_polarity.h"
                    PROFILE_STC falcon_12_8t_lane_polarity[] =
#include "falcon_12_8T_lane_polarity.h"
                        // Default belly2belly is FALSE
                        while (lanesSwapProfilePtr[laneItr].profileType != PROFILE_TYPE_LAST_E)
        {
            if ((lanesSwapProfilePtr[laneItr].profileType == PROFILE_TYPE_PORT_MAP_E) ||
                (lanesSwapProfilePtr[laneItr].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
            {
                /* in falcon lanes are in multiples of 8 */
                if ((lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum % 8) == 0)
                {
                    if ((IS_DEVICE_FALCON_6_4(devType)) ||
                        (IS_DEVICE_FALCON_3_2(devType)) ||
                        (IS_DEVICE_FALCON_12_8(devType)) ||
                        (IS_DEVICE_FALCON_2(devType)))
                    {
                        if (IS_DEVICE_FALCON_6_4(devType))
                        {
                            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,
                                                                   lanesSwapProfilePtr[laneItr].profileValue.portMap.portNum,
                                                                   &falcon_6_4T_MacToSerdesMap[ lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum /8]);
                        }
                        else if (IS_DEVICE_FALCON_12_8(devType))
                        {
                            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,
                                                                   lanesSwapProfilePtr[laneItr].profileValue.portMap.portNum,
                                                                   &falcon_Belly2Belly_MacToSerdesMap[ lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum /8]);
                        }
                        else if (IS_DEVICE_FALCON_3_2(devType))
                        {
                            if (IS_DEVICE_EBOF_PROFILE(devType))
                            {
                                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,
                                                                       lanesSwapProfilePtr[laneItr].profileValue.portMap.portNum,
                                                                       &falcon_3_2T_MacToSerdesMap[ lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum /8]);
                            }
                            else
                            {
                                /* set to default - 6.4 variant */
                                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,
                                                                       lanesSwapProfilePtr[laneItr].profileValue.portMap.portNum,
                                                                       &falcon_6_4T_MacToSerdesMap[ lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum /8]);
                            }
                        }
                        else if (IS_DEVICE_FALCON_2(devType))
                        {
                            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,
                                                                   lanesSwapProfilePtr[laneItr].profileValue.portMap.portNum,
                                                                   &falcon_2T4T_MacToSerdesMap[ lanesSwapProfilePtr[laneItr].profileValue.portMap.macNum /8]);

                        }
                        else
                        {
                            MRVL_HAL_API_NOTICE("!!WARNING!!ERROR!! Serdes MUX CONFIG is NOT Applied", rc);
                        }

                        if (rc != GT_OK)
                        {
                            MRVL_HAL_API_NOTICE("cpssDxChPortLaneMacToSerdesMuxSet", rc);
                        }
                    }
                }
            }
            laneItr++;
        }
        //take the polarity profile based on board type
        if (IS_DEVICE_FALCON_6_4(devType))
        {
            lanesSwapProfilePtr = falcon_6_4T_lane_polarity;
        }
        else if (IS_DEVICE_FALCON_12_8(devType))
        {
            lanesSwapProfilePtr = falcon_12_8t_lane_polarity;
        }
        else if (IS_DEVICE_FALCON_3_2(devType))
        {
            if (IS_DEVICE_EBOF_PROFILE(devType))
            {
                lanesSwapProfilePtr = falcon_3_2T_lane_polarity_ebof;
            }
            else /* default */
            {
                lanesSwapProfilePtr = falcon_6_4T_lane_polarity;
            }
        }
        else if (IS_DEVICE_FALCON_2(devType))
        {
            lanesSwapProfilePtr = falcon_2T_lane_polarity;
        }
    }

    laneItr=0;
    while (lanesSwapProfilePtr[laneItr].profileType != PROFILE_TYPE_LAST_E)
    {
        if (lanesSwapProfilePtr[laneItr].profileType == PROFILE_TYPE_LANE_SWAP_E)
        {
            rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                                   lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.laneNum,
                                                   lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.invertTx,
                                                   lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.invertRx);

            if (rc != GT_OK)
            {
                MRVL_HAL_API_TRACE("cpssDxChPortSerdesLanePolaritySet", rc);
                return rc;
            }
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "set lane num %d polarity Tx %d Rx %d \n",
                  lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.laneNum,
                  lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.invertTx,
                  lanesSwapProfilePtr[laneItr].profileValue.serdes_polarity.invertRx);
        }
        laneItr++;
    }

    return GT_OK;
}

/*******************************************************************************
* cpssHalInitializePort
*
* DESCRIPTION:
*       initialize port speed and mode
*       initialize mru
*       initialize smi/xsmi access
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*       portNum - ports start from 1 ...
*       speed   -  speed of port to be configured
*       mru - maximum recieve unit
*       interfaceMode - interface mode to be configured
*       smiInterface - smi interface potensial phy is connected to, use
*                       CPSS_PHY_SMI_INTERFACE_MAX_E to indicate no phy connected
*       smiAddress - smi address to get phy type from
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
GT_STATUS cpssHalInitializePort
(
    GT_U8                       devNum,
    XP_DEV_TYPE_T               xpDevType,
    GT_PHYSICAL_PORT_NUM            portNum,
    CPSS_PORT_SPEED_ENT         speed,
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode,
    GT_BOOL                     existPhy,
    CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    CPSS_PHY_XSMI_INTERFACE_ENT  xsmiInterface,
    GT_U32                      smiAddress,
    int                             isRxTxParamValid,
    CPSS_PORT_SERDES_RX_CONFIG_UNT  *rxParam,
    CPSS_PORT_SERDES_TX_CONFIG_UNT  *txParam
)
{
    GT_STATUS                       rc = GT_ERROR;
    CPSS_INTERFACE_INFO_STC         physicalInfo;
    GT_HW_DEV_NUM                   hwDevNum = (GT_HW_DEV_NUM)devNum;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    CPSS_PM_PORT_PARAMS_STC         portApParams;
    uint8_t                         lanenum;
    uint32_t                        cableLen;
    CPSS_PORT_FEC_MODE_ENT          cpssFecMode = CPSS_PORT_FEC_MODE_LAST_E;
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor;

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(devNum, portNum,
                                                             &physicalInfo);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgEportToPhysicalPortTargetMappingTableGet", rc);
        return rc;
    }
    if (physicalInfo.type == CPSS_INTERFACE_PORT_E)
    {
        physicalInfo.devPort.hwDevNum = hwDevNum;
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,  portNum,
                                                                 &physicalInfo);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", rc);
            return rc;
        }
    }

    /* set phy address and interface if exists */
    if (existPhy)
    {

        if (!IS_DEVICE_AC5X(xpDevType))
        {
            if (smiInterface != CPSS_PHY_SMI_INTERFACE_INVALID_E)
            {
                /*Set SMI interface*/
                rc = cpssDxChPhyPortSmiInterfaceSet(devNum, portNum, smiInterface);
                if (rc != GT_OK)
                {
                    MRVL_HAL_API_TRACE("cpssDxChPhyPortSmiInterfaceSet", rc);
                    return rc;
                }
            }
            else
            {
                cpssOsPrintf("%s:%d SHOULD NOT COME HERE dev %d port %d existPhy %d smi %d xsmi %d\n",
                             __func__, __LINE__,
                             devNum, portNum, existPhy, smiInterface, xsmiInterface);
                /*Set XSMI interface*/
                rc = cpssDxChPhyPortXSmiInterfaceSet(devNum, portNum, xsmiInterface);
                if (rc != GT_OK)
                {
                    MRVL_HAL_API_TRACE("cpssDxChPhyPortXSmiInterfaceSet", rc);
                    return rc;
                }
            }

            /* To do test on the board */
            /*Set PHY address*/
            rc = cpssDxChPhyPortAddrSet(devNum, portNum, smiAddress);
            if (rc != GT_OK)
            {
                MRVL_HAL_API_TRACE("cpssDxChPhyPortAddrSet", rc);
                //return rc;
            }

            if (IS_DEVICE_FUJITSU_SMALL(xpDevType)) 
            {
                divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E;
                rc = cpssDxChPhySmiMdcDivisionFactorSet(devNum, portNum, divisionFactor);
                if (rc != GT_OK)
                {
                    MRVL_HAL_API_TRACE("cpssDxChPhySmiMdcDivisionFactorSet", rc);
                    //return rc;
                }
            }
            rc = cpssDxChPortInbandAutoNegEnableSet(devNum, portNum, GT_TRUE);
            if (rc != GT_OK)
            {
                MRVL_HAL_API_TRACE("cpssDxChPortInbandAutoNegEnableSet", rc);
                //return rc;
            }
        }
    }


    /* set PM parameters */
    if (gEnableCpssLog)
    {
        cpssHalEnableLog(!gEnableCpssLog);
    }
    cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E,
                                            &portParams);
    if (gEnableCpssLog)
    {
        cpssHalEnableLog(gEnableCpssLog);
    }

    portParams.portParamsType.regPort.ifMode = interfaceMode;
    portParams.portParamsType.regPort.speed = speed;
    portParams.portParamsType.regPort.portAttributes.fecMode =
        CPSS_PORT_FEC_MODE_DISABLED_E;
    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);

    if (speed == CPSS_PORT_SPEED_1000_E)
    {
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);
        if (interfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR_E)
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.inbandEnable =
                GT_TRUE;
        if (!IS_DEVICE_FUJITSU_LARGE(xpDevType))
        {
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.duplexEnable =
                GT_TRUE;
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.speedEnable =
                GT_TRUE;
        }
        else
        {
            if (interfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR_E)
            {
                portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlEnable =
                    GT_TRUE;
                portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlPauseAdvertiseEnable =
                    GT_TRUE;
            }
        }
        if (IS_DEVICE_AC5X(xpDevType))
        {
#ifndef ASIC_SIMULATION
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable =
                GT_FALSE;
#else
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable =
                GT_TRUE;
#endif
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlEnable
                =
                    GT_FALSE;
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlPauseAdvertiseEnable
                =
                    GT_FALSE;
            portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlAsmAdvertiseEnable
                =
                    GT_FALSE;
            if (interfaceMode == CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E)
            {
                // Need to enabled FEC for AC5X 1G port which are based on 2.5G ports(32-47)
                portParams.portParamsType.regPort.portAttributes.fecMode =
                    CPSS_PORT_RS_FEC_MODE_ENABLED_E;
                CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
            }
        }
        else
        {
            if (!IS_DEVICE_FUJITSU_LARGE(xpDevType) && interfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR_E)
                portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable =
                    GT_TRUE;
        }
    }

    if (speed == CPSS_PORT_SPEED_2500_E && xpDevType != AC3XRAMAN && xpDevType != AC3XMCS)
    {
        portParams.portParamsType.regPort.portAttributes.fecMode =
            CPSS_PORT_RS_FEC_MODE_ENABLED_E;
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);

        portParams.portParamsType.regPort.portAttributes.autoNegotiation.inbandEnable =
            GT_TRUE;
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.duplexEnable =
            GT_TRUE;
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.speedEnable =
            GT_TRUE;
#ifndef ASIC_SIMULATION
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable =
            GT_FALSE;
#else
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable =
            GT_TRUE;
#endif
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlEnable
            =
                GT_FALSE;
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlPauseAdvertiseEnable
            =
                GT_FALSE;
        portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlAsmAdvertiseEnable
            =
                GT_FALSE;

    }


    /* Attributes */
    /* common param - FEC params */

    if (speed == CPSS_PORT_SPEED_50000_E)
    {
        if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR_E ||
             interfaceMode == CPSS_PORT_INTERFACE_MODE_CR_E))
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
        else
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
    }
    if (speed == CPSS_PORT_SPEED_100G_E)
    {
        if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR2_E ||
             interfaceMode == CPSS_PORT_INTERFACE_MODE_CR2_E))
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
        else if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR4_E ||
                  interfaceMode == CPSS_PORT_INTERFACE_MODE_CR4_E))
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
    }
    if (speed == CPSS_PORT_SPEED_200G_E)
    {
        if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR4_E ||
             interfaceMode == CPSS_PORT_INTERFACE_MODE_CR4_E))
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
        else if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR8_E ||
                  interfaceMode == CPSS_PORT_INTERFACE_MODE_CR8_E))
        {
            portParams.portParamsType.regPort.portAttributes.fecMode =
                CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
    }
    if (speed == CPSS_PORT_SPEED_400G_E)
    {
        portParams.portParamsType.regPort.portAttributes.fecMode =
            CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    }

    if (cpssHalIsEbofDevice(devNum))
    {
        cpssFecMode = cpssHalPortFecParamGet(devNum, portNum);
        if (cpssFecMode != CPSS_PORT_FEC_MODE_LAST_E)
        {
            portParams.portParamsType.regPort.portAttributes.fecMode = cpssFecMode;
            CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
        }
    }

    if ((interfaceMode == CPSS_PORT_INTERFACE_MODE_KR8_E) ||
        (interfaceMode == CPSS_PORT_INTERFACE_MODE_CR8_E))
    {
        for (lanenum = 0 ; lanenum <8 ; lanenum++)
        {
            portParams.portParamsType.regPort.laneParams[lanenum].validLaneParamsBitMask =
                0;

            portParams.portParamsType.regPort.laneParams[lanenum].globalLaneNum  = portNum +
                                                                                   lanenum;
        }

        //portParams.portParamsType.regPort.portAttributes.validAttrsBitMask = 0;
    }
    /* overriding default rx training to be enhanced tune */
    //portParams.portParamsType.regPort.portAttributes.trainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E;
    //CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);

    /* Fixed Speed Port */
    if (!cpssHalPortMacAnEnableGet(devNum, portNum))
    {

        CPSS_PORT_SERDES_RX_CONFIG_STC rxConfig;
        CPSS_PORT_SERDES_TX_CONFIG_STC txConfig;

        cableLen = cpssHalMacGetPortCableLen(devNum, portNum);
        //check for optic cable type
        if (cableLen == 3)
        {
            if (interfaceMode == CPSS_PORT_INTERFACE_MODE_KR_E ||
                interfaceMode == CPSS_PORT_INTERFACE_MODE_CR_E)
            {
                portParams.portParamsType.regPort.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            }

            if (interfaceMode == CPSS_PORT_INTERFACE_MODE_KR2_E ||
                interfaceMode == CPSS_PORT_INTERFACE_MODE_CR2_E)
            {
                portParams.portParamsType.regPort.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR2_E;
            }

            if (interfaceMode == CPSS_PORT_INTERFACE_MODE_KR4_E ||
                interfaceMode == CPSS_PORT_INTERFACE_MODE_CR4_E)
            {
                portParams.portParamsType.regPort.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            }

            if (interfaceMode == CPSS_PORT_INTERFACE_MODE_KR8_E ||
                interfaceMode == CPSS_PORT_INTERFACE_MODE_CR8_E)
            {
                portParams.portParamsType.regPort.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR8_E;
            }

        }

        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E);

        if (cableLen == 0)
        {
            portParams.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E;

        }
        else if (cableLen == 1)
        {
            portParams.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_DEFAULT_E;

        }
        else if (cableLen == 2)
        {
            portParams.portParamsType.regPort.portAttributes.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_2_E;

        }
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_PORT_ATTR_PREEMPTION_E);
        portParams.portParamsType.regPort.portAttributes.preemptionParams.type =
            CPSS_PM_MAC_PREEMPTION_DISABLED_E;

        rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChPortManagerPortParamsSet", rc);
            return rc;
        }

        if (isRxTxParamValid)
        {
            int lane, maxLanes = 1;
            switch (interfaceMode)
            {
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                case CPSS_PORT_INTERFACE_MODE_CR_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                    maxLanes = 1;
                    break;
                case CPSS_PORT_INTERFACE_MODE_KR2_E:
                case CPSS_PORT_INTERFACE_MODE_CR2_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
                    maxLanes = 2;
                    break;
                case CPSS_PORT_INTERFACE_MODE_KR4_E:
                case CPSS_PORT_INTERFACE_MODE_CR4_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
                    maxLanes = 4;
                    break;
                case CPSS_PORT_INTERFACE_MODE_KR8_E:
                case CPSS_PORT_INTERFACE_MODE_CR8_E:
                case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
                    maxLanes = 8;
                    break;
                default:
                    maxLanes = 1;
                    break;
            }
            for (lane = 0; lane < maxLanes; lane++)
            {
                if (isRxTxParamValid & 1)
                {
                    osMemSet(&rxConfig, 0, sizeof(rxConfig));
                    /* Configure seredes RX params */
                    rxConfig.type = CPSS_PORT_SERDES_AVAGO_E;
                    rxConfig.rxTune.avago.DC    = rxParam[lane].avago.DC   ;
                    rxConfig.rxTune.avago.BW    = rxParam[lane].avago.BW   ;
                    rxConfig.rxTune.avago.HF    = rxParam[lane].avago.HF   ;
                    rxConfig.rxTune.avago.LF    = rxParam[lane].avago.LF   ;
                    rxConfig.rxTune.avago.sqlch = rxParam[lane].avago.sqlch;
                    rxConfig.rxTune.avago.minLf = rxParam[lane].avago.minLf;
                    rxConfig.rxTune.avago.maxLf = rxParam[lane].avago.maxLf;
                    rxConfig.rxTune.avago.minHf = rxParam[lane].avago.minHf;
                    rxConfig.rxTune.avago.maxHf = rxParam[lane].avago.maxHf;

                    rc = cpssDxChSamplePortManagerSerdesRxParamsSet(devNum, portNum, lane,
                                                                    &rxConfig);
                    if (rc != GT_OK)
                    {
                        MRVL_HAL_API_TRACE("cpssDxChSamplePortManagerSerdesRxParamsSet", rc);
                        //return rc;
                    }
                    cpssOsPrintf("Serdes Rx Param: portNum=%d Lane=%d Speed=%d IF=%d DC=%d BW=%d HF=%d LF=%d minLf=%d maxLf=%d minHf=%d maxHf=%d, sqlch=%d\n",
                                 portNum, lane, speed, interfaceMode,
                                 rxConfig.rxTune.avago.DC, rxConfig.rxTune.avago.BW, rxConfig.rxTune.avago.HF,
                                 rxConfig.rxTune.avago.LF, rxConfig.rxTune.avago.minLf,
                                 rxConfig.rxTune.avago.maxLf, rxConfig.rxTune.avago.minHf,
                                 rxConfig.rxTune.avago.maxHf, rxConfig.rxTune.avago.sqlch
                                );
                }
                if (isRxTxParamValid & 2)
                {
                    osMemSet(&txConfig, 0, sizeof(txConfig));
                    /* Configure seredes TX params */
                    txConfig.type = CPSS_PORT_SERDES_AVAGO_E;
                    txConfig.txTune.avago.post  = txParam[lane].avago.post;
                    txConfig.txTune.avago.pre   = txParam[lane].avago.pre;
                    txConfig.txTune.avago.pre2  = txParam[lane].avago.pre2;
                    txConfig.txTune.avago.pre3  = txParam[lane].avago.pre3;
                    txConfig.txTune.avago.atten = txParam[lane].avago.atten;

                    rc = cpssDxChSamplePortManagerSerdesTxParamsSet(devNum, portNum, lane,
                                                                    &txConfig);
                    if (rc != GT_OK)
                    {
                        MRVL_HAL_API_TRACE("cpssDxChSamplePortManagerSerdesTxParamsSet", rc);
                        //return rc;
                    }
                    cpssOsPrintf("Serdes Tx Param: portNum=%d Lane=%d Speed=%d IF=%d post=%d pre=%d pre2=%d pre3=%d atten=%d\n",
                                 portNum, lane, speed, interfaceMode,
                                 txConfig.txTune.avago.post, txConfig.txTune.avago.pre,
                                 txConfig.txTune.avago.pre2,
                                 txConfig.txTune.avago.pre3, txConfig.txTune.avago.atten
                                );
                }
            }
        }

        /* set PM event create */
        CPSS_PORT_MANAGER_STC portEventStc;
        if (IS_DEVICE_AC5X(xpDevType))
        {
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E;
        }
        else
        {
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
        }
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerEventSet devNum %d portNum %d speed %d, ifmode %d\n",
                         devNum, portNum, speed, interfaceMode);
            MRVL_HAL_API_TRACE("cpssDxChPortManagerEventSet", rc);
            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
            MRVL_HAL_API_TRACE("cpssDxChPortManagerEventSet retry", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else   /* AP Enabled Port */
    {
        CPSS_PORT_FEC_MODE_ENT          fecSupported, fecRequested;
        // Override KR to CR as ixia doesn't work with KR4
        if (interfaceMode == CPSS_PORT_INTERFACE_MODE_KR4_E &&
            speed == CPSS_PORT_SPEED_100G_E)
        {
            interfaceMode  = CPSS_PORT_INTERFACE_MODE_CR4_E;
        }


        fecSupported = fecRequested =
                           portParams.portParamsType.regPort.portAttributes.fecMode;
        rc = cpssDxChSamplePortManagerApMandatoryParamsSet(devNum, portNum,
                                                           interfaceMode, speed, fecSupported, fecRequested);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChSamplePortManagerApMandatoryParamsSet", rc);
            return rc;
        }

        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portApParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[cpssDxChPortManagerPortParamsGet] can't read port params. dev %d port %d\n",
                         devNum, portNum);
            MRVL_HAL_API_TRACE("cpssDxChSamplePortManagerApMandatoryParamsSet", rc);
            return rc;
        }
        //set cable length.
        cableLen = cpssHalMacGetPortCableLen(devNum, portNum);
        cpssOsPrintf("setting len for cable in init port num %d portNum, cable len %d ",
                     portNum, cableLen);

        if (cableLen == 1)
        {
            portApParams.portParamsType.apPort.apAttrs.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_DEFAULT_E;
            CPSS_PM_SET_VALID_AP_ATTR(&portParams,
                                      CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
        }
        else if (cableLen == 2)
        {
            portApParams.portParamsType.apPort.apAttrs.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_2_E;
            CPSS_PM_SET_VALID_AP_ATTR(&portParams,
                                      CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
        }
        else if (cableLen == 0)
        {
            portApParams.portParamsType.apPort.apAttrs.interconnectProfile =
                CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_1_E;
            CPSS_PM_SET_VALID_AP_ATTR(&portParams,
                                      CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);
        }

        rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portApParams);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerParamsSet devNum %d portNum %d speed %d, ifmode %d\n",
                         devNum, portNum, speed, interfaceMode);
            MRVL_HAL_API_TRACE("cpssDxChPortManagerParamsSet", rc);
            return rc;
        }
        CPSS_PORT_MANAGER_STC portEventStc;
        //portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_AND_FORCE_LINK_DOWN_E;
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChPortManagerEventSet devNum %d portNum %d speed %d, ifmode %d\n",
                         devNum, portNum, speed, interfaceMode);
            MRVL_HAL_API_TRACE("cpssDxChPortManagerEventSet", rc);
            return rc;
        }

        rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, (GT_PHYSICAL_PORT_NUM) portNum,
                                                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChBrgEgrFltPortLinkEnableSet", rc);
            return rc;
        }
    }
    /* set MRU */
    rc = cpssDxChPortMruSet(devNum, portNum, cpssHalSys_param_mru(xpDevType));
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortMruSet", rc);
        return rc;
    }

    /*
    * JIRA-300 : FlowControlAutoNeg was enabled for 1G and disabled for 10G ports.
    * This is was enabling FlowControlRX for 1G and traffic was paused on receving
    * 802_3x frames.
    * As per requirement, disabling the Flow control Autoneg mode and FlowControlEnable on
      both TX and RX.
    * Forcing this on both 1G and 10G ports.
    * Current Port Config: FlowControlMode = CPSS_DXCH_PORT_FC_MODE_802_3X_E
    *                 FlowControlAutoNeg = GT_FALSE
    *                 FlowControlEnable = CPSS_PORT_FLOW_CONTROL_DISABLE_E
    *  commnted as of now as recieved GT_NOT_SUPPORTED
    */

#if 0
    rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, portNum, GT_FALSE,
                                               GT_FALSE);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortFlowCntrlAutoNegEnableSet", rc);
        return rc;
    }
#endif

    /* JIRA LARCH-16
     * We need to enable Bypass for 2.5G CPU port
     * on RAMAN and MCS boards
     */
    if (speed == CPSS_PORT_SPEED_2500_E && (xpDevType == AC3XRAMAN || xpDevType == AC3XMCS))
    {

        rc = cpssDxChPortInBandAutoNegBypassEnableSet(devNum, portNum,
                                                      GT_TRUE);
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE("cpssDxChPortInBandAutoNegBypassEnableSet", rc);
            return rc;
        }
    }
    rc = cpssDxChPortFlowControlEnableSet(devNum, portNum,
                                          CPSS_PORT_FLOW_CONTROL_DISABLE_E);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChPortFlowControlEnableSet", rc);
        return rc;
    }

    rc = cpssDxChBrgVlanPortAccFrameTypeSet(devNum, portNum,
                                            CPSS_PORT_ACCEPT_FRAME_ALL_E);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChBrgVlanPortAccFrameTypeSet", rc);
        return rc;
    }

    /* save port as supported */
    MRVSetPort(portNum);

    return GT_OK;
}

typedef enum GT_PHY_WRITE_TYPE_ENT
{
    AD_FIRST_PORT = 0
    , AD_ALL_PORTS  = 1
    , AD_BAD_PORT  = -1
} GT_PHY_WRITE_TYPE_ENT;

typedef struct
{
    GT_PHY_WRITE_TYPE_ENT allPorts;
    GT_U8                 phyRegAddr;
    GT_U16                phyRegData;
} APP_DEMO_QUAD_PHY_CFG_STC;


static GT_STATUS macSec_PTP_disable_PHY_88E1680M_RevA0_Init
(
    GT_U8                 devNum,
    GT_PHYSICAL_PORT_NUM  firstPort
)
{
    GT_STATUS       rc;           /* return code */
    GT_U32          ii;           /* iterator */
    GT_PHYSICAL_PORT_NUM ppPort;  /* port of PP for PHY configuration */
    GT_U8           phyRegAddr;   /* PHY reg address */
    GT_U16          phyRegData;   /* PHY reg data */
    static APP_DEMO_QUAD_PHY_CFG_STC
    macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MTL_Init_Array [] =
    {
        /* for all/first port, regaddr, regvalue, delay if need in ms */
        { AD_ALL_PORTS,  22, 0x0000 } /* ensure we on page 0 */
        , { AD_ALL_PORTS,   4, 0x01E1 } /* RW u1 P0-3 R4 H01E1 - restore default of register #4
                                          that may be overridden by BC2 B0 during power-up */
        , { AD_ALL_PORTS,  16, 0x3370 } /* PHY power up (reg 0x10_0.2=0 and reg 0x0_0.11=0) */
        , { AD_ALL_PORTS,   0, 0x1140 }
        , { AD_ALL_PORTS,  22, 0x0003 } /* Link indication config */
        , { AD_ALL_PORTS,  16, 0x1117 }
        , { AD_ALL_PORTS,  22, 0x0004 } /* Link indication config */
        , { AD_ALL_PORTS,  27, 0x3FA0 }
        , { AD_ALL_PORTS,  22, 0x0000 }
        , { AD_ALL_PORTS,  22, 0x0012 } /* MACSec and PTP disable */
        , { AD_ALL_PORTS,  27, 0x0000 }
        , { AD_ALL_PORTS,  22, 0x0000 }
        , { AD_ALL_PORTS,  22, 0x00FD } /* QSGMII Tx Amp change */
        , { AD_ALL_PORTS,   8, 0x0B53 }
        , { AD_ALL_PORTS,   7, 0x200D }
        , { AD_ALL_PORTS,  22, 0x00FF } /* EEE Initialization */
        , { AD_ALL_PORTS,  17, 0xB030 }
        , { AD_ALL_PORTS,  16, 0x215C }
        , { AD_ALL_PORTS,  22, 0x0000 }
        , { AD_ALL_PORTS,   0, 0x9140 } /* Soft Reset */
    };

    APP_DEMO_QUAD_PHY_CFG_STC     *configArrayPtr =
        &macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MTL_Init_Array[0];
    GT_U32 numOfConfigs = sizeof(
                              macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MTL_Init_Array)/sizeof(
                              macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MTL_Init_Array[0]);

    ppPort = firstPort;
    for (ii = 0; ii < numOfConfigs; ii++)
    {
        phyRegAddr = (GT_U8)configArrayPtr[ii].phyRegAddr;
        phyRegData = configArrayPtr[ii].phyRegData;
        //cpssOsPrintf("\n            PP port[%d] Reg[%d] Data[%04X] ... ", ppPort,phyRegAddr, phyRegData);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, ppPort, phyRegAddr, phyRegData);
        //CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
        if (rc != GT_OK)
        {
            cpssOsPrintf("ERROR : macSec_PTP_disable_PHY_88E1680M_RevA0_Init  rc%d: portNum[%d] PPort[%d] Reg[%d] Data[%04X]\n",
                         rc, firstPort, ppPort, phyRegAddr, phyRegData);
            return rc;
        }
    }
    return GT_OK;
}

static GT_STATUS fujitsu_small_init_phy_0
(
    GT_U8                 devNum,
    GT_PHYSICAL_PORT_NUM  firstPort
)
{
    GT_STATUS       rc;           /* return code */
    GT_U32          ii;           /* iterator */
    GT_PHYSICAL_PORT_NUM ppPort;  /* port of PP for PHY configuration */
    GT_U8           phyRegAddr;   /* PHY reg address */
    GT_U16          phyRegData;   /* PHY reg data */
    static APP_DEMO_QUAD_PHY_CFG_STC
    fujitsu_small_init_phy_0_Init_Array [] =
    {
        { AD_ALL_PORTS,  0x00, 0x9140 } ,
        { AD_ALL_PORTS,  0x1c, 0x8806 } ,
        { AD_ALL_PORTS,  0x1c, 0x0806 } ,
        { AD_ALL_PORTS,  0x1c, 0xb4a0 } ,
        { AD_ALL_PORTS,  0x1c, 0x34a0 } ,
        { AD_ALL_PORTS,  0x17, 0x0f04 } ,
        { AD_ALL_PORTS,  0x15, 0x0100 } ,
        { AD_ALL_PORTS,  0x1c, 0xfc08 } ,
        { AD_ALL_PORTS,  0x1c, 0x7c08 } ,
        { AD_ALL_PORTS,  0x04, 0x01e1 } ,
        { AD_ALL_PORTS,  0x09, 0x0200 } ,
        { AD_ALL_PORTS,  0x00, 0x1340 } ,
        { AD_ALL_PORTS,  0x18, 0x0901 } ,
        { AD_ALL_PORTS,  0x18, 0x1007 } 
    };

    APP_DEMO_QUAD_PHY_CFG_STC     *configArrayPtr =
        &fujitsu_small_init_phy_0_Init_Array[0];
    GT_U32 numOfConfigs = sizeof(
                              fujitsu_small_init_phy_0_Init_Array)/sizeof(
                              fujitsu_small_init_phy_0_Init_Array[0]);

    ppPort = firstPort;
    for (ii = 0; ii < numOfConfigs; ii++)
    {
        phyRegAddr = (GT_U8)configArrayPtr[ii].phyRegAddr;
        phyRegData = configArrayPtr[ii].phyRegData;
        //cpssOsPrintf("\n            PP port[%d] Reg[%d] Data[%04X] ... ", ppPort,phyRegAddr, phyRegData);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, ppPort, phyRegAddr, phyRegData);
        //CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
        if (rc != GT_OK)
        {
            cpssOsPrintf("ERROR : fujitsu_small_init_phy_0_Init  rc%d: portNum[%d] PPort[%d] Reg[%d] Data[%04X]\n",
                         rc, firstPort, ppPort, phyRegAddr, phyRegData);
            return rc;
        }
    }
    return GT_OK;
}


static GT_STATUS fujitsu_small_init_phy_1
(
    GT_U8                 devNum,
    GT_PHYSICAL_PORT_NUM  firstPort
)
{
    GT_STATUS       rc;           /* return code */
    GT_U32          ii;           /* iterator */
    GT_PHYSICAL_PORT_NUM ppPort;  /* port of PP for PHY configuration */
    GT_U8           phyRegAddr;   /* PHY reg address */
    GT_U16          phyRegData;   /* PHY reg data */
    static APP_DEMO_QUAD_PHY_CFG_STC
    fujitsu_small_init_phy_1_Init_Array [] =
    {
        { AD_ALL_PORTS,  0x00, 0x9140 } ,
        { AD_ALL_PORTS,  0x1c, 0xf9e6 } ,
        { AD_ALL_PORTS,  0x1c, 0x79e6 } ,
        { AD_ALL_PORTS,  0x1c, 0xb863 } ,
        { AD_ALL_PORTS,  0x1c, 0x3863 } ,
        { AD_ALL_PORTS,  0x1c, 0x8806 } ,
        { AD_ALL_PORTS,  0x1c, 0x0806 } ,
        { AD_ALL_PORTS,  0x1c, 0xb40a } ,
        { AD_ALL_PORTS,  0x1c, 0x340a } ,
        { AD_ALL_PORTS,  0x17, 0x0f04 } ,
        { AD_ALL_PORTS,  0x15, 0x0108 } ,
        { AD_ALL_PORTS,  0x1c, 0xfc0b } ,
        { AD_ALL_PORTS,  0x1c, 0x7c0b } ,
        { AD_ALL_PORTS,  0x04, 0x0060 } ,
        { AD_ALL_PORTS,  0x00, 0x1340 } ,
        { AD_ALL_PORTS,  0x1c, 0xcc0b } ,
        { AD_ALL_PORTS,  0x1c, 0x4c0b } 
    };

    APP_DEMO_QUAD_PHY_CFG_STC     *configArrayPtr =
        &fujitsu_small_init_phy_1_Init_Array[0];
    GT_U32 numOfConfigs = sizeof(
                              fujitsu_small_init_phy_1_Init_Array)/sizeof(
                              fujitsu_small_init_phy_1_Init_Array[0]);

    ppPort = firstPort;
    for (ii = 0; ii < numOfConfigs; ii++)
    {
        phyRegAddr = (GT_U8)configArrayPtr[ii].phyRegAddr;
        phyRegData = configArrayPtr[ii].phyRegData;
        //cpssOsPrintf("\n            PP port[%d] Reg[%d] Data[%04X] ... ", ppPort,phyRegAddr, phyRegData);
        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, ppPort, phyRegAddr, phyRegData);
        //CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
        if (rc != GT_OK)
        {
            cpssOsPrintf("ERROR : fujitsu_small_init_phy_1_Init  rc%d: portNum[%d] PPort[%d] Reg[%d] Data[%04X]\n",
                         rc, firstPort, ppPort, phyRegAddr, phyRegData);
            return rc;
        }
    }
    return GT_OK;
}
/*******************************************************************************
* cpssHalInitializePortsApi
*
* DESCRIPTION:
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*       lanesSwapProfilePtr - Lanes Swap Profile structure
*
* OUTPUTS:
*        None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
GT_STATUS cpssHalInitializePorts
(
    GT_U8           devNum,
    PROFILE_STC         *profile,
    XP_DEV_TYPE_T       xpDevType
)
{
    GT_U32              laneItr;
    GT_STATUS           rc;
    GT_U32              ledPosition;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32  startPort = ~0, endPort = 0;
    int numPorts = 0;
    int maxPorts = 0;
    laneItr=0;
    ledPosition = 0;
    if (!IS_DEVICE_AC5X(xpDevType))
    {
        /* TODO AC5X init */
        //enable LED
        rc =  cpssHalLedInit(devNum, xpDevType, cpssHalLedProfileGet(devNum));
        if (rc != GT_OK)
        {
            MRVL_HAL_API_TRACE(" cpssHalLedConfig failed", rc);
        }
    }
    while (profile[laneItr].profileType != PROFILE_TYPE_LAST_E)
    {
        if ((profile[laneItr].profileType == PROFILE_TYPE_PORT_MAP_E) ||
            (profile[laneItr].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
        {
            switch ((profile[laneItr].profileValue.portMap.interfaceSpeed*PROFILE_SPEED_UNITS_CNS))
            {
                case 1000:
                    speed = CPSS_PORT_SPEED_1000_E;
                    break;
                case 2500:
                    speed = CPSS_PORT_SPEED_2500_E;
                    break;
                case 10000:
                    speed = CPSS_PORT_SPEED_10000_E;
                    break;
                case 25000:
                    speed = CPSS_PORT_SPEED_25000_E;
                    break;
                case 40000:
                    speed = CPSS_PORT_SPEED_40000_E;
                    break;
                case 50000:
                    speed = CPSS_PORT_SPEED_50000_E;
                    break;
                case 100000:
                    speed = CPSS_PORT_SPEED_100G_E;
                    break;
                case 200000:
                    speed = CPSS_PORT_SPEED_200G_E;
                    break;
                case 400000:
                    speed = CPSS_PORT_SPEED_400G_E;
                    break;
                default:
                    return GT_BAD_PARAM;
            }
            rc = cpssHalInitializePort(devNum, xpDevType,
                                       profile[laneItr].profileValue.portMap.portNum,
                                       speed,
                                       profile[laneItr].profileValue.portMap.interfaceMode,
                                       profile[laneItr].profileValue.portMap.phyInfo.existsPhy,
                                       profile[laneItr].profileValue.portMap.phyInfo.smiIf,
                                       profile[laneItr].profileValue.portMap.phyInfo.xSmiIf,
                                       profile[laneItr].profileValue.portMap.phyInfo.phyAddr,
                                       profile[laneItr].profileValue.portMap.isRxTxParamValid,
                                       profile[laneItr].profileValue.portMap.rxParam,
                                       profile[laneItr].profileValue.portMap.txParam
                                      );

            cpssOsPrintf("Port Init: portNum=%d Speed=%d IF=%d IsPhy=%d PhyAddr=%d smiIf=%d xsmiif=%d\n",
                         profile[laneItr].profileValue.portMap.portNum,
                         speed,
                         profile[laneItr].profileValue.portMap.interfaceMode,
                         profile[laneItr].profileValue.portMap.phyInfo.existsPhy,
                         profile[laneItr].profileValue.portMap.phyInfo.phyAddr,
                         profile[laneItr].profileValue.portMap.phyInfo.smiIf,
                         profile[laneItr].profileValue.portMap.phyInfo.xSmiIf
                        );
            if (rc != GT_OK)
            {
                MRVL_HAL_API_TRACE("cpssHalInitializePort", rc);
                MRVL_HAL_API_TRACE("CRITICAL: Ignoring Error ", rc);
                //return rc;
            }

            //Per port Led init
            if (speed >= CPSS_PORT_SPEED_10000_E || IS_DEVICE_FUJITSU_LARGE(xpDevType))
            {
                if (IS_DEVICE_FALCON(xpDevType) || xpDevType == ALDRIN2XL ||
                    IS_DEVICE_AC5X(xpDevType) || IS_DEVICE_FUJITSU_LARGE(xpDevType))
                {
                    ledPosition = -1; // Override position
                }
                rc = cpssHalLedPortConfig(devNum, xpDevType,
                                          profile[laneItr].profileValue.portMap.portNum, ledPosition,
                                          cpssHalLedProfileGet(0));
                ledPosition++;
                if (rc != GT_OK)
                {
                    MRVL_HAL_API_TRACE("cpssHalLedPortConfig failed", rc);
                }
            }
            if (profile[laneItr].profileValue.portMap.phyInfo.existsPhy)
            {
                if (startPort > profile[laneItr].profileValue.portMap.portNum)
                {
                    startPort = profile[laneItr].profileValue.portMap.portNum;
                }
                if (endPort < profile[laneItr].profileValue.portMap.portNum)
                {
                    endPort = profile[laneItr].profileValue.portMap.portNum;
                }
                if (!IS_DEVICE_AC5X(xpDevType) && !IS_DEVICE_FUJITSU_SMALL(xpDevType))
                {
                    /*TODO AC5X */
                    /* Do Phy initialization with static configuration
                    * Default AN enable
                    * Reset Phy
                    */
                    rc = macSec_PTP_disable_PHY_88E1680M_RevA0_Init(devNum,
                                                                    profile[laneItr].profileValue.portMap.portNum);
                    if (rc != GT_OK)
                    {
                        //TODO: Jira SONIC-63 Few ports mac write fails due to invalid port passed
                        //MRVL_HAL_API_TRACE("macSec_PTP_disable_PHY_88E1680M_RevA0_Init", rc);
                    }
                }
                else if (IS_DEVICE_FUJITSU_SMALL(xpDevType))
                {
                    switch (xpDevType)
                    {
                    case AC3XFS:
                        break;

                    case AC3XROB:
                        if (profile[laneItr].profileValue.portMap.portNum < 3)
                        {
                            rc = fujitsu_small_init_phy_0(devNum, profile[laneItr].profileValue.portMap.portNum);
                            if (rc != GT_OK)
                            {
                                MRVL_HAL_API_TRACE("fujitsu_small_init_phy_0 failed", rc);
                            }
                        } 
                        else if (profile[laneItr].profileValue.portMap.portNum == 3)
                        {
                            rc = fujitsu_small_init_phy_1(devNum, profile[laneItr].profileValue.portMap.portNum);
                            if (rc != GT_OK)
                            {
                                MRVL_HAL_API_TRACE("fujitsu_small_init_phy_1 failed", rc);
                            }
                        }
                        break;
                    
                    case AC3XMCS:
                        break;
                    
                    case AC3XRAMAN:
                        break;
                    
                    default:
                        break;
                    }   
                }
            }

            if (profile[laneItr].profileType == PROFILE_TYPE_PORT_MAP_E)
            {
                // Front panel ports
                numPorts++;
            }
            if (maxPorts<profile[laneItr].profileValue.portMap.portNum)
            {
                maxPorts = profile[laneItr].profileValue.portMap.portNum;
            }
        }
        else if ((profile[laneItr].profileType == PROFILE_TYPE_CPU_PORT_MAP_E))
        {
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
                                                    profile[laneItr].profileValue.portMap.portNum,
                                                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
            if (rc!=GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                      " CPU port (%d) enable failed with rc %d",
                      profile[laneItr].profileValue.portMap.portNum, rc);
            }
        }
        laneItr++;
    }

    //disable MAC counter clear on read.
    for (startPort = 0; startPort <= maxPorts; startPort++)
    {
        cpssDxChPortMacCountersClearOnReadSet(devNum, startPort, GT_TRUE);
    }
    //numPorts = ++maxPorts;
    MRVSave(numPorts);

    return GT_OK;
}

GT_STATUS cpssHalSimInit(PROFILE_STC profile[])
{
    GT_STATUS rc = GT_OK;
#ifdef ASIC_SIMULATION
    rc = cpssHalSimInitApi(profile);
#endif
    return rc;
}
/*******************************************************************************
* cpssHalInitializeDeviceApi
*
* DESCRIPTION:
*       initialize cpss driver and specific device
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       first phase support systems with single device
*
*******************************************************************************/
GT_STATUS cpssHalInitializeDeviceApi
(
    GT_U8                   devNum,
    XP_DEV_TYPE_T           devType,
    // gain: mrvHalProfileSetFn     profileSetFnCallback
    PROFILE_STC                 profile[] /* can be null*/
)
{
    GT_STATUS           ret;
    GT_U8           i;
    GT_PCI_INFO         pciInfo;
    GT_PCI_DEV_VENDOR_ID    device_list[mrvSupportedDevDataSize];

    cpssOsMemSet(device_list, 0, sizeof(device_list));
    /* convert CPSS_PP_DEVICE_TYPE to GT_PCI_DEV_VENDOR_ID */
    for (i=0; i<mrvSupportedDevDataSize; i++)
    {
        device_list[i].devId = mrvSupportedDevData[i].devType >> 16 ;
        device_list[i].vendorId = mrvSupportedDevData[i].devType & 0xffff;
    }
    ret = cpssHalInitServicesGetPciDev((GT_BOOL)!devNum, device_list, &pciInfo);
    if (ret != GT_OK)
    {
        if (devNum==0)
        {
            cpssOsPrintf("Could not find Prestera device on PCI bus!\n");
        }
        else
        {
            cpssOsPrintf("No more Prestera devices on PCI bus!\n");
        }

        return GT_NO_MORE;
    }

    cpssOsPrintf("PCI: %04x : %04x [%d:%d]\n", pciInfo.pciDevVendorId.vendorId,
                 pciInfo.pciDevVendorId.devId, pciInfo.pciBusNum, pciInfo.funcNo);
    extern unsigned int sleep(unsigned int seconds);
    sleep(1);

    ret = cpssHalInitializeDevice(devNum, devType, profile, &pciInfo);
    MRVL_HAL_API_RETURN("cpssHalInitializeDevice", ret);

    ret = cpssHalInitializeLanesSwap(devNum, devType, profile);
    MRVL_HAL_API_RETURN("cpssHalInitializeLanesSwapApi", ret);

    ret = cpssHalInitializePorts(devNum, profile, devType);
    MRVL_HAL_API_RETURN("cpssHalInitializePorts", ret);

    ret = cpssDxChCfgDevEnable(devNum, GT_TRUE);
    MRVL_HAL_API_RETURN("cpssDxChCfgDevEnable: %d\n", ret);

    //ret = mrvlLinkManagerTask();
    //printf("mrvlLinkManagerTask: %d\n", ret);

    ret = cpssHalDevInfoDumpApi(devNum);
    MRVL_HAL_API_RETURN("cpssHalDevInfoDumpApi: %d\n", ret);

    return GT_OK;
}

/*******************************************************************************
* cpssHalRunApi
*
* DESCRIPTION:
*       enable device and interrupts
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalRunApi
(
    void
)
{
    GT_STATUS   rc;
    GT_U8       devNum;

    /* get device number from db */
    MRVGet(devNum);
    /* enable device */
    rc = cpssDxChCfgDevEnable(devNum, GT_TRUE);
    MRVL_HAL_API_RETURN("cpssDxChCfgDevEnable", rc);

    return GT_OK;
}


/*******************************************************************************
* cpssHalDevInfoDumpApi
*
* DESCRIPTION:
*       prints device and port information
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssHalDevInfoDumpApi
(
    GT_U8                       devNum
)
{

    GT_HW_DEV_NUM               hwDevNum;
    GT_U32                      coreClkDb;
    GT_U32                      coreClkHw;
    CPSS_DXCH_CFG_DEV_INFO_STC  cpssDevInfo;
    GT_STATUS                   rc;
    GT_U32 i;


    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChCfgHwDevNumGet", rc);
        return rc;
    }
    /* Get core clock */
    rc = cpssDxChHwCoreClockGet(devNum, &coreClkDb, &coreClkHw);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get device info */
    cpssOsMemSet(&cpssDevInfo, 0, sizeof(CPSS_DXCH_CFG_DEV_INFO_STC));
    rc = cpssDxChCfgDevInfoGet(devNum, &cpssDevInfo);
    if (rc != GT_OK)
    {
        MRVL_HAL_API_TRACE("cpssDxChCfgDevInfoGet", rc);
        return rc;
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("************ Device information dump ************\n\n");
    cpssOsPrintf("HW device number: %d\n", hwDevNum);
    cpssOsPrintf("HW Core clock frequency: %d Hz\n", coreClkHw);
    cpssOsPrintf("Device type: %08X\n", cpssDevInfo.genDevInfo.devType);
    cpssOsPrintf("Device revision: %d\n", cpssDevInfo.genDevInfo.revision);
    cpssOsPrintf("Device family: %d\n", cpssDevInfo.genDevInfo.devFamily);
    cpssOsPrintf("Device sub-family: %d\n", cpssDevInfo.genDevInfo.devSubFamily);
    cpssOsPrintf("Maximum number of ports in device: %d\n",
                 cpssDevInfo.genDevInfo.maxPortNum+1);
    cpssOsPrintf("Maximum number of virtual ports in device: %d\n",
                 cpssDevInfo.genDevInfo.numOfVirtPorts);
    cpssOsPrintf("Existing ports bitmap: 0x");
    for (i = 0;
         i < cpssDevInfo.genDevInfo.maxPortNum/32 + (cpssDevInfo.genDevInfo.maxPortNum%32
                                                     > 0); i++)
    {
        cpssOsPrintf("%08X", cpssDevInfo.genDevInfo.existingPorts.ports[i]);
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("HW device number mode: %d\n",
                 cpssDevInfo.genDevInfo.hwDevNumMode);
    cpssOsPrintf("CPU port mode: %d\n", cpssDevInfo.genDevInfo.cpuPortMode);
    cpssOsPrintf("\n");
    cpssOsPrintf("********** End Of Device information dump **********\n\n");
    return GT_OK;
}

GT_STATUS cpssHalGetNumPorts
(
    GT_U8           devNum,
    GT_U32             *numOfPorts
)
{
    GT_U32 numPorts;
    MRVGet(numPorts);
    *numOfPorts = numPorts;

    return GT_OK;
}
#ifdef RETRY_PP_SOFT_RESET
extern void prvNoKm_reg_write_field(GT_UINTPTR regsBase, GT_U32 regAddr,
                                    GT_U32 mask, GT_U32 value);
void cpssHalDisableInterrupts(int devId, bool isSigRcvd)
{
    if (!isSigRcvd)
    {
        cpssOsPrintf("Dis INT Dev 0 base :0x%x \n", dev0HwInfo.resource.cnm.start);
    }
    if (dev0HwInfo.resource.cnm.start)
    {
        prvNoKm_reg_write_field(dev0HwInfo.resource.cnm.start, 0x40050, 0x00010000,
                                0x00000000);
    }
    if (!isSigRcvd)
    {
        cpssOsPrintf("Dis INT Dev 1 base :0x%x \n", dev1HwInfo.resource.cnm.start);
    }
    if (dev1HwInfo.resource.cnm.start)
    {
        prvNoKm_reg_write_field(dev1HwInfo.resource.cnm.start, 0x40050, 0x00010000,
                                0x00000000);
    }
}
#endif

GT_STATUS cpssHalResetDevice(int devId)
{
    GT_STATUS           rc = GT_OK;;
    GT_U8               devNum;
    int                 maxPorts = 0, port;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(devId, &devType);

    //disable mac for all ports
    cpssHalGetMaxGlobalPorts(&maxPorts);
    XPS_GLOBAL_PORT_ITER(port, maxPorts)
    {
        cpssHalMacPortEnable(devId, port, 0);
    }

    if (cpssHalPortManagerIsSupported(devId) == GT_OK)
    {
        rc = cpssOsTaskDelete(tuneStartTaskId);
        cpssOsPrintf("cpssHalResetDevice stop portMgr ret=%d\n", rc);
        rc = cpssOsTaskDelete(portEvntTid);
        cpssOsPrintf("cpssHalResetDevice stop portMgrEvnt ret=%d\n", rc);
    }
    //    rc = cpssOsTaskDelete(eventHandlerTid);
    //    cpssOsPrintf("cpssHalResetDevice stop rxTask ret=%d\n", rc);
    //    rc = osSemSignal((GT_SEM)437);
    //    cpssOsPrintf("cpssHalResetDevice rel sem 437 ret=%d\n", rc);
    //    rc = osSemSignal((GT_SEM)4);
    //    cpssOsPrintf("cpssHalResetDevice rel sem 4 ret=%d\n", rc);
    //    rc = osSemSignal((GT_SEM)5);
    //    cpssOsPrintf("cpssHalResetDevice rel sem 5 ret=%d\n", rc);
    //release DMA
    cpssOsPrintf("cpssHalResetDevice Freeing memory ret=%d\n", rc);

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        cpssHalResetPp(devNum, TRUE, devType);
    }

    rc = osCacheDmaFreeAll();
    return GT_OK;
}

void cpssHalResetPp(int devNum, bool resetInterrupt, XP_DEV_TYPE_T devType)
{
    GT_STATUS           rc;
    GT_PCI_INFO         pciInfo;
    GT_PCI_DEV_VENDOR_ID    device_list[mrvSupportedDevDataSize];
    GT_U32              intNum;
    GT_U8               i;

    cpssOsMemSet(device_list, 0, sizeof(device_list));
    for (i=0; i<mrvSupportedDevDataSize; i++)
    {
        device_list[i].devId = mrvSupportedDevData[i].devType >> 16 ;
        device_list[i].vendorId = mrvSupportedDevData[i].devType & 0xffff;
    }
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if (IS_DEVICE_FALCON(devType))
    {
        rc = cpssHalInitServicesGetPciDev((GT_BOOL)!devNum, device_list, &pciInfo);
        if (rc == GT_OK)
        {
            if (resetInterrupt)
            {
                //revisit, disconnect is also called from cpssDxChCfgDevRemove
                extDrvPcieGetInterruptNumber(pciInfo.pciBusNum, pciInfo.pciIdSel,
                                             pciInfo.funcNo, &intNum);
                //stop the INT task
                //rc = extDrvIntDisable(intNum);
                rc = extDrvIntDisconnect(intNum);
                cpssOsPrintf("cpssHalResetDevice stop intTask ret %d for dev %d\n", rc, devNum);
            }
#ifdef RESET_PP_EXCLUDE_PEX
            /*skip reset PCIe for ARM*/
            rc = cpssDxChHwPpSoftResetSkipParamSet(devNum,
                                                   CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
            cpssOsPrintf("cpssHalResetDevice cpssDxChHwPpSoftResetSkipParamSet ret=%d dev=%d\n",
                         rc, devNum);
#else
            if (IS_DEVICE_EBOF_PROFILE(devType))
            {
                //EBOF needs this too
                rc = cpssDxChHwPpSoftResetSkipParamSet(devNum,
                                                       CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
                cpssOsPrintf("cpssHalResetDevice cpssDxChHwPpSoftResetSkipParamSet ret=%d dev=%d\n",
                             rc, devNum);
            }
#endif
            //reset PCIe also
            rc = cpssDxChHwPpSoftResetTrigger(devNum);
            cpssOsPrintf("cpssHalResetDevice cpssDxChHwPpSoftResetTrigger ret=%d dev=%d\n",
                         rc, devNum);
            /*remove PCIe and rescan*/
#ifndef RESET_PP_EXCLUDE_PEX
#ifndef ASIC_SIMULATION
            cpssOsPrintf("cpssHalResetDevice : 'remove pex from cpss' \n");
            rc = extDrvPexRemove(
                     pciInfo.pciBusNum,
                     pciInfo.pciIdSel,
                     pciInfo.funcNo);
            if (rc != GT_OK)
            {
                cpssOsPrintf("cpssHalResetDevice extDrvPexRemove() failed, rc=%d\n", rc);
            }

            CPSS_HW_INFO_STC hwInfo;
            memset(&hwInfo, 0, sizeof(CPSS_HW_INFO_STC));

            cpssOsPrintf("cpssHalResetDevice 'rescan PEX' \n");
            rc = extDrvPexRescan(
                     pciInfo.pciBusNum,
                     pciInfo.pciIdSel,
                     pciInfo.funcNo,
                     &hwInfo);
            if (rc != GT_OK)
            {
                cpssOsPrintf("cpssHalResetDevice extDrvPexRescan() failed, rc=%d\n", rc);
            }
#endif //ASIC_SIMULATION
#endif
        }
        else
        {
            cpssOsPrintf("cpssHalResetDevice get pciInfo ret=%d dev=%d", rc,
                         devNum);
        }
    }
    else
    {
        //    osTimerWkAfter(1000);
        if (devType == ALDB2B)
        {
            GT_U32  regAddr, regValue;
            GT_BOOL isError;
            regAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E,
                                                   &isError);
            if (isError == GT_TRUE)
            {
                cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet Failed\n");
            }

            /*Mask Interrupts in Global Interrupts Summary Mask Register*/
            regAddr += 0x00000034;
            regValue = 0x0;
            rc = cpssDrvPpHwRegisterWrite(CAST_SW_DEVNUM(devNum), 0, regAddr, regValue);
            cpssOsPrintf("cpssHalResetDevice Gbl Int Mask Write : %d val 0x%x\n", rc,
                         regValue);

            /*PCI Express Command and Status Register. Set Master Enable bit to 0*/
            regValue = 0;
            rc = cpssDrvPpHwInternalPciRegRead(devNum, 0, 0x00040004, &regValue);
            cpssOsPrintf("cpssHalResetDevice PCI Read Dev %d rc %d regVal: 0x%x\n",
                         devNum, rc, regValue);

            /* Set '0' to bit_2 */
            U32_SET_FIELD_MAC(regValue, 2, 1, 0);

            rc = cpssDrvPpHwInternalPciRegWrite(devNum, 0, 0x00040004, regValue);
            cpssOsPrintf("cpssHalResetDevice PCI Write Dev %d rc %d regVal: 0x%x",
                         devNum, rc, regValue);
        }
        rc = cpssDxChHwPpSoftResetSkipParamSet(devNum,
                                               CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
        cpssOsPrintf("cpssHalResetDevice cpssDxChHwPpSoftResetSkipParamSet ret=%d dev=%d\n",
                     rc, devNum);
        rc = cpssDxChHwPpSoftResetTrigger(devNum);
        cpssOsPrintf("cpssHalResetDevice cpssDxChHwPpSoftResetTrigger ret=%d dev=%d\n",
                     rc, devNum);
    }

    osTimerWkAfter(1000);
    //Remove device also after soft reset
    rc = cpssDxChCfgDevRemove(devNum);
    cpssOsPrintf("cpssHalResetDevice cpssDxChCfgDevRemove ret=%d dev=%d", rc,
                 devNum);
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
}

GT_STATUS cpssHalPrintPortMap(int devId)
{
    GT_U32    portIdx;
    int    maxPortIdx;
    GT_U32    pipeId, tileId, dpIndex;
    XP_DEV_TYPE_T           devType;
    GT_STATUS           rc;

    cpssHalGetMaxGlobalPorts(&maxPortIdx);
    cpssHalGetDeviceType(devId, &devType);
    if (IS_DEVICE_FALCON(devType))
    {
        cpssOsPrintf("\n+----+-----+------+-------+--------------+------------+----+-----+-----+-----+----+");
        cpssOsPrintf("\n| #  | Dev | Port | Speed | MAC IF MODE  | map  Type  | mac| txq | pipe| tile| dp |");
        cpssOsPrintf("\n+----+-----+------+-------+--------------+------------+----+-----+-----+-----+----+");
    }
    else
    {
        cpssOsPrintf("\n+----+-----+------+-------+--------------+-----------------+------------------------------+");
        cpssOsPrintf("\n| #  | Dev | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |");
        cpssOsPrintf("\n+----+-----+------+-------+--------------+-----------------+------------------------------+");
    }

    int skipInvalidPort = 0;
    for (portIdx = 0 ; portIdx < (maxPortIdx + skipInvalidPort); portIdx++)
    {
        CPSS_PM_PORT_PARAMS_STC cpssPortParams;
        CPSS_PORT_SPEED_ENT cpssSpeed;
        CPSS_PORT_INTERFACE_MODE_ENT cpssIfMode;
        CPSS_DXCH_DETAILED_PORT_MAP_STC    portMap;
        GT_U8 devNum;
        GT_U32 portNum;

        devNum = xpsGlobalIdToDevId(devId, portIdx);
        portNum = xpsGlobalPortToPortnum(devId, portIdx);

        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        if (rc != GT_OK)
        {
#if 0 // DEBUG
            cpssOsPrintf("cpssDxChPortPhysicalPortDetailedMapGet failed devId %d Port %d devNum %d portNum %d rc %d",
                         devId, portIdx, devNum, portNum, rc);
#endif
            skipInvalidPort ++;
            if (portIdx == 0xFFFF)
            {
                // Safe to break
                break;
            }
            continue;
        }
        cpssDxChPortManagerPortParamsGet(devNum, (GT_PHYSICAL_PORT_NUM)portNum,
                                         &cpssPortParams);
        if (cpssPortParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
        {
            cpssSpeed = cpssPortParams.portParamsType.regPort.speed;
            cpssIfMode = cpssPortParams.portParamsType.regPort.ifMode;
        }
        else
        {
            cpssSpeed = cpssPortParams.portParamsType.apPort.modesArr[0].speed;
            cpssIfMode = cpssPortParams.portParamsType.apPort.modesArr[0].ifMode;
        }

        if (! IS_DEVICE_FALCON(devType))
        {
            cpssOsPrintf("\n| %2d | %3d | %4d | %s | %s | %-15s | %5d %5d %3d %3d %4d %3d |",
                         portIdx, devNum, portNum,
                         CPSS_SPEED_2_STR(cpssSpeed),
                         CPSS_IF_2_STR(cpssIfMode),
                         CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                         portMap.portMap.rxDmaNum, portMap.portMap.txDmaNum,
                         portMap.portMap.macNum, portMap.portMap.txqNum,
                         portMap.portMap.ilknChannel, portMap.portMap.tmPortIdx);
        }
        else
        {
            /* global pipeId (not relative to the tileId) */
            rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                                                                               portMap.portMap.macNum/*global MAC port*/,
                                                                               &pipeId, NULL);/*local MAC*/

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                                                                  portMap.portMap.rxDmaNum/*global DMA port*/,
                                                                  &dpIndex, NULL);/*local DMA*/

            tileId = pipeId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

            cpssOsPrintf("\n| %2d | %3d | %4d | %s | %s | %-10s |%3d |%4d | %3d | %3d | %2d |",
                         portIdx, devNum, portNum,
                         CPSS_SPEED_2_STR(cpssSpeed),
                         CPSS_IF_2_STR(cpssIfMode),
                         CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                         portMap.portMap.macNum,
                         portMap.portMap.txqNum,
                         pipeId,
                         tileId,
                         dpIndex);

        }
    }
    if (IS_DEVICE_FALCON(devType))
    {
        cpssOsPrintf("\n+----+-----+------+-------+--------------+------------+----+-----+-----+-----+----+\n");
    }
    else
    {
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+\n");
    }
    return GT_OK;
}

GT_STATUS cpssHalLedProfileSet(int devId, const char* ledModeStr)
{
    /* Ignore devId for now, as the ledProfileMode has to get into cpssHalDeviceStruct,
     * but is not intialized when cpssHalLedProfileSet is called
     */
    if (0 == strncasecmp(ledModeStr, LED_PROFILE_97BIT_AC3X_STR,
                         strlen(LED_PROFILE_97BIT_AC3X_STR)))
    {
        ledProfileMode =  LED_PROFILE_97BIT_AC3X;
        printf("cpssHalLedProfileSet dev=%d mode=%s ledProfile=%d\n", devId,
               ledModeStr, ledProfileMode);
    }
    else
    {
        printf("WARNING: cpssHalLedProfileSet Invalid ledProfile=%s\n",
               ledModeStr);
    }

    return GT_OK;
}

LED_PROFILE_TYPE_E cpssHalLedProfileGet(int devId)
{
    return ledProfileMode;
}

#ifdef RETRY_PP_SOFT_RESET
GT_STATUS cpssHalCheckIsHwResetDone(uint32_t devNum)
{
    GT_STATUS   rc;
    GT_U32  regAddr, regValue;
    GT_BOOL isError;

    regAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E,
                                           &isError);
    if (isError == GT_TRUE)
    {
        cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet Failed\n");
        return GT_FAIL;
    }

    regAddr += 0x0000007c;/* user defined register */
    rc = cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum), 0, regAddr, &regValue);
    cpssOsPrintf("cpssDrvPpHwRegisterRead %d value 0x%x\n", rc, regValue);
    if (regValue != 0)
    {
        cpssOsPrintf("WARNING: HW reset (soft/hard) not done. reg(%x)\n",
                     regValue);
        regValue = 0;
        rc = cpssDrvPpHwRegisterWrite(CAST_SW_DEVNUM(devNum), 0, regAddr, regValue);
        cpssOsPrintf("cpssDrvPpHwRegisterWrite : %d val 0x%x\n", rc, regValue);
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS cpssHalWriteHwResetDone(uint32_t devNum)
{
    GT_STATUS   rc;
    GT_U32  regAddr, regValue;
    GT_BOOL isError;

    regAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E,
                                           &isError);
    if (isError == GT_TRUE)
    {
        cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet Failed\n");
        return GT_FAIL;
    }

    regAddr += 0x0000007c;/* user defined register */
    regValue = 0x12345678;
    rc = cpssDrvPpHwRegisterWrite(CAST_SW_DEVNUM(devNum), 0, regAddr, regValue);
    cpssOsPrintf("cpssDrvPpHwRegisterWrite : %d val 0x%x\n", rc, regValue);
    rc = cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum), 0, regAddr, &regValue);
    cpssOsPrintf("cpssDrvPpHwRegisterRead %d \n", rc);
    if (regValue != 0x12345678)
    {
        cpssOsPrintf("cpssDrvPpHwRegisterWrite Failed%d val :0x%x\n", rc, regValue);
        return GT_FAIL;
    }

    return GT_OK;
}
#endif
