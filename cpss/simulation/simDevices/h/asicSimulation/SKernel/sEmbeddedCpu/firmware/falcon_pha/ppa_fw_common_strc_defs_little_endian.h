/*
 *  ppa_fw_common_strc_defs_little_endian.h
 *
 *  little endian PPA firmware common structures definitions
 *
 */
#ifndef __ppa_fw_common_strc_defs_h
#define __ppa_fw_common_strc_defs_h



/* PPA special register structure */
struct Special_Regs {
    uint32_t reserved_0:16;
    uint32_t checksum_accumulator:16;
    uint32_t reserved_1:32;
    uint32_t reserved_2:32;
    uint32_t reserved_3:32;
    uint32_t reserved_4:32;
    uint32_t reserved_5:32;
    uint32_t reserved_6:32;
    uint32_t reserved_7:32;
}; /* 32B */

/* IPv4 header format structure */
struct IPv4_Header {
    uint16_t total_length:16;              /* offset:16 */
    uint16_t ecn:2;                         /* offset:14 */
    uint16_t dscp:6;                        /* offset:8 */
    uint16_t ihl:4;                         /* offset:4 */
    uint16_t version:4;                     /* offset:0 */
    uint16_t fragment_offset:13;          /* offset:51 */
    uint16_t flag_more_fragment:1;        /* offset:50 */
    uint16_t flag_dont_fragment:1;        /* offset:49 */
    uint16_t flag_reserved:1;              /* offset:48 */
    uint16_t identification:16;            /* offset:32 */
    uint16_t header_checksum:16;          /* offset:80 */
    uint16_t protocol:8;                   /* offset:72 */
    uint16_t ttl:8;                         /* offset:64 */
    uint16_t sip_low:16;                   /* offset:112 */
    uint16_t sip_high:16;                  /* offset:96 */
    uint16_t dip_low:16;                   /* offset:144 */
    uint16_t dip_high:16;                  /* offset:128 */
}; /* 20B */

/* IPv6 header format structure */
struct IPv6_Header {
    uint16_t flow_label_15_0:16;         /* offset:16 */
    uint16_t flow_label_19_16:4;         /* offset:12 */
    uint16_t ecn:2;                        /* offset:10 */
    uint16_t dscp:6;                       /* offset:4 */
    uint16_t version:4;                    /* offset:0 */
    uint16_t hop_limit:8;                /* offset:56 */
    uint16_t next_header:8;              /* offset:48 */
    uint16_t payload_length:16;          /* offset:32 */
    uint16_t sip0_low:16;                /* offset:80 */
    uint16_t sip0_high:16;               /* offset:64 */
    uint16_t sip1_low:16;                /* offset:112 */
    uint16_t sip1_high:16;               /* offset:96 */
    uint16_t sip2_low:16;                /* offset:144 */
    uint16_t sip2_high:16;               /* offset:128 */
    uint16_t sip3_low:16;                /* offset:176 */
    uint16_t sip3_high:16;               /* offset:160 */
    uint16_t dip0_low:16;                /* offset:208 */
    uint16_t dip0_high:16;               /* offset:192 */
    uint16_t dip1_low:16;                /* offset:240 */
    uint16_t dip1_high:16;               /* offset:224 */
    uint16_t dip2_low:16;                /* offset:272 */
    uint16_t dip2_high:16;               /* offset:256 */
    uint16_t dip3_low:16;                /* offset:304 */
    uint16_t dip3_high:16;               /* offset:288 */
}; /* 40B */

/* Segment Routing IPv6 header format structure */
struct SRv6_Header {
    uint16_t segments_left:8;           /* offset:24 */
    uint16_t routing_type:8;            /* offset:16 */
    uint16_t hdr_ext_len:8;             /* offset:8 */
    uint16_t next_header:8;             /* offset:0 */
    uint16_t tag:16;                     /* offset:48 */
    uint16_t flags:8;                    /* offset:40 */
    uint16_t last_entry:8;              /* offset:32 */
}; /* 8B */

