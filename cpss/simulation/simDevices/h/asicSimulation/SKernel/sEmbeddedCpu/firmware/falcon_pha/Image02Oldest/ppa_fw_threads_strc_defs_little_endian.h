/*
 *
 * ppa_fw_threads_strc_defs_little_endian.h
 *
 * PPA fw threads structures definitions
 *
 */
#ifndef __ppa_fw_threads_strc_defs_h
#define __ppa_fw_threads_strc_defs_h


/**************************************************************************
 * Thread THR0_DoNothing structures
 **************************************************************************/

struct thr0_DoNothing_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr0_DoNothing_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:272 */
        uint32_t mac_da_47_32:16;        /* offset:256 */
        uint32_t mac_sa_47_32:16;        /* offset:304 */
        uint32_t mac_da_15_0:16;         /* offset:288 */
        uint32_t mac_sa_15_0:16;         /* offset:336 */
        uint32_t mac_sa_31_16:16;        /* offset:320 */
    } mac_header; /* 12B */
}; /* 44B */


struct thr0_DoNothing_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:272 */
        uint32_t mac_da_47_32:16;        /* offset:256 */
        uint32_t mac_sa_47_32:16;        /* offset:304 */
        uint32_t mac_da_15_0:16;         /* offset:288 */
        uint32_t mac_sa_15_0:16;         /* offset:336 */
        uint32_t mac_sa_31_16:16;        /* offset:320 */
    } mac_header; /* 12B */
}; /* 44B */


/**************************************************************************
 * Thread THR1_SRv6_End_Node structures
 **************************************************************************/

struct thr1_SRv6_End_Node_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr1_SRv6_End_Node_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:600 */
        uint32_t routing_type:8;         /* offset:592 */
        uint32_t hdr_ext_len:8;          /* offset:584 */
        uint32_t next_header:8;          /* offset:576 */
        uint32_t tag:16;                 /* offset:624 */
        uint32_t flags:8;                /* offset:616 */
        uint32_t last_entry:8;           /* offset:608 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:656 */
        uint32_t dip0_high:16;           /* offset:640 */
        uint32_t dip1_low:16;            /* offset:688 */
        uint32_t dip1_high:16;           /* offset:672 */
        uint32_t dip2_low:16;            /* offset:720 */
        uint32_t dip2_high:16;           /* offset:704 */
        uint32_t dip3_low:16;            /* offset:752 */
        uint32_t dip3_high:16;           /* offset:736 */
    } srv6_seg0; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:784 */
        uint32_t dip0_high:16;           /* offset:768 */
        uint32_t dip1_low:16;            /* offset:816 */
        uint32_t dip1_high:16;           /* offset:800 */
        uint32_t dip2_low:16;            /* offset:848 */
        uint32_t dip2_high:16;           /* offset:832 */
        uint32_t dip3_low:16;            /* offset:880 */
        uint32_t dip3_high:16;           /* offset:864 */
    } srv6_seg1; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:912 */
        uint32_t dip0_high:16;           /* offset:896 */
        uint32_t dip1_low:16;            /* offset:944 */
        uint32_t dip1_high:16;           /* offset:928 */
        uint32_t dip2_low:16;            /* offset:976 */
        uint32_t dip2_high:16;           /* offset:960 */
        uint32_t dip3_low:16;            /* offset:1008 */
        uint32_t dip3_high:16;           /* offset:992 */
    } srv6_seg2; /* 16B */
}; /* 128B */


struct thr1_SRv6_End_Node_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:600 */
        uint32_t routing_type:8;         /* offset:592 */
        uint32_t hdr_ext_len:8;          /* offset:584 */
        uint32_t next_header:8;          /* offset:576 */
        uint32_t tag:16;                 /* offset:624 */
        uint32_t flags:8;                /* offset:616 */
        uint32_t last_entry:8;           /* offset:608 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:656 */
        uint32_t dip0_high:16;           /* offset:640 */
        uint32_t dip1_low:16;            /* offset:688 */
        uint32_t dip1_high:16;           /* offset:672 */
        uint32_t dip2_low:16;            /* offset:720 */
        uint32_t dip2_high:16;           /* offset:704 */
        uint32_t dip3_low:16;            /* offset:752 */
        uint32_t dip3_high:16;           /* offset:736 */
    } srv6_seg0; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:784 */
        uint32_t dip0_high:16;           /* offset:768 */
        uint32_t dip1_low:16;            /* offset:816 */
        uint32_t dip1_high:16;           /* offset:800 */
        uint32_t dip2_low:16;            /* offset:848 */
        uint32_t dip2_high:16;           /* offset:832 */
        uint32_t dip3_low:16;            /* offset:880 */
        uint32_t dip3_high:16;           /* offset:864 */
    } srv6_seg1; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:912 */
        uint32_t dip0_high:16;           /* offset:896 */
        uint32_t dip1_low:16;            /* offset:944 */
        uint32_t dip1_high:16;           /* offset:928 */
        uint32_t dip2_low:16;            /* offset:976 */
        uint32_t dip2_high:16;           /* offset:960 */
        uint32_t dip3_low:16;            /* offset:1008 */
        uint32_t dip3_high:16;           /* offset:992 */
    } srv6_seg2; /* 16B */
}; /* 128B */


/**************************************************************************
 * Thread THR2_SRv6_Source_Node_1_segment structures
 **************************************************************************/

struct thr2_SRv6_Source_Node_1_segment_cfg {
    struct {
        uint32_t val_0:32;               /* offset:0 */
        uint32_t val_1:32;               /* offset:32 */
        uint32_t val_2:32;               /* offset:64 */
        uint32_t val_3:32;               /* offset:96 */
    } ipv6_sip_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr2_SRv6_Source_Node_1_segment_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t DestinationPort:16;     /* offset:592 */
        uint32_t SourcePort:16;          /* offset:576 */
        uint32_t Checksum:16;            /* offset:624 */
        uint32_t Length:16;              /* offset:608 */
    } udp_header; /* 8B */
    struct {
        uint32_t NP:8;                   /* offset:664 */
        uint32_t Reserved:16;            /* offset:648 */
        uint32_t O:1;                    /* offset:647 */
        uint32_t B:1;                    /* offset:646 */
        uint32_t P:1;                    /* offset:645 */
        uint32_t I:1;                    /* offset:644 */
        uint32_t Ver:2;                  /* offset:642 */
        uint32_t Res1:1;                 /* offset:641 */
        uint32_t Res0:1;                 /* offset:640 */
        uint32_t Reserved1:8;            /* offset:696 */
        uint32_t VNI:24;                 /* offset:672 */
    } vxlan_header; /* 8B */
}; /* 88B */


struct thr2_SRv6_Source_Node_1_segment_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
    } expansion_space; /* 8B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:80 */
        uint32_t flow_label_19_16:4;     /* offset:76 */
        uint32_t ecn:2;                  /* offset:74 */
        uint32_t dscp:6;                 /* offset:68 */
        uint32_t version:4;              /* offset:64 */
        uint32_t hop_limit:8;            /* offset:120 */
        uint32_t next_header:8;          /* offset:112 */
        uint32_t payload_length:16;      /* offset:96 */
        uint32_t sip0_low:16;            /* offset:144 */
        uint32_t sip0_high:16;           /* offset:128 */
        uint32_t sip1_low:16;            /* offset:176 */
        uint32_t sip1_high:16;           /* offset:160 */
        uint32_t sip2_low:16;            /* offset:208 */
        uint32_t sip2_high:16;           /* offset:192 */
        uint32_t sip3_low:16;            /* offset:240 */
        uint32_t sip3_high:16;           /* offset:224 */
        uint32_t dip0_low:16;            /* offset:272 */
        uint32_t dip0_high:16;           /* offset:256 */
        uint32_t dip1_low:16;            /* offset:304 */
        uint32_t dip1_high:16;           /* offset:288 */
        uint32_t dip2_low:16;            /* offset:336 */
        uint32_t dip2_high:16;           /* offset:320 */
        uint32_t dip3_low:16;            /* offset:368 */
        uint32_t dip3_high:16;           /* offset:352 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:408 */
        uint32_t routing_type:8;         /* offset:400 */
        uint32_t hdr_ext_len:8;          /* offset:392 */
        uint32_t next_header:8;          /* offset:384 */
        uint32_t tag:16;                 /* offset:432 */
        uint32_t flags:8;                /* offset:424 */
        uint32_t last_entry:8;           /* offset:416 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } SRv6_Segment0_Header; /* 16B */
    struct {
        uint32_t DestinationPort:16;     /* offset:592 */
        uint32_t SourcePort:16;          /* offset:576 */
        uint32_t Checksum:16;            /* offset:624 */
        uint32_t Length:16;              /* offset:608 */
    } udp_header; /* 8B */
    struct {
        uint32_t NP:8;                   /* offset:664 */
        uint32_t Reserved:16;            /* offset:648 */
        uint32_t O:1;                    /* offset:647 */
        uint32_t B:1;                    /* offset:646 */
        uint32_t P:1;                    /* offset:645 */
        uint32_t I:1;                    /* offset:644 */
        uint32_t Ver:2;                  /* offset:642 */
        uint32_t Res1:1;                 /* offset:641 */
        uint32_t Res0:1;                 /* offset:640 */
        uint32_t Reserved1:8;            /* offset:696 */
        uint32_t VNI:24;                 /* offset:672 */
    } vxlan_header; /* 8B */
}; /* 88B */


/**************************************************************************
 * Thread THR3_SRv6_Source_Node_First_Pass_2_3_segments structures
 **************************************************************************/

struct thr3_SRv6_Source_Node_First_Pass_2_3_segments_cfg {
    struct {
        uint32_t val_0:32;               /* offset:0 */
        uint32_t val_1:32;               /* offset:32 */
        uint32_t val_2:32;               /* offset:64 */
        uint32_t val_3:32;               /* offset:96 */
    } ipv6_sip_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr3_SRv6_Source_Node_First_Pass_2_3_segments_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:272 */
        uint32_t mac_da_47_32:16;        /* offset:256 */
        uint32_t mac_sa_47_32:16;        /* offset:304 */
        uint32_t mac_da_15_0:16;         /* offset:288 */
        uint32_t mac_sa_15_0:16;         /* offset:336 */
        uint32_t mac_sa_31_16:16;        /* offset:320 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:372 */
        uint32_t Extend0:1;              /* offset:371 */
        uint32_t UP:3;                   /* offset:368 */
        uint32_t CFI:1;                  /* offset:367 */
        uint32_t Hash_0:1;               /* offset:366 */
        uint32_t SrcIsTrunk:1;           /* offset:365 */
        uint32_t Src_ePort_4_0:5;        /* offset:360 */
        uint32_t SrcDev_4_0:5;           /* offset:355 */
        uint32_t Tag0SrcTagged:1;        /* offset:354 */
        uint32_t TagCommand:2;           /* offset:352 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t TrgDev_4_0:5;           /* offset:411 */
        uint32_t TrgPhyPort_6_0:7;       /* offset:404 */
        uint32_t use_eVIDX:1;            /* offset:403 */
        uint32_t GlobalQoS_Profile:7;    /* offset:396 */
        uint32_t SrcID_4_0:5;            /* offset:391 */
        uint32_t Routed:1;               /* offset:390 */
        uint32_t PacketIsLooped:1;       /* offset:389 */
        uint32_t DropOnSource:1;         /* offset:388 */
        uint32_t EgressFilterRegistered:1;/* offset:387 */
        uint32_t Src_ePort_6_5:2;        /* offset:385 */
        uint32_t Extend1:1;              /* offset:384 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:445 */
        uint32_t Src_ePort_15_7:9;       /* offset:436 */
        uint32_t Reserved:1;             /* offset:435 */
        uint32_t Hash_1:1;               /* offset:434 */
        uint32_t SrcDev_9_5:5;           /* offset:429 */
        uint32_t Hash_3_2:2;             /* offset:427 */
        uint32_t SrcID_11_5:7;           /* offset:420 */
        uint32_t TrgPhyPort_7:1;         /* offset:419 */
        uint32_t IsTrgPhyPortValid:1;    /* offset:418 */
        uint32_t SkipFdbSaLookup:1;      /* offset:417 */
        uint32_t Extend2:1;              /* offset:416 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t TrgDev_9_5:5;           /* offset:475 */
        uint32_t Hash_5_4:2;             /* offset:473 */
        uint32_t Trg_ePort_15_0:16;      /* offset:457 */
        uint32_t TrgPhyPort_9_8:2;       /* offset:455 */
        uint32_t SrcTag0IsOuterTag:1;    /* offset:454 */
        uint32_t Tag1SrcTagged:1;        /* offset:453 */
        uint32_t eVLAN_15_12:4;          /* offset:449 */
        uint32_t Extend3:1;              /* offset:448 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:480 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:512 */
        uint32_t flow_label_19_16:4;     /* offset:508 */
        uint32_t ecn:2;                  /* offset:506 */
        uint32_t dscp:6;                 /* offset:500 */
        uint32_t version:4;              /* offset:496 */
        uint32_t hop_limit:8;            /* offset:552 */
        uint32_t next_header:8;          /* offset:544 */
        uint32_t payload_length:16;      /* offset:528 */
        uint32_t sip0_low:16;            /* offset:576 */
        uint32_t sip0_high:16;           /* offset:560 */
        uint32_t sip1_low:16;            /* offset:608 */
        uint32_t sip1_high:16;           /* offset:592 */
        uint32_t sip2_low:16;            /* offset:640 */
        uint32_t sip2_high:16;           /* offset:624 */
        uint32_t sip3_low:16;            /* offset:672 */
        uint32_t sip3_high:16;           /* offset:656 */
        uint32_t dip0_low:16;            /* offset:704 */
        uint32_t dip0_high:16;           /* offset:688 */
        uint32_t dip1_low:16;            /* offset:736 */
        uint32_t dip1_high:16;           /* offset:720 */
        uint32_t dip2_low:16;            /* offset:768 */
        uint32_t dip2_high:16;           /* offset:752 */
        uint32_t dip3_low:16;            /* offset:800 */
        uint32_t dip3_high:16;           /* offset:784 */
    } IPv6_Header; /* 40B */
}; /* 102B */


