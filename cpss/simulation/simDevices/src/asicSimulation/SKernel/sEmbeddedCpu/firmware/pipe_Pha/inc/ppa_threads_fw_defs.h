/*
 *  ppa_threads_fw_defs.c
 *
 *  PPA fw threads definitions
 *
 */

/**************************************************************************
* Descriptor fields description
* ************************************************************************/

/* PTP packet types indexes */
#define PTP_OVER_L2_TYPE                          1
#define PTP_OVER_MPLS_PW_TYPE                     2
#define PTP_OVER_UDP_OVER_IPV4_TYPE               8
#define PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE     9
#define PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE  12
#define PTP_OVER_UDP_OVER_IPV6_TYPE               18
#define PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE     19
#define PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE  22

/* PTP action field options */
enum nonQcn_desc_PTP_Action {
    PTP_ACTION_NONE = 0,                      /* 0x0 */
    PTP_ACTION_FORWARD,                       /* 0x1 */
    PTP_ACTION_DROP,                          /* 0x2 */
    PTP_ACTION_CAPTURE,                       /* 0x3 */
    PTP_ACTION_ADD_TIME,                      /* 0x4 */
    PTP_ACTION_ADD_CORRECTION_TIME,           /* 0x5 */
    PTP_ACTION_CAPTURE_ADD_TIME,              /* 0x6 */
    PTP_ACTION_CAPTURE_ADD_CORRECTION_TIME,   /* 0x7 */
    PTP_ACTION_ADD_INGRESS_TIME,              /* 0x8 */
    PTP_ACTION_CAPTURE_ADD_INGRESS_TIME,      /* 0x9 */
    PTP_ACTION_CAPTURE_INGRESS_TIME           /* 0xA */
};

/* Indicates the packet format for timestamping purposes */
enum nonQcn_desc_PTP_Packet_Format {
    PTP_PKT_FORMAT_PTPv2 = 0,                 /* 0x0   Compatible to IEEE 1588 v2. */
    PTP_PKT_FORMAT_PTPv1,                     /* 0x1   Compatible to IEEE 1588 v1. */
    PTP_PKT_FORMAT_Y1731,                     /* 0x2   Compatible to ITU-T Y.1731. */
    PTP_PKT_FORMAT_NTPTs,                     /* 0x3   Compatible to NTP timestamp. */
    PTP_PKT_FORMAT_NTPReceive,                /* 0x4   Compatible to NTP timestamp, and the timestamp is placed in the <Receive Timestamp> field */
    PTP_PKT_FORMAT_NTPTransmit,               /* 0x5   Compatible to NTP timestamp, and the timestamp is placed in the <Transmit Timestamp> field */
    PTP_PKT_FORMAT_WAMP,                      /* 0x6   Compatible to OWAMP / TWAMP packet format */
    PTP_PKT_FORMAT_Reserved                   /* 0x7 */
};

/**************************************************************************
* IP fields description
* ************************************************************************/
/* Operation of ECN (Explicit Congestion Notification) with IP */
enum ip_ecn_operation {
    IP_NON_ECN = 0,                           /* 0x0   Non ECN-Capable Transport, Non-ECT */
    IP_ECN_ECT_0,                             /* 0x1   ECN Capable Transport, ECT(0) */
    IP_ECN_ECT_1,                             /* 0x2   ECN Capable Transport, ECT(1) */
    IP_ECN_CE                                 /* 0x3   Congestion Encountered, CE. */
};


/*************************************************************************
* Common headers structures
* ************************************************************************/
struct IPv4_Header {
    uint16_t version:4;                         /* offset:0 */
    uint16_t ihl:4;                             /* offset:4 */
    uint16_t dscp:6;                            /* offset:8 */
    uint16_t ecn:2;                             /* offset:14 */
    uint16_t total_length:16;                   /* offset:16 */
    uint16_t identification:16;                 /* offset:32 */
    uint16_t flag_reserved:1;                   /* offset:48 */
    uint16_t flag_dont_fragment:1;              /* offset:49 */
    uint16_t flag_more_fragment:1;              /* offset:50 */
    uint16_t fragment_offset:13;                /* offset:51 */
    uint16_t ttl:8;                             /* offset:64 */
    uint16_t protocol:8;                        /* offset:72 */
    uint16_t header_checksum:16;                /* offset:80 */
    uint16_t sip_high:16;                       /* offset:96 */
    uint16_t sip_low:16;                        /* offset:112 */
    uint16_t dip_high:16;                       /* offset:128 */
    uint16_t dip_low:16;                        /* offset:144 */
}; /* 20B */

struct IPv6_Header {
    uint16_t version:4;                         /* offset:0 */
    uint16_t dscp:6;                            /* offset:4 */
    uint16_t ecn:2;                             /* offset:10 */
    uint16_t flow_label_19_16:4;                /* offset:12 */
    uint16_t flow_label_15_0:16;                /* offset:16 */
    uint16_t payload_length:16;                 /* offset:32 */
    uint16_t next_header:8;                     /* offset:48 */
    uint16_t hop_limit:8;                       /* offset:56 */
    uint16_t sip0_high:16;                      /* offset:64 */
    uint16_t sip0_low:16;                       /* offset:80 */
    uint16_t sip1_high:16;                      /* offset:96 */
    uint16_t sip1_low:16;                       /* offset:112 */
    uint16_t sip2_high:16;                      /* offset:128 */
    uint16_t sip2_low:16;                       /* offset:144 */
    uint16_t sip3_high:16;                      /* offset:160 */
    uint16_t sip3_low:16;                       /* offset:176 */
    uint16_t dip0_high:16;                      /* offset:192 */
    uint16_t dip0_low:16;                       /* offset:208 */
    uint16_t dip1_high:16;                      /* offset:224 */
    uint16_t dip1_low:16;                       /* offset:240 */
    uint16_t dip2_high:16;                      /* offset:256 */
    uint16_t dip2_low:16;                       /* offset:272 */
    uint16_t dip3_high:16;                      /* offset:288 */
    uint16_t dip3_low:16;                       /* offset:304 */
}; /* 40B */

struct ptp_header {
    uint16_t transportSpecific:4;               /* offset:0 */
    uint16_t messageType:4;                     /* offset:4 */
    uint16_t reserved0:4;                       /* offset:8 */
    uint16_t versionPTP:4;                      /* offset:12 */
    uint16_t messageLength:16;                  /* offset:16 */
    uint16_t domainNumber:8;                    /* offset:32 */
    uint16_t reserved1:8;                       /* offset:40 */
    uint16_t flagField:16;                      /* offset:48 */
    uint16_t correctionField_63_48:16;          /* offset:64 */
    uint16_t correctionField_47_32:16;          /* offset:80 */
    uint16_t correctionField_31_16:16;          /* offset:96 */
    uint16_t correctionField_15_0:16;           /* offset:112 */
    uint16_t reserved2:16;                      /* offset:128 */
    uint16_t reserved3:16;                      /* offset:144 */
    uint16_t sourcePortIdentity_79_64:16;       /* offset:160 */
    uint16_t sourcePortIdentity_63_48:16;       /* offset:176 */
    uint16_t sourcePortIdentity_47_32:16;       /* offset:192 */
    uint16_t sourcePortIdentity_31_16:16;       /* offset:208 */
    uint16_t sourcePortIdentity_15_0:16;        /* offset:224 */
    uint16_t sequenceId:16;                     /* offset:240 */
    uint16_t controlField:8;                    /* offset:256 */
    uint16_t logMessageInterval:8;              /* offset:264 */
}; /* 34B */

struct nonQcn_desc {
    uint32_t timestamp_Sec:2;                   /* offset:0 */
    uint32_t timestamp_NanoSec:30;              /* offset:2 */
    uint32_t is_ptp:1;                          /* offset:32    4  */
    uint32_t mark_ecn:1;                        /* offset:33 */
    uint32_t reserved_flags:6;                  /* offset:34 */
    uint32_t packet_hash_4_0:5;                 /* offset:40 */
    uint32_t ptp_u_field:1 ;                    /* offset:45 */
    uint32_t ptp_pkt_type_idx:5;                /* offset:46 */
    uint32_t ptp_offset:7;                      /* offset:51 */
    uint32_t outer_l3_offset:6;                 /* offset:58 */
    uint32_t up0:3;                             /* offset:64 8*/
    uint32_t dei:1;                             /* offset:67 */
    uint32_t dp_1:1;                            /* offset:68 */
    uint32_t dp_0:1;                            /* offset:69 */
    uint32_t packet_hash_6_5:2;                 /* offset:70 */
    uint32_t cnc_address:16;                    /* offset:72 */
    uint32_t txq_queue_id:8;                    /* offset:88 */
    uint32_t ingress_byte_count:14;             /* offset:96    12 */
    uint32_t table_hw_error_detected:1;         /* offset:110 */
    uint32_t header_hw_error_detected:1;        /* offset:111 */
    uint32_t tc:3;                              /* offset:112 */
    uint32_t packet_is_cut_through:1;           /* offset:115 */
    uint32_t cut_through_id:11;                 /* offset:116 */
    uint32_t need_crc_removal:1;                /* offset:127 */
    uint32_t txdma2ha_header_ctrl_bus:8;        /* offset:128   16 */
    uint32_t generate_cnc:1;                    /* offset:136 */
    uint32_t qcn_generated_by_local_dev:1;      /* offset:137 */
    uint32_t truncate_packet:1;                 /* offset:138 */
    uint32_t fw_drop:1;                         /* offset:139 */
    uint32_t desc_reserved:4;                   /* offset:140 */
    uint32_t ptp_dispatching_en:1;              /* offset:144 */
    uint32_t ptp_packet_format:3;               /* offset:145 */
    uint32_t mac_timestamping_en:1;             /* offset:148 */
    uint32_t ptp_cf_wraparound_check_en:1;      /* offset:149 */
    uint32_t ptp_timestamp_queue_entry_id:10;   /* offset:150 */
    uint32_t egress_pipe_delay:30;              /* offset:160   20 */
    uint32_t ingress_timestamp_seconds:2;       /* offset:190 */
    uint32_t timestamp_offset:8;                /* offset:192   24 */
    int32_t fw_bc_modification:8;               /* offset:200 */
    uint32_t ptp_action:4;                      /* offset:208 */
    uint32_t ptp_egress_tai_sel:1;              /* offset:212 */
    uint32_t ptp_timestamp_queue_select:1;      /* offset:213 */
    uint32_t add_crc:1;                         /* offset:214 */
    uint32_t udp_checksum_offset:8;             /* offset:215 */
    uint32_t udp_checksum_update_en:1;          /* offset:223 */
    uint32_t reserved_0:32;                     /* offset:224 */
}; /* 32B */

struct Qcn_desc {
    uint32_t qcn_q_offset:14;                   /* offset:0 */
    uint32_t orig_generated_QCN_prio:3;         /* offset:14 */
    uint32_t reserved:1;                        /* offset:17 */
    uint32_t qcn_q_delta:14;                    /* offset:18 */
    uint32_t mark_ecn_4qcn:1;                   /* offset:32 */
    uint32_t reserved_flags_4qcn:6 ;            /* offset:33 */
    uint32_t packet_hash_4qcn_bits_4_0:5;       /* offset:39 */
    uint32_t congested_queue_number:3;          /* offset:44 */
    uint32_t qcn_q_fb:6;                        /* offset:47 */
    uint32_t congested_port_number:5;           /* offset:53 */
    uint32_t Outer_l3_offset:6;                 /* offset:58 */
    uint32_t up0:3;                             /* offset:64 */
    uint32_t dei:1;                             /* offset:67 */
    uint32_t dp:2;                              /* offset:68 */
    uint32_t packet_hash_bits_6_5:2;            /* offset:70 */
    uint32_t CNC_address:16;                    /* offset:72 */
    uint32_t txq_queue_id:8;                    /* offset:88 */
    uint32_t ingress_byte_count:14;             /* offset:96 */
    uint32_t table_HW_error_detected:1;         /* offset:110 */
    uint32_t header_HW_error_detected:1;        /* offset:111 */
    uint32_t TC:3;                              /* offset:112 */
    uint32_t packet_is_cut_through:1;           /* offset:115 */
    uint32_t cut_through_id:11;                 /* offset:116 */
    uint32_t need_CRC_removal:1;                /* offset:127 */
    uint32_t txdma2ha_header_ctrl_bus:8;        /* offset:128 */
    uint32_t Generate_CNC:1;                    /* offset:136 */
    uint32_t QCN_generated_by_local_Dev :1;     /* offset:137 */
    uint32_t Truncate_packet:1;                 /* offset:138 */
    uint32_t fw_drop:1;                         /* offset:139 */
    uint32_t desc_reserved:4;                   /* offset:140 */
    uint32_t PTP_dispatching_En:1;              /* offset:144 */
    uint32_t PTP_packet_format:3;               /* offset:145 */
    uint32_t MAC_timestamping_en:1;             /* offset:148 */
    uint32_t PTP_CF_wraparound_check_en :1;     /* offset:149 */
    uint32_t PTP_timestamp_Queue_Entry_ID :10;  /* offset:150 */
    uint32_t egress_pipe_delay:30;              /* offset:160 */
    uint32_t ingress_timestamp_seconds:2;       /* offset:190 */
    uint32_t timestamp_offset:8;                /* offset:192 */
    int32_t fw_bc_modification:8;               /* offset:200 */
    uint32_t PTP_action:4;                      /* offset:208 */
    uint32_t PTP_Egress_TAI_sel:1;              /* offset:212 */
    uint32_t PTP_timestamp_Queue_select :1;     /* offset:213 */
    uint32_t add_CRC:1;                         /* offset:214 */
    uint32_t UDP_checksum_offset:8;             /* offset:215 */
    uint32_t UDP_checksum_update_En:1;          /* offset:223 */
    uint32_t reserved_0:32;                     /* offset:224 */
}; /* 32B */

/**************************************************************************
 * Thread 1 (E2U) structures
 * ************************************************************************/
