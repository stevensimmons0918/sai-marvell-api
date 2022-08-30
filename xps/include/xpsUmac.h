// xpsUmac.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsUmac.h
 * \brief This file contains the common feature APIs used for configuring
 * various registers of both version's A0 and B0
*/

#ifndef _XPUMAC_H_
#define _XPUMAC_H_

#ifdef __cplusplus
//#include "xpLogModMgmtSdk.h"
//#include "xpMac.h"
//#include "xpDeviceMgr.h"
//#include "xpTable.h"
//#include "xpHwAccessMgr.h"
#endif

#define SCPU_FIRMWARE_LOCK_MCPU_PTG_NUM         11
#define SCPU_FIRMWARE_LOCK_SCPU_PTG_NUM         10
#define PTG_MCPU_LOCK_PID                       0xB055      // BOSS mcpu
#define PTG_SCPU_LOCK_PID                       0x51A7E     // SLAVE scpu
#define PTG_Z80_LOCK_PID                        0x280       // Z80 z80
#define UNLOCK_PID                              0
#define Z80_DS_GA                               (0x3800 / 8)
#define TV80_UCODE_64B_SIZE                     2048
#define Z80_DS_LOCKAQRD_OFF                     1
#define Z80_DS_LOCKFAIL_OFF                     2
#define Z80_DS_LOCKSER_AQRD_OFF                 3
#define Z80_DS_LOCKSER_FAIL_OFF                 4
#define Z80_DS_MCPU_SEM_OFF                     5
#define Z80_DS_Z80_SEM_OFF                      6
#define Z80_DS_PORT_INFO_OFF                    7
#define Z80_DS_PORT_INFO_CH0_OFF                7
#define Z80_DS_PORT_INFO_CH1_OFF                11
#define Z80_DS_PORT_INFO_CH2_OFF                15
#define Z80_DS_PORT_INFO_CH3_OFF                19
/*
 *  32-bits for 4 channels (4x8) --- DMA_PTG_TX_PREAMBLE
 *
 *     7     6      5     4     3     2    1      0
 * ...+----+----+-----+------+-----+----+-----+--------+
 *    | FC | RS |IEEE | 100G | 40G |25G | 10G | ANLTEn |
 * ...+----+----+-----+------+-----+----+-----+--------+
 *
 */
#define Z80_CFG_ANLT_ENABLE(ch)                 (1ULL<<(0+(ch*8)))
#define Z80_CFG_SPEED_10G(ch)                   (1ULL<<(1+(ch*8)))
#define Z80_CFG_SPEED_25G(ch)                   (1ULL<<(2+(ch*8)))
#define Z80_CFG_SPEED_40G(ch)                   (1ULL<<(3+(ch*8)))
//#define Z80_CFG_SPEED_50G(ch)                   (1<<(4+(ch*8))) NOT SUPPORTED
#define Z80_CFG_SPEED_100G(ch)                  (1ULL<<(4+(ch*8)))
#define Z80_CFG_IEEE_MODE(ch)                   (1ULL<<(5+(ch*8)))
#define Z80_CFG_RS_FEC(ch)                      (1ULL<<(6+(ch*8)))
#define Z80_CFG_FC_FEC(ch)                      (1ULL<<(7+(ch*8)))
/*
 *  4-bits for 4 channels (4x1) --- DMA_PTG_TX_PREAMBLE
 *
 *      35 34 33 32
 *   ...+-+-+-+-+
 *      | | | | |
 *   ...+-+-+-+-+
 *
 */
#define Z80_CFG_AN_WAIT_STATE(ch)               (1ULL<<(32+(ch)))
/*
 *  8-bits for 4 channels (4x2) --- DMA_PTG_TX_PREAMBLE
 *
 *       42  40  38  36
 *  ...+----+---+---+---+
 *     | :  | : | : | : |
 *  ...+----+---+---+---+
 *
 */
#define Z80_25G_CRS_MODE(ch)                     (1ULL<<(36+(ch*2)))
#define Z80_25G_CR_MODE(ch)                      (2ULL<<(36+(ch*2)))

#define UMAC_MAX_CHAN                           4
#define UMAC_REG_OFFSET(i)                      (i)
#define MAX_MDIO_MASTER_CONFIGURATION           9
#define UMAC_STAT_MAX_NUM_TRY                   1000
#define UMAC_MDIO_BUSY_TIMEOUS_US               1000
#define UMAC_MAX_PCS_LANES                      20
#define UMAC_MID_MAC_ADDR_BIT_NUM               16
#define UMAC_HIGH_MAC_ADDR_BIT_NUM              32
#define UMAC_HASHTABLE2_BIT_NUM                 16
#define UMAC_HASHTABLE3_BIT_NUM                 32
#define UMAC_HASHTABLE4_BIT_NUM                 48
#define UMAC_TIMESTAMP_VAL1_BIT_NUM             16
#define UMAC_TIMESTAMP_VAL2_BIT_NUM             32
#define UMAC_TIMESTAMP_VAL3_BIT_NUM             48
#define UMAC_TIMESTAMP_VAL                      4
#define UMAC_TIMESTAMP_VAL_SIZE                 16
#define AN_SELECTOR_BIT_NUM                     4
#define AN_PAUSE_ABILITY_BIT_NUM                4
#define INT_STATUS_REG_NUM_BITS                 16
#define BPAN_INT_STAUS_REG_NUM_BITS             16
#define FEC_INT_STATUS_REG_NUM_BITS             16
#define MDIO_PHY_REG_ADDR_BIT_POS               5
#define MDIO_TRANS_CODE_BIT_POS                 10
#define MDIO_TRANS_TYPE_BIT_POS                 12
#define MDIO_GENERATE_FRM_BIT_POS               15

#define UMAC_MDIO_CONTROL_VALUE(GENERATE_MDIO_FRAME, DIRECT_INDIRECT_ADDR,  \
        TRANSACTION_CODE, REG_ADDR,PHY_ADDR)                \
(PHY_ADDR | (REG_ADDR << MDIO_PHY_REG_ADDR_BIT_POS)  |          \
 (TRANSACTION_CODE << MDIO_TRANS_CODE_BIT_POS) |                \
 (DIRECT_INDIRECT_ADDR << MDIO_TRANS_TYPE_BIT_POS)|             \
 (GENERATE_MDIO_FRAME << MDIO_GENERATE_FRM_BIT_POS) )




/******* U M A C   R E G I S T E R   V A L U E S *********/
#define UMAC_INIT_SLEEP_DURATION_MS                     3000
#define UMAC_INIT_WAIT_PCS_STATUS_RETRIES                   9
#define BIT_OFFSET_MIX_MODE                             4
#define BIT_OFFSET_SERDES_SIGNAL_OVERRIDE               8

/*Needs to be change for actual Start as per XP80*/
#define UMAC_REG_START_ADDR                                 0x000000
#define UMAC_REG_SPACE_SIZE                                         0xFFFF

#define INTERRUPT_STAT_REG_SIZE                         16
#define BIT_OFFSET_MAC_CH0_INTERRUPT                        0
#define BIT_OFFSET_MAC_CH3_INTERRUPT                        3
#define BIT_OFFSET_PCS_CH0_INTERRUPT                        4
#define BIT_OFFSET_PCS_CH3_INTERRUPT                        7
#define BIT_OFFSET_FEC_CH0_INTERRUPT                        8
#define BIT_OFFSET_FEC_CH3_INTERRUPT                        11
#define BIT_OFFSET_BPAN_CH0_INTERRUPT                       12
#define BIT_OFFSET_BPAN_CH3_INTERRUPT                       15
#define BIT_OFFSET_MAC_CH0_INTERRUPT                        0

#define UMAC_PCS_LOOPBACK_ENABLE                        0x4000
#define UMAC_PCS_INTERFACE_LOOPBACK_ENABLE              0x0008
#define UMAC_FIFO_LOOPBACK_ENABLE                       0x0010

#define UMAC_100GB_MODE_VAL                                     0x0000
#define UMAC_1X40GB_MODE_VAL                                    0x0002
#define UMAC_2X50GB_MODE_VAL                                    0x0003
#define UMAC_10GB_MODE_VAL                                      0x0004
#define UMAC_4X25GB_MODE_VAL                                    0x0005
#define UMAC_QSGMII_MODE_VAL                                    0x0008
#define UMAC_SGMII_MODE_VAL                                     0x0009
#define UMAC_MIX_MODE_VAL                                   0x000F

#define UMAC_CHANNEL0_SPEED_1GB_VALUE                       0x0000
#define UMAC_CHANNEL0_SPEED_1GB_BITMASK                     0x0300

#define UMAC_CHANNEL0_SPEED_100MB_VALUE                     0X0100
#define UMAC_CHANNEL0_SPEED_100MB_BITMASK                   0X0300

#define UMAC_CHANNEL0_SPEED_10MB_VALUE                      0x0200
#define UMAC_CHANNEL0_SPEED_10MB_BITMASK                    0x0300

#define UMAC_CHANNEL1_SPEED_1GB_VALUE                       0x0000
#define UMAC_CHANNEL1_SPEED_1GB_BITMASK                     0x0C00

