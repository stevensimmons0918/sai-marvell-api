/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtOs/gtOsExc.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <gtExtDrv/drivers/gtGenDrv.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CPU_AARCH64v8        1
#define X7121M_NUM_OF_SD     16 /* LINE SIDE **ONLY** */
#define MULTIPLE_ADDR        1

GT_BOOL xsmiInitDone = GT_FALSE;
extern GT_BOOL systemInitialized;

#define MZD_PMA_DEV    1
#define MZD_LINE_SIDE  3
#define MZD_HOST_SIDE  4
#define MZD_BOTH_SIDE  5
#define MZD_AUTONEG    7
#define MZD_PORT_REG   31
#define MZD_CHIP_REG   31

/* Chip Level Common Registers */
#define MZD_GLOBAL_RESET            0xF404
#define MZD_GLOBAL_INTR_STAT1       0xF420
#define MZD_GLOBAL_INTR_CNTL        0xF421
#define MZD_GLOBAL_INTR_STAT2       0xF422

/* Advanced Peripheral Bus (APB) Indirect Access Commands */
#define MZD_APB_READ_CMD          0x0000
#define MZD_APB_WRITE_CMD         0x0001
#define MZD_APB_WRITE_INC_CMD     0xC001     /* post increment on writes only */
#define MZD_APB_READ_INC_CMD      0x8000     /* post increment on reads only */

#define MZD_APB_ACCESS_CTRL       0xF0AB
#define MZD_APB_ACCESS_MSB_REG    0xF0AF
#define MZD_APB_ACCESS_LSB_REG    0xF0AC
#define MZD_APB_DATA_MSB          0xF0AD  /* Need to write first before LSB */
#define MZD_APB_DATA_LSB          0xF0AE

#define MZD_COMPHY_CMN_SYS_REG               0x0000A314UL

MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMap0[] =
{
/* 0    C112GX4_PIN_RESET               */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 1    C112GX4_PIN_ISOLATION_ENB       */  {MV_HWS_SERDES_TIED_PIN,        1,              0},
/* 2    C112GX4_PIN_BG_RDY              */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 3    C112GX4_PIN_SIF_SEL             */  {0xF423,                        0,/*lane num*/  1},
/* 4    C112GX4_PIN_MCU_CLK             */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 5    C112GX4_PIN_DIRECT_ACCESS_EN    */  {0xF024,                        0,              1},
/* 6    C112GX4_PIN_PRAM_FORCE_RESET    */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 7    C112GX4_PIN_PRAM_RESET          */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 8    C112GX4_PIN_PRAM_SOC_EN         */  {0xF024,                        1,              1},
/* 9    C112GX4_PIN_PRAM_SIF_SEL        */  {MV_HWS_SERDES_TIED_PIN,        0,              0},
/* 10   C112GX4_PIN_PHY_MODE            */  {0xF023,                        1,              3},
/* 11   C112GX4_PIN_REFCLK_SEL          */  {0xF023,                        9,              1},
/* 12   C112GX4_PIN_REF_FREF_SEL        */  {0xF023,                        4,              5},
/* 13   C112GX4_PIN_PHY_GEN_TX0         */  {0xF005,                        8,              5},
/* 14   C112GX4_PIN_PHY_GEN_TX1         */  {0xF006,                        8,              5},
/* 15   C112GX4_PIN_PHY_GEN_TX2         */  {0xF007,                        8,              5},
/* 16   C112GX4_PIN_PHY_GEN_TX3         */  {0xF008,                        8,              5},
/* 17   C112GX4_PIN_PHY_GEN_RX0         */  {0xF005,                        0,              5},
/* 18   C112GX4_PIN_PHY_GEN_RX1         */  {0xF006,                        0,              5},
/* 19   C112GX4_PIN_PHY_GEN_RX2         */  {0xF007,                        0,              5},
/* 20   C112GX4_PIN_PHY_GEN_RX3         */  {0xF008,                        0,              5},
/* 21   C112GX4_PIN_DFE_EN0             */  {0xF181,                        4,              1},
/* 22   C112GX4_PIN_DFE_EN1             */  {0xF1A1,                        4,              1},
/* 23   C112GX4_PIN_DFE_EN2             */  {0xF1C1,                        4,              1},
/* 24   C112GX4_PIN_DFE_EN3             */  {0xF1E1,                        4,              1},
/* 25   C112GX4_PIN_DFE_UPDATE_DIS0     */  {0xF181,                        0,              1},
/* 26   C112GX4_PIN_DFE_UPDATE_DIS1     */  {0xF1A1,                        0,              1},
/* 27   C112GX4_PIN_DFE_UPDATE_DIS2     */  {0xF1C1,                        0,              1},
/* 28   C112GX4_PIN_DFE_UPDATE_DIS3     */  {0xF1E1,                        0,              1},
/* 29   C112GX4_PIN_PU_PLL0             */  {0xF020,                        11,             1},
/* 30   C112GX4_PIN_PU_PLL1             */  {0xF020,                        12,             1},
/* 31   C112GX4_PIN_PU_PLL2             */  {0xF020,                        13,             1},
/* 32   C112GX4_PIN_PU_PLL3             */  {0xF020,                        14,             1},
/* 33   C112GX4_PIN_PU_RX0              */  {0xF020,                        1,              1},
/* 34   C112GX4_PIN_PU_RX1              */  {0xF020,                        2,              1},
/* 35   C112GX4_PIN_PU_RX2              */  {0xF020,                        3,              1},
/* 36   C112GX4_PIN_PU_RX3              */  {0xF020,                        4,              1},
/* 37   C112GX4_PIN_PU_TX0              */  {0xF020,                        6,              1},
/* 38   C112GX4_PIN_PU_TX1              */  {0xF020,                        7,              1},
/* 39   C112GX4_PIN_PU_TX2              */  {0xF020,                        8,              1},
/* 40   C112GX4_PIN_PU_TX3              */  {0xF020,                        9,              1},
/* 41   C112GX4_PIN_TX_IDLE0            */  {0xF181,                        6,              1},
/* 42   C112GX4_PIN_TX_IDLE1            */  {0xF1A1,                        6,              1},
/* 43   C112GX4_PIN_TX_IDLE2            */  {0xF1C1,                        6,              1},
/* 44   C112GX4_PIN_TX_IDLE3            */  {0xF1E1,                        6,              1},
/* 45   C112GX4_PIN_PU_IVREF            */  {0xF024,                        2,              1},
/* 46   C112GX4_PIN_RX_TRAIN_ENABLE0    */  {0xF181,                        12,             1},
/* 47   C112GX4_PIN_RX_TRAIN_ENABLE1    */  {0xF1A1,                        12,             1},
/* 48   C112GX4_PIN_RX_TRAIN_ENABLE2    */  {0xF1C1,                        12,             1},
/* 49   C112GX4_PIN_RX_TRAIN_ENABLE3    */  {0xF1E1,                        12,             1},
/* 50   C112GX4_PIN_RX_TRAIN_COMPLETE0  */  {0xF183,                        6,              1},
/* 51   C112GX4_PIN_RX_TRAIN_COMPLETE1  */  {0xF1A3,                        6,              1},
/* 52   C112GX4_PIN_RX_TRAIN_COMPLETE2  */  {0xF1C3,                        6,              1},
/* 53   C112GX4_PIN_RX_TRAIN_COMPLETE3  */  {0xF1E3,                        6,              1},
/* 54   C112GX4_PIN_RX_TRAIN_FAILED0    */  {0xF183,                        4,              1},
/* 55   C112GX4_PIN_RX_TRAIN_FAILED1    */  {0xF1A3,                        4,              1},
/* 56   C112GX4_PIN_RX_TRAIN_FAILED2    */  {0xF1C3,                        4,              1},
/* 57   C112GX4_PIN_RX_TRAIN_FAILED3    */  {0xF1E3,                        4,              1},
/* 58   C112GX4_PIN_TX_TRAIN_ENABLE0    */  {0xF181,                        10,             1},
/* 59   C112GX4_PIN_TX_TRAIN_ENABLE1    */  {0xF1A1,                        10,             1},
/* 60   C112GX4_PIN_TX_TRAIN_ENABLE2    */  {0xF1C1,                        10,             1},
/* 61   C112GX4_PIN_TX_TRAIN_ENABLE3    */  {0xF1E1,                        10,             1},
/* 62   C112GX4_PIN_TX_TRAIN_COMPLETE0  */  {0xF183,                        2,              1},
/* 63   C112GX4_PIN_TX_TRAIN_COMPLETE1  */  {0xF1A3,                        2,              1},
/* 64   C112GX4_PIN_TX_TRAIN_COMPLETE2  */  {0xF1C3,                        2,              1},
/* 65   C112GX4_PIN_TX_TRAIN_COMPLETE3  */  {0xF1E3,                        2,              1},
/* 66   C112GX4_PIN_TX_TRAIN_FAILED0    */  {0xF183,                        0,              1},
/* 67   C112GX4_PIN_TX_TRAIN_FAILED1    */  {0xF1A3,                        0,              1},
/* 68   C112GX4_PIN_TX_TRAIN_FAILED2    */  {0xF1C3,                        0,              1},
/* 69   C112GX4_PIN_TX_TRAIN_FAILED3    */  {0xF1E3,                        0,              1},
/* 70   C112GX4_PIN_SQ_DETECTED_LPF0    */  {0xF183,                        10,             1},
/* 71   C112GX4_PIN_SQ_DETECTED_LPF1    */  {0xF1A3,                        10,             1},
/* 72   C112GX4_PIN_SQ_DETECTED_LPF2    */  {0xF1C3,                        10,             1},
/* 73   C112GX4_PIN_SQ_DETECTED_LPF3    */  {0xF1E3,                        10,             1},
/* 74   C112GX4_PIN_RX_INIT0            */  {0xF181,                        14,             1},
/* 75   C112GX4_PIN_RX_INIT1            */  {0xF1A1,                        14,             1},
/* 76   C112GX4_PIN_RX_INIT2            */  {0xF1C1,                        14,             1},
/* 77   C112GX4_PIN_RX_INIT3            */  {0xF1E1,                        14,             1},
/* 78   C112GX4_PIN_RX_INIT_DONE0       */  {0xF183,                        8,              1},
/* 79   C112GX4_PIN_RX_INIT_DONE1       */  {0xF1A3,                        8,              1},
/* 80   C112GX4_PIN_RX_INIT_DONE2       */  {0xF1C3,                        8,              1},
/* 81   C112GX4_PIN_RX_INIT_DONE3       */  {0xF1E3,                        8,              1},
};

MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMap1[] =
{
/* 0    C112GX4_PIN_RESET               */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 1    C112GX4_PIN_ISOLATION_ENB       */  {MV_HWS_SERDES_TIED_PIN,        1,              0},
/* 2    C112GX4_PIN_BG_RDY              */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 3    C112GX4_PIN_SIF_SEL             */  {0xF423,                        1,/*lane num*/  1},
/* 4    C112GX4_PIN_MCU_CLK             */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 5    C112GX4_PIN_DIRECT_ACCESS_EN    */  {0xF024,                        0,              1},
/* 6    C112GX4_PIN_PRAM_FORCE_RESET    */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 7    C112GX4_PIN_PRAM_RESET          */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 8    C112GX4_PIN_PRAM_SOC_EN         */  {0xF024,                        1,              1},
/* 9    C112GX4_PIN_PRAM_SIF_SEL        */  {MV_HWS_SERDES_TIED_PIN,        0,              0},
/* 10   C112GX4_PIN_PHY_MODE            */  {0xF023,                        1,              3},
/* 11   C112GX4_PIN_REFCLK_SEL          */  {0xF023,                        9,              1},
/* 12   C112GX4_PIN_REF_FREF_SEL        */  {0xF023,                        4,              5},
/* 13   C112GX4_PIN_PHY_GEN_TX0         */  {0xF005,                        8,              5},
/* 14   C112GX4_PIN_PHY_GEN_TX1         */  {0xF006,                        8,              5},
/* 15   C112GX4_PIN_PHY_GEN_TX2         */  {0xF007,                        8,              5},
/* 16   C112GX4_PIN_PHY_GEN_TX3         */  {0xF008,                        8,              5},
/* 17   C112GX4_PIN_PHY_GEN_RX0         */  {0xF005,                        0,              5},
/* 18   C112GX4_PIN_PHY_GEN_RX1         */  {0xF006,                        0,              5},
/* 19   C112GX4_PIN_PHY_GEN_RX2         */  {0xF007,                        0,              5},
/* 20   C112GX4_PIN_PHY_GEN_RX3         */  {0xF008,                        0,              5},
/* 21   C112GX4_PIN_DFE_EN0             */  {0xF181,                        4,              1},
/* 22   C112GX4_PIN_DFE_EN1             */  {0xF1A1,                        4,              1},
/* 23   C112GX4_PIN_DFE_EN2             */  {0xF1C1,                        4,              1},
/* 24   C112GX4_PIN_DFE_EN3             */  {0xF1E1,                        4,              1},
/* 25   C112GX4_PIN_DFE_UPDATE_DIS0     */  {0xF181,                        0,              1},
/* 26   C112GX4_PIN_DFE_UPDATE_DIS1     */  {0xF1A1,                        0,              1},
/* 27   C112GX4_PIN_DFE_UPDATE_DIS2     */  {0xF1C1,                        0,              1},
/* 28   C112GX4_PIN_DFE_UPDATE_DIS3     */  {0xF1E1,                        0,              1},
/* 29   C112GX4_PIN_PU_PLL0             */  {0xF020,                        11,             1},
/* 30   C112GX4_PIN_PU_PLL1             */  {0xF020,                        12,             1},
/* 31   C112GX4_PIN_PU_PLL2             */  {0xF020,                        13,             1},
/* 32   C112GX4_PIN_PU_PLL3             */  {0xF020,                        14,             1},
/* 33   C112GX4_PIN_PU_RX0              */  {0xF020,                        1,              1},
/* 34   C112GX4_PIN_PU_RX1              */  {0xF020,                        2,              1},
/* 35   C112GX4_PIN_PU_RX2              */  {0xF020,                        3,              1},
/* 36   C112GX4_PIN_PU_RX3              */  {0xF020,                        4,              1},
/* 37   C112GX4_PIN_PU_TX0              */  {0xF020,                        6,              1},
/* 38   C112GX4_PIN_PU_TX1              */  {0xF020,                        7,              1},
/* 39   C112GX4_PIN_PU_TX2              */  {0xF020,                        8,              1},
/* 40   C112GX4_PIN_PU_TX3              */  {0xF020,                        9,              1},
/* 41   C112GX4_PIN_TX_IDLE0            */  {0xF181,                        6,              1},
/* 42   C112GX4_PIN_TX_IDLE1            */  {0xF1A1,                        6,              1},
/* 43   C112GX4_PIN_TX_IDLE2            */  {0xF1C1,                        6,              1},
/* 44   C112GX4_PIN_TX_IDLE3            */  {0xF1E1,                        6,              1},
/* 45   C112GX4_PIN_PU_IVREF            */  {0xF024,                        2,              1},
/* 46   C112GX4_PIN_RX_TRAIN_ENABLE0    */  {0xF181,                        12,             1},
/* 47   C112GX4_PIN_RX_TRAIN_ENABLE1    */  {0xF1A1,                        12,             1},
/* 48   C112GX4_PIN_RX_TRAIN_ENABLE2    */  {0xF1C1,                        12,             1},
/* 49   C112GX4_PIN_RX_TRAIN_ENABLE3    */  {0xF1E1,                        12,             1},
/* 50   C112GX4_PIN_RX_TRAIN_COMPLETE0  */  {0xF183,                        6,              1},
/* 51   C112GX4_PIN_RX_TRAIN_COMPLETE1  */  {0xF1A3,                        6,              1},
/* 52   C112GX4_PIN_RX_TRAIN_COMPLETE2  */  {0xF1C3,                        6,              1},
/* 53   C112GX4_PIN_RX_TRAIN_COMPLETE3  */  {0xF1E3,                        6,              1},
/* 54   C112GX4_PIN_RX_TRAIN_FAILED0    */  {0xF183,                        4,              1},
/* 55   C112GX4_PIN_RX_TRAIN_FAILED1    */  {0xF1A3,                        4,              1},
/* 56   C112GX4_PIN_RX_TRAIN_FAILED2    */  {0xF1C3,                        4,              1},
/* 57   C112GX4_PIN_RX_TRAIN_FAILED3    */  {0xF1E3,                        4,              1},
/* 58   C112GX4_PIN_TX_TRAIN_ENABLE0    */  {0xF181,                        10,             1},
/* 59   C112GX4_PIN_TX_TRAIN_ENABLE1    */  {0xF1A1,                        10,             1},
/* 60   C112GX4_PIN_TX_TRAIN_ENABLE2    */  {0xF1C1,                        10,             1},
/* 61   C112GX4_PIN_TX_TRAIN_ENABLE3    */  {0xF1E1,                        10,             1},
/* 62   C112GX4_PIN_TX_TRAIN_COMPLETE0  */  {0xF183,                        2,              1},
/* 63   C112GX4_PIN_TX_TRAIN_COMPLETE1  */  {0xF1A3,                        2,              1},
/* 64   C112GX4_PIN_TX_TRAIN_COMPLETE2  */  {0xF1C3,                        2,              1},
/* 65   C112GX4_PIN_TX_TRAIN_COMPLETE3  */  {0xF1E3,                        2,              1},
/* 66   C112GX4_PIN_TX_TRAIN_FAILED0    */  {0xF183,                        0,              1},
/* 67   C112GX4_PIN_TX_TRAIN_FAILED1    */  {0xF1A3,                        0,              1},
/* 68   C112GX4_PIN_TX_TRAIN_FAILED2    */  {0xF1C3,                        0,              1},
/* 69   C112GX4_PIN_TX_TRAIN_FAILED3    */  {0xF1E3,                        0,              1},
/* 70   C112GX4_PIN_SQ_DETECTED_LPF0    */  {0xF183,                        10,             1},
/* 71   C112GX4_PIN_SQ_DETECTED_LPF1    */  {0xF1A3,                        10,             1},
/* 72   C112GX4_PIN_SQ_DETECTED_LPF2    */  {0xF1C3,                        10,             1},
/* 73   C112GX4_PIN_SQ_DETECTED_LPF3    */  {0xF1E3,                        10,             1},
/* 74   C112GX4_PIN_RX_INIT0            */  {0xF181,                        14,             1},
/* 75   C112GX4_PIN_RX_INIT1            */  {0xF1A1,                        14,             1},
/* 76   C112GX4_PIN_RX_INIT2            */  {0xF1C1,                        14,             1},
/* 77   C112GX4_PIN_RX_INIT3            */  {0xF1E1,                        14,             1},
/* 78   C112GX4_PIN_RX_INIT_DONE0       */  {0xF183,                        8,              1},
/* 79   C112GX4_PIN_RX_INIT_DONE1       */  {0xF1A3,                        8,              1},
/* 80   C112GX4_PIN_RX_INIT_DONE2       */  {0xF1C3,                        8,              1},
/* 81   C112GX4_PIN_RX_INIT_DONE3       */  {0xF1E3,                        8,              1},
};

MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMap2[] =
{
/* 0    C112GX4_PIN_RESET               */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 1    C112GX4_PIN_ISOLATION_ENB       */  {MV_HWS_SERDES_TIED_PIN,        1,              0},
/* 2    C112GX4_PIN_BG_RDY              */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 3    C112GX4_PIN_SIF_SEL             */  {0xF423,                        2,/*lane num*/  1},
/* 4    C112GX4_PIN_MCU_CLK             */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 5    C112GX4_PIN_DIRECT_ACCESS_EN    */  {0xF024,                        0,              1},
/* 6    C112GX4_PIN_PRAM_FORCE_RESET    */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 7    C112GX4_PIN_PRAM_RESET          */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 8    C112GX4_PIN_PRAM_SOC_EN         */  {0xF024,                        1,              1},
/* 9    C112GX4_PIN_PRAM_SIF_SEL        */  {MV_HWS_SERDES_TIED_PIN,        0,              0},
/* 10   C112GX4_PIN_PHY_MODE            */  {0xF023,                        1,              3},
/* 11   C112GX4_PIN_REFCLK_SEL          */  {0xF023,                        9,              1},
/* 12   C112GX4_PIN_REF_FREF_SEL        */  {0xF023,                        4,              5},
/* 13   C112GX4_PIN_PHY_GEN_TX0         */  {0xF005,                        8,              5},
/* 14   C112GX4_PIN_PHY_GEN_TX1         */  {0xF006,                        8,              5},
/* 15   C112GX4_PIN_PHY_GEN_TX2         */  {0xF007,                        8,              5},
/* 16   C112GX4_PIN_PHY_GEN_TX3         */  {0xF008,                        8,              5},
/* 17   C112GX4_PIN_PHY_GEN_RX0         */  {0xF005,                        0,              5},
/* 18   C112GX4_PIN_PHY_GEN_RX1         */  {0xF006,                        0,              5},
/* 19   C112GX4_PIN_PHY_GEN_RX2         */  {0xF007,                        0,              5},
/* 20   C112GX4_PIN_PHY_GEN_RX3         */  {0xF008,                        0,              5},
/* 21   C112GX4_PIN_DFE_EN0             */  {0xF181,                        4,              1},
/* 22   C112GX4_PIN_DFE_EN1             */  {0xF1A1,                        4,              1},
/* 23   C112GX4_PIN_DFE_EN2             */  {0xF1C1,                        4,              1},
/* 24   C112GX4_PIN_DFE_EN3             */  {0xF1E1,                        4,              1},
/* 25   C112GX4_PIN_DFE_UPDATE_DIS0     */  {0xF181,                        0,              1},
/* 26   C112GX4_PIN_DFE_UPDATE_DIS1     */  {0xF1A1,                        0,              1},
/* 27   C112GX4_PIN_DFE_UPDATE_DIS2     */  {0xF1C1,                        0,              1},
/* 28   C112GX4_PIN_DFE_UPDATE_DIS3     */  {0xF1E1,                        0,              1},
/* 29   C112GX4_PIN_PU_PLL0             */  {0xF020,                        11,             1},
/* 30   C112GX4_PIN_PU_PLL1             */  {0xF020,                        12,             1},
/* 31   C112GX4_PIN_PU_PLL2             */  {0xF020,                        13,             1},
/* 32   C112GX4_PIN_PU_PLL3             */  {0xF020,                        14,             1},
/* 33   C112GX4_PIN_PU_RX0              */  {0xF020,                        1,              1},
/* 34   C112GX4_PIN_PU_RX1              */  {0xF020,                        2,              1},
/* 35   C112GX4_PIN_PU_RX2              */  {0xF020,                        3,              1},
/* 36   C112GX4_PIN_PU_RX3              */  {0xF020,                        4,              1},
/* 37   C112GX4_PIN_PU_TX0              */  {0xF020,                        6,              1},
/* 38   C112GX4_PIN_PU_TX1              */  {0xF020,                        7,              1},
/* 39   C112GX4_PIN_PU_TX2              */  {0xF020,                        8,              1},
/* 40   C112GX4_PIN_PU_TX3              */  {0xF020,                        9,              1},
/* 41   C112GX4_PIN_TX_IDLE0            */  {0xF181,                        6,              1},
/* 42   C112GX4_PIN_TX_IDLE1            */  {0xF1A1,                        6,              1},
/* 43   C112GX4_PIN_TX_IDLE2            */  {0xF1C1,                        6,              1},
/* 44   C112GX4_PIN_TX_IDLE3            */  {0xF1E1,                        6,              1},
/* 45   C112GX4_PIN_PU_IVREF            */  {0xF024,                        2,              1},
/* 46   C112GX4_PIN_RX_TRAIN_ENABLE0    */  {0xF181,                        12,             1},
/* 47   C112GX4_PIN_RX_TRAIN_ENABLE1    */  {0xF1A1,                        12,             1},
/* 48   C112GX4_PIN_RX_TRAIN_ENABLE2    */  {0xF1C1,                        12,             1},
/* 49   C112GX4_PIN_RX_TRAIN_ENABLE3    */  {0xF1E1,                        12,             1},
/* 50   C112GX4_PIN_RX_TRAIN_COMPLETE0  */  {0xF183,                        6,              1},
/* 51   C112GX4_PIN_RX_TRAIN_COMPLETE1  */  {0xF1A3,                        6,              1},
/* 52   C112GX4_PIN_RX_TRAIN_COMPLETE2  */  {0xF1C3,                        6,              1},
/* 53   C112GX4_PIN_RX_TRAIN_COMPLETE3  */  {0xF1E3,                        6,              1},
/* 54   C112GX4_PIN_RX_TRAIN_FAILED0    */  {0xF183,                        4,              1},
/* 55   C112GX4_PIN_RX_TRAIN_FAILED1    */  {0xF1A3,                        4,              1},
/* 56   C112GX4_PIN_RX_TRAIN_FAILED2    */  {0xF1C3,                        4,              1},
/* 57   C112GX4_PIN_RX_TRAIN_FAILED3    */  {0xF1E3,                        4,              1},
/* 58   C112GX4_PIN_TX_TRAIN_ENABLE0    */  {0xF181,                        10,             1},
/* 59   C112GX4_PIN_TX_TRAIN_ENABLE1    */  {0xF1A1,                        10,             1},
/* 60   C112GX4_PIN_TX_TRAIN_ENABLE2    */  {0xF1C1,                        10,             1},
/* 61   C112GX4_PIN_TX_TRAIN_ENABLE3    */  {0xF1E1,                        10,             1},
/* 62   C112GX4_PIN_TX_TRAIN_COMPLETE0  */  {0xF183,                        2,              1},
/* 63   C112GX4_PIN_TX_TRAIN_COMPLETE1  */  {0xF1A3,                        2,              1},
/* 64   C112GX4_PIN_TX_TRAIN_COMPLETE2  */  {0xF1C3,                        2,              1},
/* 65   C112GX4_PIN_TX_TRAIN_COMPLETE3  */  {0xF1E3,                        2,              1},
/* 66   C112GX4_PIN_TX_TRAIN_FAILED0    */  {0xF183,                        0,              1},
/* 67   C112GX4_PIN_TX_TRAIN_FAILED1    */  {0xF1A3,                        0,              1},
/* 68   C112GX4_PIN_TX_TRAIN_FAILED2    */  {0xF1C3,                        0,              1},
/* 69   C112GX4_PIN_TX_TRAIN_FAILED3    */  {0xF1E3,                        0,              1},
/* 70   C112GX4_PIN_SQ_DETECTED_LPF0    */  {0xF183,                        10,             1},
/* 71   C112GX4_PIN_SQ_DETECTED_LPF1    */  {0xF1A3,                        10,             1},
/* 72   C112GX4_PIN_SQ_DETECTED_LPF2    */  {0xF1C3,                        10,             1},
/* 73   C112GX4_PIN_SQ_DETECTED_LPF3    */  {0xF1E3,                        10,             1},
/* 74   C112GX4_PIN_RX_INIT0            */  {0xF181,                        14,             1},
/* 75   C112GX4_PIN_RX_INIT1            */  {0xF1A1,                        14,             1},
/* 76   C112GX4_PIN_RX_INIT2            */  {0xF1C1,                        14,             1},
/* 77   C112GX4_PIN_RX_INIT3            */  {0xF1E1,                        14,             1},
/* 78   C112GX4_PIN_RX_INIT_DONE0       */  {0xF183,                        8,              1},
/* 79   C112GX4_PIN_RX_INIT_DONE1       */  {0xF1A3,                        8,              1},
/* 80   C112GX4_PIN_RX_INIT_DONE2       */  {0xF1C3,                        8,              1},
/* 81   C112GX4_PIN_RX_INIT_DONE3       */  {0xF1E3,                        8,              1},
};

MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMap3[] =
{
/* 0    C112GX4_PIN_RESET               */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 1    C112GX4_PIN_ISOLATION_ENB       */  {MV_HWS_SERDES_TIED_PIN,        1,              0},
/* 2    C112GX4_PIN_BG_RDY              */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 3    C112GX4_PIN_SIF_SEL             */  {0xF423,                        3,/*lane num*/  1},
/* 4    C112GX4_PIN_MCU_CLK             */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 5    C112GX4_PIN_DIRECT_ACCESS_EN    */  {0xF024,                        0,              1},
/* 6    C112GX4_PIN_PRAM_FORCE_RESET    */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 7    C112GX4_PIN_PRAM_RESET          */  {MV_HWS_SERDES_REG_NA,          0,              0},
/* 8    C112GX4_PIN_PRAM_SOC_EN         */  {0xF024,                        1,              1},
/* 9    C112GX4_PIN_PRAM_SIF_SEL        */  {MV_HWS_SERDES_TIED_PIN,        0,              0},
/* 10   C112GX4_PIN_PHY_MODE            */  {0xF023,                        1,              3},
/* 11   C112GX4_PIN_REFCLK_SEL          */  {0xF023,                        9,              1},
/* 12   C112GX4_PIN_REF_FREF_SEL        */  {0xF023,                        4,              5},
/* 13   C112GX4_PIN_PHY_GEN_TX0         */  {0xF005,                        8,              5},
/* 14   C112GX4_PIN_PHY_GEN_TX1         */  {0xF006,                        8,              5},
/* 15   C112GX4_PIN_PHY_GEN_TX2         */  {0xF007,                        8,              5},
/* 16   C112GX4_PIN_PHY_GEN_TX3         */  {0xF008,                        8,              5},
/* 17   C112GX4_PIN_PHY_GEN_RX0         */  {0xF005,                        0,              5},
/* 18   C112GX4_PIN_PHY_GEN_RX1         */  {0xF006,                        0,              5},
/* 19   C112GX4_PIN_PHY_GEN_RX2         */  {0xF007,                        0,              5},
/* 20   C112GX4_PIN_PHY_GEN_RX3         */  {0xF008,                        0,              5},
/* 21   C112GX4_PIN_DFE_EN0             */  {0xF181,                        4,              1},
/* 22   C112GX4_PIN_DFE_EN1             */  {0xF1A1,                        4,              1},
/* 23   C112GX4_PIN_DFE_EN2             */  {0xF1C1,                        4,              1},
/* 24   C112GX4_PIN_DFE_EN3             */  {0xF1E1,                        4,              1},
/* 25   C112GX4_PIN_DFE_UPDATE_DIS0     */  {0xF181,                        0,              1},
/* 26   C112GX4_PIN_DFE_UPDATE_DIS1     */  {0xF1A1,                        0,              1},
/* 27   C112GX4_PIN_DFE_UPDATE_DIS2     */  {0xF1C1,                        0,              1},
/* 28   C112GX4_PIN_DFE_UPDATE_DIS3     */  {0xF1E1,                        0,              1},
/* 29   C112GX4_PIN_PU_PLL0             */  {0xF020,                        11,             1},
/* 30   C112GX4_PIN_PU_PLL1             */  {0xF020,                        12,             1},
/* 31   C112GX4_PIN_PU_PLL2             */  {0xF020,                        13,             1},
/* 32   C112GX4_PIN_PU_PLL3             */  {0xF020,                        14,             1},
/* 33   C112GX4_PIN_PU_RX0              */  {0xF020,                        1,              1},
/* 34   C112GX4_PIN_PU_RX1              */  {0xF020,                        2,              1},
/* 35   C112GX4_PIN_PU_RX2              */  {0xF020,                        3,              1},
/* 36   C112GX4_PIN_PU_RX3              */  {0xF020,                        4,              1},
/* 37   C112GX4_PIN_PU_TX0              */  {0xF020,                        6,              1},
/* 38   C112GX4_PIN_PU_TX1              */  {0xF020,                        7,              1},
/* 39   C112GX4_PIN_PU_TX2              */  {0xF020,                        8,              1},
/* 40   C112GX4_PIN_PU_TX3              */  {0xF020,                        9,              1},
/* 41   C112GX4_PIN_TX_IDLE0            */  {0xF181,                        6,              1},
/* 42   C112GX4_PIN_TX_IDLE1            */  {0xF1A1,                        6,              1},
/* 43   C112GX4_PIN_TX_IDLE2            */  {0xF1C1,                        6,              1},
/* 44   C112GX4_PIN_TX_IDLE3            */  {0xF1E1,                        6,              1},
/* 45   C112GX4_PIN_PU_IVREF            */  {0xF024,                        2,              1},
/* 46   C112GX4_PIN_RX_TRAIN_ENABLE0    */  {0xF181,                        12,             1},
/* 47   C112GX4_PIN_RX_TRAIN_ENABLE1    */  {0xF1A1,                        12,             1},
/* 48   C112GX4_PIN_RX_TRAIN_ENABLE2    */  {0xF1C1,                        12,             1},
/* 49   C112GX4_PIN_RX_TRAIN_ENABLE3    */  {0xF1E1,                        12,             1},
/* 50   C112GX4_PIN_RX_TRAIN_COMPLETE0  */  {0xF183,                        6,              1},
/* 51   C112GX4_PIN_RX_TRAIN_COMPLETE1  */  {0xF1A3,                        6,              1},
/* 52   C112GX4_PIN_RX_TRAIN_COMPLETE2  */  {0xF1C3,                        6,              1},
/* 53   C112GX4_PIN_RX_TRAIN_COMPLETE3  */  {0xF1E3,                        6,              1},
/* 54   C112GX4_PIN_RX_TRAIN_FAILED0    */  {0xF183,                        4,              1},
/* 55   C112GX4_PIN_RX_TRAIN_FAILED1    */  {0xF1A3,                        4,              1},
/* 56   C112GX4_PIN_RX_TRAIN_FAILED2    */  {0xF1C3,                        4,              1},
/* 57   C112GX4_PIN_RX_TRAIN_FAILED3    */  {0xF1E3,                        4,              1},
/* 58   C112GX4_PIN_TX_TRAIN_ENABLE0    */  {0xF181,                        10,             1},
/* 59   C112GX4_PIN_TX_TRAIN_ENABLE1    */  {0xF1A1,                        10,             1},
/* 60   C112GX4_PIN_TX_TRAIN_ENABLE2    */  {0xF1C1,                        10,             1},
/* 61   C112GX4_PIN_TX_TRAIN_ENABLE3    */  {0xF1E1,                        10,             1},
/* 62   C112GX4_PIN_TX_TRAIN_COMPLETE0  */  {0xF183,                        2,              1},
/* 63   C112GX4_PIN_TX_TRAIN_COMPLETE1  */  {0xF1A3,                        2,              1},
/* 64   C112GX4_PIN_TX_TRAIN_COMPLETE2  */  {0xF1C3,                        2,              1},
/* 65   C112GX4_PIN_TX_TRAIN_COMPLETE3  */  {0xF1E3,                        2,              1},
/* 66   C112GX4_PIN_TX_TRAIN_FAILED0    */  {0xF183,                        0,              1},
/* 67   C112GX4_PIN_TX_TRAIN_FAILED1    */  {0xF1A3,                        0,              1},
/* 68   C112GX4_PIN_TX_TRAIN_FAILED2    */  {0xF1C3,                        0,              1},
/* 69   C112GX4_PIN_TX_TRAIN_FAILED3    */  {0xF1E3,                        0,              1},
/* 70   C112GX4_PIN_SQ_DETECTED_LPF0    */  {0xF183,                        10,             1},
/* 71   C112GX4_PIN_SQ_DETECTED_LPF1    */  {0xF1A3,                        10,             1},
/* 72   C112GX4_PIN_SQ_DETECTED_LPF2    */  {0xF1C3,                        10,             1},
/* 73   C112GX4_PIN_SQ_DETECTED_LPF3    */  {0xF1E3,                        10,             1},
/* 74   C112GX4_PIN_RX_INIT0            */  {0xF181,                        14,             1},
/* 75   C112GX4_PIN_RX_INIT1            */  {0xF1A1,                        14,             1},
/* 76   C112GX4_PIN_RX_INIT2            */  {0xF1C1,                        14,             1},
/* 77   C112GX4_PIN_RX_INIT3            */  {0xF1E1,                        14,             1},
/* 78   C112GX4_PIN_RX_INIT_DONE0       */  {0xF183,                        8,              1},
/* 79   C112GX4_PIN_RX_INIT_DONE1       */  {0xF1A3,                        8,              1},
/* 80   C112GX4_PIN_RX_INIT_DONE2       */  {0xF1C3,                        8,              1},
/* 81   C112GX4_PIN_RX_INIT_DONE3       */  {0xF1E3,                        8,              1},
};