struct thr1_e2u_cfg {
    struct {
        uint32_t E_PCP:3;                      /* offset:0 */
        uint32_t E_DEI:1;                      /* offset:3 */
        uint32_t Ingress_E_CID_base:12;        /* offset:4 */
        uint32_t IPL_Direction:1;              /* offset:16 */
        uint32_t Reserved:1;                   /* offset:17 */
        uint32_t GRP:2;                        /* offset:18 */
        uint32_t E_CID_base:12;                /* offset:20 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:32 */
        uint32_t E_CID_ext:8;                  /* offset:40 */
        uint32_t reserved:16;                  /* offset:48 */
    } etag; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:64 */
        uint32_t reserved_1:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 8B */
    struct {
        uint32_t PCID:12;                   /* offset:128 */
        uint32_t Default_VLAN_Tag:16;          /* offset:140 */
        uint32_t Uplink_Port:1;                 /* offset:156 */
        uint32_t reserved:3;                    /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved:3;                /* offset:189 */
        uint32_t PCID:12;                   /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr1_e2u_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr1_e2u_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
    } expansion_space; /* 12B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:96 */
        uint32_t mac_da_31_16:16;           /* offset:112 */
        uint32_t mac_da_15_0:16;            /* offset:128 */
        uint32_t mac_sa_47_32:16;           /* offset:144 */
        uint32_t mac_sa_31_16:16;           /* offset:160 */
        uint32_t mac_sa_15_0:16;            /* offset:176 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:192 */
        uint32_t E_PCP:3;                   /* offset:208 */
        uint32_t E_DEI:1;                   /* offset:211 */
        uint32_t Ingress_E_CID_base:12;     /* offset:212 */
        uint32_t IPL_Direction:1;           /* offset:224 */
        uint32_t Reserved:1;                /* offset:225 */
        uint32_t GRP:2;                     /* offset:226 */
        uint32_t E_CID_base:12;             /* offset:228 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:240 */
        uint32_t E_CID_ext:8;               /* offset:248 */
    } etag; /* 8B */
}; /* 32B */


/**************************************************************************
 * Thread 2 (U2E) structures
 * ************************************************************************/
struct thr2_u2e_cfg {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PCID:12;                   /* offset:128 */
        uint32_t Source_Filtering_Bitvector:16; /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t TargetPort:4;              /* offset:176 */
        uint32_t reserved:12;               /* offset:180 */
        uint32_t PCID:12;                   /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr2_u2e_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t E_PCP:3;                   /* offset:272 */
        uint32_t E_DEI:1;                   /* offset:275 */
        uint32_t Ingress_E_CID_base:12;     /* offset:276 */
        uint32_t IPL_Direction:1;           /* offset:288 */
        uint32_t Reserved:1;                /* offset:289 */
        uint32_t GRP:2;                     /* offset:290 */
        uint32_t E_CID_base:12;             /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:304 */
        uint32_t E_CID_ext:8;               /* offset:312 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                   /* offset:320 */
        uint32_t up:3;                      /* offset:336 */
        uint32_t cfi:1;                     /* offset:339 */
        uint32_t vid:12;                    /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr2_u2e_with_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
        uint32_t reserved_6:32;             /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:224 */
        uint32_t mac_da_31_16:16;           /* offset:240 */
        uint32_t mac_da_15_0:16;            /* offset:256 */
        uint32_t mac_sa_47_32:16;           /* offset:272 */
        uint32_t mac_sa_31_16:16;           /* offset:288 */
        uint32_t mac_sa_15_0:16;            /* offset:304 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:320 */
        uint32_t up:3;                      /* offset:336 */
        uint32_t cfi:1;                     /* offset:339 */
        uint32_t vid:12;                    /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr2_u2e_without_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
        uint32_t reserved_6:32;             /* offset:192 */
        uint32_t reserved_7:32;             /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:256 */
        uint32_t mac_da_31_16:16;           /* offset:272 */
        uint32_t mac_da_15_0:16;            /* offset:288 */
        uint32_t mac_sa_47_32:16;           /* offset:304 */
        uint32_t mac_sa_31_16:16;           /* offset:320 */
        uint32_t mac_sa_15_0:16;            /* offset:336 */
    } mac_header; /* 12B */
}; /* 44B */


/**************************************************************************
 * Thread 3 (U2C) structures
 * ************************************************************************/
struct thr3_u2c_cfg {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PCID:12;                   /* offset:128 */
        uint32_t Source_Filtering_Bitvector:16;  /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved:3;                /* offset:189 */
        uint32_t PCID:12;                   /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr3_u2c_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t E_PCP:3;                   /* offset:272 */
        uint32_t E_DEI:1;                   /* offset:275 */
        uint32_t Ingress_E_CID_base:12;     /* offset:276 */
        uint32_t IPL_Direction:1;           /* offset:288 */
        uint32_t Reserved:1;                /* offset:289 */
        uint32_t GRP:2;                     /* offset:290 */
        uint32_t E_CID_base:12;             /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:304 */
        uint32_t E_CID_ext:8;               /* offset:312 */
    } etag; /* 8B */
}; /* 40B */

struct thr3_u2c_with_etag_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t E_PCP:3;                   /* offset:272 */
        uint32_t E_DEI:1;                   /* offset:275 */
        uint32_t Ingress_E_CID_base:12;     /* offset:276 */
        uint32_t IPL_Direction:1;           /* offset:288 */
        uint32_t Reserved:1;                /* offset:289 */
        uint32_t GRP:2;                     /* offset:290 */
        uint32_t E_CID_base:12;             /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:304 */
        uint32_t E_CID_ext:8;               /* offset:312 */
    } etag; /* 8B */
}; /* 40B */

struct thr3_u2c_without_etag_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
        uint32_t reserved_6:32;             /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:224 */
        uint32_t mac_da_31_16:16;           /* offset:240 */
        uint32_t mac_da_15_0:16;            /* offset:256 */
        uint32_t mac_sa_47_32:16;           /* offset:272 */
        uint32_t mac_sa_31_16:16;           /* offset:288 */
        uint32_t mac_sa_15_0:16;            /* offset:304 */
    } mac_header; /* 12B */
}; /* 40B */


/**************************************************************************
 * Thread 4 (ET2U) structures
 * ************************************************************************/
struct thr4_et2u_cfg {
    struct {
        uint32_t tagCommand:2;              /* offset:0 */
        uint32_t srcTagged:1;               /* offset:2 */
        uint32_t srcDev:5;                  /* offset:3 */
        uint32_t SrcPort_4_0:5;             /* offset:8 */ /* validity: srcIsTrunk == 0 */
        uint32_t srcIsTrunk:1;              /* offset:13 */
        uint32_t core_id_2:1;               /* offset:14 */
        uint32_t cfi:1;                     /* offset:15 */
        uint32_t up:3;                      /* offset:16 */
        uint32_t extended:1;                /* offset:19 */
        uint32_t vid:12;                    /* offset:20 */
    } DSA_fwd; /* 4B */
    struct {
        uint32_t reserved_0:32;             /* offset:32 */
        uint32_t reserved_1:32;             /* offset:64 */
        uint32_t reserved_2:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 12B */
    struct {
        uint32_t PVID:12;                   /* offset:128 */
        uint32_t src_port_num:4;            /* offset:140 */
        uint32_t reserved:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved_0:3;              /* offset:189 */
        uint32_t reserved_1:12;             /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr4_et2u_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t cfi:1;                     /* offset:275 */
        uint32_t vid:12;                    /* offset:276 */
    } vlan; /* 4B */
}; /* 36B */

struct thr4_et2u_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;              /* offset:256 */
        uint32_t srcTagged:1;               /* offset:258 */
        uint32_t srcDev:5;                  /* offset:259 */
        uint32_t SrcPort_4_0:5;             /* offset:264 */ /* validity: srcIsTrunk == 0 */
        uint32_t srcIsTrunk:1;              /* offset:269 */
        uint32_t core_id_2:1;               /* offset:270 */
        uint32_t cfi:1;                     /* offset:271 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t extended:1;                /* offset:275 */
        uint32_t vid:12;                    /* offset:276 */
    } DSA_fwd; /* 4B */
}; /* 36B */


/**************************************************************************
 * Thread 5 (EU2U) structures
 * ************************************************************************/
struct thr5_eu2u_cfg {
    struct {
        uint32_t tagCommand:2;              /* offset:0 */
        uint32_t srcTagged:1;               /* offset:2 */
        uint32_t srcDev:5;                  /* offset:3 */
        uint32_t SrcPort_4_0:5;             /* offset:8 */ /* validity: srcIsTrunk == 0 */
        uint32_t srcIsTrunk:1;              /* offset:13 */
        uint32_t core_id_2:1;               /* offset:14 */
        uint32_t cfi:1;                     /* offset:15 */
        uint32_t up:3;                      /* offset:16 */
        uint32_t extended:1;                /* offset:19 */
        uint32_t vid:12;                    /* offset:20 */
    } DSA_fwd; /* 4B */
    struct {
        uint32_t reserved_0:32;             /* offset:32 */
        uint32_t reserved_1:32;             /* offset:64 */
        uint32_t reserved_2:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 12B */
    struct {
        uint32_t PVID:12;                   /* offset:128 */
        uint32_t src_port_num:4;            /* offset:140 */
        uint32_t reserved:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved_0:3;              /* offset:189 */
        uint32_t reserved_1:12;             /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr5_eu2u_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr5_eu2u_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
    } expansion_space; /* 16B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:128 */
        uint32_t mac_da_31_16:16;           /* offset:144 */
        uint32_t mac_da_15_0:16;            /* offset:160 */
        uint32_t mac_sa_47_32:16;           /* offset:176 */
        uint32_t mac_sa_31_16:16;           /* offset:192 */
        uint32_t mac_sa_15_0:16;            /* offset:208 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;              /* offset:224 */
        uint32_t srcTagged:1;               /* offset:226 */
        uint32_t srcDev:5;                  /* offset:227 */
        uint32_t SrcPort_4_0:5;             /* offset:232 */ /* validity: srcIsTrunk == 0 */
        uint32_t srcIsTrunk:1;              /* offset:237 */
        uint32_t core_id_2:1;               /* offset:238 */
        uint32_t cfi:1;                     /* offset:239 */
        uint32_t up:3;                      /* offset:240 */
        uint32_t extended:1;                /* offset:243 */
        uint32_t vid:12;                    /* offset:244 */
    } DSA_fwd; /* 4B */
}; /* 32B */


/**************************************************************************
 * Thread 6 (U2E) structures
 * ************************************************************************/
struct thr6_u2e_cfg {
    struct {
        uint32_t Own_Device:5;                 /* offset:0 */
        uint32_t Reserved:27;                  /* offset:5 */
    } HA_Table_Own_device; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:32 */
        uint32_t up:3;                         /* offset:48 */
        uint32_t cfi:1;                        /* offset:51 */
        uint32_t vid:12;                       /* offset:52 */
    } vlan; /* 4B */
    struct {
        uint32_t reserved_0:32;                /* offset:64 */
        uint32_t reserved_1:32;                /* offset:96 */
    } HA_Table_reserved_space; /* 8B */
    struct {
        uint32_t PVID:12;                   /* offset:128 */
        uint32_t src_port_num:4;            /* offset:140 */
        uint32_t reserved:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t TargetPort:4;              /* offset:176 */
        uint32_t reserved_0:12;             /* offset:180 */
        uint32_t reserved_1:12;             /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr6_u2e_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;              /* offset:256 */
        uint32_t srcTagged:1;               /* offset:258 */
        uint32_t srcDev:5;                  /* offset:259 */
        uint32_t SrcPort_4_0:5;             /* offset:264 */ /* validity: srcIsTrunk == 0 */
        uint32_t srcIsTrunk:1;              /* offset:269 */
        uint32_t core_id_2:1;               /* offset:270 */
        uint32_t cfi:1;                     /* offset:271 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t extended:1;                /* offset:275 */
        uint32_t vid:12;                    /* offset:276 */
    } extDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend:1;                  /* offset:288 */
        uint32_t SrcPort_6_5:2;             /* offset:289 */ /* validity: srcIsTrunk == 0 */
        uint32_t EgressFilterRegistered:1;  /* offset:291 */
        uint32_t DropOnSource:1;            /* offset:292 */
        uint32_t PacketIsLooped:1;          /* offset:293 */
        uint32_t routed:1;                  /* offset:294 */
        uint32_t Src_ID:5;                  /* offset:295 */
        uint32_t QosProfile:7;              /* offset:300 */
        uint32_t use_vidx:1;                /* offset:307 */
        uint32_t Reserved:1;                /* offset:308 */
        uint32_t TrgPort:6;                 /* offset:309 */
        uint32_t TrgDev:5;                  /* offset:315 */
    } extDSA_fwd_w1; /* 4B */
}; /* 40B */

struct thr6_u2e_with_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
    } expansion_space; /* 24B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:192 */
        uint32_t mac_da_31_16:16;           /* offset:208 */
        uint32_t mac_da_15_0:16;            /* offset:224 */
        uint32_t mac_sa_47_32:16;           /* offset:240 */
        uint32_t mac_sa_31_16:16;           /* offset:256 */
        uint32_t mac_sa_15_0:16;            /* offset:272 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:288 */
        uint32_t up:3;                      /* offset:304 */
        uint32_t cfi:1;                     /* offset:307 */
        uint32_t vid:12;                    /* offset:308 */
    } vlan; /* 4B */
}; /* 40B */

struct thr6_u2e_without_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
        uint32_t reserved_6:32;             /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:224 */
        uint32_t mac_da_31_16:16;           /* offset:240 */
        uint32_t mac_da_15_0:16;            /* offset:256 */
        uint32_t mac_sa_47_32:16;           /* offset:272 */
        uint32_t mac_sa_31_16:16;           /* offset:288 */
        uint32_t mac_sa_15_0:16;            /* offset:304 */
    } mac_header; /* 12B */
}; /* 40B */


/**************************************************************************
* Thread THR7_Mrr2E structures
* ************************************************************************/
struct thr7_mrr2e_cfg {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                   /* offset:128 */
        uint32_t src_port_num:4;            /* offset:140 */
        uint32_t reserved:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved_0:3;              /* offset:189 */
        uint32_t reserved_1:12;             /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr7_mrr2e_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;              /* offset:256 */
        uint32_t TrgTagged:1;               /* offset:258 */
        uint32_t TrgDev:5;                  /* offset:259 */
        uint32_t TrgPhyPort_4_0:5;          /* offset:264 */
        uint32_t use_eVIDX:1;               /* offset:269 */
        uint32_t TC0:1;                     /* offset:270 */
        uint32_t cfi:1;                     /* offset:271 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t Extend:1;                  /* offset:275 */
        uint32_t eVLAN:12;                  /* offset:276 */
    } DSA_from_cpu; /* 4B */
}; /* 36B */

