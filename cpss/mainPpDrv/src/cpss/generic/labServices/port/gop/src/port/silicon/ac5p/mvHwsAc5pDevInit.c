/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsAc5pDevInit.c
*
* DESCRIPTION:
*     Hawk specific HW Services init
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
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs400If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs200If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs50If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacRev2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiUsxPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuSgPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

extern  GT_BOOL hwsPpHwTraceFlag;

extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiExtRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiExtBrRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100RegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100BrRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac400RegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs400UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs200UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs100UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMifRegistersDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs50UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiRsFecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkAnpUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC anUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtipUsxExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkLpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxmUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxRsfecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkUsxPcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuRsfecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtipCpuExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuPcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuMacUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkCpuSgpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkRsfecStatisticsUnitsDb[];

GT_STATUS mvHwsHawkRegisterMisconfigurationSet
(
    IN GT_U8    devNum
);

HWS_UNIT_BASE_ADDR_CALC_BIND    hawkBaseAddrCalcFunc = NULL;

/************************* definition *****************************************************/
const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMapSdw[] =
{
    /* C112GX4_PIN_RESET              = 0   */ {SDW_GENERAL_CONTROL_0           ,2                     ,1},
    /* C112GX4_PIN_ISOLATION_ENB      = 1   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_BG_RDY             = 2   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_SIF_SEL            = 3   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_MCU_CLK            = 4   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_DIRECT_ACCESS_EN   = 5   */ {SDW_GENERAL_CONTROL_1           ,15                    ,1},
    /* C112GX4_PIN_PRAM_FORCE_RESET   = 5   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_PRAM_RESET         = 5   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_PRAM_SOC_EN        = 5   */ {SDW_GENERAL_CONTROL_1           ,14                    ,1},
    /* C112GX4_PIN_PRAM_SIF_SEL       = 5   */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* C112GX4_PIN_PHY_MODE           = 5   */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
    /* C112GX4_PIN_REFCLK_SEL         = 6   */ {SDW_GENERAL_CONTROL_1           ,1                     ,1},
    /* C112GX4_PIN_REF_FREF_SEL       = 7   */ {SDW_GENERAL_CONTROL_1           ,3                     ,5},
    /* C112GX4_PIN_PHY_GEN_TX0        = 8   */ {SDW_LANE_CONTROL_0              ,8                     ,5},
    /* C112GX4_PIN_PHY_GEN_TX1        = 9   */ {SDW_LANE_CONTROL_0              ,8                     ,5},
    /* C112GX4_PIN_PHY_GEN_TX2        = 10  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
    /* C112GX4_PIN_PHY_GEN_TX3        = 11  */ {SDW_LANE_CONTROL_0              ,8                     ,5},
    /* C112GX4_PIN_PHY_GEN_RX0        = 12  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
    /* C112GX4_PIN_PHY_GEN_RX1        = 13  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
    /* C112GX4_PIN_PHY_GEN_RX2        = 14  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
    /* C112GX4_PIN_PHY_GEN_RX3        = 15  */ {SDW_LANE_CONTROL_0              ,3                     ,5},
    /* C112GX4_PIN_DFE_EN0            = 16  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
    /* C112GX4_PIN_DFE_EN1            = 17  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
    /* C112GX4_PIN_DFE_EN2            = 18  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
    /* C112GX4_PIN_DFE_EN3            = 19  */ {SDW_LANE_CONTROL_2              ,2                     ,1},
    /* C112GX4_PIN_DFE_UPDATE_DIS0    = 20  */ {SDW_LANE_CONTROL_2              ,0                     ,1},
    /* C112GX4_PIN_DFE_UPDATE_DIS1    = 21  */ {SDW_LANE_CONTROL_2              ,0                     ,1},
    /* C112GX4_PIN_DFE_UPDATE_DIS2    = 22  */ {SDW_LANE_CONTROL_2              ,0                     ,1},
    /* C112GX4_PIN_DFE_UPDATE_DIS3    = 23  */ {SDW_LANE_CONTROL_2              ,0                     ,1},
    /* C112GX4_PIN_PU_PLL0            = 24  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* C112GX4_PIN_PU_PLL1            = 25  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* C112GX4_PIN_PU_PLL2            = 26  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* C112GX4_PIN_PU_PLL3            = 27  */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* C112GX4_PIN_PU_RX0             = 28  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* C112GX4_PIN_PU_RX1             = 29  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* C112GX4_PIN_PU_RX2             = 30  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* C112GX4_PIN_PU_RX3             = 31  */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* C112GX4_PIN_PU_TX0             = 32  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* C112GX4_PIN_PU_TX1             = 33  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* C112GX4_PIN_PU_TX2             = 34  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* C112GX4_PIN_PU_TX3             = 35  */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* C112GX4_PIN_TX_IDLE0           = 36  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
    /* C112GX4_PIN_TX_IDLE1           = 37  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
    /* C112GX4_PIN_TX_IDLE2           = 38  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
    /* C112GX4_PIN_TX_IDLE3           = 39  */ {SDW_LANE_CONTROL_0              ,24                    ,1},
    /* C112GX4_PIN_PU_IVREF           = 40  */ {SDW_GENERAL_CONTROL_0           ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_ENABLE0   = 41  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_ENABLE1   = 42  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_ENABLE2   = 43  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_ENABLE3   = 44  */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_COMPLETE0 = 45  */ {SDW_LANE_STATUS_1               ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_COMPLETE1 = 46  */ {SDW_LANE_STATUS_1               ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_COMPLETE2 = 47  */ {SDW_LANE_STATUS_1               ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_COMPLETE3 = 48  */ {SDW_LANE_STATUS_1               ,0                     ,1},
    /* C112GX4_PIN_RX_TRAIN_FAILED0   = 49  */ {SDW_LANE_STATUS_1               ,1                     ,1},
    /* C112GX4_PIN_RX_TRAIN_FAILED1   = 50  */ {SDW_LANE_STATUS_1               ,1                     ,1},
    /* C112GX4_PIN_RX_TRAIN_FAILED2   = 51  */ {SDW_LANE_STATUS_1               ,1                     ,1},
    /* C112GX4_PIN_RX_TRAIN_FAILED3   = 52  */ {SDW_LANE_STATUS_1               ,1                     ,1},
    /* C112GX4_PIN_TX_TRAIN_ENABLE0   = 53  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* C112GX4_PIN_TX_TRAIN_ENABLE1   = 54  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* C112GX4_PIN_TX_TRAIN_ENABLE2   = 55  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* C112GX4_PIN_TX_TRAIN_ENABLE3   = 56  */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* C112GX4_PIN_TX_TRAIN_COMPLETE0 = 57  */ {SDW_LANE_STATUS_1               ,2                     ,1},
    /* C112GX4_PIN_TX_TRAIN_COMPLETE1 = 58  */ {SDW_LANE_STATUS_1               ,2                     ,1},
    /* C112GX4_PIN_TX_TRAIN_COMPLETE2 = 59  */ {SDW_LANE_STATUS_1               ,2                     ,1},
    /* C112GX4_PIN_TX_TRAIN_COMPLETE3 = 60  */ {SDW_LANE_STATUS_1               ,2                     ,1},
    /* C112GX4_PIN_TX_TRAIN_FAILED0   = 61  */ {SDW_LANE_STATUS_1               ,3                     ,1},
    /* C112GX4_PIN_TX_TRAIN_FAILED1   = 62  */ {SDW_LANE_STATUS_1               ,3                     ,1},
    /* C112GX4_PIN_TX_TRAIN_FAILED2   = 63  */ {SDW_LANE_STATUS_1               ,3                     ,1},
    /* C112GX4_PIN_TX_TRAIN_FAILED3   = 64  */ {SDW_LANE_STATUS_1               ,3                     ,1},
    /* C112GX4_PIN_SQ_DETECTED_LPF0   = 65  */ {SDW_LANE_STATUS_0               ,6                     ,1},
    /* C112GX4_PIN_SQ_DETECTED_LPF1   = 66  */ {SDW_LANE_STATUS_0               ,6                     ,1},
    /* C112GX4_PIN_SQ_DETECTED_LPF2   = 67  */ {SDW_LANE_STATUS_0               ,6                     ,1},
    /* C112GX4_PIN_SQ_DETECTED_LPF3   = 68  */ {SDW_LANE_STATUS_0               ,6                     ,1},
    /* C112GX4_PIN_RX_INIT0           = 69  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* C112GX4_PIN_RX_INIT1           = 70  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* C112GX4_PIN_RX_INIT2           = 71  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* C112GX4_PIN_RX_INIT3           = 72  */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* C112GX4_PIN_RX_INIT_DONE0      = 73  */ {SDW_LANE_STATUS_0               ,4                     ,1},
    /* C112GX4_PIN_RX_INIT_DONE1      = 74  */ {SDW_LANE_STATUS_0               ,4                     ,1},
    /* C112GX4_PIN_RX_INIT_DONE2      = 75  */ {SDW_LANE_STATUS_0               ,4                     ,1},
    /* C112GX4_PIN_RX_INIT_DONE3      = 76  */ {SDW_LANE_STATUS_0               ,4                     ,1},
    /* C112GX4_PIN_DFE_PAT_DIS0       = 77  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
    /* C112GX4_PIN_DFE_PAT_DIS1       = 78  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
    /* C112GX4_PIN_DFE_PAT_DIS2       = 79  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
    /* C112GX4_PIN_DFE_PAT_DIS3       = 80  */ {SDW_LANE_CONTROL_2              ,1                     ,1},
};

#define NA NA_16BIT
/** COMPHY_12nm_Serdes_Init_rev0.23 */
const MV_HWS_SERDES_TXRX_TUNE_PARAMS ac5pTxRxTuneParams[] =
#ifndef WIN32
{/*
                                             PRE2 PRE MAIN POST                CURRENT1 RL1_SEL RL1_EXSEL RES1_SEL CAP1_SEL CL1_SEL EN_MID_FREQ CS1_MID RS1_MID RF_CTRL RL1_TIA_SEL RL1_TIA_EXTRA HPF_RSEL_1ST CURRENT_TIA_SEL RL2_TUNE RL2_SEL RS2_SEL CURRENT2_SEL	CAP2_SEL HPF_RSEL_2ND SELMUFI SELMUFF SELMUPI SELMUPF SQUELCH	*/
    {_1_25G,                {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={  0,       0,      0,       14,      15,      0,         0,        0,      0,      1,       0,           0,            0,            0,           3,      0,     13,         0,        15,         0,        4,       5,      4,      1,    11}}},
    {_1_25G_SR_LR,          {.txComphyC112G={ 0 , 0 , 47,  0}},{.rxComphyC112G={ NA,      NA,     NA,       14,      15,     NA,        NA,       NA,     NA,      1,      NA,          NA,           NA,           NA,           3,      0,     13,         0,        15,        NA,        4,       5,      4,      1,    11}}},
    {_2_578125,             {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ NA,      NA,     NA,       13,      15,     NA,        NA,       NA,     NA,      1,      NA,          NA,           NA,           NA,           3,      1,     12,         1,        15,        NA,        4,       5,      4,      1,    11}}},
    {_3_125G,               {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={  0,       0,      0,       13,      15,      0,         0,        0,      0,      1,       0,           0,            0,            0,           3,      1,     12,         1,        15,         0,        4,       5,      4,      1,    11}}},
    {_5G,                   {.txComphyC112G={ 0 , 0 , 45,  0}},{.rxComphyC112G={  1,       1,      0,       12,      15,      0,         0,        0,      0,      2,       1,           0,            0,            1,           3,      8,      0,         8,        15,         0,        4,       5,      4,      2,    11}}},
    {_5_15625G,             {.txComphyC112G={ 0 , 6 , 55,  0}},{.rxComphyC112G={  6,       6,      0,       12,      15,      0,         0,        0,      0,      2,       6,           0,            0,            6,           3,      8,      0,         8,        15,         0,        4,       5,      4,      2,    11}}},
    {_10G,                  {.txComphyC112G={ 0 , 0 , 61,  2}},{.rxComphyC112G={ 15,      15,      0,        6,       0,      0,         1,        0,      0,      4,      12,           0,            0,           12,           3,     15,      0,        15,         0,         0,        4,       5,      4,      3,    11}}},
    {_10_3125G,             {.txComphyC112G={ 0 , 0 , 61,  2}},{.rxComphyC112G={ 15,      15,      0,        6,       0,      0,         1,        0,      0,      4,      12,           0,            0,           12,           3,     15,      0,        15,         0,         0,        4,       5,      4,      3,    11}}},
    {_10_3125G_SR_LR,       {.txComphyC112G={ 0 , 0 , 42,  7}},{.rxComphyC112G={ 15,      15,      0,        6,       0,      0,         1,        0,      0,      4,      12,           0,            0,           12,           3,     15,      0,        15,         0,         0,        4,       4,      4,      4,    11}}},
    {_20_625G,              {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    11}}},
    {_25_78125G,            {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    11}}},
    {_25_78125G_SR_LR,      {.txComphyC112G={ 0 , 0 , 59,  4}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    13}}},
    {_26_5625G,             {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    11}}},
    {_26_5625G_SR_LR,       {.txComphyC112G={ 0 , 0 , 59,  4}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    13}}},
    {_27_34375G,            {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,    11}}},
    {_26_5625G_PAM4,        {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,     3}}},
    {_26_5625G_PAM4_SR_LR,  {.txComphyC112G={ 0 , 0 , 59,  4}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,     5}}},
    {_28_125G_PAM4,         {.txComphyC112G={ 0 , 0 , 63,  0}},{.rxComphyC112G={ 15,      15,      0,        5,      31,      0,         1,        3,      3,      9,      12,           0,            0,           12,           3,      3,      0,         3,        15,         0,        4,       4,      4,      4,     3}}},

    {LAST_MV_HWS_SERDES_SPEED,{.txComphyC112G={0, 0,   0,  0}},{.rxComphyC112G={ 0,        0,      0,        0,       0,      0,         0,        0,      0,      0,       0,           0,            0,            0,           0,      0,      0,         0,         0,         0,        0,       0,      0,      0,     0}}},
};
#else
{
    {0}
};
#endif
extern const MV_HWS_COMPHY_REG_FIELD_STC      mvHwsComphyC28GP4X1PinToRegMapSdw[];
extern const MV_HWS_SERDES_TXRX_TUNE_PARAMS   ac5xTxRxTuneParams[];
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes0Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes1Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes2Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes3Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes4Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes5Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes6Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes7Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes8Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes9Data  = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes10Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes11Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes12Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes13Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes14Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes15Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes16Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes17Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes18Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes19Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes20Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes21Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes22Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes23Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes24Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes25Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes26Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes27Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes28Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes29Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes30Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes31Data = {mvHwsComphyC112GX4PinToRegMapSdw, NULL, ac5pTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ac5pSerdes32Data = {mvHwsComphyC28GP4X1PinToRegMapSdw,NULL, ac5xTxRxTuneParams, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC ac5pSerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes0Data }},
    {NULL, 1,  1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes1Data }},
    {NULL, 2,  2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes2Data }},
    {NULL, 3,  3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes3Data }},
    {NULL, 4,  0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes4Data }},
    {NULL, 5,  1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes5Data }},
    {NULL, 6,  2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes6Data }},
    {NULL, 7,  3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes7Data }},
    {NULL, 8,  0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes8Data }},
    {NULL, 9,  1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes9Data }},
    {NULL, 10, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes10Data}},
    {NULL, 11, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes11Data}},
    {NULL, 12, 0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes12Data}},
    {NULL, 13, 1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes13Data}},
    {NULL, 14, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes14Data}},
    {NULL, 15, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes15Data}},
    {NULL, 16, 0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes16Data}},
    {NULL, 17, 1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes17Data}},
    {NULL, 18, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes18Data}},
    {NULL, 19, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes19Data}},
    {NULL, 20, 0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes20Data}},
    {NULL, 21, 1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes21Data}},
    {NULL, 22, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes22Data}},
    {NULL, 23, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes23Data}},
    {NULL, 24, 0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes24Data}},
    {NULL, 25, 1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes25Data}},
    {NULL, 26, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes26Data}},
    {NULL, 27, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes27Data}},
    {NULL, 28, 0, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes28Data}},
    {NULL, 29, 1, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes29Data}},
    {NULL, 30, 2, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes30Data}},
    {NULL, 31, 3, COM_PHY_C112GX4,   {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes31Data}},
    {NULL, 32, 0, COM_PHY_C28GP4X1,  {(MV_HWS_COMPHY_SERDES_DATA*)&ac5pSerdes32Data}}
};

