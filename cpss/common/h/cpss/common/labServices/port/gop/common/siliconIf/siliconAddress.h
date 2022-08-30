/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
* siliconAddress.h
*
* DESCRIPTION:
*       Different Silicon registers addresses
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*
*******************************************************************************/

#ifndef __siliconAddress_h
#define __siliconAddress_h
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtipUsxExtUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtiMac400RegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtiMac100BrRegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcs50Units.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcs100Units.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcs200Units.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcs400Units.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsRsfecUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsLpcsUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsUsxmUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsUsxRsfecUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMifRegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsUsxPcsUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMacCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcsCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsCpuRsfecUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtipCpuExtUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsCpuSgpcsUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsRsfecStatisticsUnit.h>

#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsAc5pMtiExtRegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsAc5pMtiExtBrRegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsAc5pAnpUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsAnUnits.h>

#include <cpss/common/labServices/port/gop/port/silicon/harrier/regDb/mvHwsHarrierAnpUnits.h>

#include <cpss/common/labServices/port/gop/port/silicon/ac5x/regDb/mvHwsAc5xMtiExtRegDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5x/regDb/mvHwsAc5xPcs25Units.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5x/regDb/mvHwsAc5xAnpUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5x/regDb/mvHwsAc5xRsfecUnits.h>
/*IronmanL section*/
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanAnpUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GMacUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GMacBRUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtipMacGlobalExtUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtipMacPortExtUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtipPcsExtUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GPcsLpcsGeneralUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GPcsLpcsUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GPcsBaseRPcsUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ironman/regDb/mvHwsIronmanMtip10GPcsFcfecUnits.h>


/********* Addresses for Avago Serdes configuration ********/
#define AVAGO_SD_METAL_FIX                  (0x8)
#define AVAGO_EXTERNAL_STATUS               (0xC)
#define AVAGO_CORE_STATUS                   (0x14)
#define AVAGO_INTERRUPT_CAUSE_REG0          (0x28)

/********* server registers ********/
#define SERVER_PLL0_CONFIG_REG          0xF8204
#define SERVER_PLL_CORE_PARAM_REG       0xF82E0
#define SERVER_HD_EFUSE_SLAVE_REG       0xF9000
#define SERVER_HD_EFUSE_WORD_2          0xF9004
#define SERVER_HD_EFUSE_WORD_3          0xF9008
#define SERVER_SECONDARY_EFUSE          0xF8008

#define  DEVICE_GENERAL_CONTROL_1       0xF8254
#define  DEVICE_GENERAL_CONTROL_2       0xF8258
#define  DEVICE_GENERAL_CONTROL_11      0xF827C
#define  DEVICE_GENERAL_CONTROL_12      0xF8280
#define  DEVICE_GENERAL_CONTROL_13      0xF8284
#define  DEVICE_GENERAL_CONTROL_16      0xF8290
#define  DEVICE_GENERAL_CONTROL_17      0xF8294
#define  DEVICE_GENERAL_CONTROL_18      0xF8298
#define  DEVICE_GENERAL_CONTROL_19      0xF829C
#define  DEVICE_GENERAL_CONTROL_20      0xF82A0
#define  DEVICE_GENERAL_CONTROL_21      0xF82A4
#define  DEVICE_GENERAL_CONTROL_22      0xF82A8
#define  DEVICE_GENERAL_CONTROL_27      0xF8D0C
#define  DEVICE_GENERAL_STATUS_1        0xF8C84
#define  DEVICE_GENERAL_STATUS_3        0xF8C8C
#define  DEVICE_GENERAL_STATUS_4        0xF8C90

#define DFX_SERVER_AVS_CONTROL          0xF8120
#define DFX_SERVER_AVS_DISABLE_CONTROL1 0xF8128
#define DFX_SERVER_AVS_ENABLE_CONTROL   0xF8130
#define DFX_SERVER_AVS_STATUS           0xF8134
#define DFX_SERVER_DRO_READ_CONTROL     0xF8498
#define DFX_SERVER_DRO_READ_STATUS      0xF849C
#define DFX_SERVER_EXTERNAL_TEMPERATUE_SENSOR_12NM_STATUS    0xF80DC
#define DFX_SERVER_EXTERNAL_TEMPERATUE_SENSOR_12NM_CONTROL_0 0XF80D0
#define DFX_SERVER_AVS_5NM_ENABLED_CONTROL   0xF8E2C    /*Aldrim3M/Harrier*/


#define DFX_SERVER_STATUS_REG_ADDR_CNS  0xF8010

#define  HWS_2_LSB_MASK_CNS             0xFFFFFFFC
/**** falcon address ****/
#define RAVEN_DFX_BASE_ADDR             0x700000

