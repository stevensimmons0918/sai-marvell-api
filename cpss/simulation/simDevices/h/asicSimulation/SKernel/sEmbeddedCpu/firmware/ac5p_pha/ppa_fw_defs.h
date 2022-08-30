/*
 *  ppa_fw_defs.h
 *
 *  ppa fw definitions
 *
 */

#ifndef __ppa_fw_defs_h
#define __ppa_fw_defs_h

#ifdef ASIC_SIMULATION
    #define inline /* empty */
#ifndef __MINGW64__
    #define __attribute__(x) /*empty*/
#endif /*__MINGW64__*/
#endif

#define INLINE inline

/**************************************************************************
 * Function prototypes & Externs
 * ************************************************************************/
int main();
void ac5p_init();
void ac5p_exception_power_up();
void ac5p_init_shared_data_memory();
void ac5p_get_fw_version (uint32_t imageId, uint32_t fwVersion);
void ac5p_queue_length_encoder_init ();
INLINE void packet_swap_sequence();
void ac5p_sFlowTablesInit ();
void ac5p_ppn2mg_powerup();
void ppn2mg_failure_event(uint32_t event_id, uint32_t* err_params_ptr ,uint32_t num_of_params, uint32_t stop_ppn);


#ifndef ASIC_SIMULATION
void call_threads();
void ac5p_stack_overflow_checker_power_up();
void ac5p_stack_overflow_checker();
#endif




/**************************************************************************
 * Pragma definitions. Real meaning in Tensilica build. Empty on CPSS side. 
 **************************************************************************/
#ifdef ASIC_SIMULATION
    #define __PRAGMA_NO_REORDER               /*empty*/
    #define __PRAGMA_FLUSH_MEMORY             /*empty*/
    #define __PRAGMA_FREQUENCY_HINT_NEVER     /*empty*/
    #define __PRAGMA_FREQUENCY_HINT_FREQUENT  /*empty*/
#else
    #define __PRAGMA_NO_REORDER               pragma_no_reorder();
    #define __PRAGMA_FLUSH_MEMORY             pragma_flush_memory();
    #define __PRAGMA_FREQUENCY_HINT_NEVER     pragma_frequency_hint_never();
    #define __PRAGMA_FREQUENCY_HINT_FREQUENT  pragma_frequency_hint_frequent();
#endif


/**************************************************************************
 * Accelerator Instruction: Base addresses
 **************************************************************************/
#define ACCEL_CMDS_CFG_BASE_ADDR 0x40
#define ACCEL_CMDS_ALIAS_BASE_ADDR 0x1000
#define ACCEL_CMDS_TRIG_BASE_ADDR (ACCEL_CMDS_ALIAS_BASE_ADDR + ACCEL_CMDS_CFG_BASE_ADDR)


/**************************************************************************
 * PPN ID address in SP
 **************************************************************************/
#define PPN_ID_ADDR (0xC)

/**************************************************************************
 * Stack overflow check definitions
 **************************************************************************/
#define CHECK_STACK         (0)
#define STACK_INITIAL_VAL   (0x900dCAFE)

/**************************************************************************
 * PPA READ & WRITE macros
 **************************************************************************/
#ifdef ASIC_SIMULATION
    #define PPA_FW_SP_WRITE(addr, data)                 ppaFwSpWrite(addr, data)
    #define PPA_FW_SP_READ(addr)                        ppaFwSpRead(addr)
    #define PPA_FW_SP_SHORT_WRITE(addr, data)           ppaFwSpShortWrite(addr, data)
    #define PPA_FW_SP_SHORT_READ(addr)                  ppaFwSpShortRead(addr)
    #define PPA_FW_SP_BYTE_WRITE(addr, data)            ppaFwSpByteWrite(addr, data)
    #define PPA_FW_SP_BYTE_READ(addr)                   ppaFwSpByteRead(addr)
    #define PPA_FW_SP_OFFSET_READ(addr, off)            PPA_FW_SP_READ  (addr + (4*off))
    #define PPA_FW_SP_OFFSET_WRITE(addr, off, data)     PPA_FW_SP_WRITE((addr + (4*off)), data)
    #define PPA_FW_SHARED_DRAM_WRITE(addr, data)        PPA_FW_SP_WRITE(addr, data)
    #define PPA_FW_SHARED_DRAM_READ(addr)               PPA_FW_SP_READ(addr)
    #define PPA_FW_SHARED_DRAM_SHORT_WRITE(addr, data)  PPA_FW_SP_SHORT_WRITE(addr, data)
    #define PPA_FW_SHARED_DRAM_SHORT_READ(addr)         PPA_FW_SP_SHORT_READ(addr)

    #define AC5P_MEM_CAST(addr,structCast)          (struct structCast *) ac5pMemCast_##structCast(addr)

    #include <asicSimulation/SLog/simLog.h>
    extern SKERNEL_DEVICE_OBJECT * current_fw_devObjPtr;
    #define __LOG_FW(x)    {devObjPtr = current_fw_devObjPtr;   \
                            __LOG_NO_LOCATION_META_DATA(("FW_LOG:"));\
                            __LOG_NO_LOCATION_META_DATA(x)}
    #define __LOG_PARAM_FW(x) {devObjPtr = current_fw_devObjPtr;   \
                            __LOG_NO_LOCATION_META_DATA(("FW_LOG:"));\
                            __LOG_PARAM_NO_LOCATION_META_DATA(x)}

