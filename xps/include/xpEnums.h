// xpEnums.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpEnums.h
 * \brief
 *
 */

#ifndef _xpEnums_h_
#define _xpEnums_h_

#include "openXpsEnums.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS        ///< Helper macros for printing variable
#endif
#include <inttypes.h>

typedef float Float;

/**
 * \public
 * \brief xpHdrProfile_t
 * Enum describing the header profiles supported in the
 *        system
 */
typedef enum
{
    // L2/Nexthop Insert/Modify Hdr Profile
    XP_VLAN_UNTAGGED_HDR_PROFILE = 0,   // Modify: Strips .1Q tag if present. Insert: Creates untagged 1.q header.
    XP_VLAN_TAGGED_HDR_PROFILE,              // Modify: Writes .1Q tag. Insert: Creates .1Q tagged header. Uses info from VIF.
    XP_VLAN_TAGGED_TOKEN_HDR_PROFILE,  // Modify: Writes .1Q tag. Insert: No OP. Uses info from Token.
    XP_VLAN_Q_IN_Q_NOTAG_HDR_PROFILE,    // Modify: Strips both sTag and cTag. Insert: No OP.
    XP_VLAN_Q_IN_Q_CTAGGED_HDR_PROFILE,// Modify: Strips sTag. Insert: No OP.
    XP_VLAN_Q_IN_Q_STAGGED_HDR_PROFILE, //Modify: Writes sTag. Insert: Creates double tagged QinQ header. Uses info from VIF.
    XP_VLAN_Q_IN_Q_STAGGED_TOKEN_HDR_PROFILE,//Modify: Writes sTag. Insert: No OP. Uses sTag from Token.

    // v4 Unicast Routing Insert Header Profile
    XP_IPV4_UCAST_UNTAGGED_L2_HDR_PROFILE,
    XP_IPV4_UCAST_TAGGED_L2_HDR_PROFILE,
    XP_IPV4_UCAST_Q_IN_Q_TAG_L2_HDR_PROFILE,

    XP_IPV4_MCAST_UNTAGGED_L2_HDR_PROFILE,
    XP_IPV4_MCAST_TAGGED_L2_HDR_PROFILE,
    XP_IPV4_MCAST_Q_IN_Q_TAG_L2_HDR_PROFILE,

    //IP Tunnels Insert Profiles
    XP_VXLAN_INSERT_HDR_PROFILE,
    XP_NVGRE_INSERT_HDR_PROFILE,
    XP_GRE_INSERT_HDR_PROFILE,
    XP_GRE_ERSPAN2_INSERT_HDR_PROFILE,  // 16
    XP_GRE_VPN_INSERT_HDR_PROFILE,      // 17
    XP_IP_OVER_IP_INSERT_HDR_PROFILE,
    XP_GENEVE_FIXED_INSERT_HDR_PROFILE,
    XP_GENEVE_FLOW_ID_INSERT_HDR_PROFILE,
    XP_GENEVE_FLOW_TEMPLATE_ID_INSERT_HDR_PROFILE,

    //MPLS Tunnel Insert Profiles
    XP_L3VPN_INSERT_HDR_PROFILE,
    XP_IPOMPLS_INSERT_PROFILE,
    XP_MPLS_ONE_LABEL_INSERT_HDR_PROFILE,
    XP_MPLS_TWO_LABEL_INSERT_HDR_PROFILE,

    // PBB insertion profile
    XP_PBB_INSERT_HDR_PROFILE,

    XP_NOPE_INSERT_PROFILE,


    XP_IPV6_UCAST_UNTAGGED_L2_HDR_PROFILE,
    XP_IPV6_UCAST_TAGGED_L2_HDR_PROFILE,
    XP_IPV6_UCAST_Q_IN_Q_TAG_L2_HDR_PROFILE,

    XP_IPV6_MCAST_UNTAGGED_L2_HDR_PROFILE,
    XP_IPV6_MCAST_TAGGED_L2_HDR_PROFILE,
    XP_IPV6_MCAST_Q_IN_Q_TAG_L2_HDR_PROFILE,

    XP_NOPE_MODIFY_PROFILE,
    XP_ETAG_REMOVE_PROFILE,

    XP_SRV6_ORIGINATION_HDR_PROFILE,
    XP_SRV6_TERMINATION_HDR_PROFILE,
    XP_GENEVE_PACKET_TRAKKER_INSERT_HDR_PROFILE,
    XP_SRV6_TRANSIT_HDR_PROFILE,

    XP_HDR_PROFILE_TOTAL,
    XP_HDR_PROFILE_INVALID = -1
} xpHdrProfile_t;

/**
 * \public
 * \brief This type (enum) defines type of Dwrr Table.
 */
typedef enum
{
    XP_DWRR_PIPE_SCHED,
    XP_DWRR_PORT_SCHED,
    XP_DWRR_H1_SCHED,
    XP_DWRR_H2_SCHED,
    XP_DWRR_TOTAL_TYPE
} xpDwrrType_t;

/**
 * \public
 * \brief This type (enum) defines type of H1COUNTER Table.
 */
typedef enum
{
    XP_H1_LENGTH_COUNTER,
    XP_H1_PKT_CNT,
    XP_H1COUNTER_TOTAL_TYPE
} xpH1CounterType_t;

/**
 * \public
 * \brief This type (enum) defines type of H2COUNTER Table.
 */
typedef enum
{
    XP_H2_LENGTH_COUNTER,
    XP_H2_PKT_CNT,
    XP_H2COUNTER_TOTAL_TYPE
} xpH2CounterType_t;


/**
 * \brief xpSerdesEyeVbtc
 * holding vertical bathtub calculation results.
 */
typedef struct xpSerdesEyeVbtc
{
    uint32_t topIndex;         ///< Inner most top index of points
    ///<   used in extrapolation.
    uint32_t bottomIndex;      ///< Inner most bottom index of points
    ///<   used in extrapolation.
    uint32_t topPoints;        ///< Number of points used for the top side extrapolation.
    ///<   Zero if not enough.
    uint32_t bottomPoints;     ///< Number of points used for the bottom side extrapolation.
    ///<   Zero if not enough.
    uint32_t points;            ///< Number of points (on each edge) used to do
    ///<    the extrapolation.
    ///< Will be zero if not enough points.

    int32_t dataColumn;        ///< Data column index.  Is -1 if mission data.
    int32_t totalColumns;      ///< Total columns in eye data.  Used to id center.

    /* VBTC data: */
    Float dcBalance;       ///< Default is 0.5.

    Float topRj;           ///< Top estimated random jitter in mV (rms).
    Float topSlope;        ///< Top slope in mV/Q.
    Float topIntercept;    ///< Top x-intercept in Q.
    Float topRSquared;    ///< Top R-squared confidence.

    Float bottomRj;        ///< Bottom estimated random jitter in mV (rms).
    Float bottomSlope;     ///< Bottom slope in mV/Q.
    Float bottomIntercept; ///< Bottom x-intercept in Q.
    Float bottomRSquared; ///< Bottom R-squared confidence.

    double vertBer0mV;    ///< Extrapolated BER at 0 mV eye opening.
    double vertBer25mV;   ///< Extrapolated BER at 25 mV eye opening.
    Float height0mV;       ///< Q value for vert_ber_0mV.
    Float height25mV;      ///< Q value for vert_ber_25mV.

    int32_t Vmean;              ///< Eye height at Q==0, in mV.
    int32_t vertEye1e06;      ///< Eye height at 1e-06 in mV.
    int32_t vertEye1e10;      ///< Eye height at 1e-10 in mV.
    int32_t vertEye1e12;      ///< Eye height at 1e-12 in mV.
    int32_t vertEye1e15;      ///< Eye height at 1e-15 in mV.
    int32_t vertEye1e17;      ///< Eye height at 1e-17 in mV.

} xpSerdesEyeVbtc_t;

/**
 * \brief xpSerdesTxPllClk_t
 * Select the TX PLL clock source.  See firmware Clock Swap Int 0x30
 */
typedef enum
{
    SERDES_TX_PLL_REFCLK             = 0x00,    ///< Use REF clock; default.
    SERDES_TX_PLL_RX_DIVX            = 0x10,    ///< Use the recovered clock.
    SERDES_TX_PLL_OFF                = 0x30,    ///< Use no clock.
    SERDES_TX_PLL_PCIE_CORE_CLK      = 0x70,    ///< Use core clock.
    SERDES_TX_PLL_PCIE_CORE_CLK_DIV2 = 0xf0     ///< Use core clock / 2.
} xpSerdesTxPllClk_t;

/**
 * \brief xpSerdesPcsFifoClk_t
 * available serdes PCS FIFO clocks
 */
typedef enum
{
    SERDES_PCS_FIFO_F66   = 0x0000, ///< default
    SERDES_PCS_FIFO_F50   = 0x0800, ///< Various clock speeds.
    SERDES_PCS_FIFO_F60   = 0x0900, ///< TBD
    SERDES_PCS_FIFO_F70   = 0x0a00, ///< TBD
    SERDES_PCS_FIFO_F80   = 0x0b00, ///< TBD
    SERDES_PCS_FIFO_F90   = 0x0c00, ///< TBD
    SERDES_PCS_FIFO_F100  = 0x0d00, ///< TBD
    SERDES_PCS_FIFO_F110  = 0x0e00, ///< TBD
    SERDES_PCS_FIFO_F120  = 0x0f00  ///< TBD
} xpSerdesPcsFifoClk_t;

/**
 * \brief xpSerdesEncodingMode_t
 * The width mode and line encoding selections
 */
typedef enum
{
    SERDES_NRZ  = 0,    ///< Transmit using 2 levels of encoding
    SERDES_PAM2 = 0,    ///< Transmit using 2 levels of encoding
    SERDES_PAM4 = 1     ///< Transmit using 4 levels of encoding
} xpSerdesEncodingMode_t;

/**
 * \brief xpSerdesDdr3FrequencyConfig_t
 * serdes ddr3 frequency configuration
 */
typedef enum
{
    SERDES_DDR3_800_CL6   = 0,    ///< TBD
    SERDES_DDR3_1600_CL11 = 1,    ///< TBD
    SERDES_DDR3_1866_CL12 = 2,    ///< TBD
    SERDES_DDR3_1866_CL13 = 3,    ///< TBD
    SERDES_DDR3_2133_CL13 = 4,    ///< TBD
    SERDES_DDR3_2133_CL14 = 5     ///< TBD
} xpSerdesDdr3FrequencyConfig_t;

/**
 * \brief xpSerdesDdr3Parameter_t
 * serdes ddr3 parameter enum
 */
typedef enum
{
    SERDES_DDR3_PERFORM_DRAM_INIT             = 0,     ///< TBD
    SERDES_DDR3_RD_GAP1_EN                    = 1,     ///< TBD
    SERDES_DDR3_RD_GAP                        = 2,     ///< TBD
    SERDES_DDR3_WR_GAP                        = 3,     ///< TBD
    SERDES_DDR3_RD_BURST_LOOP                 = 4,     ///< TBD
    SERDES_DDR3_BANK                          = 5,     ///< TBD
    SERDES_DDR3_ROW_OFFSET                    = 6,     ///< TBD
    SERDES_DDR3_COL_OFFSET                    = 7,     ///< TBD
    SERDES_DDR3_PATTERN0                      = 8,     ///< TBD
    SERDES_DDR3_PATTERN1                      = 9,     ///< TBD
    SERDES_DDR3_PATTERN2                      = 10,    ///< TBD
    SERDES_DDR3_PATTERN3                      = 11,    ///< TBD
    SERDES_DDR3_DM_PATTERN0                   = 12,    ///< TBD
    SERDES_DDR3_DM_PATTERN1                   = 13,    ///< TBD
    SERDES_DDR3_DM_PATTERN2                   = 14,    ///< TBD
    SERDES_DDR3_DM_PATTERN3                   = 15,    ///< TBD
    SERDES_DDR3_PHY_FREQ_CONFIG               = 16,    ///< TBD
    SERDES_DDR3_PHY_T_RDLAT_OFFSET            = 17,    ///< TBD
    SERDES_DDR3_MR0_TRN_WRR                   = 18,    ///< TBD
    SERDES_DDR3_MEM_CAS_LAT                   = 19,    ///< TBD
    SERDES_DDR3_MEM_CAS_WRLAT                 = 20,    ///< TBD
    SERDES_DDR3_MAX_TIMEOUT                   = 21,    ///< TBD
    SERDES_DDR3_PBDS_ENABLE                   = 22,    ///< TBD
    SERDES_DDR3_PBDS_DELAY_INCR               = 23,    ///< TBD
    SERDES_DDR3_PBDS_PASS_COUNT               = 24,    ///< TBD
    SERDES_DDR3_PBDS_SEPERATE_READ_WRITE      = 25,    ///< TBD
    SERDES_DDR3_PBDS_DATAMASK_ENABLE          = 26,    ///< TBD
    SERDES_DDR3_DISABLE_UPPER_NIBBLE_BYTES    = 27,    ///< TBD
    SERDES_DDR3_LOOP_WRITE_READ_COMPARE_COUNT = 28,    ///< TBD
    SERDES_DDR3_RANDOMIZE_PATTERN             = 29     ///< TBD
} xpSerdesDdr3Parameter_t;