typedef enum lion2Regs
{
    MG_Media_Interface_Reg = 0x38C,

/********* Addresses for UNIT port ********/
    GIG_PORT_MAC_CONTROL_REGISTER0 = 0,
    GIG_PORT_MAC_CONTROL_REGISTER1 = 4,
    GIG_PORT_MAC_CONTROL_REGISTER2 = 8,
    GIG_Port_Auto_Negotiation_Configuration = 0xC,
    GIG_Port_Status_Reg = 0x10,
    GIG_Port_PRBS_Status_Reg = 0x38,
    GIG_Port_PRBS_Error_Cntr_Reg = 0x3C,
    GIG_PORT_MAC_CONTROL_REGISTER4 = 0x90,
    GIG_PORT_MAC_MACRO_CONTROL = 0xE0,
    GIG_PORT_pulse_1ms_Low_Reg = 0xd4,
    GIG_PORT_pulse_1ms_High_Reg = 0xd8,

/********* Addresses for TABLE MSM_Regs.cfg ********/

    MSM_PORT_MAC_CONTROL_REGISTER0 = 0,
    MSM_PORT_MAC_CONTROL_REGISTER1 = 0x4,
    MSM_PORT_MAC_CONTROL_REGISTER2 = 0x8,
    MSM_PORT_STATUS = 0xc,
    MSM_PORT_FIFOS_THRESHOLDS_CONFIGURATION = 0x10,
    MSM_PORT_INTERRUPT_CAUSE = 0x14,
    MSM_PORT_INTERRUPT_MASK = 0x18,
    MSM_PORT_MAC_CONTROL_REGISTER3 = 0x1c,
    MSM_PORT_PER_PRIO_FC_STATUS = 0x20,
    DEBUG_BUS_STATUS = 0x24,
    MSM_PORT_METAL_FIX = 0x2c,
    MSM_MIB_COUNTERS_CONTROL_REGISTER = 0x30,
    MSM_CCFC_FC_TIMERP = 0x38,
    EXTERNAL_UNITS_INTERRUPTS_CAUSE = 0x58,
    EXTERNAL_UNITS_INTERRUPTS_MASK = 0x5c,
    MSM_PPFC_CONTROL = 0x60,
    MSM_LLFC_DSA_TAG_REGISTER_0 = 0x68,
    MSM_LLFC_DSA_TAG_REGISTER_1 = 0x6c,
    MSM_LLFC_DSA_TAG_REGISTER_2 = 0x70,
    MSM_LLFC_DSA_TAG_REGISTER_3 = 0x74,
    MSM_LLFC_FLOW_CONTROL_WINDOW_REG_0 = 0x78,
    MSM_LLFC_FLOW_CONTROL_WINDOW_REG_1 = 0x7c,
    MSM_DIC_BUDGET_COMPENSATION = 0x80,
    MSM_PORT_MAC_CONTROL_REGISTER4 = 0x84,
    MSM_PORT_MAC_CONTROL_REGISTER5 = 0x88,
    EXTERNAL_CONTROL = 0x90,
    MACRO_CONTROL = 0x94,

    XPCS_Global_Configuration_Reg0 = 0,
    XPCS_Global_Configuration_Reg1 = 0x4,
    XPCS_Global_Fifo_Threshold_Configuration = 0x8,
    XPCS_Global_Status_Reg = 0x10,
    XPCS_Global_Deskew_Error_Counter = 0x20,

    XPCS_Tx_Packets_Cntr_LSB = 0x30,
    XPCS_Tx_Packets_Cntr_MSB = 0x34,
    XPCS_Internal_Metal_Fix = 0x40,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane0 = 0x7C,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane0 = 0x80,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane1 = 0xC0,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane1 = 0xC4,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane2 = 0x104,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane2 = 0x108,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane3 = 0x148,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane3 = 0x14C,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane4 = 0x18C,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane4 = 0x190,
    XPCS_Rx_Bad_Packets_Cntr_LSB_Lane5 = 0x1D0,
    XPCS_Rx_Bad_Packets_Cntr_MSB_Lane5 = 0x1D4,
    XPCS_Lane0_configuration_Reg = 0x50,
    XPCS_Lane1_configuration_Reg = 0x94,
    XPCS_Lane2_configuration_Reg = 0xD8,
    XPCS_Lane3_configuration_Reg = 0x11C,
    XPCS_Lane4_configuration_Reg = 0x160,
    XPCS_Lane5_configuration_Reg = 0x1A4,

    XPCS_PRBS_Error_Cntr_Lane0 = 0x70,
    XPCS_PRBS_Error_Cntr_Lane1 = 0xB4,
    XPCS_PRBS_Error_Cntr_Lane2 = 0xF8,
    XPCS_PRBS_Error_Cntr_Lane3 = 0x13C,
    XPCS_PRBS_Error_Cntr_Lane4 = 0x180,
    XPCS_PRBS_Error_Cntr_Lane5 = 0x1C4,
    XPCS_Lane_Status_Lane0 = 0x5C,
    XPCS_Lane_Status_Lane1 = 0xA0,
    XPCS_Lane_Status_Lane2 = 0xE4,
    XPCS_Lane_Status_Lane3 = 0x128,
    XPCS_Lane_Status_Lane4 = 0x16C,
    XPCS_Lane_Status_Lane5 = 0x1B0,


/********* Addresses for TABLE MMPCS_Regs.cfg ********/
    PCS40G_COMMON_INTERRUPT_CAUSE = 0x8,
    PCS40G_COMMON_INTERRUPT_MASK = 0xc,
    PCS40G_ALIGNMENT_CYCLE_CONTROL = 0x10,
    PCS40G_COMMON_CONTROL = 0x14,
    PCS40G_COMMON_CONTROL_2 = 0x18,
    TX_IDLE_REMOVAL_THRESHOLDS = 0x20,
    TX_IDLE_INSERTION_THRESHOLDS = 0x24,
    RX_IDLE_REMOVAL_THRESHOLDS = 0x28,
    RX_IDLE_INSERTION_THRESHOLDS = 0x2c,
    PCS40G_COMMON_STATUS = 0x30,
    RX_PPM_FIFO_COUNTERS = 0x34,
    TX_PPM_FIFO_COUNTERS = 0x38,
    TEST_PATTERN_CONFIGURATION = 0x3c,
    TEST_PATTERN_ERROR_COUNT = 0x40,
    CHANNEL0_BER_COUNTER = 0x44,
    CHANNEL1_BER_COUNTER = 0x48,
    CHANNEL2_BER_COUNTER = 0x4c,
    CHANNEL3_BER_COUNTER = 0x50,
    CHANNEL0_BIP_COUNTER = 0x54,
    CHANNEL1_BIP_COUNTER = 0x58,
    CHANNEL2_BIP_COUNTER = 0x5c,
    CHANNEL3_BIP_COUNTER = 0x60,
    CHANNEL0_BIP_NOT_COUNTER = 0x64,
    CHANNEL1_BIP_NOT_COUNTER = 0x68,
    CHANNEL2_BIP_NOT_COUNTER = 0x6c,
    CHANNEL3_BIP_NOT_COUNTER = 0x70,
    CHANNEL_CONFIGURATION = 0x74,
    CHANNEL0_RX_GB_FIFO_CONFIGURATION = 0x78,
    CHANNEL1_RX_GB_FIFO_CONFIGURATION = 0x7c,
    CHANNEL2_RX_GB_FIFO_CONFIGURATION = 0x80,
    CHANNEL3_RX_GB_FIFO_CONFIGURATION = 0x84,
    LANE0_ALIGNMENT_MARKER_0 = 0x88,
    LANE0_ALIGNMENT_MARKER_1 = 0x8c,
    LANE0_ALIGNMENT_MARKER_2 = 0x90,
    LANE0_ALIGNMENT_MARKER_3 = 0x94,
    LANE1_ALIGNMENT_MARKER_0 = 0x98,
    LANE1_ALIGNMENT_MARKER_1 = 0x9c,
    LANE1_ALIGNMENT_MARKER_2 = 0xa0,
    LANE1_ALIGNMENT_MARKER_3 = 0xa4,
    LANE2_ALIGNMENT_MARKER_0 = 0xa8,
    LANE2_ALIGNMENT_MARKER_1 = 0xac,
    LANE2_ALIGNMENT_MARKER_2 = 0xb0,
    LANE2_ALIGNMENT_MARKER_3 = 0xb4,
    LANE3_ALIGNMENT_MARKER_0 = 0xb8,
    LANE3_ALIGNMENT_MARKER_1 = 0xbc,
    LANE3_ALIGNMENT_MARKER_2 = 0xc0,
    LANE3_ALIGNMENT_MARKER_3 = 0xc4,
    GEARBOX_STATUS = 0xd0,
    METAL_FIX = 0xd4,
    FEC_DEC_DMA_CONTROL = 0xd8,
    FEC_DEC_DMA_WR_DATA = 0xdc,
    FEC_DEC_DMA_RD_DATA = 0xe0,
    FEC_DEC_CONFIGURATION_REG_0 = 0xe4,
    FEC_DEC_STATUS_REG = 0xe8,
    FDEC_RECEIVED_BLOCKS_REG_0 = 0xec,
    FDEC_RECEIVED_BLOCKS_REG_1 = 0xf0,
    FDEC_RECEIVED_NERROR_REG_0 = 0xf4,
    FDEC_RECEIVED_NERROR_REG_1 = 0xf8,
    FDEC_RECEIVED_CERROR_REG_0 = 0xfc,
    FDEC_RECEIVED_CERROR_REG_1 = 0x100,
    FDEC_RECEIVED_UNCERROR_REG_0 = 0x104,
    FDEC_RECEIVED_UNCERROR_REG_1 = 0x108,
    FDEC_RECEIVED_CERROR_BITS_REG_0 = 0x10c,
    FDEC_RECEIVED_CERROR_BITS_REG_1 = 0x110,
    FDEC_RECEIVED_FERROR_REG_0 = 0x114,
    FDEC_RECEIVED_FERROR_REG_1 = 0x118,
    FDEC_RG_LAST_P_S = 0x11c,
    FDEC_RG_LAST_E_S = 0x120,
    FENC_CONTROL_REG_0 = 0x128,
    FENC_CONTROL_REG_1 = 0x12c,
    FENC_CONTROL_REG_2 = 0x130,
    FENC_BLCK_NUM_LSB = 0x134,
    FENC_BLCK_NUM_MSB = 0x138,
    AP_CONFIGURATION_REG_0 = 0x13c,
    AP_CONFIGURATION_REG_1 = 0x140,
    AP_STATUS_REG_0 = 0x144,
    AP_STATUS_REG_1 = 0x148,
    PCS_RESET_REG = 0x14c,
    PRBS_CONTROL_REG0 = 0x150,
    PRBS_CONTROL_REG1 = 0x154,
    PRBS_STATUS_REG0 = 0x158,
    TEST_PATTERN_SEED_A_REG_0 = 0x15c,
    TEST_PATTERN_SEED_A_REG_1 = 0x160,
    TEST_PATTERN_SEED_A_REG_2 = 0x164,
    TEST_PATTERN_SEED_A_REG_3 = 0x168,
    TEST_PATTERN_SEED_B_REG_0 = 0x16c,
    TEST_PATTERN_SEED_B_REG_1 = 0x170,
    TEST_PATTERN_SEED_B_REG_2 = 0x174,
    TEST_PATTERN_SEED_B_REG_3 = 0x178,
    ENCODER_CONFIGURATION_REG = 0x17c,

/********* Addresses for TABLE SD_Regs.cfg ********/
    SERDES_EXTERNAL_CONFIGURATION_0 = 0x0,
    SERDES_EXTERNAL_CONFIGURATION_1 = 0x4,
    SERDES_EXTERNAL_CONFIGURATION_2 = 0x8,
    SERDES_EXTERNAL_CONFIGURATION_3 = 0xc,
    SERDES_EXTERNAL_CONFIGURATION_5 = 0x14,
    SD_METAL_FIX = SERDES_EXTERNAL_CONFIGURATION_5,
    SERDES_EXTERNAL_STATUS_0 = 0x18,
    SERDES_EXTERNAL_STATUS_1 = 0x1c,
    SERDES_EXTERNAL_CONFIGURATION_4 = 0x20,
    SERDES_EXTERNAL_STATUS_2 = 0x24,
    SERDES_RESERVED_REGISTER_0 = 0x28,
    SERDES_RESERVED_REGISTER_1 = 0x2c,
    SERDES_RESERVED_REGISTER_2 = 0x30,
    SERDES_RESERVED_STATUS_0 = 0x34,
    SERDES_PHY_REGS = 0x200,

/************ Addresses for Port_PTP_Units **********/
    PTP_PORT_CONTROL_REGISTER = 0x8,

/************ Addresses RxDMA_IP_Units **********/
    HIGH_DMA_I_OR_F_CONFIGURATION = 0x0F000094,

/************ Addresses TxDMA_IP_Units **********/
    EXTENEDED_PORTS_CONFIGURATION = 0x0F800070

}MV_LION2_REGS;