#else
    #define PPA_FW_SP_WRITE(addr, data)                 (*((uint32_t*)(addr))) = (data)
    #define PPA_FW_SP_READ(addr)                        (*((uint32_t*)(addr)))
    #define PPA_FW_SP_SHORT_WRITE(addr, data)           (*((uint16_t*)(addr))) = ((uint16_t)(data))
    #define PPA_FW_SP_SHORT_READ(addr)                  (*((uint16_t*)(addr)))
    #define PPA_FW_SP_BYTE_WRITE(addr, data)            (*((uint8_t*)(addr))) = ((uint8_t)(data))
    #define PPA_FW_SP_BYTE_READ(addr)                   (*((uint8_t*)(addr)))
    #define PPA_FW_SP_OFFSET_READ(addr, off)            (*((uint32_t*)(addr) + off))
    #define PPA_FW_SP_OFFSET_WRITE(addr, off, data)     (*((uint32_t*)(addr) + off)) = (data)
    #define PPA_FW_SHARED_DRAM_WRITE(addr, data)        PPA_FW_SP_WRITE(addr, data)
    #define PPA_FW_SHARED_DRAM_READ(addr)               PPA_FW_SP_READ(addr)
    #define PPA_FW_SHARED_DRAM_SHORT_WRITE(addr, data)  PPA_FW_SP_SHORT_WRITE(addr, data)
    #define PPA_FW_SHARED_DRAM_SHORT_READ(addr)         PPA_FW_SP_SHORT_READ(addr)

    #define AC5P_MEM_CAST(addr,structCast)          (struct structCast *)(addr)

    #define __LOG_FW(x)         /*empty*/
    #define __LOG_PARAM_FW(x)   /*empty*/

#endif

/**************************************************************************
 * PPA constant parameters SP addresses
 **************************************************************************/
#define AC5P_FW_VERSION_ADDR  (0x0)
#define AC5P_FW_IMAGE_ID_ADDR  (0x8)

/**************************************************************************
 * Special instructions Registers addresses
 * ************************************************************************/
#define PKT_SWAP_INST_ADDR    0x04000
#define PKT_LOAD_PC_INST_ADDR 0x04004

/**************************************************************************
 * SHARED DRAM memory definitions
 * ************************************************************************/
#define DRAM_BASE_ADDR       0x00010000
#define DRAM_SIZE            0x2000
#define DRAM_INIT_DONE_ADDR  DRAM_BASE_ADDR + (DRAM_SIZE - 4) /*last 4 bytes*/
#define DRAM_INIT_DONE_VALUE 0x900d900d
#define DRAM_ENC_Q_LEN_VALS_TABLE_ADDR  DRAM_BASE_ADDR /* currently not in use */


/* Memory locations to hold CC ERSPAN template parameters for each analyzer index (1:7)
   Template includes: for IPv4 Word 0:4     for IPv6 Word 0:9    IPv4 or IPv6
                               Word 5                Word 10     contains session ID    => | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |  
                               Word 6:9              Word 11:14  contains L2 parameters => | MAC DA SA | TPID | UP | CFI | VID |
                               Word 10:15            Word 15     reserved
*/
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE 64

#define DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR  DRAM_BASE_ADDR /* 0x10000 */
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE 
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_3_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_4_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_3_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_5_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_4_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_6_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_5_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE
#define DRAM_CC_ERSPAN_ANALYZER_EPORT_7_TEMPLATE_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_6_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE

/* Global area to hold Falcon device ID and VoQ thread indication 
   |      device ID | VoQ  | reserved |
    31......18 17 16      8          0
*/
#define DRAM_CC_ERSPAN_GLOBAL_ADDR  DRAM_CC_ERSPAN_ANALYZER_EPORT_7_TEMPLATE_ADDR + DRAM_CC_ERSPAN_ANALYZER_EPORT_TEMPLATE_SIZE


/* Table of 128 entries 4B each. Entry per source port.
   Contains the sFlow packet sequence number for the associated source port.
   The sequence number is incremented by the PHA firmware for every sFlow packet from this source port.
*/
#define DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR  (DRAM_CC_ERSPAN_GLOBAL_ADDR + 4) /* 0x000101C4 */
#define DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_SIZE  (128*4) /*128 entries * 4 bytes*/                         

/* Table of 128 entries 2B each. Entry per source port.
   Contains the sFlow packet drops number for the associated source port.
   The drops number is incremented by the PHA firmware for every sFlow packet Hard/Soft drop from this source port.
*/
#define DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR  (DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR + DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_SIZE) /* 0x000103C4 */
#define DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_SIZE  (128*2) /*128 entries * 2 bytes*/

/* Table of 6 entries 4B each. Entry per target analyzer ID.
   Contains sequence number counters for sFlow packets.
   The counter is incremented by PHA firmware for every sFlow packet goes through certain sFlow threads.
*/
#define DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR  (DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR + DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_SIZE) /* 0x000104C4 */
#define DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_SIZE  (6*4) /*6 entries * 4 bytes*/
        

/**************************************************************************
 * Packet Register File: Base addresses
 * ************************************************************************/
#define AC5P_CFG_REGs_lo      0x2000
#define AC5P_SPECIAL_REGs_lo  0x2020
#define AC5P_DESC_REGs_lo     0x2100
#define AC5P_PKT_REGs_lo      0x2200

/**************************************************************************
 * PPN I/O Registers addresses
 **************************************************************************/
#define PPN_IO_REG_STATUS_ADDR              0x03000
#define PPN_IO_REG_PKT_IO_STATUS_ADDR       0x03004
#define PPN_IO_REG_SHARED_DMEM_STATUS_ADDR  0x03008
#define PPN_IO_REG_DOORBELL_ADDR            0x03010
#define PPN_IO_REG_SCENARIO_FIRST_PC_ADDR   0x03020
#define PPN_IO_REG_PKT_HDR_OFST_ADDR        0x03024
#define PPN_IO_REG_INTERNAL_ERR_CAUSE_ADDR  0x03040
#define PPN_IO_REG_INTERNAL_ERR_MASK_ADDR   0x03044
#define PPN_IO_REG_TOD_0_WORD_0_ADDR        0x03050
#define PPN_IO_REG_TOD_0_WORD_1_ADDR        0x03054
#define PPN_IO_REG_TOD_0_WORD_2_ADDR        0x03058
#define PPN_IO_REG_TOD_1_WORD_0_ADDR        0x03060
#define PPN_IO_REG_TOD_1_WORD_1_ADDR        0x03064
#define PPN_IO_REG_TOD_1_WORD_2_ADDR        0x03068

/* Macro to read TOD IF registers */
#define TOD_WORD_0  (0)    /* Represent Nano seconds */
#define TOD_WORD_1  (1)    /* Represent Seconds bits [31:0] */
#define TOD_WORD_2  (2)    /* Represent Seconds bits [47:32] place in bits 31:16, u bit place in bit 15 */
#define READ_TOD_IF_REG(word_num, tai_sel)  PPA_FW_SP_READ( PPN_IO_REG_TOD_0_WORD_0_ADDR + word_num*4 + tai_sel*0x10 )
#define WRITE_TOD_IF_REG(word_num, tai_sel,value)   PPA_FW_SP_WRITE( PPN_IO_REG_TOD_0_WORD_0_ADDR + word_num*4 + tai_sel*0x10, value )



/**************************************************************************
 * PPN Internal Error Mask bits
 **************************************************************************/
