/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsAc5xDevInit.c
*
* DESCRIPTION:
*     Phoenix specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>

#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5x/mvHwsAc5xPortIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs25If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs50If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiUsxPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuSgPcsIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacRev2If.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

extern  GT_BOOL hwsPpHwTraceFlag;

extern const MV_HWS_REG_ADDR_FIELD_STC ac5xMtiExtRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC ac5xPcs25UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100RegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs50UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC ac5xMtiRsFecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMifRegistersDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC ac5xAnpUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC anUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtipUsxExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkLpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxmUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxPcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuRsfecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtipCpuExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuPcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuMacUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxRsfecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs100UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuSgpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkRsfecStatisticsUnitsDb[];
extern const MV_HWS_PORT_INIT_PARAMS   *hwsPortsPhoenix_ParamsSupModesMap[];
extern const MV_HWS_PORT_INIT_PARAMS   *hwsPortsIronman_ParamsSupModesMap[];



GT_STATUS mvHwsPhoenixRegisterMisconfigurationSet
(
    IN GT_U8    devNum
);

/************************* definition *****************************************************/

/************************* Globals *******************************************************/

extern const MV_HWS_PORT_INIT_PARAMS **hwsPortsPhoenixParamsSupModesMap;
extern GT_STATUS hwsPhoenixPortElementsDbInit(GT_U8 devNum);

/* init per device */


