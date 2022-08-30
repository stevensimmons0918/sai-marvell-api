/* eip163_hw_interface.h
 *
 * EIP-163 Classification Engine Register Internal interface
 *
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.5                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2020-Feb-27                                              */
/*                                                                            */
/* Copyright (c) 2008-2020 INSIDE Secure B.V. All Rights Reserved             */
/*                                                                            */
/* This confidential and proprietary software may be used only as authorized  */
/* by a licensing agreement from INSIDE Secure.                               */
/*                                                                            */
/* The entire notice above must be reproduced on all authorized copies that   */
/* may only be made to the extent permitted by a licensing agreement from     */
/* INSIDE Secure.                                                             */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

#ifndef EIP163_HW_INTERFACE_H_
#define EIP163_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP163/incl/c_eip163.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>         /* BIT definitions, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */
#include <Kit/DriverFramework/incl/device_rw.h>          /* Read32, Write32 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Read/Write register constants */

/*****************************************************************************
 * Byte offsets of the EIP-163 Packet Engine registers
 *****************************************************************************/
/* EIP-163 EIP number (0xA3) and complement (0x5C) */
#define EIP163_SIGNATURE                    ((uint16_t)0x5CA3)

#define EIP163_REG_OFFS                     4

/* Input TCAM entries (n), */
/* each entry has (m) 32-bit words, for control word m=0 */
#define EIP163_REG_TCAM_ENTRY_OFFS          0x40 /* TCAM entry size in bytes */
#define EIP163_REG_TCAM_KEY_WORD_COUNT      8
#define EIP163_REG_TCAM_MASK_WORD_COUNT     EIP163_REG_TCAM_KEY_WORD_COUNT
#define EIP163_REG_TCAM_KEY(n,m)            (EIP163_CONF_BASE + \
                                             (0x10000 * (n>>9)) + \
                                             EIP163_REG_TCAM_ENTRY_OFFS * ((n)& 0x1ff) + \
                                             EIP163_REG_OFFS * m)
#define EIP163_REG_TCAM_CTRL_KEY(n)         EIP163_REG_TCAM_KEY((n), 0)
#define EIP163_REG_TCAM_MASK(n,m)           (EIP163_CONF_BASE + 0x0020 + \
                                             (0x10000 * ((n) >> 9)) + \
                                             EIP163_REG_TCAM_ENTRY_OFFS * ((n) & 0x1ff)+ \
                                             EIP163_REG_OFFS * (m))
#define EIP163_REG_TCAM_CTRL_MASK(n)        EIP163_REG_TCAM_MASK((n), 0)

/* TCAM match enable control */
#define EIP163_REG_TCAM_ENTRY_ENABLE(n)     (EIP163_CONF_BASE + 0x8000 + \
                                             (0x10000 * ((n) >> 4)) + \
                                             EIP163_REG_OFFS * ((n) & 0xf))
#define EIP163_REG_TCAM_ENTRY_SET(n)        (EIP163_CONF_BASE + 0x8080 + \
                                             (0x10000 * ((n) >> 4)) + \
                                             EIP163_REG_OFFS * ((n) & 0xf))
#define EIP163_REG_TCAM_ENTRY_CLEAR(n)      (EIP163_CONF_BASE + 0x80C0 + \
                                             (0x10000 * ((n) >> 4)) + \
                                             EIP163_REG_OFFS * ((n) & 0xf))
#define EIP163_REG_TCAM_ENTRY_ENABLE_CTRL   (EIP163_CONF_BASE + 0x8100)
#define EIP163_REG_TCAM_STATUS              (EIP163_CONF_BASE + 0x8120)
#define EIP163_REG_SAM_IN_FLIGHT            (EIP163_CONF_BASE + 0x8104)

#define EIP163_REG_TCAM_COUNT_SUMMARY_CHAN1_CNT_MASK    MASK_4_BITS


/* TCAM policy */
#define EIP163_REG_TCAM_POLICY_OFFS         (EIP163_CONF_BASE + 0x8800)
#define EIP163_REG_TCAM_POLICY(n)           (EIP163_REG_TCAM_POLICY_OFFS + \
                                             (0x10000 * ((n) >> 9)) + \
                                             EIP163_REG_OFFS * ((n) & 0x1ff))