#define PPN_ERR_UNMASK_PPN_DOORBELL_INT           (0x10) /* Bit 4 */
#define PPN_ERR_UNMASK_NEAR_EDGE_IMEM_ACCESS_INT  (0x8)  /* Bit 3 */
#define PPN_ERR_UNMASK_CORE_MEMORY_ERR            (0x4)  /* Bit 2 */
#define PPN_ERR_UNMASK_HOST_UNMAPPED_ACCESS_ERR   (0x2)  /* Bit 1 */

/**************************************************************************
 * General AC5P constant definitions
 * ************************************************************************/
/* IP header size in bytes (not including the extension headers and options)   */
#define IPV6_BASIC_HEADER_SIZE    (40)
#define IPV4_BASIC_HEADER_SIZE    (20)
/* GRE basic header size in bytes (not including the optional cases) */
#define GRE_BASIC_HEADER_SIZE    (4)
/* Segment Routing header size in bytes (not including the Segment lists)   */
#define SR_BASIC_HEADER_SIZE      (8)
/* Segment list size in bytes */
#define SEGMENT_LIST_SIZE     (16)
/* Packet command allocate4d by FW set to Hard drop */
#define HARD_DROP     (3)
/* 8 bytes size */
#define EIGHT_BYTES   (8)
/* Segment left field offset in SR header */
#define SEGMENT_LEFT_OFST_IN_SR_HDR (3)
/* UDP header size in bytes */
#define UDP_HDR_SIZE    (8)
/* VXLAN header size in bytes */
#define VXLAN_HDR_SIZE  (8)
/* INT header size in bytes */
#define INT_HDR_SIZE    (8)
/* IOAM header size in bytes */
#define IOAM_TRACE_TYPE_HDR_SIZE    (8)
/* UDP LENGTH FIELD from UDP header (2Bytes) */
#define SIZE_OF_UDP_LENGTH_FIELD    (2)

/* INT MD size in bytes */
#define INT_MD_SIZE (4)
/* Size of expansion space for INT use cases in bytes units */
#define INT_EXPANSION_SPACE_SIZE     (64)
/* Size of expansion space for IOAM use cases in bytes units */
#define IOAM_EXPANSION_SPACE_SIZE    (64)
/* Size of expansion space for IOAM Egress switch use cases in bytes units */
#define IOAM_EGRESS_EXPANSION_SPACE_SIZE    (32)
/* Size of expansion space for IPv6 SR use cases in bytes units */
#define IPV6_SR_EXPANSION_SPACE_SIZE (32)
/* Offset in bytes from IP to UDP length */
#define IPV6_TO_UDP_LENGTH_OFFSET (IPV6_BASIC_HEADER_SIZE + 4)
#define IPV4_TO_UDP_LENGTH_OFFSET (IPV4_BASIC_HEADER_SIZE + 4)
/* Offset in bytes from IP to UDP checksum */
#define IPV6_TO_UDP_CS_OFFSET (IPV6_BASIC_HEADER_SIZE + 6)
#define IPV4_TO_UDP_CS_OFFSET (IPV4_BASIC_HEADER_SIZE + 6)
/* Offset in bytes from IP to INT header */
#define IPV6_TO_INT_HDR_OFFSET (IPV6_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE)
#define IPV4_TO_INT_HDR_OFFSET (IPV4_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE)
/* Offset in bytes from IP to INT META DATA */
#define IPV6_TO_INT_MD_OFFSET (IPV6_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE + INT_HDR_SIZE)
#define IPV4_TO_INT_MD_OFFSET (IPV4_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE + INT_HDR_SIZE)
/* Offset in bytes from IP to IOAM header */
#define IPV4_TO_IOAM_HDR_OFFSET (IPV4_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE)
/* Offset in bytes from IP to IOAM META DATA */
#define IPV4_TO_IOAM_TRACE_MD_OFFSET (IPV4_BASIC_HEADER_SIZE + UDP_HDR_SIZE + VXLAN_HDR_SIZE + IOAM_TRACE_TYPE_HDR_SIZE)
/* Packet maximum size should be 128 bytes in IPv6 SR use cases */
#define IPv6_SR_PKT_MAX_LENGTH  (128)
/* Packet maximum size should be 96 bytes in IOAM use cases */
#define IOAM_PKT_MAX_LENGTH (96)
/* Offsets in bytes of UDP fields. Offsets are from beginning of UDP header */
#define UDP_LENGTH_OFFSET (4)
#define UDP_CS_OFFSET     (6)


