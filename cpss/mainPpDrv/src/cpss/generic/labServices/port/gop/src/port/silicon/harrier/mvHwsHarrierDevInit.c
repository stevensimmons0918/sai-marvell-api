/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsHarrierDevInit.c
*
* DESCRIPTION:
*     Harrier specific HW Services init
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

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

extern  GT_BOOL hwsPpHwTraceFlag;

extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiExtBrRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac100BrRegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiMac400RegDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs400UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs200UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs100UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMifRegistersDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiPcs50UnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkMtiRsFecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC harrierAnpUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkLpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC anUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC hawkRsfecStatisticsUnitsDb[];

GT_STATUS mvHwsHarrierRegisterMisconfigurationSet
(
    IN GT_U8    devNum
);

/************************* definition *****************************************************/
const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyN5XC56GP5X4PinToRegMapSdw[] =
{
    /* N5XC56GP5X4_PIN_RESET           = 0  */ {SDW_GENERAL_CONTROL_0           ,2                     ,1},
    /* N5XC56GP5X4_PIN_ISOLATION       = 1  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* N5XC56GP5X4_PIN_BG_RDY          = 2  */ {MV_HWS_SERDES_REG_NA            ,0                     ,1},
    /* N5XC56GP5X4_PIN_SIF_SEL         = 3  */ {SDW_GENERAL_CONTROL_1           ,0                     ,1},
    /* N5XC56GP5X4_PIN_MCU_CLK         = 4  */ {SDW_GENERAL_CONTROL_2           ,16                   ,16},
    /* N5XC56GP5X4_PIN_DIRECTACCES     = 5  */ {SDW_GENERAL_CONTROL_1           ,15                    ,1},
    /* N5XC56GP5X4_PIN_PHY_MODE        = 6  */ {MV_HWS_SERDES_TIED_PIN          ,4/*<-default value*/  ,0},
    /* N5XC56GP5X4_PIN_REFCLK_TX0      = 7  */ {SDW_LANE_CONTROL_0              ,31                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_TX1      = 8  */ {SDW_LANE_CONTROL_0              ,31                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_TX2      = 9  */ {SDW_LANE_CONTROL_0              ,31                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_TX3      = 10 */ {SDW_LANE_CONTROL_0              ,31                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_RX0      = 11 */ {SDW_LANE_CONTROL_0              ,30                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_RX1      = 12 */ {SDW_LANE_CONTROL_0              ,30                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_RX2      = 13 */ {SDW_LANE_CONTROL_0              ,30                    ,1},
    /* N5XC56GP5X4_PIN_REFCLK_RX3      = 14 */ {SDW_LANE_CONTROL_0              ,30                    ,1},
    /* N5XC56GP5X4_PIN_REFFREF_TX0     = 15 */ {SDW_LANE_CONTROL_0              ,17                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_TX1     = 16 */ {SDW_LANE_CONTROL_0              ,17                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_TX2     = 17 */ {SDW_LANE_CONTROL_0              ,17                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_TX3     = 18 */ {SDW_LANE_CONTROL_0              ,17                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_RX0     = 19 */ {SDW_LANE_CONTROL_0              ,22                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_RX1     = 20 */ {SDW_LANE_CONTROL_0              ,22                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_RX2     = 21 */ {SDW_LANE_CONTROL_0              ,22                    ,5},
    /* N5XC56GP5X4_PIN_REFFREF_RX3     = 22 */ {SDW_LANE_CONTROL_0              ,22                    ,5},
    /* N5XC56GP5X4_PIN_PHY_GEN_TX0     = 23 */ {SDW_LANE_CONTROL_0              ,9                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_TX1     = 24 */ {SDW_LANE_CONTROL_0              ,9                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_TX2     = 25 */ {SDW_LANE_CONTROL_0              ,9                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_TX3     = 26 */ {SDW_LANE_CONTROL_0              ,9                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_RX0     = 27 */ {SDW_LANE_CONTROL_0              ,3                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_RX1     = 28 */ {SDW_LANE_CONTROL_0              ,3                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_RX2     = 29 */ {SDW_LANE_CONTROL_0              ,3                     ,6},
    /* N5XC56GP5X4_PIN_PHY_GEN_RX3     = 30 */ {SDW_LANE_CONTROL_0              ,3                     ,6},
    /* N5XC56GP5X4_PIN_PU_PLL0         = 31 */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* N5XC56GP5X4_PIN_PU_PLL1         = 32 */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* N5XC56GP5X4_PIN_PU_PLL2         = 33 */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* N5XC56GP5X4_PIN_PU_PLL3         = 34 */ {SDW_LANE_CONTROL_0              ,0                     ,1},
    /* N5XC56GP5X4_PIN_PU_RX0          = 35 */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* N5XC56GP5X4_PIN_PU_RX1          = 36 */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* N5XC56GP5X4_PIN_PU_RX2          = 37 */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* N5XC56GP5X4_PIN_PU_RX3          = 38 */ {SDW_LANE_CONTROL_0              ,1                     ,1},
    /* N5XC56GP5X4_PIN_PU_TX0          = 39 */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* N5XC56GP5X4_PIN_PU_TX1          = 40 */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* N5XC56GP5X4_PIN_PU_TX2          = 41 */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* N5XC56GP5X4_PIN_PU_TX3          = 42 */ {SDW_LANE_CONTROL_0              ,2                     ,1},
    /* N5XC56GP5X4_PIN_TX_IDLE0        = 43 */ {SDW_LANE_CONTROL_0              ,15                    ,1},
    /* N5XC56GP5X4_PIN_TX_IDLE1        = 44 */ {SDW_LANE_CONTROL_0              ,15                    ,1},
    /* N5XC56GP5X4_PIN_TX_IDLE2        = 45 */ {SDW_LANE_CONTROL_0              ,15                    ,1},
    /* N5XC56GP5X4_PIN_TX_IDLE3        = 46 */ {SDW_LANE_CONTROL_0              ,15                    ,1},
    /* N5XC56GP5X4_PIN_PU_IVREF        = 47 */ {SDW_GENERAL_CONTROL_0           ,0                     ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINEN0     = 48 */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINEN1     = 49 */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINEN2     = 50 */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINEN3     = 51 */ {SDW_LANE_CONTROL_3              ,0                     ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINCO0     = 52 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),4/*SDW_LANE_STATUS_1               ,0   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINCO1     = 53 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),4/*SDW_LANE_STATUS_1               ,0   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINCO2     = 54 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),4/*SDW_LANE_STATUS_1               ,0   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINCO3     = 55 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),4/*SDW_LANE_STATUS_1               ,0   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINFA0     = 56 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),3/*SDW_LANE_STATUS_1               ,1   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINFA1     = 57 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),3/*SDW_LANE_STATUS_1               ,1   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINFA2     = 58 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),3/*SDW_LANE_STATUS_1               ,1   */                  ,1},
    /* N5XC56GP5X4_PIN_RX_TRAINFA3     = 59 */ {(SDW_INTERNAL_REG_OFFSET+0x5020),3/*SDW_LANE_STATUS_1               ,1   */                  ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINEN0     = 60 */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINEN1     = 61 */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINEN2     = 62 */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINEN3     = 63 */ {SDW_LANE_CONTROL_3              ,1                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINCO0     = 64 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),6 */SDW_LANE_STATUS_1               ,2                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINCO1     = 65 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),6 */SDW_LANE_STATUS_1               ,2                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINCO2     = 66 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),6 */SDW_LANE_STATUS_1               ,2                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINCO3     = 67 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),6 */SDW_LANE_STATUS_1               ,2                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINFA0     = 68 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),5 */SDW_LANE_STATUS_1               ,3                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINFA1     = 69 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),5 */SDW_LANE_STATUS_1               ,3                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINFA2     = 70 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),5 */SDW_LANE_STATUS_1               ,3                     ,1},
    /* N5XC56GP5X4_PIN_TX_TRAINFA3     = 71 */ {/*(SDW_INTERNAL_REG_OFFSET+0x5020),5 */SDW_LANE_STATUS_1               ,3                     ,1},
    /* N5XC56GP5X4_PIN_SQ_DET_LPF0     = 72 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3270),14*/SDW_LANE_STATUS_0               ,6                     ,1},
    /* N5XC56GP5X4_PIN_SQ_DET_LPF1     = 73 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3270),14*/SDW_LANE_STATUS_0               ,6                     ,1},
    /* N5XC56GP5X4_PIN_SQ_DET_LPF2     = 74 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3270),14*/SDW_LANE_STATUS_0               ,6                     ,1},
    /* N5XC56GP5X4_PIN_SQ_DET_LPF3     = 75 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3270),14*/SDW_LANE_STATUS_0               ,6                     ,1},
    /* N5XC56GP5X4_PIN_RX_INIT0        = 76 */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* N5XC56GP5X4_PIN_RX_INIT1        = 77 */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* N5XC56GP5X4_PIN_RX_INIT2        = 78 */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* N5XC56GP5X4_PIN_RX_INIT3        = 79 */ {SDW_LANE_CONTROL_0              ,16                    ,1},
    /* N5XC56GP5X4_PIN_RX_INITDON0     = 80 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3200),19*/SDW_LANE_STATUS_0               ,4                    ,1},
    /* N5XC56GP5X4_PIN_RX_INITDON1     = 81 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3200),19*/SDW_LANE_STATUS_0               ,4                    ,1},
    /* N5XC56GP5X4_PIN_RX_INITDON2     = 82 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3200),19*/SDW_LANE_STATUS_0               ,4                    ,1},
    /* N5XC56GP5X4_PIN_RX_INITDON3     = 83 */ {/*(SDW_INTERNAL_REG_OFFSET+0x3200),19*/SDW_LANE_STATUS_0               ,4                    ,1},
    /* N5XC56GP5X4_PIN_AVDD_SEL        = 84 */ {SDW_GENERAL_CONTROL_0           ,24                    ,3},
    /* N5XC56GP5X4_PIN_SPD_CFG         = 85 */ {SDW_GENERAL_CONTROL_0           ,20                    ,4},
    /* N5XC56GP5X4_PIN_PIPE_SEL        = 86 */ {MV_HWS_SERDES_TIED_PIN          ,0                     ,0},
    /* N5XC56GP5X4_PIN_TX_GRAY_CODE_EN0= 87 */ {SDW_LANE_CONTROL_3              ,24                    ,1},
    /* N5XC56GP5X4_PIN_TX_GRAY_CODE_EN1= 88 */ {SDW_LANE_CONTROL_3              ,24                    ,1},
    /* N5XC56GP5X4_PIN_TX_GRAY_CODE_EN2= 89 */ {SDW_LANE_CONTROL_3              ,24                    ,1},
    /* N5XC56GP5X4_PIN_TX_GRAY_CODE_EN3= 90 */ {SDW_LANE_CONTROL_3              ,24                    ,1},
    /* N5XC56GP5X4_PIN_RX_GRAY_CODE_EN0= 91 */ {SDW_LANE_CONTROL_3              ,25                    ,1},
    /* N5XC56GP5X4_PIN_RX_GRAY_CODE_EN1= 92 */ {SDW_LANE_CONTROL_3              ,25                    ,1},
    /* N5XC56GP5X4_PIN_RX_GRAY_CODE_EN2= 93 */ {SDW_LANE_CONTROL_3              ,25                    ,1},
    /* N5XC56GP5X4_PIN_RX_GRAY_CODE_EN3= 94 */ {SDW_LANE_CONTROL_3              ,25                    ,1},
    /* N5XC56GP5X4_PIN_TX_PRE_CODE_EN0 = 95 */ {SDW_LANE_CONTROL_3              ,26                    ,1},
    /* N5XC56GP5X4_PIN_TX_PRE_CODE_EN1 = 96 */ {SDW_LANE_CONTROL_3              ,26                    ,1},
    /* N5XC56GP5X4_PIN_TX_PRE_CODE_EN2 = 97 */ {SDW_LANE_CONTROL_3              ,26                    ,1},
    /* N5XC56GP5X4_PIN_TX_PRE_CODE_EN3 = 98 */ {SDW_LANE_CONTROL_3              ,26                    ,1},
    /* N5XC56GP5X4_PIN_RX_PRE_CODE_EN0 = 99 */ {SDW_LANE_CONTROL_3              ,27                    ,1},
    /* N5XC56GP5X4_PIN_RX_PRE_CODE_EN1 = 100*/ {SDW_LANE_CONTROL_3              ,27                    ,1},
    /* N5XC56GP5X4_PIN_RX_PRE_CODE_EN2 = 101*/ {SDW_LANE_CONTROL_3              ,27                    ,1},
    /* N5XC56GP5X4_PIN_RX_PRE_CODE_EN3 = 102*/ {SDW_LANE_CONTROL_3              ,27                    ,1}
};
#define NA      NA_16BIT
#define NA_S    NA_16BIT_SIGNED
/** COMPHY_12nm_Serdes_Init_rev0.13 */
const MV_HWS_SERDES_TXRX_TUNE_PARAMS harrierTxRxTuneParams[] =
#ifndef WIN32
{/**                                                                                      __
                                                                                         |E |            __
                                                                                 __      |N |           |R |
                                                                           __   |R |__ __|  |      __   |L |_____
                                                                          |C |__|L |R |C |M |__ _ |C |__|2 |R |C |_ __ __ __
                                                                          |U |R |1 |E |A |I |C |R |U |R |  |E |A |S |S |S |S |__
                                                                          |R |L |  |S |P |D |S |S |R |L |T |S |P |E |E |E |E |S |
                                            __    __ ___                  |1 |1 |E |1 |1 |  |1 |1 |2 |2 |U |2 |2 |L |L |L |L |Q |
                                           |P |__|M |P  |__               |  |  |X |  |  |F |  |  |  |  |N |  |  |M |M |M |M |E |
                                           |R |P |A |O  |U |              |S |S |T |S |S |R |M |M |S |S |E |S |S |U |U |U |U |L |
                                           |E |R |I |S  |S |              |E |E |R |E |E |E |I |I |E |E |  |E |E |F |F |P |P |C |
                                           |2 |E |N |T  |R |              |L |L |A |L |L |Q |D |D |L  L |G |L |L |I |F |I |F |H |*/
    {_1_25G,                {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_1_25G_SR_LR,          {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_2_578125,             {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_3_125G,               {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_5G,                   {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_5_15625G,             {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_10G,                  {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_10_3125G,             {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_10_3125G_SR_LR,       {.txComphyC56G={0, 1, 52,10, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_25_78125G,            {.txComphyC56G={0, 0, 63, 0, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_25_78125G_SR_LR,      {.txComphyC56G={0, 4, 56, 3, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_26_5625G,             {.txComphyC56G={0, 1, 58, 4, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_26_5625G_SR_LR,       {.txComphyC56G={0, 4, 56, 3, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_26_5625G_PAM4,        {.txComphyC56G={0, 1, 58, 4, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
    {_26_5625G_PAM4_SR_LR,  {.txComphyC56G={0, 5, 56, 2, 0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},

    {LAST_MV_HWS_SERDES_SPEED,{.txComphyC56G={0,0,0,0,0}},{.rxComphyC56G={NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA_S}}},
};
#else
{
    {0}
};
#endif

HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes0Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes1Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes2Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes3Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes4Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes5Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes6Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes7Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes8Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes9Data  = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes10Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes11Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes12Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes13Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes14Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes15Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes16Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes17Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes18Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA harrierSerdes19Data = {mvHwsComphyN5XC56GP5X4PinToRegMapSdw, NULL, harrierTxRxTuneParams, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC harrierSerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes0Data }},
    {NULL, 1,  1, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes1Data }},
    {NULL, 2,  2, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes2Data }},
    {NULL, 3,  3, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes3Data }},
    {NULL, 4,  0, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes4Data }},
    {NULL, 5,  1, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes5Data }},
    {NULL, 6,  2, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes6Data }},
    {NULL, 7,  3, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes7Data }},
    {NULL, 8,  0, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes8Data }},
    {NULL, 9,  1, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes9Data }},
    {NULL, 10, 2, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes10Data}},
    {NULL, 11, 3, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes11Data}},
    {NULL, 12, 0, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes12Data}},
    {NULL, 13, 1, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes13Data}},
    {NULL, 14, 2, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes14Data}},
    {NULL, 15, 3, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes15Data}},
    {NULL, 16, 0, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes16Data}},
    {NULL, 17, 1, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes17Data}},
    {NULL, 18, 2, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes18Data}},
    {NULL, 19, 3, COM_PHY_N5XC56GP5X4,   {(MV_HWS_COMPHY_SERDES_DATA*)&harrierSerdes19Data}},
};

/************************* Globals *******************************************************/

extern const MV_HWS_PORT_INIT_PARAMS **hwsPortsHarrierParamsSupModesMap;
extern GT_VOID hwsHarrierPortElementsDbInit(GT_VOID);


#ifdef MV_HWS_REDUCED_BUILD
 /* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL harrierDbInitDone = GT_FALSE;
#define PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    _var
#else
#define PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.harrierDevInitSrc._var)
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
    CHECK_STATUS(mvHwsMtiLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));

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
    CHECK_STATUS(mvHwsMti400MacRev2IfInit(hwsMacFuncsPtr));


    return GT_OK;
}

/*******************************************************************************
* hwsHarrierSerdesIfInit
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
static GT_STATUS hwsHarrierSerdesIfInit(GT_U8 devNum , MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock,GT_BOOL callbackInitOnly)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;

    if(GT_FALSE==callbackInitOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {

            CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, GT_TRUE));

            /* Enable SWD rx_clk_4x_enable, tx_clk_4x_enable , reset rf_reset phase fifo */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                SDW_LANE_CONTROL_1,
                                                0xC000010,
                                                0xC000010));
        }
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));
    if (serdesRefClock == MHz_25)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_N5XC56GP5X4, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else if (serdesRefClock == MHz_156)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_N5XC56GP5X4, hwsSerdesFuncsPtr,
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
                                                    0x5534,
                                                    0x1,
                                                    0x1));
                /* Enable SD(internal memory) RX: RXDCLK_4X_EN_LANE */
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0x5630,
                                                    0x40,
                                                    0x40));
            }
        }
   }

    return GT_OK;
}