#define UMAC_CHANNEL1_SPEED_100MB_VALUE                     0X0400
#define UMAC_CHANNEL1_SPEED_100MB_BITMASK                   0X0C00

#define UMAC_CHANNEL1_SPEED_10MB_VALUE                      0x0800
#define UMAC_CHANNEL1_SPEED_10MB_BITMASK                    0x0C00

#define UMAC_CHANNEL2_SPEED_1GB_VALUE                       0x0000
#define UMAC_CHANNEL2_SPEED_1GB_BITMASK                     0x3000

#define UMAC_CHANNEL2_SPEED_100MB_VALUE                     0X1000
#define UMAC_CHANNEL2_SPEED_100MB_BITMASK                   0X3000

#define UMAC_CHANNEL2_SPEED_10MB_VALUE                      0x2000
#define UMAC_CHANNEL2_SPEED_10MB_BITMASK                    0x3000

#define UMAC_CHANNEL3_SPEED_1GB_VALUE                       0x0000
#define UMAC_CHANNEL3_SPEED_1GB_BITMASK                     0xC000

#define UMAC_CHANNEL3_SPEED_100MB_VALUE                     0X4000
#define UMAC_CHANNEL3_SPEED_100MB_BITMASK                   0XC000

#define UMAC_CHANNEL3_SPEED_10MB_VALUE                      0x8000
#define UMAC_CHANNEL3_SPEED_10MB_BITMASK                    0xC000

#define UMAC_CHANNEL0_SGMII_SPEED_GET                       0x0300
#define UMAC_CHANNEL1_SGMII_SPEED_GET                       0x0C00
#define UMAC_CHANNEL2_SGMII_SPEED_GET                       0x3000
#define UMAC_CHANNEL3_SGMII_SPEED_GET                       0xC000

#define UMAC_RX_CRC_DISABLE                         0x0001
#define UMAC_RX_PREAMBLE_LENGTH_4BYTE                       0x0008
#define UMAC_RX_PREAMBLE_LEN_MASK                       0x0080
#define UMAC_RX_EXTRACT_CUSTOM_PREAMBLE_ENABLE                  0x0040
#define UMAC_RX_FLOW_CONTROL_DECODE_ENABLE                  0x0020
#define UMAC_TX_PREAMBLE_INSERTION_ENABLE                   0x0400
#define UMAC_TX_IFG_CONTROL_PER_FRAME_ON                    0x4000
#define UMAC_RX_PROMISCUOUS_MODE_ENABLE                     0x0010
#define UMAC_RX_STRIP_FCS_ENABLE                        0x0002
#define UMAC_MAXFRMSIZE_WITHOUT_VLAN_REG_VALUE                  0x0000
#define UMAC_MAXFRMSIZE_WITH_1VLAN_REG_VALUE                    0x0004
#define UMAC_MAXFRMSIZE_WITH_2VLAN_REG_VALUE                    0x0008
#define UMAC_MAXFRMSIZE_WITH_3VLAN_REG_VALUE                    0x000C
#define UMAC_RX_PAUSE_FRAME_FILTER_BITMASK                                      0x0100
#define UMAC_TX_CRC_DISABLE                         0x0001
#define UMAC_TX_PRIORITYFLOW_CONTROL_ENABLE                 0x0200
#define UMAC_TX_FLOW_CONTROL_ENABLE                     0x0100
#define UMAC_TX_FCS_INVERT_ENABLE                       0x0002
#define UMAC_RX_ENABLE                              0x0002
#define UMAC_TX_ENABLE                              0x0001
#define UMAC_SOFT_RESET_ENABLE                          0x0004
#define UMAC_STATISTIC_CNT_RESET                        0x0020
#define UMAC_STATISTIC_CNTROL_REG_BITMASK                                       0x9FFF
#define UMAC_PCS_RESET_ENABLE                           0x8000
#define UMAC_PCS_SQUARE_WAVE_TEST_PATTERN                   0x0002
#define UMAC_PCS_RX_TEST_MODE_ENABLE                        0x0004
#define UMAC_PCS_TX_TEST_MODE_ENABLE                        0x0008
#define UMAC_PCS_PRBS31_TX_ENABLE                       0x0010
#define UMAC_PCS_PRBS31_RX_ENABLE                       0x0020
#define UMAC_PCS_PRBS9_TX_ENABLE                        0x0040
#define UMAC_PCS_SCRAMBLED_IDEA_TEST_PATTERN_ENABLE             0x0080
#define UMAC_PCS_DOUBLE_SPEED_ENABLE                        0x0001
#define UMAC_PCS_10G_ENCODING_ENABLE                        0x0002
#define UMAC_PCS_BYPASS_BUFFER                          0x0010
#define UMAC_PCS_LOW_LATENCY_MODE                       0x2000
#define UMAC_FEC_ENABLE                             0x4000
#define UMAC_PAUSE_FRAME_BIT_VALUE                      0x0001
#define UMAC_PCS_LOCAL_FAULT_DATA_PATTERN_VALUE                 0x0000
#define UMAC_PCS_ZERO_DATA_PATTERN_VALUE                    0x0001

#define UMAC_SGMII_CH0_SPEED_BIT_POS                        8
#define UMAC_SGMII_CH1_SPEED_BIT_POS                        10
#define UMAC_SGMII_CH2_SPEED_BIT_POS                        12
#define UMAC_SGMII_CH3_SPEED_BIT_POS                        14
#define UMAC_TX_IFG_LENGTH_BIT_POS                      2
#define UMAC_STAT_CTRL_CH_NUM_BIT_POS                       5
#define UMAC_PCS_DECODE_TRAP_SEL_BIT_POS                    10
#define UMAC_PCS_INTERRUPT_BIT_POS                      4
#define UMAC_BPAN_INTERRUPT_STATUS_BIT_POS                  12
#define UMAC_FEC_INTERRUPT_STATUS_BIT_POS                   8
#define UMAC_PCS_INTERRUPT_STATUS_BIT_POS                   4
#define UMAC_REG_SIZE                               16
#define UMAC_PCS_BLOCK_LOCK_1_LEN                       8
#define UMAC_PCS_ALIGNMENT_STATUS_FIELD_LEN                 1
#define UMAC_PCS_AM_LOCK_1_FIELD_LEN                        8
#define UMAC_PCS_DEBUG_DESKEW_OVERFLOW_FIELD_LEN                4

#define UMAC_NUM_OF_PCS_TEST_PATTERN_SEED_A_REG                 4
#define UMAC_NUM_OF_PCS_TEST_PATTERN_SEED_B_REG                 4
#define UMAC_NUM_OF_STAT_DATA_REG                       4
#define UMAC_STAT_DATA_REG_SIZE                         16
#define UMAC_QSGMII_CH0_AN_RESTART_BIT_POS                  4
#define UMAC_CH0_ENABLE_BIT_POS                         8
#define UMAC_PCS_TEST_PATTERN_SEED_A_REG_SIZE                   16
#define UMAC_PCS_TEST_PATTERN_SEED_B_REG_SIZE                   16
#define UMAC_MAC_MODE_2X40G_CH1_RELEVANT_PORT_OFFSET                2

#define UMAC_MDIO_FRAME_GENERATE                        0x01
#define UMAC_MDIO_DIRECT_ADDRESS_ENA                        0x00
#define UMAC_MDIO_WRITE_OP_ENA                          0x01
#define UMAC_MDIO_READ_OP_ENA                           0x02

#define UMAC_SGMII_1GB_SPEED_VAL                        0x0000
#define UMAC_SGMII_100MB_SPEED_VAL                      0x0001
#define UMAC_SGMII_10MB_SPEED_VAL                       0x0002

#define UMAC_SGMII_AN_10MB_SPEED_VAL                        0x0000
#define UMAC_SGMII_AN_100MB_SPEED_VAL                       0x0400
#define UMAC_SGMII_AN_1GB_SPEED_VAL                     0x0800

#define UMAC_100G_DATA_PATH_WIDTH                       16
#define UMAC_40G_DATA_PATH_WIDTH                        8
#define UMAC_10G_DATA_PATH_WIDTH                        4
#define UMAC_1G_DATA_PATH_WIDTH                         1

#define UMAC_MDIO_CLK_DIVISON_CTRL_MAX_VAL                  0xFE
#define UMAC_TX_PRIORITY_PAUSE_VECTOR_MAX_VAL                   0xFF
#define UMAC_TX_IFG_MAX_LENGTH                          63

#define UMAC_BPAN_SELECTOR_MAX_VAL                      8
#define UMAC_PCS_DBG_FULL_THRESHOLD_MAX_VAL                 8
#define UMAC_BPAN_FORCE_TNONCE_MAX_VAL                      31
#define UMAC_BPAN_FORCE_ENONCE_MAX_VAL                      31

#define UMAC_FEC_10G_SOFT_RESET_ENA                     0x0001
#define UMAC_FEC_10G_ERR_TO_PCS_ENA                     0x0002
#define UMAC_FEC_10G_ENA                            0x0001