/**
 * \brief xpSerdesIpType_t
 * serdes IP type
 */
typedef enum
{
    SERDES_UNKNOWN_IP                 = 0x00,    ///< TBD
    SERDES                            = 0x01,    ///< TBD
    SERDES_SBUS_CONTROLLER            = 0x02,    ///< TBD
    SERDES_SPICO                      = 0x03,    ///< TBD
    SERDES_QPI                        = 0x04,    ///< TBD
    SERDES_FBD                        = 0x05,    ///< TBD
    SERDES_PCS64B66B                  = 0x06,    ///< TBD
    SERDES_AUTO_NEGOTIATION           = 0x07,    ///< TBD
    SERDES_PCS64B66B_FEC              = 0x08,    ///< TBD
    SERDES_PCIE_PCS                   = 0x09,    ///< TBD
    SERDES_PLL                        = 0x0a,    ///< TBD
    SERDES_PMRO                       = 0x0b,    ///< TBD
    SERDES_DDR_ADDRESS                = 0x0c,    ///< TBD
    SERDES_DDR_DATA                   = 0x0d,    ///< TBD
    SERDES_DDR_TRAIN                  = 0x0e,    ///< TBD
    SERDES_DDR_CTC                    = 0x0f,    ///< TBD
    SERDES_DDR_STOP                   = 0x10,    ///< TBD
    SERDES_THERMAL_SENSOR             = 0x11,    ///< TBD
    SERDES_RMON                       = 0x12,    ///< TBD
    SERDES_LINK_EMULATOR              = 0x13,    ///< TBD
    SERDES_AVSP_CONTROL_LOGIC         = 0x14,    ///< TBD
    SERDES_M4                         = 0x15,    ///< TBD
    SERDES_P1                         = 0x16,    ///< TBD
    SERDES_MLD                        = 0x17,    ///< TBD
    SERDES_RSFEC_BRIDGE               = 0x18,    ///< TBD
    SERDES_CROSSPOINT                 = 0x19,    ///< TBD
    SERDES_SAPPH_GBX                  = 0x20,    ///< TBD
    SERDES_SAPPH_GBX_TOP              = 0x21,    ///< TBD
    SERDES_OPAL_RSFEC528              = 0x22,    ///< TBD
    SERDES_OPAL_RSFEC528_544          = 0x23,    ///< TBD
    SERDES_OPAL_HOST_ALIGNER          = 0x24,    ///< TBD
    SERDES_OPAL_MOD_ALIGNER           = 0x25,    ///< TBD
    SERDES_OPAL_CONTROL               = 0x26,    ///< TBD

    SERDES_LINK_EMULATOR_2            = 0x81,    ///< TBD
    SERDES_SLE_PKT                    = 0x82,    ///< TBD
    SERDES_SLE                        = 0x83,    ///< TBD
    /* If new enums are added here, add them to serdes/str_conv.c as well! */

    /* The following types are not defined by their enum number: */
    /*  they're special cases that must be handled carefully -- see system.c. */

    SERDES_RAM_PMRO,    ///< TBD
    SERDES_PANDORA_LSB,    ///< TBD

    SERDES_MAX_RING_ADDRESS           = 0xdf,    ///< Last address usable in sbus ring
    SERDES_SERDES_P1_BROADCAST        = 0xed,    ///< TBD
    SERDES_SERDES_M4_BROADCAST        = 0xee,    ///< TBD
    SERDES_SERDES_D6_BROADCAST        = 0xff,    ///< TBD
    SERDES_THERMAL_SENSOR_BROADCAST   = 0xef,    ///< TBD
    SERDES_DDR_STOP_BROADCAST         = 0xf0,    ///< TBD
    SERDES_DDR_CTC_BROADCAST          = 0xf1,    ///< TBD
    SERDES_DDR_TRAIN_BROADCAST        = 0xf2,    ///< TBD
    SERDES_DDR_DATA_BROADCAST         = 0xf3,    ///< TBD
    SERDES_DDR_ADDRESS_BROADCAST      = 0xf4,    ///< TBD
    SERDES_PMRO_BROADCAST             = 0xf5,    ///< TBD
    SERDES_RESERVED_BROADCAST         = 0xf6,    ///< TBD
    SERDES_PCIE_PCS_BROADCAST         = 0xf7,    ///< TBD
    SERDES_PCS64B66B_BROADCAST        = 0xf8,    ///< TBD
    SERDES_AUTO_NEGOTIATION_BROADCAST = 0xf9,    ///< TBD
    SERDES_FBD_BROADCAST              = 0xfb,    ///< TBD
    SERDES_QPI_BROADCAST              = 0xfc,    ///< TBD
    SERDES_SPICO_BROADCAST            = 0xfd,    ///< TBD
    SERDES_BROADCAST                  = 0xff     ///< TBD
} xpSerdesIpType_t;

/**
 * \brief xpSerdesBsbClkSel_t
 * serdes BSB clock selection
 */
typedef enum
{
    SERDES_BSB_GND              = 0,    ///< TBD
    SERDES_BSB_RX_F10           = 1,    ///< TBD
    SERDES_BSB_RX_F20           = 2,    ///< TBD
    SERDES_BSB_RX_F40_FIFO_CLK  = 3,    ///< TBD
    SERDES_BSB_RX_CLK           = 4,    ///< TBD
    SERDES_BSB_RX_FIFO_CLK      = 5,    ///< TBD
    SERDES_BSB_RX_F66_CLK       = 6,    ///< TBD
    SERDES_BSB_PCS6466_FIFO_CLK = 7,    ///< TBD
    SERDES_BSB_TX_F10           = 8,    ///< TBD
    SERDES_BSB_TX_F20           = 9,    ///< TBD
    SERDES_BSB_TX_F40           = 0xa,    ///< TBD
    SERDES_BSB_TX_FIFO_CLK      = 0xb,    ///< TBD
    SERDES_BSB_SBUS_CLK         = 0xc,    ///< TBD
    SERDES_BSB_SBUS_CLK_TEST    = 0xd,    ///< TBD
    SERDES_BSB_TX_TEST_CLK      = 0xf,    ///< TBD
    SERDES_BSB_REFCLK           = 0x11,    ///< TBD
    SERDES_BSB_REFCLK_TEST      = 0x12,    ///< TBD
    SERDES_BSB_TX_DIVX_CLK      = 0x14,    ///< TBD
    SERDES_BSB_RX_DIVX_CLK      = 0x15,    ///< TBD
    SERDES_BSB_DIVX_CLK_TEST    = 0x16,    ///< TBD
    SERDES_BSB_TX_F10_CLK_VAR   = 0x17,    ///< TBD
    SERDES_BSB_TX_F20_CLK_VAR   = 0x18,    ///< TBD
    SERDES_BSB_TX_CLK_TEST      = 0x19,    ///< TBD
    SERDES_BSB_LSSEL            = 0x1b,    ///< TBD
    SERDES_BSB_RESET_COMPLETE   = 0x1c,    ///< TBD
    SERDES_BSB_RX_PI_CLK        = 0x1e,    ///< TBD
    SERDES_BSB_AVDD             = 0x1f     ///< TBD
} xpSerdesBsbClkSel_t;

/**
 * \brief xpSerdesBsbMode_t
 * various modes of serdes BSB
 */
typedef enum
{
    SERDES_BSB_DISABLE,     ///< Disables the BSB and sets passthru mode
    SERDES_BSB_CLK,         ///< send local clock select by SERDES_BSB_CLK_SEL
    SERDES_BSB_PASSTHRU,    ///< sends incoming BSB data to the next SerDes
    SERDES_BSB_DMA,         ///< send out DMA address live
    SERDES_BSB_SBUS,        ///< send out SBus address live
    SERDES_BSB_CORE         ///< send out data from ASIC core
} xpSerdesBsbMode_t;

/**
 * \brief xpSerdesTxDataSel_t
 * serdes tx data selection enum
 */
typedef enum
{
    SERDES_TX_DATA_SEL_PRBS7    = 0, ///< PRBS7 (x^7+x^6+1) generator.
    SERDES_TX_DATA_SEL_PRBS9    = 1, ///< PRBS9 (x^9+x^7+1).
    SERDES_TX_DATA_SEL_PRBS11   = 2, ///< PRBS11 (x^11+x^9+1).
    SERDES_TX_DATA_SEL_PRBS15   = 3, ///< PRBS15 (x^15+x^14+1).
    SERDES_TX_DATA_SEL_PRBS23   = 4, ///< PRBS23 (x^23+x^18+1).
    SERDES_TX_DATA_SEL_PRBS31   = 5, ///< PRBS31 (x^31+x^28+1).
    SERDES_TX_DATA_SEL_PRBS13   = 6, ///< PRBS13 (x^13+x^12+x^2+x^1+1).
    SERDES_TX_DATA_SEL_USER     = 7, ///< User pattern generator.
    SERDES_TX_DATA_SEL_CORE     = 8, ///< External data.
    SERDES_TX_DATA_SEL_LOOPBACK = 9, ///< Parallel loopback from receiver.
    SERDES_TX_DATA_SEL_PMD      = 10,///< PMD training data
    SERDES_TX_DATA_SEL_AN       = 11 ///< Auto-negotiation data
} xpSerdesTxDataSel_t;

/**
 * \brief  xpSerdesMemType_t
 * Selects memory access type.
 * \details LSB_DIRECT uses SBus transactions to read/write data,
 *          which avoids interrupting the processor, and also works if the
 *          FIRMWARE processor is not running.
 * \details ESB_DIRECT also works if the processor is not running,
 *          though it is much slower than ESB.
 */
typedef enum
{
    ESB,                  /**< Electrical Sub-Block access. */
    ESB_DIRECT,           /**< ESB access using SBus instead of an interrupt. */
    LSB,                  /**< Logical Sub-Block access. */
    LSB_DIRECT,           /**< LSB access using SBus instead of an interrupt. */
    DMEM,                 /**< Data memory access. */
    DMEM_PREHALTED,       /**< Data memory access, processor already halted. */
    IMEM,                 /**< Instruction memory access. */
    IMEM_PREHALTED,       /**< Instruction memory access, processor already halted. */
    BIG_REG_16,     /**< Access 16 bit big register reads/writes */
    BIG_REG_32      /**< Access 32 bit big register reads/writes */
} xpSerdesMemType_t;

/**
 * \brief xpSerdesClk_t
 * Select the clock source.  See Firmware Clock Swap Int 0x30.
 */
typedef enum
{
    SERDES_REFCLK                = 0, ///< default
    SERDES_PCIE_CORE_CLK         = 1, ///< PCIe core clock
    SERDES_TX_F10_CLK_FIXED      = 2, ///< TBD
    SERDES_TX_F40_CLK_FIXED      = 3, ///< TBD
    SERDES_REFCLK_DIV2           = 4, ///< TBD
    SERDES_PCIE_CORE_CLK_DIV2    = 5, ///< PCIe core clock div2
    SERDES_TX_F10_CLK_FIXED_DIV2 = 6, ///< TBD
    SERDES_TX_F40_CLK_FIXED_DIV2 = 7, ///< TBD
    SERDES_XP_TX_DATA_CLK        = 2,   /**< P1 only */
    SERDES_XP_TX_DATA_CLK_DIV2   = 6,   /**< P1 only */
    SERDES_XP_TX_F20_CLK_FIXED   = 0xe,
    SERDES_XP_TX_F80_CLK_FIXED   = 0xf
} xpSerdesClk_t;

/**
 * \brief xpSerdesRxCmpMode_t
 * Receiver data comparison mode.
 */
typedef enum
{
    /* 0-rx_data, 1=rx_offset, 2=prbs, 3=tx_data, 4=tx_prbs, 6=0, 7=fffff */
    /*      S=source, C=compare               SC */
    SERDES_RX_CMP_MODE_OFF         = 0x0000, ///< Turn Off
    SERDES_RX_CMP_MODE_XOR         = 0x0100, ///< Test^Mission
    SERDES_RX_CMP_MODE_TEST_PATGEN = 0x0120, ///< Test^PatGen.
    SERDES_RX_CMP_MODE_MAIN_PATGEN = 0x0200  ///< Mission^PatGen
} xpSerdesRxCmpMode_t;

/**
 * \brief xpSerdesRxDataQual_t
 * Data qualification filter for purposes of eye measurement.
 */
