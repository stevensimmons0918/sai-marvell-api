/*
 *  ppa_fw_defs.h
 *
 *  ppa fw definitions
 *
 */

#ifndef PPA_FW_DEFS_H
#define PPA_FW_DEFS_H

#ifdef ASIC_SIMULATION
    #include "wm_asic_sim_defs.h"
    #define inline /* empty */
    #ifndef __MINGW64__
        #define __attribute__(x) /*empty*/
    #endif /*__MINGW64__*/
#endif

#define INLINE inline

/**************************************************************************
 * Function prototypes & Externs
 * ************************************************************************/
INLINE void packet_swap_sequence_for_profiler();
INLINE void packet_swap_sequence();
int main();
void pipe_init();
void set_ePort_params();
void exception_power_up();
void get_fw_version ();
void init_shared_data_memory();
void stack_overflow_checker_power_up();
void stack_overflow_checker();

/**************************************************************************
 * PPN ID address in SP
 **************************************************************************/
#define PPN_ID_ADDR (0x8)

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
    #define PPA_FW_SHARED_DRAM_WRITE(addr, data)        PPA_FW_SP_WRITE(DRAM_BASE_ADDR + addr, data)
    #define PPA_FW_SHARED_DRAM_READ(addr)               PPA_FW_SP_READ(DRAM_BASE_ADDR + addr)
    #define PPA_FW_SHARED_DRAM_SHORT_WRITE(addr, data)  PPA_FW_SP_SHORT_WRITE(DRAM_BASE_ADDR + addr, data)
    #define PPA_FW_SHARED_DRAM_SHORT_READ(addr)         PPA_FW_SP_SHORT_READ(DRAM_BASE_ADDR + addr)

    #define PIPE_MEM_CAST(addr,structCast)          (struct structCast *) pipeMemCast_##structCast(addr)

    #include <asicSimulation/SLog/simLog.h>
    extern SKERNEL_DEVICE_OBJECT * current_fw_devObjPtr;

    #define __LOG_FW(x)    {devObjPtr = current_fw_devObjPtr;   \
                            __LOG_NO_LOCATION_META_DATA(("FW_LOG:"));\
                            __LOG_NO_LOCATION_META_DATA(x)}
    #define __LOG_PARAM_FW(x) {devObjPtr = current_fw_devObjPtr;   \
                            __LOG_NO_LOCATION_META_DATA(("FW_LOG:"));\
                            __LOG_PARAM_NO_LOCATION_META_DATA(x)}

#else
    #define PPA_FW_SP_WRITE(addr, data)             (*((uint32_t*)(addr))) = (data)
    #define PPA_FW_SP_READ(addr)                    (*((uint32_t*)(addr)))
    #define PPA_FW_SP_SHORT_WRITE(addr, data)       (*((uint16_t*)(addr))) = ((uint16_t)(data))
    #define PPA_FW_SP_SHORT_READ(addr)              (*((uint16_t*)(addr)))
    #define PPA_FW_SP_BYTE_WRITE(addr, data)        (*((uint8_t*)(addr))) = ((uint8_t)(data))
    #define PPA_FW_SP_BYTE_READ(addr)               (*((uint8_t*)(addr)))
    #define PPA_FW_SP_OFFSET_READ(addr, off)        (*((uint32_t*)(addr) + off))
    #define PPA_FW_SP_OFFSET_WRITE(addr, off, data) (*((uint32_t*)(addr) + off)) = (data)
    #define PPA_FW_SHARED_DRAM_WRITE(addr, data)        PPA_FW_SP_WRITE(DRAM_BASE_ADDR + addr, data)
    #define PPA_FW_SHARED_DRAM_READ(addr)               PPA_FW_SP_READ(DRAM_BASE_ADDR + addr)
    #define PPA_FW_SHARED_DRAM_SHORT_WRITE(addr, data)  PPA_FW_SP_SHORT_WRITE(DRAM_BASE_ADDR + addr, data)
    #define PPA_FW_SHARED_DRAM_SHORT_READ(addr)         PPA_FW_SP_SHORT_READ(DRAM_BASE_ADDR + addr)

    #define PIPE_MEM_CAST(addr,structCast)          (struct structCast *)(addr)

    #define __LOG_FW(x)         /*empty*/
    #define __LOG_PARAM_FW(x)   /*empty*/
#endif

/**************************************************************************
 * PPA constant parameters SP addresses
 *************************************************************************/
#define MIN_E_PORT_MINUS_1_SP_ADDR    0xA0       /* (14 bits) */
#define MAX_E_PORT_SP_ADDR            0xA4       /* (14 bits) */
#define MULTICAST_OFFSET_SP_ADDR      0xA8       /* (14 bits) */
#define MIN_E_PORT_SP_ADDR            0xAC       /* (14 bits) */

/* FW version - 8 bytes */
struct pipe_fw_version {
        char            name[4];     /* Name of project  */
        unsigned char        major_x;     /* Represents year  */
        unsigned char        minor_y;     /* Represents month */
        unsigned char        local_z;     /* Represents version number   */
        unsigned char        debug_d;     /* Used for private versions, should be zero for official version */
};

#define PIPE_FW_VERSION_ADDR  (0x0)
#define VER_MAJOR   (0x20)   /* Year 2020 */
#define VER_MINOR	(0x07)   /* Month July */
#define VER_LOCAL	(0x00)   /* Number of version within a month */
#define VER_DEBUG   (0x00)   /* Used for private or debug versions, should be zero for official version */