/* vPort policy */
#define EIP163_REG_VPORT_POLICY_OFFS        (EIP163_CONF_BASE + 0x9000)
#define EIP163_REG_VPORT_POLICY(n)          (EIP163_REG_VPORT_POLICY_OFFS + \
                                             (0x10000 * ((n) >> 10)) + \
                                             EIP163_REG_OFFS * ((n) & 0x3ff))

/* Per-channel classifier settings, n - channel identifier */
#define EIP163_REG_CH_CFY_CTRL_OFFS         (EIP163_CONF_BASE + 0xA000)
#define EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS   0x40
#define EIP163_REG_CP_MATCH_MODE_MASK       (MASK_10_BITS << 8)
#define EIP163_REG_CP_MATCH_ENABLE_MASK     (BIT_31 | MASK_21_BITS)
#define EIP163_REG_CP_MAC_DA_MATCH_MASK     MASK_8_BITS
#define EIP163_REG_CP_MAC_DA_ET_MATCH_MASK  MASK_16_BITS
#define EIP163_REG_CP_MAC_DA_ET_RANGE_MASK  (BIT_17 | BIT_16)
#define EIP163_REG_CP_MAC_DA_RANGE_MASK     BIT_18
#define EIP163_REG_CP_MAC_DA_44BIT_MASK     BIT_19
#define EIP163_REG_CP_MAC_DA_48BIT_MASK     BIT_20

#define EIP163_REG_CP_MATCH_MODE(n)         (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x00 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_CP_MATCH_ENABLE(n)       (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x04 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SAM_NM_PARAMS(n)         (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x08 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SAM_POLICY(n)            (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x0C + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_EGRESS_HDR_ETYPE(n)      (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x18 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SAM_CP_TAG_ENB(n)        (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x1C + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SCP_MATCH_MODE(n)        (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x20 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SCP_MATCH_ENABLE(n)      (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x24 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))
#define EIP163_REG_SAM_SCP_TAG_ENB(n)       (EIP163_REG_CH_CFY_CTRL_OFFS + \
                                             0x3C + \
                                             (0x10000 * ((n)>> 5)) + \
                                             EIP163_REG_CH_CFY_CTRL_ENTRY_OFFS * ((n)& 0x1F))


/* Global Control Packet Detector (CPD) control registers */
#define EIP163_REG_CPD_OFFS                     (EIP163_CONF_BASE  + 0xA800)
#define EIP163_REG_CP_MAC_DA_MATCH_0            (EIP163_REG_CPD_OFFS + 0x00)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_0         (EIP163_REG_CPD_OFFS + 0x04)
#define EIP163_REG_CP_MAC_DA_MATCH_1            (EIP163_REG_CPD_OFFS + 0x08)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_1         (EIP163_REG_CPD_OFFS + 0x0C)
#define EIP163_REG_CP_MAC_DA_MATCH_2            (EIP163_REG_CPD_OFFS + 0x10)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_2         (EIP163_REG_CPD_OFFS + 0x14)
#define EIP163_REG_CP_MAC_DA_MATCH_3            (EIP163_REG_CPD_OFFS + 0x18)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_3         (EIP163_REG_CPD_OFFS + 0x1C)
#define EIP163_REG_CP_MAC_DA_MATCH_4            (EIP163_REG_CPD_OFFS + 0x20)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_4         (EIP163_REG_CPD_OFFS + 0x24)
#define EIP163_REG_CP_MAC_DA_MATCH_5            (EIP163_REG_CPD_OFFS + 0x28)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_5         (EIP163_REG_CPD_OFFS + 0x2C)
#define EIP163_REG_CP_MAC_DA_MATCH_6            (EIP163_REG_CPD_OFFS + 0x30)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_6         (EIP163_REG_CPD_OFFS + 0x34)
#define EIP163_REG_CP_MAC_DA_MATCH_7            (EIP163_REG_CPD_OFFS + 0x38)
#define EIP163_REG_CP_MAC_DA_ET_MATCH_7         (EIP163_REG_CPD_OFFS + 0x3C)
#define EIP163_REG_CP_MAC_DA_MATCH(n)           (EIP163_REG_CP_MAC_DA_MATCH_0 + \
                                                 (n) * 0x08)
#define EIP163_REG_CP_MAC_DA_ET_MATCH(n)        (EIP163_REG_CP_MAC_DA_ET_MATCH_0 + \
                                                 (n)* 0x08)