/* PTP Transport type */
#define ETHERNET_TRANSPORT_TYPE (0x88F7)
#define IPV4_TRANSPORT_TYPE     (0x0800)
#define IPV6_TRANSPORT_TYPE     (0x86DD)

/* L3 defines */
#define IPv4_VER (4)
#define IPv6_VER (6)

/* Correction Field offset */
#define CF_OFST (8)
/* Ethernet type field size in bytes */
#define ETHER_TYPE_SIZE (2)
/* UDP checksum field size in bytes */
#define UDP_CS_SIZE   (2)

/* Maximum offset in bytes for udp checksum trailer (last two bytes of packet) which is no more than 8bits size */
#define UDP_CS_TRAILER_MAX_OFFSET   (255)

/* Remove or add bytes according to user configuration. Used in debug thread number 28. */
#define ADD_BYTES_CASE      (1)
#define REMOVE_BYTES_CASE   (0)


/* Macro definitions to concatenate strings to each other */
#define GLUE4(a,b,c,d)       a##b##c##d
#define GLUEWRAPPER(a,b,c,d)   GLUE4(a,b,c,d)

#define RUN(name, routine)  GLUEWRAPPER(Falcon,name,_,routine)
#define PPA_FW(routine)   GLUEWRAPPER(TARGET_DEVICE,FW_IMAGE_NAME,_,routine)

#define CALL(thrdFunc, isEnable)   \
if (isEnable)                      \
    RUN(FW_IMAGE_NAME, thrdFunc)();  


/* PTP action field options */
enum desc_PTP_Action {
    PTP_ACTION_NONE_E = 0,                        /* 0x0 */
    PTP_ACTION_FORWARD_E,                         /* 0x1 */
    PTP_ACTION_DROP_E,                            /* 0x2 */
    PTP_ACTION_CAPTURE_E,                         /* 0x3 */
    PTP_ACTION_ADD_TIME_E,                        /* 0x4 */
    PTP_ACTION_ADD_CORRECTION_TIME_E,             /* 0x5 */
    PTP_ACTION_CAPTURE_ADD_TIME_E,                /* 0x6 */
    PTP_ACTION_CAPTURE_ADD_CORRECTION_TIME_E,     /* 0x7 */
    PTP_ACTION_ADD_INGRESS_TIME_E,                /* 0x8 */
    PTP_ACTION_CAPTURE_ADD_INGRESS_TIME_E,        /* 0x9 */
    PTP_ACTION_CAPTURE_INGRESS_TIME_E,            /* 0xA */
    PTP_ACTION_RESERVED_E,                        /* 0xB */
    PTP_ACTION_CAPTURE_PCH_E                      /* 0xC */
};

/* Indicates the packet format for timestamping purposes */
enum desc_PTP_Packet_Format {
    PTP_PKT_FORMAT_PTP_V2_E = 0,                /* 0x0   Compatible to IEEE 1588 v2. */
    PTP_PKT_FORMAT_PTP_V1_E,                    /* 0x1   Compatible to IEEE 1588 v1. */
    PTP_PKT_FORMAT_Y1731_E,                     /* 0x2   Compatible to ITU-T Y.1731. */
    PTP_PKT_FORMAT_NTP_TS_E,                    /* 0x3   Compatible to NTP timestamp. */
    PTP_PKT_FORMAT_NTP_RECEIVE_E,               /* 0x4   Compatible to NTP timestamp, and the timestamp is placed in the <Receive Timestamp> field */
    PTP_PKT_FORMAT_NTP_TRANSMIT_E,              /* 0x5   Compatible to NTP timestamp, and the timestamp is placed in the <Transmit Timestamp> field */
    PTP_PKT_FORMAT_WAMP_E,                      /* 0x6   Compatible to OWAMP / TWAMP packet format */
    PTP_PKT_FORMAT_RESERVED_E                   /* 0x7   reserved */
};

/* Operation of ECN (Explicit Congestion Notification) with IP */
enum ip_ecn_operation {
    IP_NON_ECN = 0,                           /* 0x0   Non ECN-Capable Transport, Non-ECT */
    IP_ECN_ECT_0,                             /* 0x1   ECN Capable Transport, ECT(0) */
    IP_ECN_ECT_1,                             /* 0x2   ECN Capable Transport, ECT(1) */
    IP_ECN_CE                                 /* 0x3   Congestion Encountered, CE. */
};



#endif /*__ppa_fw_defs_h*/