typedef enum
{
    /* Note: Partial eyes are listed first and ordered to match */
    /*   vert_alpha[] array elements [0..5]. */
    SERDES_RX_DATA_QUAL_UNQUAL= 0,     ///< Select all bits
    SERDES_RX_DATA_QUAL_DEFAULT=0x0100,///< prev0 + prev1
    SERDES_RX_DATA_QUAL_UNLOCK =0x0101,///< Release counter resource, reset to prev_bits.
    SERDES_RX_DATA_QUAL_EVEN  = 0x4000,///< Select only even bits
    SERDES_RX_DATA_QUAL_ODD   = 0x8000,///< Select only odd bits
    SERDES_RX_DATA_QUAL_PREV0 = 0x2000,///< Only select bits following a 0.
    SERDES_RX_DATA_QUAL_PREV1 = 0x2800,///< Only select bits following a 1.
    SERDES_RX_DATA_QUAL_PREV0E= 0x6000,///< Only select bits following a 0 from even channel.
    SERDES_RX_DATA_QUAL_PREV1E= 0x6800,///< Only select bits following a 1 from even channel.
    SERDES_RX_DATA_QUAL_PREV0O= 0xa000,///< Only select bits following a 0 from odd channel.
    SERDES_RX_DATA_QUAL_PREV1O= 0xa800 ///< Only select bits following a 1 from odd channel.
} xpSerdesRxDataQual_t;

/**
 * \brief xpSerdesRxTerm_t
 * Specify the RX termination value.
 */
typedef enum
{
    SERDES_RX_TERM_AGND = 0,    ///< Resistor to ground
    SERDES_RX_TERM_AVDD = 1,    ///< Resistor to AVDD
    SERDES_RX_TERM_FLOAT = 2    ///< No resistor.
} xpSerdesRxTerm_t;

/**
 * \struct xpSerdesTxEqLimits_t
 * \brief  The minimum and maximum supported values for the transmitter,equalization settings
 *      Note that values are device type specific.
 */
typedef struct
{
    int32_t pre_min;     ///< Minimum pre-cursor value.
    int32_t pre_max;     ///< Maximum pre-cursor value.
    int32_t atten_min;   ///< Minimum attenuator value.
    int32_t atten_max;   ///< Maximum attenuator value.
    int32_t post_min;    ///< Minimum post-cursor value
    int32_t post_max;    ///< Maximum post-cursor value
    int32_t total_eq;    ///< Maximum total equalization allowed: sum(abs(pre),abs(atten),abs(post))
} xpSerdesTxEqLimits_t;

/**
 * \struct xpSerdesTxEq_t
 * \brief The TX equalization settings:
 */
typedef struct
{
    short pre;            ///< Pre-cursor setting
    short atten;          ///< Attenuator setting
    short post;           ///< Post-cursor setting
    short slew;           ///< Slew rate setting [0..3], 0 is fastest
    short preLsb;        /**< Pre-cursor LSB setting - valid only for CM4/OM4 */
    short preMsb;        /**< Pre-cursor MSB setting - valid only for CM4/OM4 */
    short attenLsb;      /**< Attenuator LSB setting - valid only for CM4/OM4 */
    short attenMsb;      /**< Attenuator MSB setting - valid only for CM4/OM4 */
    short postLsb;       /**< Post-cursor LSB setting - valid only for CM4/OM4 */
    short postMsb;       /**< Post-cursor MSB setting - valid only for CM4/OM4 */
    short pre2Lsb;       /**< Pre-cursor LSB setting - valid only for CM4 */
    short pre2Msb;       /**< Pre-cursor MSB setting - valid only for CM4 */
    short amp;            /**< AMP setting valid only for HVD6 */
    short t2;             /**< T2 setting - valid only for OM4 */
} xpSerdesTxEq_t;
/**
 * \brief xpSerdesRxCmpData_t
 * Specify the pattern to use in validating the received data.
 */
typedef enum
{
    SERDES_RX_CMP_DATA_PRBS7    = 0, ///< PRBS7 (x^7+x^6+1) generator.
    SERDES_RX_CMP_DATA_PRBS9    = 1, ///< PRBS9 (x^9+x^7+1).
    SERDES_RX_CMP_DATA_PRBS11   = 2, ///< PRBS11 (x^11+x^9+1).
    SERDES_RX_CMP_DATA_PRBS15   = 3, ///< PRBS15 (x^15+x^14+1).
    SERDES_RX_CMP_DATA_PRBS23   = 4, ///< PRBS23 (x^23+x^18+1).
    SERDES_RX_CMP_DATA_PRBS31   = 5, ///< PRBS31 (x^31+x^28+1).
    SERDES_RX_CMP_DATA_PRBS13   = 6, ///< PRBS13 (x^13+x^12+x^2+x^1+1).
    SERDES_RX_CMP_DATA_SELF_SEED= 7, ///< Auto-seed to received 40 bit repeating pattern.
    ///< NOTE: This is USER mode in firmware.
    SERDES_RX_CMP_DATA_OFF      = 8  ///< Disable cmp data generator
} xpSerdesRxCmpData_t;

/**
 * \brief Structure for holding the horizontal bathtub calculation results
 */
typedef struct xpSerdesEyeHbtc
{
    uint32_t leftIndex;        ///< Inner most left index of points
    ///<   used in extrapolation.
    uint32_t rightIndex;       ///< Inner most right index of points
    ///<   used in extrapolation.
    uint32_t leftPoints;      ///< Number of points used for the left side extrapolation.
    ///< Zero if not enough.
    uint32_t rightPoints;      ///< Number of points used for the right side extrapolation.
    ///< Zero if not enough.
    uint32_t points;            ///< Number of points (on each side) used to do
    ///<    the extrapolation.
    ///< Will be zero if not enough points.

    uint32_t dataRow;          ///< Data row index.
    uint32_t totalRows;        ///< Total rows in eye data. Used to id center.

    /* HBTC data: */
    Float transDensity;    ///< Default is 0.5.

    Float dj;               ///< Estimated DJpp in mUI (milliUI).
    Float leftRj;          ///< Left estimated random jitter in mUI (rms).
    Float leftSlope;       ///< Left slope in Q/UI.
    Float leftIntercept;   ///< Left y-intercept in Q.
    Float leftRSquared;   ///< Left R-squared confidence.

    Float rightRj;         ///< Right estimated random jitter in mUI (rms).
    Float rightSlope;      ///< Right slope in Q/UI.
    Float rightIntercept;  ///< Right y-intercept in Q.
    Float rightRSquared;  ///< Right R-squared confidence.

    int32_t horzEye1e06;      ///< Eye width at 1e-06 in mUI.
    int32_t horzEye1e10;      ///< Eye width at 1e-10 in mUI.
    int32_t horzEye1e12;      ///< Eye width at 1e-12 in mUI.
    int32_t horzEye1e15;      ///< Eye width at 1e-15 in mUI.
    int32_t horzEye1e17;      ///< Eye width at 1e-17 in mUI.

    Float width0mUI;       ///< Q value for horz_ber_0mUI.
    Float width100mUI;     ///< Q value for horz_ber_100mUI.
    double horzBer0mUI;   ///< Extrapolated BER at 0 mUI eye opening.
    double horzBer100mUI; ///< Extrapolated BER at 100 mUI eye opening.

} xpSerdesEyeHbtc_t;

/**
 * \brief xpSerdesLogType_t is used for the serdes struct log buffers
 */
typedef enum
{
    XP_SERDES_DEBUG0 = 0,     ///< TBD
    XP_SERDES_DEBUG1,     ///< TBD
    XP_SERDES_DEBUG2,     ///< TBD
    XP_SERDES_DEBUG3,     ///< TBD
    XP_SERDES_DEBUG4,     ///< TBD
    XP_SERDES_DEBUG5,     ///< TBD
    XP_SERDES_DEBUG6,     ///< TBD
    XP_SERDES_DEBUG7,     ///< TBD
    XP_SERDES_DEBUG8,     ///< TBD
    XP_SERDES_DEBUG9,     ///< TBD
    XP_SERDES_MEM_LOG,    ///< TBD
    XP_SERDES_DATA_CHAR,  ///< TBD
    XP_SERDES_DATA_CHAR_ADD,  ///< TBD
    XP_SERDES_ERR,            ///< TBD
    XP_SERDES_WARNING,        ///< TBD
    XP_SERDES_INFO    ///< TBD
} xpSerdesLogType_t;
/**
 * \brief xpSerdesDfeMode_t
 * Controls which Rx DFE settings are updated.
 */
typedef enum
{
    DFE_MODE_CTLE,          ///< Update DC,LF,HF,BW
    DFE_MODE_DFE,           ///< Update dfe
    DFE_MODE_VOS,           ///< Update vos
    DFE_MODE_CTLE_DFE,      ///< Update DC,LF,HF,BW,dfeTap
    DFE_MODE_CTLE_DFE_VOS,  ///< Update DC,LF,HF,BW,dfe*,vos
    DFE_MODE_PARAM,         ///< Update Parameters
    DFE_MODE_TESTLEV,       ///< Update testLEV, used by eye plotting
    DFE_MODE_DATALEV,       ///< Update dataLEV, used by eye plotting
    DFE_MODE_ALL            ///< Update entire struct
} xpSerdesDfeMode_t;

/**
 * \public
 * \brief This type (enum) defines type of PORTCOUNTER Table.
 */
typedef enum
{
    XP_PORT_LENGTH_COUNTER,
    XP_PORT_PKT_CNT,
    XP_PORTCOUNTER_TOTAL_TYPE
} xpPortCounterType_t;

/**
 * \brief xpResetPortLevel
 * Various MAC Reset level/options
 */
typedef enum xpResetPortLevel
{
    SERDES_RESET = 0,       ///< To RESET the serdes channels for ports
    MAC_RESET,              ///< To RESET the MAC for respective device
    PCS_RESET,              ///< To RESET the PCS for respective device
    ALL_RESET               ///< To RESET All component
} xpResetPortLevel;

/**
 * \public
 * \brief This type (enum) defines type of Shapers Table.
 */
typedef enum
{
    XP_SHAPERS_PIPE_CB_FAST_UPD,
    XP_SHAPERS_PORT_CB_FAST_UPD,
    XP_SHAPERS_H1_CB_FAST_UPD,
    XP_SHAPERS_H1_CB_SLOW_UPD,
    XP_SHAPERS_H2_CB_FAST_UPD,
    XP_SHAPERS_H2_CB_SLOW_UPD,
    XP_SHAPERS_TOTAL_TYPE
} xpShapersType_t;

/**
 * \public
 * \brief This type (enum) defines type of FastShapers Mapping.
 */
typedef enum
{
    XP_FASTSHAPERS_FAST_UPD_IDX_TO_Q,
    XP_FASTSHAPERS_FAST_UPD_IDX_TO_PORT,
    XP_FASTSHAPERS_FAST_UPD_IDX_TO_H1,
    XP_FASTSHAPERS_FAST_UPD_IDX_TO_H2,
    XP_FASTSHAPERS_TOTAL_TYPE
} xpFastShapersMappingType_t;

/**
 * \private
 * \brief xpAcmBankPackerType_t
 * This type (enum) defines instances of packer in a device.
 *
 */
typedef enum
{
    ACMBANK_DEV_PACKER_TYPE0,    // Counter Mode A
    ACMBANK_DEV_PACKER_TYPE1,    // Counter Counter Mode B
    ACMBANK_DEV_PACKER_TYPE2,    // Counter Mode C
    ACMBANK_DEV_PACKER_TYPE3,    // Counter Mode D
    ACMBANK_DEV_PACKER_TYPE4,    // Policer counter bank in Mode PC
    ACMBANK_DEV_PACKER_TYPE5,    // Policer bank either in mode P or mode PC
    ACMBANK_DEV_PACKER_TYPE6,    // sampling

    ACMBANK_DEV_PACKER_NUM
} xpAcmBankPackerType_t;
/**
 * \brief XP_ACM_RANGE_TYPE
 *
 */
typedef enum
{
    XP_ACM_RANGE_NONE,
    XP_ACM_RANGE_STATIC,
    XP_ACM_RANGE_DYNAMIC
} XP_ACM_RANGE_TYPE;

typedef enum xpPolicerStandard_e
{
    POL_STANDARD_RFC2698 = 0,
    POL_STANDARD_MEF10P1 = 1,
    POL_STANDARD_MAX
} xpPolicerStandard_e;

typedef enum xpPolicerType_e
{
    POL_TYPE_BYTES = 0,
    POL_TYPE_PACKETS = 1,
    POL_TYPE_MAX
} xpPolicerType_e;

#ifdef __cplusplus
/**
 * \brief xpRsnCode_t
 * Remove ifdef once enum is filled
 *
 */
typedef enum
{} xpRsnCode_t;
#endif

/**
 * \brief xpLdeTblType_t
 * tbl type enum
 */
typedef enum
{
    KFIT = 0,
    OFIT,
    TEMPLATE_TBL,
    PROFILE_TBL,
    NEXTFUNC_TBL,
    LDE_TBL_TYPE_MAX
} xpLdeTblType_t;