struct thr7_mrr2e_unchange_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;              /* offset:256 */
        uint32_t TrgTagged:1;               /* offset:258 */
        uint32_t TrgDev:5;                  /* offset:259 */
        uint32_t TrgPhyPort_4_0:5;          /* offset:264 */
        uint32_t use_eVIDX:1;               /* offset:269 */
        uint32_t TC0:1;                     /* offset:270 */
        uint32_t cfi:1;                     /* offset:271 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t Extend:1;                  /* offset:275 */
        uint32_t eVLAN:12;                  /* offset:276 */
    } DSA_from_cpu; /* 4B */
}; /* 36B */

struct thr7_mrr2e_remove_dsa_from_cpu_tag_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
    } expansion_space; /* 24B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:192 */
        uint32_t mac_da_31_16:16;           /* offset:208 */
        uint32_t mac_da_15_0:16;            /* offset:224 */
        uint32_t mac_sa_47_32:16;           /* offset:240 */
        uint32_t mac_sa_31_16:16;           /* offset:256 */
        uint32_t mac_sa_15_0:16;            /* offset:272 */
    } mac_header; /* 12B */
}; /* 36B */


/**************************************************************************
* Thread THR8_E_V2U structures
* ************************************************************************/
struct thr8_e_v2u_cfg {
    struct {
        uint32_t TagCommand:2;              /* offset:0 */
        uint32_t Tag0SrcTagged:1;           /* offset:2 */
        uint32_t SrcDev_4_0:5;              /* offset:3 */
        uint32_t Src_ePort_4_0:5;           /* offset:8 */
        uint32_t SrcIsTrunk:1;              /* offset:13 */
        uint32_t Reserved0:1;               /* offset:14 */
        uint32_t CFI:1;                     /* offset:15 */
        uint32_t UP:3;                      /* offset:16 */
        uint32_t Extend0:1;                 /* offset:19 */
        uint32_t eVLAN_11_0:12;             /* offset:20 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                 /* offset:32 */
        uint32_t Src_ePort_6_5:2;           /* offset:33 */
        uint32_t EgressFilterRegistered:1;  /* offset:35 */
        uint32_t DropOnSource:1;            /* offset:36 */
        uint32_t PacketIsLooped:1;          /* offset:37 */
        uint32_t Routed:1;                  /* offset:38 */
        uint32_t SrcID_4_0:5;               /* offset:39 */
        uint32_t GlobalQoS_Profile:7;       /* offset:44 */
        uint32_t use_eVIDX:1;               /* offset:51 */
        uint32_t TrgPhyPort_6_0:7;          /* offset:52 */
        uint32_t TrgDev_4_0:5;              /* offset:59 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                 /* offset:64 */
        uint32_t Reserved1:1;               /* offset:65 */
        uint32_t IsTrgPhyPortValid:1;       /* offset:66 */
        uint32_t TrgPhyPort_7:1;            /* offset:67 */
        uint32_t SrcID_11_5:7;              /* offset:68 */
        uint32_t SrcDev_11_5:7;             /* offset:75 */
        uint32_t Reserved2:1;               /* offset:82 */
        uint32_t Src_ePort_16_7:10;         /* offset:83 */
        uint32_t TPID_index:3;              /* offset:93 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                 /* offset:96 */
        uint32_t eVLAN_15_12:4;             /* offset:97 */
        uint32_t Tag1SrcTagged:1;           /* offset:101 */
        uint32_t SrcTag0IsOuterTag:1;       /* offset:102 */
        uint32_t Reserved3:1;               /* offset:103 */
        uint32_t Trg_ePort:17;              /* offset:104 */
        uint32_t TrgDev:7;                  /* offset:121 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t src_ePort_13_7:7;          /* offset:128 */
        uint32_t src_ePort_6_5:2;           /* offset:135 */
        uint32_t src_ePort_4_0:5;           /* offset:137 */
        uint32_t reserved1:10;              /* offset:142 */
        uint32_t reserved2:8;               /* offset:159 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t trg_ePort_13_11:3;         /* offset:189 */
        uint32_t trg_ePort_10_0:11;         /* offset:192 */
        uint32_t reserved:1;                /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr8_e_v2u_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t up:3;                      /* offset:272 */
        uint32_t cfi:1;                     /* offset:275 */
        uint32_t vid:12;                    /* offset:276 */
    } vlan; /* 4B */
}; /* 36B */

struct thr8_e_v2u_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
    } expansion_space; /* 8B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:64 */
        uint32_t mac_da_31_16:16;            /* offset:80 */
        uint32_t mac_da_15_0:16;             /* offset:96 */
        uint32_t mac_sa_47_32:16;            /* offset:112 */
        uint32_t mac_sa_31_16:16;            /* offset:128 */
        uint32_t mac_sa_15_0:16;             /* offset:144 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;               /* offset:160 */
        uint32_t Tag0SrcTagged:1;            /* offset:162 */
        uint32_t SrcDev_4_0:5;               /* offset:163 */
        uint32_t Src_ePort_4_0:5;            /* offset:168 */
        uint32_t SrcIsTrunk:1;               /* offset:173 */
        uint32_t Reserved0:1;                /* offset:174 */
        uint32_t CFI:1;                      /* offset:175 */
        uint32_t UP:3;                       /* offset:176 */
        uint32_t Extend0:1;                  /* offset:179 */
        uint32_t eVLAN_11_0:12;              /* offset:180 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:192 */
        uint32_t Src_ePort_6_5:2;            /* offset:193 */
        uint32_t EgressFilterRegistered:1;   /* offset:195 */
        uint32_t DropOnSource:1;             /* offset:196 */
        uint32_t PacketIsLooped:1;           /* offset:197 */
        uint32_t Routed:1;                   /* offset:198 */
        uint32_t SrcID_4_0:5;                /* offset:199 */
        uint32_t GlobalQoS_Profile:7;        /* offset:204 */
        uint32_t use_eVIDX:1;                /* offset:211 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:212 */
        uint32_t TrgDev_4_0:5;               /* offset:219 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:224 */
        uint32_t Reserved1:1;                /* offset:225 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:226 */
        uint32_t TrgPhyPort_7:1;             /* offset:227 */
        uint32_t SrcID_11_5:7;               /* offset:228 */
        uint32_t SrcDev_11_5:7;              /* offset:235 */
        uint32_t Reserved2:1;                /* offset:242 */
        uint32_t Src_ePort_16_7:10;          /* offset:243 */
        uint32_t TPID_index:3;               /* offset:253 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:256 */
        uint32_t eVLAN_15_12:4;              /* offset:257 */
        uint32_t Tag1SrcTagged:1;            /* offset:261 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:262 */
        uint32_t Reserved3:1;                /* offset:263 */
        uint32_t Trg_ePort:17;               /* offset:264 */
        uint32_t TrgDev:7;                   /* offset:281 */
    } eDSA_fwd_w3; /* 4B */
}; /* 36B */


/**************************************************************************
* Thread THR9_E2U structures
* ************************************************************************/
struct thr9_e2u_cfg {
    struct {
        uint32_t TagCommand:2;               /* offset:0 */
        uint32_t Tag0SrcTagged:1;            /* offset:2 */
        uint32_t SrcDev_4_0:5;               /* offset:3 */
        uint32_t Src_ePort_4_0:5;            /* offset:8 */
        uint32_t SrcIsTrunk:1;               /* offset:13 */
        uint32_t Reserved0:1;                /* offset:14 */
        uint32_t CFI:1;                      /* offset:15 */
        uint32_t UP:3;                       /* offset:16 */
        uint32_t Extend0:1;                  /* offset:19 */
        uint32_t eVLAN_11_0:12;              /* offset:20 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:32 */
        uint32_t Src_ePort_6_5:2;            /* offset:33 */
        uint32_t EgressFilterRegistered:1;   /* offset:35 */
        uint32_t DropOnSource:1;             /* offset:36 */
        uint32_t PacketIsLooped:1;           /* offset:37 */
        uint32_t Routed:1;                   /* offset:38 */
        uint32_t SrcID_4_0:5;                /* offset:39 */
        uint32_t GlobalQoS_Profile:7;        /* offset:44 */
        uint32_t use_eVIDX:1;                /* offset:51 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:52 */
        uint32_t TrgDev_4_0:5;               /* offset:59 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:64 */
        uint32_t Reserved1:1;                /* offset:65 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:66 */
        uint32_t TrgPhyPort_7:1;             /* offset:67 */
        uint32_t SrcID_11_5:7;               /* offset:68 */
        uint32_t SrcDev_11_5:7;              /* offset:75 */
        uint32_t Reserved2:1;                /* offset:82 */
        uint32_t Src_ePort_16_7:10;          /* offset:83 */
        uint32_t TPID_index:3;               /* offset:93 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:96 */
        uint32_t eVLAN_15_12:4;              /* offset:97 */
        uint32_t Tag1SrcTagged:1;            /* offset:101 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:102 */
        uint32_t Reserved3:1;                /* offset:103 */
        uint32_t Trg_ePort:17;               /* offset:104 */
        uint32_t TrgDev:7;                   /* offset:121 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t src_ePort_13_7:7;           /* offset:128 */
        uint32_t src_ePort_6_5:2;            /* offset:135 */
        uint32_t src_ePort_4_0:5;            /* offset:137 */
        uint32_t reserved1:10;               /* offset:142 */
        uint32_t reserved2:8;                /* offset:159 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t trg_ePort_13_11:3;          /* offset:189 */
        uint32_t trg_ePort_10_0:11;          /* offset:192 */
        uint32_t reserved:1;                 /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr9_e2u_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr9_e2u_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
    } expansion_space; /* 4B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:32 */
        uint32_t mac_da_31_16:16;            /* offset:48 */
        uint32_t mac_da_15_0:16;             /* offset:64 */
        uint32_t mac_sa_47_32:16;            /* offset:80 */
        uint32_t mac_sa_31_16:16;            /* offset:96 */
        uint32_t mac_sa_15_0:16;             /* offset:112 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;               /* offset:128 */
        uint32_t Tag0SrcTagged:1;            /* offset:130 */
        uint32_t SrcDev_4_0:5;               /* offset:131 */
        uint32_t Src_ePort_4_0:5;            /* offset:136 */
        uint32_t SrcIsTrunk:1;               /* offset:141 */
        uint32_t Reserved0:1;                /* offset:142 */
        uint32_t CFI:1;                      /* offset:143 */
        uint32_t UP:3;                       /* offset:144 */
        uint32_t Extend0:1;                  /* offset:147 */
        uint32_t eVLAN_11_0:12;              /* offset:148 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:160 */
        uint32_t Src_ePort_6_5:2;            /* offset:161 */
        uint32_t EgressFilterRegistered:1;   /* offset:163 */
        uint32_t DropOnSource:1;             /* offset:164 */
        uint32_t PacketIsLooped:1;           /* offset:165 */
        uint32_t Routed:1;                   /* offset:166 */
        uint32_t SrcID_4_0:5;                /* offset:167 */
        uint32_t GlobalQoS_Profile:7;        /* offset:172 */
        uint32_t use_eVIDX:1;                /* offset:179 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:180 */
        uint32_t TrgDev_4_0:5;               /* offset:187 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:192 */
        uint32_t Reserved1:1;                /* offset:193 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:194 */
        uint32_t TrgPhyPort_7:1;             /* offset:195 */
        uint32_t SrcID_11_5:7;               /* offset:196 */
        uint32_t SrcDev_11_5:7;              /* offset:203 */
        uint32_t Reserved2:1;                /* offset:210 */
        uint32_t Src_ePort_16_7:10;          /* offset:211 */
        uint32_t TPID_index:3;               /* offset:221 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:224 */
        uint32_t eVLAN_15_12:4;              /* offset:225 */
        uint32_t Tag1SrcTagged:1;            /* offset:229 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:230 */
        uint32_t Reserved3:1;                /* offset:231 */
        uint32_t Trg_ePort:17;               /* offset:232 */
        uint32_t TrgDev:7;                   /* offset:249 */
    } eDSA_fwd_w3; /* 4B */
}; /* 32B */