#define UMAC_FEC_100G_BYPASS_ENA                        0x0001
#define UMAC_FEC_100G_SOFT_RESET_ENA                        0x0001
#define UMAC_FEC_100G_ENA                           0x0002

#define UMAC_BPAN_AN_ENABLE                         0x0001
#define UMAC_BPAN_NORMAL_MODE                           0x0002
#define UMAC_BPAN_PN_SEQUENCE_ENABLE                        0x0004
#define UMAC_BPAN_ARBITRATION_FUN_ENABLE                    0x0008
#define UMAC_BPAN_SOFT_RESET_ENABLE                     0x0200
#define UMAC_BPAN_TX_ENABLE                         0x0400
#define UMAC_BPAN_RX_ENABLE                         0x0800
#define UMAC_BPAN_RXGB_RESYNC_ENABLE                        0x1000
#define UMAC_BPAN_NEXT_PAGE_TRANS_ENABLE                    0x2000
#define UMAC_BPAN_TNONCE_FORCE_ENABLE                       0x0020
#define UMAC_BPAN_ECHOED_NONCE_FORCE_ENABLE                 0x0020
#define UMAC_BPAN_FCE_ABILITY_ENABLE                        0x0001
#define UMAC_BPAN_FCE_REQUEST_ENABLE                        0x0002
#define UMAC_BPAN_REMOTE_FCE_REQUEST_ENABLE                 0x0002
#define UMAC_BPAN_REMOTE_FCE_ABILITY_ENABLE                 0x0001

#define UMAC_PCS_DEBUG_CONTROL_100GB_VAL                    0x1D81
#define UMAC_PCS_DEBUG_CONTROL_1X40GB_VAL                   0x1D81
#define UMAC_PCS_DEBUG_CONTROL_2X40GB_VAL                   0x1D81
#define UMAC_PCS_DEBUG_CONTROL_10GB_VAL                     0x1D41
#define UMAC_PCS_DEBUG_CONTROL_MIX_MODE_VAL                 0x1D41

#define UMAC_VENDOR_REG_INIT_VALUE                          0x0001

#define UMAC_TX_TIMESTAMP_FIFO_AVAILABLE_INT_ENA_VAL                0x0100
#define UMAC_TX_TIMESTAMP_FIFO_OFLOW_INT_ENA_VAL                0x0004
#define UMAC_TX_JABBER_INT_ENA_VAL                      0x0002
#define UMAC_TX_UNDER_RUN_INT_ENA_VAL                       0x0001

#define UMAC_PCS_CH0_INT_EN_VAL                         0x0010
#define UMAC_HSPCS_CH0_INT_EN_VAL                       0x0010
#define UMAC_LSPCS_CH0_INT_EN_VAL                       0x0100
#define UMAC_BPAN_TX_BASE_PAGE_INT_ENA_VAL                  0x0001
#define UMAC_BPAN_TX_NEXT_PAGE_INT_ENA_VAL                  0x0002
#define UMAC_BPAN_RX_BASE_PAGE_INT_ENA_VAL                  0x0008
#define UMAC_BPAN_RX_NEXT_PAGE_INT_ENA_VAL                  0x0010
#define UMAC_BPAN_COMPLETE_INT_VAL                      0x0100

//BPAN Technology Ability Register LOW -- BITS
//      bit0: 1000BASE-KX
//      bit1:  10GBACE-KX4
//      bit2:  10GBACE-KR
//      bit3:  40GBACE-KR4
//      bit4:  40GBACE-CR4
//      bit5: 100GBACE-CR10
//      bit6: 100GBACE-KP4
//      bit7: 100GBACE-KR4
//      bit8: 100GBACE-CR4
#define IEEE_TECH_ABILITY_1000BASE_KX_BIT      0x0001
#define IEEE_TECH_ABILITY_10GBASE_KX4_BIT      0x0002
#define IEEE_TECH_ABILITY_10GBASE_KR_BIT       0x0004
#define IEEE_TECH_ABILITY_40GBASE_KR4_BIT      0x0008
#define IEEE_TECH_ABILITY_40GBASE_CR4_BIT      0x0010
#define IEEE_TECH_ABILITY_100GBASE_CR10_BIT    0x0020
#define IEEE_TECH_ABILITY_100GBASE_KP4_BIT     0x0040
#define IEEE_TECH_ABILITY_100GBASE_KR4_BIT     0x0080
#define IEEE_TECH_ABILITY_100GBASE_CR4_BIT     0x0100
#define IEEE_TECH_ABILITY_25GBASE_KR1_BIT      0x0200
#define IEEE_TECH_ABILITY_25GBASE_CR1_BIT      0x0400
#define IEEE_TECH_ABILITY_50GBASE_KR2_BIT      0x0800
#define IEEE_TECH_ABILITY_50GBASE_CR2_BIT      0x1000
#define IEEE_TECH_ABILITY_50GBASE_KR_CR_BIT    0x2000
#define IEEE_TECH_ABILITY_100GBASE_KR2_CR2_BIT 0x4000
#define IEEE_TECH_ABILITY_200GBASE_KR4_CR4_BIT 0x8000
#define IEEE_TECH_ABILITY_400GBASE_KR8_BIT     0x10000
#define IEEE_TECH_ABILITY_400GBASE_CR8_BIT     0x20000

#define UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT         0x0001
#define UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT           0x0002
#define UMAC_BPAN_AN_CTRL_FEC_MODE_RS               0x0004        ///<  XP defined An Ctrl Fec mode RS offset
#define UMAC_BPAN_AN_CTRL_FEC_MODE_FC               0x0008        ///<  XP defined An Ctrl Fec mode FC offset
#define UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514       0x0010
#define RS_FEC_SUPPORT  0x04
#define FC_FEC_SUPPORT  0x08


#define TX_TIMESTAMP_FIFO_AVAILABLE                     0x0100
#define TX_TIMESTAMP_FIFO_OFLOW                         0x0004
#define TX_JABBER_INT                               0x0002
#define TX_UNDER_RUN_INT                            0x0001

#define TX_BASE_PAGE_INT                            0x0001
#define TX_NEXT_PAGE_INT                            0x0002
#define RX_BASE_PAGE_INT                            0x0080
#define RX_NEXT_PAGE_INT                            0x0010
#define BPAN_COMPLETE_INT                           0x0100
#define FEC_ALIGNMENT_MAKER_LOST_LANE0_STATUS_VAL               0x0001
#define FEC_ALIGNMENT_MAKER_LOST_LANE1_STATUS_VAL               0x0002
#define FEC_ALIGNMENT_MAKER_LOST_LANE2_STATUS_VAL               0x0004
#define FEC_ALIGNMENT_MAKER_LOST_LANE3_STATUS_VAL               0x0008
#define FEC_UNCORRECTABLE_FRM_STATUS_VAL                    0x0010
#define FEC_DESKEW_LOST_STATUS_VAL                      0x0020
#define FEC_BER_OVER_THRESHOLD_VAL                      0x0040
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE0_INT_VAL             0x0001
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE1_INT_VAL             0x0002
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE2_INT_VAL             0x0004
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE3_INT_VAL             0x0008
#define UMAC_RS_FEC_UNCORRECTABLE_FRM_INT_VAL                   0x0010
#define UMAC_RS_FEC_DESKEW_LOST_INT_VAL                     0x0020
#define UMAC_RS_FEC_BER_OVER_THRESHOLD_INT_VAL                  0x0040