/* PPA descriptor format structure */
struct ppa_in_desc {
    struct {
        uint32_t oam_opcode:8;                      /* offset:24 */
        uint32_t egress_header_size:8;             /* offset:16 */
        uint32_t force_new_dsa_tag:1;              /* offset:15 */
        uint32_t oam_rdi:1;                         /* offset:14 */
        uint32_t mark_ecn:1;                        /* offset:13 */
        uint32_t latency_monitoring_en:1;         /* offset:12 */
        uint32_t ip_legal:1;                        /* offset:11 */
        uint32_t egress_timestamp_tagged:3;       /* offset:8 */
        uint32_t two_byte_header_added:1;         /* offset:7 */
        uint32_t l4_valid:1;                        /* offset:6 */
        uint32_t is_trill:1;                        /* offset:5 */
        uint32_t truncated:1;                       /* offset:4 */
        uint32_t egress_outer_is_tagged:1;        /* offset:3 */
        uint32_t reserved_alignment:3;             /* offset:0 */
        uint32_t cfi0:1;                             /* offset:63 */
        uint32_t cfi1:1;                             /* offset:62 */
        uint32_t sst_id:12;                          /* offset:50 */
        uint32_t ptp_packet_format:3;              /* offset:47 */
        uint32_t egress_outer_packet_type:2;      /* offset:45 */
        uint32_t local_dev_src_port:10;            /* offset:35 */
        uint32_t queue_dp:2;                        /* offset:33 */
        uint32_t oam_processing_en:1;              /* offset:32 4*/
        uint32_t local_dev_src_eport:13;           /* offset:83 */
        uint32_t tunnel_terminated:1;              /* offset:82 */
        uint32_t dec_ttl:1;                          /* offset:81 */
        uint32_t tc:3;                               /* offset:78 */
        uint32_t local_dev_trg_phy_port:10;       /* offset:68 */
        uint32_t mac_timestamping_en:1;            /* offset:67 */
        uint32_t egress_tag_state:3;               /* offset:64 8*/
        uint32_t ptp_dispatching_en:1;             /* offset:127 */
        uint32_t ptp_timestamp_queue_entry_id:10; /* offset:117 */
        uint32_t do_nat:1;                           /* offset:116 */
        uint32_t copy_reserved:20;                  /* offset:96 12*/
        uint32_t latency_profile:9;                /* offset:151 */
        uint32_t qos_mapped_up:3;                  /* offset:148 */
        uint32_t egress_inner_packet_type:2;      /* offset:146 */
        uint32_t oam_tx_period:3;                   /* offset:143 */
        uint32_t do_route_ha:1;                     /* offset:142 */
        uint32_t egress_byte_count:14;             /* offset:128 16*/
        uint32_t pha_metadata:32;                  /* offset:160 20*/
        uint32_t packet_hash:12;                   /* offset:212 */
        uint32_t queue_length:20;                  /* offset:192 24*/
        uint32_t ptp_action:4;                     /* offset:252 */
        uint32_t ptp_timestamp_queue_select:1;    /* offset:251 */
        uint32_t ptp_tai_select:1;                 /* offset:250 */
        uint32_t queue_limit:20;                   /* offset:230 */
        uint32_t egress_outer_l3_offset:6;        /* offset:224 28*/
        uint32_t ptp_cf_wraparound_check_en:1;   /* offset:287 */
        uint32_t routed:1;                          /* offset:286 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") */
        uint32_t egress_filter_registered:1;     /* offset:285 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") */
        uint32_t sr_end_node:1;                    /* offset:284 */ /* validity: (QCN Generated By Local Dev = "False") */
        uint32_t dp:2;                              /* offset:282 */
        uint32_t is_trg_phy_port_valid:1;         /* offset:281 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") and (Use VIDX = "0") */
        uint32_t timestamp_tagged:3;              /* offset:278 */
        uint32_t timestamp_offset:8;              /* offset:270 */
        uint32_t udp_checksum_offset:8;           /* offset:262 */
        uint32_t udp_checksum_update_en:1;        /* offset:261 */
        uint32_t timestamp_mask_profile:3;       /* offset:258 */
        uint32_t egress_checksum_mode:1;          /* offset:257 */
        uint32_t ptp_egress_tai_sel:1;            /* offset:256 32*/
        uint32_t l4_offset:7;                      /* offset:416 */ /* validity: (None) */
        uint32_t egress_inner_header_offset:7;   /* offset:416 */ /* validity: (Egress Tunnel Start = "TS") */
        uint32_t ptp_offset:7;                     /* offset:416 */ /* validity: (Timestamp En = "Disable") and (LM Counter Insert En = "Disable") */
        uint32_t egress_inner_l3_offset:6;       /* offset:416 */ /* validity: (Egress Tunnel Start = "TS") */
        uint32_t analyzer_index:3;                /* offset:416 */ /* validity: (Outgoing Mtag Cmd != "TO ANALYZER") */
        uint32_t ptp_trigger_type:2;              /* offset:416 */ /* validity: (L2 Echo = "0") and (Is PTP = "True") */
        uint32_t cpu_code:8;                       /* offset:416 */ /* validity: (Egress Packet Cmd != "Forward") */
        uint32_t egress_dscp:6;                   /* offset:416 */ /* validity: (None) */
        uint32_t flow_id:13;                       /* offset:416 */ /* validity: (None) */
        uint32_t lm_counter_insert_en:1;          /* offset:416 */ /* validity: (None) */
        uint32_t egress_tunnel_start:1;           /* offset:416 */ /* validity: (None) */
        uint32_t qcn_generated_by_local_dev:1;   /* offset:416 */ /* validity: (None) */
        uint32_t l2_echo:1;                        /* offset:416 */ /* validity: (None) */
        uint32_t tables_read_error:1;             /* offset:416 */ /* validity: (None) */
        uint32_t trg_phy_port:10;                 /* offset:416 */ /* validity: (Outgoing Mtag Cmd != "TO CPU") and (Use VIDX = "0") */
        uint32_t trg_dev:10;                      /* offset:416 */ /* validity: (Outgoing Mtag Cmd != "TO CPU") and (Use VIDX = "0") */
        uint32_t ttl:8;                            /* offset:416 */ /* validity: (QCN Generated By Local Dev = "False") */
        uint32_t sr_eh_offset:4;                  /* offset:416 */ /* validity: (QCN Generated By Local Dev = "False") */
        uint32_t rx_sniff:1;                       /* offset:416 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") */
        uint32_t reserved_9_1:9;                  /* offset:416 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") */
        uint32_t orig_src_eport:13;               /* offset:416 */ /* validity: ((Outgoing Mtag Cmd = "FORWARD") */
        uint32_t egress_packet_cmd:3;             /* offset:416 */ /* validity: (None) */
        uint32_t use_vidx:1;                       /* offset:416 */ /* validity: (Outgoing Mtag Cmd != "TO CPU") */
        uint32_t orig_is_trunk:1;                 /* offset:416 */ /* validity: (Outgoing Mtag Cmd = "FORWARD") */
        uint32_t is_ptp:1;                         /* offset:416 */ /* validity: (L2 Echo = "0") */
        uint32_t timestamp_en:1;                  /* offset:416 */ /* validity: (None) */
        uint32_t outgoing_mtag_cmd:2;            /* offset:416 */ /* validity: (None) */
        uint32_t timestamp:32;                     /* offset:416 */ /* validity: (LM Counter Insert En = "Disable") */
    } phal2ppa; /* 56B */
    struct {
        int32_t fw_bc_modification:8;           /* offset:472 */
        uint32_t bypass_buffer_id:8;            /* offset:464 */
        uint32_t accessible_header_length:8;   /* offset:456 */
        uint32_t accessible_header_start:8;    /* offset:448 */
        uint32_t align_to_32b:3;                /* offset:509 */
        uint32_t skip_count:5;                  /* offset:504 */
        uint32_t fw_drop_code:8;                /* offset:496 */
        uint32_t fw_packet_command:3;          /* offset:493 */
        uint32_t reserved_1:1;                  /* offset:492 */
        uint32_t prep_table_read_error:1;      /* offset:491 */
        uint32_t invalid_header_anchor:1;      /* offset:490 */
        uint32_t extension_increase:2;          /* offset:488 */
        uint32_t header_remainder_size:8;      /* offset:480 */
    } ppa_internal_desc; /* 8B */
}; /* 64B */



#endif /*__ppa_fw_common_strc_defs_h*/