#define CGMAC_IND_OFFSET (0x300000)
#define PUMA3_CG_BASE (0x01800000)

typedef enum puma3Regs
{
    /* CG_UNIT */
    CG_CONTROL_0 = 0x0,
    CG_CONTROL_1 = 0x4,
    CG_CONTROL_2 = 0xC,
    CG_RESETS = 0x10,
    CG_RXDMA_CONVERTER_PACKET_SIZE_RESTRICTION = 0x14,
    CG_CREDIT_LIMIT = 0x1c,
    CG_DMA_FIFO_CONFIG = 0x1c,
    CG_CONVERTERS_STATUS = 0x20,
    CG_RECEIVED_PACKET_STATUS = 0x24,
    CG_IP_STATUS = 0x28,
    CG_IP_STATUS_2 = 0x2C,
    CG_RX_OVERSIZE_DROPPED_PACKET_COUNTER = 0x30,
    CG_RX_UNDERSIZE_DROPPED_PACKET_COUNTER = 0x34,
    CG_CG_INTERRUPT_CAUSE = 0x40,
    CG_CG_INTERRUPT_MASK = 0x44,
    CG_FC_CONTROL_0 = 0x60,
    CG_CONV_MARKER0_VL0 = 0x64,
    CG_CONV_MARKER1_VL0 = 0x74,
    CG_CONV_MARKER2_VL0 = 0x84,
    CG_CONV_MARKER3_VL0 = 0x94,
    CG_FEC_CERROR_COUNTER = 0xa0,
    CG_MULTI_LANE_ALIGN_STATUS1 = 0xC8,
    CG_MULTI_LANE_ALIGN_STATUS2 = 0xCC,
    CG_FEC_NCERROR_COUNTER = 0x100,
    CG_VIRTUAL_LANE_INTERVAL_1_0 = 0x104,
    CG_VIRTUAL_LANE_INTERVAL_3_2 = 0x108,
    CG_COMMAND_CONFIG = 0x408,
    CG_FRM_LENGTH = 0x414,
    CG_TX_FIFO_SECTIONS = 0x420,
    CG_TX_IPG_LENGTH = 0x444,
    CG_RX_CRC_OPT = 0x448,
    CG_INIT_CREDIT = 0x44C,

    /* CG_PCS_UNIT */
    CGPCS_CONTROL_1 = 0,
    CG_VIRTUAL_LANE_INTERVAL = 0x808,

    CG_VL0_0  = 0x900,
    CG_VL0_1  = 0x904,
    CG_VL1_0  = 0x908,
    CG_VL1_1  = 0x90C,
    CG_VL2_0  = 0x910,
    CG_VL2_1  = 0x914,
    CG_VL3_0  = 0x918,
    CG_VL3_1  = 0x91C,
    CG_VL4_0  = 0x920,
    CG_VL4_1  = 0x924,
    CG_VL5_0  = 0x928,
    CG_VL5_1  = 0x92C,
    CG_VL6_0  = 0x930,
    CG_VL6_1  = 0x934,
    CG_VL7_0  = 0x938,
    CG_VL7_1  = 0x93C,
    CG_VL8_0  = 0x940,
    CG_VL8_1  = 0x944,
    CG_VL9_0  = 0x948,
    CG_VL9_1  = 0x94C,
    CG_VL10_0 = 0x950,
    CG_VL10_1 = 0x954,
    CG_VL11_0 = 0x958,
    CG_VL11_1 = 0x95C,
    CG_VL12_0 = 0x960,
    CG_VL12_1 = 0x964,
    CG_VL13_0 = 0x968,
    CG_VL13_1 = 0x96C,
    CG_VL14_0 = 0x970,
    CG_VL14_1 = 0x974,
    CG_VL15_0 = 0x978,
    CG_VL15_1 = 0x97C,
    CG_VL16_0 = 0x980,
    CG_VL16_1 = 0x984,
    CG_VL17_0 = 0x988,
    CG_VL17_1 = 0x98C,
    CG_VL18_0 = 0x990,
    CG_VL18_1 = 0x994,
    CG_VL19_0 = 0x998,
    CG_VL19_1 = 0x99C,

    /* CG_RS_FEC Regs */
    CG_RS_FEC_CCW_LO                     = 0x8,
    CG_RS_FEC_CCW_HI                     = 0xC,
    CG_RS_FEC_NCCW_LO                    = 0x10,
    CG_RS_FEC_NCCW_HI                    = 0x14,
    CG_RS_FEC_SYMBLERR0_LO               = 0x28,
    CG_RS_FEC_SYMBLERR0_HI               = 0x2C,
    CG_RS_FEC_SYMBLERR1_LO               = 0x30,
    CG_RS_FEC_SYMBLERR1_HI               = 0x34,
    CG_RS_FEC_SYMBLERR2_LO               = 0x38,
    CG_RS_FEC_SYMBLERR2_HI               = 0x3C,
    CG_RS_FEC_SYMBLERR3_LO               = 0x40,
    CG_RS_FEC_SYMBLERR3_HI               = 0x44,
    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_0 = 0x228,
    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_1 = 0x22C,
    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_2 = 0x230,
    CG_RS_FEC_VENDOR_DECODER_THRESHOLD_3 = 0x234,

    /* Global ILKN unit INTLKN_UNIT WO */
    ILKN_0_MAC_CFG_0 = 0,
    ILKN_0_CH_FC_CFG_0 = 4,
    ILKN_0_MAC_CFG_2 = 0xC,
    ILKN_0_MAC_CFG_3 = 0x10,
    ILKN_0_MAC_CFG_4 = 0x14,
    ILKN_0_MAC_CFG_6 = 0x1C,
    ILKN_0_CH_FC_CFG_1 = 0x60,
    ILKN_0_ALIGN_STATUS = 0x10C,
    ILKN_0_PCS_CFG_0 = 0x200,
    ILKN_0_PCS_CFG_1 = 0x204,
    ILKN_0_EN = 0x20C,
    ILKN_0_STAT_EN = 0x238,

    /* Fabric Interlaken unit */
    GENX_converter_control_0 = 0,
    ILKN_RESETS = 0x4,
    SD_FC_LED_converter_control_0 = 0x10,
    TXDMA_converter_control_0 = 0x20,
    RXDMA_converter_control_0 = 0x40,

    /* ETI (external TCAM) unit */
    General_ETI_configuration0 = 0,
    General_ETI_configuration1 = 0x4,

    HWS_LAST_REG_ADDRESS
}MV_PUMA3_REGS;


#define HWS_MSDB_UNIT_OFFSET                    0x1000
#define HWS_MSDB_CHANNEL_OFFSET                 4
#define HWS_MPF_UNIT_OFFSET                     0x8000
#define HWS_MPF_CHANNEL_OFFSET                  0x4000