struct thr3_SRv6_Source_Node_First_Pass_2_3_segments_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } expansion_space; /* 16B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:144 */
        uint32_t mac_da_47_32:16;        /* offset:128 */
        uint32_t mac_sa_47_32:16;        /* offset:176 */
        uint32_t mac_da_15_0:16;         /* offset:160 */
        uint32_t mac_sa_15_0:16;         /* offset:208 */
        uint32_t mac_sa_31_16:16;        /* offset:192 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:244 */
        uint32_t Extend0:1;              /* offset:243 */
        uint32_t UP:3;                   /* offset:240 */
        uint32_t CFI:1;                  /* offset:239 */
        uint32_t Hash_0:1;               /* offset:238 */
        uint32_t SrcIsTrunk:1;           /* offset:237 */
        uint32_t Src_ePort_4_0:5;        /* offset:232 */
        uint32_t SrcDev_4_0:5;           /* offset:227 */
        uint32_t Tag0SrcTagged:1;        /* offset:226 */
        uint32_t TagCommand:2;           /* offset:224 */
    } eDSA_fwd_w0; /* 4B */
    struct {
        uint32_t TrgDev_4_0:5;           /* offset:283 */
        uint32_t TrgPhyPort_6_0:7;       /* offset:276 */
        uint32_t use_eVIDX:1;            /* offset:275 */
        uint32_t GlobalQoS_Profile:7;    /* offset:268 */
        uint32_t SrcID_4_0:5;            /* offset:263 */
        uint32_t Routed:1;               /* offset:262 */
        uint32_t PacketIsLooped:1;       /* offset:261 */
        uint32_t DropOnSource:1;         /* offset:260 */
        uint32_t EgressFilterRegistered:1;/* offset:259 */
        uint32_t Src_ePort_6_5:2;        /* offset:257 */
        uint32_t Extend1:1;              /* offset:256 */
    } eDSA_fwd_w1; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:317 */
        uint32_t Src_ePort_15_7:9;       /* offset:308 */
        uint32_t Reserved:1;             /* offset:307 */
        uint32_t Hash_1:1;               /* offset:306 */
        uint32_t SrcDev_9_5:5;           /* offset:301 */
        uint32_t Hash_3_2:2;             /* offset:299 */
        uint32_t SrcID_11_5:7;           /* offset:292 */
        uint32_t TrgPhyPort_7:1;         /* offset:291 */
        uint32_t IsTrgPhyPortValid:1;    /* offset:290 */
        uint32_t SkipFdbSaLookup:1;      /* offset:289 */
        uint32_t Extend2:1;              /* offset:288 */
    } eDSA_fwd_w2; /* 4B */
    struct {
        uint32_t TrgDev_9_5:5;           /* offset:347 */
        uint32_t Hash_5_4:2;             /* offset:345 */
        uint32_t Trg_ePort_15_0:16;      /* offset:329 */
        uint32_t TrgPhyPort_9_8:2;       /* offset:327 */
        uint32_t SrcTag0IsOuterTag:1;    /* offset:326 */
        uint32_t Tag1SrcTagged:1;        /* offset:325 */
        uint32_t eVLAN_15_12:4;          /* offset:321 */
        uint32_t Extend3:1;              /* offset:320 */
    } eDSA_fwd_w3; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:352 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:384 */
        uint32_t flow_label_19_16:4;     /* offset:380 */
        uint32_t ecn:2;                  /* offset:378 */
        uint32_t dscp:6;                 /* offset:372 */
        uint32_t version:4;              /* offset:368 */
        uint32_t hop_limit:8;            /* offset:424 */
        uint32_t next_header:8;          /* offset:416 */
        uint32_t payload_length:16;      /* offset:400 */
        uint32_t sip0_low:16;            /* offset:448 */
        uint32_t sip0_high:16;           /* offset:432 */
        uint32_t sip1_low:16;            /* offset:480 */
        uint32_t sip1_high:16;           /* offset:464 */
        uint32_t sip2_low:16;            /* offset:512 */
        uint32_t sip2_high:16;           /* offset:496 */
        uint32_t sip3_low:16;            /* offset:544 */
        uint32_t sip3_high:16;           /* offset:528 */
        uint32_t dip0_low:16;            /* offset:576 */
        uint32_t dip0_high:16;           /* offset:560 */
        uint32_t dip1_low:16;            /* offset:608 */
        uint32_t dip1_high:16;           /* offset:592 */
        uint32_t dip2_low:16;            /* offset:640 */
        uint32_t dip2_high:16;           /* offset:624 */
        uint32_t dip3_low:16;            /* offset:672 */
        uint32_t dip3_high:16;           /* offset:656 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t dip0_low:16;            /* offset:704 */
        uint32_t dip0_high:16;           /* offset:688 */
        uint32_t dip1_low:16;            /* offset:736 */
        uint32_t dip1_high:16;           /* offset:720 */
        uint32_t dip2_low:16;            /* offset:768 */
        uint32_t dip2_high:16;           /* offset:752 */
        uint32_t dip3_low:16;            /* offset:800 */
        uint32_t dip3_high:16;           /* offset:784 */
    } SRv6_Segment2_Header; /* 16B */
}; /* 102B */


/**************************************************************************
 * Thread THR4_SRv6_Source_Node_Second_Pass_3_segments structures
 **************************************************************************/

struct thr4_SRv6_Source_Node_Second_Pass_3_segments_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr4_SRv6_Source_Node_Second_Pass_3_segments_in_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:400 */
        uint32_t mac_da_47_32:16;        /* offset:384 */
        uint32_t mac_sa_47_32:16;        /* offset:432 */
        uint32_t mac_da_15_0:16;         /* offset:416 */
        uint32_t mac_sa_15_0:16;         /* offset:464 */
        uint32_t mac_sa_31_16:16;        /* offset:448 */
        uint32_t vid:12;                 /* offset:500 */
        uint32_t cfi:1;                  /* offset:499 */
        uint32_t up:3;                   /* offset:496 */
        uint32_t TPID:16;                /* offset:480 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t data_3_0:32;            /* offset:784 */
        uint32_t data_7_4:32;            /* offset:752 */
        uint32_t data_11_8:32;           /* offset:720 */
        uint32_t data_15_12:32;          /* offset:688 */
        uint32_t data_19_16:32;          /* offset:656 */
        uint32_t data_23_20:32;          /* offset:624 */
        uint32_t data_27_24:32;          /* offset:592 */
        uint32_t data_31_28:32;          /* offset:560 */
        uint32_t data_35_32:32;          /* offset:528 */
        uint32_t ethertype:16;           /* offset:512 */
    } Generic_TS_Data; /* 54B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:832 */
        uint32_t mac_da_47_32:16;        /* offset:816 */
        uint32_t mac_sa_47_32:16;        /* offset:864 */
        uint32_t mac_da_15_0:16;         /* offset:848 */
        uint32_t mac_sa_15_0:16;         /* offset:896 */
        uint32_t mac_sa_31_16:16;        /* offset:880 */
    } mac_header; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:912 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:944 */
        uint32_t flow_label_19_16:4;     /* offset:940 */
        uint32_t ecn:2;                  /* offset:938 */
        uint32_t dscp:6;                 /* offset:932 */
        uint32_t version:4;              /* offset:928 */
        uint32_t hop_limit:8;            /* offset:984 */
        uint32_t next_header:8;          /* offset:976 */
        uint32_t payload_length:16;      /* offset:960 */
        uint32_t sip0_low:16;            /* offset:1008 */
        uint32_t sip0_high:16;           /* offset:992 */
        uint32_t sip1_low:16;            /* offset:1040 */
        uint32_t sip1_high:16;           /* offset:1024 */
        uint32_t sip2_low:16;            /* offset:1072 */
        uint32_t sip2_high:16;           /* offset:1056 */
        uint32_t sip3_low:16;            /* offset:1104 */
        uint32_t sip3_high:16;           /* offset:1088 */
        uint32_t dip0_low:16;            /* offset:1136 */
        uint32_t dip0_high:16;           /* offset:1120 */
        uint32_t dip1_low:16;            /* offset:1168 */
        uint32_t dip1_high:16;           /* offset:1152 */
        uint32_t dip2_low:16;            /* offset:1200 */
        uint32_t dip2_high:16;           /* offset:1184 */
        uint32_t dip3_low:16;            /* offset:1232 */
        uint32_t dip3_high:16;           /* offset:1216 */
    } IPv6_Header; /* 40B */
}; /* 156B */


struct thr4_SRv6_Source_Node_Second_Pass_3_segments_in_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:400 */
        uint32_t mac_da_47_32:16;        /* offset:384 */
        uint32_t mac_sa_47_32:16;        /* offset:432 */
        uint32_t mac_da_15_0:16;         /* offset:416 */
        uint32_t mac_sa_15_0:16;         /* offset:464 */
        uint32_t mac_sa_31_16:16;        /* offset:448 */
        uint32_t vid:12;                 /* offset:500 */
        uint32_t cfi:1;                  /* offset:499 */
        uint32_t up:3;                   /* offset:496 */
        uint32_t TPID:16;                /* offset:480 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t data_3_0:32;            /* offset:784 */
        uint32_t data_7_4:32;            /* offset:752 */
        uint32_t data_11_8:32;           /* offset:720 */
        uint32_t data_15_12:32;          /* offset:688 */
        uint32_t data_19_16:32;          /* offset:656 */
        uint32_t data_23_20:32;          /* offset:624 */
        uint32_t data_27_24:32;          /* offset:592 */
        uint32_t data_31_28:32;          /* offset:560 */
        uint32_t data_35_32:32;          /* offset:528 */
        uint32_t ethertype:16;           /* offset:512 */
    } Generic_TS_Data; /* 54B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:832 */
        uint32_t mac_da_47_32:16;        /* offset:816 */
        uint32_t mac_sa_47_32:16;        /* offset:864 */
        uint32_t mac_da_15_0:16;         /* offset:848 */
        uint32_t mac_sa_15_0:16;         /* offset:896 */
        uint32_t mac_sa_31_16:16;        /* offset:880 */
    } mac_header; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:932 */
        uint32_t cfi:1;                  /* offset:931 */
        uint32_t up:3;                   /* offset:928 */
        uint32_t TPID:16;                /* offset:912 */
    } vlan_header; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:944 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:976 */
        uint32_t flow_label_19_16:4;     /* offset:972 */
        uint32_t ecn:2;                  /* offset:970 */
        uint32_t dscp:6;                 /* offset:964 */
        uint32_t version:4;              /* offset:960 */
        uint32_t hop_limit:8;            /* offset:1016 */
        uint32_t next_header:8;          /* offset:1008 */
        uint32_t payload_length:16;      /* offset:992 */
        uint32_t sip0_low:16;            /* offset:1040 */
        uint32_t sip0_high:16;           /* offset:1024 */
        uint32_t sip1_low:16;            /* offset:1072 */
        uint32_t sip1_high:16;           /* offset:1056 */
        uint32_t sip2_low:16;            /* offset:1104 */
        uint32_t sip2_high:16;           /* offset:1088 */
        uint32_t sip3_low:16;            /* offset:1136 */
        uint32_t sip3_high:16;           /* offset:1120 */
        uint32_t dip0_low:16;            /* offset:1168 */
        uint32_t dip0_high:16;           /* offset:1152 */
        uint32_t dip1_low:16;            /* offset:1200 */
        uint32_t dip1_high:16;           /* offset:1184 */
        uint32_t dip2_low:16;            /* offset:1232 */
        uint32_t dip2_high:16;           /* offset:1216 */
        uint32_t dip3_low:16;            /* offset:1264 */
        uint32_t dip3_high:16;           /* offset:1248 */
    } IPv6_Header; /* 40B */
}; /* 160B */


