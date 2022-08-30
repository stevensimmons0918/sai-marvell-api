/* eip164_hw_interface.h
 *
 * EIP-164 Engine Hardware Register Internal interface
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

#ifndef EIP164_HW_INTERFACE_H_
#define EIP164_HW_INTERFACE_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>  /* BIT definitions, bool, uint32_t */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Read/Write register constants */

/*****************************************************************************
 * Byte offsets of the EIP-164 Packet Engine registers
 *****************************************************************************/
/* EIP-164 EIP number (0xA4) and complement (0x5B) */
#define EIP164_SIGNATURE                  ((uint16_t)0x5BA4)

#define EIP164_REG_OFFS                     4

#define EIP164_SAM_FLOW_CTRL_BP_TYPE        0 /* Bypass flow type */
#define EIP164_SAM_FLOW_CTRL_DROP_TYPE      1 /* Drop flow type */

/* ------------ */
/* Transform records (SA = Security Association) area */
#define EIP164_XFORM_REC_WORD_COUNT         32
#define EIP164_REG_XFORM_REC(n)             (EIP164_CONF_BASE + \
                                             (0x10000 * ((n)>> 7)) + \
                                             (EIP164_XFORM_REC_WORD_COUNT * \
                                             EIP164_REG_OFFS * ((n)& 0x7f)))

/*------------- */
/* Packet parser and lookup debug registers */
#define EIP164_REG_DBG_PARSED_DA_LO         (EIP164_CONF_BASE + 0x5400)
#define EIP164_REG_DBG_PARSED_DA_HI         (EIP164_CONF_BASE + 0x5404)
#define EIP164_REG_DBG_PARSED_SA_LO         (EIP164_CONF_BASE + 0x5408)
#define EIP164_REG_DBG_PARSED_SA_HI         (EIP164_CONF_BASE + 0x540c)
#define EIP164_REG_DBG_PARSED_SECTAG_LO     (EIP164_CONF_BASE + 0x5410)
#define EIP164_REG_DBG_PARSED_SECTAG_HI     (EIP164_CONF_BASE + 0x5414)
#define EIP164_REG_DBG_PARSED_SCI_LO        (EIP164_CONF_BASE + 0x5418)
#define EIP164_REG_DBG_PARSED_SCI_HI        (EIP164_CONF_BASE + 0x541c)
#define EIP164_REG_SECTAG_DEBUG             (EIP164_CONF_BASE + 0x5420)
#define EIP164_REG_PARSER_IN_DEBUG          (EIP164_CONF_BASE + 0x5454)
#define EIP164_REG_DBG_RXCAM_SCI_LO         (EIP164_CONF_BASE + 0x5468)
#define EIP164_REG_DBG_RXCAM_SCI_HI         (EIP164_CONF_BASE + 0x546c)

/* ------------ */
/* Receiver Secure Channel (RxSC) lookup table (CAM) (ingress only) */
#define EIP164_REG_RXSC_CAM_OFFS            16
#define EIP164_REG_RXSC_CAM_SCI_LO(n)       (EIP164_CONF_BASE + 0x4000 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_RXSC_CAM_OFFS * \
                                              ((n)& 0x3f)))

#define EIP164_REG_RXSC_CAM_SCI_HI(n)       (EIP164_CONF_BASE + 0x4004 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_RXSC_CAM_OFFS * \
                                              ((n)& 0x3f)))

#define EIP164_REG_RXSC_CAM_CTRL(n)         (EIP164_CONF_BASE + 0x4008 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_RXSC_CAM_OFFS * \
                                              ((n)& 0x3f)))

#define  EIP164_REG_RXSC_ENTRY_ENABLE(n)    (EIP164_CONF_BASE + 0x4400 + \
                                             (EIP164_REG_OFFS * ((n)- 1)))

#define EIP164_REG_RXSC_ENTRY_ENABLE_CTRL_DEFAULT   0x00000000
#define EIP164_REG_RXSC_ENTRY_ENABLE_CTRL   (EIP164_CONF_BASE + 0x47fc)
#define EIP164_REG_RXSC_STATUS              (EIP164_CONF_BASE + 0x47f8)
/* ------------ */
/* Secure Channel (SC) to Security Association (SA) mapping, AN decoding */
#define EIP164_REG_SC_SA_MAP_OFFS           8
#define EIP164_REG_SC_SA_MAP1_DEFAULT       0x00000000
#define EIP164_REG_SC_SA_MAP1(n)            (EIP164_CONF_BASE + 0x4800 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_SC_SA_MAP_OFFS * \
                                              ((n)& 0x3f)))