typedef enum falconRegs
{
    MTI_MAC_REVISION             =  0x0,
    MTI_MAC_SCRATCH              =  0x4,
    MTI_MAC_COMMAND_CONFIG       =  0x8,
    MTI_MAC_MAC_ADDR_0           =  0xC,
    MTI_MAC_MAC_ADDR_1           =  0x10,
    MTI_MAC_FRM_LENGTH           =  0x14,
    MTI_MAC_RX_FIFO_SECTIONS     =  0x1C,
    MTI_MAC_TX_FIFO_SECTIONS     =  0x20,
    MTI_MAC_MDIO_CFG_STATUS      =  0x30,
    MTI_MAC_MDIO_COMMAND         =  0x34,
    MTI_MAC_MDIO_DATA            =  0x38,
    MTI_MAC_MDIO_REGADDR         =  0x3C,
    MTI_MAC_STATUS               =  0x40,
    MTI_MAC_TX_IPG_LENGTH        =  0x44,
    MTI_MAC_CRC_MODE             =  0x48,
    MTI_MAC_CL01_PAUSE_QUANTA    =  0x54,
    MTI_MAC_CL23_PAUSE_QUANTA    =  0x58,
    MTI_MAC_CL45_PAUSE_QUANTA    =  0x5C,
    MTI_MAC_CL67_PAUSE_QUANTA    =  0x60,
    MTI_MAC_CL89_PAUSE_QUANTA    =  0x84,
    MTI_MAC_CL1011_PAUSE_QUANTA  =  0x88,
    MTI_MAC_CL1213_PAUSE_QUANTA  =  0x8C,
    MTI_MAC_CL1415_PAUSE_QUANTA  =  0x90,
    MTI_MAC_CL01_QUANTA_THRESH   =  0x64,
    MTI_MAC_CL23_QUANTA_THRESH   =  0x68,
    MTI_MAC_CL45_QUANTA_THRESH   =  0x6C,
    MTI_MAC_CL67_QUANTA_THRESH   =  0x70,
    MTI_MAC_RX_PAUSE_STATUS      =  0x74,
    MTI_MAC_TS_TIMESTAMP         =  0x7C,
    MTI_MAC_XIF_MODE             =  0x80,
    MTI_MAC_STATN_CONFIG         =  0xE0,
    MTI_MAC_STATN_CLEARVALUE_LO  =  0xE4,
    MTI_MAC_STATN_CLEARVALUE_HI  =  0xE8,

    /************** MTIP_PCS ****************/
    MTI_PCS_CONTROL1               = 0,
    MTI_PCS_STATUS1                = 0x4,
    MTI_PCS_STATUS2                = 0x20,
    MTI_PCS_BASER_STATUS1          = 0x80,
    MTI_PCS_BASER_STATUS2          = 0x84,
    MTI_PCS_BER_HIGH_ORDER_CNT     = 0xB0,
    MTI_PCS_ERR_BLK_HIGH_ORDER_CNT = 0xB4,
    MTI_PCS_VENDOR_VL_INTVL        = 0x808,
    MTI_PCS_VENDOR_TX_LANE_THRESH  = 0x80C,
    MTI_PCS_VENDOR_TX_CDMII_PACE   = 0x81C,
    MTI_PCS_VENDOR_AM_0            = 0x820,
    MTI_PCS_VENDOR_AM_1            = 0x824,
    MTI_PCS_VENDOR_PCS_MODE        = 0x840,
    MTI_PCS_VL0_0                  = 0x900,
    MTI_PCS_VL0_1                  = 0x904,
    MTI_PCS_VL1_0                  = 0x908,
    MTI_PCS_VL1_1                  = 0x90C,
    MTI_PCS_VL2_0                  = 0x910,
    MTI_PCS_VL2_1                  = 0x914,
    MTI_PCS_VL3_0                  = 0x918,
    MTI_PCS_VL3_1                  = 0x91C,
    MTI_PCS_VL4_0                  = 0x920,
    MTI_PCS_VL4_1                  = 0x924,
    MTI_PCS_VL5_0                  = 0x928,
    MTI_PCS_VL5_1                  = 0x92C,
    MTI_PCS_VL6_0                  = 0x930,
    MTI_PCS_VL6_1                  = 0x934,
    MTI_PCS_VL7_0                  = 0x938,
    MTI_PCS_VL7_1                  = 0x93C,
    MTI_PCS_VL8_0                  = 0x940,
    MTI_PCS_VL8_1                  = 0x944,
    MTI_PCS_VL9_0                  = 0x948,
    MTI_PCS_VL9_1                  = 0x94C,
    MTI_PCS_VL10_0                 = 0x950,
    MTI_PCS_VL10_1                 = 0x954,
    MTI_PCS_VL11_0                 = 0x958,
    MTI_PCS_VL11_1                 = 0x95C,
    MTI_PCS_VL12_0                 = 0x960,
    MTI_PCS_VL12_1                 = 0x964,
    MTI_PCS_VL13_0                 = 0x968,
    MTI_PCS_VL13_1                 = 0x96C,
    MTI_PCS_VL14_0                 = 0x970,
    MTI_PCS_VL14_1                 = 0x974,
    MTI_PCS_VL15_0                 = 0x978,
    MTI_PCS_VL15_1                 = 0x97C,
    MTI_PCS_VL16_0                 = 0x980,
    MTI_PCS_VL16_1                 = 0x984,
    MTI_PCS_VL17_0                 = 0x988,
    MTI_PCS_VL17_1                 = 0x98C,
    MTI_PCS_VL18_0                 = 0x990,
    MTI_PCS_VL18_1                 = 0x994,
    MTI_PCS_VL19_0                 = 0x998,
    MTI_PCS_VL19_1                 = 0x99C,

    /************** MTIP_LPCS ****************/
    MTI_LPCS_CONTROL               = 0x0,
    MTI_LPCS_STATUS                = 0x4,
    MTI_LPCS_DEV_ABILITY           = 0x010,
    MTI_LPCS_PARTNER_ABILITY       = 0x014,
    MTI_LPCS_LINK_TIMER_0          = 0x048,
    MTI_LPCS_LINK_TIMER_1          = 0x04C,
    MTI_LPCS_IF_MODE               = 0x050,
    MTI_LPCS_GMODE                 = 0x3E0,
    MTI_LPCS_GSTATUS               = 0x3E4,


    /************** MTIP_RSFEC ****************/
    MTI_RSFEC_CONTROL              = 0x0,
    MTI_RSFEC_STATUS               = 0x4,
    MTI_RSFEC_CCW_LO               = 0x8,
    MTI_RSFEC_CCW_HI               = 0xC,
    MTI_RSFEC_NCCW_LO              = 0x10,
    MTI_RSFEC_NCCW_HI              = 0x14,
    MTI_RSFEC_DEC_THRESH           = 0x1C,
    MTI_RSFEC_VENDOR_VL0_0         = 0x180,
    MTI_RSFEC_VENDOR_VL0_1         = 0x184,
    MTI_RSFEC_VENDOR_VL1_0         = 0x188,
    MTI_RSFEC_VENDOR_VL1_1         = 0x18c,
    MTI_RSFEC_VENDOR_VL2_0         = 0x190,
    MTI_RSFEC_VENDOR_VL2_1         = 0x194,
    MTI_RSFEC_VENDOR_VL3_0         = 0x198,
    MTI_RSFEC_VENDOR_VL3_1         = 0x19c,
    MTI_RSFEC_VENDOR_VL4_0         = 0x1a0,
    MTI_RSFEC_VENDOR_VL4_1         = 0x1a4,
    MTI_RSFEC_VENDOR_VL5_0         = 0x1a8,
    MTI_RSFEC_VENDOR_VL5_1         = 0x1ac,
    MTI_RSFEC_VENDOR_VL6_0         = 0x1b0,
    MTI_RSFEC_VENDOR_VL6_1         = 0x1b4,
    MTI_RSFEC_VENDOR_VL7_0         = 0x1b8,
    MTI_RSFEC_VENDOR_VL7_1         = 0x1bc,
    MTI_RSFEC_VENDOR_VL8_0         = 0x1c0,
    MTI_RSFEC_VENDOR_VL8_1         = 0x1c4,
    MTI_RSFEC_VENDOR_VL9_0         = 0x1c8,
    MTI_RSFEC_VENDOR_VL9_1         = 0x1cc,
    MTI_RSFEC_VENDOR_VL10_0        = 0x1d0,
    MTI_RSFEC_VENDOR_VL10_1        = 0x1d4,
    MTI_RSFEC_VENDOR_VL11_0        = 0x1d8,
    MTI_RSFEC_VENDOR_VL11_1        = 0x1dc,
    MTI_RSFEC_VENDOR_VL12_0        = 0x1e0,
    MTI_RSFEC_VENDOR_VL12_1        = 0x1e4,
    MTI_RSFEC_VENDOR_VL13_0        = 0x1e8,
    MTI_RSFEC_VENDOR_VL13_1        = 0x1ec,
    MTI_RSFEC_VENDOR_VL14_0        = 0x1f0,
    MTI_RSFEC_VENDOR_VL14_1        = 0x1f4,
    MTI_RSFEC_VENDOR_VL15_0        = 0x1f8,
    MTI_RSFEC_VENDOR_VL15_1        = 0x1fc,
    MTI_RSFEC_SYMBLERR0_LO         = 0x200,
    MTI_RSFEC_SYMBLERR0_HI         = 0x204,
    MTI_RSFEC_SYMBLERR1_LO         = 0x208,
    MTI_RSFEC_SYMBLERR1_HI         = 0x20c,
    MTI_RSFEC_SYMBLERR2_LO         = 0x210,
    MTI_RSFEC_SYMBLERR2_HI         = 0x214,
    MTI_RSFEC_SYMBLERR3_LO         = 0x218,
    MTI_RSFEC_SYMBLERR3_HI         = 0x21c,
    MTI_RSFEC_SYMBLERR4_LO         = 0x220,
    MTI_RSFEC_SYMBLERR4_HI         = 0x224,
    MTI_RSFEC_SYMBLERR5_LO         = 0x228,
    MTI_RSFEC_SYMBLERR5_HI         = 0x22c,
    MTI_RSFEC_SYMBLERR6_LO         = 0x230,
    MTI_RSFEC_SYMBLERR6_HI         = 0x234,
    MTI_RSFEC_SYMBLERR7_LO         = 0x238,
    MTI_RSFEC_SYMBLERR7_HI         = 0x23c,
    MTI_RSFEC_SYMBLERR8_LO         = 0x240,
    MTI_RSFEC_SYMBLERR8_HI         = 0x244,
    MTI_RSFEC_SYMBLERR9_LO         = 0x248,
    MTI_RSFEC_SYMBLERR9_HI         = 0x24c,
    MTI_RSFEC_SYMBLERR10_LO        = 0x250,
    MTI_RSFEC_SYMBLERR10_HI        = 0x254,
    MTI_RSFEC_SYMBLERR11_LO        = 0x258,
    MTI_RSFEC_SYMBLERR11_HI        = 0x25c,
    MTI_RSFEC_SYMBLERR12_LO        = 0x260,
    MTI_RSFEC_SYMBLERR12_HI        = 0x264,
    MTI_RSFEC_SYMBLERR13_LO        = 0x268,
    MTI_RSFEC_SYMBLERR13_HI        = 0x26c,
    MTI_RSFEC_SYMBLERR14_LO        = 0x270,
    MTI_RSFEC_SYMBLERR14_HI        = 0x274,
    MTI_RSFEC_SYMBLERR15_LO        = 0x278,
    MTI_RSFEC_SYMBLERR15_HI        = 0x27c,
    /************** MTIP_EXT ****************/
    MTIP_EXT_GLOBAL_PMA_CONTROL     = 0x0,
    MTIP_EXT_GLOBAL_FEC_CONTROL     = 0x4,
    MTIP_EXT_GLOBAL_CHANNEL_CONTROL = 0x8,
    MTIP_EXT_GLOBAL_CLOCK_CONTROL   = 0xC,
    MTIP_EXT_GLOBAL_CLOCK_ENABLE    = 0x10,
    MTIP_EXT_GLOBAL_RESET_CONTROL   = 0x14,
    MTIP_EXT_GLOBAL_METAL_FIX       = 0x58,
    MTIP_EXT_SEG_PORT_CONTROL       = 0x5c,
    MTIP_EXT_SEG_PORT_STATUS        = 0x60,
    MTIP_EXT_PORT_CONTROL           = 0x84,
    MTIP_EXT_PORT_STATUS            = 0x88,

    /************** MTIP_CPU_EXT **************/
    MTIP_CPU_EXT_PORT_CONTROL       = 0x0,
    MTIP_CPU_EXT_PORT_STATUS        = 0x4,
    MTIP_CPU_EXT_PORT_RESET         = 0x18,
    MTIP_CPU_EXT_PORT_CLOCKS        = 0x1C,

    /************** D2D address ***************/
    D2D_PHY_TXC_APBWAIT           = 0x4   ,
    D2D_PHY_TXC_OVRCTRL           = 0x10  ,
    D2D_PHY_TXC_CFG               = 0x14  ,
    D2D_PHY_TXC_PLL_CTRL          = 0x40  ,
    D2D_PHY_TXC_PLL_LOCK_STS      = 0x44  ,
    D2D_PHY_TXC_PLL_CP_CTRL       = 0x50  ,
    D2D_PHY_TXC_PLL_LOOP_CTRL     = 0x54  ,
    D2D_PHY_TXC_SLCTHRES0         = 0xB8  ,
    D2D_PHY_TXC_PFBD              = 0x60  ,          /* PLL multiplier */
    D2D_PHY_TXC_PLL_DEBUG_CTRL    = 0x68  ,
    D2D_PHY_TX_CFG                = 0x440 ,
    D2D_PHY_TX_PATSEL1            = 0x448 ,
    D2D_PHY_TX_DRVSLCEN_7_0       = 0x4A0 ,
    D2D_PHY_TX_DRVSLCEN_11_8      = 0x4A4 ,
    D2D_PHY_TX_COMMON_MODE_TERM_ENABLE = 0x4A8,
    D2D_PHY_TX_EQCMSLCEN          = 0x4AC ,
    D2D_PHY_TX_SST_DRIVER_ENABLE  = 0x4B0 ,
    D2D_PHY_RXC_AUTOSTART         = 0x2004,
    D2D_PHY_RXC_AUTOSTART_ERR     = 0x2008,
    D2D_PHY_RXC_AUTOCAL           = 0x200C,
    D2D_PHY_RXC_AUTOSTAT          = 0x2010,
    D2D_PHY_RXC_CFG               = 0x201C,
    D2D_PHY_RXC_BIAS_CONFIG       = 0x2050,
    D2D_PHY_RXC_DAC_CONFIG        = 0x2080,
    D2D_PHY_RXC_PLL_REG_LEVEL     = 0x20F0,
    D2D_PHY_RXC_DEBUG_CTRL        = 0x2140,

    D2D_PHY_RX_AUTOCAL            = 0x2400,
    D2D_PHY_RX_AUTOSTAT           = 0x2404,
    D2D_PHY_RX_ANALYSIS_MODE      = 0x240C,

    D2D_PHY_RX_PAT_ERR_COUNT_BASE = 0x2570,
    D2D_PHY_RX_PATPRBS            = 0x25C8,
    D2D_PHY_RX_PATRSTSNAP         = 0x25D4,
    D2D_PHY_RX_PATSTAT            = 0x25DC,
    D2D_PHY_RX_CDA_PI_KP          = 0x2650,
    D2D_PHY_RX_CDA_CFG            = 0x2654,
    D2D_PHY_RX_PI_CODE_OVERRIDE_VALUE = 0x2684,
    D2D_PHY_RX_SCHCFG             = 0x26C0,
    D2D_PHY_RX_SCHEXP             = 0x26C4,
    D2D_PHY_RX_CENTERLEVEL        = 0x2724,
    D2D_PHY_RX_CTLE_CTRL          = 0x2740,
    D2D_PHY_RX_CFG                = 0x2748,
    D2D_PHY_RX_PLL_LOCK_STS       = 0x274C,
    D2D_PHY_RX_PLL_LOCK_CTRL      = 0x2750,
    D2D_PHY_RX_CTLE_EQ            = 0x2784,

    D2D_PCS_PCS_CFG_ENA             = 0x7000,
    D2D_PCS_PCS_TX_CAL_CTRL         = 0x7004,
    D2D_PCS_PCS_RX_CAL_CTRL         = 0x7008,
    D2D_PCS_PCS_TX_RATE_CTRL        = 0x700C,
    D2D_PCS_PCS_STAT_TX_BLOCK_LOW   = 0x7010,
    D2D_PCS_PCS_STAT_TX_BLOCK_HIGH  = 0x7014,
    D2D_PCS_PCS_STAT_RX_BLOCK_LOW   = 0x7018,
    D2D_PCS_PCS_STAT_RX_BLOCK_HIGH  = 0x701c,
    D2D_PCS_AM_BER_CNT_LANE         = 0x7020,
    D2D_PCS_EVENT_CAUSE             = 0x7040,
    D2D_PCS_PCS_RX_STATUS           = 0x7058,
    D2D_PCS_DEBUG_ERROR_INJECT      = 0x7074,
    D2D_PCS_PCS_OOR_CFG             = 0x7084,
    D2D_PCS_DBG_LANE_CTRL           = 0x7100,
    D2D_PCS_PCS_TX_CAL_BASE         = 0x7200,
    D2D_PCS_PCS_RX_CAL_BASE         = 0x7400,
    D2D_PCS_RX_IFC_REMAP_8          = 0x7620,
    D2D_PCS_RX_IFC_REMAP_9          = 0x7624,
    D2D_PCS_RX_IFC_REMAP_16         = 0x7640,
    D2D_PCS_RX_IFC_REMAP_17         = 0x7644,
    D2D_PMA_CFG_RESET               = 0x8004,
    D2D_PMA_CFG_EN                  = 0x8008,
    D2D_PMA_DBG_ALIGN_CTRL_LANE_ID  = 0x8020,
    D2D_PMA_STICKY_STATUS           = 0x8030,
    D2D_PMA_METAL_FIX               = 0x803C,
    D2D_PMA_PMA_FEC_COR_CNT         = 0x8040,
    D2D_PMA_PMA_FEC_UNCOR_CNT       = 0x8050,

    D2D_KDU_FIFO_CTRL               = 0x9000,
    D2D_KDU_FIFO_FRZ_CTRL           = 0x9004,
    D2D_KDU_FIFO_XBARCFG_LANE       = 0x901C,
    D2D_KDU_FIFO_LANE_CTRL          = 0x9044,
    D2D_KDU_PAT_RX_CTRL_LANE        = 0x9800,
    D2D_KDU_PAT_TX_CTRL_LANE        = 0x9C00,
    D2D_KDU_PAT_RX_PSEL_LANE        = 0x9804,
    D2D_KDU_PAT_TX_PSEL_LANE        = 0x9C04,
    D2D_KDU_PAT_RX_STS              = 0x9808,
    D2D_KDU_PAT_TX_STS              = 0x9C08,
    D2D_KDU_PAT_RX_PUSER_PAT_LANE   = 0x9810,
    D2D_KDU_PAT_TX_PUSER_PAT_LANE   = 0x9C10,
    D2D_KDU_PAT_RX_PERR_CNT_LANE    = 0x9820,
    D2D_KDU_PAT_TX_PERR_CNT_LANE    = 0x9C20,
    D2D_KDU_PAT_RX_PUSER_MASK_LANE  = 0x9830,
    D2D_KDU_PAT_TX_PUSER_MASK_LANE  = 0x9C30,

    D2D_MAC_CFG_TX_CHANNEL                 = 0x4004,
    D2D_MAC_CFG_TX_FIFO_SEG_POINTER        = 0x4104,
    D2D_MAC_TX_STAT_GOOD_PACKETS_LOW       = 0x4204,
    D2D_MAC_TX_STAT_GOOD_PACKETS_HIGH      = 0x4208,
    D2D_MAC_TX_STAT_BAD_PACKETS_LOW        = 0x4404,
    D2D_MAC_TX_STAT_BAD_PACKETS_HIGH       = 0x4408,
    D2D_MAC_CFG_RX_TDM_SCHEDULE            = 0x4604,
    D2D_MAC_CFG_RX_FIFO_BANDWIDTH_SELECT   = 0X47E4,
    D2D_MAC_CFG_RX_CHANNEL                 = 0x4804,
    D2D_MAC_CFG_RX_FIFO_SEG_POINTER        = 0x4904,
    D2D_MAC_CFG_RX_CHANNEL_2               = 0x4E04,
    D2D_MAC_RX_STAT_GOOD_PACKETS_LOW       = 0x4A04,
    D2D_MAC_RX_STAT_GOOD_PACKETS_HIGH      = 0x4A08,
    D2D_MAC_RX_STAT_BAD_PACKETS_LOW        = 0x4C04,
    D2D_MAC_RX_STAT_BAD_PACKETS_HIGH       = 0x4C08,
    D2D_MAC_DBG_CFG_FRM_EN                 = 0x4FFC,
    D2D_MAC_DBG_FRM_GEN_CFG                = 0x5000,
    D2D_MAC_DBG_FRM_GEN_CFG_STAT           = 0x5100,
    D2D_MAC_DBG_FRM_TBL_START_ADDR         = 0x5200,
    D2D_MAC_DBG_FRM_TBL_LEN_START_ADDR     = 0x5300,
    D2D_MAC_DBG_FRM_TBL_HEADER_START_ADDR  = 0x5400,
    D2D_MAC_DBG_FRM_TBL_PAYLOAD_START_ADDR = 0x5B00,
    D2D_MAC_DBG_FRM_CHECK_CFG_START_ADDR   = 0x5C00,
    D2D_MAC_DBG_FRM_CNT_GOOD_START_ADDR    = 0x5E00,
    D2D_MAC_DBG_FRM_CNT_BAD_START_ADDR     = 0x5F00,

    MSDB_CHANNEL_TX_CONTROL                = 0x000,
    MSDB_CHANNEL_RX_CONTROL                = 0x200,
    MSDB_FCU_CHANNEL_TX_CONTROL            = 0x500,
    MSDB_FCU_CHANNEL_RX_CONTROL            = 0x540,
    MSDB_FCU_CONTROL                       = 0x584,
    MSDB_SEGMENTED_CHANNEL_TX_CONTROL      = 0x100,
    MSDB_SEGMENTED_CHANNEL_RX_CONTROL      = 0x300,

    MPFS_PAU_CONTROL                       = 0,
    MPFS_SAU_CONTROL                       = 0x100,
    MPFS_SAU_STATUS                        = 0x104,
    MPFS_CLOCK_CONTROL                     = 0x31C,
    MPFS_LOOPBACK_CONTROL                  = 0x310,

    TSU_CONTROL                            = 0x08,

    EAGLE_D2D_CP_CFG_X2D_RDREQ_TIMEOUT     = 0x002E000C,
    EAGLE_D2D_CP_CFG_D2X_REQ_TIMEOUT       = 0x002E0010,
    EAGLE_D2D_CP_INTERRUPT_MASK            = 0x002E0054,
    D2D_CP_CFG_X2D_RDREQ_TIMEOUT           = 0x002F000C,
    D2D_CP_CFG_D2X_REQ_TIMEOUT             = 0x002F0010,
    D2D_CP_CFG_INT_CBE                     = 0x002F0030,
    D2D_CP_CFG_INT_CBE_DISABLE             = 0x002F0034,
    D2D_CP_INTERRUPT_MASK                  = 0x002F0054,

    MG_XSMI_MANAGEMENT                     = 0x330000,
    MG_XSMI_ADDRESS                        = 0x330008,
    MG_XSMI_CONFIGURATION                  = 0x33000C,
    MPP_CONTROL_0_7                        = 0x118000,
    MPP_CONTROL_8_15                       = 0x118004
}MV_FALCON_REGS;