/**
 * \public
 * \brief xpFdbTblEntryType_e
 * This type defines the type of an FDB entry(static / dynamic).
 */
typedef enum
{
    XP_FDB_ENTRY_TYPE_DYNAMIC,
    XP_FDB_ENTRY_TYPE_STATIC,
    XP_FDB_ENTRY_TYPE_ALL,
} xpFdbTblEntryType_e;

/**
 * \public
 * \brief xpFdbEntryAction_e
 * This type defines FDB entry access type.
 */
typedef enum xpFdbEntryAction_e
{
    XP_FDB_ENTRY_NO_ACTION,
    XP_FDB_ENTRY_ADD,
    XP_FDB_ENTRY_MODIFY,
    XP_FDB_ENTRY_REMOVE,
} xpFdbEntryAction_e;

/**
 * \public
 * \brief xpFdbFlushType_e
 * This type defines FDB Flush Type.
 */
typedef enum xpFdbFlushType_e
{
    XP_FDB_FLUSH_ALL,
    XP_FDB_FLUSH_BY_INTF,
    XP_FDB_FLUSH_BY_VLAN,
    XP_FDB_FLUSH_BY_INTF_VLAN,
} xpFdbFlushType_e;

/**
 * \public
 * \brief xpVlanRouteMcMode_t
 * This enum defines an enumeration of configurable modes
 *        of multicast routing per vlan.
 */
typedef enum
{
    MC_ROUTE_MODE_S_G_V = 0,
    MC_ROUTE_MODE_0_G_V,
    MC_ROUTE_MODE_MAX
} xpVlanRouteMcMode_t;

/**
 * \public
 * \brief xpVlanNatMode_e
 * This type defines an enumeration of all the NAT modes
 *        configurable on a vlan instance.
 */
typedef enum
{
    NAT_DISABLE = 0,    ///< NAT Disabled
    NAT_IP_ONLY,        ///< Translate SIP or DIP
    NAT_IP_PORT,        ///< Translate SIP or DIP and port
    NAT_SIP_AND_DIP,    ///< Translate SIP & DIP
    NAT_MAX
} xpVlanNatMode_e;

/**
 * \public
 * \brief xpVlanEncap_t
 *        This enumeration provides all basic VLAN encapsulation
 *        for a port or a lag that can be added to a vlan
 *        context.
 */
typedef enum
{
    UNTAG = 0,
    TAG,
    PRI_TAG,
    QINQ,
    STRIP_OUTER,
    ENCAP_MAX
} xpVlanEncap_t;

/**
 * \public
 * \brief xpProfileEncapType_e
 * This type defines an enumeration of encap types used in the profile pipeline.
 */
typedef enum
{
    XP_PROFILE_ENCAP_IP_PBB,
    XP_PROFILE_ENCAP_UN_SINGLE_TAG,
    XP_PROFILE_ENCAP_MPLS,
    XP_PROFILE_ENCAP_DOUBLE_TAG,
    XP_PROFILE_ENCAP_DOUBLE_MAX
} xpProfileEncapType_e;

/**
 * \public
 * \brief xpUrpfMode_e
 * This Enum type represents URPF Check Types over VRF
 */
typedef enum
{
    XP_URPF_MODE_LOOSE,
    XP_URPF_MODE_STRICT,
    XP_URPF_MODE_MAX
} xpUrpfMode_e;


/**
 * \brief xpL3EncapType_e
 * This type defines an enumeration of possible egress encapsulation
 * types for an L3 Domain.
 */
typedef enum xpL3EncapType_e
{
    XP_L3_ENCAP_UNTAGGED_PORT = 0,
    XP_L3_ENCAP_TAGGED_PORT,
    XP_L3_ENCAP_Q_IN_Q_PORT,
    XP_L3_ENCAP_MPLS_TUNNEL,
    XP_L3_ENCAP_L3VPN_TUNNEL,
    XP_L3_ENCAP_GRE_TUNNEL,
    XP_L3_ENCAP_VXLAN_TUNNEL,
    XP_L3_ENCAP_NVGRE_TUNNEL,
    XP_L3_ENCAP_IP_OVER_IP_TUNNEL,
    XP_L3_ENCAP_GENEVE_TUNNEL,
    XP_L3_ENCAP_PBB_TUNNEL,
    XP_L3_ENCAP_L3VPN_GRE_TUNNEL,
    XP_L3_ENCAP_SRV6_TUNNEL,
    XP_L3_ENCAP_INVALID
} xpL3EncapType_e;


/**
 * \public
 * \brief This type (enum) defines the validity of TCAM entries for display table.
 *
 **/
typedef enum
{
    XP_TCAM_DISP_ENTRY_INVALID,
    XP_TCAM_DISP_ENTRY_VALID,
    XP_TCAM_DISP_ENTRY_ALL,
} xpTcamDisplayEntryFlag_e;

/**
 * \public
 * \brief This type (enum) defines the validity of Hash entries for display table.
 */
typedef enum
{
    XP_HASH_DISP_ENTRY_INVALID,
    XP_HASH_DISP_ENTRY_VALID,
    XP_HASH_DISP_ENTRY_ALL,
} xpHashDisplayEntryFlag_e;
/**
 * \public
 * \brief This type (enum) defines type of Vif Table.
 */
typedef enum
{
    XP_VIF_SINGLE_PORT,
    XP_VIF_SINGLE_PORT_VLAN,
    XP_VIF_EXTENDED_PORT,
    XP_VIF_LAG,
    XP_VIF_TUNNEL_ENCAP,
    XP_VIF_MULTI_PORT,
    XP_VIF_SEGMENT_ROUTE,
    XP_VIF_TOTAL_TYPE,
} xpVifType_t;

/**
 * \public
 * \brief This type (enum) defines the type of mirroring in xpMdtMirrorEntryData_t.
 */
typedef enum
{
    XP_MIRROR_LOCAL     = 0,
    XP_MIRROR_ERSPAN2   = 1,
    XP_MIRROR_RSPAN     = 2,
    XP_MTU_MIRROR       = 3,
    XP_MIRROR_PT        = 4,  // Used for Packet trakker Mirroring
    XP_MIRROR_TOTAL_TYPE,
} xpMirrorType_e;

/**
 * \public
 * \brief xpInsertionType_t
 * This type (enum) defines type of Insertion Table.
 *
 **/
typedef enum
{
    XP_INSERTION0,
    XP_INSERTION1,
    XP_INSERTION2,
    XP_INSERTION3,
    XP_INSERTION_TOTAL_TYPE,
} xpInsertionType_t;

/**
 * \brief xpBdRange_t
 */
typedef enum
{
    XP_VLAN_BD_RANGE_T,
    XP_NON_VLAN_BD_RANGE_T,
    XP_MAX_BD_RANGE_T
} xpBdRange_t;


/**
 * \public
 * \brief xpQosMapPflType_t
 * This type (enum) is used to differentiate between
 * different types of QOS Profiles
 */
typedef enum
{
    L2QOS_PROFILE,
    L3QOS_PROFILE,
    MPLSQOS_PROFILE,
    MAXQOS_PROFILE
} xpQosMapPflType_t;

/**
 * \public
 * \brief xpPlHashDynamicTblList
 * This type (enum) defines the list of dynamic tables.
 *
 */

typedef enum
{
    FDB_TBL,
    PORTVLANIVIF_TBL,
    TNL_IVIF_TBL,
    MPLS_LABEL_TBL,
    IPV4HOST_TBL,
    IPV6HOST_TBL,
    IPV4BRIDGEMC_TBL,
    IPV6BRIDGEMC_TBL,
    IPV4ROUTEMC_TBL,
    IPV6ROUTEMC_TBL,
    IPV4PIMBIDIRRPF_TBL,
    IPV6PIMBIDIRRPF_TBL,
    REMOTE_VTEP_TNL_ID_TBL,
    OPENFLOW0_TBL,
    OPENFLOW1_TBL,
    OPENFLOW_MPLS0_TBL,
    OPENFLOW_MPLS1_TBL,
    EGR_COS_MAP_TBL,
    COPP_TBL,
    INT_TBL,
    MY_SID_TBL,
    IPV4BRIDGEXGMC_TBL,
    IPV6BRIDGEXGMC_TBL,
    IPV4ROUTEXGMC_TBL,
    IPV6ROUTEXGMC_TBL,
    LOCAL_VTEP_TBL,
    HASH_DYNAMIC_TBL_NUM
} xpPlHashDynamicTblList;

/**
 * \public
 * \brief xpPureOfPlHashDynamicTblList
 * This type (enum) defines the list of dynamic tables.
 *
 */

typedef enum
{
    PURE_OPENFLOW0_TBL,
    PURE_OPENFLOW1_TBL,
    PURE_OPENFLOW2_TBL,
    PURE_OPENFLOW3_TBL,
    PURE_OF_HASH_DYNAMIC_TBL_NUM
} xpPureOfPlHashDynamicTblList;

/**
 * \brief xpPlDaDynamicTblList
 *
 */
typedef enum
{
    EVIF32_TBL,
    IVIF32_TBL,
    EVIF64_TBL,
    IVIF64_TBL,
    EVIF128_TBL,
    IVIF128_TBL,
    EVIF256_TBL,
    IVIF256_TBL,
    INS0_TBL,
    INS1_TBL,
    INS2_TBL,
    INS3_TBL,
    MDT_TBL,
    EGRESS_BD_TBL,
    IVIF_PORTBITMAP_PTR_TBL,
    EVIF_PORTBITMAP_PTR_TBL,
    DA_DYNAMIC_TBL_NUM
} xpPlDaDynamicTblList;

/**
 * \brief xpPlTcamDynamicTblList
 *
 */
typedef enum
{
    IACL0_TBL,
    IACL1_TBL,
    IACL2_TBL,
    IACL3_TBL,
    IACL4_TBL,
    IACL5_TBL,
    NATIPV4_TBL,
    EACL0_TBL,
    OF0_TBL,
    OF1_TBL,
    OF_MPLS0_TBL,
    OF_MPLS1_TBL,
    TCAM_DYNAMIC_TBL_NUM
} xpPlTcamDynamicTblList;



typedef enum
{
    BD_TBL,
    MPLS_TNL_TBL,
    ISME_DYNAMIC_TBL_NUM
} xpPlIsmeDynamicTblList;

/**
 * \brief xpPlLpmDynamicTblList
 *
 */
typedef enum
{
    LPM_V4_TBL,
    LPM_V6_TBL,
    LPM_DYNAMIC_TBL_NUM
} xpPlLpmDynamicTblList;

/**
 * \brief xpPlNhDynamicTblList
 *
 */
typedef enum
{
    NH_TBL,
    NH_DYNAMIC_TBL_NUM
} xpPlNhDynamicTblList;

typedef enum
{
    WCM_TBL_1,
    WCM_TBL_2,
    WCM_DYNAMIC_TBL_NUM
} xpPlWcmDynamicTblList;

/**
 * \enum xpPortMode
 * \brief Various Port Modes
 *
 */
typedef enum xpPortMode
{
    MODE_SGMII = 0,
    MODE_1000BASE_R,
    MODE_1000BASE_X,
    MODE_10G,
    MODE_40GX1,
    MODE_40GX2,
    MODE_100GX1,
    MODE_25GX4,
    MODE_50GX2,
    MODE_MAX
} xpPortMode;

/**
 * \brief xpMacLoopbackLevel
 * Various MAC Loop back level/options
 *
 */
typedef enum xpMacLoopbackLevel
{
    TXTORX_SGMII_SERDES_INTERFACE_LOOPBACK = 0, // Available in A0
    RXTOTX_GMII_INTERFACE_LOOPBACK,             // Available in A0
    MAC_LEVEL_LOOPBACK,          // Loopback at the mac block. pcs block would be excluded withh LB. present in A0 and B0
    MAC_FIFO_INTERFACE_LOOPBACK, // Loopback at fifo interface. present in A0 and B0
    MAC_PCS_LEVEL_LOOPBACK,      // Loopback at pcs-serdes interface. serdes mux would be excluded. present in A0 and B0
    MAC_MAX_LOOPBACK_MODES
} xpMacLoopbackLevel;

/**
 * \brief xpPlTcamDynamicTblList
 *
 */
typedef enum
{
    PURE_OF_0,
    PURE_OF_1,
    PURE_OF_2,
    PURE_OF_3,
    PURE_OF_TCAM_DYNAMIC_TBL_NUM
} xpPureOfPlTcamDynamicTblList;


/**
 * \brief xpPortEnable
 * Various configuration options for enabling/disabling the port
 *
 */
typedef enum xpPortEnable
{
    RXTX_ENABLE = 0,
    RXTX_DISABLE,
    RX_ENABLE,
    TX_ENABLE,
    RX_DISABLE,
    TX_DISABLE
} xpPortEnable;

/**
 * \brief PCS Data Patterns
 *
 */