/************************* Globals *******************************************************/

extern const MV_HWS_PORT_INIT_PARAMS **hwsPortsHawkParamsSupModesMap;

#ifdef MV_HWS_REDUCED_BUILD
 /* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL hawkDbInitDone = GT_FALSE;
#define PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    _var
#else
#define PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.ac5pDevInitSrc._var)
#endif

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

    CHECK_STATUS(mvHwsMtiPcs50Rev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs100Rev2IfInit(hwsPcsFuncsPtr));
    /*CHECK_STATUS(mvHwsMtiPcs200Rev2IfInit(hwsPcsFuncsPtr)); use 400 pcs */
    CHECK_STATUS(mvHwsMtiPcs400Rev2IfInit(hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsMtiUsxLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiUsxPcsRev2IfInit(hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsMtiLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsMtiCpuPcsRev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiCpuSgPcsIfInit(hwsPcsFuncsPtr));

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

    CHECK_STATUS(mvHwsMtiMac100BrRev2IfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMtiUsxMacRev2IfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMti400MacRev2IfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMtiCpuMacRev2IfInit(hwsMacFuncsPtr));


    return GT_OK;
}

/*******************************************************************************
* hwsHawkSerdesIfInit
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
static GT_STATUS hwsHawkSerdesIfInit(GT_U8 devNum, MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock,GT_BOOL callbackInitOnly)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;

    for(sdIndex = 0; (GT_FALSE==callbackInitOnly)&&sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {

        CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, GT_TRUE));

        /* Enable SWD rx_clk_4x_enable, tx_clk_4x_enable */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                            hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                            SDW_LANE_CONTROL_1,
                                            0xC000000,
                                            0xC000000));
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    if (serdesRefClock == MHz_25)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X1, hwsSerdesFuncsPtr,
                     (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                     callbackInitOnly));
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C112GX4,  hwsSerdesFuncsPtr,
                     (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware))? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                     callbackInitOnly));
    }
    else if (serdesRefClock == MHz_156)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C28GP4X1, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_C112GX4,  hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else
    {
        return GT_BAD_STATE;
    }

    for(sdIndex = 0; (GT_FALSE==callbackInitOnly)&&sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {

        {
            /* Enable SD(internal memory) TX: TXDCLK_4X_EN_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                0x2030,
                                                0x1,
                                                0x1));
            /* Enable SD(internal memory) RX: RXDCLK_4X_EN_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                0x2114,
                                                0x1,
                                                0x1));
        }
    }

    return GT_OK;
}

/**
* @internal hwsHawkRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsHawkRegDbInit
(
    GT_U8 devNum
)
{
    GT_STATUS rc;

    rc = mvHwsRegDbInit(devNum, MTI_EXT_UNIT, hawkMtiExtBrRegDb);
    rc += mvHwsRegDbInit(devNum, MTI_MAC100_UNIT, hawkMtiMac100BrRegDb);
    rc += mvHwsRegDbInit(devNum, MTI_MAC400_UNIT, hawkMtiMac400RegDb);
    rc += mvHwsRegDbInit(devNum, MIF_400_UNIT, hawkMifRegistersDb);
    rc += mvHwsRegDbInit(devNum, MIF_400_SEG_UNIT, hawkMifRegistersDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS400_UNIT, hawkMtiPcs400UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS200_UNIT, hawkMtiPcs200UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS100_UNIT, hawkMtiPcs100UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS50_UNIT, hawkMtiPcs50UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_UNIT, hawkMtiRsFecUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_400_UNIT, hawkAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_LOW_SP_PCS_UNIT, hawkLpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MIF_USX_UNIT, hawkMifRegistersDb);
    rc += mvHwsRegDbInit(devNum, MIF_CPU_UNIT, hawkMifRegistersDb);
    rc += mvHwsRegDbInit(devNum, ANP_USX_UNIT, hawkAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_USX_O_UNIT, hawkAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_CPU_UNIT, hawkAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_MAC_UNIT, hawkMtiMac100RegDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_EXT_UNIT, hawkMtipUsxExtUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_LPCS_UNIT, hawkLpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_MULTIPLEXER_UNIT, hawkUsxmUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_UNIT, hawkUsxRsfecUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_PCS_UNIT, hawkUsxPcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_RSFEC_UNIT, hawkCpuRsfecUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_EXT_UNIT, hawkMtipCpuExtUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_PCS_UNIT, hawkCpuPcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_MAC_UNIT, hawkCpuMacUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_SGPCS_UNIT, hawkCpuSgpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_400_UNIT, anUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_USX_UNIT, anUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_CPU_UNIT, anUnitsDb);

    return rc;
}


/**
* @internal hwsHawkIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsHawkIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_STATUS rc;
    GT_U32 i;

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

    hwsDeviceSpecInfo[devNum].devType = AC5P;
    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    rc = hwsHawkRegDbInit(devNum);

    return rc;
}


GT_STATUS hwsHawkSerdesGetClockAndInit
(
    GT_U8 devNum,
    GT_BOOL callbackInitOnly
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
     MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc != NULL)
    {
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc(devNum, &serdesRefClock));
        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsHawkSerdesIfInit(devNum,serdesRefClock,callbackInitOnly));
    }

     return GT_OK;
}



#ifdef SHARED_MEMORY

GT_STATUS hwsHawkIfReIoad
(
    GT_U8 devNum
)
{
    CHECK_STATUS(macIfInit(devNum));

    CHECK_STATUS(hwsHawkSerdesGetClockAndInit(devNum,GT_TRUE));
    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    return GT_OK;
}

#endif

GT_VOID hwsHawkPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    hwsHawkPortElementsDbInit();
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsHawkParamsSupModesMap;
}

GT_VOID hwsHawkIfInitHwsDevFunc
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
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = hwsHawkPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsHawkPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsExtMacClockEnableGet;
}

/*******************************************************************************
* hwsHawkIfInit
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
GT_STATUS hwsHawkIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    MV_HWS_DEV_TYPE               devType;
    MV_HWS_DEV_FUNC_PTRS          *hwsDevFunc;
    GT_BOOL                       skipWritingHW = GT_FALSE;
    GT_U32                        sdIndex;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if (PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));

    if (funcPtr == NULL)
    {
        skipWritingHW = GT_TRUE;
    }


#ifdef MV_HWS_REDUCED_BUILD
    if(hawkDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsHawkIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = AC5P;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_C112GX4; /* most common SD type in the device */
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(ac5pSerdesDb) / sizeof(ac5pSerdesDb[0]);
#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
    for (sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {
        CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, sdIndex, &ac5pSerdesDb[sdIndex]));
    }
#else
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = ac5pSerdesDb;
#endif


    hwsHawkPortsParamsSupModesMapSet(devNum);

    /**
     * To support MTI CPU Global MAC 105, portsNum 106 = 4 * 26 + 2
     */
    hwsDeviceSpecInfo[devNum].portsNum = (4*MV_HWS_AC5P_GOP_PORT_NUM_CNS) + 2;

    devType = hwsDeviceSpecInfo[devNum].devType;
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    hwsHawkIfInitHwsDevFunc(devNum,funcPtr);


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Hawk ifInit start ******\n");
    }
#endif

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
#ifndef MV_HWS_REDUCED_BUILD
    if(PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] == GT_FALSE)
    {
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
    }