/**************************************************************************
* Thread THR10_C_V2U structures
* ************************************************************************/
struct thr10_c_v2u_cfg {
    struct {
        uint32_t TagCommand:2;               /* offset:0 */
        uint32_t Tag0SrcTagged:1;            /* offset:2 */
        uint32_t SrcDev_4_0:5;               /* offset:3 */
        uint32_t Src_ePort_4_0:5;            /* offset:8 */
        uint32_t SrcIsTrunk:1;               /* offset:13 */
        uint32_t Reserved0:1;                /* offset:14 */
        uint32_t CFI:1;                      /* offset:15 */
        uint32_t UP:3;                       /* offset:16 */
        uint32_t Extend0:1;                  /* offset:19 */
        uint32_t eVLAN_11_0:12;              /* offset:20 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:32 */
        uint32_t Src_ePort_6_5:2;            /* offset:33 */
        uint32_t EgressFilterRegistered:1;   /* offset:35 */
        uint32_t DropOnSource:1;             /* offset:36 */
        uint32_t PacketIsLooped:1;           /* offset:37 */
        uint32_t Routed:1;                   /* offset:38 */
        uint32_t SrcID_4_0:5;                /* offset:39 */
        uint32_t GlobalQoS_Profile:7;        /* offset:44 */
        uint32_t use_eVIDX:1;                /* offset:51 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:52 */
        uint32_t TrgDev_4_0:5;               /* offset:59 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:64 */
        uint32_t Reserved1:1;                /* offset:65 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:66 */
        uint32_t TrgPhyPort_7:1;             /* offset:67 */
        uint32_t SrcID_11_5:7;               /* offset:68 */
        uint32_t SrcDev_11_5:7;              /* offset:75 */
        uint32_t Reserved2:1;                /* offset:82 */
        uint32_t Src_ePort_16_7:10;          /* offset:83 */
        uint32_t TPID_index:3;               /* offset:93 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:96 */
        uint32_t eVLAN_15_12:4;              /* offset:97 */
        uint32_t Tag1SrcTagged:1;            /* offset:101 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:102 */
        uint32_t Reserved3:1;                /* offset:103 */
        uint32_t Trg_ePort:17;               /* offset:104 */
        uint32_t TrgDev:7;                   /* offset:121 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t src_ePort_13_7:7;           /* offset:128 */
        uint32_t src_ePort_6_5:2;            /* offset:135 */
        uint32_t src_ePort_4_0:5;            /* offset:137 */
        uint32_t reserved1:2;                /* offset:142 */
        uint32_t reserved2:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t trg_ePort_13_11:3;          /* offset:189 */
        uint32_t trg_ePort_10_0:11;          /* offset:192 */
        uint32_t reserved:1;                 /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr10_c_v2u_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                    /* offset:256 */
        uint32_t E_PCP:3;                    /* offset:272 */
        uint32_t E_DEI:1;                    /* offset:275 */
        uint32_t Ingress_E_CID_base:12;      /* offset:276 */
        uint32_t IPL_Direction:1;              /* offset:288 */
        uint32_t Reserved:1;                   /* offset:289 */
        uint32_t GRP:2;                      /* offset:290 */
        uint32_t E_CID_base:12;              /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;        /* offset:304 */
        uint32_t E_CID_ext:8;                /* offset:312 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                    /* offset:320 */
        uint32_t up:3;                       /* offset:336 */
        uint32_t cfi:1;                      /* offset:339 */
        uint32_t vid:12;                     /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr10_c_v2u_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
    } expansion_space; /* 16B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:128 */
        uint32_t mac_da_31_16:16;            /* offset:144 */
        uint32_t mac_da_15_0:16;             /* offset:160 */
        uint32_t mac_sa_47_32:16;            /* offset:176 */
        uint32_t mac_sa_31_16:16;            /* offset:192 */
        uint32_t mac_sa_15_0:16;             /* offset:208 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;               /* offset:224 */
        uint32_t Tag0SrcTagged:1;            /* offset:226 */
        uint32_t SrcDev_4_0:5;               /* offset:227 */
        uint32_t Src_ePort_4_0:5;            /* offset:232 */
        uint32_t SrcIsTrunk:1;               /* offset:237 */
        uint32_t Reserved0:1;                /* offset:238 */
        uint32_t CFI:1;                      /* offset:239 */
        uint32_t UP:3;                       /* offset:240 */
        uint32_t Extend0:1;                  /* offset:243 */
        uint32_t eVLAN_11_0:12;              /* offset:244 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:256 */
        uint32_t Src_ePort_6_5:2;            /* offset:257 */
        uint32_t EgressFilterRegistered:1;   /* offset:259 */
        uint32_t DropOnSource:1;             /* offset:260 */
        uint32_t PacketIsLooped:1;           /* offset:261 */
        uint32_t Routed:1;                   /* offset:262 */
        uint32_t SrcID_4_0:5;                /* offset:263 */
        uint32_t GlobalQoS_Profile:7;        /* offset:268 */
        uint32_t use_eVIDX:1;                /* offset:275 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:276 */
        uint32_t TrgDev_4_0:5;               /* offset:283 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:288 */
        uint32_t Reserved1:1;                /* offset:289 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:290 */
        uint32_t TrgPhyPort_7:1;             /* offset:291 */
        uint32_t SrcID_11_5:7;               /* offset:292 */
        uint32_t SrcDev_11_5:7;              /* offset:299 */
        uint32_t Reserved2:1;                /* offset:306 */
        uint32_t Src_ePort_16_7:10;          /* offset:307 */
        uint32_t TPID_index:3;               /* offset:317 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:320 */
        uint32_t eVLAN_15_12:4;              /* offset:321 */
        uint32_t Tag1SrcTagged:1;            /* offset:325 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:326 */
        uint32_t Reserved3:1;                /* offset:327 */
        uint32_t Trg_ePort:17;               /* offset:328 */
        uint32_t TrgDev:7;                   /* offset:345 */
    } eDSA_fwd_w3; /* 4B */
}; /* 44B */

/**************************************************************************
* Thread THR11_C2U structures
* ************************************************************************/
struct thr11_c2u_cfg {
    struct {
        uint32_t TagCommand:2;               /* offset:0 */
        uint32_t Tag0SrcTagged:1;            /* offset:2 */
        uint32_t SrcDev_4_0:5;               /* offset:3 */
        uint32_t Src_ePort_4_0:5;            /* offset:8 */
        uint32_t SrcIsTrunk:1;               /* offset:13 */
        uint32_t Reserved0:1;                /* offset:14 */
        uint32_t CFI:1;                      /* offset:15 */
        uint32_t UP:3;                       /* offset:16 */
        uint32_t Extend0:1;                  /* offset:19 */
        uint32_t eVLAN_11_0:12;              /* offset:20 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:32 */
        uint32_t Src_ePort_6_5:2;            /* offset:33 */
        uint32_t EgressFilterRegistered:1;   /* offset:35 */
        uint32_t DropOnSource:1;             /* offset:36 */
        uint32_t PacketIsLooped:1;           /* offset:37 */
        uint32_t Routed:1;                   /* offset:38 */
        uint32_t SrcID_4_0:5;                /* offset:39 */
        uint32_t GlobalQoS_Profile:7;        /* offset:44 */
        uint32_t use_eVIDX:1;                /* offset:51 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:52 */
        uint32_t TrgDev_4_0:5;               /* offset:59 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:64 */
        uint32_t Reserved1:1;                /* offset:65 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:66 */
        uint32_t TrgPhyPort_7:1;             /* offset:67 */
        uint32_t SrcID_11_5:7;               /* offset:68 */
        uint32_t SrcDev_11_5:7;              /* offset:75 */
        uint32_t Reserved2:1;                /* offset:82 */
        uint32_t Src_ePort_16_7:10;          /* offset:83 */
        uint32_t TPID_index:3;               /* offset:93 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:96 */
        uint32_t eVLAN_15_12:4;              /* offset:97 */
        uint32_t Tag1SrcTagged:1;            /* offset:101 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:102 */
        uint32_t Reserved3:1;                /* offset:103 */
        uint32_t Trg_ePort:17;               /* offset:104 */
        uint32_t TrgDev:7;                   /* offset:121 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t src_ePort_13_7:7;           /* offset:128 */
        uint32_t src_ePort_6_5:2;            /* offset:135 */
        uint32_t src_ePort_4_0:5;            /* offset:137 */
        uint32_t reserved1:2;                /* offset:142 */
        uint32_t reserved2:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t trg_ePort_13_11:3;          /* offset:189 */
        uint32_t trg_ePort_10_0:11;          /* offset:192 */
        uint32_t reserved:1;                 /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr11_c2u_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                    /* offset:256 */
        uint32_t E_PCP:3;                    /* offset:272 */
        uint32_t E_DEI:1;                    /* offset:275 */
        uint32_t Ingress_E_CID_base:12;      /* offset:276 */
        uint32_t IPL_Direction:1;              /* offset:288 */
        uint32_t Reserved:1;                   /* offset:289 */
        uint32_t GRP:2;                      /* offset:290 */
        uint32_t E_CID_base:12;              /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;        /* offset:304 */
        uint32_t E_CID_ext:8;                /* offset:312 */
    } etag; /* 8B */
}; /* 40B */

struct thr11_c2u_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
    } expansion_space; /* 12B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:96 */
        uint32_t mac_da_31_16:16;            /* offset:112 */
        uint32_t mac_da_15_0:16;             /* offset:128 */
        uint32_t mac_sa_47_32:16;            /* offset:144 */
        uint32_t mac_sa_31_16:16;            /* offset:160 */
        uint32_t mac_sa_15_0:16;             /* offset:176 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;               /* offset:192 */
        uint32_t Tag0SrcTagged:1;            /* offset:194 */
        uint32_t SrcDev_4_0:5;               /* offset:195 */
        uint32_t Src_ePort_4_0:5;            /* offset:200 */
        uint32_t SrcIsTrunk:1;               /* offset:205 */
        uint32_t Reserved0:1;                /* offset:206 */
        uint32_t CFI:1;                      /* offset:207 */
        uint32_t UP:3;                       /* offset:208 */
        uint32_t Extend0:1;                  /* offset:211 */
        uint32_t eVLAN_11_0:12;              /* offset:212 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:224 */
        uint32_t Src_ePort_6_5:2;            /* offset:225 */
        uint32_t EgressFilterRegistered:1;   /* offset:227 */
        uint32_t DropOnSource:1;             /* offset:228 */
        uint32_t PacketIsLooped:1;           /* offset:229 */
        uint32_t Routed:1;                   /* offset:230 */
        uint32_t SrcID_4_0:5;                /* offset:231 */
        uint32_t GlobalQoS_Profile:7;        /* offset:236 */
        uint32_t use_eVIDX:1;                /* offset:243 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:244 */
        uint32_t TrgDev_4_0:5;               /* offset:251 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:256 */
        uint32_t Reserved1:1;                /* offset:257 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:258 */
        uint32_t TrgPhyPort_7:1;             /* offset:259 */
        uint32_t SrcID_11_5:7;               /* offset:260 */
        uint32_t SrcDev_11_5:7;              /* offset:267 */
        uint32_t Reserved2:1;                /* offset:274 */
        uint32_t Src_ePort_16_7:10;          /* offset:275 */
        uint32_t TPID_index:3;               /* offset:285 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:288 */
        uint32_t eVLAN_15_12:4;              /* offset:289 */
        uint32_t Tag1SrcTagged:1;            /* offset:293 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:294 */
        uint32_t Reserved3:1;                /* offset:295 */
        uint32_t Trg_ePort:17;               /* offset:296 */
        uint32_t TrgDev:7;                   /* offset:313 */
    } eDSA_fwd_w3; /* 4B */
}; /* 40B */


/**************************************************************************
* Thread THR12_U_UC2C structures
* ************************************************************************/
struct thr12_u_uc2c_cfg {
    struct {
        uint32_t E_PCP:3;                      /* offset:0 */
        uint32_t E_DEI:1;                      /* offset:3 */
        uint32_t Ingress_E_CID_base:12;        /* offset:4 */
        uint32_t IPL_Direction:1;              /* offset:16 */
        uint32_t Reserved:1;                   /* offset:17 */
        uint32_t GRP:2;                        /* offset:18 */
        uint32_t E_CID_base:12;                /* offset:20 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:32 */
        uint32_t E_CID_ext:8;                  /* offset:40 */
        uint32_t reserved:16;                  /* offset:48 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                    /* offset:64 */
        uint32_t up:3;                       /* offset:80 */
        uint32_t cfi:1;                      /* offset:83 */
        uint32_t vid:12;                     /* offset:84 */
    } vlan; /* 4B */
    struct {
        uint32_t trg_ePort_full:14;          /* offset:96 */
        uint32_t reserved:2;                 /* offset:110 */
        uint32_t fw_bc_modification_temp:16; /* offset:112 */
    } trg_ePort; /* 4B */
    struct {
        uint32_t src_ePort:14;               /* offset:128 */
        uint32_t reserved:2;                 /* offset:142 */
        uint32_t reserved1:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t trg_ePort_13_11:3;          /* offset:189 */
        uint32_t trg_ePort_10_0:11;          /* offset:192 */
        uint32_t reserved:1;                 /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr12_u_uc2c_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;               /* offset:256 */
        uint32_t Tag0SrcTagged:1;            /* offset:258 */
        uint32_t SrcDev_4_0:5;               /* offset:259 */
        uint32_t Src_ePort_4_0:5;            /* offset:264 */
        uint32_t SrcIsTrunk:1;               /* offset:269 */
        uint32_t Reserved0:1;                /* offset:270 */
        uint32_t CFI:1;                      /* offset:271 */
        uint32_t UP:3;                       /* offset:272 */
        uint32_t Extend0:1;                  /* offset:275 */
        uint32_t eVLAN_11_0:12;              /* offset:276 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                  /* offset:288 */
        uint32_t Src_ePort_6_5:2;            /* offset:289 */
        uint32_t EgressFilterRegistered:1;   /* offset:291 */
        uint32_t DropOnSource:1;             /* offset:292 */
        uint32_t PacketIsLooped:1;           /* offset:293 */
        uint32_t Routed:1;                   /* offset:294 */
        uint32_t SrcID_4_0:5;                /* offset:295 */
        uint32_t GlobalQoS_Profile:7;        /* offset:300 */
        uint32_t use_eVIDX:1;                /* offset:307 */
        uint32_t TrgPhyPort_6_0:7;           /* offset:308 */
        uint32_t TrgDev_4_0:5;               /* offset:315 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                  /* offset:320 */
        uint32_t Reserved1:1;                /* offset:321 */
        uint32_t IsTrgPhyPortValid:1;        /* offset:322 */
        uint32_t TrgPhyPort_7:1;             /* offset:323 */
        uint32_t SrcID_11_5:7;               /* offset:324 */
        uint32_t SrcDev_11_5:7;              /* offset:331 */
        uint32_t Reserved2:1;                /* offset:338 */
        uint32_t Src_ePort_16_7:10;          /* offset:339 */
        uint32_t TPID_index:3;               /* offset:349 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                  /* offset:352 */
        uint32_t eVLAN_15_12:4;              /* offset:353 */
        uint32_t Tag1SrcTagged:1;            /* offset:357 */
        uint32_t SrcTag0IsOuterTag:1;        /* offset:358 */
        uint32_t Reserved3:1;                /* offset:359 */
        uint32_t Trg_ePort:17;               /* offset:360 */
        uint32_t TrgDev:7;                   /* offset:377 */
    } eDSA_fwd_w3; /* 4B */
}; /* 48B */

struct thr12_u_uc2c_add_vlan_and_etag_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
        uint32_t reserved_5:32;              /* offset:160 */
    } expansion_space; /* 24B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:192 */
        uint32_t mac_da_31_16:16;            /* offset:208 */
        uint32_t mac_da_15_0:16;             /* offset:224 */
        uint32_t mac_sa_47_32:16;            /* offset:240 */
        uint32_t mac_sa_31_16:16;            /* offset:256 */
        uint32_t mac_sa_15_0:16;             /* offset:272 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                    /* offset:288 */
        uint32_t E_PCP:3;                    /* offset:304 */
        uint32_t E_DEI:1;                    /* offset:307 */
        uint32_t Ingress_E_CID_base:12;      /* offset:308 */
        uint32_t IPL_Direction:1;              /* offset:320 */
        uint32_t Reserved:1;                   /* offset:321 */
        uint32_t GRP:2;                      /* offset:322 */
        uint32_t E_CID_base:12;              /* offset:324 */
        uint32_t Ingress_E_CID_ext:8;        /* offset:336 */
        uint32_t E_CID_ext:8;                /* offset:344 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                    /* offset:352 */
        uint32_t up:3;                       /* offset:368 */
        uint32_t cfi:1;                      /* offset:371 */
        uint32_t vid:12;                     /* offset:372 */
    } vlan; /* 4B */
}; /* 48B */