MV_HWS_COMPHY_SERDES_DATA x7121mSerdes0Data  = {mvHwsComphyC112GX4PinToRegMap0, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes1Data  = {mvHwsComphyC112GX4PinToRegMap1, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes2Data  = {mvHwsComphyC112GX4PinToRegMap2, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes3Data  = {mvHwsComphyC112GX4PinToRegMap3, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes4Data  = {mvHwsComphyC112GX4PinToRegMap0, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes5Data  = {mvHwsComphyC112GX4PinToRegMap1, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes6Data  = {mvHwsComphyC112GX4PinToRegMap2, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes7Data  = {mvHwsComphyC112GX4PinToRegMap3, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes8Data  = {mvHwsComphyC112GX4PinToRegMap0, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes9Data  = {mvHwsComphyC112GX4PinToRegMap1, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes10Data = {mvHwsComphyC112GX4PinToRegMap2, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes11Data = {mvHwsComphyC112GX4PinToRegMap3, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes12Data = {mvHwsComphyC112GX4PinToRegMap0, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes13Data = {mvHwsComphyC112GX4PinToRegMap1, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes14Data = {mvHwsComphyC112GX4PinToRegMap2, NULL, NULL, NULL};
MV_HWS_COMPHY_SERDES_DATA x7121mSerdes15Data = {mvHwsComphyC112GX4PinToRegMap3, NULL, NULL, NULL};

MV_HWS_PER_SERDES_INFO_STC x7121mSerdesDb[X7121M_NUM_OF_SD]= /* LINE SIDE **ONLY** */
{
    {NULL, 0,  0, COM_PHY_C112GX4, {&x7121mSerdes0Data}},
    {NULL, 1,  1, COM_PHY_C112GX4, {&x7121mSerdes1Data}},
    {NULL, 2,  2, COM_PHY_C112GX4, {&x7121mSerdes2Data}},
    {NULL, 3,  3, COM_PHY_C112GX4, {&x7121mSerdes3Data}},
    {NULL, 4,  0, COM_PHY_C112GX4, {&x7121mSerdes4Data}},
    {NULL, 5,  1, COM_PHY_C112GX4, {&x7121mSerdes5Data}},
    {NULL, 6,  2, COM_PHY_C112GX4, {&x7121mSerdes6Data}},
    {NULL, 7,  3, COM_PHY_C112GX4, {&x7121mSerdes7Data}},
    {NULL, 8,  0, COM_PHY_C112GX4, {&x7121mSerdes8Data}},
    {NULL, 9,  1, COM_PHY_C112GX4, {&x7121mSerdes9Data}},
    {NULL, 10, 2, COM_PHY_C112GX4, {&x7121mSerdes10Data}},
    {NULL, 11, 3, COM_PHY_C112GX4, {&x7121mSerdes11Data}},
    {NULL, 12, 0, COM_PHY_C112GX4, {&x7121mSerdes12Data}},
    {NULL, 13, 1, COM_PHY_C112GX4, {&x7121mSerdes13Data}},
    {NULL, 14, 2, COM_PHY_C112GX4, {&x7121mSerdes14Data}},
    {NULL, 15, 3, COM_PHY_C112GX4, {&x7121mSerdes15Data}},
};

/*******************************************************************************
* boardTypePrint
*
* DESCRIPTION:
*       This function prints type of PX board.
*
* INPUTS:
*       boardName - board name
*       devName   - device name
*
* OUTPUTS:
*       none
*
* RETURNS:
*       none
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_VOID boardTypePrint
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
}


GT_STATUS x7121mXsmiDump(GT_U32 xsmiPhyAddr, GT_U32 mmd, GT_U32 reg)
{
    GT_STATUS rc;
    GT_U32     val;

    rc = extDrvXSmiReadReg(xsmiPhyAddr, mmd, reg, &val);
    if (rc != GT_OK)
    {
        osPrintf("extDrvXSmiReadReg fails for PRTADD %d DEVADD %d, ADDR %d\n", xsmiPhyAddr, mmd, reg);
        return GT_FAIL;
    }

    osPrintf("READ MDIO: PRTADD %d, DEVADD %d, ADDR 0x%04X, OUT DATA 0x%04X\n", xsmiPhyAddr, mmd, reg,val);

    return GT_OK;
}

GT_STATUS x7121mExactDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 mils
)
{
    devNum = devNum;
    portGroup = portGroup;

    return cpssOsTimerWkAfter(mils);
}
GT_STATUS x7121mReadReg
(
    IN  GT_U8   devNum,     /* phyId   */
    IN  GT_U32  portGroup,  /* devAddr */
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
)
{
    GT_U32 tempData;

    extDrvXSmiReadReg(devNum, portGroup, address, &tempData);
    *data = tempData & mask;

    return GT_OK;
}

GT_STATUS x7121mWriteReg
(
    IN GT_U8  devNum,    /* phyId   */
    IN GT_U32 portGroup, /* devAddr */
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    GT_U32 tempData;

    extDrvXSmiReadReg(devNum, portGroup, address, &tempData);
    tempData = (tempData & (~mask)) | (data & mask);
    extDrvXSmiWriteReg(devNum, portGroup, address, tempData);

    return GT_OK;
}
GT_STATUS x7121mReadRegWrapper
(
    IN GT_U8  devNum,    /* phyId   */
    IN GT_U32 portGroup, /* devAddr */
    IN GT_U32 address,
    IN GT_U32 mask
)
{
    GT_STATUS rc;
    GT_UREG_DATA data;
    /*if (INTERNAL_REG == regType) regAddr = regAddr*4;*/

    rc = x7121mReadReg(devNum,portGroup,address,&data,mask);
    if (rc!=GT_OK)
    {
        return GT_FAIL;
    }
    osPrintf("read:  devNum=%d ,portGroup=%d, address=0x%x, mask=0x%x, data=0x%x\n",devNum, portGroup, address, mask, data);
    return GT_OK;
}

GT_STATUS x7121mWriteRegWrapper
(
    IN GT_U8  devNum,    /* phyId   */
    IN GT_U32 portGroup, /* devAddr */
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    GT_STATUS rc;
    /*if (INTERNAL_REG == regType) regAddr = regAddr*4;*/

    rc = x7121mWriteReg(devNum,portGroup,address,data,mask);
    if (rc!=GT_OK)
    {
        return GT_FAIL;
    }
    osPrintf("write: devNum=%d ,portGroup=%d, address=0x%x, mask=0x%x, data=0x%x\n",devNum, portGroup, address, mask, data);
    return GT_OK;
}

/******************************************************************************
 MZD_STATUS mzdHwAPBusRead
    Used for general indirect memory read access to the Advanced Peripheral Bus (APB).

    Returns the 32-bit register data
*******************************************************************************/
GT_STATUS x7121mAPBusRead
(
    IN  GT_U32 mdioPort,
    IN  GT_U32 regAPBAddr,
    OUT GT_U32 *data
)
{
    GT_U32 retVal;

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, MZD_APB_READ_CMD));                              */
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, MZD_APB_READ_CMD);

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_MSB_REG, (MZD_U16)((regAPBAddr & 0xFFFF0000) >> 16))); */
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_MSB_REG, (GT_U16)((regAPBAddr & 0xFFFF0000) >> 16));

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_LSB_REG, (MZD_U16)regAPBAddr));                        */
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_LSB_REG, (GT_U16)regAPBAddr);

    {
        GT_U32 cnt = 0;
        while (1)
        {
            /*MZD_ATTEMPT(mzdHwXmdioRead(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, &retVal));*/
            extDrvXSmiReadReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, &retVal);

            if ((retVal & 0x2) == 0x2)
            {
                break;
            }
            else
            {
                if ((cnt++) > 1000)
                {
                    *data = 0xffffffff;
                    return GT_FAIL;
                }
                /*MZD_ATTEMPT(mzdWait(pDev, 1));*/
                x7121mExactDelay(0,0,10);
            }
        }
    }

    /* MZD_ATTEMPT(mzdHwXmdioRead(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_DATA_MSB, &retVal));*/
    extDrvXSmiReadReg(mdioPort, MZD_CHIP_REG, MZD_APB_DATA_MSB, &retVal);
    *data = (retVal << 16) & 0xFFFF0000;

    /*MZD_ATTEMPT(mzdHwXmdioRead(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_DATA_LSB, &retVal));*/
    extDrvXSmiReadReg(mdioPort, MZD_CHIP_REG, MZD_APB_DATA_LSB, &retVal);
    *data |= retVal & 0xFFFF;

    return GT_OK;
}

/******************************************************************************
 MZD_STATUS mzdHwAPBusWrite
    Used for general indirect memory write access to the Advanced Peripheral Bus (APB).
*******************************************************************************/
GT_STATUS x7121mAPBusWrite
(
    IN GT_U32 mdioPort,
    IN GT_U32 regAPBAddr,
    IN GT_U32 data
)
{
    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, MZD_APB_WRITE_CMD));                            */
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_CTRL, MZD_APB_WRITE_CMD);

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_MSB_REG, (MZD_U16)((regAPBAddr & 0xFFFF0000) >> 16)));*/
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_MSB_REG, (GT_U16)((regAPBAddr & 0xFFFF0000) >> 16));

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_LSB_REG, (MZD_U16)regAPBAddr));                       */
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_ACCESS_LSB_REG, (GT_U16)regAPBAddr);

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_DATA_MSB, (data & 0xFFFF0000) >> 16));*/
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_DATA_MSB, ((data & 0xFFFF0000) >> 16));

    /*MZD_ATTEMPT(mzdHwXmdioWrite(pDev, pDev->mdioPort, MZD_CHIP_REG, MZD_APB_DATA_LSB, (data & 0xFFFF)));*/
    extDrvXSmiWriteReg(mdioPort, MZD_CHIP_REG, MZD_APB_DATA_LSB, (data & 0xFFFF));

    return GT_OK;
}