#endif

    if (skipWritingHW == GT_TRUE)
    {
        CHECK_STATUS(hwsHawkSerdesGetClockAndInit(devNum, GT_TRUE));
    }
    else
    {
        CHECK_STATUS(hwsHawkSerdesGetClockAndInit(devNum, GT_FALSE));
    }


    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

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
    PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_TRUE;

    if (skipWritingHW == GT_FALSE)
    {
        /************************************************************/
        /* Power reduction feature - Raven MTI EXT units reset      */
        /************************************************************/
        CHECK_STATUS(mvHwsHawkRegisterMisconfigurationSet(devNum));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** Start of SDW Config ******\n");
        }
#endif

        /*
            AC5P - fix exists - both fix (a) and fix (b) exists for the 112Gx4. Only fix (a) exists for the 28G (CPU).

                CPU: bit[0] set to 0x1:
                    EPI/<SDW_28G>SDW IP/<SDW_28G> SDW IP TLU/SDW IP Units/DFX/SDW Metal Fix Register

                112G (need to set per Quad):
                    1. Enable fix in COMPHY - set bit[18] to 0x1:
                       EPI/<SDW>SDW Ip %a/<COMPHY_112G_X4> COMPHY_112G_X4_1_2/units/Memory Map/Address Block/Train Control 8
                    2. Enable fix in SDW - set bit[7:4] to 0xF:
                       EPI/<SDW>SDW Ip %a/<SDW> SDW IP TLU/SDW IP Units/DFX/SDW Metal Fix Register

                NOTE: fix (a) is still available in bits[3:0], it is don't care when using fix (b) and it is disabled by default. No need to change.
        */
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {
            if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesType == COM_PHY_C112GX4)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                            hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                            0x609C,
                                                            0x40000,
                                                            0x40000));

                if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].internalLane == 0)
                {
                    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                        hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                        SDW_METAL_FIX,
                                                        0xF0,
                                                        0xF0));
                }
            }
            else if(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesType == COM_PHY_C28GP4X1) /*CPU port (28G)*/
            {
                CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    SDW_METAL_FIX,
                                                    0x1,
                                                    0x1));
            }
        }

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of SDW Config ******\n");
        }