#ifdef MV_HWS_REDUCED_BUILD
 /* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL phoenixDbInitDone = GT_FALSE;
#define PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    _var
#else
#define PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.ac5xDevInitSrc._var)
#endif

const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC28GP4X4PinToRegMapSdw[] =
{
/* C28GP4X4_PIN_RESET              = 0   */ {SDW_GENERAL_CONTROL_0           ,2                     ,1},
/* C28GP4X4_PIN_ISOLATION_ENB      = 1   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X4_PIN_BG_RDY             = 2   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X4_PIN_SIF_SEL            = 3   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X4_PIN_MCU_CLK            = 4   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X4_PIN_DIRECT_ACCESS_EN   = 5   */ {SDW_GENERAL_CONTROL_1           ,15                    ,1},
/* C28GP4X4_PIN_PHY_MODE           = 6   */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
/* C28GP4X4_PIN_REFCLK_SEL         = 7   */ {SDW_GENERAL_CONTROL_1           ,1                     ,1},
/* C28GP4X4_PIN_REF_FREF_SEL       = 8   */ {SDW_GENERAL_CONTROL_1           ,3                     ,5},
/* C28GP4X4_PIN_PHY_GEN_TX0        = 9   */ {SDW_LANE_CONTROL_0              ,8                     ,5},
/* C28GP4X4_PIN_PHY_GEN_TX1        = 10  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
/* C28GP4X4_PIN_PHY_GEN_TX2        = 11  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
/* C28GP4X4_PIN_PHY_GEN_TX3        = 12  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
/* C28GP4X4_PIN_PHY_GEN_RX0        = 13  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
/* C28GP4X4_PIN_PHY_GEN_RX1        = 14  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
/* C28GP4X4_PIN_PHY_GEN_RX2        = 15  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
/* C28GP4X4_PIN_PHY_GEN_RX3        = 16  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
/* C28GP4X4_PIN_DFE_EN0            = 17  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
/* C28GP4X4_PIN_DFE_EN1            = 18  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
/* C28GP4X4_PIN_DFE_EN2            = 19  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
/* C28GP4X4_PIN_DFE_EN3            = 20  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
/* C28GP4X4_PIN_PU_PLL0            = 21  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
/* C28GP4X4_PIN_PU_PLL1            = 22  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
/* C28GP4X4_PIN_PU_PLL2            = 23  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
/* C28GP4X4_PIN_PU_PLL3            = 24  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
/* C28GP4X4_PIN_PU_RX0             = 25  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
/* C28GP4X4_PIN_PU_RX1             = 26  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
/* C28GP4X4_PIN_PU_RX2             = 27  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
/* C28GP4X4_PIN_PU_RX3             = 28  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
/* C28GP4X4_PIN_PU_TX0             = 29  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
/* C28GP4X4_PIN_PU_TX1             = 30  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
/* C28GP4X4_PIN_PU_TX2             = 31  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
/* C28GP4X4_PIN_PU_TX3             = 32  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
/* C28GP4X4_PIN_TX_IDLE0           = 33  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
/* C28GP4X4_PIN_TX_IDLE1           = 34  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
/* C28GP4X4_PIN_TX_IDLE2           = 35  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
/* C28GP4X4_PIN_TX_IDLE3           = 36  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
/* C28GP4X4_PIN_PU_IVREF           = 37  */ {SDW_GENERAL_CONTROL_0           ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_ENABLE0   = 38  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_ENABLE1   = 39  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_ENABLE2   = 40  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_ENABLE3   = 41  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_COMPLETE0 = 42  */ {SDW_LANE_STATUS_1               ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_COMPLETE1 = 43  */ {SDW_LANE_STATUS_1               ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_COMPLETE2 = 44  */ {SDW_LANE_STATUS_1               ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_COMPLETE3 = 45  */ {SDW_LANE_STATUS_1               ,0                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_FAILED0   = 46  */ {SDW_LANE_STATUS_1               ,1                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_FAILED1   = 47  */ {SDW_LANE_STATUS_1               ,1                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_FAILED2   = 48  */ {SDW_LANE_STATUS_1               ,1                     ,1},
/* C28GP4X4_PIN_RX_TRAIN_FAILED3   = 49  */ {SDW_LANE_STATUS_1               ,1                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_ENABLE0   = 50  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_ENABLE1   = 51  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_ENABLE2   = 52  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_ENABLE3   = 53  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_COMPLETE0 = 54  */ {SDW_LANE_STATUS_1               ,2                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_COMPLETE1 = 55  */ {SDW_LANE_STATUS_1               ,2                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_COMPLETE2 = 56  */ {SDW_LANE_STATUS_1               ,2                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_COMPLETE3 = 57  */ {SDW_LANE_STATUS_1               ,2                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_FAILED0   = 58  */ {SDW_LANE_STATUS_1               ,3                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_FAILED1   = 59  */ {SDW_LANE_STATUS_1               ,3                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_FAILED2   = 60  */ {SDW_LANE_STATUS_1               ,3                     ,1},
/* C28GP4X4_PIN_TX_TRAIN_FAILED3   = 61  */ {SDW_LANE_STATUS_1               ,3                     ,1},
/* C28GP4X4_PIN_SQ_DETECTED_LPF0   = 62  */ {SDW_LANE_STATUS_0               ,6                     ,1},
/* C28GP4X4_PIN_SQ_DETECTED_LPF1   = 63  */ {SDW_LANE_STATUS_0               ,6                     ,1},
/* C28GP4X4_PIN_SQ_DETECTED_LPF2   = 64  */ {SDW_LANE_STATUS_0               ,6                     ,1},
/* C28GP4X4_PIN_SQ_DETECTED_LPF3   = 65  */ {SDW_LANE_STATUS_0               ,6                     ,1},
/* C28GP4X4_PIN_RX_INIT0           = 66  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
/* C28GP4X4_PIN_RX_INIT1           = 67  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
/* C28GP4X4_PIN_RX_INIT2           = 68  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
/* C28GP4X4_PIN_RX_INIT3           = 69  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
/* C28GP4X4_PIN_RX_INIT_DONE0      = 70  */ {SDW_LANE_STATUS_0               ,4                     ,1},
/* C28GP4X4_PIN_RX_INIT_DONE1      = 71  */ {SDW_LANE_STATUS_0               ,4                     ,1},
/* C28GP4X4_PIN_RX_INIT_DONE2      = 72  */ {SDW_LANE_STATUS_0               ,4                     ,1},
/* C28GP4X4_PIN_RX_INIT_DONE3      = 73  */ {SDW_LANE_STATUS_0               ,4                     ,1},
/* C28GP4X4_PIN_PRAM_SOC_EN        = 74  */ {SDW_GENERAL_CONTROL_1           ,14                    ,1},
/* C28GP4X1_PIN_DFE_PAT_DIS0       = 75  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
/* C28GP4X1_PIN_DFE_PAT_DIS1       = 76  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
/* C28GP4X1_PIN_DFE_PAT_DIS2       = 77  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
/* C28GP4X1_PIN_DFE_PAT_DIS3       = 78  */ {SDW_LANE_CONTROL_2              ,1                     ,1}
};

const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC28GP4X1PinToRegMapSdw[] =
{
/* C28GP4X1_PIN_RESET               = 0  */ {SDW_GENERAL_CONTROL_0           ,2                     ,1},
/* C28GP4X1_PIN_ISOLATION_ENB       = 1  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X1_PIN_BG_RDY              = 2  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X1_PIN_SIF_SEL             = 3  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X1_PIN_MCU_CLK             = 4  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
/* C28GP4X1_PIN_DIRECT_ACCESS_EN    = 5  */ {SDW_GENERAL_CONTROL_1           ,15                    ,1},
/* C28GP4X1_PIN_PHY_MODE            = 6  */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
/* C28GP4X1_PIN_REFCLK_SEL          = 7  */ {SDW_GENERAL_CONTROL_1           ,1                     ,1},
/* C28GP4X1_PIN_REF_FREF_SEL        = 8  */ {SDW_GENERAL_CONTROL_1           ,3                     ,5},
/* C28GP4X1_PIN_PHY_GEN_TX0         = 9  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
/* C28GP4X1_PIN_PHY_GEN_RX0         = 10 */ {SDW_LANE_CONTROL_0              ,3                     ,5},
/* C28GP4X1_PIN_DFE_EN0             = 11 */ {SDW_LANE_CONTROL_2              ,2                     ,1},
/* C28GP4X1_PIN_PU_PLL0             = 12 */ {SDW_LANE_CONTROL_0              ,0                     ,1},
/* C28GP4X1_PIN_PU_RX0              = 13 */ {SDW_LANE_CONTROL_0              ,1                     ,1},
/* C28GP4X1_PIN_PU_TX0              = 14 */ {SDW_LANE_CONTROL_0              ,2                     ,1},
/* C28GP4X1_PIN_TX_IDLE0            = 15 */ {SDW_LANE_CONTROL_0              ,24                    ,1},
/* C28GP4X1_PIN_PU_IVREF            = 16 */ {SDW_GENERAL_CONTROL_0           ,0                     ,1},
/* C28GP4X1_PIN_RX_TRAIN_ENABLE0    = 17 */ {SDW_LANE_CONTROL_3              ,0                     ,1},
/* C28GP4X1_PIN_RX_TRAIN_COMPLETE0  = 18 */ {SDW_LANE_STATUS_1               ,0                     ,1},
/* C28GP4X1_PIN_RX_TRAIN_FAILED0    = 19 */ {SDW_LANE_STATUS_1               ,1                     ,1},
/* C28GP4X1_PIN_TX_TRAIN_ENABLE0    = 20 */ {SDW_LANE_CONTROL_3              ,1                     ,1},
/* C28GP4X1_PIN_TX_TRAIN_COMPLETE0  = 21 */ {SDW_LANE_STATUS_1               ,2                     ,1},
/* C28GP4X1_PIN_TX_TRAIN_FAILED0    = 22 */ {SDW_LANE_STATUS_1               ,3                     ,1},
/* C28GP4X1_PIN_SQ_DETECTED_LPF0    = 23 */ {SDW_LANE_STATUS_0               ,6                     ,1},
/* C28GP4X1_PIN_RX_INIT0            = 24 */ {SDW_LANE_CONTROL_0              ,16                    ,1},
/* C28GP4X1_PIN_RX_INIT_DONE0       = 25 */ {SDW_LANE_STATUS_0               ,4                     ,1},
/* C28GP4X1_PIN_PRAM_SOC_EN         = 26 */ {SDW_GENERAL_CONTROL_1           ,14                    ,1},
/* C28GP4X1_PIN_DFE_PAT_DIS0        = 27  */{SDW_LANE_CONTROL_2              ,1                     ,1}
};

/** COMPHY_12nm_Serdes_Init_rev0.23 */
const MV_HWS_SERDES_TXRX_TUNE_PARAMS ac5xTxRxTuneParams[] =
#ifndef WIN32
{
    /* SerDes speed                              PRE PEAK POST                 DATA_RATE RES1_SEL RES2_SEL CAP1_SEL CAP2_SEL SELMUFI SELMUFF SELMUPI SELMUPF MID_L_TH_K MID_S_TH_K MID_L_TH_C MID_L_TH_S DFE_RES_H_TH_INIT DFE_RES_H_TH_END Squelch_TH */
    {_1_25G,                   {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={2,       6,        0,      12,      15,       4,     5,       3,      2,       9,         8,       2,          0,            40,                50,          10}}},
    {_1_25G_SR_LR,             {.txComphyC28GP4={ 0, 10,   0}}, {.rxComphyC28GP4={2,       6,        0,      12,      15,       4,     5,       3,      2,       9,         8,       2,          0,            40,                50,          10}}},
    {_3_125G,                  {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={3,       6,        0,      12,      15,       4,     5,       3,      2,       9,         8,       2,          0,            40,                50,          10}}},
    {_2_578125,                {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={2,       6,        0,      12,      15,       4,     5,       3,      2,       9,         8,       2,          0,            40,                50,          10}}},
    {_5G,                      {.txComphyC28GP4={ 0,  9,   2}}, {.rxComphyC28GP4={7,      11,        4,       8,       0,       2,     2,       3,      3,       9,         8,       2,          0,            40,                50,          10}}},
    {_5_15625G,                {.txComphyC28GP4={ 4,  0,   0}}, {.rxComphyC28GP4={4,       6,        0,      12,      15,       4,     5,       3,      2,       9,         8,       2,          0,            40,                50,          10}}},
    {_10G,                     {.txComphyC28GP4={ 0,  0,   1}}, {.rxComphyC28GP4={7,      11,        4,       8,       0,       2,     2,       3,      3,       9,         8,       2,          0,            40,                50,           3}}},
    {_10_3125G,                {.txComphyC28GP4={ 0,  0,   1}}, {.rxComphyC28GP4={7,      11,        4,       8,       0,       2,     2,       3,      3,       9,         8,       2,          0,            40,                50,           3}}},
    {_10_3125G_SR_LR,          {.txComphyC28GP4={ 1,  8,   4}}, {.rxComphyC28GP4={7,      11,        4,       8,       0,       2,     2,       3,      3,       9,         8,       2,          0,            40,                50,           5}}},
    {_20_625G,                 {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={12,      6,        0,      12,      15,       2,     2,       2,      3,      15,         0,      15,          0,            24,                24,           3}}},
    {_25_78125G,               {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={12,      6,        0,      12,      15,       2,     2,       2,      3,      15,         0,      15,          0,            24,                24,           3}}},
    {_25_78125G_SR_LR,         {.txComphyC28GP4={ 8,  8,   2}}, {.rxComphyC28GP4={12,      6,        0,      12,      15,       2,     2,       2,      3,      15,         0,      15,          0,            24,                24,           5}}},
    {_26_5625G,                {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={12,      6,        0,      12,      15,       2,     2,       2,      3,      15,         0,      15,          0,            24,                24,           3}}},
    {_26_5625G_SR_LR,          {.txComphyC28GP4={ 8,  8,   2}}, {.rxComphyC28GP4={12,      6,        0,      12,      15,       2,     2,       2,      3,      15,         0,      15,          0,            24,                24,           5}}},

    {LAST_MV_HWS_SERDES_SPEED, {.txComphyC28GP4={ 0,  0,   0}}, {.rxComphyC28GP4={0,       0,        0,       0,       0,       0,     0,       0,      0,       0,         0,       0,          0,             0,                 0,           0}}},
};
#else
{
    {0}
};
#endif

HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes0Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes1Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes2Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes3Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes4Data  = {mvHwsComphyC28GP4X1PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes5Data  = {mvHwsComphyC28GP4X1PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes6Data  = {mvHwsComphyC28GP4X1PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes7Data  = {mvHwsComphyC28GP4X1PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes8Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes9Data  = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes10Data = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5xSerdes11Data = {mvHwsComphyC28GP4X4PinToRegMapSdw, NULL, ac5xTxRxTuneParams, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC ac5xSerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes0Data}},
    {NULL, 1,  1, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes1Data}},
    {NULL, 2,  2, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes2Data}},
    {NULL, 3,  3, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes3Data}},
    {NULL, 4,  0, COM_PHY_C28GP4X1,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes4Data}},
    {NULL, 5,  0, COM_PHY_C28GP4X1,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes5Data}},
    {NULL, 6,  0, COM_PHY_C28GP4X1,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes6Data}},
    {NULL, 7,  0, COM_PHY_C28GP4X1,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes7Data}},
    {NULL, 8,  0, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes8Data}},
    {NULL, 9,  1, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes9Data}},
    {NULL, 10, 2, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes10Data}},
    {NULL, 11, 3, COM_PHY_C28GP4X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5xSerdes11Data}},
};
/************************* pre-declaration ***********************************************/


/*****************************************************************************************/

/*******************************************************************************
* hwsPcsIfInit
*
* DESCRIPTION:
*       Init all supported PCS types relevant for devices.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS **hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    if(!hwsIsIronmanAsPhoenix())
    {
        CHECK_STATUS(mvHwsMtiPcs50Rev2IfInit(hwsPcsFuncsPtr));
        CHECK_STATUS(mvHwsMtiPcs25Rev2IfInit(hwsPcsFuncsPtr));
        CHECK_STATUS(mvHwsMtiLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));
        CHECK_STATUS(mvHwsMtiCpuPcsRev2IfInit(hwsPcsFuncsPtr));
        CHECK_STATUS(mvHwsMtiPcs100Rev2IfInit(hwsPcsFuncsPtr));
        CHECK_STATUS(mvHwsMtiCpuSgPcsIfInit(hwsPcsFuncsPtr));
    }

    CHECK_STATUS(mvHwsMtiUsxLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiUsxPcsRev2IfInit(hwsPcsFuncsPtr));
    return GT_OK;
}

/*******************************************************************************
* hwsMacIfInit
*
* DESCRIPTION:
*       Init all supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS **hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    if(!hwsIsIronmanAsPhoenix())
    {
        CHECK_STATUS(mvHwsMti100MacRev2IfInit(hwsMacFuncsPtr));
        CHECK_STATUS(mvHwsMtiCpuMacRev2IfInit(hwsMacFuncsPtr));
    }
    CHECK_STATUS(mvHwsMtiUsxMacRev2IfInit(hwsMacFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsPhoenixSerdesIfInit
*
* DESCRIPTION:
*       Init all supported Serdes types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS hwsPhoenixSerdesIfInit
(
    GT_U8 devNum,
    MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock,
    GT_BOOL callbackInitOnly
)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;

    if(hwsIsIronmanAsPhoenix())
    {
        return GT_OK;
    }

    if(GT_FALSE==callbackInitOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {

            CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, GT_TRUE));

            /* Enable SWD rx_clk_4x_enable, tx_clk_4x_enable */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                SDW_LANE_CONTROL_1,
                                                0xC000000,
                                                0xC000000));
        }
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    if (serdesRefClock == MHz_25)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X1, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X4, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else if (serdesRefClock == MHz_156)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X1, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X4, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else
    {
        return GT_BAD_STATE;
    }

    if(GT_FALSE==callbackInitOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {

            {
                /* Enable SD(internal memory) TX: TXDCLK_4X_EN_LANE */
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0x2020,
                                                    0x40000000,
                                                    0x40000000));
                /* Enable SD(internal memory) RX: RXDCLK_4X_EN_LANE */
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0x211C,
                                                    0x1,
                                                    0x1));
            }
        }
    }
    return GT_OK;
}