/**
* @internal hwsHarrierRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsHarrierRegDbInit
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
    rc += mvHwsRegDbInit(devNum, ANP_400_UNIT, harrierAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_LOW_SP_PCS_UNIT, hawkLpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_400_UNIT, anUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);

    return rc;
}

/**
* @internal hwsHarrierIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsHarrierIfPreInit
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

    hwsDeviceSpecInfo[devNum].devType = Harrier;
    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    rc =hwsHarrierRegDbInit(devNum);

    return rc;
}

GT_STATUS hwsHarrierSerdesGetClockAndInit
(
    GT_U8 devNum,
    GT_BOOL callbackInitOnly
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc != NULL)
    {
        MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock;
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc(devNum, &serdesRefClock));
        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsHarrierSerdesIfInit(devNum,serdesRefClock,callbackInitOnly));
    }

     return GT_OK;
}




#ifdef SHARED_MEMORY

GT_STATUS hwsHarrierIfReIoad
(
    GT_U8 devNum
)
{
    CHECK_STATUS(macIfInit(devNum));

    CHECK_STATUS(hwsHarrierSerdesGetClockAndInit(devNum,GT_TRUE));
    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    return GT_OK;
}

#endif

GT_VOID hwsHarrierPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    hwsHarrierPortElementsDbInit();
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsHarrierParamsSupModesMap;
}



GT_VOID hwsHarrierIfInitHwsDevFunc
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
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsHawkPortReset ;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortGeneralAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = hwsHarrierPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsHarrierPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsExtMacClockEnableGet;
}

/*******************************************************************************
* hwsHarrierIfInit
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
GT_STATUS hwsHarrierIfInit
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

    if (PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        return GT_ALREADY_EXIST;
    }

    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));

    if (funcPtr == NULL)
    {
        skipWritingHW = GT_TRUE;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(harrierDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsHarrierIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Harrier;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_N5XC56GP5X4;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr = NULL;
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(harrierSerdesDb) / sizeof(harrierSerdesDb[0]);
#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
    for (sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {
        CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, sdIndex, &harrierSerdesDb[sdIndex]));
    }
#else
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = harrierSerdesDb;
#endif

    hwsHarrierPortsParamsSupModesMapSet(devNum);

    hwsDeviceSpecInfo[devNum].portsNum = 40;/*actually 20 ports on 'range of 40 ports'*/

    devType = hwsDeviceSpecInfo[devNum].devType;
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;

    hwsHarrierIfInitHwsDevFunc(devNum,funcPtr);

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Harrier ifInit start ******\n");
    }