#endif

        /*************************************************************************/
        /* CPU port power reduction feature - Raven CPU MTI EXT units reset      */
        /*************************************************************************/
        CPSS_TBD_BOOKMARK_AC5P
#if 0
        for (portNum = 0; portNum < 2; portNum++)
        {
            CHECK_STATUS(mvHwsHawkCpuExtPowerReductionInit(devNum, portNum));
        }
#endif
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Hawk ifInit end ********\n");
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
    CPSS_TBD_BOOKMARK_AC5P

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
    CPSS_TBD_BOOKMARK_AC5P

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsHawkIfClose
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
void hwsHawkIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_FALSE;
    }
    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        PRV_SHARED_LAB_SERVICES_DIR_AC5P_DEV_INIT_SRC_GLOBAL_VAR(hawkDbInitDone) = GT_FALSE;
    }

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);

    mvHwsAnpClose(devNum);

    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif

GT_STATUS mvHwsHawkRegisterMisconfigurationSet
(
    IN GT_U8    devNum
)
{
    GT_U32                      rc;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      baseAddr, unitIndex, unitNum, regAddr;
    MV_HWS_REG_ADDR_FIELD_STC   fieldReg;
    GT_U32                      portNum;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsHawkRegisterMisconfigurationSet ******\n");
    }