#define FALCON_DFX_BASE_ADDRESS_CNS  0x1BE00000
#define FALCON_TILE_OFFSET           0x20000000

#define MV_HWS_SERDES_TIED_PIN      0x7FFF
#define MV_HWS_SERDES_REG_NA        0xFFFF

#define SDW_INTERNAL_REG_OFFSET     0x20000
#define SDW_EXTERNAL_REG_OFFSET     0x00000
#define SDW_CONFIGURATIONS_OFFSET   0x000
#define SDW_STATUS_OFFSET           0x200
#define SDW_INTERRUPTS_OFFSET       0x300
#define SDW_DEBUG_OFFSET            0x400
#define SDW_DFX_OFFSET              0x500

typedef enum sdwRegs
{
    /************** Configurations ****************/
    /* General Configurations */
    SDW_GENERAL_CONTROL_0                   = SDW_CONFIGURATIONS_OFFSET + 0x0,
    SDW_GENERAL_CONTROL_1                   = SDW_CONFIGURATIONS_OFFSET + 0x4,
    SDW_GENERAL_CONTROL_2                   = SDW_CONFIGURATIONS_OFFSET + 0x8,
    SDW_GENERAL_CONTROL_3                   = SDW_CONFIGURATIONS_OFFSET + 0xC,
    SDW_GENERAL_MUX_CONTROL                 = SDW_CONFIGURATIONS_OFFSET + 0x10,
    SDW_WATCHDOG_TIMER_CONTROL              = SDW_CONFIGURATIONS_OFFSET + 0x14,

    /* Per Lane Configurations*/
    SDW_LANE_CONTROL_0                      = SDW_CONFIGURATIONS_OFFSET + 0x20,
    SDW_LANE_CONTROL_1                      = SDW_CONFIGURATIONS_OFFSET + 0x40,
    SDW_LANE_CONTROL_2                      = SDW_CONFIGURATIONS_OFFSET + 0x60,
    SDW_LANE_CONTROL_3                      = SDW_CONFIGURATIONS_OFFSET + 0x80,
    SDW_LANE_CONTROL_4                      = SDW_CONFIGURATIONS_OFFSET + 0xA0,
    SDW_LANE_CONTROL_5                      = SDW_CONFIGURATIONS_OFFSET + 0xC0,
    SDW_LANE_MUX_CONTROL_0                  = SDW_CONFIGURATIONS_OFFSET + 0xE0,
    SDW_LANE_MUX_CONTROL_1                  = SDW_CONFIGURATIONS_OFFSET + 0x100,
    SDW_LANE_CONTROL_6                      = SDW_CONFIGURATIONS_OFFSET + 0x120,

    /****************** Status ********************/
    /* General Status */
    SDW_GENERAL_STATUS_0                    = SDW_STATUS_OFFSET + 0x0,
    SDW_GENERAL_STATUS_1                    = SDW_STATUS_OFFSET + 0x4,

    /* Per Lane Status */
    SDW_LANE_STATUS_0                       = SDW_STATUS_OFFSET + 0x20,
    SDW_LANE_STATUS_1                       = SDW_STATUS_OFFSET + 0x40,
    SDW_LANE_STATUS_2                       = SDW_STATUS_OFFSET + 0x60,
    SDW_LANE_STATUS_3                       = SDW_STATUS_OFFSET + 0x80,
    SDW_LANE_STATUS_4                       = SDW_STATUS_OFFSET + 0xA0,

    /**************** Interrupts ******************/
    SDW_INTERRUPT_CAUSE                     = SDW_INTERRUPTS_OFFSET + 0x0,
    SDW_INTERRUPT_MASK                      = SDW_INTERRUPTS_OFFSET + 0x4,
    SDW_INVALID_ADDRESS_LATCH               = SDW_INTERRUPTS_OFFSET + 0x8,

    /****************** Debug *********************/

    /******************** DFX *********************/
    SDW_METAL_FIX                           = SDW_DFX_OFFSET + 0x0,

}MV_SDW_REGS;
#define MV_SDW_IS_PER_LANE_REG_MAC(_reg) (((SDW_LANE_CONTROL_0 <= _reg) && (_reg <= SDW_LANE_CONTROL_6)) \
                                       || ((SDW_LANE_STATUS_0  <= _reg) && (_reg <= SDW_LANE_STATUS_4)))