#define UMAC_STAT_CNT_STATUS_BITMASK                        0x8000
#define UMAC_RX_CRC_BITMASK                         0x0001
#define UMAC_RX_FLOW_CONTROL_DECODE_BITMASK                 0x0020
#define UMAC_RX_EXTRACT_CUSTOM_PREAMBLE_BITMASK                 0x0040
#define UMAC_RX_PREAMBLE_LENGTH_4BYTE_BITMASK                   0x0080
#define UMAC_MAXFRMSIZE_WITH_VLAN_BITMASK                   0x000C
#define UMAC_RX_STRIP_FCS_BITMASK                       0x0002
#define UMAC_RX_PROMISCUOUS_MODE_BITMASK                    0x0010
#define UMAC_TX_IFGLENGTH_MASKBIT                       0x00FC
#define UMAC_TX_IFG_CONTROL_PER_FRAME_MASKBIT                   0x4000
#define UMAC_TX_CRC_BITMASK                         0x0001
#define UMAC_TX_FLOW_CONTROL_BITMASK                        0x0100
#define UMAC_TX_PRIORITYFLOW_CONTROL_BITMASK                    0x0200
#define UMAC_TX_FCS_INVERT_BITMASK                      0x0002
#define UMAC_TX_PREAMBLE_INSERTION_BITMASK                  0x0400
#define UMAC_TX_PREAMBLE_LENGTH_BITMASK                     0x3800
#define UMAC_TX_PREAMBLE_LENGTH_BITFIELD                    11
#define UMAC_PCS_INTERFACE_LOOPBACK_BITMASK                 0x0008
#define UMAC_FIFO_INTERFACE_LOOPBACK_BITMASK                    0x0010
#define UMAC_CHANNEL_SGMII_LOOPBACK_BITMASK                 0x4000
#define UMAC_CHANNEL_SGMIISERDES_LOOPBACK_BITMASK               0x8000
#define UMAC_TX_UNDERRUN_INT_ENABLE_BITMASK                 0x0001
#define UMAC_TX_JABBER_INT_ENABLE_BITMASK                   0x0002
#define UMAC_TX_TIMESTAMPS_FIFO_OVER_FLOW_BITMASK               0x0004
#define UMAC_PCS_LOOPBACK_BITMASK                       0x4000
#define UMAC_PCS_FAULT_BITMASK                          0x0080
#define UMAC_SGMII_AUTO_NEGOTIATION_BITMASK                                     0x0001
#define UMAC_SGMII_AUTO_NEG_SYNC_DONE_BITMASK                       0x0002
#define UMAC_SGMII_AUTO_NEG_DONE_BITMASK                    0x0001
#define UMAC_SGMII_SPEED_GET_BITMASK                        0x0C00
#define UMAC_RXTX_BITMASK                           0x0003
#define UMAC_SOFT_RESET_BITMASK                         0x0004
#define UMAC_STATISTIC_CNT_RESET_BITMASK                    0x0020
#define UMAC_RX_BITMASK                             0x0002
#define UMAC_TX_BITMASK                             0x0001
#define UMAC_TX_TIMESTAMP_ID_BITMASK                        0x0003
#define UMAC_TX_TIMESTAMP_VALUE_VALID_BITMASK                   0x8000
#define UMAC_PCS_RESET_BITMASK                          0x8000
#define UMAC_PCS_TEST_PATTERN_BITMASK                       0x0002
#define UMAC_PCS_RX_TEST_MODE_BITMASK                       0x0004
#define UMAC_PCS_TX_TEST_MODE_BITMASK                       0x0008
#define UMAC_PCS_PRBS31_TX_BITMASK                      0x0010
#define UMAC_PCS_PRBS31_RX_BITMASK                      0x0020
#define UMAC_PCS_PRBS9_TX_BITMASK                       0x0040
#define UMAC_PCS_SCRAMBLED_IDEA_TEST_PATTERN_BITMASK                0x0080
#define UMAC_PCS_LOCK_BITMASK                           0x0001
#define UMAC_PCS_BLOCK_LOCK_1_BIT_MASK                      0x00FF
#define UMAC_PCS_BLOCK_LOCK_2_BIT_MASK                      0x0FFF
#define UMAC_PCS_LOWER_NIBBLE_BIT_MASK                      0x000F
#define UMAC_PCS_HI_BER_BITMASK                         0x0002
#define UMAC_PCS_OPERATIONAL_STATE_BITMASK                  0x1000
#define UMAC_PCS_ALIGNMENT_BITMASK                      0x1000
#define UMAC_PCS_AM_BIT_MASK_1                          0x00FF
#define UMAC_PCS_AM_BIT_MASK_2                          0x0FFF
#define UMAC_TX_GEARBOX_BITMASK                         0x000F
#define UMAC_PCS_DEBUG_BIT_MASK_1                       0xFFFF
#define UMAC_PCS_ENABLE_BITMASK                         0x0001
#define UMAC_PCS_BYPASS_SCRAMBLER_BITMASK                   0x0002
#define UMAC_PCS_USE_SHORT_TIMER_BITMASK                    0x0004
#define UMAC_PCS_IGNORE_SIGNAL_OK_BITMASK                   0x0008
#define UMAC_PCS_DOUBLE_SPEED_BITMASK                       0x0001
#define UMAC_PCS_10G_ENCODING_BITMASK                       0x0002
#define UMAC_PCS_BYPASS_BUFFER_BITMASK                      0x0010
#define UMAC_PCS_LOW_LATENCY_MODE_BITMASK                   0x2000
#define UMAC_FEC_ENABLE_BITMASK                         0x0002
#define UMAC_PCS_DECODE_TRAP_BITMASK                        0x1C00
#define UMAC_PCS_FULL_THRESHOLD_BITMASK                     0x03C0
#define UMAC_PCS_DATA_PATTERN_SELECT_BITMASK                    0x0001
#define UMAC_PAUSE_FRAME_BIT_STATUS_BITMASK                 0x0001
#define UMAC_PCS_LANE_MAPPING_BITMASK                       0x001F
#define UMAC_PCS_CH1_LANE_MAPPING_BITMASK                   0x0003
#define UMAC_PCS_CH0_INTERRUPT_ENABLE_BITMASK                   0x0010
#define UMAC_HSPCS_CH0_INTERRUPT_ENABLE_BITMASK                 0x0010
#define UMAC_LSPCS_CH0_INTERRUPT_ENABLE_BITMASK                 0x0010
#define UMAC_LOWER_MAC_ADDR_BITMASK                     0xFFFFULL
#define UMAC_MID_MAC_ADDR_BITMASK                       0xFFFF0000ULL
#define UMAC_HIGH_MAC_ADDR_BITMASK                      0xFFFF00000000ULL
#define UMAC_LOW_WORD_BITMASK                           0xFFFFULL
#define UMAC_MID1_WORD_BITMASK                          0xFFFF0000ULL
#define UMAC_MID2_WORD_BITMASK                          0xFFFF00000000ULL
#define UMAC_HIGH_WORD_BITMASK                          0xFFFF000000000000ULL
#define UMAC_FEC_10G_ABILITY_BITMASK                        0x0001
#define UMAC_FEC_10G_SOFT_RESET_ENA_BITMASK                 0x0001
#define UMAC_FEC_10G_SOFT_RESET_FORCE_BITMASK                   0x0002
#define UMAC_FEC_10G_ERR_INDICATION_ABILITY_BITMASK             0x0002
#define UMAC_FEC_10G_ENA_BITMASK                        0x0001
#define UMAC_FEC_10G_ERR_INDICATION_ABILITY_BITMASK             0x0002
#define UMAC_FEC_10G_ERR_TO_PCS_BITMASK                     0x0002

#define UMAC_FEC_ENA_BITMASK                            0x0002
#define UMAC_FEC_100G_SOFT_RESET_ENA_BITMASK                    0x0001
#define UMAC_FEC_100G_BYPASS_CORRE_ABILITY_BITMASK              0x0002
#define UMAC_FEC_100G_ERR_INDI_ABILITY                      0x0004
#define UMAC_FEC_100G_RS_ABILITY_BITMASK                    0x0001
#define UMAC_FEC_100G_ENA_ERR_INDI_BITMASK                  0x0002
#define UMAC_FEC_100G_BYPASS_ENA_BITMASK                    0x0001
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE0_INT_BITMASK         0x0001
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE1_INT_BITMASK         0x0002
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE2_INT_BITMASK         0x0004
#define UMAC_RS_FEC_ALLIGNMENT_MAKER_LOST_LANE3_INT_BITMASK         0x0008
#define UMAC_RS_FEC_UNCORRECTABLE_FRM_INT_BITMASK               0x0010
#define UMAC_RS_FEC_DESKEW_LOST_INT_BITMASK                 0x0020
#define UMAC_RS_FEC_BER_OVER_THRESHOLD_INT_BITMASK              0x0040