#endif

    for (portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
    {
        rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _10GBase_KR, &convertIdx);
        if (rc == GT_OK)
        {
            if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum ));
                rc = genUnitRegDbEntryGet(devNum, portNum, MTI_CPU_EXT_UNIT, _10GBase_KR, MTIP_CPU_EXT_UNITS_CLOCKS_APP_CLK_EN_E, &fieldReg, &convertIdx);
                CHECK_STATUS(rc);
                regAddr = baseAddr + fieldReg.regOffset;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));
            }
            else
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum ));
                rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, _10GBase_KR, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E, &fieldReg, &convertIdx);
                CHECK_STATUS(rc);
                if (convertIdx.ciderIndexInUnit % 8 == 0)
                {
                    regAddr = baseAddr + fieldReg.regOffset;
                    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
                    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));


                    extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_REF_RESET__E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, 0x1, NULL));

                    /*
                        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in0.set(1);
                        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in2.set(1);
                        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in4.set(1);
                        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in6.set(1);
                    */
                    extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN0_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, 0x1, NULL));
                    extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN2_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, 0x1, NULL));
                    extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN4_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, 0x1, NULL));
                    extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN6_E;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, 0x1, NULL));
                }
            }
        }

        rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _10G_OUSGMII, &convertIdx);
        if (rc == GT_OK)
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_USX_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum ));
            rc = genUnitRegDbEntryGet(devNum, portNum, MTI_USX_EXT_UNIT, _10G_OUSGMII, MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E, &fieldReg, &convertIdx);
            CHECK_STATUS(rc);
            if (convertIdx.ciderIndexInUnit % 8 == 0)
            {
                regAddr = baseAddr + fieldReg.regOffset;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0xFFFFFFFF));
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 32, 0));
            }
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsHawkRegisterMisconfigurationSet ******\n");
    }
#endif

    return GT_OK;
}