typedef enum xpPCSDataPattern
{
    LOCAL_FAULT_DATA_PATTERN = 0,
    ZERO_DATA_PATTERN
} xpPCSDataPattern;

/**
 * \brief xpMacTxFaultGen
 */
typedef enum xpMacTxFaultGen
{
    TX_LOCAL_FAULT = 0,
    TX_REMOTE_FAULT,
} xpMacTxFaultGen;

/**
 * \brief xpMacRxFault
 * Enum for local/remote fault
 */
typedef enum xpMacRxFault
{
    RX_LOCAL = 0,
    RX_REMOTE,
    RX_OK
} xpMacRxFault;

/**
 * \brief xpPreambleLenBytes
 * Various options for setting Preamble Length bytes
 *
 */
typedef enum xpPreambleLenBytes
{
    PREAMBLE_LEN_BYTES_8 = 0x000,
    PREAMBLE_LEN_BYTES_7 = 0x007,
    PREAMBLE_LEN_BYTES_6 = 0x006,
    PREAMBLE_LEN_BYTES_5 = 0x005,
    PREAMBLE_LEN_BYTES_4 = 0x004,
    PREAMBLE_LEN_BYTES_3 = 0x003,
    PREAMBLE_LEN_BYTES_2 = 0x002,
    PREAMBLE_LEN_BYTES_1 = 0x001,
    PREAMBLE_LEN_BYTES_MAX_VAL = 0x008
} xpPreambleLenBytes;
/**
 * \brief xpRxPreambleLenBytes
 * Various options for setting Rx Preamble Length bytes
 *
 */
typedef enum xpRxPreambleLenBytes
{
    PREAMBLE_RX_LEN_BYTES_8 = 0x000,
    PREAMBLE_RX_LEN_BYTES_4 = 0x001,
    PREAMBLE_RX_LEN_BYTES_MAX_VAL = 0x002
} xpRxPreambleLenBytes;
/**
 * \brief xpVlanCheckMode
 * VLAN Check modes
 *
 */
typedef enum xpVlanCheckMode
{
    MAXFRMSIZE_WITHOUT_VLAN = 0,
    MAXFRMSIZE_WITH_VLAN_TAG1,
    MAXFRMSIZE_WITH_VLAN_TAG2,
    MAXFRMSIZE_WITH_VLAN_TAG3
} xpVlanCheckMode;
/**
 * \brief xpPCSTestPattern
 * PCS Test Patterns
 *
 */
typedef enum xpPCSTestPattern
{
    PSEUDO_RANDOM_TEST_MODE = 0,
    SQUARE_WAVE_TEST_PATTERN
} xpPCSTestPattern;
/**
 * \brief xpPCSEnableTestMode
 * PCS test Modes
 *
 */
typedef enum xpPCSEnableTestMode
{
    RX_TEST_MODE_ENABLE = 0,
    RX_TEST_MODE_DISABLE,
    TX_TEST_MODE_ENABLE,
    TX_TEST_MODE_DISABLE
} xpPCSEnableTestMode;
/**
 * \brief xpDecodeTrap
 * TBD
 *
 */
typedef enum xpDecodeTrap
{
    C_TYPE = 0,
    S_TYPE,
    T_TYPE,
    D_TYPE,
    E_TYPE,
    TRAP_REMOTE,
    TRAP_LOCAL,
    NO_TRAP
} xpDecodeTrap;

/**
 * \brief XP_MCAST_DOMAIN_ID_TYPE
 */
typedef enum
{
    XP_MCAST_DOMAIN_TYPE_NONE,
    XP_MCAST_DOMAIN_TYPE_VLAN,    // for VLAN Snooping
    XP_MCAST_DOMAIN_TYPE_VRF,    // for VRF Routing
    XP_MCAST_DOMAIN_TYPE_MAX_NUM
} XP_MCAST_DOMAIN_ID_TYPE;

typedef enum
{
    XP_VIF_PORTBITMAPPTR_TYPE_NONE,
    XP_VIF_PORTBITMAPPTR_TYPE_IVIF, // IVIF PortbitmapPointer Table
    XP_VIF_PORTBITMAPPTR_TYPE_EVIF, // EVIF PortbitmapPointer Table
    XP_VIF_PORTBITMAPPTR_TYPE_MAX_NUM
} XP_VIF_PORTBITMAPPTR_TABLE_TYPE;


/**
 * \brief xpBackPlaneAbilityModes
 * Various BPAN ability modes
 *
 */
typedef enum xpBackPlaneAbilityModes
{
    /* Don't add enums above this */
    MODE_1000BASE_KX = 0,
    MODE_10GBASE_KX4,
    MODE_10GBASE_KR,
    MODE_40GBASE_KR4,
    MODE_40GBASE_CR4,
    MODE_100GBASE_CR10,
    MODE_100GBASE_KP4,
    MODE_100GBASE_KR4,
    MODE_100GBASE_CR4
    /* Don't add enums above this */
} xpBackPlaneAbilityModes;

/**
 * \brief xpPcsEventType
 * PCS event source identifiers
 *
 */
typedef enum xpPcsEventType
{
    BLOCK_LOCK_CHANGE = 0,
    ALIGNMENT_LOCK_CHANGE,
    FAULT,
    DESKEW_FIFO_OVERFLOW,
    TX_GEARBOX_ERROR,
    RX_DECODE_TRAP,
    AN_DONE,
    AN_SYNC_STATUS
} xpPcsEventType;
/**
 * \brief xpAllocatorObjectId
 */
typedef enum xpAllocatorObjectId
{
    XP_ALLOCATOR_OID_STARTS = 0,
    XP_MDT_ALLOCATOR_OID = 1,
    XP_ANALYZER_ALLOCATOR_OID = 2,
    XP_BD_ALLOCATOR_OID1 = 3,
    XP_BD_ALLOCATOR_OID2 = 4,
    XP_ACM_ALLOCATOR_OID1 = 5,
    XP_EGRESS_FLT_ALLOCATOR_OID = 6,
    XP_NEXTHOP_ALLOCATOR_OID = 7,
    XP_NEXTHOP_GRP_ALLOCATOR_OID = 8,
    XP_SE_INST_ALLOCATOR_OID1 = 9,
    XP_SE_INST_ALLOCATOR_OID2 = 10,
    XP_SE_INST_ALLOCATOR_OID3 = 11,
    XP_SE_INST_ALLOCATOR_OID4 = 12,
    XP_VIF_ALLOCATOR_OID1 = 13,
    XP_VIF_ALLOCATOR_OID2 = 14,
    XP_VIF_ALLOCATOR_OID3 = 15,
    XP_VIF_ALLOCATOR_OID4 = 16,
    XP_MIRROR_SESSION_ALLOCATOR_OID = 17,
    XP_ACM_ALLOCATOR_ACM_RANGE_OID_START = 256,
    XP_ACM_ALLOCATOR_ACM_RANGE_OID_END = 511,
    XP_NHI_ALLOCATOR_OID_START = 768,
    XP_NHI_ALLOCATOR_OID_END = 1023,
    XPS_STATE_GLOBAL_DB_BUCKET_OID = 1024,
    XPS_STATE_PERDEV_DB_BUCKET_OID = 1025,
    XP_ALLOCATOR_TCAM_MGR_OID = 1027,
    XP_ALLOCATOR_TCAM_MGR_OID_END = 1030,
    XP_SFLOW_ALLOCATOR_OID = 1031,
    XP_ALLOCATOR_BR_VIF_OID = 1032,
    XP_ALLOCATOR_MCASTL2DOMAINID_OID_START = 1033,
    XP_ALLOCATOR_MCASTL2DOMAINID_OID_END = 1035,
    XP_ALLOCATOR_ACL_TABLE_OID = 1036,
    XP_ALLOCATOR_ACL_COUNTER_OID_START = 1037,
    XP_ALLOCATOR_ACL_COUNTER_OID_END = 1041,
    XP_EACL_TABLE_OID = 1042,
    XP_IACL_TABLE_OID = 1043,
    XP_OF_TABLE_OID = 1044,
    XP_PL_MGR_ALLOCATOR_OID = 1049,
    XPS_SCOPE_ARRAY_OID = 1050,
    XP_ALLOCATOR_ACL_ENTRY_OID = 1057,
    XP_ALLOCATOR_ACL_OID = 1059,
    XP_ALLOCATOR_AN_INFO = 1060,
    XP_ALLOCATOR_VIF_PORTBITMAPPTR_OID_START = 1061,
    XP_ALLOCATOR_VIF_PORTBITMAPPTR_OID_END = 1062,
    XP_ALLOCATOR_AC_OID = 1063,
    XP_ALLOCATOR_PACL_ID_OID = 1064,
    XP_ALLOCATOR_BACL_ID_OID = 1065,
    XP_ALLOCATOR_RACL_ID_OID = 1066,
    XP_ALLOCATOR_PORT_AUTO_TUNE_INFO = 1068,
    XP_ALLOCATOR_DEVICE_PTG_READ_ID = 1069,
    XP_ALLOCATOR_DEVICE_TYPE_SAL_ID = 1070,
    XP_USR_DEFINED_ALLOCATOR_OID_START = 2048,
    XP_USR_DEFINED_ALLOCATOR_OID_END = 3071,
    XP_USR_DEFINED_MEM_POOL_OID_START = 3072,
    XP_USR_DEFINED_MEM_POOL_OID_END = 4095,
    XP_ALLOCATOR_OID_ENDS
} xpAllocatorObjectId;

/**
 * \brief xpsAllocatorId
 * */
typedef enum xpsAllocatorId
{
    XPS_ALLOCATOR_STARTS = 0,
    XPS_ALLOCATOR_VIF_PORT,
    XPS_ALLOCATOR_VIF_LAG,
    XPS_ALLOCATOR_VIF_TUNNEL_ENCAP,
    XPS_ALLOCATOR_VIF_MULTI_PORT,
    XPS_ALLOCATOR_L2_DOMAIN_TYPE_ROUTE_ONLY,
    XPS_ALLOCATOR_AC,
    XPS_ALLOCATOR_VIF_SEGMENT_ROUTE,
    XPS_ALLOCATOR_BRIDGE_PORT,
    XPS_ALLOCATOR_ACL_TABLE_ID,
    XPS_ALLOCATOR_ACL_GLOBAL_TABLE_INDEXING,
    XPS_ALLOCATOR_EACL_TABLE_ID,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_1,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_2,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_3,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_4,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_5,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_6,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_7,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_8,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_9,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_10,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_11,
    XPS_ALLOCATOR_ACL_COUNTER_BLOCK_12,
    XPS_ALLOCATOR_INGRESS_PORT_POLICER_ENTRY,
    XPS_ALLOCATOR_EGRESS_PORT_POLICER_ENTRY,
    XPS_ALLOCATOR_INGRESS_FLOW_POLICER_ENTRY,
    XPS_ALLOCATOR_EGRESS_FLOW_POLICER_ENTRY,
    XPS_ALLOCATOR_COPP_POLICER_ENTRY,
    XP_SAI_ALLOC_ACL_TBL,
    XP_SAI_ALLOC_PACL_TBL,
    XP_SAI_ALLOC_BACL_TBL,
    XP_SAI_ALLOC_RACL_TBL,
    XP_SAI_ALLOC_ACL_ENTRY,
    XP_SAI_ALLOC_ACL_TCP_UDP_PORT_RANGE,
    XP_SAI_ALLOC_ACL_TBL_GRP,
    XP_SAI_ALLOC_ACL_TBL_GRP_MEMBER,
    XP_SAI_ALLOC_ACL_COUNTER_START,//TODO need to handle separately
    XP_SAI_ALLOC_ACL_COUNTER_END, //TODO
    XP_SAI_ALLOC_HOST_INTF,
    XP_SAI_ALLOC_HOST_INTF_GRP,
    XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY,
    XP_SAI_ALLOC_SCHD_PFL,
    XP_SAI_ALLOC_POLICER,
    XP_SAI_ALLOC_QOS_MAP,
    XP_SAI_ALLOC_BUFFER_POOL,
    XP_SAI_ALLOC_BUFFER_PROFILE,
    XP_SAI_ALLOC_INGRESS_POOL,
    XP_SAI_ALLOC_EGRESS_POOL,
    XP_SAI_ALLOC_HASH,
    XP_SAI_ALLOC_VRF,
    XP_SAI_ALLOC_LOOPBACK_RIF,
    XP_SAI_ALLOC_L2MC_GRP,
    XP_SAI_ALLOC_L2MC_GRP_MEMBER,
    XP_SAI_ALLOC_TUNNEL,
    XP_SAI_ALLOC_TUNNEL_TERM_ENTRY,
    XP_SAI_ALLOC_TUNNEL_MAP,
    XP_SAI_ALLOC_TUNNEL_MAP_ENTRY,
    XP_SAI_ALLOC_RIF,
    XP_SAI_ALLOC_IPMC_GRP,
    XP_SAI_ALLOC_IPMC_GRP_MEMBER,
    XP_SAI_ALLOC_RPF_GRP,
    XP_SAI_ALLOC_RPF_GRP_MEMBER,
    XPS_ALLOCATOR_ROUTER_ARP_DA_AND_TUNNL_START, /* Tunnel Start and Router ARP MAC DA shares same table. Tunnel Start entry utilize 4 entries*/
    XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
    XPS_ALLOCATOR_L3_NEIGH_TBL,
    XPS_ALLOCATOR_MAC2ME_TBL,
    XPS_ALLOCATOR_STP_ID,
    XPS_ALLOCATOR_L3_NEXT_HOP_GRP,
    XPS_ALLOCATOR_ECMP_TBL,
    XPS_ALLOCATOR_NETDEV_ID,
    XPS_ALLOCATOR_WRED_ID,
    XPS_ALLOCATOR_TUNNEL_START_v4,
    XPS_ALLOCATOR_TUNNEL_START_v6,
    XPS_ALLOCATOR_ANALYZER,
    XP_ALLOC_TUNNEL_TERM_HW_ENTRY,
    XP_ALLOC_RIF_HW_ENTRY,
    XP_ALLOC_PBR_HW_ENTRY,
    XP_ALLOC_MLL_PAIR_HW_ENTRY,
    XP_ALLOC_MC_VIDX_HW_TBL,
    XP_ALLOC_MC_EVIDX_HW_TBL,
    XP_ALLOC_GLOBAL_EPORT,
    XP_ALLOC_L2_ECMP_EPORT,
    XP_ALLOC_LTT_ECMP,
    XP_ALLOC_VXLAN_UNI_TTI_HW_ENTRY,
    XP_ALLOC_VXLAN_IPV6_ACL,
    XPS_ALLOCATOR_ENDS,
} xpsAllocatorId;