#define UMAC_BPAN_TX_BASE_PAGE_STARTED_BITMASK                  0x0001
#define UMAC_BPAN_TX_NEXT_PAGE_STARTED_BITMASK                  0x0002
#define UMAC_BPAN_RX_BASE_PAGE_STARTED_BITMASK                  0x0008
#define UMAC_BPAN_RX_NEXT_PAGE_STARTED_BITMASK                  0x0010
#define UMAC_BPAN_COMPLETED_BITMASK                     0x0100
#define UMAC_BPAN_AN_BITMASK                            0x0001
#define UMAC_BPAN_NORMAL_MODE_BITMASK                       0x0002
#define UMAC_BPAN_PN_SEQUENCE_BITMASK                       0x0004
#define UMAC_BPAN_ARBITRATION_FUN_BITMASK                   0x0008
#define UMAC_BPAN_SELECTOR_BITMASK                      0x01F0
#define UMAC_BPAN_SOFT_RESET_BITMASK                        0x0200
#define UMAC_BPAN_TX_BITMASK                            0x0400
#define UMAC_BPAN_RX_BITMASK                            0x0800
#define UMAC_BPAN_RXGB_RESYNC_BITMASK                       0x1000
#define UMAC_BPAN_NEXT_PAGE_TRANS_BITMASK                   0x2000
#define UMAC_BPAN_TNONCE_FORCE_BITMASK                      0x0020
#define UMAC_BPAN_TNONCE_FORCE_VAL_BITMASK                  0x001F
#define UMAC_BPAN_ECHOED_NONCE_FORCE_BITMASK                    0x0020
#define UMAC_BPAN_ECHOED_NONCE_FORCE_VAL_BITMASK                0X001F
#define UMAC_BPAN_FCE_ABILITY_ENABLE_BITMASK                    0x0001
#define UMAC_BPAN_FCE_REQUEST_ENABLE_BITMASK                    0x0002
#define UMAC_BPAN_REMOTE_FAULT_BITMASK                      0x0004
#define UMAC_BPAN_ACK_STATUS_BITMASK                        0x0008
#define UMAC_BPAN_PAUSE_ABILITY_BITMASK                     0x0030
#define UMAC_BPAN_NEXT_PAGE_BITMASK                     0x0020
#define UMAC_BPAN_REMOTE_FCE_ABILITY_ENABLE_BITMASK             0x0001
#define UMAC_BPAN_REMOTE_FCE_REQUEST_ENABLE_BITMASK             0x0002
#define UMAC_BPAN_REMOTE_RFAULT_BITMASK                     0x0004
#define UMAC_BPAN_REMOTE_ACK_STATUS_BITMASK                 0x0008
#define UMAC_BPAN_REMOTE_PAUSE_ABILITY_BITMASK                  0x0030
#define UMAC_BPAN_REMOTE_NEXT_PAGE_BITMASK                  0x0020
#define UMAC_BPAN_TX_LOWER_CODE_FIELD_BITMASK                   0x07FF
#define UMAC_BPAN_TX_ACK2_BITMASK                       0x1000
#define UMAC_BPAN_TX_MP_BITMASK                         0x2000
#define UMAC_BPAN_TX_ACK_BITMASK                        0x4000
#define UMAC_BPAN_TX_NEXT_PAGE_BITMASK                      0x8000
#define UMAC_BPAN_REMOTE_RX_LOWER_CODE_FIELD_BITMASK                0x07FF
#define UMAC_BPAN_REMOTE_RX_ACK2_BITMASK                    0x1000
#define UMAC_BPAN_REMOTE_RX_MP_BITMASK                      0x2000
#define UMAC_BPAN_REMOTE_RX_ACK_BITMASK                     0x4000
#define UMAC_BPAN_REMOTE_RX_NEXT_PAGE_BITMASK                   0x8000
#define UMAC_QSGMII_TXRX_SERDES_LOOPBACK_BITMASK                0x8000
#define UMAC_QSGMII_RXTX_GMII_LOOPBACK_BITMASK                  0x4000
#define UMAC_SGMII_AUTO_NEGOTIATION_RESTART_BITMASK                             0x0002
#define UMAC_GLOBAL_MODE_REG_BITMASK                        0xFF
#define UMAC_GLOBAL_MODE_SELECT_BITMASK                     0xF
#define UMAC_GLOBAL_MODE_MIX_FIELD_BITMASK                  0xF0
#define UMAC_SGMII_MODE_BITMASK                         0xF
#define UMAC_CLEAR_LINK_AND_FAULT_INT                                   0x00FF

#define UMAC_PCS_ENABLE                             0x0001
#define UMAC_PCS_BYPASS_SCRAMBLER                       0x0002
#define UMAC_PCS_USE_SHORT_TIMER                        0x0004
#define UMAC_PCS_IGNORE_SIGNAL_OK                       0x0008

#define UMAC_CHANNEL0_LINK_CHANGE                       0x0001
#define UMAC_CHANNEL0_LINK_UP                           0x0001

#define UMAC_CHANNEL1_LINK_CHANGE                       0x0002
#define UMAC_CHANNEL1_LINK_UP                           0x0002

#define UMAC_CHANNEL2_LINK_CHANGE                       0x0004
#define UMAC_CHANNEL2_LINK_UP                           0x0004

#define UMAC_CHANNEL3_LINK_CHANGE                       0x0008
#define UMAC_CHANNEL3_LINK_UP                           0x0008

#define UMAC_CHANNEL0_PCS10GB_SELECT                        0xFFEF
#define UMAC_CHANNEL0_MIXMODE_SGMII_SELECT                  0x0010

#define UMAC_CHANNEL1_PCS10GB_SELECT                        0xFFDF
#define UMAC_CHANNEL1_MIXMODE_SGMII_SELECT                  0x0020

#define UMAC_CHANNEL2_PCS10GB_SELECT                        0xFFBF
#define UMAC_CHANNEL2_MIXMODE_SGMII_SELECT                  0x0040

#define UMAC_CHANNEL3_PCS10GB_SELECT                        0xFF7F
#define UMAC_CHANNEL3_MIXMODE_SGMII_SELECT                  0x0080

#define UMAC_CHANNEL_SGMII_LOOPBACK_ENABLE                  0x4000
#define UMAC_CHANNEL_SGMII_LOOPBACK_DISABLE                 0xBFFF

#define UMAC_CHANNEL_SGMIISERDES_LOOPBACK_ENABLE                0x8000
#define UMAC_CHANNEL_SGMIISERDES_LOOPBACK_DISABLE               0x7FFF

#define UMAC_CONTROL_REG_DEFAULT_VALUE                      0x0
#define UMAC_TX_CONFIG_DEFAULT_VALUE                        0x48
#define UMAC_RX_CONFIG_DEFAULT_VALUE                        0x01

#define UMAC_SGMII_AUTO_NEGOTIATION_ENABLE                  0x0001
#define UMAC_SGMII_AUTO_NEGOTIATION_DISABLE                     0xDFFF

#define UMAC_SGMII_AUTO_NEGOTIATION_RESTART                 0x0002

#define UMAC_SGMII_AUTO_NEG_SYNC_DONE                       0x0002
#define UMAC_SGMII_AUTO_NEG_DONE                            0x0001

#define UMAC_QSGMII_TXRX_SERDES_LOOPBACK_ENABLE                 0x8000
#define UMAC_QSGMII_RXTX_GMII_LOOPBACK_ENABLE                   0x4000
#define UMAC_QSGMII_AUTO_NEG_SYNC_STATUS_BITMASK            0x0010
#define UMAC_SOFT_RESET_ALL_CHAN_BITMASK                    0x0001
#define UMAC_TX_FIFO_THRESOLD_4CH_BITMASK                    0x000F
#define UMAC_TX_FIFO_THRESOLD_2CH_BITMASK                    0x00FF
#define UMAC_TX_FIFO_THRESOLD_1CH_BITMASK                    0x1F00
#define UMAC_FIFO_THRESOLD_1CH_BIT_POS                       8
#define UMAC_TX_INTERFACE_TYPE_BITMASK                       0x2000
#define UMAC_RX_INTERFACE_TYPE_BITMASK                       0x4000
#define UMAC_BPAN_RX_SELECTOR_BITMASK                       0x001F
#define UMAC_BPAN_RX_NONCE_BITMASK                      0x1F00
#define UMAC_AN_RXNONCE_BIT_POS                         8
#define UMAC_TX_ACTIVE_BITMASK                          0x0001
#define UMAC_TX_ERROR_BITMASK                           0x0002
#define UMAC_TX_UNDERUN_BITMASK                         0x0004
#define UMAC_TX_PAUSE_BITMASK                           0x0008
#define UMAC_RX_ACTIVE_BITMASK                          0x0010
#define UMAC_RX_CRC_ERROR_BITMASK                       0x0020
#define UMAC_RX_ERROR_BITMASK                           0x0040
#define UMAC_RX_OVER_FLOW_BITMASK                       0x0080
#define UMAC_RX_PAUSE_BITMASK                           0x0100
#define UMAC_FEC_ERROR_BLK_INT_BITMASK                      0x0001
#define UMAC_FEC_UNCORRECTED_BLK_INT_BITMASK                    0x0002
#define UMAC_FEC_UNCORRECTED_BLK_BIT_POSITION                   1
#define UMAC_FEC_BLK_LOCK_INT_BITMASK                       0x0004
#define UMAC_FEC_BLK_LOCK_LOST_INT_BITMASK                  0x0008
#define UMAC_FEC_BLK_LOCK_CHANGED_INT_BITMASK                   0x0010
#define UMAC_FEC_ERROR_BLK_INT_ENABLE_BITMASK                   0x0001
#define UMAC_FEC_ERROR_BLK_INT_ENABLE                       0x0001
#define UMAC_FEC_UNCORRECTED_BLK_INT_ENABLE_BITMASK                 0x0002
#define UMAC_FEC_UNCORRECTED_BLK_INT_ENABLE                 0x0002
#define UMAC_FEC_BLK_LOCK_INT_ENABLE_BITMASK                    0x0004
#define UMAC_FEC_BLK_LOCK_INT_ENABLE                        0x0004
#define UMAC_FEC_BLK_LOCK_LOST_INT_ENABLE_BITMASK               0x0008
#define UMAC_FEC_BLK_LOCK_LOST_INT_ENABLE                   0x0008
#define UMAC_FEC_BLK_LOCK_CHANGED_INT_ENABLE_BITMASK                0x0010
#define UMAC_FEC_BLK_LOCK_CHANGED_INT_ENABLE                    0x0010
#define UMAC_FEC_ERROR_BLK_INT_TEST_BITMASK                     0x0001
#define UMAC_FEC_ERROR_BLK_INT_TEST_ENABLE                  0x0001
#define UMAC_FEC_UNCORRECTED_BLK_INT_TEST_BITMASK               0x0002
#define UMAC_FEC_UNCORRECTED_BLK_INT_TEST_ENABLE                0x0002
#define UMAC_FEC_BLK_LOCK_INT_TEST_BITMASK                  0x0004
#define UMAC_FEC_BLK_LOCK_INT_TEST_ENABLE                   0x0004
#define UMAC_FEC_BLK_LOCK_LOST_INT_TEST_BITMASK                 0x0008
#define UMAC_FEC_BLK_LOCK_LOST_INT_TEST_ENABLE                  0x0008
#define UMAC_FEC_BLK_LOCK_CHANGED_INT_TEST_BITMASK              0x0010
#define UMAC_FEC_BLK_LOCK_CHANGED_INT_TEST_ENABLE               0x0010
#define UMAC_SGMII_INTERRUPT_ENABLE_BITMASK                  0x0001
#define UMAC_SGMII_INTERRUPT_ENABLE                      0x0001
#define UMAC_SGMII_INTERRUPT_TEST_BITMASK                   0x8000
#define UMAC_SGMII_INTERRUPT_TEST_ENABLE                    0x8000
#define UMAC_MIB_COUNTER_CLEAN_ON_READ_BITMASK                   0x2000
#define UMAC_MIB_COUNTER_CLEAN_ON_READ_ENABLE                    0x2000
#define UMAC_MIB_COUNTER_PRIORITY_READ_BITMASK                   0x4000
#define UMAC_MIB_COUNTER_PRIORITY_READ_ENABLE                    0x4000
#define UMAC_TX_DRAIN_BITMASK                           0x0040
#define UMAC_EARLY_EOF_DETECTION_BITMASK                    0x0200
#define UMAC_DISPARITY_CHECK_BITMASK                         0x0100
#define UMAC_DISPARITY_CHECK_ENABLE                      0x0100
#define UMAC_DEBUG_Short_AMP_BITMASK                         0x0004
#define UMAC_DEBUG_Short_AMP_ENABLE                      0x0004
#define UMAC_DEBUG_TEST_INIT_BITMASK                         0x0008
#define UMAC_DEBUG_TEST_INIT_ENABLE                      0x0008
#define UMAC_CW_TEST_STOP_ASM_BITMASK                       0x0010
#define UMAC_CW_TEST_STOP_ASM_ENABLE                        0x0010
#define UMAC_BPAN_SERDES_TX_SEL_BITMASK                     0x0002
#define UMAC_BPAN_MODE_BITMASK                      0x0008
#define UMAC_RX_GEARBOX_RE_SYNC_BITMASK                     0x1000
#define UMAC_BPAN_TX_TOGGEL_BITMASK                         0x0800
#define UMAC_BPAN_RX_TOGGEL_BITMASK                         0x0800
#define UMAC_TX_INTERFACE_TYPE_VALUE                        0x2000
#define UMAC_RX_INTERFACE_TYPE_VALUE                        0x4000
#define UMAC_RS_FEC_INTERRUPT_BITMASK                       0x0001
#define UMAC_RS_FEC_AMP_LOCK_BITMASK                        0x00F0
#define UMAC_RS_FEC_AMP_LOCK_BIT_POS                        4
#define UMAC_RS_FEC_LAN0_MAP_BITMASK                        0x0300
#define UMAC_RS_FEC_LAN0_MAP_BIT_POS                        8
#define UMAC_RS_FEC_LAN1_MAP_BITMASK                        0x0C00
#define UMAC_RS_FEC_LAN1_MAP_BIT_POS                        10
#define UMAC_RS_FEC_LAN2_MAP_BITMASK                        0x3000
#define UMAC_RS_FEC_LAN2_MAP_BIT_POS                        12
#define UMAC_RS_FEC_LAN3_MAP_BITMASK                        0xC000
#define UMAC_RS_FEC_LAN3_MAP_BIT_POS                        14