struct thr12_u_uc2c_remove_eDSA_tag_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
        uint32_t reserved_5:32;              /* offset:160 */
        uint32_t reserved_6:32;              /* offset:192 */
        uint32_t reserved_7:32;              /* offset:224 */
        uint32_t reserved_8:32;              /* offset:256 */
    } expansion_space; /* 36B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:288 */
        uint32_t mac_da_31_16:16;            /* offset:304 */
        uint32_t mac_da_15_0:16;             /* offset:320 */
        uint32_t mac_sa_47_32:16;            /* offset:336 */
        uint32_t mac_sa_31_16:16;            /* offset:352 */
        uint32_t mac_sa_15_0:16;             /* offset:368 */
    } mac_header; /* 12B */
}; /* 48B */


/**************************************************************************
 * Thread 13 (U_MC2C) structures
 * ************************************************************************/
struct thr13_u_mc2c_cfg {
    struct {
        uint32_t E_PCP:3;                      /* offset:0 */
        uint32_t E_DEI:1;                      /* offset:3 */
        uint32_t Ingress_E_CID_base:12;        /* offset:4 */
        uint32_t eVIDX_full:16;                /* offset:16 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:32 */
        uint32_t E_CID_ext:8;                  /* offset:40 */
        uint32_t reserved:16;                  /* offset:48 */
    } etag_u_mc2c; /* 8B */
    struct {
        uint32_t TPID:16;                   /* offset:64 */
        uint32_t up:3;                      /* offset:80 */
        uint32_t cfi:1;                     /* offset:83 */
        uint32_t eVLAN:12;                  /* offset:84 */
    } vlan_eDSA; /* 4B */
    struct {
        uint32_t src_ePort_full:17;         /* offset:96 */
        uint32_t reserved:15;               /* offset:113 */
    } src_ePort; /* 4B */
    struct {
        uint32_t src_ePort:14;              /* offset:128 */
        uint32_t reserved:2;                /* offset:142 */
        uint32_t reserved1:16;              /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t trg_ePort_13_11:3;         /* offset:189 */
        uint32_t trg_ePort_10_0:11;         /* offset:192 */
        uint32_t reserved:1;                /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr13_u_mc2c_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;              /* offset:256 */
        uint32_t Tag0SrcTagged:1;           /* offset:258 */
        uint32_t SrcDev_4_0:5;              /* offset:259 */
        uint32_t Src_ePort_4_0:5;           /* offset:264 */
        uint32_t SrcIsTrunk:1;              /* offset:269 */
        uint32_t Reserved0:1;               /* offset:270 */
        uint32_t CFI:1;                     /* offset:271 */
        uint32_t UP:3;                      /* offset:272 */
        uint32_t Extend0:1;                 /* offset:275 */
        uint32_t eVLAN_11_0:12;             /* offset:276 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                 /* offset:288 */
        uint32_t Src_ePort_6_5:2;           /* offset:289 */
        uint32_t EgressFilterRegistered:1;  /* offset:291 */
        uint32_t DropOnSource:1;            /* offset:292 */
        uint32_t PacketIsLooped:1;          /* offset:293 */
        uint32_t Routed:1;                  /* offset:294 */
        uint32_t SrcID_4_0:5;               /* offset:295 */
        uint32_t GlobalQoS_Profile:7;       /* offset:300 */
        uint32_t use_eVIDX:1;               /* offset:307 */
        uint32_t eVIDX_11_0:12;             /* offset:308 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                 /* offset:320 */
        uint32_t Reserved1:1;               /* offset:321 */
        uint32_t IsTrgPhyPortValid:1;       /* offset:322 */
        uint32_t TrgPhyPort_7:1;            /* offset:323 */
        uint32_t SrcID_11_5:7;              /* offset:324 */
        uint32_t SrcDev_11_5:7;             /* offset:331 */
        uint32_t Reserved2:1;               /* offset:338 */
        uint32_t Src_ePort_16_7:10;         /* offset:339 */
        uint32_t TPID_index:3;              /* offset:349 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                 /* offset:352 */
        uint32_t eVLAN_15_12:4;             /* offset:353 */
        uint32_t Tag1SrcTagged:1;           /* offset:357 */
        uint32_t SrcTag0IsOuterTag:1;       /* offset:358 */
        uint32_t Reserved3:1;               /* offset:359 */
        uint32_t eVIDX_15_12:4;             /* offset:360 */
        uint32_t orig_src_phy_is_trunk:1;   /* offset:364 */
        uint32_t orig_src_phy_port:12;      /* offset:365 */
        uint32_t phy_src_mc_filter_en:1;    /* offset:377 */
        uint32_t reserved6:6;               /* offset:378 */
    } eDSA_fwd_w3; /* 4B */
}; /* 48B */

struct thr13_u_mc2c_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
    } expansion_space; /* 24B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:192 */
        uint32_t mac_da_31_16:16;           /* offset:208 */
        uint32_t mac_da_15_0:16;            /* offset:224 */
        uint32_t mac_sa_47_32:16;           /* offset:240 */
        uint32_t mac_sa_31_16:16;           /* offset:256 */
        uint32_t mac_sa_15_0:16;            /* offset:272 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:288 */
        uint32_t E_PCP:3;                   /* offset:304 */
        uint32_t E_DEI:1;                   /* offset:307 */
        uint32_t Ingress_E_CID_base:12;     /* offset:308 */
        uint32_t eVIDX_full:16;             /* offset:320 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:336 */
        uint32_t E_CID_ext:8;               /* offset:344 */
    } etag_u_mc2c; /* 8B */
    struct {
        uint32_t TPID:16;                   /* offset:352 */
        uint32_t up:3;                      /* offset:368 */
        uint32_t cfi:1;                     /* offset:371 */
        uint32_t eVLAN:12;                  /* offset:372 */
    } vlan_eDSA; /* 4B */
}; /* 48B */


/**************************************************************************
* Thread THR14_U_MR2C structures
* ************************************************************************/
struct thr14_u_mr2c_cfg {
    struct {
        uint32_t E_PCP:3;                      /* offset:0 */
        uint32_t E_DEI:1;                      /* offset:3 */
        uint32_t Ingress_E_CID_base:12;        /* offset:4 */
        uint32_t IPL_Direction:1;              /* offset:16 */
        uint32_t Reserved:1;                   /* offset:17 */
        uint32_t GRP:2;                        /* offset:18 */
        uint32_t E_CID_base:12;                /* offset:20 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:32 */
        uint32_t E_CID_ext:8;                  /* offset:40 */
        uint32_t reserved:16;                  /* offset:48 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                   /* offset:64 */
        uint32_t up:3;                      /* offset:80 */
        uint32_t cfi:1;                     /* offset:83 */
        uint32_t vid:12;                    /* offset:84 */
    } vlan; /* 4B */
    struct {
        uint32_t trg_ePort_full:14;         /* offset:96 */
        uint32_t reserved:2;                /* offset:110 */
        uint32_t fw_bc_modification_temp:16;/* offset:112 */
    } trg_ePort; /* 4B */
    struct {
        uint32_t src_ePort:14;      /* offset:128 */
        uint32_t reserved:2;                /* offset:142 */
        uint32_t reserved1:16;              /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t trg_ePort_13_11:3;         /* offset:189 */
        uint32_t trg_ePort_10_0:11;         /* offset:192 */
        uint32_t reserved:1;                /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr14_u_mr2c_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;              /* offset:256 */
        uint32_t Tag0SrcTagged:1;           /* offset:258 */
        uint32_t SrcDev_4_0:5;              /* offset:259 */
        uint32_t Src_Phy_Port_4_0:5;        /* offset:264 */
        uint32_t RxSniff:1;                 /* offset:269 */
        uint32_t Reserved0:1;               /* offset:270 */
        uint32_t CFI:1;                     /* offset:271 */
        uint32_t UP:3;                      /* offset:272 */
        uint32_t Extend0:1;                 /* offset:275 */
        uint32_t eVLAN_11_0:12;             /* offset:276 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Extend1:1;                 /* offset:288 */
        uint32_t Reserved1:1;               /* offset:290 */
        uint32_t DropOnSource:1;            /* offset:291 */
        uint32_t PacketIsLooped:1;          /* offset:292 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;      /* offset:293 */
        uint32_t Analyzer_Use_eVIDX:1;      /* offset:294 */
        uint32_t Analyzer_Trg_Dev:12;       /* offset:295 */
        uint32_t Reserved2:2;               /* offset:307 */
        uint32_t Src_Phy_Port_6:1;          /* offset:308 */
        uint32_t Src_Phy_Port_5:1;          /* offset:309 */
        uint32_t Reserved3:2;               /* offset:310 */
        uint32_t Analyzer_Trg_Phy_Port:8;   /* offset:312 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t Extend2:1;                 /* offset:320 */
        uint32_t Reserved1:10;              /* offset:321 */
        uint32_t Src_Phy_Port_7:1;          /* offset:331 */
        uint32_t Trg_ePort:17;              /* offset:332 */
        uint32_t TPID_index:3;              /* offset:349 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t Extend3:1;                 /* offset:352 */
        uint32_t eVLAN_15_12:4;             /* offset:353 */
        uint32_t Reserved1:3;               /* offset:357 */
        uint32_t Analyzer_ePort:17;         /* offset:360 */
        uint32_t SrcDev_11_5:7;             /* offset:377 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 48B */

struct thr14_u_mr2c_add_vlan_and_etag_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
    } expansion_space; /* 24B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:192 */
        uint32_t mac_da_31_16:16;           /* offset:208 */
        uint32_t mac_da_15_0:16;            /* offset:224 */
        uint32_t mac_sa_47_32:16;           /* offset:240 */
        uint32_t mac_sa_31_16:16;           /* offset:256 */
        uint32_t mac_sa_15_0:16;            /* offset:272 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:288 */
        uint32_t E_PCP:3;                   /* offset:304 */
        uint32_t E_DEI:1;                   /* offset:307 */
        uint32_t Ingress_E_CID_base:12;     /* offset:308 */
        uint32_t IPL_Direction:1;              /* offset:320 */
        uint32_t Reserved:1;                   /* offset:321 */
        uint32_t GRP:2;                     /* offset:322 */
        uint32_t E_CID_base:12;             /* offset:324 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:336 */
        uint32_t E_CID_ext:8;               /* offset:344 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                   /* offset:352 */
        uint32_t up:3;                      /* offset:368 */
        uint32_t cfi:1;                     /* offset:371 */
        uint32_t vid:12;                    /* offset:372 */
    } vlan; /* 4B */
}; /* 48B */

struct thr14_u_mr2c_remove_eDSA_tag_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
        uint32_t reserved_5:32;             /* offset:160 */
        uint32_t reserved_6:32;             /* offset:192 */
        uint32_t reserved_7:32;             /* offset:224 */
        uint32_t reserved_8:32;             /* offset:256 */
    } expansion_space; /* 36B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:288 */
        uint32_t mac_da_31_16:16;           /* offset:304 */
        uint32_t mac_da_15_0:16;            /* offset:320 */
        uint32_t mac_sa_47_32:16;           /* offset:336 */
        uint32_t mac_sa_31_16:16;           /* offset:352 */
        uint32_t mac_sa_15_0:16;            /* offset:368 */
    } mac_header; /* 12B */
}; /* 48B */


/**************************************************************************
 * Thread 15 (QCN) structures
 * ************************************************************************/
