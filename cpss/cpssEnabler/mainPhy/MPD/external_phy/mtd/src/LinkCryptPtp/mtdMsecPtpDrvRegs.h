/*******************************************************************************
Copyright (C) 2014 - 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
 * mtdMsecPtpDrvRegs.h
 *
 * DESCRIPTION:
 *       definitions of the register map of Marvell X-3xxx Device for Macsec/PTP
 *       registers.
 *
 * DEPENDENCIES:
 *
 * FILE REVISION NUMBER:
 *
 *******************************************************************************/
#ifndef __mtdMsecPtpDrvRegsh
#define __mtdMsecPtpDrvRegsh

#if C_LINKAGE
#ifdef __cplusplus
extern "C" {
#endif
#endif

#if MTD_LINKCRYPT_MSECPTP

/* Marvell X-314* Registers Device*/
#define MTD_REG_DEVICE_1                        0x00010000
#define MTD_REG_DEVICE_2                        0x00020000
#define MTD_REG_DEVICE_3                        0x00030000
#define MTD_REG_DEVICE_4                        0x00040000
#define MTD_REG_DEVICE_7                        0x00070000
#define MTD_REG_DEVICE_9                        0x00090000
#define MTD_REG_DEVICE_29                        0x001d0000
#define MTD_REG_DEVICE_30                        0x001e0000
#define MTD_REG_DEVICE_31                        0x001f0000


#define MTD_REG_T_ADV_PCS         0x00038000        /* MTD_REG_DEVICE_3, 8000 */
#define MTD_REG_X_COMMON_PCS      0x0003F000        /* MTD_REG_DEVICE_3, F000 */
#define MTD_REG_H_COMMON_PCS      0x0004F000        /* MTD_REG_DEVICE_4, F000*/

#define MTD_REG_WOL_DEVICE          MTD_REG_DEVICE_9
#define MTD_REG_MACSEC_CONFIG                    (MTD_REG_DEVICE_31+0xF000)

/* Marvell X-3xxx function Registers of T unit(line side, Copper)*/
#define MTD_REG_T_SPEC_CONTROL_1                (MTD_REG_T_ADV_PCS+0x0000)
#define MTD_REG_T_SPEC_CONTROL_2                (MTD_REG_T_ADV_PCS+0x0001)
#define MTD_REG_T_SPEC_CONTROL_3                (MTD_REG_T_ADV_PCS+0x0002)
#define MTD_REG_T_DOWN_SHIFT_CONTROL             (MTD_REG_T_ADV_PCS+0x0003)
#define MTD_REG_T_REF_CLK_CONTROL               (MTD_REG_T_ADV_PCS+0x0004)
#define MTD_REG_T_SPEC_STATUS_1                     (MTD_REG_T_ADV_PCS+0x0008)
#define MTD_REG_T_SPEC_STATUS_2                    (MTD_REG_T_ADV_PCS+0x0009)
#define MTD_REG_T_RECEIVE_ERR_COUNTER_0_15        (MTD_REG_T_ADV_PCS+0x000A)
#define MTD_REG_T_RECEIVE_ERR_COUNTER_16_31        (MTD_REG_T_ADV_PCS+0x000B)
#define MTD_REG_T_SPEC_INT_ENABLE_1                (MTD_REG_T_ADV_PCS+0x0010)
#define MTD_REG_T_SPEC_INT_1                      (MTD_REG_T_ADV_PCS+0x0011)
#define MTD_REG_T_SPEC_INT_ENABLE_2                (MTD_REG_T_ADV_PCS+0x0012)
#define MTD_REG_T_SPEC_INT_2                      (MTD_REG_T_ADV_PCS+0x0013)

#define MTD_REG_T_PKT_GENERATOR_CTRL_1                (MTD_REG_T_ADV_PCS+0x0030)
#define MTD_REG_T_XOR_MASK_01_PKT_GENERATOR_LSB        (MTD_REG_T_ADV_PCS+0x0031)
#define MTD_REG_T_XOR_MASK_23_PKT_GENERATOR_MSB        (MTD_REG_T_ADV_PCS+0x0032)
#define MTD_REG_T_PKT_GENERATOR_LENGTH                (MTD_REG_T_ADV_PCS+0x0033)
#define MTD_REG_T_PKT_GENERATOR_BURST_SEQUENCE        (MTD_REG_T_ADV_PCS+0x0034)
#define MTD_REG_T_PKT_GENERATOR_IPG                    (MTD_REG_T_ADV_PCS+0x0035)
#define MTD_REG_T_TRANSMIT_PKT_COUNTER_0_15            (MTD_REG_T_ADV_PCS+0x0036)
#define MTD_REG_T_TRANSMIT_PKT_COUNTER_16_31        (MTD_REG_T_ADV_PCS+0x0037)
#define MTD_REG_T_TRANSMIT_BYTE_COUNTER_0_15        (MTD_REG_T_ADV_PCS+0x0038)
#define MTD_REG_T_TRANSMIT_BYTE_COUNTER_16_31        (MTD_REG_T_ADV_PCS+0x0039)
#define MTD_REG_T_RECEIVE_PKT_COUNTER_0_15            (MTD_REG_T_ADV_PCS+0x003A)
#define MTD_REG_T_RECEIVE_PKT_COUNTER_16_31            (MTD_REG_T_ADV_PCS+0x003B)
#define MTD_REG_T_RECEIVE_BYTE_COUNTER_0_15            (MTD_REG_T_ADV_PCS+0x003C)
#define MTD_REG_T_RECEIVE_BYTE_COUNTER_16_31        (MTD_REG_T_ADV_PCS+0x003D)
#define MTD_REG_T_RECEIVE_PKT_ERR_COUNTER_0_15        (MTD_REG_T_ADV_PCS+0x003E)
#define MTD_REG_T_RECEIVE_PKT_ERR_COUNTER_16_31        (MTD_REG_T_ADV_PCS+0x003F)

#define MTD_REG_T_1G_PMD_TRANSMIT_DISABLE           (MTD_REG_T_ADV_PCS+0x0109)
#define MTD_REG_T_1G_PMD_RECEIVE_SIG_DETECT           (MTD_REG_T_ADV_PCS+0x010A)
#define MTD_REG_T_1G_PAIR_SWAP_POLARITRY            (MTD_REG_T_ADV_PCS+0x0182)

/* Marvell X-3xxx function Registers of X unit(line side, Optic)*/
#define MTD_REG_X_PCS_CONFIG_1                        (MTD_REG_X_COMMON_PCS+0x0000)
#define MTD_REG_X_SERDES_CONTROL_REG_1                (MTD_REG_X_COMMON_PCS+0x0003)
#define MTD_REG_X_SERDES_CONTROL_REG_2                (MTD_REG_X_COMMON_PCS+0x0004)
#define MTD_REG_X_FIFO_CRC_INTERRUPT_ENABLE            (MTD_REG_X_COMMON_PCS+0x000A)
#define MTD_REG_X_FIFO_CRC_INTERRUPT_STATUS            (MTD_REG_X_COMMON_PCS+0x000B)
#define MTD_REG_X_PPM_FIFO_CONTROL_1                (MTD_REG_X_COMMON_PCS+0x000C)
#define MTD_REG_X_PKT_GENERATOR_CTRL_1                (MTD_REG_X_COMMON_PCS+0x0010)
#define MTD_REG_X_PKT_GENERATOR_CTRL_2                (MTD_REG_X_COMMON_PCS+0x0011)
#define MTD_REG_X_XOR_MASK_01_PKT_GENERATOR_LSB        (MTD_REG_X_COMMON_PCS+0x0012)
#define MTD_REG_X_XOR_MASK_23_PKT_GENERATOR_MSB        (MTD_REG_X_COMMON_PCS+0x0013)
#define MTD_REG_X_PKT_GENERATOR_LENGTH                (MTD_REG_X_COMMON_PCS+0x0016)
#define MTD_REG_X_PKT_GENERATOR_BURST_SEQUENCE        (MTD_REG_X_COMMON_PCS+0x0017)
#define MTD_REG_X_PKT_GENERATOR_IPG                    (MTD_REG_X_COMMON_PCS+0x0018)
#define MTD_REG_X_TRANSMIT_PKT_COUNTER_0_15            (MTD_REG_X_COMMON_PCS+0x001B)
#define MTD_REG_X_TRANSMIT_PKT_COUNTER_16_31        (MTD_REG_X_COMMON_PCS+0x001C)
#define MTD_REG_X_TRANSMIT_PKT_COUNTER_32_47        (MTD_REG_X_COMMON_PCS+0x001D)
#define MTD_REG_X_TRANSMIT_BYTE_COUNTER_0_15        (MTD_REG_X_COMMON_PCS+0x001E)
#define MTD_REG_X_TRANSMIT_BYTE_COUNTER_16_31        (MTD_REG_X_COMMON_PCS+0x001F)
#define MTD_REG_X_TRANSMIT_BYTE_COUNTER_32_47        (MTD_REG_X_COMMON_PCS+0x0020)
#define MTD_REG_X_RECEIVE_PKT_COUNTER_0_15            (MTD_REG_X_COMMON_PCS+0x0021)
#define MTD_REG_X_RECEIVE_PKT_COUNTER_16_31            (MTD_REG_X_COMMON_PCS+0x0022)
#define MTD_REG_X_RECEIVE_PKT_COUNTER_32_47            (MTD_REG_X_COMMON_PCS+0x0023)
#define MTD_REG_X_RECEIVE_BYTE_COUNTER_0_15            (MTD_REG_X_COMMON_PCS+0x0024)
#define MTD_REG_X_RECEIVE_BYTE_COUNTER_16_31        (MTD_REG_X_COMMON_PCS+0x0025)
#define MTD_REG_X_RECEIVE_BYTE_COUNTER_32_47        (MTD_REG_X_COMMON_PCS+0x0026)
#define MTD_REG_X_RECEIVE_PKT_ERR_COUNTER_0_15        (MTD_REG_X_COMMON_PCS+0x0027)
#define MTD_REG_X_RECEIVE_PKT_ERR_COUNTER_16_31        (MTD_REG_X_COMMON_PCS+0x0028)
#define MTD_REG_X_RECEIVE_PKT_ERR_COUNTER_32_47        (MTD_REG_X_COMMON_PCS+0x0029)

/* Marvell X-3xxx function Registers of H unit(Mac side)*/
#define MTD_REG_H_PCS_CONFIG_1                        (MTD_REG_H_COMMON_PCS+0x0000)
#define MTD_REG_H_SERDES_CONTROL_REG_1                (MTD_REG_H_COMMON_PCS+0x0003)
#define MTD_REG_H_SERDES_CONTROL_REG_2                (MTD_REG_H_COMMON_PCS+0x0004)
#define MTD_REG_H_REF_CLK_CONTROL                   (MTD_REG_H_COMMON_PCS+0x0005)

#define MTD_REG_H_FIFO_CRC_INTERRUPT_ENABLE            (MTD_REG_H_COMMON_PCS+0x000A)
#define MTD_REG_H_FIFO_CRC_INTERRUPT_STATUS            (MTD_REG_H_COMMON_PCS+0x000B)
#define MTD_REG_H_PPM_FIFO_CONTROL_1                (MTD_REG_H_COMMON_PCS+0x000C)
#define MTD_REG_H_PKT_GENERATOR_CTRL_1                (MTD_REG_H_COMMON_PCS+0x0010)
#define MTD_REG_H_PKT_GENERATOR_CTRL_2                (MTD_REG_H_COMMON_PCS+0x0011)
#define MTD_REG_H_XOR_MASK_01_PKT_GENERATOR_LSB        (MTD_REG_H_COMMON_PCS+0x0012)
#define MTD_REG_H_XOR_MASK_23_PKT_GENERATOR_MSB        (MTD_REG_H_COMMON_PCS+0x0013)
#define MTD_REG_H_PKT_GENERATOR_LENGTH                (MTD_REG_H_COMMON_PCS+0x0016)
#define MTD_REG_H_PKT_GENERATOR_BURST_SEQUENCE        (MTD_REG_H_COMMON_PCS+0x0017)
#define MTD_REG_H_PKT_GENERATOR_IPG                    (MTD_REG_H_COMMON_PCS+0x0018)

#define MTD_REG_H_TRANSMIT_PKT_COUNTER_0_15            (MTD_REG_H_COMMON_PCS+0x001B)
#define MTD_REG_H_TRANSMIT_PKT_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x001C)
#define MTD_REG_H_TRANSMIT_PKT_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x001D)
#define MTD_REG_H_TRANSMIT_BYTE_COUNTER_0_15        (MTD_REG_H_COMMON_PCS+0x001E)
#define MTD_REG_H_TRANSMIT_BYTE_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x001F)
#define MTD_REG_H_TRANSMIT_BYTE_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x0020)
#define MTD_REG_H_RECEIVE_PKT_COUNTER_0_15            (MTD_REG_H_COMMON_PCS+0x0021)
#define MTD_REG_H_RECEIVE_PKT_COUNTER_16_31            (MTD_REG_H_COMMON_PCS+0x0022)
#define MTD_REG_H_RECEIVE_PKT_COUNTER_32_47            (MTD_REG_H_COMMON_PCS+0x0023)
#define MTD_REG_H_RECEIVE_BYTE_COUNTER_0_15            (MTD_REG_H_COMMON_PCS+0x0024)
#define MTD_REG_H_RECEIVE_BYTE_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x0025)
#define MTD_REG_H_RECEIVE_BYTE_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x0026)
#define MTD_REG_H_RECEIVE_PKT_ERR_COUNTER_0_15        (MTD_REG_H_COMMON_PCS+0x0027)
#define MTD_REG_H_RECEIVE_PKT_ERR_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x0028)
#define MTD_REG_H_RECEIVE_PKT_ERR_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x0029)

#define MTD_REG_H_PRBS0_CONTROL                        (MTD_REG_H_COMMON_PCS+0x0030)
#define MTD_REG_H_PRBS0_SYMBOL_TX_COUNTER_0_15        (MTD_REG_H_COMMON_PCS+0x0031)
#define MTD_REG_H_PRBS0_SYMBOL_TX_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x0032)
#define MTD_REG_H_PRBS0_SYMBOL_TX_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x0033)
#define MTD_REG_H_PRBS0_SYMBOL_RX_COUNTER_0_15        (MTD_REG_H_COMMON_PCS+0x0034)
#define MTD_REG_H_PRBS0_SYMBOL_RX_COUNTER_16_31        (MTD_REG_H_COMMON_PCS+0x0035)
#define MTD_REG_H_PRBS0_SYMBOL_RX_COUNTER_32_47        (MTD_REG_H_COMMON_PCS+0x0036)
#define MTD_REG_H_PRBS0_ERR_COUNTER_0_15            (MTD_REG_H_COMMON_PCS+0x0037)
#define MTD_REG_H_PRBS0_ERR_COUNTER_16_31            (MTD_REG_H_COMMON_PCS+0x0038)
#define MTD_REG_H_PRBS0_ERR_COUNTER_32_47            (MTD_REG_H_COMMON_PCS+0x0039)
#define MTD_REG_H_PRBS1_CONTROL                        (MTD_REG_H_COMMON_PCS+0x0040)
#define MTD_REG_H_PRBS1_TX_COUNTER_0_15                (MTD_REG_H_COMMON_PCS+0x0041)
#define MTD_REG_H_PRBS1_TX_COUNTER_16_31            (MTD_REG_H_COMMON_PCS+0x0042)
#define MTD_REG_H_PRBS1_TX_COUNTER_32_47            (MTD_REG_H_COMMON_PCS+0x0043)
#define MTD_REG_H_PRBS1_RX_COUNTER_0_15                (MTD_REG_H_COMMON_PCS+0x0044)
#define MTD_REG_H_PRBS1_RX_COUNTER_16_31            (MTD_REG_H_COMMON_PCS+0x0045)
#define MTD_REG_H_PRBS1_RX_COUNTER_32_47            (MTD_REG_H_COMMON_PCS+0x0046)
#define MTD_REG_H_PRBS1_ERR_COUNTER_0_15            (MTD_REG_H_COMMON_PCS+0x0047)
#define MTD_REG_H_PRBS1_ERR_COUNTER_16_31            (MTD_REG_H_COMMON_PCS+0x0048)
#define MTD_REG_H_PRBS1_ERR_COUNTER_32_47            (MTD_REG_H_COMMON_PCS+0x0049)

/* Marvell X-3xxx function WOL control and status  */
#define MTD_REG_WOL_CONTROL                  (MTD_REG_WOL_DEVICE+0xf200+0x10)
#define MTD_REG_WOL_STATUS                   (MTD_REG_WOL_DEVICE+0xf200+0x11)
#define MTD_REG_WOL_SRAM_PKT_76_LEN          (MTD_REG_WOL_DEVICE+0xf200+0x12)
#define MTD_REG_WOL_SRAM_PKT_54_LEN          (MTD_REG_WOL_DEVICE+0xf200+0x13)
#define MTD_REG_WOL_SRAM_PKT_32_LEN          (MTD_REG_WOL_DEVICE+0xf200+0x14)
#define MTD_REG_WOL_SRAM_PKT_10_LEN          (MTD_REG_WOL_DEVICE+0xf200+0x15)
#define MTD_REG_WOL_MAGIC_PKT_DST_ADDR_2     (MTD_REG_WOL_DEVICE+0xf200+0x17)
#define MTD_REG_WOL_MAGIC_PKT_DST_ADDR_1     (MTD_REG_WOL_DEVICE+0xf200+0x18)
#define MTD_REG_WOL_MAGIC_PKT_DST_ADDR_0     (MTD_REG_WOL_DEVICE+0xf200+0x19)
#define MTD_REG_WOL_SRAM_BYTE_ADDR_CTRL      (MTD_REG_WOL_DEVICE+0xf200+0x1A)
#define MTD_REG_WOL_SRAM_BYTE_DATA_CTRL      (MTD_REG_WOL_DEVICE+0xf200+0x1B)
#define MTD_REG_WOL_SRAM_READ_BYTE_DATA      (MTD_REG_WOL_DEVICE+0xf200+0x1C)
#define MTD_REG_WOL_SRAM_BIST                (MTD_REG_WOL_DEVICE+0xf200+0x1D)

#define MTD_PAGE2        2
#define MTD_PAGE17        17

#define MTD_REG_MAC_CONTROL_1                (MTD_REG_WOL_DEVICE+0xf200+0x1D)

#endif  /* MTD_LINKCRYPT_MSECPTP */

#if C_LINKAGE
#ifdef __cplusplus
}
#endif
#endif


#endif /* __mtdMsecPtpDrvRegsh */
