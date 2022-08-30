/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAlleycat5DevInit.c
*
* @brief Bobcat specific HW Services init
*
* @version   23
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/alleycat3/mvHwsAlleycat3PortIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

/************************* definition *****************************************************/
#define ALLEYCAT3_NW_GIG_BASE   (0x12000000)
#define ALLEYCAT3_FB_GIG_BASE   (0x10200000)
#define ALLEYCAT3_NW_XPCS_BASE  (0x12180400)
#define ALLEYCAT3_FB_XPCS_BASE  ALLEYCAT3_NW_XPCS_BASE
#define ALLEYCAT3_NW_XLG_BASE   (0x120C0000)
#define ALLEYCAT3_NW_MPCS_BASE  (0x12180000)
#define ALLEYCAT3_FB_MPCS_BASE  ALLEYCAT3_NW_MPCS_BASE

#define ALLEYCAT3_SERDES_BASE   (0x13000000)
#define ALLEYCAT3_SERDES_PHY_BASE   (0x13000800)

#define IND_OFFSET (0x1000)

/************************* Globals *******************************************************/
extern const MV_HWS_PORT_INIT_PARAMS *hwsPortsAlleycat5ParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL alleycat5DbInitDone = GT_FALSE;

#if 1
const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC12GP41P2VPinToRegMap[] = /* Based on SD1 */
{
/* C12GP41P2V_PIN_RESET                = 0   */ {SERDES_EXTERNAL_CONFIGURATION_1 ,3                     ,1},
/* C12GP41P2V_PIN_ISOLATION_ENB        = 1   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_BG_RDY               = 2   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_SIF_SEL              = 3   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_PHY_MODE             = 4   */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
/* C12GP41P2V_PIN_REFCLK_SEL           = 5   */ {SERDES_EXTERNAL_CONFIGURATION_5 ,7                     ,1},
/* C12GP41P2V_PIN_REF_FREF_SEL         = 6   */ {SERDES_EXTERNAL_CONFIGURATION_4 ,12                    ,4},
/* C12GP41P2V_PIN_PHY_GEN_TX0          = 7   */ {SERDES_EXTERNAL_CONFIGURATION_0 ,7                     ,4},
/* C12GP41P2V_PIN_PHY_GEN_RX0          = 8   */ {SERDES_EXTERNAL_CONFIGURATION_0 ,3                     ,4},
/* C12GP41P2V_PIN_DFE_EN0              = 9   */ {SERDES_EXTERNAL_CONFIGURATION_2 ,4                     ,1},
/* C12GP41P2V_PIN_DFE_UPDATE_DIS0      = 10  */ {SERDES_EXTERNAL_CONFIGURATION_2 ,6                     ,1},
/* C12GP41P2V_PIN_PU_PLL0              = 11  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,1                     ,1},
/* C12GP41P2V_PIN_PU_RX0               = 12  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,11                    ,1},
/* C12GP41P2V_PIN_PU_TX0               = 13  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,12                    ,1},
/* C12GP41P2V_PIN_TX_IDLE0             = 14  */ {SERDES_EXTERNAL_CONFIGURATION_1 ,2                     ,1},
/* C12GP41P2V_PIN_PU_IVREF             = 15  */ {MV_HWS_SERDES_TIED_PIN          ,1/*<-default value*/  ,0},
/* C12GP41P2V_PIN_RX_TRAIN_ENABLE0     = 16  */ {SERDES_EXTERNAL_CONFIGURATION_3 ,7                     ,1},
/* C12GP41P2V_PIN_RX_TRAIN_COMPLETE0   = 17  */ {0x1000 | (0x251 << 2)           ,4                     ,1},
/* C12GP41P2V_PIN_RX_TRAIN_FAILED0     = 18  */ {0x1000 | (0x251 << 2)           ,3                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_ENABLE0     = 19  */ {SERDES_EXTERNAL_CONFIGURATION_3 ,8                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_COMPLETE0   = 20  */ {0x1000 | (0x251 << 2)           ,6                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_FAILED0     = 21  */ {0x1000 | (0x251 << 2)           ,5                     ,1},
/* C12GP41P2V_PIN_SQ_DETECTED_LPF0     = 22  */ {SERDES_EXTERNAL_STATUS_0        ,15                    ,1},
/* C12GP41P2V_PIN_RX_INIT0             = 23  */ {SERDES_EXTERNAL_CONFIGURATION_1 ,4                     ,1},
/* C12GP41P2V_PIN_RX_INIT_DONE0        = 24  */ {SERDES_EXTERNAL_STATUS_0        ,4                     ,1},
/* C12GP41P2V_PIN_DFE_PAT_DIS0         = 25  */ {SERDES_EXTERNAL_CONFIGURATION_2 ,5                     ,1},
};
#else
const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC12GP41P2VPinToRegMap[] = /* Based on SD1 */
{
/* C12GP41P2V_PIN_RESET                = 0   */ {SERDES_EXTERNAL_CONFIGURATION_1 ,3                     ,1},
/* C12GP41P2V_PIN_ISOLATION_ENB        = 1   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_BG_RDY               = 2   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_SIF_SEL              = 3   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C12GP41P2V_PIN_PHY_MODE             = 4   */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
/* C12GP41P2V_PIN_REFCLK_SEL           = 5   */ {SERDES_EXTERNAL_CONFIGURATION_5 ,7                     ,1},
/* C12GP41P2V_PIN_REF_FREF_SEL         = 6   */ {SERDES_EXTERNAL_CONFIGURATION_4 ,12                    ,4},
/* C12GP41P2V_PIN_PHY_GEN_TX0          = 7   */ {SERDES_EXTERNAL_CONFIGURATION_0 ,7                     ,4},
/* C12GP41P2V_PIN_PHY_GEN_RX0          = 8   */ {SERDES_EXTERNAL_CONFIGURATION_0 ,3                     ,4},
/* C12GP41P2V_PIN_DFE_EN0              = 9   */ {SERDES_EXTERNAL_CONFIGURATION_2 ,4                     ,1},
/* C12GP41P2V_PIN_DFE_UPDATE_DIS0      = 10  */ {SERDES_EXTERNAL_CONFIGURATION_2 ,6                     ,1},
/* C12GP41P2V_PIN_PU_PLL0              = 11  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,1                     ,1},
/* C12GP41P2V_PIN_PU_RX0               = 12  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,11                    ,1},
/* C12GP41P2V_PIN_PU_TX0               = 13  */ {SERDES_EXTERNAL_CONFIGURATION_0 ,12                    ,1},
/* C12GP41P2V_PIN_TX_IDLE0             = 14  */ {SERDES_EXTERNAL_CONFIGURATION_1 ,2                     ,1},
/* C12GP41P2V_PIN_PU_IVREF             = 15  */ {MV_HWS_SERDES_TIED_PIN          ,1/*<-default value*/  ,0},
/* C12GP41P2V_PIN_RX_TRAIN_ENABLE0     = 16  */ {SERDES_EXTERNAL_CONFIGURATION_3 ,7                     ,1},
/* C12GP41P2V_PIN_RX_TRAIN_COMPLETE0   = 17  */ {SERDES_EXTERNAL_STATUS_1        ,0                     ,1},
/* C12GP41P2V_PIN_RX_TRAIN_FAILED0     = 18  */ {SERDES_EXTERNAL_STATUS_1        ,1                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_ENABLE0     = 19  */ {SERDES_EXTERNAL_CONFIGURATION_3 ,8                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_COMPLETE0   = 20  */ {SERDES_EXTERNAL_STATUS_1        ,2                     ,1},
/* C12GP41P2V_PIN_TX_TRAIN_FAILED0     = 21  */ {SERDES_EXTERNAL_STATUS_1        ,3                     ,1},
/* C12GP41P2V_PIN_SQ_DETECTED_LPF0     = 22  */ {SERDES_EXTERNAL_STATUS_0        ,15                    ,1},
/* C12GP41P2V_PIN_RX_INIT0             = 23  */ {SERDES_EXTERNAL_CONFIGURATION_1 ,4                     ,1},
/* C12GP41P2V_PIN_RX_INIT_DONE0        = 24  */ {SERDES_EXTERNAL_STATUS_0        ,4                     ,1},
/* C12GP41P2V_PIN_DFE_PAT_DIS0         = 25  */ {SERDES_EXTERNAL_CONFIGURATION_2 ,5                     ,1},
};
#endif

/** COMPHY_12nm_Serdes_Init_rev0.23 */
const MV_HWS_SERDES_TXRX_TUNE_PARAMS alleycat5TxRxTuneParams[] =
#ifndef WIN32
{  /* SerDes speed                                   PRE  PEAK POST                      RES_SEL RES_SHIFT CAP_SEL  SELMUFI SELMUFF SELMUPI SELMUPF SQLCH*/
    {_1_25G,                   {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={7,       0,      11,        3,      3,      5,      5,    10}}},
    {_1_25G_SR_LR,             {.txComphyC12GP41P2V={ 0,  13,   0}}, {.rxComphyC12GP41P2V={7,       0,      11,        3,      3,      5,      5,    10}}},
    {_3_125G,                  {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={7,       0,      11,        3,      3,      5,      5,    10}}},
    {_2_578125,                {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={7,       0,      11,        3,      3,      5,      5,    10}}},
    {_5G,                      {.txComphyC12GP41P2V={ 0,   9,   2}}, {.rxComphyC12GP41P2V={7,       0,      11,        3,      3,      5,      5,    10}}},
    {_5_15625G,                {.txComphyC12GP41P2V={ 4,   0,   0}}, {.rxComphyC12GP41P2V={5,       1,      10,        3,      3,      4,      4,    10}}},
    {_10_3125G,                {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={5,       1,      10,        3,      3,      4,      4,     4}}},
    {_10_3125G_SR_LR,          {.txComphyC12GP41P2V={ 1,   2,   8}}, {.rxComphyC12GP41P2V={5,       1,      10,        3,      3,      4,      5,     4}}},
    {_11_5625G,                {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={5,       1,      10,        3,      3,      4,      4,     4}}},

    {LAST_MV_HWS_SERDES_SPEED, {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={0,       0,       0,        0,      0,      0,      0,     0}}}
};
#else
{
    {0}
};
#endif

HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes0Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes1Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes2Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes3Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes4Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes5Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes6Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes7Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes8Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes9Data  = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes10Data = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA alleycat5Serdes11Data = {mvHwsComphyC12GP41P2VPinToRegMap, NULL, alleycat5TxRxTuneParams, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC alleycat5SerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes0Data}},
    {NULL, 1,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes1Data}},
    {NULL, 2,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes2Data}},
    {NULL, 3,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes3Data}},
    {NULL, 4,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes4Data}},
    {NULL, 5,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes5Data}},
    {NULL, 6,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes6Data}},
    {NULL, 7,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes7Data}},
    {NULL, 8,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes8Data}},
    {NULL, 9,  0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes9Data}},
    {NULL, 10, 0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes10Data}},
    {NULL, 11, 0, COM_PHY_C12GP41P2V, {(MV_HWS_COMPHY_SERDES_DATA*)&alleycat5Serdes11Data}},
};
/************************* pre-declaration ***********************************************/