struct thr4_SRv6_Source_Node_Second_Pass_3_segments_out_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_15:16;         /* offset:480 */
    } expansion_space; /* 62B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:512 */
        uint32_t mac_da_47_32:16;        /* offset:496 */
        uint32_t mac_sa_47_32:16;        /* offset:544 */
        uint32_t mac_da_15_0:16;         /* offset:528 */
        uint32_t mac_sa_15_0:16;         /* offset:576 */
        uint32_t mac_sa_31_16:16;        /* offset:560 */
    } mac_header; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:592 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:624 */
        uint32_t flow_label_19_16:4;     /* offset:620 */
        uint32_t ecn:2;                  /* offset:618 */
        uint32_t dscp:6;                 /* offset:612 */
        uint32_t version:4;              /* offset:608 */
        uint32_t hop_limit:8;            /* offset:664 */
        uint32_t next_header:8;          /* offset:656 */
        uint32_t payload_length:16;      /* offset:640 */
        uint32_t sip0_low:16;            /* offset:688 */
        uint32_t sip0_high:16;           /* offset:672 */
        uint32_t sip1_low:16;            /* offset:720 */
        uint32_t sip1_high:16;           /* offset:704 */
        uint32_t sip2_low:16;            /* offset:752 */
        uint32_t sip2_high:16;           /* offset:736 */
        uint32_t sip3_low:16;            /* offset:784 */
        uint32_t sip3_high:16;           /* offset:768 */
        uint32_t dip0_low:16;            /* offset:816 */
        uint32_t dip0_high:16;           /* offset:800 */
        uint32_t dip1_low:16;            /* offset:848 */
        uint32_t dip1_high:16;           /* offset:832 */
        uint32_t dip2_low:16;            /* offset:880 */
        uint32_t dip2_high:16;           /* offset:864 */
        uint32_t dip3_low:16;            /* offset:912 */
        uint32_t dip3_high:16;           /* offset:896 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:952 */
        uint32_t routing_type:8;         /* offset:944 */
        uint32_t hdr_ext_len:8;          /* offset:936 */
        uint32_t next_header:8;          /* offset:928 */
        uint32_t tag:16;                 /* offset:976 */
        uint32_t flags:8;                /* offset:968 */
        uint32_t last_entry:8;           /* offset:960 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:1008 */
        uint32_t dip0_high:16;           /* offset:992 */
        uint32_t dip1_low:16;            /* offset:1040 */
        uint32_t dip1_high:16;           /* offset:1024 */
        uint32_t dip2_low:16;            /* offset:1072 */
        uint32_t dip2_high:16;           /* offset:1056 */
        uint32_t dip3_low:16;            /* offset:1104 */
        uint32_t dip3_high:16;           /* offset:1088 */
    } SRv6_Segment0_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:1136 */
        uint32_t dip0_high:16;           /* offset:1120 */
        uint32_t dip1_low:16;            /* offset:1168 */
        uint32_t dip1_high:16;           /* offset:1152 */
        uint32_t dip2_low:16;            /* offset:1200 */
        uint32_t dip2_high:16;           /* offset:1184 */
        uint32_t dip3_low:16;            /* offset:1232 */
        uint32_t dip3_high:16;           /* offset:1216 */
    } SRv6_Segment1_Header; /* 16B */
}; /* 156B */


struct thr4_SRv6_Source_Node_Second_Pass_3_segments_out_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_15:16;         /* offset:480 */
    } expansion_space; /* 62B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:512 */
        uint32_t mac_da_47_32:16;        /* offset:496 */
        uint32_t mac_sa_47_32:16;        /* offset:544 */
        uint32_t mac_da_15_0:16;         /* offset:528 */
        uint32_t mac_sa_15_0:16;         /* offset:576 */
        uint32_t mac_sa_31_16:16;        /* offset:560 */
    } mac_header; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:612 */
        uint32_t cfi:1;                  /* offset:611 */
        uint32_t up:3;                   /* offset:608 */
        uint32_t TPID:16;                /* offset:592 */
    } vlan_header; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:624 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:656 */
        uint32_t flow_label_19_16:4;     /* offset:652 */
        uint32_t ecn:2;                  /* offset:650 */
        uint32_t dscp:6;                 /* offset:644 */
        uint32_t version:4;              /* offset:640 */
        uint32_t hop_limit:8;            /* offset:696 */
        uint32_t next_header:8;          /* offset:688 */
        uint32_t payload_length:16;      /* offset:672 */
        uint32_t sip0_low:16;            /* offset:720 */
        uint32_t sip0_high:16;           /* offset:704 */
        uint32_t sip1_low:16;            /* offset:752 */
        uint32_t sip1_high:16;           /* offset:736 */
        uint32_t sip2_low:16;            /* offset:784 */
        uint32_t sip2_high:16;           /* offset:768 */
        uint32_t sip3_low:16;            /* offset:816 */
        uint32_t sip3_high:16;           /* offset:800 */
        uint32_t dip0_low:16;            /* offset:848 */
        uint32_t dip0_high:16;           /* offset:832 */
        uint32_t dip1_low:16;            /* offset:880 */
        uint32_t dip1_high:16;           /* offset:864 */
        uint32_t dip2_low:16;            /* offset:912 */
        uint32_t dip2_high:16;           /* offset:896 */
        uint32_t dip3_low:16;            /* offset:944 */
        uint32_t dip3_high:16;           /* offset:928 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:984 */
        uint32_t routing_type:8;         /* offset:976 */
        uint32_t hdr_ext_len:8;          /* offset:968 */
        uint32_t next_header:8;          /* offset:960 */
        uint32_t tag:16;                 /* offset:1008 */
        uint32_t flags:8;                /* offset:1000 */
        uint32_t last_entry:8;           /* offset:992 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:1040 */
        uint32_t dip0_high:16;           /* offset:1024 */
        uint32_t dip1_low:16;            /* offset:1072 */
        uint32_t dip1_high:16;           /* offset:1056 */
        uint32_t dip2_low:16;            /* offset:1104 */
        uint32_t dip2_high:16;           /* offset:1088 */
        uint32_t dip3_low:16;            /* offset:1136 */
        uint32_t dip3_high:16;           /* offset:1120 */
    } SRv6_Segment0_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:1168 */
        uint32_t dip0_high:16;           /* offset:1152 */
        uint32_t dip1_low:16;            /* offset:1200 */
        uint32_t dip1_high:16;           /* offset:1184 */
        uint32_t dip2_low:16;            /* offset:1232 */
        uint32_t dip2_high:16;           /* offset:1216 */
        uint32_t dip3_low:16;            /* offset:1264 */
        uint32_t dip3_high:16;           /* offset:1248 */
    } SRv6_Segment1_Header; /* 16B */
}; /* 160B */


/**************************************************************************
 * Thread THR5_SRv6_Source_Node_Second_Pass_2_segments structures
 **************************************************************************/

struct thr5_SRv6_Source_Node_Second_Pass_2_segments_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr5_SRv6_Source_Node_Second_Pass_2_segments_in_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:400 */
        uint32_t mac_da_47_32:16;        /* offset:384 */
        uint32_t mac_sa_47_32:16;        /* offset:432 */
        uint32_t mac_da_15_0:16;         /* offset:416 */
        uint32_t mac_sa_15_0:16;         /* offset:464 */
        uint32_t mac_sa_31_16:16;        /* offset:448 */
        uint32_t vid:12;                 /* offset:500 */
        uint32_t cfi:1;                  /* offset:499 */
        uint32_t up:3;                   /* offset:496 */
        uint32_t TPID:16;                /* offset:480 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t data_3_0:32;            /* offset:784 */
        uint32_t data_7_4:32;            /* offset:752 */
        uint32_t data_11_8:32;           /* offset:720 */
        uint32_t data_15_12:32;          /* offset:688 */
        uint32_t data_19_16:32;          /* offset:656 */
        uint32_t data_23_20:32;          /* offset:624 */
        uint32_t data_27_24:32;          /* offset:592 */
        uint32_t data_31_28:32;          /* offset:560 */
        uint32_t data_35_32:32;          /* offset:528 */
        uint32_t ethertype:16;           /* offset:512 */
    } Generic_TS_Data; /* 54B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:832 */
        uint32_t mac_da_47_32:16;        /* offset:816 */
        uint32_t mac_sa_47_32:16;        /* offset:864 */
        uint32_t mac_da_15_0:16;         /* offset:848 */
        uint32_t mac_sa_15_0:16;         /* offset:896 */
        uint32_t mac_sa_31_16:16;        /* offset:880 */
    } mac_header; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:912 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:944 */
        uint32_t flow_label_19_16:4;     /* offset:940 */
        uint32_t ecn:2;                  /* offset:938 */
        uint32_t dscp:6;                 /* offset:932 */
        uint32_t version:4;              /* offset:928 */
        uint32_t hop_limit:8;            /* offset:984 */
        uint32_t next_header:8;          /* offset:976 */
        uint32_t payload_length:16;      /* offset:960 */
        uint32_t sip0_low:16;            /* offset:1008 */
        uint32_t sip0_high:16;           /* offset:992 */
        uint32_t sip1_low:16;            /* offset:1040 */
        uint32_t sip1_high:16;           /* offset:1024 */
        uint32_t sip2_low:16;            /* offset:1072 */
        uint32_t sip2_high:16;           /* offset:1056 */
        uint32_t sip3_low:16;            /* offset:1104 */
        uint32_t sip3_high:16;           /* offset:1088 */
        uint32_t dip0_low:16;            /* offset:1136 */
        uint32_t dip0_high:16;           /* offset:1120 */
        uint32_t dip1_low:16;            /* offset:1168 */
        uint32_t dip1_high:16;           /* offset:1152 */
        uint32_t dip2_low:16;            /* offset:1200 */
        uint32_t dip2_high:16;           /* offset:1184 */
        uint32_t dip3_low:16;            /* offset:1232 */
        uint32_t dip3_high:16;           /* offset:1216 */
    } IPv6_Header; /* 40B */
}; /* 156B */


struct thr5_SRv6_Source_Node_Second_Pass_2_segments_in_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:400 */
        uint32_t mac_da_47_32:16;        /* offset:384 */
        uint32_t mac_sa_47_32:16;        /* offset:432 */
        uint32_t mac_da_15_0:16;         /* offset:416 */
        uint32_t mac_sa_15_0:16;         /* offset:464 */
        uint32_t mac_sa_31_16:16;        /* offset:448 */
        uint32_t vid:12;                 /* offset:500 */
        uint32_t cfi:1;                  /* offset:499 */
        uint32_t up:3;                   /* offset:496 */
        uint32_t TPID:16;                /* offset:480 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t data_3_0:32;            /* offset:784 */
        uint32_t data_7_4:32;            /* offset:752 */
        uint32_t data_11_8:32;           /* offset:720 */
        uint32_t data_15_12:32;          /* offset:688 */
        uint32_t data_19_16:32;          /* offset:656 */
        uint32_t data_23_20:32;          /* offset:624 */
        uint32_t data_27_24:32;          /* offset:592 */
        uint32_t data_31_28:32;          /* offset:560 */
        uint32_t data_35_32:32;          /* offset:528 */
        uint32_t ethertype:16;           /* offset:512 */
    } Generic_TS_Data; /* 54B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:832 */
        uint32_t mac_da_47_32:16;        /* offset:816 */
        uint32_t mac_sa_47_32:16;        /* offset:864 */
        uint32_t mac_da_15_0:16;         /* offset:848 */
        uint32_t mac_sa_15_0:16;         /* offset:896 */
        uint32_t mac_sa_31_16:16;        /* offset:880 */
    } mac_header; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:932 */
        uint32_t cfi:1;                  /* offset:931 */
        uint32_t up:3;                   /* offset:928 */
        uint32_t TPID:16;                /* offset:912 */
    } vlan_header; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:944 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:976 */
        uint32_t flow_label_19_16:4;     /* offset:972 */
        uint32_t ecn:2;                  /* offset:970 */
        uint32_t dscp:6;                 /* offset:964 */
        uint32_t version:4;              /* offset:960 */
        uint32_t hop_limit:8;            /* offset:1016 */
        uint32_t next_header:8;          /* offset:1008 */
        uint32_t payload_length:16;      /* offset:992 */
        uint32_t sip0_low:16;            /* offset:1040 */
        uint32_t sip0_high:16;           /* offset:1024 */
        uint32_t sip1_low:16;            /* offset:1072 */
        uint32_t sip1_high:16;           /* offset:1056 */
        uint32_t sip2_low:16;            /* offset:1104 */
        uint32_t sip2_high:16;           /* offset:1088 */
        uint32_t sip3_low:16;            /* offset:1136 */
        uint32_t sip3_high:16;           /* offset:1120 */
        uint32_t dip0_low:16;            /* offset:1168 */
        uint32_t dip0_high:16;           /* offset:1152 */
        uint32_t dip1_low:16;            /* offset:1200 */
        uint32_t dip1_high:16;           /* offset:1184 */
        uint32_t dip2_low:16;            /* offset:1232 */
        uint32_t dip2_high:16;           /* offset:1216 */
        uint32_t dip3_low:16;            /* offset:1264 */
        uint32_t dip3_high:16;           /* offset:1248 */
    } IPv6_Header; /* 40B */
}; /* 160B */