#define UMAC_TX_CONFIG_INIT_VAL                         0x8030
#define UMAC_TX_CONFIG_INIT_VAL_10G_40G                     0x802C //bits 2-7 IPG
#define UMAC_RX_MAX_FRM_LEN_INIT_VAL                        16000
#define UMAC_TX_JABBER_FRM_INIT_VAL                     16016
#define UMAC_RX_JABBER_FRM_INIT_VAL                     16016
#define UMAC_TX_PRIORITY_PAUSE_VECTOR_INIT_VAL                  0x00FF
/* mdio register values */
#define UMAC_MDIO_CHECK_BUSY                            0x8000
#define UMAC_TABLE_ENTRY_BIT_WIDTH                      (64)
#define UMAC_XON_PAUSE_TIME_VALUE                       0x00FF
#define UMAC_CHANNEL_ENABLE_MASK                        0x0F00
#define UMAC_TWO_CHANNEL_ENABLE_MASK                        0x0300
#define UMAC_CHANNEL_ENABLE_AND_MAC_MODE_MASK                   0x0F0F
#define UMAC_LINK_STATUS_INT_BIT                        3
#define UMAC_FAULT_STATUS_INT_BIT                       7
#define UMAC_SERDES_SIG_STATUS_INT_BIT                      11
#define BIT_OFFSET_LINK_STATUS_CH0_INT                      0
#define BIT_OFFSET_SERDES_SIG_STATUS_CH3_INT                    15
#define UMAC_MAX_SERDES_LANE_NUM                        8

/**
 * \brief Register address' common enum of A0 and B0
 */