GT_STATUS x7121mSerdesReadReg
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA *data,
    IN  GT_UREG_DATA mask
)
{
    GT_U32                      serdesIndex;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U32                      regAPBAddr;
    GT_U16                      regTypeID;
    GT_U32                      regAddrOffset;
    GT_U32                      tempData;
    GT_U32                      portIndex;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo));
    portIndex = serdesNum/4;

    if (INTERNAL_REG == regType)
    {
#if MULTIPLE_ADDR
        regAddr = regAddr/4; /* Cancel the multiplication that was done on the higher level */
#endif
        if (portIndex == 0)
        {
            regTypeID = 0x4; /* MZD_LINE_SIDE */
            regAddrOffset = 0x0;
        }
        else if (portIndex == 1)
        {
            regTypeID = 0x4; /* MZD_LINE_SIDE */
            regAddrOffset = 0x80000;
        }
        else if (portIndex == 2)
        {
            regTypeID = 0x6; /* MZD_LINE_SIDE */
            regAddrOffset = 0x0;
        }
        else if (portIndex == 3)
        {
            regTypeID = 0x6; /* MZD_LINE_SIDE */
            regAddrOffset = 0x80000;
        }
        else
        {
            cpssOsPrintf("internalSerdesNum=%d is not valid\n", portIndex);
            return GT_BAD_PARAM;
        }

        regAPBAddr = (regTypeID & 0x0FFF) << 20;
        regAPBAddr |= ((regAddr + regAddrOffset) & 0x000FFFFF);
        CHECK_STATUS(x7121mAPBusRead(0, regAPBAddr, &tempData));
        *data = tempData & mask;
        /*osPrintf("mdioPort=%d, internalSerdesNum=%d, regAPBAddr=0x%x, data=0x%x\n",
                 port, serdesInfo->internalSerdesNum, regAPBAddr, *data);*/
    }
    else
    {
        /*extDrvXSmiReadReg((serdesNum/4), MZD_LINE_SIDE, regAddr, &tempData);
        *data = tempData & mask;*/
        if (regAddr == 0xF423/* 3  C112GX4_PIN_SIF_SEL */)
        {
            osPrintf("--[%s]--[%d]--\n",__FUNCNAME__,__LINE__);
            x7121mReadReg(0, MZD_CHIP_REG, regAddr, data, mask);
        }
        else
        {
            x7121mReadReg(portIndex, MZD_LINE_SIDE, regAddr, data, mask);
        }
    }
    return GT_OK;
}

GT_STATUS x7121mSerdesWriteReg
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA data,
    IN  GT_UREG_DATA mask
)
{
    GT_U32                      serdesIndex;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U32                      regAPBAddr;
    GT_U16                      regTypeID;
    GT_U32                      regAddrOffset;
    /*GT_U32                      tempData;*/
    GT_U32                      portIndex;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo));
    portIndex = serdesNum/4;

    if (INTERNAL_REG == regType)
    {
#if MULTIPLE_ADDR
        regAddr = regAddr/4; /* Cancel the multiplication that was done on the higher level */
#endif
        if (portIndex == 0)
        {
            regTypeID = 0x4; /* MZD_LINE_SIDE */
            regAddrOffset = 0x0;
        }
        else if (portIndex == 1)
        {
            regTypeID = 0x4; /* MZD_LINE_SIDE */
            regAddrOffset = 0x80000;
        }
        else if (portIndex == 2)
        {
            regTypeID = 0x6; /* MZD_LINE_SIDE */
            regAddrOffset = 0x0;
        }
        else if (portIndex == 3)
        {
            regTypeID = 0x6; /* MZD_LINE_SIDE */
            regAddrOffset = 0x80000;
        }
        else
        {
            cpssOsPrintf("internalSerdesNum=%d is not valid\n", portIndex);
            return GT_BAD_PARAM;
        }

        regAPBAddr = (regTypeID & 0x0FFF) << 20;
        regAPBAddr |= ((regAddr + regAddrOffset) & 0x000FFFFF);
        CHECK_STATUS(x7121mAPBusWrite(0, regAPBAddr, data & mask));
        /*osPrintf("mdioPort=%d, internalSerdesNum=%d, regAPBAddr=0x%x, data=0x%x\n",
                 port, serdesInfo->internalSerdesNum, regAPBAddr, data & mask);*/
    }
    else
    {
        /*extDrvXSmiReadReg((serdesNum/4), MZD_LINE_SIDE, regAddr, &tempData);
        tempData = (tempData & (~mask)) | (data & mask);
        extDrvXSmiWriteReg((serdesNum/4), MZD_LINE_SIDE, regAddr, tempData);*/
        if (regAddr == 0xF423/* 3  C112GX4_PIN_SIF_SEL */  )
        {
            osPrintf("--[%s]--[%d]--\n",__FUNCNAME__,__LINE__);
            x7121mWriteReg(0, MZD_CHIP_REG, regAddr, data, mask);
        }
        else
        {
            x7121mWriteReg(portIndex, MZD_LINE_SIDE, regAddr, data, mask);
            x7121mWriteReg(portIndex, MZD_HOST_SIDE, regAddr, data, mask);
        }
    }
    return GT_OK;
}