#define EIP163_REG_CP_MAC_DA_START0_LO          (EIP163_REG_CPD_OFFS + 0x60)
#define EIP163_REG_CP_MAC_DA_START0_HI          (EIP163_REG_CPD_OFFS + 0x64)
#define EIP163_REG_CP_MAC_DA_END0_LO            (EIP163_REG_CPD_OFFS + 0x68)
#define EIP163_REG_CP_MAC_DA_END0_HI            (EIP163_REG_CPD_OFFS + 0x6C)
#define EIP163_REG_CP_MAC_DA_START1_LO          (EIP163_REG_CPD_OFFS + 0x70)
#define EIP163_REG_CP_MAC_DA_START1_HI          (EIP163_REG_CPD_OFFS + 0x74)
#define EIP163_REG_CP_MAC_DA_END1_LO            (EIP163_REG_CPD_OFFS + 0x78)
#define EIP163_REG_CP_MAC_DA_END1_HI            (EIP163_REG_CPD_OFFS + 0x7C)
#define EIP163_REG_CP_MAC_DA_START_LO(n)        (EIP163_REG_CP_MAC_DA_START0_LO + \
                                                 (n)* 0x08)
#define EIP163_REG_CP_MAC_DA_START_HI(n)        (EIP163_REG_CP_MAC_DA_START0_HI + \
                                                 (n)* 0x08)
#define EIP163_REG_CP_MAC_DA_END_LO(n)          (EIP163_REG_CP_MAC_DA_END0_LO + \
                                                 (n)* 0x08)
#define EIP163_REG_CP_MAC_DA_END_HI(n)          (EIP163_REG_CP_MAC_DA_END0_HI + \
                                                 (n)* 0x08)

/* Range only (RO) */
#define EIP163_REG_CP_MAC_DA_START_RO_LO        (EIP163_REG_CPD_OFFS + 0x80)
#define EIP163_REG_CP_MAC_DA_START_RO_HI        (EIP163_REG_CPD_OFFS + 0x84)
#define EIP163_REG_CP_MAC_DA_END_RO_LO          (EIP163_REG_CPD_OFFS + 0x88)
#define EIP163_REG_CP_MAC_DA_END_RO_HI          (EIP163_REG_CPD_OFFS + 0x8C)

#define EIP163_REG_CP_MAC_DA_44_BITS_LO         (EIP163_REG_CPD_OFFS + 0x90)
#define EIP163_REG_CP_MAC_DA_44_BITS_HI         (EIP163_REG_CPD_OFFS + 0x94)
#define EIP163_REG_CP_MAC_DA_48_BITS_LO         (EIP163_REG_CPD_OFFS + 0x98)
#define EIP163_REG_CP_MAC_DA_48_BITS_HI         (EIP163_REG_CPD_OFFS + 0x9C)

/* Global Header Parser (GHP) control registers */
#define EIP163_REG_GHP_OFFS                     (EIP163_CONF_BASE  + 0xA900)
#define EIP163_REG_SAM_CP_TAG                   (EIP163_REG_GHP_OFFS + 0x00)
#define EIP163_REG_SAM_PP_TAGS                  (EIP163_REG_GHP_OFFS + 0x04)
#define EIP163_REG_SAM_PP_TAGS2                 (EIP163_REG_GHP_OFFS + 0x08)
#define EIP163_REG_SAM_CP_TAG2                  (EIP163_REG_GHP_OFFS + 0x0C)

/* Packet processing debug registers */
#define EIP163_REG_CP_MATCH_DEBUG               (EIP163_CONF_BASE + 0xAC00)
#define EIP163_REG_TCAM_DEBUG                   (EIP163_CONF_BASE + 0xAC24)
#define EIP163_REG_SECTAG_DEBUG                 (EIP163_CONF_BASE + 0xAC28)
#define EIP163_REG_SAMPP_DEBUG1                 (EIP163_CONF_BASE + 0xAC80)
#define EIP163_REG_SAMPP_DEBUG2                 (EIP163_CONF_BASE + 0xAC84)
#define EIP163_REG_SAMPP_DEBUG3                 (EIP163_CONF_BASE + 0xAC88)