static  GT_U32  ironmanAsPhoenix = 0;
/**
* @internal hwsIronmanAsPhoenix function
* @endinternal
* function to allow ironman temporary supported in the HWS as 'phoenix' (AC5X)
*/
GT_STATUS hwsIronmanAsPhoenix(void)
{
    ironmanAsPhoenix = 1;
    return GT_OK;
}

/**
* @internal hwsIsIronmanAsPhoenix function
* @endinternal
* function to check if ironman temporary supported in the HWS as 'phoenix' (AC5X)
*/
GT_U32  hwsIsIronmanAsPhoenix(void)
{
    return ironmanAsPhoenix;
}

/**
* @internal hwsPhoenixRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsPhoenixRegDbInit
(
    GT_U8 devNum
)
{
    GT_STATUS rc;

    if(!hwsIsIronmanAsPhoenix())
    {
        rc =  mvHwsRegDbInit(devNum, MTI_EXT_UNIT, ac5xMtiExtRegDb);
        rc += mvHwsRegDbInit(devNum, MTI_MAC100_UNIT, hawkMtiMac100RegDb);
        rc += mvHwsRegDbInit(devNum, MTI_PCS50_UNIT, hawkMtiPcs50UnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_PCS25_UNIT, ac5xPcs25UnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_RSFEC_UNIT, ac5xMtiRsFecUnitsDb);

        rc += mvHwsRegDbInit(devNum, MIF_400_UNIT, hawkMifRegistersDb);
        rc += mvHwsRegDbInit(devNum, MIF_USX_UNIT, hawkMifRegistersDb);
        rc += mvHwsRegDbInit(devNum, MIF_CPU_UNIT, hawkMifRegistersDb);

        rc += mvHwsRegDbInit(devNum, ANP_400_UNIT, ac5xAnpUnitsDb);
        rc += mvHwsRegDbInit(devNum, ANP_USX_UNIT, ac5xAnpUnitsDb);
        rc += mvHwsRegDbInit(devNum, ANP_USX_O_UNIT, ac5xAnpUnitsDb);
        rc += mvHwsRegDbInit(devNum, ANP_CPU_UNIT, ac5xAnpUnitsDb);

        rc += mvHwsRegDbInit(devNum, AN_400_UNIT, anUnitsDb);
        rc += mvHwsRegDbInit(devNum, AN_USX_UNIT, anUnitsDb);
        rc += mvHwsRegDbInit(devNum, AN_CPU_UNIT, anUnitsDb);

        rc += mvHwsRegDbInit(devNum, MTI_USX_MAC_UNIT, hawkMtiMac100RegDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_EXT_UNIT, hawkMtipUsxExtUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_LPCS_UNIT, hawkLpcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_MULTIPLEXER_UNIT, hawkUsxmUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_PCS_UNIT, hawkUsxPcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_UNIT, hawkUsxRsfecUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_CPU_RSFEC_UNIT, hawkCpuRsfecUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_CPU_EXT_UNIT, hawkMtipCpuExtUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_CPU_PCS_UNIT, hawkCpuPcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_CPU_MAC_UNIT, hawkCpuMacUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_PCS100_UNIT, hawkMtiPcs100UnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_LOW_SP_PCS_UNIT, hawkLpcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_CPU_SGPCS_UNIT, hawkCpuSgpcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
    }
    else
    {
        rc = 0;
        rc += mvHwsRegDbInit(devNum, MIF_USX_UNIT, hawkMifRegistersDb);
        rc += mvHwsRegDbInit(devNum, ANP_USX_UNIT, ac5xAnpUnitsDb);
        rc += mvHwsRegDbInit(devNum, ANP_USX_O_UNIT, ac5xAnpUnitsDb);
        rc += mvHwsRegDbInit(devNum, AN_USX_UNIT, anUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_MAC_UNIT, hawkMtiMac100RegDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_EXT_UNIT, hawkMtipUsxExtUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_LPCS_UNIT, hawkLpcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_MULTIPLEXER_UNIT, hawkUsxmUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_PCS_UNIT, hawkUsxPcsUnitsDb);
        rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_UNIT, hawkUsxRsfecUnitsDb);
    }

    return rc;

}


/**
* @internal hwsPhoenixIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsPhoenixIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_STATUS rc = GT_OK;

    if (funcPtr == NULL)
        return GT_FAIL;

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
        return GT_BAD_PTR;
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
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;

    hwsDeviceSpecInfo[devNum].devType = AC5X;

    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;

    rc =  hwsPhoenixRegDbInit(devNum);

    return rc;
}

#ifdef SHARED_MEMORY

GT_STATUS hwsPhoenixIfReIoad
(
    GT_U8 devNum
)
{

    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    CHECK_STATUS(hwsPhoenixSerdesGetClockAndInit(devNum,GT_TRUE));

    return GT_OK;
}

#endif

GT_VOID hwsPhoenixPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    const MV_HWS_PORT_INIT_PARAMS **hwsPortsPhoenixParamsSupModesMap;

    if(hwsIsIronmanAsPhoenix())
    {
        hwsPortsPhoenixParamsSupModesMap = hwsPortsIronman_ParamsSupModesMap;
    }
    else
    {
        hwsPortsPhoenixParamsSupModesMap = hwsPortsPhoenix_ParamsSupModesMap;
    }

    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsPhoenixParamsSupModesMap;
}


GT_STATUS hwsPhoenixSerdesGetClockAndInit
(
    GT_U8 devNum,
    GT_BOOL callbackInitOnly
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock;
    MV_HWS_SERDES_REF_CLOCK_GET_FUNC_PTR    serdesRefClockGetFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    serdesRefClockGetFunc = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;

    if(hwsIsIronmanAsPhoenix())
    {
        /* dont loop on SERDEes ... as WM not have them at the moment */
    }
    else
    /* By default - give control over the SerDes to the register-file instead of the ANP */
    if (serdesRefClockGetFunc != NULL)
    {
        CHECK_STATUS(serdesRefClockGetFunc(devNum, &serdesRefClock));

        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsPhoenixSerdesIfInit(devNum,serdesRefClock,callbackInitOnly));
    }

    return GT_OK;
}