struct thr5_SRv6_Source_Node_Second_Pass_2_segments_out_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
        uint32_t reserved_16:32;         /* offset:512 */
        uint32_t reserved_17:32;         /* offset:544 */
        uint32_t reserved_18:32;         /* offset:576 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_19:16;         /* offset:608 */
    } expansion_space; /* 78B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:640 */
        uint32_t mac_da_47_32:16;        /* offset:624 */
        uint32_t mac_sa_47_32:16;        /* offset:672 */
        uint32_t mac_da_15_0:16;         /* offset:656 */
        uint32_t mac_sa_15_0:16;         /* offset:704 */
        uint32_t mac_sa_31_16:16;        /* offset:688 */
    } mac_header; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:720 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:752 */
        uint32_t flow_label_19_16:4;     /* offset:748 */
        uint32_t ecn:2;                  /* offset:746 */
        uint32_t dscp:6;                 /* offset:740 */
        uint32_t version:4;              /* offset:736 */
        uint32_t hop_limit:8;            /* offset:792 */
        uint32_t next_header:8;          /* offset:784 */
        uint32_t payload_length:16;      /* offset:768 */
        uint32_t sip0_low:16;            /* offset:816 */
        uint32_t sip0_high:16;           /* offset:800 */
        uint32_t sip1_low:16;            /* offset:848 */
        uint32_t sip1_high:16;           /* offset:832 */
        uint32_t sip2_low:16;            /* offset:880 */
        uint32_t sip2_high:16;           /* offset:864 */
        uint32_t sip3_low:16;            /* offset:912 */
        uint32_t sip3_high:16;           /* offset:896 */
        uint32_t dip0_low:16;            /* offset:944 */
        uint32_t dip0_high:16;           /* offset:928 */
        uint32_t dip1_low:16;            /* offset:976 */
        uint32_t dip1_high:16;           /* offset:960 */
        uint32_t dip2_low:16;            /* offset:1008 */
        uint32_t dip2_high:16;           /* offset:992 */
        uint32_t dip3_low:16;            /* offset:1040 */
        uint32_t dip3_high:16;           /* offset:1024 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:1080 */
        uint32_t routing_type:8;         /* offset:1072 */
        uint32_t hdr_ext_len:8;          /* offset:1064 */
        uint32_t next_header:8;          /* offset:1056 */
        uint32_t tag:16;                 /* offset:1104 */
        uint32_t flags:8;                /* offset:1096 */
        uint32_t last_entry:8;           /* offset:1088 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:1136 */
        uint32_t dip0_high:16;           /* offset:1120 */
        uint32_t dip1_low:16;            /* offset:1168 */
        uint32_t dip1_high:16;           /* offset:1152 */
        uint32_t dip2_low:16;            /* offset:1200 */
        uint32_t dip2_high:16;           /* offset:1184 */
        uint32_t dip3_low:16;            /* offset:1232 */
        uint32_t dip3_high:16;           /* offset:1216 */
    } SRv6_Segment0_Header; /* 16B */
}; /* 156B */


struct thr5_SRv6_Source_Node_Second_Pass_2_segments_out_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
        uint32_t reserved_16:32;         /* offset:512 */
        uint32_t reserved_17:32;         /* offset:544 */
        uint32_t reserved_18:32;         /* offset:576 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_19:16;         /* offset:608 */
    } expansion_space; /* 78B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:640 */
        uint32_t mac_da_47_32:16;        /* offset:624 */
        uint32_t mac_sa_47_32:16;        /* offset:672 */
        uint32_t mac_da_15_0:16;         /* offset:656 */
        uint32_t mac_sa_15_0:16;         /* offset:704 */
        uint32_t mac_sa_31_16:16;        /* offset:688 */
    } mac_header; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:740 */
        uint32_t cfi:1;                  /* offset:739 */
        uint32_t up:3;                   /* offset:736 */
        uint32_t TPID:16;                /* offset:720 */
    } vlan_header; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:752 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:784 */
        uint32_t flow_label_19_16:4;     /* offset:780 */
        uint32_t ecn:2;                  /* offset:778 */
        uint32_t dscp:6;                 /* offset:772 */
        uint32_t version:4;              /* offset:768 */
        uint32_t hop_limit:8;            /* offset:824 */
        uint32_t next_header:8;          /* offset:816 */
        uint32_t payload_length:16;      /* offset:800 */
        uint32_t sip0_low:16;            /* offset:848 */
        uint32_t sip0_high:16;           /* offset:832 */
        uint32_t sip1_low:16;            /* offset:880 */
        uint32_t sip1_high:16;           /* offset:864 */
        uint32_t sip2_low:16;            /* offset:912 */
        uint32_t sip2_high:16;           /* offset:896 */
        uint32_t sip3_low:16;            /* offset:944 */
        uint32_t sip3_high:16;           /* offset:928 */
        uint32_t dip0_low:16;            /* offset:976 */
        uint32_t dip0_high:16;           /* offset:960 */
        uint32_t dip1_low:16;            /* offset:1008 */
        uint32_t dip1_high:16;           /* offset:992 */
        uint32_t dip2_low:16;            /* offset:1040 */
        uint32_t dip2_high:16;           /* offset:1024 */
        uint32_t dip3_low:16;            /* offset:1072 */
        uint32_t dip3_high:16;           /* offset:1056 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:1112 */
        uint32_t routing_type:8;         /* offset:1104 */
        uint32_t hdr_ext_len:8;          /* offset:1096 */
        uint32_t next_header:8;          /* offset:1088 */
        uint32_t tag:16;                 /* offset:1136 */
        uint32_t flags:8;                /* offset:1128 */
        uint32_t last_entry:8;           /* offset:1120 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:1168 */
        uint32_t dip0_high:16;           /* offset:1152 */
        uint32_t dip1_low:16;            /* offset:1200 */
        uint32_t dip1_high:16;           /* offset:1184 */
        uint32_t dip2_low:16;            /* offset:1232 */
        uint32_t dip2_high:16;           /* offset:1216 */
        uint32_t dip3_low:16;            /* offset:1264 */
        uint32_t dip3_high:16;           /* offset:1248 */
    } SRv6_Segment0_Header; /* 16B */
}; /* 160B */


/**************************************************************************
 * Thread THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4 structures
 **************************************************************************/

struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg {
    struct {
        uint32_t mac_da_31_16:16;        /* offset:16 */
        uint32_t mac_da_47_32:16;        /* offset:0 */
        uint32_t mac_sa_47_32:16;        /* offset:48 */
        uint32_t mac_da_15_0:16;         /* offset:32 */
        uint32_t mac_sa_15_0:16;         /* offset:80 */
        uint32_t mac_sa_31_16:16;        /* offset:64 */
        uint32_t vlan_vid:12;            /* offset:116 */
        uint32_t vlan_cfi:1;             /* offset:115 */
        uint32_t vlan_up:3;              /* offset:112 */
        uint32_t reserved:16;            /* offset:96 */
    } cc_erspan_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_5:16;          /* offset:160 */
    } expansion_space; /* 22B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:192 */
        uint32_t mac_da_47_32:16;        /* offset:176 */
        uint32_t mac_sa_47_32:16;        /* offset:224 */
        uint32_t mac_da_15_0:16;         /* offset:208 */
        uint32_t mac_sa_15_0:16;         /* offset:256 */
        uint32_t mac_sa_31_16:16;        /* offset:240 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t VID:12;                 /* offset:292 */
        uint32_t Extend0:1;              /* offset:291 */
        uint32_t UP:3;                   /* offset:288 */
        uint32_t CFI:1;                  /* offset:287 */
        uint32_t Reserved:1;             /* offset:286 */
        uint32_t RxSniff:1;              /* offset:285 */
        uint32_t SrcTrgAnalyzer_Port_4_0:5;/* offset:280 */
        uint32_t SrcTrgAnalyzer_Dev:5;   /* offset:275 */
        uint32_t SrcTrg_Tagged:1;        /* offset:274 */
        uint32_t TagCommand:2;           /* offset:272 */
    } extended_DSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Reserved2:10;           /* offset:326 */
        uint32_t SrcTrgAnalyzer_Port_5:1;/* offset:325 */
        uint32_t Reserved1:17;           /* offset:308 */
        uint32_t PacketIsLooped:1;       /* offset:307 */
        uint32_t DropOnSource:1;         /* offset:306 */
        uint32_t Reserved0:1;            /* offset:305 */
        uint32_t Extend1:1;              /* offset:304 */
    } extended_DSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:336 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t total_length:16;        /* offset:368 */
        uint32_t ecn:2;                  /* offset:366 */
        uint32_t dscp:6;                 /* offset:360 */
        uint32_t ihl:4;                  /* offset:356 */
        uint32_t version:4;              /* offset:352 */
        uint32_t fragment_offset:13;     /* offset:403 */
        uint32_t flag_more_fragment:1;   /* offset:402 */
        uint32_t flag_dont_fragment:1;   /* offset:401 */
        uint32_t flag_reserved:1;        /* offset:400 */
        uint32_t identification:16;      /* offset:384 */
        uint32_t header_checksum:16;     /* offset:432 */
        uint32_t protocol:8;             /* offset:424 */
        uint32_t ttl:8;                  /* offset:416 */
        uint32_t sip_low:16;             /* offset:464 */
        uint32_t sip_high:16;            /* offset:448 */
        uint32_t dip_low:16;             /* offset:496 */
        uint32_t dip_high:16;            /* offset:480 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


/**************************************************************************
 * Thread THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6 structures
 **************************************************************************/

struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg {
    struct {
        uint32_t mac_da_31_16:16;        /* offset:16 */
        uint32_t mac_da_47_32:16;        /* offset:0 */
        uint32_t mac_sa_47_32:16;        /* offset:48 */
        uint32_t mac_da_15_0:16;         /* offset:32 */
        uint32_t mac_sa_15_0:16;         /* offset:80 */
        uint32_t mac_sa_31_16:16;        /* offset:64 */
        uint32_t vlan_vid:12;            /* offset:116 */
        uint32_t vlan_cfi:1;             /* offset:115 */
        uint32_t vlan_up:3;              /* offset:112 */
        uint32_t reserved:16;            /* offset:96 */
    } cc_erspan_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_out_hdr {
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_0:16;          /* offset:0 */
    } expansion_space; /* 2B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:32 */
        uint32_t mac_da_47_32:16;        /* offset:16 */
        uint32_t mac_sa_47_32:16;        /* offset:64 */
        uint32_t mac_da_15_0:16;         /* offset:48 */
        uint32_t mac_sa_15_0:16;         /* offset:96 */
        uint32_t mac_sa_31_16:16;        /* offset:80 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t VID:12;                 /* offset:132 */
        uint32_t Extend0:1;              /* offset:131 */
        uint32_t UP:3;                   /* offset:128 */
        uint32_t CFI:1;                  /* offset:127 */
        uint32_t Reserved:1;             /* offset:126 */
        uint32_t RxSniff:1;              /* offset:125 */
        uint32_t SrcTrgAnalyzer_Port_4_0:5;/* offset:120 */
        uint32_t SrcTrgAnalyzer_Dev:5;   /* offset:115 */
        uint32_t SrcTrg_Tagged:1;        /* offset:114 */
        uint32_t TagCommand:2;           /* offset:112 */
    } extended_DSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Reserved2:10;           /* offset:166 */
        uint32_t SrcTrgAnalyzer_Port_5:1;/* offset:165 */
        uint32_t Reserved1:17;           /* offset:148 */
        uint32_t PacketIsLooped:1;       /* offset:147 */
        uint32_t DropOnSource:1;         /* offset:146 */
        uint32_t Reserved0:1;            /* offset:145 */
        uint32_t Extend1:1;              /* offset:144 */
    } extended_DSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:176 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:208 */
        uint32_t flow_label_19_16:4;     /* offset:204 */
        uint32_t ecn:2;                  /* offset:202 */
        uint32_t dscp:6;                 /* offset:196 */
        uint32_t version:4;              /* offset:192 */
        uint32_t hop_limit:8;            /* offset:248 */
        uint32_t next_header:8;          /* offset:240 */
        uint32_t payload_length:16;      /* offset:224 */
        uint32_t sip0_low:16;            /* offset:272 */
        uint32_t sip0_high:16;           /* offset:256 */
        uint32_t sip1_low:16;            /* offset:304 */
        uint32_t sip1_high:16;           /* offset:288 */
        uint32_t sip2_low:16;            /* offset:336 */
        uint32_t sip2_high:16;           /* offset:320 */
        uint32_t sip3_low:16;            /* offset:368 */
        uint32_t sip3_high:16;           /* offset:352 */
        uint32_t dip0_low:16;            /* offset:400 */
        uint32_t dip0_high:16;           /* offset:384 */
        uint32_t dip1_low:16;            /* offset:432 */
        uint32_t dip1_high:16;           /* offset:416 */
        uint32_t dip2_low:16;            /* offset:464 */
        uint32_t dip2_high:16;           /* offset:448 */
        uint32_t dip3_low:16;            /* offset:496 */
        uint32_t dip3_high:16;           /* offset:480 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


/**************************************************************************
 * Thread THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4 structures
 **************************************************************************/

struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg {
    struct {
        uint32_t mac_da_31_16:16;        /* offset:16 */
        uint32_t mac_da_47_32:16;        /* offset:0 */
        uint32_t mac_sa_47_32:16;        /* offset:48 */
        uint32_t mac_da_15_0:16;         /* offset:32 */
        uint32_t mac_sa_15_0:16;         /* offset:80 */
        uint32_t mac_sa_31_16:16;        /* offset:64 */
        uint32_t vlan_vid:12;            /* offset:116 */
        uint32_t vlan_cfi:1;             /* offset:115 */
        uint32_t vlan_up:3;              /* offset:112 */
        uint32_t reserved:16;            /* offset:96 */
    } cc_erspan_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_out_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_7:16;          /* offset:224 */
    } expansion_space; /* 30B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:256 */
        uint32_t mac_da_47_32:16;        /* offset:240 */
        uint32_t mac_sa_47_32:16;        /* offset:288 */
        uint32_t mac_da_15_0:16;         /* offset:272 */
        uint32_t mac_sa_15_0:16;         /* offset:320 */
        uint32_t mac_sa_31_16:16;        /* offset:304 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:336 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t total_length:16;        /* offset:368 */
        uint32_t ecn:2;                  /* offset:366 */
        uint32_t dscp:6;                 /* offset:360 */
        uint32_t ihl:4;                  /* offset:356 */
        uint32_t version:4;              /* offset:352 */
        uint32_t fragment_offset:13;     /* offset:403 */
        uint32_t flag_more_fragment:1;   /* offset:402 */
        uint32_t flag_dont_fragment:1;   /* offset:401 */
        uint32_t flag_reserved:1;        /* offset:400 */
        uint32_t identification:16;      /* offset:384 */
        uint32_t header_checksum:16;     /* offset:432 */
        uint32_t protocol:8;             /* offset:424 */
        uint32_t ttl:8;                  /* offset:416 */
        uint32_t sip_low:16;             /* offset:464 */
        uint32_t sip_high:16;            /* offset:448 */
        uint32_t dip_low:16;             /* offset:496 */
        uint32_t dip_high:16;            /* offset:480 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_out_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_6:16;          /* offset:192 */
    } expansion_space; /* 26B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:224 */
        uint32_t mac_da_47_32:16;        /* offset:208 */
        uint32_t mac_sa_47_32:16;        /* offset:256 */
        uint32_t mac_da_15_0:16;         /* offset:240 */
        uint32_t mac_sa_15_0:16;         /* offset:288 */
        uint32_t mac_sa_31_16:16;        /* offset:272 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:324 */
        uint32_t cfi:1;                  /* offset:323 */
        uint32_t up:3;                   /* offset:320 */
        uint32_t TPID:16;                /* offset:304 */
    } vlan; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:336 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t total_length:16;        /* offset:368 */
        uint32_t ecn:2;                  /* offset:366 */
        uint32_t dscp:6;                 /* offset:360 */
        uint32_t ihl:4;                  /* offset:356 */
        uint32_t version:4;              /* offset:352 */
        uint32_t fragment_offset:13;     /* offset:403 */
        uint32_t flag_more_fragment:1;   /* offset:402 */
        uint32_t flag_dont_fragment:1;   /* offset:401 */
        uint32_t flag_reserved:1;        /* offset:400 */
        uint32_t identification:16;      /* offset:384 */
        uint32_t header_checksum:16;     /* offset:432 */
        uint32_t protocol:8;             /* offset:424 */
        uint32_t ttl:8;                  /* offset:416 */
        uint32_t sip_low:16;             /* offset:464 */
        uint32_t sip_high:16;            /* offset:448 */
        uint32_t dip_low:16;             /* offset:496 */
        uint32_t dip_high:16;            /* offset:480 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


/**************************************************************************
 * Thread THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6 structures
 **************************************************************************/

struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg {
    struct {
        uint32_t mac_da_31_16:16;        /* offset:16 */
        uint32_t mac_da_47_32:16;        /* offset:0 */
        uint32_t mac_sa_47_32:16;        /* offset:48 */
        uint32_t mac_da_15_0:16;         /* offset:32 */
        uint32_t mac_sa_15_0:16;         /* offset:80 */
        uint32_t mac_sa_31_16:16;        /* offset:64 */
        uint32_t vlan_vid:12;            /* offset:116 */
        uint32_t vlan_cfi:1;             /* offset:115 */
        uint32_t vlan_up:3;              /* offset:112 */
        uint32_t reserved:16;            /* offset:96 */
    } cc_erspan_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_out_hdr_no_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_2:16;          /* offset:64 */
    } expansion_space; /* 10B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:96 */
        uint32_t mac_da_47_32:16;        /* offset:80 */
        uint32_t mac_sa_47_32:16;        /* offset:128 */
        uint32_t mac_da_15_0:16;         /* offset:112 */
        uint32_t mac_sa_15_0:16;         /* offset:160 */
        uint32_t mac_sa_31_16:16;        /* offset:144 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:176 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:208 */
        uint32_t flow_label_19_16:4;     /* offset:204 */
        uint32_t ecn:2;                  /* offset:202 */
        uint32_t dscp:6;                 /* offset:196 */
        uint32_t version:4;              /* offset:192 */
        uint32_t hop_limit:8;            /* offset:248 */
        uint32_t next_header:8;          /* offset:240 */
        uint32_t payload_length:16;      /* offset:224 */
        uint32_t sip0_low:16;            /* offset:272 */
        uint32_t sip0_high:16;           /* offset:256 */
        uint32_t sip1_low:16;            /* offset:304 */
        uint32_t sip1_high:16;           /* offset:288 */
        uint32_t sip2_low:16;            /* offset:336 */
        uint32_t sip2_high:16;           /* offset:320 */
        uint32_t sip3_low:16;            /* offset:368 */
        uint32_t sip3_high:16;           /* offset:352 */
        uint32_t dip0_low:16;            /* offset:400 */
        uint32_t dip0_high:16;           /* offset:384 */
        uint32_t dip1_low:16;            /* offset:432 */
        uint32_t dip1_high:16;           /* offset:416 */
        uint32_t dip2_low:16;            /* offset:464 */
        uint32_t dip2_high:16;           /* offset:448 */
        uint32_t dip3_low:16;            /* offset:496 */
        uint32_t dip3_high:16;           /* offset:480 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_out_hdr_with_vlan {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t reserved_1:16;          /* offset:32 */
    } expansion_space; /* 6B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:64 */
        uint32_t mac_da_47_32:16;        /* offset:48 */
        uint32_t mac_sa_47_32:16;        /* offset:96 */
        uint32_t mac_da_15_0:16;         /* offset:80 */
        uint32_t mac_sa_15_0:16;         /* offset:128 */
        uint32_t mac_sa_31_16:16;        /* offset:112 */
    } mac_header_outer; /* 12B */
    struct {
        uint32_t vid:12;                 /* offset:164 */
        uint32_t cfi:1;                  /* offset:163 */
        uint32_t up:3;                   /* offset:160 */
        uint32_t TPID:16;                /* offset:144 */
    } vlan; /* 4B */
    struct {
        uint32_t dummy:16;                     /* added to align to 32 bits words */
        uint32_t ethertype:16;           /* offset:176 */
    } ethertype_header; /* 2B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:208 */
        uint32_t flow_label_19_16:4;     /* offset:204 */
        uint32_t ecn:2;                  /* offset:202 */
        uint32_t dscp:6;                 /* offset:196 */
        uint32_t version:4;              /* offset:192 */
        uint32_t hop_limit:8;            /* offset:248 */
        uint32_t next_header:8;          /* offset:240 */
        uint32_t payload_length:16;      /* offset:224 */
        uint32_t sip0_low:16;            /* offset:272 */
        uint32_t sip0_high:16;           /* offset:256 */
        uint32_t sip1_low:16;            /* offset:304 */
        uint32_t sip1_high:16;           /* offset:288 */
        uint32_t sip2_low:16;            /* offset:336 */
        uint32_t sip2_high:16;           /* offset:320 */
        uint32_t sip3_low:16;            /* offset:368 */
        uint32_t sip3_high:16;           /* offset:352 */
        uint32_t dip0_low:16;            /* offset:400 */
        uint32_t dip0_high:16;           /* offset:384 */
        uint32_t dip1_low:16;            /* offset:432 */
        uint32_t dip1_high:16;           /* offset:416 */
        uint32_t dip2_low:16;            /* offset:464 */
        uint32_t dip2_high:16;           /* offset:448 */
        uint32_t dip3_low:16;            /* offset:496 */
        uint32_t dip3_high:16;           /* offset:480 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t NP:16;                  /* offset:528 */
        uint32_t Version:3;              /* offset:525 */
        uint32_t Flags:5;                /* offset:520 */
        uint32_t Recur:3;                /* offset:517 */
        uint32_t s:1;                    /* offset:516 */
        uint32_t S:1;                    /* offset:515 */
        uint32_t K:1;                    /* offset:514 */
        uint32_t R:1;                    /* offset:513 */
        uint32_t C:1;                    /* offset:512 */
        uint32_t Seq_num:32;             /* offset:544 */
    } GRE_header; /* 8B */
    struct {
        uint32_t Session_id:10;          /* offset:598 */
        uint32_t T:1;                    /* offset:597 */
        uint32_t En:2;                   /* offset:595 */
        uint32_t COS:3;                  /* offset:592 */
        uint32_t VLAN:12;                /* offset:580 */
        uint32_t Ver:4;                  /* offset:576 */
        uint32_t Index:20;               /* offset:620 */
        uint32_t Reserved:12;            /* offset:608 */
    } ERSPAN_type_II_header; /* 8B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:656 */
        uint32_t mac_da_47_32:16;        /* offset:640 */
        uint32_t mac_sa_47_32:16;        /* offset:688 */
        uint32_t mac_da_15_0:16;         /* offset:672 */
        uint32_t mac_sa_15_0:16;         /* offset:720 */
        uint32_t mac_sa_31_16:16;        /* offset:704 */
    } mac_header; /* 12B */
}; /* 92B */


/**************************************************************************
 * Thread THR10_Cc_Erspan_TypeII_SrcDevMirroring structures
 **************************************************************************/

struct thr10_Cc_Erspan_TypeII_SrcDevMirroring_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr10_Cc_Erspan_TypeII_SrcDevMirroring_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