#define EIP164_REG_SC_SA_MAP2_DEFAULT       0x00000000
#define EIP164_REG_SC_SA_MAP2(n)            (EIP164_CONF_BASE + 0x4804 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_SC_SA_MAP_OFFS * \
                                              ((n)& 0x3f)))

/* ------------ */
/* Flow control: vPort, SecY, SC policy */
#define EIP164_REG_SAM_FLOW_CTRL_OFFS       8
#define EIP164_REG_SAM_FLOW_CTRL1(n)        (EIP164_CONF_BASE + 0x4A00 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_SAM_FLOW_CTRL_OFFS * \
                                              ((n)& 0x3f)))

#define EIP164_REG_SAM_FLOW_CTRL2(n)        (EIP164_CONF_BASE + 0x4A04 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_SAM_FLOW_CTRL_OFFS * \
                                              ((n)& 0x3f)))

/* ------------ */
/* Classifier control/status */
/* SecTAG parser controls (ch n) (ingress only) */
#define EIP164_REG_SAM_NM_PARAMS_DEFAULT    0xE588007F
#define EIP164_REG_SAM_NM_PARAMS(n)         (EIP164_CONF_BASE + 0x5000 + \
                                             (0x10000 * ((n) >> 5)) + \
                                             (EIP164_REG_OFFS * ((n) & 0x1f)))

/* In-flight counter */
#define EIP164_REG_SAM_IN_FLIGHT_DEFAULT    0x00000000
#define EIP164_REG_SAM_IN_FLIGHT            (EIP164_CONF_BASE + 0x5104)

/* Crypt-authenticate mode register */
#define EIP164_REG_CRYPT_AUTH_CTRL_DEFAULT  0x00000000
#define EIP164_REG_CRYPT_AUTH_CTRL          (EIP164_CONF_BASE + 0x5108)

/* Expired SA summary (egress only) */
#define EIP164_REG_SA_EXP_SUMMARY(n)        (EIP164_CONF_BASE + 0x5200 + \
                                             0x10000 * ((n-1)>>6) +  \
                                             (EIP164_REG_OFFS * (((n)- 1)&0x3f)))

/* ------------ */
/* EIP-66 MACsec crypto-core - see eip66_level0.h file for layout and use */

/* -------------------------------------------------------- */
/* Statistics Counters */
/* SecY statistics counters: n - counter ID */
#define EIP164_REG_SECY_STATISTICS_SET_OFFS         0x80
#define EIP164_REG_SECY_STATISTICS_BANK_OFFS        0x10000
#define EIP164_REG_SECY_STATISTICS_OFFS             0x6000

#define EIP164_REG_SECY_STATISTICS(n)  (EIP164_CONF_BASE + \
                                        EIP164_REG_SECY_STATISTICS_OFFS + \
                                        (EIP164_REG_SECY_STATISTICS_BANK_OFFS * \
                                         ((n)>> 6)) + \
                                        (EIP164_REG_SECY_STATISTICS_SET_OFFS * \
                                         ((n)& 0x3f)))

#define EIP164_SECY_STAT_TRANSFORM_ERROR_PKTS       0

#define EIP164_SECY_STAT_E_OUT_PKTS_CTRL            1
#define EIP164_SECY_STAT_E_OUT_PKTS_UNTAGGED        2

#define EIP164_SECY_STAT_I_IN_PKTS_CTRL             1
#define EIP164_SECY_STAT_I_IN_PKTS_UNTAGGED         2
#define EIP164_SECY_STAT_I_IN_PKTS_NOTAG            3
#define EIP164_SECY_STAT_I_IN_PKTS_BADTAG           4
#define EIP164_SECY_STAT_I_IN_PKTS_NOSCI            5
#define EIP164_SECY_STAT_I_IN_PKTS_UNKNOWNSCI       6
#define EIP164_SECY_STAT_I_IN_PKTS_TAGGEDCTRL       7