typedef struct
{
    GT_U16 regOffset;
    GT_U16 offsetFormula;
    GT_U8  fieldStart;
    GT_U8  fieldLen;
} MV_HWS_REG_ADDR_FIELD_STC;

typedef enum {
    MV_HWS_REG_NEXT_PORT_STEP_NONE_E    = 0 ,
    MV_HWS_REG_NEXT_PORT_STEP_USE_DEFAULT_E ,
    MV_HWS_REG_NEXT_PORT_STEP_ONE_E         ,
    MV_HWS_REG_NEXT_PORT_STEP_LAST
}MV_HWS_REG_NEXT_PORT_STEP_ENT;


#define MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field) _regPrefix##0##_field
#define MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field) _regPrefix##1##_field
#define MV_HWS_SET_FIELD_IDX2_MAC(_regPrefix,_field) _regPrefix##2##_field
#define MV_HWS_SET_FIELD_IDX3_MAC(_regPrefix,_field) _regPrefix##3##_field
#define MV_HWS_SET_FIELD_IDX4_MAC(_regPrefix,_field) _regPrefix##4##_field
#define MV_HWS_SET_FIELD_IDX5_MAC(_regPrefix,_field) _regPrefix##5##_field
#define MV_HWS_SET_FIELD_IDX6_MAC(_regPrefix,_field) _regPrefix##6##_field
#define MV_HWS_SET_FIELD_IDX7_MAC(_regPrefix,_field) _regPrefix##7##_field
#define MV_HWS_SET_FIELD_IDX8_MAC(_regPrefix,_field) _regPrefix##8##_field
#define MV_HWS_SET_FIELD_IDX9_MAC(_regPrefix,_field) _regPrefix##9##_field
#define MV_HWS_SET_FIELD_IDX10_MAC(_regPrefix,_field) _regPrefix##10##_field
#define MV_HWS_SET_FIELD_IDX11_MAC(_regPrefix,_field) _regPrefix##11##_field
#define MV_HWS_SET_FIELD_IDX12_MAC(_regPrefix,_field) _regPrefix##12##_field
#define MV_HWS_SET_FIELD_IDX13_MAC(_regPrefix,_field) _regPrefix##13##_field
#define MV_HWS_SET_FIELD_IDX14_MAC(_regPrefix,_field) _regPrefix##14##_field
#define MV_HWS_SET_FIELD_IDX15_MAC(_regPrefix,_field) _regPrefix##15##_field
#define MV_HWS_SET_FIELD_IDX16_MAC(_regPrefix,_field) _regPrefix##16##_field
#define MV_HWS_SET_FIELD_IDX17_MAC(_regPrefix,_field) _regPrefix##17##_field
#define MV_HWS_SET_FIELD_IDX18_MAC(_regPrefix,_field) _regPrefix##18##_field
#define MV_HWS_SET_FIELD_IDX19_MAC(_regPrefix,_field) _regPrefix##19##_field
#define MV_HWS_SET_FIELD_IDX20_MAC(_regPrefix,_field) _regPrefix##20##_field
#define MV_HWS_SET_FIELD_IDX21_MAC(_regPrefix,_field) _regPrefix##21##_field
#define MV_HWS_SET_FIELD_IDX22_MAC(_regPrefix,_field) _regPrefix##22##_field
#define MV_HWS_SET_FIELD_IDX23_MAC(_regPrefix,_field) _regPrefix##23##_field
#define MV_HWS_SET_FIELD_IDX24_MAC(_regPrefix,_field) _regPrefix##24##_field