GT_VOID hwsPhoenixIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
     MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
     GT_U32  data, version;

     hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc = funcPtr->serdesRefClockGet;
    }

#ifndef MV_HWS_FREE_RTOS
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
    }
#endif



    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);


    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsHawkPortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAnpStartFunc = mvHwsHawkPortAnpStart;

#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsHawkPortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortGeneralAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = hwsPhoenixPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsPhoenixPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsExtMacClockEnableGet;
}

/*******************************************************************************
* hwsPhoenixIfInit
*
* DESCRIPTION:
*       Init all supported units needed for port initialization.
*       Must be called per device.
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsPhoenixIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    MV_HWS_DEV_TYPE               devType;
    GT_BOOL                       skipWritingHW = GT_FALSE;
    GT_U32                        sdIndex;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if (PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));

    if (funcPtr == NULL)
    {
        skipWritingHW = GT_TRUE;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(phoenixDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsPhoenixIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = AC5X;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;

    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_C28GP4X4; /* most common SD type in the device */
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(ac5xSerdesDb) / sizeof(ac5xSerdesDb[0]);
#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
    for (sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {
        CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, sdIndex, &ac5xSerdesDb[sdIndex]));
    }
#else
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
#endif

    CHECK_STATUS(hwsPhoenixPortElementsDbInit(devNum));

    hwsPhoenixPortsParamsSupModesMapSet(devNum);

    if(hwsIsIronmanAsPhoenix())
    {
        hwsDeviceSpecInfo[devNum].portsNum = 56;/*56 MACs +1 SDMA ports */
    }
    else
    {
        hwsDeviceSpecInfo[devNum].portsNum = 54;/*54 MACs +2 SDMA ports */
    }

    devType = hwsDeviceSpecInfo[devNum].devType;


    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;
    hwsPhoenixIfInitHwsDevFunc(devNum,funcPtr);
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Phoenix ifInit start ******\n");
    }