#define EIP163_REG_DBG_PARSED_DA_LO             (EIP163_CONF_BASE + 0xAD00)
#define EIP163_REG_DBG_PARSED_DA_HI             (EIP163_CONF_BASE + 0xAD04)
#define EIP163_REG_DBG_PARSED_SA_LO             (EIP163_CONF_BASE + 0xAD08)
#define EIP163_REG_DBG_PARSED_SA_HI             (EIP163_CONF_BASE + 0xAD0C)
#define EIP163_REG_DBG_PARSED_SECTAG_LO         (EIP163_CONF_BASE + 0xAD10)
#define EIP163_REG_DBG_PARSED_SECTAG_HI         (EIP163_CONF_BASE + 0xAD14)
#define EIP163_REG_DEBUG_FLOW_LOOKUP            (EIP163_CONF_BASE + 0xAD20)

/* TCAM registers bank offsets */
/* EIP-217 register bank: statistics counters */
#define EIP163_REG_TCAM_STAT_COUNT_OFFS(n)      (EIP163_CONF_BASE + 0xC000 + \
                                                 (0x10000 *((n) >> 9)) + 8 * ((n) & 0x1ff))

#define EIP163_REG_TCAM_STAT_POSTED_RES         (EIP163_CONF_BASE + 0xDC00)
/* EIP-217 register bank: statistics controls */
#define EIP163_REG_TCAM_STAT_CTRL_OFFS          (EIP163_CONF_BASE + 0xD800)
#define EIP163_REG_TCAM_SUMMARY_P_OFFS          (EIP163_CONF_BASE + 0xD000)

/* CHAN registers bank offsets */
/* EIP-217 register bank: statistics counters */
#define EIP163_CHAN_STAT_TCAM_HIT_MULTIPLE          0
#define EIP163_CHAN_STAT_HDRPARSER_DROPPED_PKTS     1
#define EIP163_CHAN_STAT_TCAM_MISS                  2
#define EIP163_CHAN_STAT_PKTS_CTRL                  3
#define EIP163_CHAN_STAT_PKTS_DATA                  4
#define EIP163_CHAN_STAT_PKTS_DROP                  5
#define EIP163_CHAN_STAT_PKTS_ERR_IN                6

#define EIP164_REG_CHn_STAT_BASE_OFFS               0x40
#define EIP163_REG_CHAN_STAT_COUNT_OFFS(n)          (EIP163_CONF_BASE + 0xE000 + \
                                                     (0x10000 * ((n)>> 5)) + \
                                                     (EIP164_REG_CHn_STAT_BASE_OFFS * ((n)& 0x1F)))
#define EIP163_REG_CHAN_STAT_POSTED_RES             (EIP163_CONF_BASE + 0xEE00)

/* EIP-217 register bank: statistics controls */
#define EIP163_REG_CHAN_STAT_CTRL_OFFS              (EIP163_CONF_BASE + 0xEC00)
#define EIP163_REG_CHAN_SUMMARY_P_OFFS              (EIP163_CONF_BASE + 0xE800)

/* Input adapter */
#define EIP163_REG_RX_INSERT_SOP(n)         (EIP163_CONF_BASE + 0xFC80 + \
                                             (0x10000 * (n)))
#define EIP163_REG_RX_INSERT_EOP(n)         (EIP163_CONF_BASE + 0xFC84 + \
                                             (0x10000 * (n)))
#define EIP163_REG_RX_TIMEOUT_CTRL(n)       (EIP163_CONF_BASE + 0xFC88 + \
                                             (0x10000 * (n)))
#define EIP163_REG_RX_TIMEOUT_VAL           (EIP163_CONF_BASE + 0xFC8C)
#define EIP163_REG_RX_TIMEOUT_STATE(n)      (EIP163_CONF_BASE + 0xFC90 + \
                                             (0x10000 * (n)))
#define EIP163_REG_RX_FILTER_STATE(n)       (EIP163_CONF_BASE + 0xFC94 + \
                                             (0x10000 * (n)))

/* Channel controls */
#define EIP163_REG_CHANNEL_CTRL(n)          (EIP163_CONF_BASE + 0xFE00 + \
                                             (0x10000 * ((n)>> 5)) + \
                                             (EIP163_REG_OFFS * ((n)& 0x1f)))