extern GT_STATUS hwsAlleycat3Ports1To23Cfg (GT_U8 devNum, GT_U32 portGroup);

extern GT_STATUS hwsPulse1msRegCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  coreClk
);

extern GT_STATUS hwsAc3PcsIfInit
(
    IN GT_U8 devNum
);

extern GT_STATUS hwsAc3MacIfInit
(
    IN GT_U8 devNum
);

#if (!defined MV_HWS_REDUCED_BUILD)
extern GT_STATUS mvHwsAlleycat3ClkSelCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);
#endif

extern GT_STATUS hwsAc3SerdesIfInit
(
    IN GT_U8 devNum
);

GT_STATUS mvHwsAlleycat5PowerReductionInit
(
    IN GT_U8               devNum,
    IN GT_U32              portNum
);

/*****************************************************************************************/

/**
* @internal hwsAc5SerdesIfInit function
* @endinternal
*
* @brief   Init all supported SERDES types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsAc5SerdesIfInit
(
    IN GT_U8 devNum
)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;

    for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {
        /* JIRA ALLEYCAT5-222 - disable watchdog timer that is not aligned to apb protocol */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                            hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                            SERDES_EXTERNAL_CONFIGURATION_2,
                                            0x0,
                                            0x200));
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    CHECK_STATUS(mvHwsComphyIfInit(devNum,COM_PHY_C12GP41P2V,hwsSerdesFuncsPtr,MV_HWS_COMPHY_FIRMWARE_NR,GT_FALSE));

    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD

GT_VOID hwsAlleycat5PortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsAlleycat5ParamsSupModesMap;
}

#endif

GT_VOID hwsAlleycat5IfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32 data, version;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }

#ifndef MV_HWS_FREE_RTOS
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
    }
#endif

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    /* AlleyCat3 has no redundancy lanes, thus the value of redundVectorGetFunc is NULL */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc  = hwsAlleycat5PortParamsGetLanes;

#if (!defined MV_HWS_REDUCED_BUILD)

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc       = hwsAlleycat5PortParamsSet;

    /* Configures the DP/Core Clock Selector on port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc    = mvHwsAlleycat3ClkSelCfg;
#endif

    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc  = mvHwsAlleycat3PortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsAlleycat3ApPortInit;  /* for AP port */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsAlleycat3PortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsAlleycat5PortAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = mvHwsAlleycat3PortExtendedModeCfg;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = mvHwsAlleycat3PortExtendedModeCfgGet;
#if (!defined MV_HWS_REDUCED_BUILD)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
}

#ifdef SHARED_MEMORY

GT_STATUS hwsAlleycat5IfReIoad
(
    GT_U8 devNum
)
{   MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(hwsAc3MacIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(hwsAc3PcsIfInit(devNum));


    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    CHECK_STATUS(mvHwsComphyIfInit(devNum,COM_PHY_C12GP41P2V,hwsSerdesFuncsPtr,MV_HWS_COMPHY_FIRMWARE_NR,GT_TRUE));


    return GT_OK;
}
#endif

/**
* @internal mvHwsAlleycat5IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat5IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
)
{

    MV_HWS_DEV_TYPE devType;

#ifndef MV_HWS_REDUCED_BUILD
GT_U32  serdesIndex;
#endif

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    if(alleycat5DbInitDone == GT_FALSE)
    {
        if (funcPtr == NULL)
            return GT_BAD_PARAM;

        if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
            (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
            (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
            (funcPtr->osMemCopyPtr == NULL) ||
            (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
            (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
            (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
#ifndef MV_HWS_FREE_RTOS
            || (funcPtr->timerGet == NULL)
#endif
            )
        {
            return GT_BAD_PARAM;
        }

        if (funcPtr->osMicroDelayPtr == NULL)
        {
            hwsOsMicroDelayPtr = hwsOsLocalMicroDelay;
        }
        else
        {
            hwsOsMicroDelayPtr = funcPtr->osMicroDelayPtr;
        }
        hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
        hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
        hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
        hwsOsFreeFuncPtr = funcPtr->osFreePtr;
        hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
        hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
        hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
        hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
        hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
        hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
        hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;
        hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
        hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
        hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
#ifndef MV_HWS_FREE_RTOS
        hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    }

    hwsDeviceSpecInfo[devNum].devType = AC5;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = 32;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV1;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_C12GP41P2V;
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(alleycat5SerdesDb) / sizeof(alleycat5SerdesDb[0]);
#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
    for (serdesIndex = 0; serdesIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; serdesIndex++)
    {
        CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, serdesIndex, &alleycat5SerdesDb[serdesIndex]));
    }
#else
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = alleycat5SerdesDb;
#endif

    /* define Host to HWS connection type  - now Legacy mode*/
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

#if (!defined MV_HWS_REDUCED_BUILD)
    hwsAlleycat5PortsParamsSupModesMapSet(devNum);
#endif

    hwsAlleycat5IfInitHwsDevFunc(devNum,funcPtr);

   /* Init HWS Firmware */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, "AP_AC5"));