#endif

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
#ifndef MV_HWS_REDUCED_BUILD
    if(PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] == GT_FALSE)
    {
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
    }
#endif

    /* if func ptr is null, do only init callback (no need to do serdes muxing and FW writing) */
    if (skipWritingHW == GT_TRUE)
    {
        CHECK_STATUS(hwsPhoenixSerdesGetClockAndInit(devNum, GT_TRUE));
    }
    else
    {
        CHECK_STATUS(hwsPhoenixSerdesGetClockAndInit(devNum, GT_FALSE));
    }

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* mvHwsMifInit for Ironman bypasses MIF initialization          */
    /* MIF devices initialized by prvCpssDxChPortDpIronmanDeviceInit */
    if (!((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
             (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
    {
        /* Init all MIF units relevant for current device */
        CHECK_STATUS(mvHwsMifInit(devNum));
    }
    /* Init all ANP units relevant for current device */
    CHECK_STATUS(mvHwsAnpInit(devNum,skipWritingHW /*False*/));

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_TRUE;

    if (skipWritingHW == GT_FALSE)
    {
        if (!hwsIsIronmanAsPhoenix())
        {
            /************************************************************/
            /* Power reduction feature - Raven MTI EXT units reset      */
            /************************************************************/

            CHECK_STATUS(mvHwsPhoenixRegisterMisconfigurationSet(devNum));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** Start of SDW Config ******\n");
        }
#endif

            /*
                AC5x - fix exists only for the Quad (28x4) and not for the single, as there was no need.
                To enable fix, set bit[3:0] to 0xF (bit per lane):
                /EPI/<SDW_28G>SDW %a/<SDW_28G> SDW IP TLU/SDW IP Units/DFX/SDW Metal Fix Register
            */
            for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
            {
                if ((hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesType == COM_PHY_C28GP4X4) &&
                    (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].internalLane == 0))
                {
                    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                        hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                        SDW_METAL_FIX,
                                                        0xF,
                                                        0xF));
                }

                if ((hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesType == COM_PHY_C28GP4X1) &&
                    (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].internalLane == 0))
                {
                    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                        hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                        SDW_METAL_FIX,
                                                        0x1,
                                                        0x1));
                }

            }
        }

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of SDW Config ******\n");
        }