/* SA statistics counters: n - counter ID */
#define EIP164_REG_SA_STATISTICS_ID_OFFS            0x80
#define EIP164_REG_SA_STATISTICS_BANK_OFFS          0x10000
#define EIP164_REG_SA_STATISTICS_OFFS               0x8000
#define EIP164_REG_SA_STATISTICS(n)                 (EIP164_CONF_BASE + \
                                                     EIP164_REG_SA_STATISTICS_OFFS + \
                                                     (EIP164_REG_SA_STATISTICS_BANK_OFFS * \
                                                      ((n)>> 7)) + \
                                                     (EIP164_REG_SA_STATISTICS_ID_OFFS * \
                                                      ((n)& (EIP164_REG_SA_STATISTICS_ID_OFFS-1))))

#define EIP164_SA_STAT_E_OUT_OCTETS_ENC_PROT        0
#define EIP164_SA_STAT_E_OUT_PKTS_ENC_PROT          1
#define EIP164_SA_STAT_E_OUT_PKTS_TOO_LONG          2
#define EIP164_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE     3

#define EIP164_SA_STAT_I_IN_OCTETS_DEC              0
#define EIP164_SA_STAT_I_IN_OCTETS_VALIDATED        1
#define EIP164_SA_STAT_I_IN_PKTS_UNCHECKED          2
#define EIP164_SA_STAT_I_IN_PKTS_DELAYED            3
#define EIP164_SA_STAT_I_IN_PKTS_LATE               4
#define EIP164_SA_STAT_I_IN_PKTS_OK                 5
#define EIP164_SA_STAT_I_IN_PKTS_INVALID            6
#define EIP164_SA_STAT_I_IN_PKTS_NOT_VALID          7
#define EIP164_SA_STAT_I_IN_PKTS_NOT_USING_SA       8
#define EIP164_SA_STAT_I_IN_PKTS_UNUSED_SA          9

/* Interface statistics counters: n - counter ID */
#define EIP164_REG_IFC_STATISTICS_ID_OFFS           0x40
#define EIP164_REG_IFC_STATISTICS_BANK_OFFS         0x10000

#define EIP164_REG_IFC_UNCTL_STATISTICS_OFFS        0xC000
#define EIP164_REG_IFC_UNCTL_STATISTICS(n)  (EIP164_CONF_BASE + \
                                             EIP164_REG_IFC_UNCTL_STATISTICS_OFFS + \
                                             (EIP164_REG_IFC_STATISTICS_BANK_OFFS * \
                                              ((n)>> 6)) + \
                                             (EIP164_REG_IFC_STATISTICS_ID_OFFS * \
                                              ((n)& (EIP164_REG_IFC_STATISTICS_ID_OFFS-1))))

#define EIP164_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED               0
#define EIP164_IFC_STAT_E_OUT_OCTETS_CONTROLLED                 1
#define EIP164_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED           2
#define EIP164_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED           3
#define EIP164_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED           4

#define EIP164_IFC_STAT_I_IN_OCTETS_UNCONTROLLED                0
#define EIP164_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED            1
#define EIP164_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED            2
#define EIP164_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED            3

#define EIP164_REG_IFC_CTL_STATISTICS_OFFS          0xD000
#define EIP164_REG_IFC_CTL_STATISTICS(n)    (EIP164_CONF_BASE + \
                                             EIP164_REG_IFC_CTL_STATISTICS_OFFS + \
                                             (EIP164_REG_IFC_STATISTICS_BANK_OFFS * \
                                              ((n)>> 6)) + \
                                             (EIP164_REG_IFC_STATISTICS_ID_OFFS * \
                                              ((n)& (EIP164_REG_IFC_STATISTICS_ID_OFFS-1))))

#define EIP164_IFC_STAT_E_OUT_OCTETS_COMMON                     0
#define EIP164_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED             1
#define EIP164_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED             2
#define EIP164_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED             3

#define EIP164_IFC_STAT_I_IN_OCTETS_CONTROLLED                  0
#define EIP164_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED              1
#define EIP164_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED              2
#define EIP164_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED              3

/* ------------ */
/* Statistics control and RxCam/Channel statistics */
#define EIP164_REG_COUNT_CONTROL_DEFAULT                    0x00000001