/* create field to match cider register and field by cocatenating _regPrefix
   cider index and _field to one enum
   this enum support cider index 0-7*/
#define MV_HWS_SET_FIELD_IDX0_7_MAC(_output, _ciderIdx, _regPrefix,_field)  \
switch(_ciderIdx)                                                        \
{                                                                        \
    case 0:                                                              \
        _output = MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field); break;   \
    case 1:                                                              \
        _output = MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field); break;   \
    case 2:                                                              \
        _output = MV_HWS_SET_FIELD_IDX2_MAC(_regPrefix,_field); break;   \
    case 3:                                                              \
        _output = MV_HWS_SET_FIELD_IDX3_MAC(_regPrefix,_field); break;   \
    case 4:                                                              \
        _output = MV_HWS_SET_FIELD_IDX4_MAC(_regPrefix,_field); break;   \
    case 5:                                                              \
        _output = MV_HWS_SET_FIELD_IDX5_MAC(_regPrefix,_field); break;   \
    case 6:                                                              \
        _output = MV_HWS_SET_FIELD_IDX6_MAC(_regPrefix,_field); break;   \
    case 7:                                                              \
        _output = MV_HWS_SET_FIELD_IDX7_MAC(_regPrefix,_field); break;   \
    default:                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);   \
}

/* create field to match cider register and field by cocatenating _regPrefix
   cider index and _field to one enum
   this enum support cider index 0-15*/