#endif

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
#ifndef MV_HWS_REDUCED_BUILD
    if(PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] == GT_FALSE)
    {
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
    }

#endif
    if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E) && (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) ||
          (skipWritingHW == GT_TRUE))
    {
        /*no HW write */
        CHECK_STATUS(hwsHarrierSerdesGetClockAndInit(devNum,GT_TRUE));
    }
    else
    {
        CHECK_STATUS(hwsHarrierSerdesGetClockAndInit(devNum,GT_FALSE));
    }
    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /*in case of HS and HA port related regisiters shouldn't be reset */
    if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
          (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
          (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
    {
        /* Init all MIF units relevant for current device */
        CHECK_STATUS(mvHwsMifInit(devNum));
    }

    /* Init all ANP units relevant for current device */
    CHECK_STATUS(mvHwsAnpInit(devNum,skipWritingHW /*False*/));

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_TRUE;

    /************************************************************/
    /* Power reduction feature - Raven MTI EXT units reset      */
    /************************************************************/
    if (skipWritingHW == GT_FALSE)
    {
        if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
              (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
              (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
        {
            CHECK_STATUS(mvHwsHarrierRegisterMisconfigurationSet(devNum));
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Harrier ifInit end ********\n");
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
    CPSS_TBD_BOOKMARK_HARRIER

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
    CPSS_TBD_BOOKMARK_HARRIER

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsHarrierIfClose
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
void hwsHarrierIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_FALSE;
    }
    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        PRV_SHARED_LAB_SERVICES_DIR_HARRIER_DEV_INIT_SRC_GLOBAL_VAR(harrierDbInitDone) = GT_FALSE;
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

GT_STATUS mvHwsHarrierRegisterMisconfigurationSet
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
        hwsOsPrintf("****** Start of mvHwsHarrierRegisterMisconfigurationSet ******\n");
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
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsHarrierRegisterMisconfigurationSet ******\n");
    }
#endif

    return GT_OK;
}

#endif