struct thr10_Cc_Erspan_TypeII_SrcDevMirroring_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:528 */
        uint32_t mac_da_47_32:16;        /* offset:512 */
        uint32_t mac_sa_47_32:16;        /* offset:560 */
        uint32_t mac_da_15_0:16;         /* offset:544 */
        uint32_t mac_sa_15_0:16;         /* offset:592 */
        uint32_t mac_sa_31_16:16;        /* offset:576 */
    } mac_header; /* 12B */
    struct {
        uint32_t eVLAN_11_0:12;          /* offset:628 */
        uint32_t Extend0:1;              /* offset:627 */
        uint32_t UP:3;                   /* offset:624 */
        uint32_t CFI:1;                  /* offset:623 */
        uint32_t Reserved0:1;            /* offset:622 */
        uint32_t RxSniff:1;              /* offset:621 */
        uint32_t SrcTrg_Phy_Port_4_0:5;  /* offset:616 */
        uint32_t SrcTrg_Dev_4_0:5;       /* offset:611 */
        uint32_t SrcTrg_Tagged:1;        /* offset:610 */
        uint32_t TagCommand:2;           /* offset:608 */
    } eDSA_w0_ToAnalyzer; /* 4B */
    struct {
        uint32_t Analyzer_Trg_Phy_Port:10;/* offset:662 */
        uint32_t SrcTrg_Phy_Port_5:1;    /* offset:661 */
        uint32_t SrcTrg_Phy_Port_6:1;    /* offset:660 */
        uint32_t Reserved1:2;            /* offset:658 */
        uint32_t Analyzer_Trg_Dev:12;    /* offset:646 */
        uint32_t Analyzer_Use_eVIDX:1;   /* offset:645 */
        uint32_t Analyzer_Is_Trg_Phy_Port_Valid:1;/* offset:644 */
        uint32_t PacketIsLooped:1;       /* offset:643 */
        uint32_t DropOnSource:1;         /* offset:642 */
        uint32_t Reserved0:1;            /* offset:641 */
        uint32_t Extend1:1;              /* offset:640 */
    } eDSA_w1_ToAnalyzer; /* 4B */
    struct {
        uint32_t TPID_index:3;           /* offset:701 */
        uint32_t SrcTrg_ePort:17;        /* offset:684 */
        uint32_t SrcTrg_Phy_Port_7:1;    /* offset:683 */
        uint32_t Reserved:10;            /* offset:673 */
        uint32_t Extend2:1;              /* offset:672 */
    } eDSA_w2_ToAnalyzer; /* 4B */
    struct {
        uint32_t SrcTrg_Dev_11_5:7;      /* offset:729 */
        uint32_t Analyzer_ePort:17;      /* offset:712 */
        uint32_t Reserved:3;             /* offset:709 */
        uint32_t eVLAN_15_12:4;          /* offset:705 */
        uint32_t Extend3:1;              /* offset:704 */
    } eDSA_w3_ToAnalyzer; /* 4B */
}; /* 92B */


/**************************************************************************
 * Thread THR11_VXLAN_GPB_SourceGroupPolicyID structures
 **************************************************************************/

struct thr11_vxlan_gpb_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t CopyReservedLSB:5;      /* offset:123 */
        uint32_t CopyReservedMSB:5;      /* offset:118 */
        uint32_t reserved_3:22;          /* offset:96 */
    } vxlan_gpb_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr11_vxlan_gpb_in_hdr_ipv4 {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t total_length:16;        /* offset:272 */
        uint32_t ecn:2;                  /* offset:270 */
        uint32_t dscp:6;                 /* offset:264 */
        uint32_t ihl:4;                  /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t fragment_offset:13;     /* offset:307 */
        uint32_t flag_more_fragment:1;   /* offset:306 */
        uint32_t flag_dont_fragment:1;   /* offset:305 */
        uint32_t flag_reserved:1;        /* offset:304 */
        uint32_t identification:16;      /* offset:288 */
        uint32_t header_checksum:16;     /* offset:336 */
        uint32_t protocol:8;             /* offset:328 */
        uint32_t ttl:8;                  /* offset:320 */
        uint32_t sip_low:16;             /* offset:368 */
        uint32_t sip_high:16;            /* offset:352 */
        uint32_t dip_low:16;             /* offset:400 */
        uint32_t dip_high:16;            /* offset:384 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t DestinationPort:16;     /* offset:432 */
        uint32_t SourcePort:16;          /* offset:416 */
        uint32_t Checksum:16;            /* offset:464 */
        uint32_t Length:16;              /* offset:448 */
    } udp_header; /* 8B */
    struct {
        uint32_t Group_Policy_ID:16;     /* offset:496 */
        uint32_t Res12:1;                /* offset:495 */
        uint32_t Res11:1;                /* offset:494 */
        uint32_t Res10:1;                /* offset:493 */
        uint32_t A:1;                    /* offset:492 */
        uint32_t Res9:1;                 /* offset:491 */
        uint32_t Res8:1;                 /* offset:490 */
        uint32_t D:1;                    /* offset:489 */
        uint32_t Res7:1;                 /* offset:488 */
        uint32_t Res6:1;                 /* offset:487 */
        uint32_t Res5:1;                 /* offset:486 */
        uint32_t Res4:1;                 /* offset:485 */
        uint32_t I:1;                    /* offset:484 */
        uint32_t Res3:1;                 /* offset:483 */
        uint32_t Res2:1;                 /* offset:482 */
        uint32_t Res1:1;                 /* offset:481 */
        uint32_t G:1;                    /* offset:480 */
        uint32_t Reserved:8;             /* offset:536 */
        uint32_t VNI:24;                 /* offset:512 */
    } vxlan_gbp_header; /* 8B */
}; /* 68B */


struct thr11_vxlan_gpb_in_hdr_ipv6 {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t DestinationPort:16;     /* offset:592 */
        uint32_t SourcePort:16;          /* offset:576 */
        uint32_t Checksum:16;            /* offset:624 */
        uint32_t Length:16;              /* offset:608 */
    } udp_header; /* 8B */
    struct {
        uint32_t Group_Policy_ID:16;     /* offset:656 */
        uint32_t Res12:1;                /* offset:655 */
        uint32_t Res11:1;                /* offset:654 */
        uint32_t Res10:1;                /* offset:653 */
        uint32_t A:1;                    /* offset:652 */
        uint32_t Res9:1;                 /* offset:651 */
        uint32_t Res8:1;                 /* offset:650 */
        uint32_t D:1;                    /* offset:649 */
        uint32_t Res7:1;                 /* offset:648 */
        uint32_t Res6:1;                 /* offset:647 */
        uint32_t Res5:1;                 /* offset:646 */
        uint32_t Res4:1;                 /* offset:645 */
        uint32_t I:1;                    /* offset:644 */
        uint32_t Res3:1;                 /* offset:643 */
        uint32_t Res2:1;                 /* offset:642 */
        uint32_t Res1:1;                 /* offset:641 */
        uint32_t G:1;                    /* offset:640 */
        uint32_t Reserved:8;             /* offset:696 */
        uint32_t VNI:24;                 /* offset:672 */
    } vxlan_gbp_header; /* 8B */
}; /* 88B */


struct thr11_vxlan_gpb_out_hdr_ipv4 {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t total_length:16;        /* offset:272 */
        uint32_t ecn:2;                  /* offset:270 */
        uint32_t dscp:6;                 /* offset:264 */
        uint32_t ihl:4;                  /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t fragment_offset:13;     /* offset:307 */
        uint32_t flag_more_fragment:1;   /* offset:306 */
        uint32_t flag_dont_fragment:1;   /* offset:305 */
        uint32_t flag_reserved:1;        /* offset:304 */
        uint32_t identification:16;      /* offset:288 */
        uint32_t header_checksum:16;     /* offset:336 */
        uint32_t protocol:8;             /* offset:328 */
        uint32_t ttl:8;                  /* offset:320 */
        uint32_t sip_low:16;             /* offset:368 */
        uint32_t sip_high:16;            /* offset:352 */
        uint32_t dip_low:16;             /* offset:400 */
        uint32_t dip_high:16;            /* offset:384 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t DestinationPort:16;     /* offset:432 */
        uint32_t SourcePort:16;          /* offset:416 */
        uint32_t Checksum:16;            /* offset:464 */
        uint32_t Length:16;              /* offset:448 */
    } udp_header; /* 8B */
    struct {
        uint32_t Group_Policy_ID:16;     /* offset:496 */
        uint32_t Res12:1;                /* offset:495 */
        uint32_t Res11:1;                /* offset:494 */
        uint32_t Res10:1;                /* offset:493 */
        uint32_t A:1;                    /* offset:492 */
        uint32_t Res9:1;                 /* offset:491 */
        uint32_t Res8:1;                 /* offset:490 */
        uint32_t D:1;                    /* offset:489 */
        uint32_t Res7:1;                 /* offset:488 */
        uint32_t Res6:1;                 /* offset:487 */
        uint32_t Res5:1;                 /* offset:486 */
        uint32_t Res4:1;                 /* offset:485 */
        uint32_t I:1;                    /* offset:484 */
        uint32_t Res3:1;                 /* offset:483 */
        uint32_t Res2:1;                 /* offset:482 */
        uint32_t Res1:1;                 /* offset:481 */
        uint32_t G:1;                    /* offset:480 */
        uint32_t Reserved:8;             /* offset:536 */
        uint32_t VNI:24;                 /* offset:512 */
    } vxlan_gbp_header; /* 8B */
}; /* 68B */


struct thr11_vxlan_gpb_out_hdr_ipv6 {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t DestinationPort:16;     /* offset:592 */
        uint32_t SourcePort:16;          /* offset:576 */
        uint32_t Checksum:16;            /* offset:624 */
        uint32_t Length:16;              /* offset:608 */
    } udp_header; /* 8B */
    struct {
        uint32_t Group_Policy_ID:16;     /* offset:656 */
        uint32_t Res12:1;                /* offset:655 */
        uint32_t Res11:1;                /* offset:654 */
        uint32_t Res10:1;                /* offset:653 */
        uint32_t A:1;                    /* offset:652 */
        uint32_t Res9:1;                 /* offset:651 */
        uint32_t Res8:1;                 /* offset:650 */
        uint32_t D:1;                    /* offset:649 */
        uint32_t Res7:1;                 /* offset:648 */
        uint32_t Res6:1;                 /* offset:647 */
        uint32_t Res5:1;                 /* offset:646 */
        uint32_t Res4:1;                 /* offset:645 */
        uint32_t I:1;                    /* offset:644 */
        uint32_t Res3:1;                 /* offset:643 */
        uint32_t Res2:1;                 /* offset:642 */
        uint32_t Res1:1;                 /* offset:641 */
        uint32_t G:1;                    /* offset:640 */
        uint32_t Reserved:8;             /* offset:696 */
        uint32_t VNI:24;                 /* offset:672 */
    } vxlan_gbp_header; /* 8B */
}; /* 88B */


/**************************************************************************
 * Thread THR12_MPLS_SR_NO_EL structures
 **************************************************************************/

struct thr12_MPLS_NO_EL_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr12_MPLS_NO_EL_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t New_L3_ofst:8;          /* offset:408 */
        uint32_t EL1_ofst:8;             /* offset:400 */
        uint32_t EL2_ofst:8;             /* offset:392 */
        uint32_t EL3_ofst:8;             /* offset:384 */
    } MPLS_data; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


struct thr12_MPLS_NO_EL_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
    } expansion_space; /* 52B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


/**************************************************************************
 * Thread THR13_MPLS_SR_ONE_EL structures
 **************************************************************************/

struct thr13_MPLS_ONE_EL_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr13_MPLS_ONE_EL_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t New_L3_ofst:8;          /* offset:408 */
        uint32_t EL1_ofst:8;             /* offset:400 */
        uint32_t EL2_ofst:8;             /* offset:392 */
        uint32_t EL3_ofst:8;             /* offset:384 */
    } MPLS_data; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


struct thr13_MPLS_ONE_EL_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
    } expansion_space; /* 44B */
    struct {
        uint32_t TTL:8;                  /* offset:376 */
        uint32_t BoS:1;                  /* offset:375 */
        uint32_t TC:3;                   /* offset:372 */
        uint32_t label_val:20;           /* offset:352 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:408 */
        uint32_t BoS:1;                  /* offset:407 */
        uint32_t TC:3;                   /* offset:404 */
        uint32_t label_val:20;           /* offset:384 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_ELI_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_EL_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


/**************************************************************************
 * Thread THR14_MPLS_SR_TWO_EL structures
 **************************************************************************/

struct thr14_MPLS_TWO_EL_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr14_MPLS_TWO_EL_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t New_L3_ofst:8;          /* offset:408 */
        uint32_t EL1_ofst:8;             /* offset:400 */
        uint32_t EL2_ofst:8;             /* offset:392 */
        uint32_t EL3_ofst:8;             /* offset:384 */
    } MPLS_data; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