GT_STATUS x7121mSerdesReadRegWrapper
(
    IN  GT_U8  devNum,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA mask
)
{
    GT_STATUS rc;
    GT_UREG_DATA data;
    GT_U32                      serdesIndex;
    GT_U8                       serdesLane;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
#if MULTIPLE_ADDR
    if (INTERNAL_REG == regType) regAddr = regAddr*4;
#endif
    CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo));
    serdesLane = serdesInfo->internalLane;

    /* Lane switching is necessary for lane registers if broadcast mode is disabled */
    if ((regType == INTERNAL_REG) && (regAddr < 0x8000) && (serdesLane != 0xff))
    {
#if MULTIPLE_ADDR
        x7121mSerdesWriteReg(devNum,0,regType, serdesNum, MZD_COMPHY_CMN_SYS_REG * 4, serdesLane << 29, 0xE0000000);
#else
        x7121mSerdesWriteReg(devNum,0,regType, serdesNum, MZD_COMPHY_CMN_SYS_REG, serdesLane << 29, 0xE0000000);
#endif
    }

    rc = x7121mSerdesReadReg(devNum,0,regType,serdesNum,regAddr,&data,mask);
    if (rc!=GT_OK)
    {
        return GT_FAIL;
    }
    osPrintf("read:  devNum=%d ,regType=%d ,serdesNum=%3d ,regAddr=0x%08x, mask=0x%08x, data=0x%08x\n",devNum, regType, serdesNum, regAddr, mask, data);
    return GT_OK;
}

GT_STATUS x7121mSerdesWriteRegWrapper
(
    IN  GT_U8  devNum,
    IN  GT_U8  regType,
    IN  GT_U8  serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA data,
    IN  GT_UREG_DATA mask
)
{
    GT_STATUS rc;
    /*if (INTERNAL_REG == regType) regAddr = regAddr*4;*/
    GT_U32                      serdesIndex;
    GT_U8                       serdesLane;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
#if MULTIPLE_ADDR
    if (INTERNAL_REG == regType) regAddr = regAddr*4;
#endif
    CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo));
    serdesLane = serdesInfo->internalLane;

    /* Lane switching is necessary for lane registers if broadcast mode is disabled */
    if ((regType == INTERNAL_REG) && (regAddr < 0x8000) && (serdesLane != 0xff))
    {
#if MULTIPLE_ADDR
        x7121mSerdesWriteReg(devNum,0,regType, serdesNum, MZD_COMPHY_CMN_SYS_REG * 4, serdesLane << 29, 0xE0000000);
#else
        x7121mSerdesWriteReg(devNum,0,regType, serdesNum, MZD_COMPHY_CMN_SYS_REG, serdesLane << 29, 0xE0000000);
#endif
    }

    rc = x7121mSerdesWriteReg(devNum,0,regType,serdesNum,regAddr,data,mask);
    if (rc!=GT_OK)
    {
        return GT_FAIL;
    }
    osPrintf("write: devNum=%d ,regType=%d ,serdesNum=%3d ,regAddr=0x%08x, mask=0x%08x, data=0x%08x\n",devNum, regType, serdesNum, regAddr, mask, data);
    return GT_OK;
}

GT_STATUS x7121mInitSystem(GT_U8 loadFwBmp)
{
    GT_STATUS           rc;
    HWS_OS_FUNC_PTR     func;
    GT_U32              val;
    GT_U8               devNum = 0, serdesi;


    func.osExactDelayPtr            = x7121mExactDelay;
    func.osTimerWkPtr               = cpssOsTimerWkAfter;
    func.osMemSetPtr                = cpssOsMemSet;
    func.osFreePtr                  = cpssOsFree;
    func.osMallocPtr                = cpssOsMalloc;
    func.serdesRegGetAccess         = (MV_SERDES_REG_ACCESS_GET)x7121mSerdesReadReg;
    func.serdesRegSetAccess         = (MV_SERDES_REG_ACCESS_SET)x7121mSerdesWriteReg;
    func.serverRegGetAccess         = (MV_SERVER_REG_ACCESS_GET)cpssDrvHwPpResetAndInitControllerWriteReg;
    func.serverRegSetAccess         = (MV_SERVER_REG_ACCESS_SET)cpssDrvHwPpResetAndInitControllerReadReg;
    func.serverRegFieldSetAccess    = NULL;
    func.serverRegFieldGetAccess    = NULL;
    func.osMemCopyPtr               = (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
    func.timerGet                   = cpssOsTimeRT;
    func.registerGetAccess          = x7121mReadReg;
    func.registerSetAccess          = x7121mWriteReg;

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
    {
        cpssOsPrintf("osFatalErrorInit fail\n");
        return rc;
    }

    boardTypePrint("SVB" /*boardName*/, "88X7121M" /*devName*/);
    /* Init XSMI subsystem */
    if (xsmiInitDone == GT_FALSE)
    {
        rc = extDrvXSmiInit();
        if (rc != GT_OK)
        {
            osPrintf("extDrvXSmiInit fail retVal = %d\n", rc);
            return rc;
        }
    }
    else
    {
        xsmiInitDone = GT_TRUE;
    }

    rc = extDrvXSmiReadReg(0, 1, 2, &val);
    if (rc != GT_OK || val != 0x2b)
    {
        osPrintf("extDrvXSmiReadReg fails for PRTADD %d DEVADD %d, ADDR %d\n", 1, 0, 0);
        return GT_FAIL;
    }
    osPrintf("xsmi read success: PRTADD %d, DEVADD %d, ADDR 0x%04X, OUT DATA 0x%04X\n", 1, 0, 0,val);
#if 1
    osPrintf("Performing chip HW reset... ");
    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 10, 1, 1));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 1<<10, 1<<10);
    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 9, 1, 1));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 1<<9, 1<<9);

    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 10, 1, 0));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 0<<10, 1<<10);
    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 9, 1, 0));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 0<<9, 1<<9);

    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 14, 1, 1));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 1<<14, 1<<14);

    /*MZD_ATTEMPT(mzdHwSetPhyRegField(pDev, mdioPort1st, MZD_CHIP_REG, MZD_GLOBAL_RESET, 12, 1, 0));*/
    x7121mWriteReg(0, MZD_CHIP_REG, MZD_GLOBAL_RESET, 0<<12, 1<<12);

    osPrintf("HW reset done\n");
#endif
#if 1
    x7121mWriteReg(0, 3, 0xF099, 0x1, 0xFFFF);               /* MCM_BROADCAST */

    /* change speed for comphy to 313(312.5MHz) changeComphyPLL() */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E0, 0x8, 0x1FF);     /* CORE_PLL_0 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E1, 0x3, 0x1FF);     /* CORE_PLL_1 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E2, 0x18, 0x3C);     /* CORE_PLL_2 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E4, 0x80, 0x3FF);    /* CORE_PLL_4 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E7, 0x80, 0x3FF);    /* CORE_PLL_7 */

    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E0, 0x8000, 0x8000); /* CORE_PLL_0 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E0, 0x8000, 0x8000); /* CORE_PLL_0 */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF4E0, 0x8000, 0x0000); /* CORE_PLL_0 */

    /* Set Broadcast mode */
    x7121mWriteReg(0, 4, 0xF091, 0, 0xFFFF);                 /* MCM_BROADCAST */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF403, 0x7, 0x7);       /* MCM_BROADCAST */