/* SA Statistics */
#define EIP164_REG_SA_STATISTICS_CONTROLS   (EIP164_CONF_BASE + 0xE000)
#define EIP164_REG_SA_COUNT_SUMMARY_(n)     (EIP164_CONF_BASE + 0xE200 +\
                                             (0x10000 * ((n)>> 7)) + \
                                             (EIP164_REG_OFFS * ((n)& 0x7f)))
#define EIP164_REG_SA_SUMMARY_P             (EIP164_REG_SA_STATISTICS_CONTROLS + 0x100)
#define EIP164_REG_SA_STAT_POSTED_RES        (EIP164_CONF_BASE + 0xF200)

/* SecY Statistics */
#define EIP164_REG_SECY_STATISTICS_CONTROLS (EIP164_CONF_BASE + 0xE400)
#define EIP164_REG_SECY_COUNT_SUMMARY_(n)   (EIP164_CONF_BASE + 0xE500 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_OFFS * ((n)& 0x3F)))
#define EIP164_REG_SECY_SUMMARY_P           (EIP164_REG_SECY_STATISTICS_CONTROLS + 0x80)
#define EIP164_REG_SECY_STAT_POSTED_RES        (EIP164_CONF_BASE + 0xF280)

/* Interface group0 */
#define EIP164_REG_IFC_STATISTICS_CONTROLS  (EIP164_CONF_BASE + 0xE600)
#define EIP164_REG_IFC_COUNT_SUMMARY_(n)    (EIP164_CONF_BASE + 0xE700 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_OFFS * ((n)& 0x3F)))
#define EIP164_REG_IFC_SUMMARY_P            (EIP164_REG_IFC_STATISTICS_CONTROLS + 0x80)
#define EIP164_REG_IFC_STAT_POSTED_RES        (EIP164_CONF_BASE + 0xF2C0)

/* Interface group1 */
#define EIP164_REG_IFC1_STATISTICS_CONTROLS (EIP164_CONF_BASE + 0xE800)
#define EIP164_REG_IFC1_COUNT_SUMMARY_(n)   (EIP164_CONF_BASE + 0xE900 + \
                                             (0x10000 * ((n)>> 6)) + \
                                             (EIP164_REG_OFFS * ((n)& 0x3F)))
#define EIP164_REG_IFC1_SUMMARY_P            (EIP164_REG_IFC1_STATISTICS_CONTROLS + 0x80)
#define EIP164_REG_IFC1_STAT_POSTED_RES        (EIP164_CONF_BASE + 0xF300)

/* RxCAM control and Statistics */
#define EIP164_REG_RXCAM_STATISTICS_CONTROLS (EIP164_CONF_BASE + 0xEA00)
#define EIP164_REG_RXCAM_SUMMARY_P           (EIP164_REG_RXCAM_STATISTICS_CONTROLS + 0x80)
#define EIP164_REG_RXCAM_STAT_POSTED_RES        (EIP164_CONF_BASE + 0xF340)
/* RxCAM statistics counters: n - counter ID */
#define EIP164_REG_RXCAM_STATISTICS_ID_OFFS         0x08
#define EIP164_REG_RXCAM_STATISTICS_BANK_OFFS       0x10000 /* regs mirrored */
#define EIP164_REG_RXCAM_STATISTICS_OFFS            0xEC00
#define EIP164_REG_RXCAM_STATISTICS(n)  (EIP164_CONF_BASE + \
                                         EIP164_REG_RXCAM_STATISTICS_OFFS + \
                                         (EIP164_REG_RXCAM_STATISTICS_BANK_OFFS * \
                                          ((n) >> 6)) +                 \
                                         (EIP164_REG_RXCAM_STATISTICS_ID_OFFS * \
                                          ((n) & 0x3f)))

#define EIP164_RXCAM_STAT_HIT                       0

/* ------------ */
/* Output post-processor */
/* SA packet number threshold summary */
#define EIP164_REG_SA_PN_THR_SUMMARY(n)     (EIP164_CONF_BASE + 0xF000 + \
                                             0x10000 * ((n-1)>>6) +  \
                                             (EIP164_REG_OFFS * (((n)- 1)&0x3f)))
/* Post-proc statistics mode */
#define EIP164_REG_PP_STAT_CTRL             (EIP164_CONF_BASE + 0xF100)