#define XP_USR_DEFINED_ALLOCATOR_MAX_NUM ((XP_USR_DEFINED_ALLOCATOR_OID_END + 1) - XP_USR_DEFINED_ALLOCATOR_OID_START)
#define XP_USR_DEFINED_MEM_POOL_MAX_NUM ((XP_USR_DEFINED_MEM_POOL_OID_END + 1) - XP_USR_DEFINED_MEM_POOL_OID_START)

//The instanceId could be deviceId or scopeId etc.
#define XP_ALLOCATOR_OID(instanceId, oidShort) ((oidShort<<32) | (instanceId))        ///< Allocator Oid

/**
 * \brief xpLockIndex_e
 * Unique managers lock indexes
 *
 */
typedef enum xpLockIndex_e
{
    XP_LOCKINDEX_SAL = 0,                        /* 0 */
    XP_LOCKINDEX_PDI,                            /* 1 */
    XP_LOCKINDEX_BUFFER_MGR,                     /* 2 */
    XP_LOCKINDEX_REGMANAGER,                     /* 3 */
    XP_LOCKINDEX_PERSIST_ALLOCATOR,              /* 4 */
    XP_LOCKINDEX_HW_OS_DRIVER,                   /* 5 */
    XP_LOCKINDEX_IPC_TX,                         /* 6 */
    XP_LOCKINDEX_IPC_RX,                         /* 7 */
    XP_LOCKINDEX_IPC_UDP,                        /* 8 */
    XP_LOCKINDEX_IPC_TCP,                        /* 9 */
    XP_LOCKINDEX_DMA,                            /* 10 */
    XP_LOCKINDEX_BUS,                            /* 11 */
    XP_LOCKINDEX_QUEUE_TX,                       /* 12 */
    XP_LOCKINDEX_QUEUE_RX,                       /* 13 */
    XP_LOCKINDEX_HAL_INTR,                       /* 14 */
    XP_LOCKINDEX_FILE_LOG,                       /* 15 */
    XP_LOCKINDEX_CONSOLE_LOG,                    /* 16 */
    XP_LOCKINDEX_BUF_LOG,                        /* 17 */
    XP_LOCKINDEX_PORT,                           /* 18 */
    XP_LOCKINDEX_PORT_CREATE,                    /* 19 */
    XP_LOCKINDEX_SHARED_BUF_LOG,                 /* 20 */
    XP_LOCKINDEX_HW_ACCESS_MGR,                  /* 21 */
    XP_LOCKINDEX_MAC_READ_WRITE,                 /* 22 */
    XP_LOCKINDEX_MAC_LIVE_RL_FAULT,              /* 23 */
    XP_LOCKINDEX_FL_ALLOCATOR_MGR,               /* 24 */
    XP_LOCKINDEX_XPS_VLAN,                       /* 25 */
    XP_LOCKINDEX_XPS_MULTICAST,                  /* 26 */
    XP_LOCKINDEX_XPS_PORT,                       /* 27 */
    XP_LOCKINDEX_XPS_STP,                        /* 28 */
    XP_LOCKINDEX_XPS_IACL,                       /* 29 */
    XP_LOCKINDEX_XPS_WCM_IACL,                   /* 30 */
    XP_LOCKINDEX_XPS_WCMIACL,                    /* 31 */
    XP_LOCKINDEX_XPS_EACL,                       /* 32 */
    XP_LOCKINDEX_XPS_INTERFACE,                  /* 33 */
    XP_LOCKINDEX_XPS_8021BR = XP_LOCKINDEX_XPS_INTERFACE,/* 33 */
    XP_LOCKINDEX_XPS_MIRROR,                     /* 34 */
    XP_LOCKINDEX_XPS_QOS,                        /* 35 */
    XP_LOCKINDEX_XPS_LAG,                        /* 36 */
    XP_LOCKINDEX_XPS_NHGRP,                      /* 37 */
    XP_LOCKINDEX_XPS_TUNNEL,                     /* 38 */
    XP_LOCKINDEX_XPS_MTU,                        /* 39 */
    XP_LOCKINDEX_XPS_MPLS,                       /* 40 */
    XP_LOCKINDEX_XPS_TCAMMGR,                    /* 41 */
    XP_LOCKINDEX_XPS_TCAMACLSHUFFLING,           /* 42 */
    XP_LOCKINDEX_XPS_TCAMLISTSHUFFLING,          /* 43 */
    XP_LOCKINDEX_XPS_OPENFLOW,                   /* 44 */
    XP_LOCKINDEX_XPS_SR,                         /* 45 */
    XP_LOCKINDEX_FL_INT_MGR,                     /* 46 */
    XP_LOCKINDEX_TEST1,                          /* 47 */
    XP_LOCKINDEX_TEST2,                          /* 48 */
    XP_LOCKINDEX_SAL_THREAD_COUNTER,             /* 49 */
    XP_LOCKINDEX_SERDES_READ_WRITE,              /* 50 */
    XP_LOCKINDEX_AN_LP_THREAD_DATA,              /* 51 */
    XP_LOCKINDEX_XPS_VSI,                        /* 52 */
    XP_LOCKINDEX_XPS_AC,                         /* 53 */
    XP_LOCKINDEX_INTERRUPT,                      /* 54 */
    XP_LOCKINDEX_IPC_NETLINK,                    /* 55 */
    XP_LOCKINDEX_PKT_CAPTURE_LOGFILE,            /* 56 */
    XP_LOCKINDEX_FDB_MGR_LOCK,                   /* 57 */
    XP_LOCKINDEX_ARP_LOCK,                       /* 58 */
    XP_LOCKINDEX_XPS_FDB_LOCK,                   /* 59 */
    XP_LOCKINDEX_MAX,
    /* Don't add enums below this */
    XP_LOCKINDEX_INVALID = 0xFFFFFFFFu
} xpLockIndex_e;

/**
 * \brief xpPtpEventHandler
 *
 */
typedef void (*xpPtpEventHandler)(void);

#define IS_TNL_ENCAP(encap) (((encap) == XP_L2_ENCAP_VXLAN) || ((encap) == XP_L2_ENCAP_NVGRE) || ((encap) == XP_L2_ENCAP_PBB) \
                            || ((encap) == XP_L2_ENCAP_GENEVE) || ((encap) == XP_L2_ENCAP_MPLS))         ///< Check if tunel is encapsulated?

// TODO: MOVE THESE TO RIGHT HEADER FILE FOR NOW NEEDS TO BE IN A COMMON LOCATION
#define XP_VLAN_DEFAULT_EGR_FILTER  0        ///< Vlan Default Egress Filter
#define XP_VLAN_INVALID_ID          0xFFFFFFFF        ///< Vlan Invalid Id
#define XP_VLAN_INVALID_MDT_IDX     0x0        ///< Vlan Invalid Mdt Index

/**
 * \brief xpBypassMode_e
 * Bypass Modes supported
 */
typedef enum
{
    XP_TXQ_EQ_BYPASS_MODE_ALL = 0,  ///< bypass all tokens
    XP_TXQ_EQ_BYPASS_MODE_CUT_THRU, ///< bypass the TxQ cut-thru tokens only with best effort.  If packet cannot bypass, falls back to XP_TXQ_EQ_BYPASS_MODE_NONE mode
    XP_TXQ_EQ_BYPASS_MODE_CUT_THRU_NON_MRE_COPY,    // bypass cut-thru non-mre-copy tokens
    XP_TXQ_EQ_BYPASS_MODE_NONE      ///< bypass disabled (default).  Store & Forward operation. TxQ will wait for EOP
} xpBypassMode_e;

/**
 * \brief xpWredMode
 * Weighted Random Early Detection Modes supported
 */
typedef enum xpWredMode
{
    XP_DISABLE_WRED,
    XP_ENABLE_WRED_DROP,
    XP_ENABLE_WRED_MARK,
    XP_MAX_WRED_TYPES
} xpWredMode_e;

/**
 * \brief xpQGuarThresholdGranularity
 */
typedef enum xpQGuarThresholdGranularity
{
    XP_Q_GUAR_THRESHOLD_GRANULARITY_1,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_2,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_4,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_8,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_16,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_32,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_64,
    XP_Q_GUAR_THRESHOLD_GRANULARITY_128
} xpQGuarThresholdGranularity_e;

/**
 * \brief xpQosColor
 */
typedef enum xpQosColor
{
    XP_COLOR_TYPE_GREEN,
    XP_COLOR_TYPE_YELLOW,
    XP_COLOR_TYPE_RED,
    XP_COLOR_TYPE_MAX
} xpQosColor_e;

/**
 * \brief xpDynThldCoeff
 * Dynamic Thresholding formula's 'alpha' coefficient's
 *        setting
 */
typedef enum xpDynThldCoeff
{
    XP_COEFF_1,
    XP_COEFF_2,
    XP_COEFF_4,
    XP_COEFF_8,
    XP_COEFF_16,
    XP_COEFF_32,
    XP_COEFF_64,
    XP_COEFF_128,
    XP_COEFF_MAX
} xpDynThldCoeff_e;

/**
 * \brief xpDynThldFraction
 * Indication on whether or not to treat a dynamic
 *        thresholding coeffecient as a fraction or a multiplier
 */
typedef enum xpDynThldFraction
{
    XP_MULTIPLIER,
    XP_DIVISOR,
    XP_INVALID_FRACTION
} xpDynThldFraction_e;

/**
 * \brief xpDynThldOperator
 *        For Dynamic Thresholding formulae that use an
 *        expression as a coefficient and can control the
 *        operator in that expression
 */
typedef enum xpDynThldOperator
{
    XP_SUBTRACTION,
    XP_ADDITION,
    XP_INVALID_OPERATOR
} xpDynThldOperator_e;

/**
 * \brief xpHashType_e
 */
typedef enum
{
    XP_LAG = 0,
    XP_L2ECMP,
    XP_L3ECMP
} xpHashType_e;

/**
 * \brief xpAclEnConfig_e
 */
typedef enum
{
    XP_ACL_DISABLED = 0,
    XP_L2_USER_ACL  = 1,
    XP_L2_QOS_ACL   = 2,
    XP_PBR_ACL      = 4,
    XP_L3_USER_ACL  = 8,
    XP_L3_QOS_ACL   = 16
} xpAclEnConfig_e;

/**
 * \brief xpPolicerField_t
 */
typedef enum
{
    XP_POLICER_POL_RESULT,
    XP_POLICER_REMARK_RED,
    XP_POLICER_REMARK_YELLOW,
    XP_POLICER_REMARK_GREEN,
    XP_POLICER_DROP_YELLOW,   //Specific to xp80
    XP_POLICER_RESERVED0,
    XP_POLICER_DROP_RED,
    XP_POLICER_COLOR_AWARE,
    XP_POLICER_PIR_EXPO,
    XP_POLICER_PIR_BASE,
    XP_POLICER_CIR_EXPO,
    XP_POLICER_CIR_BASE,
    XP_POLICER_PIR_TIME_GRAN_EXPO,
    XP_POLICER_PBS_EXPO,
    XP_POLICER_PBS_BASE,
    XP_POLICER_CIR_TIME_GRAN_EXPO,
    XP_POLICER_CBS_EXPO,
    XP_POLICER_CBS_BASE,
    XP_POLICER_PACKET_MODE    //Specific to xp70

} xpPolicerField_t;