#define EIP163_REG_LOW_LAT_CONFIG           (EIP163_CONF_BASE + 0xFE80)

/* ECC status */
#define EIP163_REG_ECC_COUNTER_RAM(n)       (EIP163_CONF_BASE + 0xFF00 + \
                                             (EIP163_REG_OFFS * (n)))
#define EIP163_REG_ECC_CORR_ENABLE          (EIP163_CONF_BASE + 0xFF60)
#define EIP163_REG_ECC_DERR_ENABLE          (EIP163_CONF_BASE + 0xFF64)
#define EIP163_REG_ECC_THR_CORR_STAT        (EIP163_CONF_BASE + 0xFF68)
#define EIP163_REG_ECC_THR_DERR_STAT        (EIP163_CONF_BASE + 0xFF6C)
#define EIP163_REG_ECC_THRESHOLDS           (EIP163_CONF_BASE + 0xFF78)


/* System control */
/* Debug clocks related */
#define EIP163_REG_CLOCK_STATE              (EIP163_CONF_BASE + 0xFFE8)
#define EIP163_REG_FORCE_CLOCK_ON           (EIP163_CONF_BASE + 0xFFEC)
#define EIP163_REG_FORCE_CLOCK_OFF          (EIP163_CONF_BASE + 0xFFF0)

/* EIP-163 configuration and version information registers */
#define EIP163_REG_CONFIG2                  (EIP163_CONF_BASE + 0xFFF4)
#define EIP163_REG_CONFIG                   (EIP163_CONF_BASE + 0xFFF8)
#define EIP163_REG_VERSION                  (EIP163_CONF_BASE + 0xFFFC)

/* Default (reset) register values */
/* Note: when updating the default register values from 0 to something else */
/*       check how this value is used in the corresponding level0 macro, */
/*       it may need to be updated too for the boolean operations! */
#define EIP163_REG_VPORT_POLICY_DEFAULT             0x00000000
#define EIP163_REG_CHANNEL_CTRL_DEFAULT             0x00000000
#define EIP163_REG_TCAM_CTRL_KEY_DEFAULT            0x00000000
#define EIP163_REG_TCAM_CTRL_MASK_DEFAULT           0x00000000
#define EIP163_REG_TCAM_ENTRY_ENABLE_CTRL_DEFAULT   0x00000000
#define EIP163_REG_EGRESS_HDR_ETYPE_DEFAULT         0x00000000
#define EIP163_REG_SAM_IN_FLIGHT_DEFAULT            0x00000000
#define EIP163_REG_TCAM_POLICY_DEFAULT              0x00000000
#define EIP163_REG_CP_MATCH_MODE_DEFAULT            0x00000000
#define EIP163_REG_CP_MATCH_ENABLE_DEFAULT          0x00000000
#define EIP163_REG_SAM_NM_PARAMS_DEFAULT            0xE5880007
#define EIP163_REG_SAM_POLICY_DEFAULT               0x00000000
#define EIP163_REG_SAM_CP_TAG_ENB_DEFAULT           0x00000000
#define EIP163_REG_SCP_MATCH_MODE_DEFAULT           0x00000000
#define EIP163_REG_SCP_MATCH_ENABLE_DEFAULT         0x00000000
#define EIP163_REG_SAM_SCP_TAG_ENB_DEFAULT          0x00000000
#define EIP163_REG_CP_MAC_DA_MATCH_DEFAULT          0x00000000
#define EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT       0x00000000
#define EIP163_REG_CP_MAC_DA_44_BITS_LO_DEFAULT     0x00C28001
#define EIP163_REG_CP_MAC_DA_44_BITS_HI_DEFAULT     0x00000000
#define EIP163_REG_CP_MAC_DA_48_BITS_LO_DEFAULT     0xCC0C0001
#define EIP163_REG_CP_MAC_DA_48_BITS_HI_DEFAULT     0x0000CCCC
#define EIP163_REG_SAM_CP_TAG_DEFAULT               0x00000000
#define EIP163_REG_SAM_CP_TAG2_DEFAULT              0x00000000
#define EIP163_REG_SAM_PP_TAGS_DEFAULT              0xA8880081
#define EIP163_REG_SAM_PP_TAGS2_DEFAULT             0x00920091

#endif /* EIP163_HW_INTERFACE_H_ */


/* end of file eip163_hw_interface.h */