/* Security failure counter masks & post-process error mask */
#define EIP164_REG_COUNT_SECFAIL1_DEFAULT   0x80FE0000
#define EIP164_REG_COUNT_SECFAIL1           (EIP164_CONF_BASE + 0xF124)
#define EIP164_REG_COUNT_SECFAIL2_DEFAULT   0x00000000
#define EIP164_REG_COUNT_SECFAIL2           (EIP164_CONF_BASE + 0xF128)


/* ------------ */
/* Max number of channels supported by the device register map */
/* Note: device configuration can support less channels */
#define EIP164_REG_CH_COUNT                 63

/* Input adapter */
#define EIP164_REG_RX_INSERT_SOP(n)         (EIP164_CONF_BASE + 0xFC40 + \
                                             (0x10000 * (n)))
#define EIP164_REG_RX_INSERT_EOP(n)         (EIP164_CONF_BASE + 0xFC44 + \
                                             (0x10000 * (n)))
#define EIP164_REG_RX_TIMEOUT_CTRL(n)       (EIP164_CONF_BASE + 0xFC48 + \
                                             (0x10000 * (n)))
#define EIP164_REG_RX_TIMEOUT_VAL           (EIP164_CONF_BASE + 0xFC4C)
#define EIP164_REG_RX_TIMEOUT_STATE(n)      (EIP164_CONF_BASE + 0xFC50 + \
                                             (0x10000 * (n)))
#define EIP164_REG_RX_FILTER_STATE(n)       (EIP164_CONF_BASE + 0xFC54 + \
                                             (0x10000 * (n)))

/* ------------ */
/* Channel controls */
#define EIP164_REG_CHANNEL_CTRL_DEFAULT     0x00000000
#define EIP164_REG_CHANNEL_CTRL(n)          ((EIP164_CONF_BASE + 0xFE00) + \
                                             (0x10000 * (n >> 5)) + \
                                             (EIP164_REG_OFFS * (n & 0x1F)))
/* ECC status */
#define EIP164_REG_ECC_COUNTER_RAM(n)       (EIP164_CONF_BASE + 0xFF00 + \
                                             (EIP164_REG_OFFS * n))
#define EIP164_REG_ECC_CORR_ENABLE          (EIP164_CONF_BASE + 0xFF60)
#define EIP164_REG_ECC_DERR_ENABLE          (EIP164_CONF_BASE + 0xFF64)
#define EIP164_REG_ECC_THR_CORR_STAT        (EIP164_CONF_BASE + 0xFF68)
#define EIP164_REG_ECC_THR_DERR_STAT        (EIP164_CONF_BASE + 0xFF6C)
#define EIP164_REG_ECC_THRESHOLDS           (EIP164_CONF_BASE + 0xFF78)

/* ------------ */
/* System control */
/* Next register is intended for channel switching, it reports if there are */
/* packets in flight for a channel */
#define EIP164_REG_IN_FLIGHT_GL(n)          (EIP164_CONF_BASE + 0xFFE0 + \
                                             (0x10000 * ((n) >> 5)))

/* Latency for MACsec mode */
#define EIP164_REG_LATENCY_CTRL_DEFAULT     0x00000050
#define EIP164_REG_LATENCY_CTRL             (EIP164_CONF_BASE + 0xFFE4)

/* Debug clocks related */
#define EIP164_REG_CLOCK_STATE              (EIP164_CONF_BASE + 0xFFE8)
#define EIP164_REG_FORCE_CLOCK_ON_DEFAULT   0x00000000
#define EIP164_REG_FORCE_CLOCK_ON           (EIP164_CONF_BASE + 0xFFEC)
#define EIP164_REG_FORCE_CLOCK_OFF_DEFAULT  0x00000000
#define EIP164_REG_FORCE_CLOCK_OFF          (EIP164_CONF_BASE + 0xFFF0)

/* EIP-164 configuration and version information registers */
#define EIP164_REG_CONFIG2                  (EIP164_CONF_BASE + 0xFFF4)
#define EIP164_REG_CONFIG                   (EIP164_CONF_BASE + 0xFFF8)
#define EIP164_REG_VERSION                  (EIP164_CONF_BASE + 0xFFFC)


#endif /* EIP164_HW_INTERFACE_H_ */


/* end of file eip164_hw_interface.h */