#define VER_CHAR_0        ('P')
#define VER_CHAR_1        ('I')
#define VER_CHAR_2        ('P')
#define VER_CHAR_3        ('E')

/**************************************************************************
 * Special instructions Registers addresses
 * ************************************************************************/
#define PKT_SWAP_INST_ADDR    0x04000
#define PKT_LOAD_PC_INST_ADDR 0x04004

/**************************************************************************
 * SHARED DRAM memory definitions
 * ************************************************************************/
#define DRAM_BASE_ADDR       0x00010000
#define DRAM_INIT_DONE_OFST  0x0
#define DRAM_INIT_DONE_VALUE 0x900d900d
/* Used in THR25_EVB_QCN thread. Each entry in this table should hold 16 bits of VLAN tag {UP[2:0],CFI,VID[11:0]}
   The table is 17 entries (ports 0-16), thus total 17x16b = 272b  */
#define DRAM_QCN_VLAN_TBL_OFST  (0x100)

/**************************************************************************
 * Packet Register File: Base addresses
 * ************************************************************************/
#define PIPE_CFG_REGs_lo  0x2000
#define PIPE_DESC_REGs_lo 0x2100
#define PIPE_PKT_REGs_lo  0x2200

/**************************************************************************
 * PPN I/O Registers addresses
 **************************************************************************/
#define PPN_IO_REG_PPN_STATUS_ADDR              0x03000
#define PPN_IO_REG_PPN_PKT_IO_STATUS_ADDR       0x03004
#define PPN_IO_REG_PPN_SHARED_DMEM_STATUS_ADDR  0x03008
#define PPN_IO_REG_PPN_DOORBELL_ADDR            0x03010
#define PPN_IO_REG_PPN_SCENARIO_FIRST_PC_ADDR   0x03020
#define PPN_IO_REG_PPN_PKT_HDR_OFST_ADDR        0x03024
#define PPN_IO_REG_PPN_INTERNAL_ERR_CAUSE_ADDR  0x03040
#define PPN_IO_REG_PPN_INTERNAL_ERR_MASK_ADDR   0x03044

/**************************************************************************
 * PPN Internal Error Mask bits
 **************************************************************************/
#define PPN_ERR_UNMASK_PPN_DOORBELL_INT           (0x10) /* Bit 4 */
#define PPN_ERR_UNMASK_NEAR_EDGE_IMEM_ACCESS_INT  (0x8)  /* Bit 3 */
#define PPN_ERR_UNMASK_CORE_MEMORY_ERR            (0x4)  /* Bit 2 */
#define PPN_ERR_UNMASK_HOST_UNMAPPED_ACCESS_ERR   (0x2)  /* Bit 1 */

/**************************************************************************
 * General PIPE constant definitions
 * ************************************************************************/
/* Points to descriptor fields is_ptp(1bit) and mark_ecn(1bit) */
#define SLOW_PATH_INDICATION_OFFSET 0x4
/* Size of expansion space in packet input in bytes units */
#define EXPANSION_SPACE_SIZE    20

/* PTP Transport type */
#define ETHERNET_TRANSPORT_TYPE (0x88F7)
#define IPV4_TRANSPORT_TYPE     (0x0800)
#define IPV6_TRANSPORT_TYPE     (0x86DD)

/* Points to source port field PtpPortMode - offset into configuration space */
#define PTP_SOURCE_PORT_MODE_OFFSET     19
#define PTP_SOURCE_PORT_MODE_MASK       0x03
#define PTP_OVER_MPLS_EN_OFFSET         2
#define PTP_OVER_MPLS_EN_MASK           0x01

/* PTPPortMode values */
#define PTP_SOURCE_PORT_MODE_TC             0
#define PTP_SOURCE_PORT_MODE_OC_BC_INGRESS  1
#define PTP_SOURCE_PORT_MODE_OC_BC_EGRESS   2
#define PTP_OC_BC_EGRESS_TIMESTAMP_IN_CF    3

/* L3 defines */
#define IPv4_VER 4
#define IPv6_VER 6

/* Correction Field offset */
#define CF_OFST 8
/* Ethernet type field size in bytes */
#define ETHERNET_SIZE 2
/* UDP checksum field size in bytes */
#define UDP_CS_SIZE   2
/* MAC DA SA size in bytes */
#define MAC_ADDR_SIZE   12


/* slow path indication definitions */
#define ECN_INDICATION_IS_ON(slowPathInd)        (0x40 == (slowPathInd&0x40))
#define PTP_INDICATION_IS_ON(slowPathInd)        (0x80 == (slowPathInd&0x80))

/* Maximium offset in bytes for udp checksum trailer (last two bytes of packet) which is no more than 8bits size */
#define UDP_CS_TRAILER_MAX_OFFSET        (255)

/* Remove or add bytes according to user configuration. Used in debug thread number 28. */
#define ADD_BYTES_CASE            (1)
#define REMOVE_BYTES_CASE        (0)

/* Useful constant for converting sec+nanosec <-> nanosec */
#define THOUSAND    (1000)
#define MILLION     (THOUSAND*THOUSAND)
#define BILLION     (MILLION*THOUSAND)
#define HALF_BILLION     (BILLION/2)



#endif /* PPA_FW_DEFS_H */