#endif
    /* Verify the phy is ready to load FW */
    CHECK_STATUS(x7121mReadReg( 0, MZD_CHIP_REG, 0xf80f, &val, 0x6));
    if (val != 0x2)
    {
        osPrintf("Not ready to load FW\n");
        return GT_BAD_STATE;
    }

    /** DEVICE PRE INIT **/
    hwsOsExactDelayPtr              = func.osExactDelayPtr;
    hwsOsTimerWkFuncPtr             = func.osTimerWkPtr;
    hwsOsMemSetFuncPtr              = func.osMemSetPtr;
    hwsOsFreeFuncPtr                = func.osFreePtr;
    hwsOsMallocFuncPtr              = func.osMallocPtr;
    hwsSerdesRegSetFuncPtr          = func.serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr          = func.serdesRegGetAccess;
    hwsServerRegSetFuncPtr          = func.serverRegSetAccess;
    hwsServerRegGetFuncPtr          = func.serverRegGetAccess;
    hwsOsMemCopyFuncPtr             = func.osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr     = func.serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr     = func.serverRegFieldGetAccess;
    hwsRegisterSetFuncPtr           = func.registerSetAccess;
    hwsRegisterGetFuncPtr           = func.registerGetAccess;
    cpssOsPrintf("hwsIfPreInit success: devNum = %d\n", devNum);


    /** DEVICE INIT *   */
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_C112GX4;
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = X7121M_NUM_OF_SD;
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = x7121mSerdesDb;
    /* Init SERDES unit relevant for current device */
    {
        MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;

        CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
        CHECK_STATUS(mvHwsComphyIfInit(devNum,COM_PHY_C112GX4,hwsSerdesFuncsPtr, MV_HWS_COMPHY_FIRMWARE_NR,GT_FALSE));
    }

    for (serdesi = 0; serdesi < 16; serdesi += 4)
    {
        if (loadFwBmp & (1 << (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesi].serdesNum / 4)))
        {
            cpssOsPrintf("Loading FW SerDes Number = %2d...", hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesi].serdesNum);
            CHECK_STATUS(mvHwsComphyC112GX4SerdesFirmwareDownload(devNum, 0, &hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesi].serdesNum, 1, 0));
            cpssOsPrintf("Done\n");
        }
    }
#if 1
    /* Set Broadcast mode */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF0AB, 0, 0xFFFF);      /* finish writing */
    x7121mWriteReg(0, MZD_CHIP_REG, 0xF403, 0x7, 0x7);       /* MCM_BROADCAST */

    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF182 ,0x0 ,0xFFFF);/* CLAMPING_REG_3_1 = 0x0;              */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1A2 ,0x0 ,0xFFFF);/* CLAMPING_REG_3_2 = 0x0;              */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1C2 ,0x0 ,0xFFFF);/* CLAMPING_REG_3_3 = 0x0;              */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1E2 ,0x0 ,0xFFFF);/* CLAMPING_REG_3_4 = 0x0;              */

    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF19C ,0x200 ,0xFFFF);/* SD0_TX_IDLE_TIME_LINE = 0x200;       */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1BC ,0x200 ,0xFFFF);/* SD1_TX_IDLE_TIME_LINE = 0x200;       */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1DC ,0x200 ,0xFFFF);/* SD2_TX_IDLE_TIME_LINE = 0x200;       */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1FC ,0x200 ,0xFFFF);/* SD3_TX_IDLE_TIME_LINE = 0x200;       */

    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF184 ,0x001e ,0xFFFF);/* REG_RX_INIT_MIN_WAIT_S_3_1 = 0x001e; */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1A4 ,0x001e ,0xFFFF);/* REG_RX_INIT_MIN_WAIT_S_3_2 = 0x001e; */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1C4 ,0x001e ,0xFFFF);/* REG_RX_INIT_MIN_WAIT_S_3_3 = 0x001e; */
    x7121mWriteReg(0, MZD_LINE_SIDE, 0xF1E4 ,0x001e ,0xFFFF);/* REG_RX_INIT_MIN_WAIT_S_3_4 = 0x001e; */

    x7121mWriteReg(0, MZD_CHIP_REG, 0xF403, 0x0, 0x7);       /* MCM_BROADCAST */


#endif
    cpssOsPrintf("hwsIfInit success: devNum = %d\n", devNum);

    return GT_OK;
}

GT_STATUS dumpSerdesReg(GT_U32 serdesNum, GT_U32 startAddr, GT_U32 endAddr)
{
    GT_U32 serdesIterator, regAddrIterator;
    for (serdesIterator = 0; serdesIterator < X7121M_NUM_OF_SD; serdesIterator++)
    {
        if ((serdesNum == x7121mSerdesDb[serdesIterator].serdesNum) || (serdesNum == 0xffff))
        {
            /*cpssOsPrintf("serdesNum=%d internalSerdesNum=%d [mdioPort=%d]\n",
                         x7121mSerdesDb[serdesIterator].serdesNum,
                         x7121mSerdesDb[serdesIterator].internalSerdesNum,
                         x7121mSerdesDb[serdesIterator].serdesNum/4);*/
            for (regAddrIterator = startAddr; regAddrIterator <= endAddr ; regAddrIterator+=4)
            {
                GT_UREG_DATA mask=0xFFFFFFFF;
                /*GT_UREG_DATA data;
                x7121mSerdesReadReg(0,0,1,x7121mSerdesDb[serdesIterator].serdesNum,regAddrIterator,&data,mask);
                osPrintf("%04x, %08x\n",regAddrIterator, data);*/
                x7121mSerdesReadRegWrapper(0,1,x7121mSerdesDb[serdesIterator].serdesNum,regAddrIterator,mask);
            }
        }
    }
    return GT_OK;
}

GT_STATUS powerup
(
    GT_BOOL                         powerUp,
    MV_HWS_SERDES_SPEED             serdesSpeed,
    MV_HWS_SERDES_MEDIA             media,
    MV_HWS_SERDES_BUS_WIDTH_ENT     busWidth,
    GT_U32                          numOfLanes,
    GT_U32                          lane0,
    GT_U32                          lane1,
    GT_U32                          lane2,
    GT_U32                          lane3,
    GT_U32                          lane4,
    GT_U32                          lane5,
    GT_U32                          lane6,
    GT_U32                          lane7
)
{
    GT_U8                       devNum = 0;
    GT_U32                      portGroup = 0;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;
    GT_U32                      curLanesList[10];

    curLanesList[0] = lane0;
    curLanesList[1] = lane1;
    curLanesList[2] = lane2;
    curLanesList[3] = lane3;
    curLanesList[4] = lane4;
    curLanesList[5] = lane5;
    curLanesList[6] = lane6;
    curLanesList[7] = lane7;

    serdesConfig.baudRate       = serdesSpeed;
    serdesConfig.media          = media;
    serdesConfig.busWidth       = busWidth;
    serdesConfig.refClock       = MHz_156;
    serdesConfig.refClockSource = PRIMARY_LINE_SRC;
    serdesConfig.serdesType     = COM_PHY_C112GX4;
    serdesConfig.opticalMode    = GT_FALSE;

    switch (serdesConfig.baudRate)
    {
        case _26_5625G_PAM4:      /* for NRZ mode at 26.5625G,  for PAM4 mode at 53.125G */
        case _28_125G_PAM4:     /* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
            serdesConfig.encoding = SERDES_ENCODING_PAM4;
            break;
        default:
            serdesConfig.encoding = SERDES_ENCODING_NA;
            break;
    }
#if 0
    for (i=0 ; i<numOfLanes ; i++)
    {
        /*clear_anReg_txReg*/
        x7121mWriteReg(curLanesList[i]/4, MZD_AUTONEG, 0x8000 + 0x200 * (curLanesList[i]%4), 0x4, 0xFFFF);
        x7121mWriteReg(curLanesList[i]/4, MZD_LINE_SIDE, 0xF190 + 0x20 * (curLanesList[i]%4), 0x4, 0xFFFF);

        x7121mWriteReg(0, MZD_LINE_SIDE, 0xF113, 0x0, ~0xFC3F);  /* Init PCS */
    }
#endif
    /* power up the serdes */
     CHECK_STATUS(mvHwsSerdesArrayPowerCtrl(devNum, portGroup, numOfLanes,
                                          curLanesList, powerUp, &serdesConfig));

    return GT_OK;
}