struct thr15_qcn_cfg {
    struct {
        uint32_t sa_mac_47_32:16;                   /* offset: 0 */
        uint32_t sa_mac_31_16:16;                   /* offset: 16 */
        uint32_t sa_mac_15_0:16;                    /* offset: 32 */
        uint32_t DSA_w0_tagCommand:2;               /* offset: 48 */
        uint32_t DSA_w0_srcTagged:1;                /* offset: 50 */
        uint32_t DSA_w0_srcDev:5;                   /* offset: 51 */
        uint32_t DSA_w0_SrcPort_4_0:5;              /* offset: 56 */ /* validity: srcIsTrunk == 0 */
        uint32_t DSA_w0_srcIsTrunk:1;               /* offset: 61 */
        uint32_t DSA_w0_Reserved:1;                 /* offset: 62 */
        uint32_t DSA_w0_cfi:1;                      /* offset: 63 */
        uint32_t DSA_w0_up:3;                       /* offset: 64 */
        uint32_t DSA_w0_Extend:1;                   /* offset: 67 */
        uint32_t DSA_w0_vid:12;                     /* offset: 68 */
        uint32_t DSA_w1_Extend:1;                   /* offset: 80 */
        uint32_t DSA_w1_Reserved1:1;                /* offset: 81 */ /* validity: srcIsTrunk == 0 */
        uint32_t DSA_w1_SrcPort_5:1;                /* offset: 82 */
        uint32_t DSA_w1_EgressFilterRegistered:1;   /* offset: 83 */
        uint32_t DSA_w1_DropOnSource:1;             /* offset: 84 */
        uint32_t DSA_w1_PacketIsLooped:1;           /* offset: 85 */
        uint32_t DSA_w1_routed:1;                   /* offset: 86 */
        uint32_t DSA_w1_Src_ID:5;                   /* offset: 87 */
        uint32_t DSA_w1_QosProfile_6_3:4;           /* offset: 92 */
        uint32_t DSA_w1_QosProfile_2_0:3;           /* offset: 96 */
        uint32_t DSA_w1_use_vidx:1;                 /* offset: 99 */
        uint32_t DSA_w1_Reserved2:1;                /* offset: 100 */
        uint32_t DSA_w1_TrgPort:6;                  /* offset: 101 */
        uint32_t DSA_w1_TrgDev:5;                   /* offset: 107 */
        uint32_t cnm_tpid:16;                       /* offset: 112 */
    } qcn_ha_table; /* 16B */
    struct {
        uint32_t zero_0:32;                         /* offset:128 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t zero_0:32;                         /* offset:160 */
        uint32_t zero_1:32;                         /* offset:192 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                     /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr15_qcn_in_hdr {
    struct {
        uint32_t reserved_0:32;                     /* offset:0 */
        uint32_t reserved_1:32;                     /* offset:32 */
        uint32_t reserved_2:32;                     /* offset:64 */
        uint32_t reserved_3:32;                     /* offset:96 */
        uint32_t reserved_4:32;                     /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;                   /* offset:160 */
        uint32_t mac_da_31_16:16;                   /* offset:176 */
        uint32_t mac_da_15_0:16;                    /* offset:192 */
        uint32_t mac_sa_47_32:16;                   /* offset:208 */
        uint32_t mac_sa_31_16:16;                   /* offset:224 */
        uint32_t mac_sa_15_0:16;                    /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr15_qcn_out_hdr {
    struct {
        uint32_t reserved_0:32;                 /* offset:0 */
        uint32_t reserved_1:32;                 /* offset:32 */
        uint32_t reserved_2:32;                 /* offset:64 */
        uint32_t reserved_3:32;                 /* offset:96 */
        uint32_t reserved_4:32;                 /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;               /* offset:160 */
        uint32_t mac_da_31_16:16;               /* offset:176 */
        uint32_t mac_da_15_0:16;                /* offset:192 */
        uint32_t mac_sa_47_32:16;               /* offset:208 */
        uint32_t mac_sa_31_16:16;               /* offset:224 */
        uint32_t mac_sa_15_0:16;                /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;                  /* offset:256 */
        uint32_t srcTagged:1;                   /* offset:258 */
        uint32_t srcDev:5;                      /* offset:259 */
        uint32_t SrcPort_4_0:5;                 /* offset:264 */
        uint32_t srcIsTrunk:1;                  /* offset:269 */
        uint32_t Reserved:1;                    /* offset:270 */
        uint32_t cfi:1;                         /* offset:271 */
        uint32_t up:3;                          /* offset:272 */
        uint32_t Extend:1;                      /* offset:275 */
        uint32_t vid:12;                        /* offset:276 */
    } QCN_extDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend:1;                      /* offset:288 */
        uint32_t Reserved1:1;                   /* offset:289 */
        uint32_t SrcPort_5:1;                   /* offset:290 */
        uint32_t EgressFilterRegistered:1;      /* offset:291 */
        uint32_t DropOnSource:1;                /* offset:292 */
        uint32_t PacketIsLooped:1;              /* offset:293 */
        uint32_t routed:1;                      /* offset:294 */
        uint32_t Src_ID:5;                      /* offset:295 */
        uint32_t QosProfile:7;                  /* offset:300 */
        uint32_t use_vidx:1;                    /* offset:307 */
        uint32_t Reserved2:1;                   /* offset:308 */
        uint32_t TrgPort:6;                     /* offset:309 */
        uint32_t TrgDev:5;                      /* offset:315 */
    } QCN_extDSA_fwd_w1; /* 4B */
    struct {
        uint32_t EtherType:16;                  /* offset:320 */
        uint32_t Version:4;                     /* offset:336 */
        uint32_t Reserved:6;                    /* offset:340 */
        uint32_t qFb:6;                         /* offset:346 */
        uint32_t CPID_63_32:32;                 /* offset:352 */
        uint32_t CPID_31_16:16;                 /* offset:384 */
        uint32_t CPID_15_3:13;                  /* offset:400 */
        uint32_t CPID_2_0:3;                    /* offset:413 */
        uint32_t Qoff:16;                       /* offset:416 */
        uint32_t Qdelta:16;                     /* offset:432 */
        uint32_t Encapsulated_priority:16;      /* offset:448 */
        uint32_t Encapsulated_MAC_DA_47_32:16;  /* offset:464 */
        uint32_t Encapsulated_MAC_DA_31_16:16;  /* offset:480 */
        uint32_t Encapsulated_MAC_DA_15_0:16;   /* offset:496 */
        uint32_t Encapsulated_SDU_length:8;     /* offset:512 */
    } qcn_tag; /* 25B */
}; /* 65B */


/**************************************************************************
 * Thread THR16_U2E structures
 **************************************************************************/
struct thr16_u2e_cfg {
    struct {
        uint32_t TPID:16;                      /* offset:0 */
        uint32_t up:3;                         /* offset:16 */
        uint32_t cfi:1;                        /* offset:19 */
        uint32_t vid:12;                       /* offset:20 */
    } vlan; /* 4B */
    struct {
        uint32_t Target_ePortFull:14;          /* offset:32 */
        uint32_t Reserved:18;                  /* offset:46 */
    } HA_Table_Target_ePort; /* 4B */
    struct {
        uint32_t reserved_0:32;                /* offset:64 */
        uint32_t reserved_1:32;                /* offset:96 */
    } HA_Table_reserved_space; /* 8B */
    struct {
        uint32_t src_ePort:14;                 /* offset:128 */
        uint32_t reserved:2;                   /* offset:142 */
        uint32_t reserved1:16;                 /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t TargetPort:4;                 /* offset:176 */
        uint32_t reserved_0:9;                 /* offset:180 */
        uint32_t trg_ePort_13_11:3;            /* offset:189 */
        uint32_t trg_ePort_10_0:11;            /* offset:192 */
        uint32_t reserved_1:1;                 /* offset:203 */
        uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t Src_ePortFull:32;             /* offset:224 */
    } cfgReservedSpaceWithSrc_ePort; /* 4B */
}; /* 32B */

struct thr16_u2e_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TagCommand:2;                 /* offset:256 */
        uint32_t Tag0SrcTagged:1;              /* offset:258 */
        uint32_t SrcDev_4_0:5;                 /* offset:259 */
        uint32_t Src_ePort_4_0:5;              /* offset:264 */
        uint32_t SrcIsTrunk:1;                 /* offset:269 */
        uint32_t Reserved0:1;                  /* offset:270 */
        uint32_t CFI:1;                        /* offset:271 */
        uint32_t UP:3;                         /* offset:272 */
        uint32_t Extend0:1;                    /* offset:275 */
        uint32_t eVLAN_11_0:12;                /* offset:276 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend1:1;                    /* offset:288 */
        uint32_t Src_ePort_6_5:2;              /* offset:289 */
        uint32_t EgressFilterRegistered:1;     /* offset:291 */
        uint32_t DropOnSource:1;               /* offset:292 */
        uint32_t PacketIsLooped:1;             /* offset:293 */
        uint32_t Routed:1;                     /* offset:294 */
        uint32_t SrcID_4_0:5;                  /* offset:295 */
        uint32_t GlobalQoS_Profile:7;          /* offset:300 */
        uint32_t use_eVIDX:1;                  /* offset:307 */
        uint32_t TrgPhyPort_6_0:7;             /* offset:308 */
        uint32_t TrgDev_4_0:5;                 /* offset:315 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t Extend2:1;                    /* offset:320 */
        uint32_t Reserved1:1;                  /* offset:321 */
        uint32_t IsTrgPhyPortValid:1;          /* offset:322 */
        uint32_t TrgPhyPort_7:1;               /* offset:323 */
        uint32_t SrcID_11_5:7;                 /* offset:324 */
        uint32_t SrcDev_11_5:7;                /* offset:331 */
        uint32_t Reserved2:1;                  /* offset:338 */
        uint32_t Src_ePort_16_7:10;            /* offset:339 */
        uint32_t TPID_index:3;                 /* offset:349 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t Extend3:1;                    /* offset:352 */
        uint32_t eVLAN_15_12:4;                /* offset:353 */
        uint32_t Tag1SrcTagged:1;              /* offset:357 */
        uint32_t SrcTag0IsOuterTag:1;          /* offset:358 */
        uint32_t Reserved3:1;                  /* offset:359 */
        uint32_t Trg_ePort:17;                 /* offset:360 */
        uint32_t TrgDev:7;                     /* offset:377 */
    } eDSA_fwd_w3; /* 4B */
}; /* 48B */

struct thr16_u2e_with_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
        uint32_t reserved_7:32;                /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:256 */
        uint32_t mac_da_31_16:16;              /* offset:272 */
        uint32_t mac_da_15_0:16;               /* offset:288 */
        uint32_t mac_sa_47_32:16;              /* offset:304 */
        uint32_t mac_sa_31_16:16;              /* offset:320 */
        uint32_t mac_sa_15_0:16;               /* offset:336 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:352 */
        uint32_t up:3;                         /* offset:368 */
        uint32_t cfi:1;                        /* offset:371 */
        uint32_t vid:12;                       /* offset:372 */
    } vlan; /* 4B */
}; /* 48B */

struct thr16_u2e_without_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
        uint32_t reserved_7:32;                /* offset:224 */
        uint32_t reserved_8:32;                /* offset:256 */
    } expansion_space; /* 36B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:288 */
        uint32_t mac_da_31_16:16;              /* offset:304 */
        uint32_t mac_da_15_0:16;               /* offset:320 */
        uint32_t mac_sa_47_32:16;              /* offset:336 */
        uint32_t mac_sa_31_16:16;              /* offset:352 */
        uint32_t mac_sa_15_0:16;               /* offset:368 */
    } mac_header; /* 12B */
}; /* 48B */


/**************************************************************************
 * Thread THR17_U2IPL structures
 **************************************************************************/
struct thr17_u2ipl_cfg {
    struct {
        uint32_t reserved_0:32;                 /* offset:0 */
        uint32_t reserved_1:32;                 /* offset:32 */
        uint32_t reserved_2:32;                 /* offset:64 */
        uint32_t reserved_3:32;                 /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PCID:12;                       /* offset:128 */
        uint32_t Source_Filtering_Bitvector:16; /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                       /* offset:160 */
        uint32_t PVID:12;                       /* offset:176 */
        uint32_t VlanEgrTagState:1;             /* offset:188 */
        uint32_t reserved:3;                    /* offset:189 */
        uint32_t PCID:12;                       /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;          /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                 /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */
struct thr17_u2ipl_in_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t E_PCP:3;                   /* offset:272 */
        uint32_t E_DEI:1;                   /* offset:275 */
        uint32_t Ingress_E_CID_base:12;     /* offset:276 */
        uint32_t IPL_Direction:1;           /* offset:288 */
        uint32_t Reserved:1;                /* offset:289 */
        uint32_t GRP:2;                     /* offset:290 */
        uint32_t E_CID_base:12;             /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:304 */
        uint32_t E_CID_ext:8;               /* offset:312 */
    } etag; /* 8B */
}; /* 40B */
struct thr17_u2ipl_out_hdr {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
        uint32_t reserved_4:32;             /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;           /* offset:160 */
        uint32_t mac_da_31_16:16;           /* offset:176 */
        uint32_t mac_da_15_0:16;            /* offset:192 */
        uint32_t mac_sa_47_32:16;           /* offset:208 */
        uint32_t mac_sa_31_16:16;           /* offset:224 */
        uint32_t mac_sa_15_0:16;            /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                   /* offset:256 */
        uint32_t E_PCP:3;                   /* offset:272 */
        uint32_t E_DEI:1;                   /* offset:275 */
        uint32_t Ingress_E_CID_base:12;     /* offset:276 */
        uint32_t IPL_Direction:1;           /* offset:288 */
        uint32_t Reserved:1;                /* offset:289 */
        uint32_t GRP:2;                     /* offset:290 */
        uint32_t E_CID_base:12;             /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;       /* offset:304 */
        uint32_t E_CID_ext:8;               /* offset:312 */
    } etag; /* 8B */
}; /* 40B */
/**************************************************************************
 * Thread THR18_IPL2IPL structures
 **************************************************************************/