#endif
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Phoenix ifInit end ********\n");
    }
#endif
    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD
/*******************************************************************************
* hwsSerdesIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported serdes types.
*
* INPUTS:
*       GT_U8 devNum
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void serdesIfClose(GT_U8 devNum)
{
    mvHwsComphySerdesDestroyDatabase(devNum);

    return;
}

/*******************************************************************************
* hwsPcsIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported PCS types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void pcsIfClose
(
    GT_U8 devNum
)
{
    CPSS_TBD_BOOKMARK_PHOENIX

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsMacIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void macIfClose
(
    GT_U8 devNum
)
{
    CPSS_TBD_BOOKMARK_PHOENIX

    devNum = devNum;

    return;
}

/**
* @internal hwsPhoenixSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for AC5x
*
* @param[in] devNum                   - device number
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
* @param[in] regType                  - type of required reg
* @param[out] addressPtr              - (ptr to) register
*                                       address None
*/
GT_VOID hwsSip6_10SerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          regType,
    GT_U32          *addressPtr
)
{
    GT_U32 baseAddr = 0;
    GT_U32 temp1, temp2;

    if (GT_OK != mvUnitExtInfoGet(devNum, SERDES_UNIT, serdesNum, &baseAddr, &temp1, &temp2)) /* phoenixUnitBaseAddrCalc / hawkUnitBaseAddrCalc */
    {
        *addressPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        return;
    }

    mvHwsSerdesSdwAddrCalc(devNum, serdesNum, regAddr, baseAddr, regType, addressPtr);

    return;
}