struct thr14_MPLS_TWO_EL_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
    } expansion_space; /* 36B */
    struct {
        uint32_t TTL:8;                  /* offset:312 */
        uint32_t BoS:1;                  /* offset:311 */
        uint32_t TC:3;                   /* offset:308 */
        uint32_t label_val:20;           /* offset:288 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:344 */
        uint32_t BoS:1;                  /* offset:343 */
        uint32_t TC:3;                   /* offset:340 */
        uint32_t label_val:20;           /* offset:320 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:376 */
        uint32_t BoS:1;                  /* offset:375 */
        uint32_t TC:3;                   /* offset:372 */
        uint32_t label_val:20;           /* offset:352 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:408 */
        uint32_t BoS:1;                  /* offset:407 */
        uint32_t TC:3;                   /* offset:404 */
        uint32_t label_val:20;           /* offset:384 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_ELI_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_EL_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_ELI_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_EL_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


/**************************************************************************
 * Thread THR15_MPLS_SR_THREE_EL structures
 **************************************************************************/

struct thr15_MPLS_THREE_EL_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr15_MPLS_THREE_EL_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
    } expansion_space; /* 48B */
    struct {
        uint32_t New_L3_ofst:8;          /* offset:408 */
        uint32_t EL1_ofst:8;             /* offset:400 */
        uint32_t EL2_ofst:8;             /* offset:392 */
        uint32_t EL3_ofst:8;             /* offset:384 */
    } MPLS_data; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


struct thr15_MPLS_THREE_EL_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
    } expansion_space; /* 28B */
    struct {
        uint32_t TTL:8;                  /* offset:248 */
        uint32_t BoS:1;                  /* offset:247 */
        uint32_t TC:3;                   /* offset:244 */
        uint32_t label_val:20;           /* offset:224 */
    } MPLS_label_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:280 */
        uint32_t BoS:1;                  /* offset:279 */
        uint32_t TC:3;                   /* offset:276 */
        uint32_t label_val:20;           /* offset:256 */
    } MPLS_label_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:312 */
        uint32_t BoS:1;                  /* offset:311 */
        uint32_t TC:3;                   /* offset:308 */
        uint32_t label_val:20;           /* offset:288 */
    } MPLS_ELI_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:344 */
        uint32_t BoS:1;                  /* offset:343 */
        uint32_t TC:3;                   /* offset:340 */
        uint32_t label_val:20;           /* offset:320 */
    } MPLS_EL_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:376 */
        uint32_t BoS:1;                  /* offset:375 */
        uint32_t TC:3;                   /* offset:372 */
        uint32_t label_val:20;           /* offset:352 */
    } MPLS_label_3; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:408 */
        uint32_t BoS:1;                  /* offset:407 */
        uint32_t TC:3;                   /* offset:404 */
        uint32_t label_val:20;           /* offset:384 */
    } MPLS_label_4; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:440 */
        uint32_t BoS:1;                  /* offset:439 */
        uint32_t TC:3;                   /* offset:436 */
        uint32_t label_val:20;           /* offset:416 */
    } MPLS_ELI_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:472 */
        uint32_t BoS:1;                  /* offset:471 */
        uint32_t TC:3;                   /* offset:468 */
        uint32_t label_val:20;           /* offset:448 */
    } MPLS_EL_2; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:504 */
        uint32_t BoS:1;                  /* offset:503 */
        uint32_t TC:3;                   /* offset:500 */
        uint32_t label_val:20;           /* offset:480 */
    } MPLS_label_5; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:536 */
        uint32_t BoS:1;                  /* offset:535 */
        uint32_t TC:3;                   /* offset:532 */
        uint32_t label_val:20;           /* offset:512 */
    } MPLS_label_6; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:568 */
        uint32_t BoS:1;                  /* offset:567 */
        uint32_t TC:3;                   /* offset:564 */
        uint32_t label_val:20;           /* offset:544 */
    } MPLS_ELI_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:600 */
        uint32_t BoS:1;                  /* offset:599 */
        uint32_t TC:3;                   /* offset:596 */
        uint32_t label_val:20;           /* offset:576 */
    } MPLS_EL_1; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:632 */
        uint32_t BoS:1;                  /* offset:631 */
        uint32_t TC:3;                   /* offset:628 */
        uint32_t label_val:20;           /* offset:608 */
    } MPLS_label_7; /* 4B */
    struct {
        uint32_t TTL:8;                  /* offset:664 */
        uint32_t BoS:1;                  /* offset:663 */
        uint32_t TC:3;                   /* offset:660 */
        uint32_t label_val:20;           /* offset:640 */
    } MPLS_label_8; /* 4B */
}; /* 84B */


/**************************************************************************
 * Thread THR53_SRV6_End_Node_GSID_COC32 structures
 **************************************************************************/

struct thr53_SRV6_End_Node_GSID_COC32_cfg {
    struct {
        uint32_t reserved_3:32;          /* offset:0 */
        uint32_t reserved_2:32;          /* offset:32 */
        uint32_t reserved_1:32;          /* offset:64 */
        uint32_t commonPrefixLength:8;   /* offset:120 */
        uint32_t reserved_0:24;          /* offset:96 */
    } SRV6_GSID_COC32_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr53_SRV6_End_Node_GSID_COC32_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:600 */
        uint32_t routing_type:8;         /* offset:592 */
        uint32_t hdr_ext_len:8;          /* offset:584 */
        uint32_t next_header:8;          /* offset:576 */
        uint32_t tag:16;                 /* offset:624 */
        uint32_t flags:8;                /* offset:616 */
        uint32_t last_entry:8;           /* offset:608 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:656 */
        uint32_t dip0_high:16;           /* offset:640 */
        uint32_t dip1_low:16;            /* offset:688 */
        uint32_t dip1_high:16;           /* offset:672 */
        uint32_t dip2_low:16;            /* offset:720 */
        uint32_t dip2_high:16;           /* offset:704 */
        uint32_t dip3_low:16;            /* offset:752 */
        uint32_t dip3_high:16;           /* offset:736 */
    } SRv6_Segment0_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:784 */
        uint32_t dip0_high:16;           /* offset:768 */
        uint32_t dip1_low:16;            /* offset:816 */
        uint32_t dip1_high:16;           /* offset:800 */
        uint32_t dip2_low:16;            /* offset:848 */
        uint32_t dip2_high:16;           /* offset:832 */
        uint32_t dip3_low:16;            /* offset:880 */
        uint32_t dip3_high:16;           /* offset:864 */
    } SRv6_Segment1_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:912 */
        uint32_t dip0_high:16;           /* offset:896 */
        uint32_t dip1_low:16;            /* offset:944 */
        uint32_t dip1_high:16;           /* offset:928 */
        uint32_t dip2_low:16;            /* offset:976 */
        uint32_t dip2_high:16;           /* offset:960 */
        uint32_t dip3_low:16;            /* offset:1008 */
        uint32_t dip3_high:16;           /* offset:992 */
    } SRv6_Segment2_Header; /* 16B */
}; /* 128B */


struct thr53_SRV6_End_Node_GSID_COC32_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t segments_left:8;        /* offset:600 */
        uint32_t routing_type:8;         /* offset:592 */
        uint32_t hdr_ext_len:8;          /* offset:584 */
        uint32_t next_header:8;          /* offset:576 */
        uint32_t tag:16;                 /* offset:624 */
        uint32_t flags:8;                /* offset:616 */
        uint32_t last_entry:8;           /* offset:608 */
    } SRv6_Header; /* 8B */
    struct {
        uint32_t dip0_low:16;            /* offset:656 */
        uint32_t dip0_high:16;           /* offset:640 */
        uint32_t dip1_low:16;            /* offset:688 */
        uint32_t dip1_high:16;           /* offset:672 */
        uint32_t dip2_low:16;            /* offset:720 */
        uint32_t dip2_high:16;           /* offset:704 */
        uint32_t dip3_low:16;            /* offset:752 */
        uint32_t dip3_high:16;           /* offset:736 */
    } SRv6_Segment0_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:784 */
        uint32_t dip0_high:16;           /* offset:768 */
        uint32_t dip1_low:16;            /* offset:816 */
        uint32_t dip1_high:16;           /* offset:800 */
        uint32_t dip2_low:16;            /* offset:848 */
        uint32_t dip2_high:16;           /* offset:832 */
        uint32_t dip3_low:16;            /* offset:880 */
        uint32_t dip3_high:16;           /* offset:864 */
    } SRv6_Segment1_Header; /* 16B */
    struct {
        uint32_t dip0_low:16;            /* offset:912 */
        uint32_t dip0_high:16;           /* offset:896 */
        uint32_t dip1_low:16;            /* offset:944 */
        uint32_t dip1_high:16;           /* offset:928 */
        uint32_t dip2_low:16;            /* offset:976 */
        uint32_t dip2_high:16;           /* offset:960 */
        uint32_t dip3_low:16;            /* offset:1008 */
        uint32_t dip3_high:16;           /* offset:992 */
    } SRv6_Segment2_Header; /* 16B */
}; /* 128B */


/**************************************************************************
 * Thread THR54_IPv4_TTL_Increment structures
 **************************************************************************/

struct thr54_IPv4_TTL_Increment_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr54_IPv4_TTL_Increment_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t total_length:16;        /* offset:272 */
        uint32_t ecn:2;                  /* offset:270 */
        uint32_t dscp:6;                 /* offset:264 */
        uint32_t ihl:4;                  /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t fragment_offset:13;     /* offset:307 */
        uint32_t flag_more_fragment:1;   /* offset:306 */
        uint32_t flag_dont_fragment:1;   /* offset:305 */
        uint32_t flag_reserved:1;        /* offset:304 */
        uint32_t identification:16;      /* offset:288 */
        uint32_t header_checksum:16;     /* offset:336 */
        uint32_t protocol:8;             /* offset:328 */
        uint32_t ttl:8;                  /* offset:320 */
        uint32_t sip_low:16;             /* offset:368 */
        uint32_t sip_high:16;            /* offset:352 */
        uint32_t dip_low:16;             /* offset:400 */
        uint32_t dip_high:16;            /* offset:384 */
    } IPv4_Header; /* 20B */
}; /* 52B */


struct thr54_IPv4_TTL_Increment_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t total_length:16;        /* offset:272 */
        uint32_t ecn:2;                  /* offset:270 */
        uint32_t dscp:6;                 /* offset:264 */
        uint32_t ihl:4;                  /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t fragment_offset:13;     /* offset:307 */
        uint32_t flag_more_fragment:1;   /* offset:306 */
        uint32_t flag_dont_fragment:1;   /* offset:305 */
        uint32_t flag_reserved:1;        /* offset:304 */
        uint32_t identification:16;      /* offset:288 */
        uint32_t header_checksum:16;     /* offset:336 */
        uint32_t protocol:8;             /* offset:328 */
        uint32_t ttl:8;                  /* offset:320 */
        uint32_t sip_low:16;             /* offset:368 */
        uint32_t sip_high:16;            /* offset:352 */
        uint32_t dip_low:16;             /* offset:400 */
        uint32_t dip_high:16;            /* offset:384 */
    } IPv4_Header; /* 20B */
}; /* 52B */


/**************************************************************************
 * Thread THR55_IPv6_LITTLE_ENDIAN_HopLimit_Increment structures
 **************************************************************************/

struct thr55_IPv6_HopLimit_Increment_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr55_IPv6_HopLimit_Increment_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
}; /* 72B */


struct thr55_IPv6_HopLimit_Increment_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
}; /* 72B */


/**************************************************************************
 * Thread THR56_Clear_Outgoing_Mtag_Cmd structures
 **************************************************************************/

struct thr56_Clear_Outgoing_Mtag_Cmd_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr56_Clear_Outgoing_Mtag_Cmd_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
}; /* 72B */


struct thr56_Clear_Outgoing_Mtag_Cmd_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:272 */
        uint32_t flow_label_19_16:4;     /* offset:268 */
        uint32_t ecn:2;                  /* offset:266 */
        uint32_t dscp:6;                 /* offset:260 */
        uint32_t version:4;              /* offset:256 */
        uint32_t hop_limit:8;            /* offset:312 */
        uint32_t next_header:8;          /* offset:304 */
        uint32_t payload_length:16;      /* offset:288 */
        uint32_t sip0_low:16;            /* offset:336 */
        uint32_t sip0_high:16;           /* offset:320 */
        uint32_t sip1_low:16;            /* offset:368 */
        uint32_t sip1_high:16;           /* offset:352 */
        uint32_t sip2_low:16;            /* offset:400 */
        uint32_t sip2_high:16;           /* offset:384 */
        uint32_t sip3_low:16;            /* offset:432 */
        uint32_t sip3_high:16;           /* offset:416 */
        uint32_t dip0_low:16;            /* offset:464 */
        uint32_t dip0_high:16;           /* offset:448 */
        uint32_t dip1_low:16;            /* offset:496 */
        uint32_t dip1_high:16;           /* offset:480 */
        uint32_t dip2_low:16;            /* offset:528 */
        uint32_t dip2_high:16;           /* offset:512 */
        uint32_t dip3_low:16;            /* offset:560 */
        uint32_t dip3_high:16;           /* offset:544 */
    } IPv6_Header; /* 40B */
}; /* 72B */