/**
 * \brief xpSchedPolicy
 * Supported Scheduling policy by Packet Driver
 *
 */
typedef enum
{
    SP = 0,
    WRR,
    RR
} xpSchedPolicy;

/**
 * \brief xpLayerType_t
 * Update xpGlobalSwitchControl.py:layerList if this is updated
 *
 */
typedef enum
{
    ETHERNET,
    TU_ETHER,
    ULL_ETHERNET,
    GTPc = ULL_ETHERNET,
    PBB,
    ARP,
    IPv4,
    IPv6,
    IPv6_SPLIT,
    MPLS,
    GRE,
    ICMP,
    IGMP,
    ICMPv6,
    UDP,
    TCP,
    FCoE,
    FCoE_SPLIT,
    CNM,
    SRH = FCoE,
    SRH_IP0 = FCoE_SPLIT,
    SRH_IP1 = CNM,
    GENEVE,
    GRE_NOLAYER,
    UDPVxLanNsh,
    NSH,
    PTP,
    TUNNEL_IPv6_SPLIT,
    TUNNEL_IPv6_ICMPv6,
    TUNNEL_IPv6_UDP,
    TUNNEL_IPv6_TCP,
    UDPVxLan,
    SRH_IP2,
    INT,
    GTPu,
    ETH_NEXT = GTPu,
    CFM = ETH_NEXT,
    IPv6_RH_EXT = ETH_NEXT,
    LAYER_NA,
    NUM_LAYERS = LAYER_NA
} xpLayerType_t;

/**
 * \brief xpXorFieldType_t
 */
typedef enum
{
    XP_XOR_FIELD01,
    XP_XOR_FIELD12,
    XP_XOR_FIELD23,
    XP_XOR_MAX_FIELDS
} xpXorFieldType_t;

typedef enum
{
    XP_IP_TUNNEL_MODE_PIPE = 0,
    XP_IP_TUNNEL_MODE_UNIFORM
} XpIpTunnelMode_t;

typedef enum
{
    XP_IP_TUNNEL_ECN_MODE_STANDARD = 0, //Valid for both Encap and Decap
    XP_IP_TUNNEL_ECN_MODE_COPY_FROM_OUTER, // Valid only for Decap
    XP_IP_TUNNEL_ECN_MODE_USER_DEFINED // Valid only for Encap
} XpIpTunnelEcnMode_t;


/**
 * \brief xpIpTunnelType_t
 */
typedef enum
{
    XP_IP_VXLAN_TUNNEL= 0,
    XP_IP_NVGRE_TUNNEL,
    XP_IP_GRE_TUNNEL,
    XP_IP_OVER_IP_TUNNEL,
    XP_IP_GENEVE_TUNNEL,
    XP_IP_VPN_GRE_TUNNEL,
    XP_IP_VPN_GRE_TUNNEL_LOOSE,
    XP_IP_GRE_ERSPAN2_TUNNEL,
    XP_IP_PBB_TUNNEL,
    XP_INVALID_TUNNEL
} xpIpTunnelType_t;

/**
 * \brief xpMplsTunnelType_t
 */
typedef enum xpMplsTunnelType_t
{
    XP_MPLS_SINGLE_LABEL_P2P_TUNNEL = 0,
    XP_MPLS_TWO_LABEL_P2P_TUNNEL,
    XP_MPLS_SINGLE_LABEL_P2MP_TUNNEL,
    XP_MPLS_TWO_LABEL_P2MP_TUNNEL,
    XP_MPLS_SINGLE_LABEL_VPN_TUNNEL,
    XP_MPLS_VPN_GRE_LOOSE_MODE_TUNNEL,
    XP_MPLS_INVALID_LABEL_TUNNEL
} xpMplsTunnelType_t;

/**
 * \brief XP_DAL_TYPE_E
 *
 */
typedef enum XP_DAL_TYPE_E
{
    XP_HARDWARE,
    XP_SHADOW_HARDWARE,
    XP_SHADOW_REMOTEWM,  // shadow on sdk and use ipc for remote whitemodel
    XP_SHADOW,
    XP_SHADOW_EMULATOR,  // for using with emulator
    XP_DAL_DEBUG,        // dal implementation for debug
    XP_DAL_I2C,
    XP_DAL_MDIO,
    XP_DAL_PIPE,
    XP_DAL_DNGL,
    XP_DAL_DUMMY,        // dummy dal
    XP_DAL_RDR,
    XP_DAL_DEFAULT,      // default dal type
    XP_DAL_FIFO,
    XP_DAL_LINUX_KERNEL,
    XP_DAL_NONE,
    XP_DAL_TYPE_TOTAL
} XP_DAL_TYPE_T;

/**
 * \brief XP_PIPE_MODE
 *  Global Number of Pipes Configuration Enum
 */
typedef enum XP_PIPE_MODE_E
{
    XP_PIPE_MODE_START = 0,
    XP_SINGLE_PIPE,
    XP_TWO_PIPE,
    XP_PIPE_MODE_TOTAL,
} XP_PIPE_MODE;

/**
 * \brief XP_DEV_TYPE_E
 *
 */
typedef enum XP_DEV_TYPE_E
{
    XP_DEV_TYPE_VALID = 0,
    XP80 = XP_DEV_TYPE_VALID,
    XP80B0,
    XP70,
    XP70A1,
    XP70A2,
    XP100,
    XPT,
    XP60,
    ALD,
    ALD2,
    BC3,
    ACX3,
    AC5XRD,
    AC5X48x1G6x10G,
    ALDB2B,
    ALDDB,
    CYGNUS,
    ALDRIN2XL,
    TG48M_P,
    FALCON32,
    FALCON_RANGE_START = FALCON32,
    FALCON32x400G,
    FC32x10016x400G,
    FC64x100GR4IXIA,
    FC32x100GR4IXIA,
    FALCONEBOF, /* 24x25G6x100G_3.2T */
    FC24x25G8x200G,
    FC24x25G8x100G,
    FC48x100G8x400G,
    FC96x100G8x400G,
    FALCON32X25G,
    FC32x25GIXIA,
    FALCON128x10G,
    FALCON128x25G,
    FALCON128x100G,
    FALCON128,
    FALCON128B2B,
    FALCON64x100GR4,
    FALCON16x400G,        /* 6.4T 16x400R8 */
    FC16x25GIXIA,         /* 6.4T 16x25R1  */
    FALCON16X25G,         /* 6.4T 16x25R1  */
    FC24x100G4x400G,      /* 6.4T 24x100R4 + 4x400R8 */
    FC16x100G8x400G,      /* 6.4T 16x100R4 + 8x400R8 */
    FC32x100G8x400G,      /* 6.4T 32x100R2 + 8x400R8 */
    FALCON64x100G,        /* 6.4T 64x100R2 */
    FALCON128x50G,        /* 6.4T 128x50R1 */
    FALCON64x25G,         /* 6.4T 64x25R1  */
    FC24x258x100G64,      /* 6.4T 24x25KR + 8x100R4  EBOF Demo */
    FC24x254x200G64,      /* 6.4T 24x25KR + 4x200R4  EBOF NetApp */
    FALCON32x25G64,       /* 6.4T 32x25G   */
    FC48x10G8x100G,       /* 6.4T 48x10GR1 + 8x100R4 */
    FC48x25G8x100G,       /* 6.4T 48x25GR1 + 8x100R4 */
    FC64x25G64x10G,       /* 12.8T 64x25GR1 + 64x10GR1 */
    FC32x25G8x100G,       /* 3.2T 32x25GR1 + 8x100GR4 */
    F2T80x25G,           /* 2T 80x25R1 */
    F2T80x25GIXIA,       /* 2T 80x25R1 */
    F2T48x10G8x100G,     /* 2T 48x10G+8x100R4 */
    F2T48x25G8x100G,     /* 2T  48x25CR1+8x100R4 */
    FALCON_RANGE_END=F2T48x25G8x100G,
    XP_DEV_TYPE_TOTAL
} XP_DEV_TYPE_T;

#define IS_DEVICE_XP70(DEV_TYPE) (((DEV_TYPE) == XP70 || (DEV_TYPE) == XP70A1 || (DEV_TYPE) == XP70A2 ) ? 1 : 0)
#define IS_DEVICE_XP100(DEV_TYPE) ((DEV_TYPE) == XP100 ? 1 : 0)
#define IS_DEVICE_XP60(DEV_TYPE) (((DEV_TYPE) == XP60) ? 1 : 0)
#define IS_DEVICE_XP70_XP60(DEV_TYPE) ((IS_DEVICE_XP70(DEV_TYPE)) || (IS_DEVICE_XP60(DEV_TYPE)))

#define IS_DEVICE_FALCON(DEV_TYPE)      ((IS_DEVICE_FALCON_12_8(DEV_TYPE)) || (IS_DEVICE_FALCON_6_4(DEV_TYPE)) || (IS_DEVICE_FALCON_3_2(DEV_TYPE)) || (IS_DEVICE_FALCON_2(DEV_TYPE)))
#define IS_DEVICE_FALCON_12_8(DEV_TYPE) ( \
                                        (FALCON64x100GR4==DEV_TYPE) || (FALCON128x10G==DEV_TYPE) || \
                                        (FALCON128x25G==DEV_TYPE) || (FALCON32x400G==DEV_TYPE) || (FC24x25G8x200G==DEV_TYPE) || \
                                        (FALCON32X25G==DEV_TYPE)||(FALCON128==DEV_TYPE)||(FALCON128B2B==DEV_TYPE) || \
                                        (FC32x25GIXIA==DEV_TYPE) || \
                                        (FC32x10016x400G==DEV_TYPE) || (FC48x100G8x400G==DEV_TYPE) || (FC96x100G8x400G==DEV_TYPE) || \
                                        (FALCON128x100G==DEV_TYPE) || (FC64x100GR4IXIA==DEV_TYPE) || (FC64x25G64x10G==DEV_TYPE) || \
                                        (FC24x25G8x100G==DEV_TYPE) \
                                        )
#define IS_DEVICE_FALCON_6_4(DEV_TYPE)  ( \
                                        (FC24x254x200G64==DEV_TYPE) || (FC24x258x100G64==DEV_TYPE) || (FALCON128x50G==DEV_TYPE)|| \
                                        (FALCON64x100G==DEV_TYPE) || (FALCON16X25G==DEV_TYPE) || (FC16x25GIXIA==DEV_TYPE) || \
                                        (FALCON16x400G==DEV_TYPE) || (FC24x100G4x400G==DEV_TYPE) || (FC32x100G8x400G==DEV_TYPE) || \
                                        (FC16x100G8x400G==DEV_TYPE) ||(FALCON32x25G64==DEV_TYPE) ||(FALCON64x25G==DEV_TYPE) || \
                                        (FC48x10G8x100G==DEV_TYPE) || (FC32x100GR4IXIA==DEV_TYPE) || (FC48x25G8x100G==DEV_TYPE)\
                                        )
#define IS_DEVICE_FALCON_3_2(DEV_TYPE)  ( \
                                        (FALCONEBOF==DEV_TYPE) || \
                                        (FC32x25G8x100G==DEV_TYPE) \
                                        )
#define IS_DEVICE_FALCON_2(DEV_TYPE)    ( \
                                        (F2T80x25G==DEV_TYPE) || (F2T48x10G8x100G==DEV_TYPE) || (F2T48x25G8x100G == DEV_TYPE) || (F2T80x25GIXIA==DEV_TYPE) \
                                        )
#define IS_DEVICE_AC3X(DEV_TYPE)        ((ALDB2B==DEV_TYPE) || (TG48M_P==DEV_TYPE))
#define IS_DEVICE_AC5X(DEV_TYPE)        ((AC5XRD==DEV_TYPE) || (AC5X48x1G6x10G==DEV_TYPE))

#define IS_DEVICE_FALCON_10G_ONLY_PROFILE(DEV_TYPE) ( \
                                               (FALCON128x10G==DEV_TYPE) \
                                               )
#define IS_DEVICE_FALCON_25G_ONLY_PROFILE(DEV_TYPE) ( \
                                               (FALCON32X25G==DEV_TYPE) || (FC32x25GIXIA==DEV_TYPE) || \
                                               (FALCON128x25G==DEV_TYPE) || (FC16x25GIXIA==DEV_TYPE) || \
                                               (FALCON16X25G==DEV_TYPE) || (FALCON64x25G==DEV_TYPE) || \
                                               (FALCON32x25G64==DEV_TYPE) ||(FALCON128==DEV_TYPE) ||\
                                               (F2T80x25G==DEV_TYPE) || (F2T80x25GIXIA==DEV_TYPE) \
                                               )

#define IS_DEVICE_FALCON_10G_25G_ONLY_PROFILE(DEV_TYPE) ( \
                                               (FC64x25G64x10G==DEV_TYPE) \
                                               )