#ifndef CO_CPU_RUN
    hwsInitPortsModesParam(devNum,NULL);
    if (hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#else
        CHECK_STATUS(hwsAlleycat3Ports1To23Cfg(devNum, 0));
#endif
    }
#endif


    /* Init all MAC units relevant for current device */
    CHECK_STATUS(hwsAc3MacIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(hwsAc3PcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsAc5SerdesIfInit(devNum));

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum, devNum));
    /* Init silicon specific base address and index for specified unit */
    mvUnitInfoSet(devType, GEMAC_UNIT, ALLEYCAT3_NW_GIG_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, MMPCS_UNIT, ALLEYCAT3_NW_MPCS_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, XPCS_UNIT, ALLEYCAT3_NW_XPCS_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, XLGMAC_UNIT, ALLEYCAT3_NW_XLG_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_UNIT, ALLEYCAT3_SERDES_BASE, IND_OFFSET);
    mvUnitInfoSet(devType, SERDES_PHY_UNIT, ALLEYCAT3_SERDES_PHY_BASE, IND_OFFSET);
#if 0
    /***************************/
    /* Power reduction feature */
    /***************************/
    for (portNum = 0; (!hwsInitDone[devNum]) && (portNum < HWS_CORE_PORTS_NUM(devNum)); portNum++)
    {
        CHECK_STATUS(mvHwsAlleycat5PowerReductionInit(devNum, portNum));
    }
    /* HWS_CORE_PORTS_NUM(devNum) = 32 , port 27 in 20G works with pcsNum 29 and pcsNum 50  */
    CHECK_STATUS(mvHwsAlleycat5PowerReductionInit(devNum, 50));
#endif
    alleycat5DbInitDone = GT_TRUE;

    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal hwsAlleycat5IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAlleycat5IfClose
(
    IN GT_U8 devNum
)
{
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        mvHwsComphySerdesDestroyDatabase(devNum);
        hwsInitDone[devNum] = GT_FALSE;
    }

    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (hwsInitDone[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        alleycat5DbInitDone = GT_FALSE;
    }

    hwsPortsElementsClose(devNum);
}
#endif

GT_STATUS mvHwsAlleycat5PowerReductionInit
(
    IN GT_U8               devNum,
    IN GT_U32              portNum
)
{
    if ( portNum >= 24 && portNum <= 29 )
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, 0, MMPCS_UNIT, portNum, PCS40G_COMMON_CONTROL, 0, 0x3000));
    }

    return GT_OK;
}