struct thr18_ipl2ipl_cfg {
    struct {
        uint32_t reserved_0:32;                  /* offset:0 */
        uint32_t reserved_1:32;                  /* offset:32 */
        uint32_t reserved_2:32;                  /* offset:64 */
        uint32_t reserved_3:32;                  /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PCID:12;                        /* offset:128 */
        uint32_t Source_Filtering_Bitvector:16;  /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                        /* offset:160 */
        uint32_t TargetPort:4;                   /* offset:176 */
        uint32_t reserved:12;                    /* offset:180 */
        uint32_t PCID:12;                        /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;           /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                  /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */
struct thr18_ipl2ipl_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */
struct thr18_ipl2ipl_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */
/**************************************************************************
 * Thread THR19_E2U_Untagged structures
 **************************************************************************/
struct thr19_e2u_untagged_cfg {
    struct {
        uint32_t E_PCP:3;                      /* offset:0 */
        uint32_t E_DEI:1;                      /* offset:3 */
        uint32_t Ingress_E_CID_base:12;        /* offset:4 */
        uint32_t IPL_Direction:1;              /* offset:16 */
        uint32_t Reserved:1;                   /* offset:17 */
        uint32_t GRP:2;                        /* offset:18 */
        uint32_t E_CID_base:12;                /* offset:20 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:32 */
        uint32_t E_CID_ext:8;                  /* offset:40 */
        uint32_t VLAN_Tag_TPID:16;             /* offset:48 */
    } HA_Table_Upstream_Ports; /* 8B */
    struct {
        uint32_t reserved_0:32;                /* offset:64 */
        uint32_t reserved_1:32;                /* offset:96 */
    } HA_Table_reserved_space; /* 8B */
    struct {
        uint32_t PCID:12;                      /* offset:128 */
        uint32_t Default_VLAN_Tag:16;          /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t PVID:12;                      /* offset:176 */
        uint32_t VlanEgrTagState:1;            /* offset:188 */
        uint32_t reserved:3;                   /* offset:189 */
        uint32_t PCID:12;                      /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */
struct thr19_e2u_untagged_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */
struct thr19_e2u_untagged_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
    } expansion_space; /* 8B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:64 */
        uint32_t mac_da_31_16:16;              /* offset:80 */
        uint32_t mac_da_15_0:16;               /* offset:96 */
        uint32_t mac_sa_47_32:16;              /* offset:112 */
        uint32_t mac_sa_31_16:16;              /* offset:128 */
        uint32_t mac_sa_15_0:16;               /* offset:144 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t E_PCP:3;                      /* offset:176 */
        uint32_t E_DEI:1;                      /* offset:179 */
        uint32_t Ingress_E_CID_base:12;        /* offset:180 */
        uint32_t IPL_Direction:1;              /* offset:192 */
        uint32_t Reserved:1;                   /* offset:193 */
        uint32_t GRP:2;                        /* offset:194 */
        uint32_t E_CID_base:12;                /* offset:196 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:208 */
        uint32_t E_CID_ext:8;                  /* offset:216 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                      /* offset:224 */
        uint32_t up:3;                         /* offset:240 */
        uint32_t cfi:1;                        /* offset:243 */
        uint32_t vid:12;                       /* offset:244 */
    } vlan; /* 4B */
}; /* 32B */
/**************************************************************************
 * Thread THR20_U2E_M4 structures
 **************************************************************************/
struct thr20_u2e_m4_cfg {
    struct {
        uint32_t ExtPort_PCID_1:12;            /* offset:0 */
        uint32_t ExtPort_PCID_2:12;            /* offset:12 */
        uint32_t reserved_0:8;                 /* offset:24 */
        uint32_t ExtPort_PCID_3:12;            /* offset:32 */
        uint32_t ExtPort_PCID_4:12;            /* offset:44 */
        uint32_t reserved_1:8;                 /* offset:56 */
        uint32_t ExtPort_PCID_5:12;            /* offset:64 */
        uint32_t ExtPort_PCID_6:12;            /* offset:76 */
        uint32_t reserved_2:8;                 /* offset:88 */
        uint32_t ExtPort_PCID_7:12;            /* offset:96 */
        uint32_t reserved_3:20;                /* offset:108 */
    } HA_Table_Extended_Ports; /* 16B */
    struct {
        uint32_t PCID:12;                      /* offset:128 */
        uint32_t Default_VLAN_Tag:16;          /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t TargetPort:4;                 /* offset:176 */
        uint32_t reserved:12;                  /* offset:180 */
        uint32_t PCID:12;                      /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr20_u2e_m4_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:256 */
        uint32_t E_PCP:3;                      /* offset:272 */
        uint32_t E_DEI:1;                      /* offset:275 */
        uint32_t Ingress_E_CID_base:12;        /* offset:276 */
        uint32_t IPL_Direction:1;              /* offset:288 */
        uint32_t Reserved:1;                   /* offset:289 */
        uint32_t GRP:2;                        /* offset:290 */
        uint32_t E_CID_base:12;                /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:304 */
        uint32_t E_CID_ext:8;                  /* offset:312 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                      /* offset:320 */
        uint32_t up:3;                         /* offset:336 */
        uint32_t cfi:1;                        /* offset:339 */
        uint32_t vid:12;                       /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr20_u2e_m4_with_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:224 */
        uint32_t mac_da_31_16:16;              /* offset:240 */
        uint32_t mac_da_15_0:16;               /* offset:256 */
        uint32_t mac_sa_47_32:16;              /* offset:272 */
        uint32_t mac_sa_31_16:16;              /* offset:288 */
        uint32_t mac_sa_15_0:16;               /* offset:304 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:320 */
        uint32_t up:3;                         /* offset:336 */
        uint32_t cfi:1;                        /* offset:339 */
        uint32_t vid:12;                       /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr20_u2e_m4_without_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
        uint32_t reserved_7:32;                /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:256 */
        uint32_t mac_da_31_16:16;              /* offset:272 */
        uint32_t mac_da_15_0:16;               /* offset:288 */
        uint32_t mac_sa_47_32:16;              /* offset:304 */
        uint32_t mac_sa_31_16:16;              /* offset:320 */
        uint32_t mac_sa_15_0:16;               /* offset:336 */
    } mac_header; /* 12B */
}; /* 44B */


/**************************************************************************
 * Thread THR21_U2E_M8 structures
 **************************************************************************/
struct thr21_u2e_m8_cfg {
    struct {
        uint32_t ExtPort_PCID_1:12;            /* offset:0 */
        uint32_t ExtPort_PCID_2:12;            /* offset:12 */
        uint32_t reserved_0:8;                 /* offset:24 */
        uint32_t ExtPort_PCID_3:12;            /* offset:32 */
        uint32_t ExtPort_PCID_4:12;            /* offset:44 */
        uint32_t reserved_1:8;                 /* offset:56 */
        uint32_t ExtPort_PCID_5:12;            /* offset:64 */
        uint32_t ExtPort_PCID_6:12;            /* offset:76 */
        uint32_t reserved_2:8;                 /* offset:88 */
        uint32_t ExtPort_PCID_7:12;            /* offset:96 */
        uint32_t reserved_3:20;                /* offset:108 */
    } HA_Table_Extended_Ports; /* 16B */
    struct {
        uint32_t PCID:12;                      /* offset:128 */
        uint32_t Default_VLAN_Tag:16;          /* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t TargetPort:4;                 /* offset:176 */
        uint32_t reserved:12;                  /* offset:180 */
        uint32_t PCID:12;                      /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr21_u2e_m8_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:256 */
        uint32_t E_PCP:3;                      /* offset:272 */
        uint32_t E_DEI:1;                      /* offset:275 */
        uint32_t Ingress_E_CID_base:12;        /* offset:276 */
        uint32_t IPL_Direction:1;              /* offset:288 */
        uint32_t Reserved:1;                   /* offset:289 */
        uint32_t GRP:2;                        /* offset:290 */
        uint32_t E_CID_base:12;                /* offset:292 */
        uint32_t Ingress_E_CID_ext:8;          /* offset:304 */
        uint32_t E_CID_ext:8;                  /* offset:312 */
    } etag; /* 8B */
    struct {
        uint32_t TPID:16;                      /* offset:320 */
        uint32_t up:3;                         /* offset:336 */
        uint32_t cfi:1;                        /* offset:339 */
        uint32_t vid:12;                       /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr21_u2e_m8_with_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:224 */
        uint32_t mac_da_31_16:16;              /* offset:240 */
        uint32_t mac_da_15_0:16;               /* offset:256 */
        uint32_t mac_sa_47_32:16;              /* offset:272 */
        uint32_t mac_sa_31_16:16;              /* offset:288 */
        uint32_t mac_sa_15_0:16;               /* offset:304 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                      /* offset:320 */
        uint32_t up:3;                         /* offset:336 */
        uint32_t cfi:1;                        /* offset:339 */
        uint32_t vid:12;                       /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr21_u2e_m8_without_vlan_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
        uint32_t reserved_5:32;                /* offset:160 */
        uint32_t reserved_6:32;                /* offset:192 */
        uint32_t reserved_7:32;                /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:256 */
        uint32_t mac_da_31_16:16;              /* offset:272 */
        uint32_t mac_da_15_0:16;               /* offset:288 */
        uint32_t mac_sa_47_32:16;              /* offset:304 */
        uint32_t mac_sa_31_16:16;              /* offset:320 */
        uint32_t mac_sa_15_0:16;               /* offset:336 */
    } mac_header; /* 12B */
}; /* 44B */


/**************************************************************************
 * Thread THR22_Discard_pkt structures
 **************************************************************************/
struct thr22_Discard_cfg {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PCID:12;                      /* offset:128 */
        uint32_t Source_Filtering_Bitvector:16;/* offset:140 */
        uint32_t Uplink_Port:1;                /* offset:156 */
        uint32_t reserved:3;                   /* offset:157 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                      /* offset:160 */
        uint32_t PVID:12;                      /* offset:176 */
        uint32_t VlanEgrTagState:1;            /* offset:188 */
        uint32_t reserved:3;                   /* offset:189 */
        uint32_t PCID:12;                      /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr22_Discard_pkt_in_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr22_Discard_out_hdr {
    struct {
        uint32_t reserved_0:32;                /* offset:0 */
        uint32_t reserved_1:32;                /* offset:32 */
        uint32_t reserved_2:32;                /* offset:64 */
        uint32_t reserved_3:32;                /* offset:96 */
        uint32_t reserved_4:32;                /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;              /* offset:160 */
        uint32_t mac_da_31_16:16;              /* offset:176 */
        uint32_t mac_da_15_0:16;               /* offset:192 */
        uint32_t mac_sa_47_32:16;              /* offset:208 */
        uint32_t mac_sa_31_16:16;              /* offset:224 */
        uint32_t mac_sa_15_0:16;               /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */


/**************************************************************************
 * Thread THR23_EVB_E2U structures
 **************************************************************************/

struct thr23_evb_e2u_cfg {
	struct {
		uint32_t TPID:16;                      /* offset:0 */
		uint32_t up:3;                         /* offset:16 */
		uint32_t cfi:1;                        /* offset:19 */
		uint32_t vid:12;                       /* offset:20 */
	} vlan; /* 4B */
	struct {
		uint32_t reserved_0:32;                /* offset:32 */
		uint32_t reserved_1:32;                /* offset:64 */
		uint32_t reserved_2:32;                /* offset:96 */
	} HA_Table_reserved_space; /* 12B */
	struct {
		uint32_t PVID:12;                      /* offset:128 */
		uint32_t src_port_num:4;               /* offset:140 */
		uint32_t reserved:16;                  /* offset:144 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr23_evb_e2u_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
}; /* 32B */


struct thr23_evb_e2u_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
	} expansion_space; /* 16B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:128 */
		uint32_t mac_da_31_16:16;              /* offset:144 */
		uint32_t mac_da_15_0:16;               /* offset:160 */
		uint32_t mac_sa_47_32:16;              /* offset:176 */
		uint32_t mac_sa_31_16:16;              /* offset:192 */
		uint32_t mac_sa_15_0:16;               /* offset:208 */
	} mac_header; /* 12B */
	struct {
		uint32_t TPID:16;                      /* offset:224 */
		uint32_t up:3;                         /* offset:240 */
		uint32_t cfi:1;                        /* offset:243 */
		uint32_t vid:12;                       /* offset:244 */
	} vlan; /* 4B */
}; /* 32B */


/**************************************************************************
 * Thread THR24_EVB_U2E structures
 **************************************************************************/

struct thr24_evb_u2e_cfg {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
	} HA_Table_reserved_space; /* 16B */
	struct {
		uint32_t PVID:12;                      /* offset:128 */
		uint32_t src_port_num:4;               /* offset:140 */
		uint32_t reserved:16;                  /* offset:144 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr24_evb_u2e_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
	struct {
		uint32_t TPID:16;                      /* offset:256 */
		uint32_t up:3;                         /* offset:272 */
		uint32_t cfi:1;                        /* offset:275 */
		uint32_t vid:12;                       /* offset:276 */
	} vlan; /* 4B */
}; /* 36B */


struct thr24_evb_u2e_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
		uint32_t reserved_5:32;                /* offset:160 */
	} expansion_space; /* 24B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:192 */
		uint32_t mac_da_31_16:16;              /* offset:208 */
		uint32_t mac_da_15_0:16;               /* offset:224 */
		uint32_t mac_sa_47_32:16;              /* offset:240 */
		uint32_t mac_sa_31_16:16;              /* offset:256 */
		uint32_t mac_sa_15_0:16;               /* offset:272 */
	} mac_header; /* 12B */
}; /* 36B */


/**************************************************************************
 * Thread THR25_EVB_QCN structures
 **************************************************************************/

struct thr25_evb_qcn_cfg {
	struct {
		uint32_t sa_mac_47_32:16;              /* offset:0 */
		uint32_t sa_mac_31_16:16;              /* offset:16 */
		uint32_t sa_mac_15_0:16;               /* offset:32 */
		uint32_t EtherType:16;                 /* offset:48 */
	} evb_qcn_ha_table; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:64 */
		uint32_t reserved_1:32;                /* offset:96 */
	} HA_Table_reserved_space; /* 8B */
	struct {
		uint32_t PVID:12;                      /* offset:128 */
		uint32_t src_port_num:4;               /* offset:140 */
		uint32_t reserved:16;                  /* offset:144 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr25_evb_qcn_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
}; /* 32B */


struct thr25_evb_qcn_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
	struct {
		uint32_t TPID:16;                      /* offset:256 */
		uint32_t up:3;                         /* offset:272 */
		uint32_t cfi:1;                        /* offset:275 */
		uint32_t vid:12;                       /* offset:276 */
	} vlan; /* 4B */
	struct {
		uint32_t EtherType:16;                 /* offset:288 */
		uint32_t Version:4;                    /* offset:304 */
		uint32_t Reserved:6;                   /* offset:308 */
		uint32_t qFb:6;                        /* offset:314 */
		uint32_t CPID_63_32:32;                /* offset:320 */
		uint32_t CPID_31_16:16;                /* offset:352 */
		uint32_t CPID_15_3:13;                 /* offset:368 */
		uint32_t CPID_2_0:3;                   /* offset:381 */
		uint32_t Qoff:16;                      /* offset:384 */
		uint32_t Qdelta:16;                    /* offset:400 */
		uint32_t Encapsulated_priority:16;     /* offset:416 */
		uint32_t Encapsulated_MAC_DA_47_32:16; /* offset:432 */
		uint32_t Encapsulated_MAC_DA_31_16:16; /* offset:448 */
		uint32_t Encapsulated_MAC_DA_15_0:16;  /* offset:464 */
		uint32_t Encapsulated_SDU_length:8;    /* offset:480 */
	} qcn_tag; /* 25B */
}; /* 61B */


/**************************************************************************
 * Thread THR26_PRE_DA_U2E structures
 **************************************************************************/

struct thr26_pre_da_u2e_cfg {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:24;                /* offset:64 */
		uint32_t ip_protocol:8;                /* offset:88 */
		uint32_t udp_dest_port:16;             /* offset:96 */
		uint32_t domain_number:8;              /* offset:112 */
		uint32_t ptp_version:4;                /* offset:120 */
		uint32_t message_type:4;               /* offset:124 */
	} HA_Table_ptp_classification_fields; /* 16B */
	struct {
		uint32_t reserved_0:8;                 /* offset:128 */
		uint32_t pre_da_port_info:8;           /* offset:136 */
		uint32_t reserved_1:13;                /* offset:144 */
		uint32_t ptpOverMplsEn:1;              /* offset:157 */
		uint32_t ptp_mode:2;                   /* offset:158 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr26_pre_da_u2e_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t reserved:8;                   /* offset:160 */
		uint32_t port_info:8;                  /* offset:168 */
	} pre_da_tag; /* 2B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:176 */
		uint32_t mac_da_31_16:16;              /* offset:192 */
		uint32_t mac_da_15_0:16;               /* offset:208 */
		uint32_t mac_sa_47_32:16;              /* offset:224 */
		uint32_t mac_sa_31_16:16;              /* offset:240 */
		uint32_t mac_sa_15_0:16;               /* offset:256 */
	} mac_header; /* 12B */
}; /* 34B */


struct thr26_pre_da_u2e_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
		uint32_t reserved_5:16;                /* offset:160 */
	} expansion_space; /* 22B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:176 */
		uint32_t mac_da_31_16:16;              /* offset:192 */
		uint32_t mac_da_15_0:16;               /* offset:208 */
		uint32_t mac_sa_47_32:16;              /* offset:224 */
		uint32_t mac_sa_31_16:16;              /* offset:240 */
		uint32_t mac_sa_15_0:16;               /* offset:256 */
	} mac_header; /* 12B */
}; /* 34B */


/**************************************************************************
 * Thread THR27_PRE_DA_E2U structures
 **************************************************************************/