typedef enum xpRegAddr_e
{
    UMAC_GLOBAL_MODE_REG_ADDR,
    UMAC_IP_VERSION,
    UMAC_LINK_STATUS_OVERRIDE_REG_ADDR,
    UMAC_LINK_STATUS_REG_ADDR,
    UMAC_GLOBAL_SGMII_MODE_REG_ADDR,
    UMAC_MDIO_CONTROL_OFFSET_ADDR,
    UMAC_MDIO_DATA_OFFSET_ADDR,
    UMAC_MDIO_CLOCK_OFFSET_ADDR,
    UMAC_SPARE1,                    //NA
    UMAC_SPARE2,                    //NA
    UMAC_DEFINE_INDICATION,         //NA
    UMAC_PRODUCT_ID,                //NA
    UMAC_STATISTICS_RD_CNT_ADDR,
    UMAC_STATISTICS_RD_DATA0_ADDR,
    UMAC_STATISTICS_RD_DATA1_ADDR,
    UMAC_STATISTICS_RD_DATA2_ADDR,
    UMAC_STATISTICS_RD_DATA3_ADDR,
    UMAC_LED_INFO_REG_ADDR,
    UMAC_SOFT_RESET_REG_ADDR,
    UMAC_SOFT_RESET_ALL_CHAN_REG_ADDR,
    UMAC_RESET_STATISTICS_COUNTER,      //NA
    UMAC_INIT_STATUS,               //NA
    UMAC_CONTROL_REG_CHANNEL0_ADDR,
    UMAC_TX_CONFIG_CHANNEL0_ADDR,
    UMAC_RX_CONFIG_CHANNEL0_ADDR,
    UMAC_RX_MAX_FRAME_LEN_CHANNEL0_ADDR,
    UMAC_TRANSMIT_JABBER_SIZE_CH0_REG_ADDR,
    UMAC_RECEIVE_JABBER_SIZE_CH0_REG_ADDR,
    UMAC_TX_PRIORITY_PAUSE_VECTOR_CH0_REG_ADDR,
    UMAC_VLAN_TAG1_CH0_REG_ADDR,
    UMAC_VLAN_TAG2_CH0_REG_ADDR,
    UMAC_VLAN_TAG3_CH0_REG_ADDR,
    UMAC_MAC_ADDRESS_LOW_REG_CH0_ADDR,
    UMAC_MAC_ADDRESS_MID_REG_CH0_ADDR,
    UMAC_MAC_ADDRESS_HIGH_REG_CH0_ADDR,
    UMAC_MULTICAST_HASH_TABLE1_CH0_REG_ADDR,
    UMAC_MULTICAST_HASH_TABLE2_CH0_REG_ADDR,
    UMAC_MULTICAST_HASH_TABLE3_CH0_REG_ADDR,
    UMAC_MULTICAST_HASH_TABLE4_CH0_REG_ADDR,
    UMAC_FLOW_CTRL_FRM_GENERATE_CTRL_CH0_REG_ADDR,
    UMAC_FLOW_CTRL_FRM_DEST_LOWMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_FRM_DEST_MIDMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_FRM_DEST_HIGHMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_FRM_SRC_LOWMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_FRM_SRC_MIDMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_FRM_SRC_HIGHMAC_ADDR_REG_CH0_ADDR,
    UMAC_FLOW_CTRL_PAUSE_TIME_CH0_REG_ADDR,
    UMAC_XOFF_PAUSE_TIME_CH0_REG_ADDR,
    UMAC_XON_PAUSE_TIME_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_INFO_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_VAL0_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_VAL1_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_VAL2_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_VAL3_CH0_REG_ADDR,
    UMAC_TX_TIMESTAMP_DELTA_REG_ADDR,
    UMAC_RX_TIMESTAMP_DELTA_REG_ADDR,
    UMAC_MIN_FRAME_SIZE_REG_ADDR,            //NA
    UMAC_TX_VLAN_TAG_REG_ADDR,               //NA
    UMAC_PAUSE_SLOT_CLOCK_COUNT,             //NA
    UMAC_TX_DEBUG_REG_ADDR,                  //NA
    UMAC_CHANNEL0_SPARE0_REG_ADDR,           //NA
    UMAC_PAUSE_uSEC_COUNTER_REG_ADDR,        //NA
    UMAC_PORT_GROUP_DMA_FIFO_CONTROL_REG_ADDR,    //B0-(FIFO_CONTROL_1)
    UMAC_APP_FIFO_LOOPBACK_REG_ADDR,
    UMAC_APP_FIFO_LOOPBACK_THRESHOLD_REG_ADDR,
    UMAC_APP_FIFO_PORT_MAPPING_0_REG_ADDR,
    UMAC_RX_FIFO_CONTROL_0_REG_ADDR,
    UMAC_RX_FIFO_CONTROL_1_REG_ADDR,
    UMAC_RX_FIFO_CONTROL_2_REG_ADDR,
    UMAC_RX_FIFO_CONTROL_3_REG_ADDR,
    UMAC_TX_FIFO_CONTROL_0_REG_ADDR,
    UMAC_TX_FIFO_CONTROL_1_REG_ADDR,
    UMAC_TX_FIFO_CONTROL_2_REG_ADDR,
    UMAC_TX_FIFO_CONTROL_3_REG_ADDR,
    UMAC_CHANNEL_MAPPING_0_REG_ADDR,
    UMAC_CHANNEL_MAPPING_1_REG_ADDR,
    UMAC_FIFO_CONTROL_2_REG_ADDR,
    UMAC_PARITY_CONTROL_REG_ADDR,
    UMAC_FIFO_SPARE_1_REG_ADDR,
    UMAC_FIFO_SPARE_0_REG_ADDR,
    UMAC_PCS_CONTROL_REG_CHANNEL0_ADDR,
    UMAC_PCS_STATUS1_CHANNEL0_ADDR,
    UMAC_PCS_BASE_R_10GBASE_T_CH0_STATUS1_ADDR,
    UMAC_PCS_BASE_R_10GBASE_T_CH0_STATUS2_ADDR,
    UMAC_PCS_TEST_PATTERN_SEED_A_CH0_ADDR0,
    UMAC_PCS_TEST_PATTERN_SEED_A_CH0_ADDR1,
    UMAC_PCS_TEST_PATTERN_SEED_A_CH0_ADDR2,
    UMAC_PCS_TEST_PATTERN_SEED_A_CH0_ADDR3,
    UMAC_PCS_TEST_PATTERN_SEED_B_CH0_ADDR0,
    UMAC_PCS_TEST_PATTERN_SEED_B_CH0_ADDR1,
    UMAC_PCS_TEST_PATTERN_SEED_B_CH0_ADDR2,
    UMAC_PCS_TEST_PATTERN_SEED_B_CH0_ADDR3,
    UMAC_PCS_TEST_PATTERN_CONTROL_CH0_ADDR,
    UMAC_PCS_TEST_PATTERN_ERROR_COUNTER_CH0_ADDR,
    UMAC_PCS_BER_HIGH_ORDER_COUNTER_CH0_ADDR,
    UMAC_PCS_ERR_BLOCKS_HIGH_ORDER_COUNTER0_CH0_ADDR,
    UMAC_PCS_MUL_LANE_BASE_R_ALIGMENT_STATUS1_CH0_ADDR,
    UMAC_PCS_MUL_LANE_BASE_R_ALIGMENT_STATUS2_CH0_ADDR,
    UMAC_PCS_MUL_LANE_BASE_R_ALIGMENT_STATUS3_CH0_ADDR,
    UMAC_PCS_MUL_LANE_BASE_R_ALIGMENT_STATUS4_CH0_ADDR,
    UMAC_PCS_LANE_X_MAPPING_CH0_ADDR,
    UMAC_PCS_LANE_X_MAPPING_CH1_ADDR,
    UMAC_PCS_MULTI_LANE_BIP_ERR_CNT_CH0_ADDR,
    UMAC_PCS_MULTI_LANE_BIP_ERR_CNT_CH1_ADDR,
    UMAC_PCS_DEBUG_CONTROL_REG_ADDR,
    UMAC_PCS_DEBUG_CONTROL2_REG_ADDR,
    UMAC_PCS_CHANNEL0_MODE_ADDR,
    UMAC_PCS_MLG_CONFIG_REG,            //NA
    UMAC_PCS_COUNTER_0_REG_ADDR,        //NA
    UMAC_PCS_COUNTER_1_REG_ADDR,        //NA
    UMAC_PCS_COUNTER_2_REG_ADDR,        //NA
    UMAC_PCS_AM_CONTROL_REG_ADDR,       //NA
    UMAC_PCS_AM_0_LOW_REG_ADDR,         //NA
    UMAC_PCS_AM_0_HIGH_REG_ADDR,        //NA
    UMAC_PCS_AM_1_LOW_REG_ADDR,         //NA
    UMAC_PCS_AM_1_HIGH_REG_ADDR,        //NA
    UMAC_PCS_AM_2_LOW_REG_ADDR,         //NA
    UMAC_PCS_AM_2_HIGH_REG_ADDR,        //NA
    UMAC_PCS_AM_3_LOW_REG_ADDR,         //NA
    UMAC_PCS_AM_3_HIGH_REG_ADDR,        //NA
    UMAC_PCS_BIT_ERROR_GAP_ADDR,        //NA
    UMAC_PCS_SYNC_HEADER_GAP_ADDR,           //NA
    UMAC_PCS_DEBUG_CODE_GROUP_0_REG_ADDR,    //NA
    UMAC_PCS_DEBUG_CODE_GROUP_1_REG_ADDR,    //NA
    UMAC_PCS_DEBUG_CODE_GROUP_2_REG_ADDR,    //NA
    UMAC_PCS_DEBUG_CODE_GROUP_3_REG_ADDR,    //NA
    UMAC_PCS_DEBUG_CODE_GROUP_4_REG_ADDR,    //NA
    UMAC_PCS_BER_STATE_MACHINE_OVERRIDE,     //NA
    UMAC_PCS_SPARE0,                         //NA
    UMAC_FEC_RS_STATUS_CH0_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_CORRECTED_COUNTER_LOW_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_CORRECTED_COUNTER_HIGH_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_UNCORRECTED_COUNTER_LOW_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_UNCORRECTED_COUNTER_HIGH_REG_ADDR,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_0_LOW,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_0_HIGH,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_1_LOW,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_1_HIGH,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_2_LOW,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_2_HIGH,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_3_LOW,
    UMAC_RS_FEC_SYMBOL_ERR_COUNTER_LANE_3_HIGH,
    UMAC_RS_FEC_CHANNEL_0_STATUS_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_DEBUG_CONTROL_REG_ADDR,
    UMAC_RS_FEC_STATUS_REG_ADDR,
    UMAC_RS_FEC_LANE_BIP_ERR_CNT_REG_ADDR,
    UMAC_FEC_RS_INTERRUPT_ENABLE_CH0_REG_ADDR,
    UMAC_FEC_RS_INTERRUPT_CLEAR_CH0_REG_ADDR,
    UMAC_FEC_100GB_CTRL_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_CONTROL_REG_ADDR,
    UMAC_RS_FEC_LANE_MAPPING_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_LANE_MAPPING_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_DEBUG_BER_INT_THRESHOLD_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_FORCE_INVALID_TIMER_LOW_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_FORCE_INVALID_TIMER_HIGH_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_SPARE_0_REG_ADDR,
    UMAC_RS_FEC_CHANNEL_0_IEEE_COMPATIBILITY_CONTROL_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_CONFIG_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_CORRECTED_BLOCKS_COUNTER_LOWER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_CORRECTED_BLOCKS_COUNTER_UPPER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_UNCORRECTED_BLOCKS_COUNTER_LOWER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_UNCORRECTED_BLOCKS_COUNTER_UPPER_REG_ADDR,
    UMAC_FEC_10G_ABILITY_CH0_ADDR,
    UMAC_FEC_SOFTWARE_RESET_REG_ADDR,
    UMAC_PORT_GROUP_FC_FEC_INTERRUPT_REG_ADDR,
    UMAC_PORT_GROUP_FC_FEC_INTERRUPT_ENABLE_REG_ADDR,
    UMAC_PORT_GROUP_FC_FEC_INTERRUPT_TEST_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_FORCE_INT0_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_STATUS_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_DEBUG_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_DEBUG_CORRECTED_BITS_COUNTER_UPPER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_DEBUG_CORRECTED_BITS_COUNTER_LOWER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_DEBUG_BLOCK_COUNT_UPPER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_DEBUG_BLOCK_COUNT_LOWER_REG_ADDR,
    UMAC_FC_FEC_CHANNEL_0_SPARE0_REG_ADDR,
    UMAC_QSGMII_CONTROL_REG_ADDR,
    UMAC_QSGMII_STATUS_REG_ADDR,
    UMAC_PORT_GROUP_QSGMII_INTERRUPT_ENABLE_REG_ADDR,
    UMAC_SGMII_CONTROL_CH0_ADDR,
    UMAC_SGMII_RX_CONFIG0_ADDR,
    UMAC_SGMII_STA0_ADDR,
    UMAC_SGMII_DISPARITY_ERR_CNT_ADDR,
    UMAC_SGMII_INVALID_CODE_COUNTER_ADDR,
    UMAC_PORT_GROUP_SGMII_INTERRUPT_ENABLE_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_ABILITY_ADVERTISEMENT_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_RX_CONFIG_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_LINK_TIMER_LOW_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_LINK_TIMER_HI_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_DISP_ERROR_COUNT_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_INV_CODE_COUNT_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_FORCE_INT0_REG_ADDR,
    UMAC_1G_PCS_CHANNEL_0_SPARE1_REG_ADDR,
    UMAC_BPAN_TNONCE_CH0_REG_ADDR,
    UMAC_BPAN_ECHOED_NONCE_CH0_REG_ADDR,
    UMAC_BPAN_ARBITRATION_CTRL_SETTING1_CH0_REG_ADDR,
    UMAC_BPAN_ARBITRATION_CTRL_SETTING2_CH0_REG_ADDR,
    UMAC_BPAN_CTRL1_CH0_REG_ADDR,
    UMAC_BPAN_TX_TECHNOLOGY_ABILITY_REGISTER_ADDR,
    UMAC_BPAN_CTRL2_CH0_REG_ADDR,
    UMAC_BPAN_NEXT_PAGE_CTRL1_CH0_REG_ADDR,
    UMAC_BPAN_REMOTE_NEXT_PAGE_CTRL1_CH0_REG_ADDR,
    UMAC_BPAN_RX_DME_BASE_PAGE_STATUS_REG2_ADDR,
    UMAC_BPAN_RX_DME_BASE_PAGE_STATUS_REG1_ADDR,
    UMAC_BPAN_REMOTE_CTRL1_CH0_REG_ADDR,
    UMAC_BPAN_TX_DME_NEXT_PAGE_STATUS2_REG_ADDR,
    UMAC_BPAN_TX_DME_NEXT_PAGE_STATUS3_REG_ADDR,
    UMAC_BPAN_RX_DME_NEXT_PAGE_STATUS2_REG_ADDR,
    UMAC_BPAN_RX_DME_NEXT_PAGE_STATUS3_REG_ADDR,
    UMAC_AN_INTERRUPT_ENABLE_CH0_REG_ADDR,
    UMAC_AN_INTERRUPT_STATUS_CHO_REG_ADDR,
    UMAC_BPAN_RX_CTRL_SETTING2_CH0_REG_ADDR,
    UMAC_BPAN_RX_CTRL_SETTING3_CH0_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_STATUS1_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_PAGE_TEST_MAX_TIMER_SETTING_2_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_PAGE_TEST_MIN_TIMER_SETTING_3_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_TX_BASE_PAGE_LOW_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_RX_BASE_PAGE_LOW_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_RX_BASE_PAGE_HIGH_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_TX_NEXT_PAGE_MID_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_TX_NEXT_PAGE_HIGH_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_RX_NEXT_PAGE_MID_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_RX_NEXT_PAGE_HIGH_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_FORCE_INT0_REG_ADDR,
    UMAC_BPAN_CHANNEL_0_EXTENDED_INT_REG_ADDR,
    UMAC_SERDES_SIGNAL_STATUS_OVERRIDE_REG_ADDR,
    UMAC_SERDES_LOOPBACK_ENABLE_REG_ADDR,
    UMAC_SERDES_LANE_REMAP_RX_0_REG_ADDR,
    UMAC_SERDES_LANE_REMAP_TX_0_REG_ADDR,
    UMAC_SERDES_FIFO_STATUS_0_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_1_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_2_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_3_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_4_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_5_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_6_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_STATUS_7_REG_ADDR,          //NA
    UMAC_SERDES_FIFO_ERROR_STATUS_REG_ADDR,
    UMAC_INTERRUPT_STATUS_REG_ADDR,
    UMAC_INTERRUPT_ENABLE_REG_ADDR,
    UMAC_PORT_GROUP_DMA_FIFOS_INTERRUPT_REG_ADDR,
    UMAC_PORT_GROUP_DMA_FIFOS_INTERRUPT_ENABLE_REG_ADDR,
    UMAC_INTERRUPT_STATUS_CH0_REG_ADDR,
    UMAC_INTERRUPT_ENABLE_CH0_REG_ADDR,
    UMAC_INTERRUPT_STATUS_CH1_REG_ADDR,
    UMAC_INTERRUPT_ENABLE_CH1_REG_ADDR,
    UMAC_INTERRUPT_STATUS_CH2_REG_ADDR,
    UMAC_INTERRUPT_ENABLE_CH2_REG_ADDR,
    UMAC_INTERRUPT_STATUS_CH3_REG_ADDR,
    UMAC_INTERRUPT_ENABLE_CH3_REG_ADDR,
    UMAC_PCS_DEBUGE_INTERRUPT_MASK_REG_ADDR,
    UMAC_PCS_DEBUGE_INTERRUPT_TEST_REG_ADDR,
    UMAC_PCS_DEBUG_STATUS_2_REG_ADDR,
    UMAC_PCS_DEBUG_STATUS_1_REG_ADDR,
    UMAC_LINK_LIVE_STATUS_REG_OFFSET_ADDR,
    UMAC_LINK_STATUS_INT_OFFSET_ADDR,
    UMAC_FORCE_INT0,
    UMAC_FORCE_INT_0_REG_ADDR,
    UMAC_FORCE_INT_1_REG_ADDR,
    UMAC_FORCE_INT_2_REG_ADDR,
    UMAC_FIFO_FORCE_INTERRUPT_0_REG_ADDR,
    UMAC_FIFO_FORCE_INTERRUPT_1_REG_ADDR,
    UMAC_PCS_FORCE_INT_0_REG_ADDR,
    UMAC_SET_INT_ENABLE0,
    UMAC_CLR_INT_ENABLE0,
    UMAC_SET_INT_ENABLE2,
    UMAC_CLR_INT_ENABLE2,
    UMAC_SET_INT_ENABLE3,
    UMAC_CLR_INT_ENABLE3,
    UMAC_SET_INT_ENABLE4,
    UMAC_CLR_INT_ENABLE4,
    UMAC_SET_INT_ENABLE5,
    UMAC_CLR_INT_ENABLE5,
    UMAC_SET_INT_ENABLE6,
    UMAC_CLR_INT_ENABLE6,
    UMAC_SET_INT_ENABLE7,
    UMAC_CLR_INT_ENABLE7,
    UMAC_SET_INT_ENABLE8,
    UMAC_CLR_INT_ENABLE8,
    UMAC_SET_INT_ENABLE9,
    UMAC_CLR_INT_ENABLE9,
    UMAC_SET_INT_ENABLE10,
    UMAC_CLR_INT_ENABLE10,
    UMAC_SET_INT_ENABLE11,
    UMAC_CLR_INT_ENABLE11,
    UMAC_SET_INT_ENABLE12,
    UMAC_CLR_INT_ENABLE12,
    UMAC_SET_INT_ENABLE13,
    UMAC_CLR_INT_ENABLE13,
    UMAC_SET_INT_ENABLE14,
    UMAC_CLR_INT_ENABLE14,
    UMAC_SET_INT_ENABLE15,
    UMAC_CLR_INT_ENABLE15,
    UMAC_INT_STATUS0,
    UMAC_INT_CLEAR0,
    UMAC_INT_STATUS1,
    UMAC_INT_CLEAR1,
    UMAC_INT_STATUS2,
    UMAC_INT_CLEAR2,
    UMAC_INT_STATUS3,
    UMAC_INT_CLEAR3,
    UMAC_INT_STATUS4,
    UMAC_INT_CLEAR4,
    UMAC_INT_STATUS5,
    UMAC_INT_CLEAR5,
    UMAC_INT_STATUS6,
    UMAC_INT_CLEAR6,
    UMAC_INT_STATUS7,
    UMAC_INT_CLEAR7,
    UMAC_INT_STATUS8,
    UMAC_INT_CLEAR8,
    UMAC_INT_STATUS9,
    UMAC_INT_CLEAR9,
    UMAC_INT_STATUS10,
    UMAC_INT_CLEAR10,
    UMAC_INT_STATUS11,
    UMAC_INT_CLEAR11,
    UMAC_INT_STATUS12,
    UMAC_INT_CLEAR12,
    UMAC_INT_STATUS13,
    UMAC_INT_CLEAR13,
    UMAC_INT_STATUS14,
    UMAC_INT_CLEAR14,
    UMAC_INT_STATUS15,
    UMAC_INT_CLEAR15,
    UMAC_PCS_AM_LEN_0_REG,
    UMAC_PCS_AM_LEN_1_REG,
    UMAC_PROG_AMP_LOCK_LEN_0,
    UMAC_PROG_AMP_LOCK_LEN_1,
    UMAC_RS_FEC_CHANNEL_0_PCW_AMP_LEN,
} xpsUmacRegAddr;

#endif