#define IS_DEVICE_EBOF_PROFILE(DEV_TYPE)       ( \
                                                (FALCONEBOF==DEV_TYPE) \
                                               )

/**
 * \brief XP_DEVICE_SUB_TYPE_E
 */
typedef enum XP_DEVICE_SUB_TYPE_E
{
    XP_DEVICE_SUB_TYPE_B0,
    XP_DEVICE_SUB_TYPE_B1,
    XP_DEVICE_SUB_TYPE_TOTAL
} XP_DEVICE_SUB_TYPE_T;


/**
 * \brief XP_ACM_PROFILE_TYPE_E
 */
typedef enum XP_ACM_PROFILE_TYPE_E
{
    XP_DEFAULT_SINGLE_PIPE_ACM_PROFILE          = 0,
    XP_DEFAULT_TWO_PIPE_ACM_PROFILE             = 1,
    XP_DEFAULT_ACM_BILLING_CNTR_PROFILE         = 2,
    XP_ROUTE_CENTRIC_SINGLE_PIPE_ACM_PROFILE    = 3,
    XP_OPENFLOW_HYBRID_SINGLE_PIPE_ACM_PROFILE  = 4,
    XP_TUNNEL_CENTRIC_SINGLE_PIPE_ACM_PROFILE   = 8,
    XP_PURE_OPENFLOW_TWO_PIPE_ACM_PROFILE       = 10,
    XP_QACL_SAI_ACM_PROFILE                     = 11,
    XP_MAX_ACM_PROFILE                          = 15
} XP_ACM_PROFILE_TYPE;

/**
 * \brief xpSerdesSignalOverride
 * For serdes signal status override. It has four kind of override state
 *
 */
typedef enum xpSerdesSignalOverride
{
    XP_SERDES_OK_LIVE,      // Pass the signal from SERDES as it is
    XP_SERDES_INVERTED,     // Inverted. Not available in A0
    XP_SERDES_OK_FALSE_TO_PCS, // Force the SERDES OK to 0 going into PCS
    XP_SERDES_OK_TRUE_TO_PCS   // Force the SERDES OK to 1 going into PCS
} xpSerdesSignalOverride_e;

/**
 * \brief xpRegAccessMode_t
 * For mode of register access over PCIe(ioctl/pcie)
 */
typedef enum xpRegAccessMode_t
{
    XP_PCIE_IOCTL,
    XP_PCIE_MMAP
} xpRegAccessMode_t;

/**
 * \brief xpHashOption
 * If this enum is changed, please update the enum in xpsGlobalSwitchControl.py
 *
 */
typedef enum xpHashOption
{
    XP_HASHA_ONLY = 0,
    XP_HASHB_ONLY = 1,
    XP_HASHA_AND_HASHB = 2
} xpHashOption_e;

typedef enum xpPreLookupType_t
{
    XP_PRELOOKUP_V4_SIP = 0,
    XP_PRELOOKUP_V4_DIP = 1,
    XP_PRELOOKUP_V6_SIP = 2,
    XP_PRELOOKUP_V6_DIP = 3,
    XP_PRELOOKUP_MAX = 4,

} xpPreLookupType_e;

/**
 * \brief xpDmaOperations
 */
typedef enum xpDmaOperations
{
    XP_DMA_TRANSMIT,
    XP_DMA_RECEIVE,
    XP_MAX_DMA_OPERATIONS
} xpDmaOperations;

/**
 * \brief xpPktDrvEventType
 */
typedef enum xpPktDrvEventType
{
    PKTDRV_RECEIVE,     ///< Reception
    PKTDRV_TRANSMIT,    ///< Transmission
    PKTDRV_ERROR,       ///< Error
    MAX_DMA_PKTDRV_TYPE
} xpPktDrvEventType;

/**
 * \brief Reset Types support
 *
 * Enums for supported reset types
 */
typedef enum xpSlaveResetType
{
    XP_SLAVE_RST_TYPE_DYNAMIC,  /// Reset the hardware state machine/logic of a block
    XP_SLAVE_RST_TYPE_CFG,      /// Reset the configuration of a block
    XP_SLAVE_RST_TYPE_MAX
} xpSlaveResetType_e;

/**
 * \brief The reset action to perform
 */
typedef enum xpSlaveResetAction
{
    XP_PUT_IN_RESET,            /// Put Device or Block in reset
    XP_TAKE_OUT_OF_RESET,       /// Take Device or Block out of reset
    XP_TOGGLE_RESET,            /// Force toggle reset of a Block or Device
    XP_RESET_MAX_ACTION
} xpSlaveResetAction_e;

/**
 * \brief XPA Device Family Reset Id
 *
 * Comprehensive list of slaves in all XP devices
 */
typedef enum xpSlaveResetId
{
    XP_SLAVE_RST_PTP,                 ///  Devices:  XP80,  XP70,  XP100  (AKA:  TS_MASTER)
    XP_SLAVE_RST_PTG_0,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_1,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_2,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_3,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_4,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_5,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_6,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_7,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_8,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_9,               ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_10,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_11,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_12,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_13,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_14,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_15,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_16,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_17,              ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_PTG_18,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_19,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_20,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_21,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_22,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_23,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_24,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_25,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_26,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_27,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_28,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_29,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_30,              ///  Devices:  XP80
    XP_SLAVE_RST_PTG_31,              ///  Devices:  XP80
    XP_SLAVE_RST_EPTG_0,              ///  Devices:  XP100
    XP_SLAVE_RST_IBUFFER_0,           ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_IBUFFER_1,           ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_IBUFFER_2,           ///  Devices:  XP100
    XP_SLAVE_RST_IBUFFER_3,           ///  Devices:  XP100
    XP_SLAVE_RST_DP_RXDMA_0,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_RXDMA_1,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_RXDMA_2,          ///  Devices:  XP100
    XP_SLAVE_RST_DP_RXDMA_3,          ///  Devices:  XP100
    XP_SLAVE_RST_DP_SDMA_0,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_SDMA_1,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_TXDMA_0,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_TXDMA_1,          ///  Devices:  XP80,  XP70.  XP100
    XP_SLAVE_RST_DP_TXDMA_2,          ///  Devices:  XP100
    XP_SLAVE_RST_DP_TXDMA_3,          ///  Devices:  XP100
    XP_SLAVE_RST_OBUFFER_0,           ///  Devices:  XP100
    XP_SLAVE_RST_OBUFFER_1,           ///  Devices:  XP100
    XP_SLAVE_RST_OBUFFER_2,           ///  Devices:  XP100
    XP_SLAVE_RST_OBUFFER_3,           ///  Devices:  XP100
    XP_SLAVE_RST_IOBUFFER_0,          ///  Devices:  XP100
    XP_SLAVE_RST_IOBUFFER_1,          ///  Devices:  XP100
    XP_SLAVE_RST_DP_PM_0,             ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_PM_1,             ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_PM_2,             ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_PM_3,             ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_DP_PM_4,             ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_PM_5,             ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_PM_6,             ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_PM_7,             ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_DP_PM_8,             ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_9,             ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_10,            ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_11,            ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_12,            ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_13,            ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_14,            ///  Devices:  XP80
    XP_SLAVE_RST_DP_PM_15,            ///  Devices:  XP80
    XP_SLAVE_RST_BM_0,                ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_BM_1,                ///  Devices:  XP100
    XP_SLAVE_RST_BM_2,                ///  Devices:  XP100
    XP_SLAVE_RST_BM_3,                ///  Devices:  XP100
    XP_SLAVE_RST_SDE_DECODER_0,       ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_DECODER_1,       ///  Devices:  XP80
    XP_SLAVE_RST_SDE_PARSER_0,        ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_PARSER_1,        ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_ACM_0,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_ACM_1,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_HDBF_0,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_HDBF_1,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_MRE_0,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_MRE_1,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_MME_0,           ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_MME_1,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_FR_0,            ///  Devices:  XP100
    XP_SLAVE_RST_SDE_FR_1,            ///  Devices:  XP100
    XP_SLAVE_RST_SDE_FM_0,            ///  Devices:  XP100
    XP_SLAVE_RST_SDE_FM_1,            ///  Devices:  XP100
    XP_SLAVE_RST_PKTTKR_0,            ///  Devices:  XP100
    XP_SLAVE_RST_PKTTKR_1,            ///  Devices:  XP100
    XP_SLAVE_RST_SDE_URW_0,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_URW_1,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_URW_2,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_URW_3,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_URW_4,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_URW_5,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_URW_6,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_URW_7,           ///  Devices:  XP80
    XP_SLAVE_RST_SDE_ISME_0,          ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_ISME_1,          ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_ISME_2,          ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_SDE_ISME_3,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_ISME_4,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_ISME_5,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_0,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_1,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_2,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_3,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_4,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_5,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_6,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_7,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_8,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_9,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_10,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_11,          ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SDE_LDE_12,          ///  Devices:  XP80,  XP100
    XP_SLAVE_RST_SDE_LDE_13,          ///  Devices:  XP80,  XP100
    XP_SLAVE_RST_SDE_LDE_14,          ///  Devices:  XP80,  XP100
    XP_SLAVE_RST_SDE_LDE_15,          ///  Devices:  XP80,  XP100
    XP_SLAVE_RST_SDE_LDE_16,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_17,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_18,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_19,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_20,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_21,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_22,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_LDE_23,          ///  Devices:  XP80
    XP_SLAVE_RST_SDE_MEM_0,           ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SDE_MEM_1,           ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_T0_0,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_T0_1,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_T1_0,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_T1_1,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_A0_0,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_A0_1,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_A1_0,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_A1_1,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_L0_0,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_L0_1,    ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_POOL_L0_2,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_3,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_4,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_5,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_6,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_7,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_8,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_9,    ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_10,   ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L0_11,   ///  Devices:  XP70
    XP_SLAVE_RST_SEARCH_POOL_L1_0,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_L1_1,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_C0_0,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_C0_1,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_C1_0,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_POOL_C1_1,    ///  Devices:  XP100
    XP_SLAVE_RST_SEARCH_SE_0,         ///  Devices:  XP80
    XP_SLAVE_RST_SEARCH_SE_1,         ///  Devices:  XP80
    XP_SLAVE_RST_SEARCH_MEM_0,        ///  Devices:  XP80
    XP_SLAVE_RST_SEARCH_AGE_0,        ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_SEARCH_AGE_1,        ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_REQNETWK_0,   ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_REQNETWK_1,   ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_RSLTNETWK_0,  ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_SEARCH_RSLTNETWK_1,  ///  Devices:  XP70,  XP100
    XP_SLAVE_RST_TXQ_DQ_0,            ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_TXQ_EQ_0,            ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_TXQ_EQ_DQ_0,         ///  Devices:  XP100
    XP_SLAVE_RST_TXQ_EQ_DQ_1,         ///  Devices:  XP100
    XP_SLAVE_RST_TXQ_EQ_DQ_2,         ///  Devices:  XP100
    XP_SLAVE_RST_TXQ_EQ_DQ_3,         ///  Devices:  XP100
    XP_SLAVE_RST_TXQ_AQM_0,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_TXQ_AQM_1,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_TXQ_AQM_2,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_TXQ_AQM_3,           ///  Devices:  XP80,  XP70,  XP100
    XP_SLAVE_RST_TXQ_TBM_0,           ///  Devices:  XP80,  XP70,  XP100  (AKA:  TFL/LLM)
    XP_SLAVE_RST_TXQ_TBM_1,           ///  Devices:  XP100  (AKA:  TFL/LLM)
    XP_SLAVE_RST_TXQ_TBM_2,           ///  Devices:  XP100  (AKA:  TFL/LLM)
    XP_SLAVE_RST_TXQ_TBM_3,           ///  Devices:  XP100  (AKA:  TFL/LLM)
    XP_SLAVE_RST_TXQ_TBWRAPPER_0,     ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_TXQ_CNT_LEN_0,       ///  Devices:  XP80,  XP70
    XP_SLAVE_RST_TXQ_CNT_STATS_0,     ///  Devices:  XP80,  XP70

    XP_SLAVE_RST_MAX
} xpSlaveResetId_e;

typedef enum
{
    XP_PT_GLOBAL_SHARED_POOL_WM,
    XP_PT_GLOBAL_MAX_THRESHOLD,
    XP_PT_GLOBAL_MC_MAX_THRESHOLD
} xpPtGlobalState_e;

typedef enum
{
    XP_PT_PORT_MAX_THRESHOLD,
    XP_PT_PORT_QUEUE_DYN_MAX_THRESHOLD,
    XP_PT_PORT_QUEUE_WM,
    XP_PT_PORT_AVG_QUEUE_WM,
} xpPtPortState_e;

typedef enum
{
    XP_PT_NO_MIRROR_MODE,
    XP_PT_STATE_CHANGE_MODE,
    XP_PT_LEVEL_CHANGE_MODE,
    XP_PT_RESERVED
} xpPtMirrorModes_e;

#endif  //_xpEnums_h_