struct thr27_pre_da_e2u_cfg {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:24;                /* offset:64 */
		uint32_t ip_protocol:8;                /* offset:88 */
		uint32_t udp_dest_port:16;             /* offset:96 */
		uint32_t domain_number:8;              /* offset:112 */
		uint32_t ptp_version:4;                /* offset:120 */
		uint32_t message_type:4;               /* offset:124 */
	} HA_Table_ptp_classification_fields; /* 16B */
	struct {
		uint32_t reserved_0:8;                 /* offset:128 */
		uint32_t pre_da_port_info:8;           /* offset:136 */
		uint32_t reserved_1:13;                /* offset:144 */
		uint32_t ptpOverMplsEn:1;              /* offset:157 */
		uint32_t ptp_mode:2;                   /* offset:158 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */


struct thr27_pre_da_e2u_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
}; /* 32B */


struct thr27_pre_da_e2u_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:16;                /* offset:128 */
	} expansion_space; /* 18B */
	struct {
		uint32_t reserved:8;                   /* offset:144 */
		uint32_t port_info:8;                  /* offset:152 */
	} pre_da_tag; /* 2B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
}; /* 32B */


/**************************************************************************
 * Thread THR45_VariableCyclesLengthWithAcclCmd structures
 **************************************************************************/
struct thr45_VariableCyclesLengthWithAcclCmd_cfg {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                    /* offset:128 */
        uint32_t src_port_num:4;             /* offset:140 */
        uint32_t reserved:16;                /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t reserved:3;                 /* offset:189 */
        uint32_t PCID:12;                    /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr45_VariableCyclesLengthWithAcclCmd_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr45_VariableCyclesLengthWithAcclCmd_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */


/**************************************************************************
 * Thread THR46_RemoveAddBytes structures
 **************************************************************************/
struct thr46_RemoveAddBytes_cfg {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
	} HA_Table_reserved_space; /* 16B */
	struct {
		uint32_t PVID:12;                      /* offset:128 */
		uint32_t src_port_num:4;               /* offset:140 */
		uint32_t reserved:16;                  /* offset:144 */
	} srcPortEntry; /* 4B */
	struct {
		uint32_t TPID:16;                      /* offset:160 */
		uint32_t PVID:12;                      /* offset:176 */
		uint32_t VlanEgrTagState:1;            /* offset:188 */
		uint32_t reserved:3;                   /* offset:189 */
		uint32_t PCID:12;                      /* offset:192 */
		uint32_t Egress_Pipe_Delay:20;         /* offset:204 */
	} targetPortEntry; /* 8B */
	struct {
		uint32_t reserved_0:32;                /* offset:224 */
	} cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr46_RemoveAddBytes_in_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
	struct {
		uint32_t reserved_0:32;                /* offset:256 */
		uint32_t reserved_1:32;                /* offset:288 */
		uint32_t reserved_2:32;                /* offset:320 */
		uint32_t reserved_3:32;                /* offset:352 */
		uint32_t reserved_4:32;                /* offset:384 */
		uint32_t reserved_5:32;                /* offset:416 */
		uint32_t reserved_6:32;                /* offset:448 */
		uint32_t reserved_7:32;                /* offset:480 */
		uint32_t reserved_8:32;                /* offset:512 */
		uint32_t reserved_9:32;                /* offset:544 */
		uint32_t reserved_10:32;               /* offset:576 */
	} after_mac_hdr_expansion_space; /* 44B */
}; /* 76B */

struct thr46_RemoveAddBytes_out_hdr {
	struct {
		uint32_t reserved_0:32;                /* offset:0 */
		uint32_t reserved_1:32;                /* offset:32 */
		uint32_t reserved_2:32;                /* offset:64 */
		uint32_t reserved_3:32;                /* offset:96 */
		uint32_t reserved_4:32;                /* offset:128 */
	} expansion_space; /* 20B */
	struct {
		uint32_t mac_da_47_32:16;              /* offset:160 */
		uint32_t mac_da_31_16:16;              /* offset:176 */
		uint32_t mac_da_15_0:16;               /* offset:192 */
		uint32_t mac_sa_47_32:16;              /* offset:208 */
		uint32_t mac_sa_31_16:16;              /* offset:224 */
		uint32_t mac_sa_15_0:16;               /* offset:240 */
	} mac_header; /* 12B */
}; /* 32B */


/**************************************************************************
 * Thread 47 (Add20Bytes) structures
 * ************************************************************************/
struct thr47_Add20Bytes_cfg {
    struct {
        uint32_t reserved_0:32;                 /* offset:0 */
        uint32_t reserved_1:32;                 /* offset:32 */
        uint32_t reserved_2:32;                 /* offset:64 */
        uint32_t reserved_3:32;                 /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                       /* offset:128 */
        uint32_t src_port_num:4;                /* offset:140 */
        uint32_t reserved:16;                   /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                       /* offset:160 */
        uint32_t PVID:12;                       /* offset:176 */
        uint32_t VlanEgrTagState:1;             /* offset:188 */
        uint32_t reserved:3;                    /* offset:189 */
        uint32_t PCID:12;                       /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;          /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                 /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr47_Add20Bytes_in_hdr {
    struct {
        uint32_t reserved_0:32;                 /* offset:0 */
        uint32_t reserved_1:32;                 /* offset:32 */
        uint32_t reserved_2:32;                 /* offset:64 */
        uint32_t reserved_3:32;                 /* offset:96 */
        uint32_t reserved_4:32;                 /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;               /* offset:160 */
        uint32_t mac_da_31_16:16;               /* offset:176 */
        uint32_t mac_da_15_0:16;                /* offset:192 */
        uint32_t mac_sa_47_32:16;               /* offset:208 */
        uint32_t mac_sa_31_16:16;               /* offset:224 */
        uint32_t mac_sa_15_0:16;                /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t TPID:16;                       /* offset:256 */
        uint32_t up:3;                          /* offset:272 */
        uint32_t cfi:1;                         /* offset:275 */
        uint32_t vid:12;                        /* offset:276 */
    } vlan; /* 4B */
}; /* 36B */

struct thr47_Add20Bytes_out_hdr {
    struct {
        uint32_t mac_da_47_32:16;               /* offset:0 */
        uint32_t mac_da_31_16:16;               /* offset:16 */
        uint32_t mac_da_15_0:16;                /* offset:32 */
        uint32_t mac_sa_47_32:16;               /* offset:48 */
        uint32_t mac_sa_31_16:16;               /* offset:64 */
        uint32_t mac_sa_15_0:16;                /* offset:80 */
    } mac_header; /* 12B */
    struct {
        uint32_t reserved_0:32;                 /* offset:96 */
        uint32_t reserved_1:32;                 /* offset:128 */
        uint32_t reserved_2:32;                 /* offset:160 */
        uint32_t reserved_3:32;                 /* offset:192 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                       /* offset:224 */
        uint32_t src_port_num:4;                /* offset:236 */
        uint32_t reserved:16;                   /* offset:240 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                       /* offset:256 */
        uint32_t up:3;                          /* offset:272 */
        uint32_t cfi:1;                         /* offset:275 */
        uint32_t vid:12;                        /* offset:276 */
    } vlan; /* 4B */
}; /* 36B */


/**************************************************************************
 * Thread 48 (Remove20Bytes) structures
 * ************************************************************************/
struct thr48_Remove20Bytes_cfg {
    struct {
        uint32_t reserved_0:32;                 /* offset:0 */
        uint32_t reserved_1:32;                 /* offset:32 */
        uint32_t reserved_2:32;                 /* offset:64 */
        uint32_t reserved_3:32;                 /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                       /* offset:128 */
        uint32_t src_port_num:4;                /* offset:140 */
        uint32_t reserved:16;                   /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                       /* offset:160 */
        uint32_t PVID:12;                       /* offset:176 */
        uint32_t VlanEgrTagState:1;             /* offset:188 */
        uint32_t reserved:3;                    /* offset:189 */
        uint32_t PCID:12;                       /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;          /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;                 /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr48_Remove20Bytes_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
    struct {
        uint32_t tagCommand:2;               /* offset:256 */
        uint32_t srcTagged:1;                /* offset:258 */
        uint32_t srcDev:5;                   /* offset:259 */
        uint32_t SrcPort_4_0:5;              /* offset:264 */
        uint32_t srcIsTrunk:1;               /* offset:269 */
        uint32_t core_id_2:1;                /* offset:270 */
        uint32_t cfi:1;                      /* offset:271 */
        uint32_t up:3;                       /* offset:272 */
        uint32_t extended:1;                 /* offset:275 */
        uint32_t vid:12;                     /* offset:276 */
    } extDSA_fwd_w0; /* 4B */
    struct {
        uint32_t Extend:1;                   /* offset:288 */
        uint32_t SrcPort_6_5:2;              /* offset:289 */
        uint32_t EgressFilterRegistered:1;   /* offset:291 */
        uint32_t DropOnSource:1;             /* offset:292 */
        uint32_t PacketIsLooped:1;           /* offset:293 */
        uint32_t routed:1;                   /* offset:294 */
        uint32_t Src_ID:5;                   /* offset:295 */
        uint32_t QosProfile:7;               /* offset:300 */
        uint32_t use_vidx:1;                 /* offset:307 */
        uint32_t Reserved:1;                 /* offset:308 */
        uint32_t TrgPort:6;                  /* offset:309 */
        uint32_t TrgDev:5;                   /* offset:315 */
    } extDSA_fwd_w1; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:320 */
        uint32_t up:3;                       /* offset:336 */
        uint32_t cfi:1;                      /* offset:339 */
        uint32_t vid:12;                     /* offset:340 */
    } vlan; /* 4B */
}; /* 44B */

struct thr48_Remove20Bytes_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
        uint32_t reserved_5:32;              /* offset:160 */
        uint32_t reserved_6:32;              /* offset:192 */
        uint32_t reserved_7:32;              /* offset:224 */
        uint32_t reserved_8:32;              /* offset:256 */
        uint32_t reserved_9:32;              /* offset:288 */
    } expansion_space; /* 40B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:320 */
        uint32_t mac_da_31_16:16;            /* offset:336 */
        uint32_t mac_da_15_0:16;             /* offset:352 */
        uint32_t mac_sa_47_32:16;            /* offset:368 */
        uint32_t mac_sa_31_16:16;            /* offset:384 */
        uint32_t mac_sa_15_0:16;             /* offset:400 */
    } mac_header; /* 12B */
}; /* 52B */


/**************************************************************************
 * Thread 49 (VariableCyclesLength) structures
 * ************************************************************************/
struct thr49_VariableCyclesLength_cfg {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                    /* offset:128 */
        uint32_t src_port_num:4;             /* offset:140 */
        uint32_t reserved:16;                /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                    /* offset:160 */
        uint32_t PVID:12;                    /* offset:176 */
        uint32_t VlanEgrTagState:1;          /* offset:188 */
        uint32_t reserved:3;                 /* offset:189 */
        uint32_t PCID:12;                    /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;       /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;              /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */

struct thr49_VariableCyclesLength_in_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */

struct thr49_VariableCyclesLength_out_hdr {
    struct {
        uint32_t reserved_0:32;              /* offset:0 */
        uint32_t reserved_1:32;              /* offset:32 */
        uint32_t reserved_2:32;              /* offset:64 */
        uint32_t reserved_3:32;              /* offset:96 */
        uint32_t reserved_4:32;              /* offset:128 */
    } expansion_space; /* 20B */
    struct {
        uint32_t mac_da_47_32:16;            /* offset:160 */
        uint32_t mac_da_31_16:16;            /* offset:176 */
        uint32_t mac_da_15_0:16;             /* offset:192 */
        uint32_t mac_sa_47_32:16;            /* offset:208 */
        uint32_t mac_sa_31_16:16;            /* offset:224 */
        uint32_t mac_sa_15_0:16;             /* offset:240 */
    } mac_header; /* 12B */
}; /* 32B */


/**************************************************************************
* Thread PTP_handler structures
* ************************************************************************/

struct thr_default_cfg {
    struct {
        uint32_t reserved_0:32;             /* offset:0 */
        uint32_t reserved_1:32;             /* offset:32 */
        uint32_t reserved_2:32;             /* offset:64 */
        uint32_t reserved_3:32;             /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t PVID:12;                   /* offset:128 */
        uint32_t src_port_num:4;            /* offset:140 */
        uint32_t reserved:16;               /* offset:144 */
    } srcPortEntry; /* 4B */
    struct {
        uint32_t TPID:16;                   /* offset:160 */
        uint32_t PVID:12;                   /* offset:176 */
        uint32_t VlanEgrTagState:1;         /* offset:188 */
        uint32_t reserved:3;                /* offset:189 */
        uint32_t PCID:12;                   /* offset:192 */
        uint32_t Egress_Pipe_Delay:20;      /* offset:204 */
    } targetPortEntry; /* 8B */
    struct {
        uint32_t reserved_0:32;             /* offset:224 */
    } cfgReservedSpace; /* 4B */
}; /* 32B */


/**************************************************************************
 * Function prototypes
 * ************************************************************************/
void call_threads();
void THR0_DoNothing();
void THR1_E2U ();
void THR2_U2E ();
void THR3_U2C ();
void THR4_ET2U();
void THR5_EU2U();
void THR6_U2E ();
void THR7_Mrr2E();
void THR8_E_V2U();
void THR9_E2U();
void THR10_C_V2U();
void THR11_C2U();
void THR12_U_UC2C();
void THR13_U_MC2C ();
void THR14_U_MR2C();
void THR15_QCN () ;
void THR16_U2E();
void THR17_U2IPL();
void THR18_IPL2IPL();
void THR19_E2U_Untagged();
void THR20_U2E_M4();
void THR21_U2E_M8();
void THR22_Discard_pkt();
void THR23_EVB_E2U();
void THR24_EVB_U2E();
void THR25_EVB_QCN();
void THR26_PRE_DA_U2E();
void THR27_PRE_DA_E2U();
void THR45_VariableCyclesLengthWithAcclCmd();
void THR46_RemoveAddBytes();
void THR47_Add20Bytes ();
void THR48_Remove20Bytes ();
void THR49_VariableCyclesLength ();
void PTP_handler (struct nonQcn_desc *desc_ptr, uint32_t enable_record,uint32_t thread_id);
inline void ECN_handler (struct nonQcn_desc *desc_ptr, uint32_t enable_record,uint32_t thread_id);
inline void packet_swap_sequence();
#if RECOVER_FROM_EXCEPTION
INLINE void pipeRecoveFromException();
#endif