#define MV_HWS_SET_FIELD_IDX0_15_MAC(_output, _ciderIdx, _regPrefix,_field)  \
switch(_ciderIdx)                                                        \
{                                                                        \
    case 0:                                                              \
        _output = MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field); break;   \
    case 1:                                                              \
        _output = MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field); break;   \
    case 2:                                                              \
        _output = MV_HWS_SET_FIELD_IDX2_MAC(_regPrefix,_field); break;   \
    case 3:                                                              \
        _output = MV_HWS_SET_FIELD_IDX3_MAC(_regPrefix,_field); break;   \
    case 4:                                                              \
        _output = MV_HWS_SET_FIELD_IDX4_MAC(_regPrefix,_field); break;   \
    case 5:                                                              \
        _output = MV_HWS_SET_FIELD_IDX5_MAC(_regPrefix,_field); break;   \
    case 6:                                                              \
        _output = MV_HWS_SET_FIELD_IDX6_MAC(_regPrefix,_field); break;   \
    case 7:                                                              \
        _output = MV_HWS_SET_FIELD_IDX7_MAC(_regPrefix,_field); break;   \
    case 8:                                                              \
        _output = MV_HWS_SET_FIELD_IDX8_MAC(_regPrefix,_field);  break;  \
    case 9:                                                              \
        _output = MV_HWS_SET_FIELD_IDX9_MAC(_regPrefix,_field);  break;  \
    case 10:                                                             \
        _output = MV_HWS_SET_FIELD_IDX10_MAC(_regPrefix,_field); break;  \
    case 11:                                                             \
        _output = MV_HWS_SET_FIELD_IDX11_MAC(_regPrefix,_field); break;  \
    case 12:                                                             \
        _output = MV_HWS_SET_FIELD_IDX12_MAC(_regPrefix,_field); break;  \
    case 13:                                                             \
        _output = MV_HWS_SET_FIELD_IDX13_MAC(_regPrefix,_field); break;  \
    case 14:                                                             \
        _output = MV_HWS_SET_FIELD_IDX14_MAC(_regPrefix,_field); break;  \
    case 15:                                                             \
        _output = MV_HWS_SET_FIELD_IDX15_MAC(_regPrefix,_field); break;  \
    default:                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);   \
}

/* create field to match cider register and field by cocatenating _regPrefix
   cider index and _field to one enum
   this enum support cider index 0-1*/
#define MV_HWS_SET_FIELD_IDX0_1_MAC(_output, _ciderIdx, _regPrefix,_field)  \
switch(_ciderIdx)                                                        \
{                                                                        \
    case 0:                                                              \
        _output = MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field); break;   \
    case 1:                                                              \
        _output = MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field); break;   \
    default:                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
}

/* create field to match cider register and field by cocatenating _regPrefix
   cider index and _field to one enum
   this enum support cider index 0-3*/
#define MV_HWS_SET_FIELD_IDX0_3_MAC(_output, _ciderIdx, _regPrefix,_field)  \
switch(_ciderIdx)                                                        \
{                                                                        \
    case 0:                                                              \
        _output = MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field); break;   \
    case 1:                                                              \
        _output = MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field); break;   \
    case 2:                                                              \
        _output = MV_HWS_SET_FIELD_IDX2_MAC(_regPrefix,_field); break;   \
    case 3:                                                              \
        _output = MV_HWS_SET_FIELD_IDX3_MAC(_regPrefix,_field); break;   \
    default:                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
}

/* create field to match cider register and field by cocatenating _regPrefix
   cider index and _field to one enum
   this enum support cider index 0-24*/
#define MV_HWS_SET_FIELD_IDX0_24_MAC(_output, _portNum, _regPrefix,_field)       \
switch(_portNum)                                                         \
{                                                                        \
    case 0:                                                              \
        _output = MV_HWS_SET_FIELD_IDX0_MAC(_regPrefix,_field);  break;  \
    case 1:                                                              \
        _output = MV_HWS_SET_FIELD_IDX1_MAC(_regPrefix,_field);  break;  \
    case 2:                                                              \
        _output = MV_HWS_SET_FIELD_IDX2_MAC(_regPrefix,_field);  break;  \
    case 3:                                                              \
        _output = MV_HWS_SET_FIELD_IDX3_MAC(_regPrefix,_field);  break;  \
    case 4:                                                              \
        _output = MV_HWS_SET_FIELD_IDX4_MAC(_regPrefix,_field);  break;  \
    case 5:                                                              \
        _output = MV_HWS_SET_FIELD_IDX5_MAC(_regPrefix,_field);  break;  \
    case 6:                                                              \
        _output = MV_HWS_SET_FIELD_IDX6_MAC(_regPrefix,_field);  break;  \
    case 7:                                                              \
        _output = MV_HWS_SET_FIELD_IDX7_MAC(_regPrefix,_field);  break;  \
    case 8:                                                              \
        _output = MV_HWS_SET_FIELD_IDX8_MAC(_regPrefix,_field);  break;  \
    case 9:                                                              \
        _output = MV_HWS_SET_FIELD_IDX9_MAC(_regPrefix,_field);  break;  \
    case 10:                                                             \
        _output = MV_HWS_SET_FIELD_IDX10_MAC(_regPrefix,_field); break;  \
    case 11:                                                             \
        _output = MV_HWS_SET_FIELD_IDX11_MAC(_regPrefix,_field); break;  \
    case 12:                                                             \
        _output = MV_HWS_SET_FIELD_IDX12_MAC(_regPrefix,_field); break;  \
    case 13:                                                             \
        _output = MV_HWS_SET_FIELD_IDX13_MAC(_regPrefix,_field); break;  \
    case 14:                                                             \
        _output = MV_HWS_SET_FIELD_IDX14_MAC(_regPrefix,_field); break;  \
    case 15:                                                             \
        _output = MV_HWS_SET_FIELD_IDX15_MAC(_regPrefix,_field); break;  \
    case 16:                                                             \
        _output = MV_HWS_SET_FIELD_IDX16_MAC(_regPrefix,_field); break;  \
    case 17:                                                             \
        _output = MV_HWS_SET_FIELD_IDX17_MAC(_regPrefix,_field); break;  \
    case 18:                                                             \
        _output = MV_HWS_SET_FIELD_IDX18_MAC(_regPrefix,_field); break;  \
    case 19:                                                             \
        _output = MV_HWS_SET_FIELD_IDX19_MAC(_regPrefix,_field); break;  \
    case 20:                                                             \
        _output = MV_HWS_SET_FIELD_IDX20_MAC(_regPrefix,_field); break;  \
    case 21:                                                             \
        _output = MV_HWS_SET_FIELD_IDX21_MAC(_regPrefix,_field); break;  \
    case 22:                                                             \
        _output = MV_HWS_SET_FIELD_IDX22_MAC(_regPrefix,_field); break;  \
    case 23:                                                             \
        _output = MV_HWS_SET_FIELD_IDX23_MAC(_regPrefix,_field); break;  \
    case 24:                                                             \
        _output = MV_HWS_SET_FIELD_IDX24_MAC(_regPrefix,_field); break;  \
    default:                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
}

#endif /* __siliconAddress_h*/