/*******************************************************************************
* hwsPhoenixIfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsPhoenixIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_FALSE;
    }
    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        PRV_SHARED_LAB_SERVICES_DIR_AC5X_DEV_INIT_SRC_GLOBAL_VAR(phoenixDbInitDone) = GT_FALSE;
    }

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);

    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

    mvHwsAnpClose(devNum);

    hwsPortsElementsClose(devNum);
}

#endif

GT_STATUS mvHwsPhoenixRegisterMisconfigurationSet
(
    IN GT_U8    devNum
)
{
    GT_U32                      rc;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      baseAddr, unitIndex, unitNum, regAddr;
    MV_HWS_REG_ADDR_FIELD_STC   fieldReg;
    GT_U32                      portNum;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** Start of mvHwsPhoenixRegisterMisconfigurationSet ******\n");
        }
#endif

    /*
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in0.set(1);
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in2.set(1);
    */
    extField = PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN0_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, 50, MTI_EXT_UNIT, extField, 0x1, NULL));
    extField = PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN2_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, 50, MTI_EXT_UNIT, extField, 0x1, NULL));

    /* USX PORTS - clock enable/disable */
    for (portNum = 0; portNum < 48; portNum+=8)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_USX_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_USX_EXT_UNIT, _10G_OUSGMII, MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E, &fieldReg, &convertIdx);
        CHECK_STATUS(rc);
        regAddr = baseAddr + fieldReg.regOffset;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));

    }

    /* CPU PORTS - clock enable/disable */
    for (portNum = 48; portNum < 50; portNum++)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum ));
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_CPU_EXT_UNIT, _10GBase_KR, MTIP_CPU_EXT_UNITS_CLOCKS_APP_CLK_EN_E, &fieldReg, &convertIdx);
        CHECK_STATUS(rc);
        regAddr = baseAddr + fieldReg.regOffset;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));
    }

    /* MTI100 PORTS - clock enable/disable */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, 50, &baseAddr, &unitIndex, &unitNum ));
    rc = genUnitRegDbEntryGet(devNum, 50, MTI_EXT_UNIT, _10GBase_KR, PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E, &fieldReg, &convertIdx);
    CHECK_STATUS(rc);
    regAddr = baseAddr + fieldReg.regOffset;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsPhoenixRegisterMisconfigurationSet ******\n");
        }
#endif

    return GT_OK;
}