/**************************************************************************
 * Thread THR57_SFLOW_V5_IPv4 structures
 **************************************************************************/

struct thr57_sflow_v5_Ipv4_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t data_format_header:32;  /* offset:64 */
        uint32_t agent_ip_address:32;    /* offset:96 */
    } sflow_ipv4_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr57_sflow_v5_Ipv4_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t total_length:16;        /* offset:528 */
        uint32_t ecn:2;                  /* offset:526 */
        uint32_t dscp:6;                 /* offset:520 */
        uint32_t ihl:4;                  /* offset:516 */
        uint32_t version:4;              /* offset:512 */
        uint32_t fragment_offset:13;     /* offset:563 */
        uint32_t flag_more_fragment:1;   /* offset:562 */
        uint32_t flag_dont_fragment:1;   /* offset:561 */
        uint32_t flag_reserved:1;        /* offset:560 */
        uint32_t identification:16;      /* offset:544 */
        uint32_t header_checksum:16;     /* offset:592 */
        uint32_t protocol:8;             /* offset:584 */
        uint32_t ttl:8;                  /* offset:576 */
        uint32_t sip_low:16;             /* offset:624 */
        uint32_t sip_high:16;            /* offset:608 */
        uint32_t dip_low:16;             /* offset:656 */
        uint32_t dip_high:16;            /* offset:640 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t DestinationPort:16;     /* offset:688 */
        uint32_t SourcePort:16;          /* offset:672 */
        uint32_t Checksum:16;            /* offset:720 */
        uint32_t Length:16;              /* offset:704 */
    } udp_header; /* 8B */
    struct {
        uint32_t reserved_0:32;          /* offset:736 */
        uint32_t reserved_1:32;          /* offset:768 */
        uint32_t reserved_2:32;          /* offset:800 */
        uint32_t reserved_3:32;          /* offset:832 */
    } template_place_holder; /* 16B */
}; /* 108B */


struct thr57_sflow_v5_Ipv4_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
    } expansion_space; /* 12B */
    struct {
        uint32_t total_length:16;        /* offset:112 */
        uint32_t ecn:2;                  /* offset:110 */
        uint32_t dscp:6;                 /* offset:104 */
        uint32_t ihl:4;                  /* offset:100 */
        uint32_t version:4;              /* offset:96 */
        uint32_t fragment_offset:13;     /* offset:147 */
        uint32_t flag_more_fragment:1;   /* offset:146 */
        uint32_t flag_dont_fragment:1;   /* offset:145 */
        uint32_t flag_reserved:1;        /* offset:144 */
        uint32_t identification:16;      /* offset:128 */
        uint32_t header_checksum:16;     /* offset:176 */
        uint32_t protocol:8;             /* offset:168 */
        uint32_t ttl:8;                  /* offset:160 */
        uint32_t sip_low:16;             /* offset:208 */
        uint32_t sip_high:16;            /* offset:192 */
        uint32_t dip_low:16;             /* offset:240 */
        uint32_t dip_high:16;            /* offset:224 */
    } IPv4_Header; /* 20B */
    struct {
        uint32_t DestinationPort:16;     /* offset:272 */
        uint32_t SourcePort:16;          /* offset:256 */
        uint32_t Checksum:16;            /* offset:304 */
        uint32_t Length:16;              /* offset:288 */
    } udp_header; /* 8B */
    struct {
        uint32_t version:32;             /* offset:320 */
        uint32_t agent_ip_version:32;    /* offset:352 */
        uint32_t agent_ip_address:32;    /* offset:384 */
        uint32_t sub_agent_id:32;        /* offset:416 */
        uint32_t sequence_number:32;     /* offset:448 */
        uint32_t uptime:32;              /* offset:480 */
        uint32_t samples_number:32;      /* offset:512 */
    } sflow_ipv4_header; /* 28B */
    struct {
        uint32_t enterprise_format_number:12;/* offset:564 */
        uint32_t smi_private_enterprise_code:20;/* offset:544 */
        uint32_t sample_length:32;       /* offset:576 */
    } sflow_data_format_header; /* 8B */
    struct {
        uint32_t sample_sequence_number:32;/* offset:608 */
        uint32_t source_id_index_value:30;/* offset:642 */
        uint32_t source_id_type:2;       /* offset:640 */
        uint32_t sampling_rate:32;       /* offset:672 */
        uint32_t sample_pool:32;         /* offset:704 */
        uint32_t drops:32;               /* offset:736 */
        uint32_t input:32;               /* offset:768 */
        uint32_t output:32;              /* offset:800 */
        uint32_t number_of_records:32;   /* offset:832 */
    } sflow_flow_sample_header; /* 32B */
}; /* 108B */


/**************************************************************************
 * Thread THR58_SFLOW_V5_IPv6 structures
 **************************************************************************/

struct thr58_sflow_v5_Ipv6_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t data_format_header:32;  /* offset:64 */
        uint32_t agent_ip_address:32;    /* offset:96 */
    } sflow_ipv4_template; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr58_sflow_v5_Ipv6_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
        uint32_t reserved_8:32;          /* offset:256 */
        uint32_t reserved_9:32;          /* offset:288 */
        uint32_t reserved_10:32;         /* offset:320 */
        uint32_t reserved_11:32;         /* offset:352 */
        uint32_t reserved_12:32;         /* offset:384 */
        uint32_t reserved_13:32;         /* offset:416 */
        uint32_t reserved_14:32;         /* offset:448 */
        uint32_t reserved_15:32;         /* offset:480 */
    } expansion_space; /* 64B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:528 */
        uint32_t flow_label_19_16:4;     /* offset:524 */
        uint32_t ecn:2;                  /* offset:522 */
        uint32_t dscp:6;                 /* offset:516 */
        uint32_t version:4;              /* offset:512 */
        uint32_t hop_limit:8;            /* offset:568 */
        uint32_t next_header:8;          /* offset:560 */
        uint32_t payload_length:16;      /* offset:544 */
        uint32_t sip0_low:16;            /* offset:592 */
        uint32_t sip0_high:16;           /* offset:576 */
        uint32_t sip1_low:16;            /* offset:624 */
        uint32_t sip1_high:16;           /* offset:608 */
        uint32_t sip2_low:16;            /* offset:656 */
        uint32_t sip2_high:16;           /* offset:640 */
        uint32_t sip3_low:16;            /* offset:688 */
        uint32_t sip3_high:16;           /* offset:672 */
        uint32_t dip0_low:16;            /* offset:720 */
        uint32_t dip0_high:16;           /* offset:704 */
        uint32_t dip1_low:16;            /* offset:752 */
        uint32_t dip1_high:16;           /* offset:736 */
        uint32_t dip2_low:16;            /* offset:784 */
        uint32_t dip2_high:16;           /* offset:768 */
        uint32_t dip3_low:16;            /* offset:816 */
        uint32_t dip3_high:16;           /* offset:800 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t DestinationPort:16;     /* offset:848 */
        uint32_t SourcePort:16;          /* offset:832 */
        uint32_t Checksum:16;            /* offset:880 */
        uint32_t Length:16;              /* offset:864 */
    } udp_header; /* 8B */
    struct {
        uint32_t reserved_0:32;          /* offset:896 */
        uint32_t reserved_1:32;          /* offset:928 */
        uint32_t reserved_2:32;          /* offset:960 */
        uint32_t reserved_3:32;          /* offset:992 */
    } template_place_holder; /* 16B */
}; /* 128B */


struct thr58_sflow_v5_Ipv6_out_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
    } expansion_space; /* 12B */
    struct {
        uint32_t flow_label_15_0:16;     /* offset:112 */
        uint32_t flow_label_19_16:4;     /* offset:108 */
        uint32_t ecn:2;                  /* offset:106 */
        uint32_t dscp:6;                 /* offset:100 */
        uint32_t version:4;              /* offset:96 */
        uint32_t hop_limit:8;            /* offset:152 */
        uint32_t next_header:8;          /* offset:144 */
        uint32_t payload_length:16;      /* offset:128 */
        uint32_t sip0_low:16;            /* offset:176 */
        uint32_t sip0_high:16;           /* offset:160 */
        uint32_t sip1_low:16;            /* offset:208 */
        uint32_t sip1_high:16;           /* offset:192 */
        uint32_t sip2_low:16;            /* offset:240 */
        uint32_t sip2_high:16;           /* offset:224 */
        uint32_t sip3_low:16;            /* offset:272 */
        uint32_t sip3_high:16;           /* offset:256 */
        uint32_t dip0_low:16;            /* offset:304 */
        uint32_t dip0_high:16;           /* offset:288 */
        uint32_t dip1_low:16;            /* offset:336 */
        uint32_t dip1_high:16;           /* offset:320 */
        uint32_t dip2_low:16;            /* offset:368 */
        uint32_t dip2_high:16;           /* offset:352 */
        uint32_t dip3_low:16;            /* offset:400 */
        uint32_t dip3_high:16;           /* offset:384 */
    } IPv6_Header; /* 40B */
    struct {
        uint32_t DestinationPort:16;     /* offset:432 */
        uint32_t SourcePort:16;          /* offset:416 */
        uint32_t Checksum:16;            /* offset:464 */
        uint32_t Length:16;              /* offset:448 */
    } udp_header; /* 8B */
    struct {
        uint32_t version:32;             /* offset:480 */
        uint32_t agent_ip_version:32;    /* offset:512 */
        uint32_t agent_ip_address:32;    /* offset:544 */
        uint32_t sub_agent_id:32;        /* offset:576 */
        uint32_t sequence_number:32;     /* offset:608 */
        uint32_t uptime:32;              /* offset:640 */
        uint32_t samples_number:32;      /* offset:672 */
    } sflow_ipv4_header; /* 28B */
    struct {
        uint32_t enterprise_format_number:12;/* offset:724 */
        uint32_t smi_private_enterprise_code:20;/* offset:704 */
        uint32_t sample_length:32;       /* offset:736 */
    } sflow_data_format_header; /* 8B */
    struct {
        uint32_t sample_sequence_number:32;/* offset:768 */
        uint32_t source_id_index_value:30;/* offset:802 */
        uint32_t source_id_type:2;       /* offset:800 */
        uint32_t sampling_rate:32;       /* offset:832 */
        uint32_t sample_pool:32;         /* offset:864 */
        uint32_t drops:32;               /* offset:896 */
        uint32_t input:32;               /* offset:928 */
        uint32_t output:32;              /* offset:960 */
        uint32_t number_of_records:32;   /* offset:992 */
    } sflow_flow_sample_header; /* 32B */
}; /* 128B */


/**************************************************************************
 * Thread THR60_DropAllTraffic structures
 **************************************************************************/

struct thr60_DropAllTraffic_cfg {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
    } HA_Table_reserved_space; /* 16B */
    struct {
        uint32_t targetPortEntry:16;     /* offset:144 */
        uint32_t srcPortEntry:16;        /* offset:128 */
    } srcTrgPortEntry; /* 4B */
}; /* 20B */


struct thr60_DropAllTraffic_in_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:272 */
        uint32_t mac_da_47_32:16;        /* offset:256 */
        uint32_t mac_sa_47_32:16;        /* offset:304 */
        uint32_t mac_da_15_0:16;         /* offset:288 */
        uint32_t mac_sa_15_0:16;         /* offset:336 */
        uint32_t mac_sa_31_16:16;        /* offset:320 */
    } mac_header; /* 12B */
}; /* 44B */


struct thr60_DropAllTraffic_hdr {
    struct {
        uint32_t reserved_0:32;          /* offset:0 */
        uint32_t reserved_1:32;          /* offset:32 */
        uint32_t reserved_2:32;          /* offset:64 */
        uint32_t reserved_3:32;          /* offset:96 */
        uint32_t reserved_4:32;          /* offset:128 */
        uint32_t reserved_5:32;          /* offset:160 */
        uint32_t reserved_6:32;          /* offset:192 */
        uint32_t reserved_7:32;          /* offset:224 */
    } expansion_space; /* 32B */
    struct {
        uint32_t mac_da_31_16:16;        /* offset:272 */
        uint32_t mac_da_47_32:16;        /* offset:256 */
        uint32_t mac_sa_47_32:16;        /* offset:304 */
        uint32_t mac_da_15_0:16;         /* offset:288 */
        uint32_t mac_sa_15_0:16;         /* offset:336 */
        uint32_t mac_sa_31_16:16;        /* offset:320 */
    } mac_header; /* 12B */
}; /* 44B */

#endif /* __ppa_fw_threads_strc_defs_h */
