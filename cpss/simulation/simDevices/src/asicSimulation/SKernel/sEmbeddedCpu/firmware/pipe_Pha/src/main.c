/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * main.c
 *
 *  Main function call pipe power up sequence
 *
 */

#ifdef ASIC_SIMULATION
#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
    #pragma warning(disable: 4244) /* conversion from 'uint32_t' to 'uint16_t', possible loss of data */
#endif /*_VISUALC*/
#endif /*ASIC_SIMULATION*/

/* ==========================
  Include headers section
  =========================== */
#include "ppa_fw_defs.h"
#include "ppa_fw_base_types.h"
#include "ppa_accelerator_commands.h"
#include "ppa_threads_fw_defs.h"
#include "ppa_fw_exception.h"
#include "ppa_fw_logging.h"
#include "ppa_fw_record.h"
#include "ppa_fw_ppn2mg.h"
#if ENABLE_RECORDING
#include "ppa_fw_record.c"
#endif


#ifdef ASIC_SIMULATION
    #include "wm_asic_sim_defs.h"
    static SKERNEL_DEVICE_OBJECT * devObjPtr;
    #undef NULL
    #define NULL 0
#else /*ASIC_SIMULATION*/
    #include <xtensa/hal.h>
    #include <xtensa/xtruntime.h>
    #include <xtensa/sim.h>
#endif /*!ASIC_SIMULATION*/

/* ===========================
   Global variables section
   =========================== */
uint32_t exception_dump_buf[XTENSA_REG_NUM];
/* Pointer and index to PPN2MG events buffer. It holds 8 entries and is cyclic therefore always holds the last 8 events */
uint32_t* ppn2mg_events_buf_ptr;
uint32_t ppn2mg_events_buf_indx;
/* Buffer in SP to hold the error parameters of the first failure event */
uint32_t ppn_err_params_buf[PPN_ERR_BUF_NUM_OF_ENTRIES];
uint32_t* ppn_err_params_buf_ptr;


/********************************************************************************************************************//**
 * funcname      ppn2mg_powerup
 * inparam       None
 * return        None
 * description   Power up sequence of PPN to MG IF.
 *               Un mask PPN error events towards MG.
 *               Initialize pointer and index to events buffer.
 *               Set first entry of error buffer with reset value.
 ************************************************************************************************************************/
void ppn2mg_powerup()
{
    /* Un mask PPN error events towards host: doorbell, near edge imem access, core memory error and unmapped host access error   */
    /* Should be done by MG (CPSS)
    PPA_FW_SP_WRITE(PPN_IO_REG_PPN_INTERNAL_ERR_MASK_ADDR, (PPN_ERR_UNMASK_PPN_DOORBELL_INT          | \
                                                            PPN_ERR_UNMASK_NEAR_EDGE_IMEM_ACCESS_INT | \
                                                            PPN_ERR_UNMASK_CORE_MEMORY_ERR           | \
                                                            PPN_ERR_UNMASK_HOST_UNMAPPED_ACCESS_ERR) );
     */

    /* Clear index    */
    ppn2mg_events_buf_indx = 0;

    /* Set pointer to ppn2mg events buffer    */
    ppn2mg_events_buf_ptr = (uint32_t*)(PPN2MG_EVENTS_BUF_ADDRESS);

    /* Set the pointer to start of buffer  */
    ppn_err_params_buf_ptr = ppn_err_params_buf;

    /* Initial first entry with reset value. It is used for saving only the first failure event error parameters  */
    *ppn_err_params_buf_ptr = PPN2MG_RESET_VAL ;
}
/********************************************************************************************************************//**
 * funcname      ppn2mg_rd_request
 * inparam       event_id: the PPN to MG event opcode
 *               stop_ppn: put/not put ppn in infinite loop
 * return        None
 * description   Save the read request event id in SP.
 *               Notify MG by triggering doorbell interrupt.
 *               If required put PPN in infinite loop to allow MG to read the PPN memories without interference.
 ************************************************************************************************************************/
void ppn2mg_rd_request(uint32_t event_id, uint32_t stop_ppn)
{
    /* Save the PPN event id that just took place. Buffer is cyclic therefore keep the last 8 events  */
    ppn2mg_events_buf_ptr[(ppn2mg_events_buf_indx++)&0x7] = event_id;

    /* Notify MG: Trigger doorbell interrupt from current PPN towards MG */
    PPA_FW_SP_WRITE(PPN_IO_REG_PPN_DOORBELL_ADDR,0);

    /* If required put PPN in infinite loop so memories won't be overrun and MG can read the memories without any interference */
    if (stop_ppn)
    {
#ifndef ASIC_SIMULATION

        __asm__ __volatile__(
                "rd_req_loop:             \n\t"
                "         nop             \n\t"
                "         nop             \n\t"
                "         J rd_req_loop   \n\t"
                );
#endif
    }
}


/********************************************************************************************************************//**
 * funcname      ppn2mg_rd_request
 * inparam       event_id:       the PPN to MG event opcode
 *               err_params_ptr: pointer to error parameters of current failure
 *               num_of_params:  number of error parameters
 *               stop_ppn:       put/not put ppn in infinite loop
 * return        None
 * description   Save the failure event id in SP.
 *               If it is the first failure then save error parameters in SP and notify MG by triggering doorbell interrupt.
 *               If it is not the first failure then just save the failure event ID in SP and return.
 *               It is done in order not to load MG with more doorbell interrupts and since the first failure event is what matter
 *               If required put PPN in infinite loop to allow MG to read the PPN memories without interference.
 ************************************************************************************************************************/
void ppn2mg_failure_event(uint32_t event_id, uint32_t* err_params_ptr ,uint32_t num_of_params, uint32_t stop_ppn)
{
    uint32_t i;

    /* Save the PPN event id that took place. Buffer is cyclic therefore keep the last 8 events  */
    ppn2mg_events_buf_ptr[(ppn2mg_events_buf_indx++)&0x7] = event_id;

    /* Check if it is the first failure event or critical error  */
    if ( (ppn_err_params_buf[0] == PPN2MG_RESET_VAL) || ( (event_id&0xFFFF) >= PPN_EXCEPTION_CRASH_EVENT_ID) )
    {
        /* Save the PPN event id in the first entry */
        *ppn_err_params_buf_ptr++ = event_id;

        /* First failure event therefore save the error parameters in SP  */
        for (i=0;i<num_of_params;i++)
        {
            *ppn_err_params_buf_ptr++ = *err_params_ptr++;
        }

        /* Notify MG: Trigger doorbell interrupt from current PPN towards MG */
        PPA_FW_SP_WRITE(PPN_IO_REG_PPN_DOORBELL_ADDR,0);

        /* If required put PPN in infinite loop so memories won't be overrun and MG can read the memories without any interference */
        if (stop_ppn)
        {
#ifndef ASIC_SIMULATION
            __asm__ __volatile__(
                    "failure_loop:              \n\t"
                    "         nop               \n\t"
                    "         nop               \n\t"
                    "         J failure_loop    \n\t"
                    );
#endif
        }
    }
}

/********************************************************************************************************************//**
 * funcname        first_packet_swap_sequence
 * inparam         None
 * return          None
 * description     Activate the below sequence: send the processing packet, gets new one, load the new PC thread and jump to it
 ************************************************************************************************************************/
INLINE void packet_swap_sequence()
{
#ifndef ASIC_SIMULATION
    uint32_t   ppa_deq  ;

    /* step 1 - swap instruction, activate packet Out & packet In machines (Enqueue packet) */
    PPA_FW_SP_WRITE(PKT_SWAP_INST_ADDR,0);
    /* step 2 - flush store before activating the blocking load */
    /*#pragma flush_memory*/
    /* step 3 - initialize a15 for pc blocking load.  load start of new packet scenario PC. */
    ppa_deq=PPA_FW_SP_READ(PKT_LOAD_PC_INST_ADDR);
    /* steps 4  - jump to start of scenario - start execution */
    asm volatile("JX %0" :: "a"(ppa_deq));
#endif /*!ASIC_SIMULATION*/
}

/********************************************************************************************************************//**
 * funcname        call_threads
 * inparam         None
 * return          None
 * description     We should never get to this point since at the end of every thread we jump to the next one
 *                 however since the Threads are not actually called but rather are jumped via JX a15 command
 *                 I added the call here to make sure the compiler build them and includes them in the disassembly
 *                 file so I can take the PC addresses
 ************************************************************************************************************************/
void call_threads()
{
    THR0_DoNothing();
    THR1_E2U();
    THR2_U2E();
    THR3_U2C();
    THR4_ET2U();
    THR5_EU2U();
    THR6_U2E();
    THR7_Mrr2E();
    THR8_E_V2U();
    THR9_E2U();
    THR10_C_V2U();
    THR11_C2U();
    THR12_U_UC2C();
    THR13_U_MC2C();
    THR14_U_MR2C();
    THR15_QCN();
    THR16_U2E();
    THR17_U2IPL();
    THR18_IPL2IPL();
    THR19_E2U_Untagged();
    THR20_U2E_M4();
    THR21_U2E_M8();
    THR22_Discard_pkt();
    THR23_EVB_E2U();
    THR24_EVB_U2E();
    THR25_EVB_QCN();
    THR26_PRE_DA_U2E();
    THR27_PRE_DA_E2U();
    THR45_VariableCyclesLengthWithAcclCmd();
    THR46_RemoveAddBytes();
    THR47_Add20Bytes();
    THR48_Remove20Bytes();
    THR49_VariableCyclesLength();
}

/********************************************************************************************************************//**
 * funcname        THR0_DoNothing
 * inparam         None
 * return          None
 * description     No add/remove tags however support cases in which PTP or ECN are still required.
 *                 For example: suppose PIPE is connected with a cascaded port towards BC3 and some IPv4 packets which don't
 *                              trigger any rule in PCP are sent to that port. They will be assigned on thread 0.
 *                              If there is a congestion on that target port the packet will not report it.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR0_DoNothing () {
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr0_DoNothing_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD0)
    init_record_ptr();
    #if (EN_REC_THRD0_INPUT)
    record_data_thread_0_DoNothing(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/
        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr0_DoNothing_desc_ptr,EN_REC_THRD0_ECN,REC_THRD_ID_0);
        }
        /*#pragma no_reorder*/
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr0_DoNothing_desc_ptr,EN_REC_THRD0_PTP,REC_THRD_ID_0);
        }
    }

#if (EN_REC_THRD0_OUTPUT)
    record_data_thread_0_DoNothing(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(0);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR1_E2U
 * inparam         None
 * return          None
 * description     Extended Port to Upstream Port (E2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA
 *                 2. Config HA table includes E TAG header type
 *                 Move MAC left 8 bytes and add TPID and E TAG
 *                 Addition for PIPE A1!!!: copy Uplink Port indication bit to IPL Direction bit in E-tag
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR1_E2U () {
    /* xt_iss_client_command("profile","disable"); */
    /* _exit(0);                                   */
    /* xt_iss_client_command("profile","enable");    */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr1_e2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD1)
    init_record_ptr();
    #if (EN_REC_THRD1_INPUT)
    record_data_thread_1_E2U(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    __LOG_FW((" Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 8 bytes left \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR1_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET, 0);
    __LOG_FW((" Copy Source Port Table[0:11] - PCID to Template<E-CID_base> \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR1_E2U_CFG_srcPortEntry__PCID_LEN12_TO_CFG_etag__E_CID_base_OFFSET, 0);
    __LOG_FW((" Copy Desc<UP> and Desc<DEI> into Template<E-PCP> and Template<E-DEI> respectively \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR1_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_etag__E_PCP_OFFSET, 0);
    __LOG_FW((" Copy Target Port Table[15:0] - TPID after New_Header<MAC SA>  (first copy it to template cfg then to packet. see next command) \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR1_E2U_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,0);
    /* Addition for PIPE A1!!!: */__LOG_FW(("Copy Source Port Table[28] - Uplink Port to Old_Header<E-Tag>[IPL Direction]"
       "Setting <IPL Direction> for CPU traffic arriving from uplink port. Single bit indicating whether the source port is an uplink port. \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR1_E2U_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_etag__IPL_Direction_OFFSET, 0);
    /*#pragma no_reorder*/
    __LOG_FW((" Copy Template into New_Header<E-Tag value>  (TPID 2B then etag 6B) \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR1_E2U_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET, 0);

    __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 8 bytes therefore the size was increment in 8 bytes. \n"));
    thr1_e2u_desc_ptr->fw_bc_modification = 8;

    __LOG_PARAM_FW(slowPathInd);
    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr1_e2u_desc_ptr,EN_REC_THRD1_ECN,REC_THRD_ID_1);
        }
        /*#pragma no_reorder*/

        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr1_e2u_desc_ptr,EN_REC_THRD1_PTP,REC_THRD_ID_1);
        }
    }

#if (EN_REC_THRD1_OUTPUT)
    record_data_thread_1_E2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(1);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname     THR2_U2E
 * inparam      None
 * return       None
 * description  Upstream Port to Extended Port (U2E) Thread:
 *              1. Pkt in includes MAC DA and MAC SA, ETAG and VLAN tag
 *              2. Config includes only target port table and source port table No tag in HA table
 *              Remove E-Tag (8B) + check if to remove vlan tag (4B)
 *              PIPE A1 addition !!!:
 *                  1. Egress Tag State: Read data shared ram to hold vlan ports bits map to check if vlan should be removed as well.
 *                  2. MultiCast SRC filtering
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR2_U2E () {
     /*xt_iss_client_command("profile","disable"); */
     /*_exit(0);                                   */
     /* xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    uint32_t VlanEgressTagState,VlanBitSelect;

    /* Get pointers to header, cfg and descriptor */
    struct thr2_u2e_in_hdr* thr2_u2e_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr2_u2e_in_hdr);
    struct thr2_u2e_cfg* thr2_u2e_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr2_u2e_cfg);
    struct nonQcn_desc* thr2_u2e_desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Addition for PIPE A1!!!:*/__LOG_FW((" Egress Tag State -> Read from data shared ram vlan ports bits map. Use vlan id to point to the right entry. \n"));
    VlanEgressTagState = PPA_FW_SHARED_DRAM_SHORT_READ(2*thr2_u2e_pkt_in_ptr->vlan.vid);
    /*#pragma no_reorder*/
    __LOG_PARAM_FW(VlanEgressTagState);

#if (EN_REC_THRD2)
    init_record_ptr();
    #if (EN_REC_THRD2_INPUT)
    record_data_thread_2_U2E(REC_INPUT);
    #endif
#endif

    /* Addition for PIPE A1 !!! MultiCast SRC filtering  */
    __LOG_PARAM_FW(thr2_u2e_cfg_ptr->targetPortEntry.PCID);
    __LOG_PARAM_FW(thr2_u2e_pkt_in_ptr->etag.Ingress_E_CID_base);
    if (thr2_u2e_cfg_ptr->targetPortEntry.PCID == thr2_u2e_pkt_in_ptr->etag.Ingress_E_CID_base)
    {
        __LOG_FW(("MultiCast SRC filtering (PCID==Ingress_E_CID_base)\n"));
        /*#pragma frequency_hint NEVER*/
        __LOG_FW((" Drop the packet. Indicates it by setting fw_drop field in descriptor to '1' \n"));
        thr2_u2e_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Get slow path indication value */
        slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
        /*#pragma no_reorder*/

        __LOG_FW(("Remove E-Tag: Move Old_Header<MAC DA> and Old_Header<MAC SA> 8 bytes such that Old_Header<E-Tag> is removed; \n"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET, 0);
        __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 8 bytes therefore the size was reduced in 8 bytes. \n"));
        thr2_u2e_desc_ptr->fw_bc_modification = -8;
        /*#pragma no_reorder*/

        /* Addition for PIPE A1 !!!  */__LOG_FW(("Set vlan bit select according to target port number that is used by this thread to extract the specific target port"
           "NOTE: CPU port (port 16) is not used by the U2E thread \n"));
        VlanBitSelect      =  thr2_u2e_cfg_ptr->targetPortEntry.TargetPort;
        /* Update vlan egress tag state according to vlan ports bits map in shared ram */
        VlanEgressTagState =  (0x1 << VlanBitSelect) & VlanEgressTagState ;

        __LOG_PARAM_FW(VlanBitSelect);
        __LOG_PARAM_FW(VlanEgressTagState);
        __LOG_PARAM_FW(thr2_u2e_pkt_in_ptr->vlan.TPID);
        __LOG_PARAM_FW(thr2_u2e_cfg_ptr->targetPortEntry.TPID);

        __LOG_FW((" If Old_Header<TPID after E-Tag>= Target Port Table[15:0] - TPID and VlanEgressTagState == 0 \n"));
        if ( (thr2_u2e_pkt_in_ptr->vlan.TPID == thr2_u2e_cfg_ptr->targetPortEntry.TPID) && (VlanEgressTagState == 0) )
        { __LOG_FW((" Remove also VLAN Tag \n"));
            __LOG_FW((" Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 more bytes such that Old_Header<VLAN-Tag> is removed \n"));
            ACCEL_CMD_TRIG(COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_PLUS8_LEN12_TO_MACSHIFTRIGHT_12_OFFSET, 0);

            __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right total 12 bytes therefore the size was reduced in 12 bytes. \n"));
            thr2_u2e_desc_ptr->fw_bc_modification = -12;
        }

        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        __LOG_PARAM_FW(slowPathInd);
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/
            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr2_u2e_desc_ptr,EN_REC_THRD2_ECN,REC_THRD_ID_2);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr2_u2e_desc_ptr,EN_REC_THRD2_PTP,REC_THRD_ID_2);
            }
        }
    }

#if (EN_REC_THRD2_OUTPUT)
    record_data_thread_2_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(2);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR3_U2C
 * inparam         None
 * return          None
 * description     Upstream Port to Cascade Port (U2C) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and E TAG type
 *                 2. Config includes only target port table and source port table No tag in HA table
 *                 According to etag.E_CID_base and etag.GRP decide if to remove etag or transmit as it is
 *                 PIPE A1 addition!!!: Reset IPL Direction bit in E-tag
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR3_U2C () {
    /* xt_iss_client_command("profile","disable"); */
    /* _exit(0);  */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointers to header, cfg and descriptor */
    struct thr3_u2c_in_hdr* thr3_u2c_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr3_u2c_in_hdr);
    struct thr3_u2c_cfg* thr3_u2c_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr3_u2c_cfg);
    struct nonQcn_desc* thr3_u2c_desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD3)
    init_record_ptr();
    #if (EN_REC_THRD3_INPUT)
    record_data_thread_3_U2C(REC_INPUT);
    #endif
#endif

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Addition for PIPE A1 !!!*/
       __LOG_FW(("Inter PIPE Link (IPL): IPL is defined as a non-standard horizontal link between two PIPE devices which act as a single standard Port Extender."
       "In order to support these links and keep standard behavior on the Port Extender external links, we add a <IPL Direction> field to the standard E-Tag."
       "Reset (clear) IPL Direction bit in E-tag  \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR3_U2C_CFG_cfgReservedSpace__reserved_0_LEN1_TO_PKT_etag__IPL_Direction_OFFSET, 0);
    /*#pragma no_reorder*/

    __LOG_PARAM_FW(thr3_u2c_pkt_in_ptr->etag.GRP);
    __LOG_PARAM_FW(thr3_u2c_pkt_in_ptr->etag.E_CID_base);
    __LOG_PARAM_FW(thr3_u2c_cfg_ptr->targetPortEntry.PCID);
    __LOG_FW((" If Old_Header<E-Tag<E-CID_base>>= Target Port Table[43:32] - PCID and Old_Header<E-Tag<GRP>>=0 then \n"));
    if ( (thr3_u2c_pkt_in_ptr->etag.E_CID_base == thr3_u2c_cfg_ptr->targetPortEntry.PCID) && (thr3_u2c_pkt_in_ptr->etag.GRP == 0) )
    { __LOG_FW((" E-Tag is removed \n"));

        __LOG_FW((" Move Old_Header<MAC DA> and Old_Header<MAC SA> 8 bytes such that Old_Header<E-Tag> is removed \n"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR3_U2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET, 0);

        __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 8 bytes therefore the size was reduced in 8 bytes. \n"));
        thr3_u2c_desc_ptr->fw_bc_modification = -8;
    }

    __LOG_PARAM_FW(slowPathInd);
    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr3_u2c_desc_ptr,EN_REC_THRD3_ECN,REC_THRD_ID_3);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr3_u2c_desc_ptr,EN_REC_THRD3_PTP,REC_THRD_ID_3);
        }
    }

#if (EN_REC_THRD3_OUTPUT)
    record_data_thread_3_U2C(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(3);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR4_ET2U
 * inparam         None
 * return          None
 * description     Extended Port Tagged to Upstream Port (ET2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and VLAN TAG type
 *                 2. HA template is in the FORWARD 4B DSA Tag format
 *                 Replace VLAN tag with FORWARD 4B DSA tag and send the packet with original size (no shift)
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR4_ET2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr4_et2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD4)
    init_record_ptr();
    #if (EN_REC_THRD4_INPUT)
    record_data_thread_4_ET2U(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    __LOG_FW((" Copy Source Port Table[12:15] - Source Port number to Template<Src Port> \n"));/*    since we copy 4 bits to 5 bits field do we need to zero pad the msb bit? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bit. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR4_ET2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET, 0);

    __LOG_FW((" Copy VLAN-Tag<VLAN, PCP, DEI> into Template<VLAN, UP, CFI> \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR4_ET2U_PKT_vlan__vid_LEN12_TO_CFG_DSA_fwd__vid_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR4_ET2U_PKT_vlan__up_LEN3_TO_CFG_DSA_fwd__up_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR4_ET2U_PKT_vlan__cfi_LEN1_TO_CFG_DSA_fwd__cfi_OFFSET, 0);
    /*#pragma no_reorder*/

    __LOG_FW((" Copy Template(4B) into New_Header<DSA tag> \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR4_ET2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET, 0);

    __LOG_PARAM_FW(slowPathInd);
    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/
        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr4_et2u_desc_ptr,EN_REC_THRD4_ECN,REC_THRD_ID_4);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr4_et2u_desc_ptr,EN_REC_THRD4_PTP,REC_THRD_ID_4);
        }
    }

#if (EN_REC_THRD4_OUTPUT)
    record_data_thread_4_ET2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(4);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR5_EU2U
 * inparam         None
 * return          None
 * description     Extended Port UnTagged to Upstream Port (EU2U) Thread:
 *                 1. Pkt in incldues MAC DA and MAC SA only
 *                 2. HA template is in the FORWARD 4B DSA Tag format
 *                 Add FORWARD 4B DSA Tag into packet after MAC SA
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR5_EU2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0);                                   */
    /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr5_eu2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD5)
    init_record_ptr();
    #if (EN_REC_THRD5_INPUT)
    record_data_thread_5_EU2U(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Set Template<VLAN>= 0 */
    ACCEL_CMD_TRIG(COPY_BITS_THR5_EU2U_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_DSA_fwd__vid_OFFSET, 0);

    /* Copy Source Port Table[12:15] - Source Port number to Template<Src Port>     since we copy 4 bits to 5 bits field do we need to zero padd the msb bit? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bit. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR5_EU2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET, 0);

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 4 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR5_EU2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 4 bytes therefore the size was increment in 4 bytes. */
    thr5_eu2u_desc_ptr->fw_bc_modification = 4;

    /* Copy Template into New_Header<DSA> after New_Header<MAC SA> */
    ACCEL_CMD_TRIG(COPY_BYTES_THR5_EU2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET, 0);


    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr5_eu2u_desc_ptr,EN_REC_THRD5_ECN,REC_THRD_ID_5);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr5_eu2u_desc_ptr,EN_REC_THRD5_PTP,REC_THRD_ID_5);
        }
    }

#if (EN_REC_THRD5_OUTPUT)
    record_data_thread_5_EU2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(5);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname      THR6_U2E
 * inparam       None
 * return        None
 * description   Upstream Port to Extended Port (U2E) Thread:
 *                1. Pkt in incldues MAC DA and MAC SA and 8B FORWARD DSA tag
 *                2. HA template includes target port table and source port table, no tag in HA table
 *                1. Checks that the packet is 8B DSA. If this is not the case discard the packet
 *                2. Checks if vlan or not and act accordingly:
 *                   No VLAN => remove extended DSA-Tag and VLAN Tag is not added
 *                   With VLAN => remove extended DSA-Tag and add VLAN Tag
 *               PIPE A1 addition !!!:
 *                1. Egress Tag State: Read data shared ram to hold vlan ports bits map to check if vlan should be removed as well.
 *                2. MultiCast SRC filtering
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR6_U2E () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0);                                   */
    /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    uint32_t VlanEgressTagState,VlanBitSelect;

    /* Get pointers to cfg, header and descriptor */
    struct thr6_u2e_cfg* thr6_u2e_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr6_u2e_cfg);
    struct thr6_u2e_in_hdr* thr6_u2e_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr6_u2e_in_hdr);
    struct nonQcn_desc* thr6_u2e_desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Addition for PIPE A1!!!: Egress Tag State -> Read from data shared ram vlan ports bits map. Use vlan id to point to the right entry. */
    VlanEgressTagState = PPA_FW_SHARED_DRAM_SHORT_READ(2*thr6_u2e_pkt_in_ptr->extDSA_fwd_w0.vid);
    /*#pragma no_reorder*/

#if (EN_REC_THRD6)
    init_record_ptr();
    #if (EN_REC_THRD6_INPUT)
    record_data_thread_6_U2E(REC_INPUT);
    #endif
#endif

    /* Get PTP and ECN marks to identify slow path flow */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);

    /* Checks that indeed the packet is 8B DSA otherwise indicates it for drop:  if Old_Header<FORWARD DSA<First Word Extended>>=0 or Old_Header<FORWARD DSA<First Word Extended>>=1 then discard the packet  */
    if ( (thr6_u2e_pkt_in_ptr->extDSA_fwd_w0.extended == 0) || (thr6_u2e_pkt_in_ptr->extDSA_fwd_w1.Extend == 1) ||    \
         /* Multicast Source Filtering:  if (Header Alteration Table<Own Device> = Old_Header<DSA[Source Device]> and Target Port Table[19:16] - Target Port = Old_Header<DSA[Src Port]>)  */
         ((thr6_u2e_cfg_ptr->HA_Table_Own_device.Own_Device == thr6_u2e_pkt_in_ptr->extDSA_fwd_w0.srcDev) && ((thr6_u2e_cfg_ptr->targetPortEntry.TargetPort == thr6_u2e_pkt_in_ptr->extDSA_fwd_w0.SrcPort_4_0) && (thr6_u2e_pkt_in_ptr->extDSA_fwd_w1.SrcPort_6_5 == 0)))  )
    {
        /*#pragma frequency_hint NEVER */
        /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
        thr6_u2e_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Addition for PIPE A1 !!! Set vlan bit select according to target port number that is used by this thread to extract the specific target port */
        VlanBitSelect      =  thr6_u2e_cfg_ptr->targetPortEntry.TargetPort;
        /* Update vlan egress tag state according to vlan ports bits map in shared ram */
        VlanEgressTagState =  (0x1 << VlanBitSelect) & VlanEgressTagState ;

        /* Check Egress Tag State to know if need to add vlan or not  */
        if (VlanEgressTagState == 0)
        {   /* Remove extended 8B fwd DSA tag and don't add VLAN tag */
            /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 8 bytes right */
            ACCEL_CMD_TRIG(COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET, 0);
            /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 8 bytes therefore the size was reduced in 8 bytes. */
            thr6_u2e_desc_ptr->fw_bc_modification = -8;
        }
        else
        {   /* Remove extended 8B fwd DSA tag and add 4B VLAN tag */
            /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 4 bytes therefore the size was reduced in 4 bytes (0xFC). */
            thr6_u2e_desc_ptr->fw_bc_modification = -4;
            /* Copy Old_Header<DSA<VLAN, UP, CFI>> into cfg vlan template then later to new packet vlan tag */
            /* Since no support in HW to copy data straight from old packet to new packet we first copy the data to reserved area in config and then from there to new packet */
            ACCEL_CMD_TRIG(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__vid_LEN12_TO_CFG_vlan__vid_OFFSET, 0);
            ACCEL_CMD_TRIG(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__up_LEN3_TO_CFG_vlan__up_OFFSET, 0);
            ACCEL_CMD_TRIG(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__cfi_LEN1_TO_CFG_vlan__cfi_OFFSET, 0);
            ACCEL_CMD_TRIG(COPY_BITS_THR6_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET, 0);
            /* #pragma no_reorder */
            /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 bytes right */
            ACCEL_CMD_TRIG(COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);
            /* #pragma no_reorder*/
            /* Copy vlan template into new output header packet */
            ACCEL_CMD_TRIG(COPY_BYTES_THR6_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET, 0);
        }

        /*---------------------------------------------------
          * Check if slow path is required
          * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/

            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr6_u2e_desc_ptr,EN_REC_THRD6_ECN,REC_THRD_ID_6);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr6_u2e_desc_ptr,EN_REC_THRD6_PTP,REC_THRD_ID_6);
            }
        }
    }

#if (EN_REC_THRD6_OUTPUT)
    record_data_thread_6_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(6);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR7_Mrr2E
 * inparam         None
 * return          None
 * description     Upstream Port Mirroring to Extended Port (Mrr2E) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and 4 bytes DSA FROM_CPU tag type.
 *                 2. Config includes only target port table and source port table No tag in HA table
 *                 Checks that the packet is indeed 4B DSA and remove the tag. If this is not the case than discard the packet.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR7_Mrr2E () {
    /*  xt_iss_client_command("profile","disable"); */
    /*  _exit(0);                                   */
    /* xt_iss_client_command("profile","enable");   */

    uint8_t slowPathInd;
    /* Get pointers to header and descriptor */
    struct thr7_mrr2e_in_hdr* thr7_mrr2e_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr7_mrr2e_in_hdr);
    struct nonQcn_desc* thr7_mrr2e_desc_ptr         = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD7)
    init_record_ptr();
    #if (EN_REC_THRD7_INPUT)
    record_data_thread_7_Mrr2E(REC_INPUT);
    #endif
#endif

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Checks that the packet is 4B DSA.  if Old_Header<FROM_CPU DSA<First Word Extended>>=1 discard the packet; */
    if (thr7_mrr2e_pkt_in_ptr->DSA_from_cpu.Extend == 1)
    {   /* Drop the packet */

        /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Mrr2E_PKT_DSA_from_cpu__Extend_LEN1_TO_DESC_nonQcn_desc__fw_drop_OFFSET, 0);
    }
    else
    {   /* Remove 4B DSA tag */

        /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 bytes such that Old_Header<DSA-Tag> is removed */
        ACCEL_CMD_TRIG(COPY_BYTES_THR7_Mrr2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 4 bytes therefore the size was reduced in 4 bytes. */
        thr7_mrr2e_desc_ptr->fw_bc_modification = -4;

        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/

            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr7_mrr2e_desc_ptr,EN_REC_THRD7_ECN,REC_THRD_ID_7);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr7_mrr2e_desc_ptr,EN_REC_THRD7_PTP,REC_THRD_ID_7);
            }
        }
    }

#if (EN_REC_THRD7_OUTPUT)
    record_data_thread_7_Mrr2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(7);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR8_E_V2U
 * inparam         None
 * return          None
 * description     Extended Port VLAN Tagged to Upstream port (E-V2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and 4 bytes VLAN tag.
 *                 2. Config includes target port table and source port table with eDSA FWD type(16B) tag in HA table
 *                 Remove vlan tag and add eDSA tag.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR8_E_V2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr8_e_v2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD8)
    init_record_ptr();
    #if (EN_REC_THRD8_INPUT)
    record_data_thread_8_E_V2U(REC_INPUT);
    #endif
#endif

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 12 bytes therefore the size was increment in 12 bytes. */
    thr8_e_v2u_desc_ptr->fw_bc_modification = 12;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 12 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR8_E_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Old_Header<VLAN> into Template<eVLAN[11:0];*/  /* Copy Old_Header<PCP> into Template<UP>; */ /* Copy Old_Header<DEI> into Template<CFI>; */
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET, 0);

    /* Copy Source Port ePort to Template<Source ePort>; */
    /* Source Port ePort in eDSA tag is divided and placed in 3 different areas. To save cycles src_ePort in srcPortEntry is divided the same way to 3 as well. */
    /* Source Port ePort in srcPortEntry table is 14 bits while in eDSA tag is 17 bits. Do we need to zero pad 3 bits ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bits. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET, 0);

    /* Copy Desc<TC> and Desc<DP> into Template<QoS Profile>; */
    /* Since TC is 3 bits and DP is 2 bits and QoS Profile is 7 bits. Do we need to pad 2 MSB bits with zeros ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bits. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR8_E_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET, 0);
    /*#pragma no_reorder*/

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr8_e_v2u_desc_ptr,EN_REC_THRD8_ECN,REC_THRD_ID_8);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr8_e_v2u_desc_ptr,EN_REC_THRD8_PTP,REC_THRD_ID_8);
        }
    }

#if (EN_REC_THRD8_OUTPUT)
    record_data_thread_8_E_V2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(8);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR9_E2U
 * inparam         None
 * return          None
 * description     Extended Port VLAN Untagged to Upstream port (E2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA
 *                 2. Config includes target port table and source port table with eDSA FWD type (16B) tag in HA table
 *                 Add eDSA tag.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR9_E2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr9_e2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD9)
    init_record_ptr();
    #if (EN_REC_THRD9_INPUT)
    record_data_thread_9_E2U(REC_INPUT);
    #endif
#endif

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 16 bytes therefore the size was increment in 16 bytes. */
    thr9_e2u_desc_ptr->fw_bc_modification = 16;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 16 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR9_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_16_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Desc<TC> and Desc<DP> into Template<QoS Profile>; */
    /* Since TC is 3 bits and DP is 2 bits and QoS Profile is 7 bits. Do we need to pad 2 MSB bits with zeros ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bits. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR9_E2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_E2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET, 0);

    /* Copy Source Port ePort to Template<Source ePort>; */
    /* Source Port ePort in eDSA tag is divided and placed in 3 different areas. To save cycles src_ePort in srcPortEntry is divided the same way to 3 as well. */
    /* Source Port ePort in srcPortEntry table is 14 bits while in eDSA tag is 17 bits. Do we need to zero padding 3 bits ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bit. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR9_E2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET, 0);
    /*#pragma no_reorder*/

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr9_e2u_desc_ptr,EN_REC_THRD9_ECN,REC_THRD_ID_9);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr9_e2u_desc_ptr,EN_REC_THRD9_PTP,REC_THRD_ID_9);
        }
    }

#if (EN_REC_THRD9_OUTPUT)
    record_data_thread_9_E2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(9);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR10_C_V2U
 * inparam         None
 * return          None
 * description     Cascade Port VLAN Tagged to Upstream port (C-V2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA, etag of 8 bytes and 4 bytes VLAN tag.
 *                 2. Config includes target port table and source port table with eDSA FWD type (16B) tag in HA table
 *                 Remove etag and vlan tag and add eDSA tag.
 *                 NOTE: CFG=>cfgReservedSpace=>reserved_0 are 4 bytes zero set by HW. We use them for zero padding.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR10_C_V2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointers to cfg, header and descriptor */
    struct thr10_c_v2u_cfg* thr10_c_v2u_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr10_c_v2u_cfg);
    struct thr10_c_v2u_in_hdr* thr10_c_v2u_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr10_c_v2u_in_hdr);
    struct nonQcn_desc* thr10_c_v2u_desc_ptr          = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Copy Old_Header<E-Tag<E-CID_base>>+Min ePort-1 to Template<Source ePort>; */
    /* Calculate and save the result first in reserved field for better core performance since Template<Source ePort> is divided into 3 different parts in eDSA template */
    /* reserved2 is 16 bits while Template<Source ePort> is total of 17 bits but the assumption here is that E_CID_base + Min ePort-1 is not bigger than 16 bits */
    thr10_c_v2u_cfg_ptr->srcPortEntry.reserved2 = thr10_c_v2u_pkt_in_ptr->etag.E_CID_base + PPA_FW_SP_SHORT_READ(MIN_E_PORT_MINUS_1_SP_ADDR);
    /*#pragma no_reorder*/

#if (EN_REC_THRD10)
    init_record_ptr();
    #if (EN_REC_THRD10_INPUT)
    record_data_thread_10_C_V2U(REC_INPUT);
    #endif
#endif

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we will shift left 4 bytes therefore the size was increment in 4 bytes. */
    thr10_c_v2u_desc_ptr->fw_bc_modification = 4;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 4 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR10_C_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Old_Header<VLAN> into Template<eVLAN[11:0]; */ /* Copy Old_Header<PCP> into Template<UP>; */ /* Copy Old_Header<DEI> into Template<CFI>; */
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET, 0);

    /* Copy Desc<TC> and Desc<DP> into Template<QoS Profile>; */
    /* Since TC is 3 bits and DP is 2 bits and QoS Profile is 7 bits. Do we need to padd 2 MSB bits with zeros ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bit. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET, 0);

    /* Since Source Port ePort in eDSA tag is divided and placed in 3 different areas need 3 commands to put it there */
    /* Since reserved2 is 16 bits and Src_ePort is 17 bits need to zero padd with 1 bit */
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET, 0);
    /*#pragma no_reorder*/
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR10_C_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET, 0);
    /*#pragma no_reorder*/

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr10_c_v2u_desc_ptr,EN_REC_THRD10_ECN,REC_THRD_ID_10);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr10_c_v2u_desc_ptr,EN_REC_THRD10_PTP,REC_THRD_ID_10);
        }
    }

#if (EN_REC_THRD10_OUTPUT)
    record_data_thread_10_C_V2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(10);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR11_C2U
 * inparam         None
 * return          None
 * description     Cascade Port VLAN Untagged to Upstream port (C2U) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and etag of 8 bytes
 *                 2. Config includes target port table and source port table with eDSA FWD type(16B) tag in HA table
 *                 Remove etag and add eDSA tag.
 *                 NOTE: CFG=>cfgReservedSpace=>reserved_0 are 4 bytes zero set by HW. We use them for zero padding.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR11_C2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    /* Get pointers to cfg, header and descriptor */
    struct thr11_c2u_cfg* thr11_c2u_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr11_c2u_cfg);
    struct thr11_c2u_in_hdr* thr11_c2u_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr11_c2u_in_hdr);
    struct nonQcn_desc* thr11_c2u_desc_ptr        = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Copy Old_Header<E-Tag<E-CID_base>>+Min ePort-1 to Template<Source ePort>; */
    /* Calculate and save the result in reserved field since Template<Source ePort> is divided into 3 different parts in eDSA template */
    /* reserved2 is 16 bits while Template<Source ePort> is total of 17 bits but the assumption here is that E_CID_base + Min ePort-1 is no longer than 16 bits */
    thr11_c2u_cfg_ptr->srcPortEntry.reserved2 = thr11_c2u_pkt_in_ptr->etag.E_CID_base + PPA_FW_SP_SHORT_READ(MIN_E_PORT_MINUS_1_SP_ADDR);
    /*#pragma no_reorder*/

#if (EN_REC_THRD11)
    init_record_ptr();
    #if (EN_REC_THRD11_INPUT)
    record_data_thread_11_C2U(REC_INPUT);
    /*#pragma no_reorder*/
    #endif
#endif

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 8 bytes therefore the size was increment in 8 bytes. */
    thr11_c2u_desc_ptr->fw_bc_modification = 8;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 8 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR11_C2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Desc<TC> and Desc<DP> into Template<QoS Profile>; */
    /* Since TC is 3 bits and DP is 2 bits and QoS Profile is 7 bits. Do we need to pad 2 MSB bits with zeros ? */
    /* Note: In meeting with CPSS,Design and Architect it was decided that firmware is not responsible to clear MSB bit. Should be left as it is!!!  */
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET, 0);

    /* Since Source Port ePort in eDSA tag is divided and placed in 3 different areas need 3 commands to put it there */
    /* Since reserved2 is 16 bits and Src_ePort is 17 bits need to zero padd with 1 bit */
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET, 0);
    /*#pragma no_reorder*/
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR11_C2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET, 0);
    /*#pragma no_reorder*/

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr11_c2u_desc_ptr,EN_REC_THRD11_ECN,REC_THRD_ID_11);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr11_c2u_desc_ptr,EN_REC_THRD11_PTP,REC_THRD_ID_11);
        }
    }

#if (EN_REC_THRD11_OUTPUT)
    record_data_thread_11_C2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(11);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR12_U_UC2C
 * inparam         None
 * return          None
 * description     Upstream port Unicast to Cascade port (U-UC2C) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and eDSA tag FWD type of 16 bytes
 *                 2. Config includes target port table and source port table with etag (8B) in HA table
 *                 Two options for packet out: 1) Remove eDSA tag 2) Remove eDSA tag and add etag and vlan tags.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR12_U_UC2C () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint16_t min_ePORT_minus_1;
    uint8_t slowPathInd;
    struct thr12_u_uc2c_cfg* thr12_u_uc2c_cfg_ptr;
    struct thr12_u_uc2c_in_hdr* thr12_u_uc2c_pkt_in_ptr;
    struct nonQcn_desc* thr12_u_uc2c_desc_ptr;
    uint32_t cfg_trg_ePort, hdr_Trg_ePort;

#if (EN_REC_THRD12)
    init_record_ptr();
    #if (EN_REC_THRD12_INPUT)
    record_data_thread_12_U_UC2C(REC_INPUT);
    #endif
#endif

    /* Get pointers to packet header, descriptor and configuration */
    thr12_u_uc2c_cfg_ptr    = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr12_u_uc2c_cfg);
    thr12_u_uc2c_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr12_u_uc2c_in_hdr);
    thr12_u_uc2c_desc_ptr   = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Get value from SP memory */
    min_ePORT_minus_1 = PPA_FW_SP_SHORT_READ(MIN_E_PORT_MINUS_1_SP_ADDR);

    /* trg_ePort in targetPortEntry structure is split to keep 32 bits alignment for the core therefore we copy the whole 14 bits to temp field and will use it for comparison */
    cfg_trg_ePort = (thr12_u_uc2c_cfg_ptr->targetPortEntry.trg_ePort_13_11<<11) | (thr12_u_uc2c_cfg_ptr->targetPortEntry.trg_ePort_10_0);
    hdr_Trg_ePort = thr12_u_uc2c_pkt_in_ptr->eDSA_fwd_w3.Trg_ePort;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<eDSA<Target ePort>>-Min ePort-1 into Template<E-CID_base>    */
    /* First save the result in 16 bits reserved field so compiler will save it in simple 16 bits store instead of load/extract/mask/or etc. */
    /* then copy it to etag.E_CID_base by accelerator command */
    thr12_u_uc2c_cfg_ptr->srcPortEntry.reserved1 = hdr_Trg_ePort - min_ePORT_minus_1;
    /*#pragma no_reorder*/

    /* If Header<eDSA<Target ePort>>= Target Port ePort then */
    if (hdr_Trg_ePort == cfg_trg_ePort)
    {  /* Remove eDSA tag */

        /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 16 bytes right such that Old_Header<eDSA-Tag> is removed */
        ACCEL_CMD_TRIG(COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 16 bytes therefore the size was reduced in 16 bytes. */
        thr12_u_uc2c_desc_ptr->fw_bc_modification = -16;
    }
    else
    {  /* Remove eDSA tag and add etag and vlan tags */

        /* Copy Min ePort-1 into New_Header<VLAN Tag<TPID>>; */
        thr12_u_uc2c_cfg_ptr->vlan.TPID = min_ePORT_minus_1;
        /*#pragma no_reorder*/

        /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 16 bytes therefore the size was reduced in 4 bytes. */
        thr12_u_uc2c_desc_ptr->fw_bc_modification = -4;
        /*#pragma no_reorder*/

        /* Copy Desc<TC> into Template<PCP> */    /* Copy Desc<DP> into Template<DEI> */
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET, 0);

        /* Copy Old_Header<eDSA<eVLAN[11:0]>> into New_Header<eVLAN[11:0];  */ /* Copy Old_Header<eDSA<UP>> into New_Header<VLAN Tag<PCP>>;  */ /* Copy Old_Header<eDSA<CFI>> into New_Header<VLAN Tag<DEI>>; */
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET, 0);
        /*#pragma no_reorder*/

        /* Copy Old_Header<eDSA<Target ePort>>-Min ePort-1 into Template<E-CID_base> */
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET, 0);
        /*#pragma no_reorder*/

        /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
        ACCEL_CMD_TRIG(COPY_BITS_THR12_U_UC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET, 0);
        /* Copy E-Tag Template into New_Header after TPID; */
        ACCEL_CMD_TRIG(COPY_BYTES_THR12_U_UC2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET, 0);
        /* Copy VLAN Tag Template into New_Header after E-Tag; */
        ACCEL_CMD_TRIG(COPY_BYTES_THR12_U_UC2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET, 0);
        /*#pragma no_reorder*/

        /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 bytes */
        ACCEL_CMD_TRIG(COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);
    }

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr12_u_uc2c_desc_ptr,EN_REC_THRD12_ECN,REC_THRD_ID_12);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr12_u_uc2c_desc_ptr,EN_REC_THRD12_PTP,REC_THRD_ID_12);
        }
    }

#if (EN_REC_THRD12_OUTPUT)
    record_data_thread_12_U_UC2C(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(12);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR13_U_MC2C
 * inparam         None
 * return          None
 * description     Upstream port Multicast to Cascade port (U-MC2C) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and eDSA FWD tag (16B)
 *                 2. HA template is in the E-Tag format
 *                 Reduce packet size by 4 bytes and add E-Tag(8B) and VLAN tag(4B)
 *                 NOTE: CFG=>cfgReservedSpace=>reserved_0 are 4 bytes zero set by HW. We use them for zero padding.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR13_U_MC2C () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint16_t min_ePORT_minus_1,min_ePORT,max_ePORT;
    uint8_t slowPathInd;
    /* Get pointers to cfg and descriptor */
    struct thr13_u_mc2c_cfg* thr13_u_mc2c_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr13_u_mc2c_cfg);
    struct nonQcn_desc* thr13_u_mc2c_desc_ptr     = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD13)
    init_record_ptr();
    #if (EN_REC_THRD13_INPUT)
    record_data_thread_13_U_MC2C(REC_INPUT);
    #endif
#endif

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Concatenate src_ePort value which is divided from 3 different eDSA words and save it in config template */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_src_ePort__src_ePort_full_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_src_ePort__src_ePort_full_PLUS10_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_src_ePort__src_ePort_full_PLUS12_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy Old_Header<eDSA<eVIDX>>-Multicast offset into Template<GRP,E-CID_base>; */
    /* Concatenate eVIDX value which is divided from 2 different eDSA words and save it in config etag template */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w3__eVIDX_15_12_LEN4_TO_CFG_etag_u_mc2c__eVIDX_full_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__eVIDX_11_0_LEN12_TO_CFG_etag_u_mc2c__eVIDX_full_PLUS4_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Get ePORT parameters from SP */
    min_ePORT_minus_1 = PPA_FW_SP_SHORT_READ(MIN_E_PORT_MINUS_1_SP_ADDR);
    min_ePORT         = PPA_FW_SP_SHORT_READ(MIN_E_PORT_SP_ADDR);
    max_ePORT         = PPA_FW_SP_SHORT_READ(MAX_E_PORT_SP_ADDR);

    /* Copy Desc<TC> into Template<PCP> and Copy Desc<DP> into Template<DEI> */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag_u_mc2c__E_PCP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag_u_mc2c__E_DEI_OFFSET, 0);
    /* Config VLAN tag: Copy Old_Header<eDSA<eVLAN[11:0]>> into New_Header<eVLAN[11:0]; */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan_eDSA__eVLAN_OFFSET, 0);
    /*#pragma no_reorder*/

    /* According to IlanY the result should fits 12 bits */
    thr13_u_mc2c_cfg_ptr->srcPortEntry.reserved1 = thr13_u_mc2c_cfg_ptr->src_ePort.src_ePort_full - min_ePORT_minus_1;
    /*#pragma no_reorder*/

    /* Config VLAN tag: Copy Old_Header<eDSA<UP>> into New_Header<VLAN Tag<PCP>>;   Copy Old_Header<eDSA<CFI>> into New_Header<VLAN Tag<DEI>>; */
    /* Since no support in HW to copy data straight from old packet to new packet we first copy the data to config and then from there to new packet */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan_eDSA__up_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan_eDSA__cfi_OFFSET, 0);
    /* Copy Min ePort-1 value into New_Header<VLAN Tag<TPID>>; */
    thr13_u_mc2c_cfg_ptr->vlan_eDSA.TPID = min_ePORT_minus_1;
    /*#pragma no_reorder*/

    /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 bytes right */
    ACCEL_CMD_TRIG(COPY_BYTES_THR13_U_MC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);
    /*#pragma no_reorder*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 4 bytes therefore the size was reduced in 4 bytes. */
    thr13_u_mc2c_desc_ptr->fw_bc_modification = -4;

    /* Decrement MULTICAST OFFSET (according to IlanY the result should fits 14 bits) */
    thr13_u_mc2c_cfg_ptr->etag_u_mc2c.eVIDX_full = thr13_u_mc2c_cfg_ptr->etag_u_mc2c.eVIDX_full - PPA_FW_SP_SHORT_READ(MULTICAST_OFFSET_SP_ADDR);

    /*if Old_Header<eDSA<source ePort>> < Min ePort or Old_Header<eDSA<source ePort>> > Max ePort then Template<Ingress E-CID_base>=0 */
    /*else Template<Ingress E-CID_base>=Old_Header<eDSA<source ePort> - Min ePort-1 */
    if ((thr13_u_mc2c_cfg_ptr->src_ePort.src_ePort_full >= min_ePORT) && (thr13_u_mc2c_cfg_ptr->src_ePort.src_ePort_full <= max_ePORT))
    {   /* According to IlanY the result should fits 12 bits */
        ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET, 0);
    }
    else
    {
        /* thr13_u_mc2c_cfg_ptr->etag_u_mc2c.Ingress_E_CID_base = 0; */
        ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET, 0);
    }
    /*#pragma no_reorder*/

    /* Copy etag(8B) and vlan(4B) templates from config to new packet. Place them after mac SA */
    ACCEL_CMD_TRIG(COPY_BITS_THR13_U_MC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag_u_mc2c__TPID_OFFSET, 0);
    /* Copy E-Tag Template into New_Header after TPID; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR13_U_MC2C_CFG_etag_u_mc2c__E_PCP_LEN6_TO_PKT_etag_u_mc2c__E_PCP_OFFSET, 0);
    /* Copy VLAN Tag Template into New_Header after E-Tag; */
    ACCEL_CMD_TRIG(COPY_BYTES_THR13_U_MC2C_CFG_vlan_eDSA__TPID_LEN4_TO_PKT_vlan_eDSA__TPID_OFFSET, 0);

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr13_u_mc2c_desc_ptr,EN_REC_THRD13_ECN,REC_THRD_ID_13);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr13_u_mc2c_desc_ptr,EN_REC_THRD13_PTP,REC_THRD_ID_13);
        }
    }

#if (EN_REC_THRD13_OUTPUT)
    record_data_thread_13_U_MC2C(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(13);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR14_U_MR2C
 * inparam         None
 * return          None
 * description     Upstream port - Mirroring to Cascade port (U-MR2C) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and eDSA tag of TO_ANALAYZER type of 16 bytes
 *                 2. Config includes target port table and source port table with etag (8B) in HA table
 *                 Two options for packet out: 1) Remove eDSA tag 2) Remove eDSA tag and add etag and vlan tags.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR14_U_MR2C () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint16_t min_ePORT_minus_1;
    uint8_t slowPathInd;
    struct thr14_u_mr2c_cfg* thr14_u_mr2c_cfg_ptr;
    struct thr14_u_mr2c_in_hdr* thr14_u_mr2c_pkt_in_ptr;
    struct nonQcn_desc* thr14_u_mr2c_desc_ptr;
    uint32_t cfg_trg_ePort, hdr_Trg_ePort;

#if (EN_REC_THRD14)
    init_record_ptr();
    #if (EN_REC_THRD14_INPUT)
    record_data_thread_14_U_MR2C(REC_INPUT);
    #endif
#endif

    /* Get pointers to packet header, descriptor and configuration */
    thr14_u_mr2c_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr14_u_mr2c_cfg);
    thr14_u_mr2c_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr14_u_mr2c_in_hdr);
    thr14_u_mr2c_desc_ptr   = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Get value from SP memory */
    min_ePORT_minus_1 = PPA_FW_SP_SHORT_READ(MIN_E_PORT_MINUS_1_SP_ADDR);

    /* trg_ePort in targetPortEntry structure is split to keep 32 bits alignment for the core therefore we copy the whole 14 bits to temp field and will use it for comparison */
    cfg_trg_ePort = (thr14_u_mr2c_cfg_ptr->targetPortEntry.trg_ePort_13_11<<11) | (thr14_u_mr2c_cfg_ptr->targetPortEntry.trg_ePort_10_0);
    hdr_Trg_ePort = thr14_u_mr2c_pkt_in_ptr->eDSA_w2_ToAnalyzer.Trg_ePort;

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Copy Old_Header<eDSA<Target ePort>>-Min ePort-1 into Template<E-CID_base>    */
    /* First save the result in 16 bits reserved field so compiler will save it in simple 16 bits store instead of load/extract/mask/or etc. */
    /* then copy it to etag.E_CID_base by accelerator command */
    thr14_u_mr2c_cfg_ptr->srcPortEntry.reserved1 = hdr_Trg_ePort - min_ePORT_minus_1;
    /*#pragma no_reorder*/

    /* If Header<eDSA<Target ePort>>= Target Port ePort then */
    if (hdr_Trg_ePort == cfg_trg_ePort)
    {   /* Remove eDSA tag */

        /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 16 bytes such that Old_Header<eDSA-Tag> is removed */
        ACCEL_CMD_TRIG(COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 16 bytes therefore the size was reduced in 16 bytes. */
        thr14_u_mr2c_desc_ptr->fw_bc_modification = -16;
    }
    else
    {   /* Remove eDSA tag. Add ETAG and vlan tags  */

        /* Copy Min ePort-1 into New_Header<VLAN Tag<TPID>>; */
        thr14_u_mr2c_cfg_ptr->vlan.TPID = min_ePORT_minus_1;
        /*#pragma no_reorder*/

        /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 16 bytes therefore the size was reduced in 4 bytes. */
        thr14_u_mr2c_desc_ptr->fw_bc_modification = -4;
        /*#pragma no_reorder*/

        /* Copy Desc<TC> into Template<PCP> */    /* Copy Desc<DP> into Template<DEI> */
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET, 0);

        /* Copy Old_Header<eDSA<eVLAN[11:0]>> into New_Header<eVLAN[11:0]; */  /* Copy Old_Header<eDSA<UP>> into New_Header<VLAN Tag<PCP>>; */ /* Copy Old_Header<eDSA<CFI>> into New_Header<VLAN Tag<DEI>>; */
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__UP_LEN3_TO_CFG_vlan__up_OFFSET, 0);
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET, 0);
        /*#pragma no_reorder*/

        /* Copy Old_Header<eDSA<Target ePort>>-Min ePort-1 into Template<E-CID_base> */
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET, 0);
        /*#pragma no_reorder*/

        /* Copy Template into New_Header after New_Header<MAC DA> and New_Header<MAC SA>; */
        ACCEL_CMD_TRIG(COPY_BITS_THR14_U_MR2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET, 0);
        /* Copy E-Tag Template into New_Header after TPID; */
        ACCEL_CMD_TRIG(COPY_BYTES_THR14_U_MR2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET, 0);
        /* Copy VLAN Tag Template into New_Header after E-Tag; */
        ACCEL_CMD_TRIG(COPY_BYTES_THR14_U_MR2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET, 0);
        /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 4 bytes */
        ACCEL_CMD_TRIG(COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);
    }

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr14_u_mr2c_desc_ptr,EN_REC_THRD14_ECN,REC_THRD_ID_14);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr14_u_mr2c_desc_ptr,EN_REC_THRD14_PTP,REC_THRD_ID_14);
        }
    }

#if (EN_REC_THRD14_OUTPUT)
    record_data_thread_14_U_MR2C(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(14);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR15_QCN
 * inparam         None
 * return          None
 * description     DSA Port Extender with Channelized Flow Control Thread (QCN) Quantized Congestion Notification:
 *                 1. Pkt in includes MAC DA and MAC SA
 *                 2. HA template includes: mac source address, 8 bytes extended DSA fwd tag and cnm tpid
 *                 This thread builds a CNM message over 8B FWD DSA tag.
 *                 The value of the fields in the packet are taken from:    * The Template field in the Header Alteration table
 *                                                                          * Packet's Descriptor
 *                                                                          * Fixed bits set by the code
 *                 Provide a means for a bridge to notify a source of congestion causing the source to reduce the flow rate
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR15_QCN () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    /* Get pointers to cfg */
    struct thr15_qcn_cfg* thr15_qcn_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr15_qcn_cfg);

    /* First clear 64 bits to zero so it will be used to clear specific fields using accelerator commands */
    thr15_qcn_cfg_ptr->targetPortEntry.zero_0 = 0;
    thr15_qcn_cfg_ptr->targetPortEntry.zero_1 = 0;
    /*#pragma no_reorder*/

#if (EN_REC_THRD15)
    init_record_ptr();
    #if (EN_REC_THRD15_INPUT)
    record_data_thread_15_QCN(REC_INPUT);
    #endif
#endif

    /* MAC Destination Address Taken from the Source MAC Address of the original packet */
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET, 0);
    /* DSA tag<Src Port> Set to Descriptor<Congested Port Number> */
    ACCEL_CMD_TRIG(COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_port_number_LEN5_TO_CFG_qcn_ha_table__DSA_w0_SrcPort_4_0_OFFSET, 0);
    /*#pragma no_reorder*/
    /* Copy all 16 bytes of HA table[127:0] */
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_QCN_CFG_qcn_ha_table__sa_mac_47_32_LEN16_TO_PKT_mac_header__mac_sa_47_32_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Clear the following fields in packet out within qcn tag: Version,Reserved,CPID_63_32,CPID_31_16,CPID_15_3,Qoff,Qdelta,Encapsulated_priority, */
    /*                                                          Encapsulated_MAC_DA_47_32,Encapsulated_MAC_DA_31_16,Encapsulated_MAC_DA_15_0,Encapsulated_SDU_length */
    ACCEL_CMD_TRIG(COPY_BITS_THR15_QCN_CFG_targetPortEntry__zero_0_LEN10_TO_PKT_qcn_tag__Version_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN12_TO_PKT_qcn_tag__CPID_63_32_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN9_TO_PKT_qcn_tag__Encapsulated_priority_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Qfb in CNM message Set to Descriptor<QCN Q FB> */
    ACCEL_CMD_TRIG(COPY_BITS_THR15_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET, 0);
    /* CPID[2:0] in CNM message Set to Descriptor<Congested Queue Number> */
    ACCEL_CMD_TRIG(COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET, 0);
    /*#pragma no_reorder*/

#if (EN_REC_THRD15_OUTPUT)
    record_data_thread_15_QCN(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(15);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR16_U2E
 * inparam      None
 * return       None
 * description  Upstream Port to Extended Port (U2E) Thread:
 *                  1. Pkt in incldues MAC DA and MAC SA and 16B FORWARD DSA tag
 *                  2. HA template includes target port table, source port table and vlan tag in HA table
 *              Remove 16B FORWARD DSA tag and conditionally add vlan tag
 *              PIPE A1 addition !!!:
 *                  1. Egress Tag State: Read data shared ram to hold vlan ports bits map to check if vlan should be removed as well.
 *                  2. Multi Cast SRC filtering
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR16_U2E () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint8_t slowPathInd;
    uint32_t VlanEgressTagState,VlanBitSelect;

    /* Get pointers to cfg, header and descriptor */
    struct thr16_u2e_cfg* thr16_u2e_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr16_u2e_cfg);
    struct thr16_u2e_in_hdr* thr16_u2e_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr16_u2e_in_hdr);
    struct nonQcn_desc* thr16_u2e_desc_ptr        = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Addition for PIPE A1!!!: Egress Tag State -> Read from data shared ram vlan ports bits map. Use vlan id to point to the right entry. */
    VlanEgressTagState = PPA_FW_SHARED_DRAM_SHORT_READ(2*thr16_u2e_pkt_in_ptr->eDSA_fwd_w0.eVLAN_11_0);
    /* #pragma no_reorder */

#if (EN_REC_THRD16)
    init_record_ptr();
    #if (EN_REC_THRD16_INPUT)
    record_data_thread_16_U2E(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);

    /* Since trg_ePort and Src_ePort are split in different places need to concatenate them before using them for comparision  */
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_10_0_LEN11_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_PLUS3_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_13_11_LEN3_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_OFFSET, 0);
    /* #pragma no_reorder */
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS27_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS25_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS15_OFFSET, 0);
    /* #pragma no_reorder */

    /* Copy Old_Header<eDSA<VLAN, UP, CFI>> and Target Port VLAN TPID into New_Header<VLAN-Tag<VLAN, PCP, UP, TPID>>; */
    /* First copy it to cfg vlan template then later if required copy all template to new packet */
    /* NOTE: Need to put some barrier (around 5 cycles) between the Store and Load operations. Therefore below commands are placed here !!!*/
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR16_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET, 0);
    /* #pragma no_reorder */

    /* Multicast Source Filtering:  if (Target Port ePort = Old_Header<DSA[Src Port]>) */
    if (thr16_u2e_cfg_ptr->HA_Table_Target_ePort.Target_ePortFull == thr16_u2e_cfg_ptr->cfgReservedSpaceWithSrc_ePort.Src_ePortFull)
    { /* Mark the descriptor to be dropped */
        /* #pragma frequency_hint NEVER */
        /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
        thr16_u2e_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Addition for PIPE A1 !!! Set vlan bit select according to target port number that is used by this thread to extract the specific target port */
        VlanBitSelect      =  thr16_u2e_cfg_ptr->targetPortEntry.TargetPort;
        /* Update vlan egress tag state according to vlan ports bits map in shared ram */
        VlanEgressTagState =  (0x1 << VlanBitSelect) & VlanEgressTagState ;

        /* If VLAN Egress Tag State = 0 */
        if  (VlanEgressTagState == 0)
        {   /* Remove 16B eDSA tag, VLAN tag is not added */
            /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 16 bytes therefore the size was reduced in 16 bytes. */
            thr16_u2e_desc_ptr->fw_bc_modification = -16;
            /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 16 bytes such that Old_Header<eDSA> is removed */
            ACCEL_CMD_TRIG(COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET, 0);
        }
        else
        {   /* Remove 16B eDSA tag, add 4B VLAN tag */
            /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 12 bytes therefore the size was reduced in 12 bytes. */
            thr16_u2e_desc_ptr->fw_bc_modification = -12;
            /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 12 bytes right */
            ACCEL_CMD_TRIG(COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET, 0);
            /* #pragma no_reorder */
            /* Copy vlan template into new output header packet */
            ACCEL_CMD_TRIG(COPY_BYTES_THR16_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET, 0);
        }

        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /* #pragma frequency_hint NEVER */

            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr16_u2e_desc_ptr,EN_REC_THRD16_ECN,REC_THRD_ID_16);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                /* Clear last 4 bytes of configuration table so it can be used in PTP handler */
                thr16_u2e_cfg_ptr->cfgReservedSpaceWithSrc_ePort.Src_ePortFull = 0;
                PTP_handler(thr16_u2e_desc_ptr,EN_REC_THRD16_PTP,REC_THRD_ID_16);
            }
        }
    }

#if (EN_REC_THRD16_OUTPUT)
    record_data_thread_16_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(16);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR17_U2IPL
 * inparam         None
 * return          None
 * description     PIPE A1 addition !!!  Upstream Port to Inter Pipe Links Port (U2IPL) Thread:
 *                 1. Pkt in includes MAC DA and MAC SA and E TAG type
 *                 2. Config includes only target port table and source port table No tag in HA table
 *                 New thread that only set a single bit in the E-Tag. Set the IPL Direction bit.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR17_U2IPL () {
    /* xt_iss_client_command("profile","disable"); */
    /* _exit(0);                                   */
    /* xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct thr17_u2ipl_in_hdr* thr17_u2ipl_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr17_u2ipl_in_hdr);
    struct nonQcn_desc* thr17_u2ipl_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD17)
    init_record_ptr();
    #if (EN_REC_THRD17_INPUT)
    record_data_thread_17_U2IPL(REC_INPUT);
    #endif
#endif

    /* Get slow path indication */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    /* Addition for PIPE A1 !!! */
       __LOG_FW(("Inter PIPE Link (IPL): IPL is defined as a non-standard horizontal link between two PIPE devices which act as a single standard Port Extender."
       "In order to support these links and keep standard behavior on the Port Extender external links, we add a <IPL Direction> field to the standard E-Tag."
       "Set IPL Direction bit \n"));
    thr17_u2ipl_pkt_in_ptr->etag.IPL_Direction = 0x1;

    __LOG_PARAM_FW(slowPathInd);
    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr17_u2ipl_desc_ptr,EN_REC_THRD17_ECN,REC_THRD_ID_17);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr17_u2ipl_desc_ptr,EN_REC_THRD17_PTP,REC_THRD_ID_17);
        }
    }

#if (EN_REC_THRD17_OUTPUT)
    record_data_thread_17_U2IPL(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(17);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/*******************************************************************************************************************************************************//**
 * funcname     THR18_IPL2IPL
 * inparam      None
 * return       None
 * description  PIPE A1 addition !!! IPL Port to IPL Port (IPL2IPL) Thread:
 *              1. Pkt in includes MAC DA and MAC SA. No tag attached.
 *              2. Config includes only target port table and source port table No tag in HA table.
 *              New thread which checks the source filtering bitvector status and drop the packet if bit is on according to target port number.
 *              Inter PIPE Link (IPL): IPL is defined as a non-standard horizontal link between 2 PIPE devices which act as a single standard Port Extender.
 *              The Source Filtering Bitvector represents the target port bitvector and defines, per source port, the target ports which should be filtered
 *              in the Inter PIPE Link to Inter PIPE Link (IPL2IPL) Thread. The CPU port (16) cannot be filtered and will never trigger the IPL2IPL thread.
 *************************************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR18_IPL2IPL () {
    /* xt_iss_client_command("profile","disable"); */
    /* _exit(0);                                   */
    /* xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd,TargetPortNumber;
    uint32_t srcFilteringBitVectorValue,srcFilteringBitVectorStatus;

    /* Get pointer to descriptor */
    struct thr18_ipl2ipl_cfg* thr18_ipl2ipl_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr18_ipl2ipl_cfg);
    struct nonQcn_desc* thr18_ipl2ipl_desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD18)
    init_record_ptr();
    #if (EN_REC_THRD18_INPUT)
    record_data_thread_18_IPL2IPL(REC_INPUT);
    #endif
#endif

    __LOG_FW((" Read value of source filtering bitvector from source port table  \n"));
    srcFilteringBitVectorValue = thr18_ipl2ipl_cfg_ptr->srcPortEntry.Source_Filtering_Bitvector;
    __LOG_PARAM_FW(srcFilteringBitVectorValue);

    __LOG_FW((" Get the required target port number  \n"));
    TargetPortNumber =  thr18_ipl2ipl_cfg_ptr->targetPortEntry.TargetPort;
    __LOG_PARAM_FW(TargetPortNumber);

    __LOG_FW((" According to the port number check this bit location in source filtering bit vector \n"));
    srcFilteringBitVectorStatus = srcFilteringBitVectorValue & (0x1 << TargetPortNumber);
    __LOG_PARAM_FW(srcFilteringBitVectorStatus);


    __LOG_FW((" If bit is '1'' then drop the packet. If Source Port Table[27:12] - Source Filtering Bitvector[Target Port Table[19:16] - Target Port] = 1 \n"));
    if ( srcFilteringBitVectorStatus !=0 )
    {   __LOG_FW((" physical port source filtering - packet arriving on an IPL should not go back to same IPL (including any member of the same LAG) \n"));
        __LOG_FW((" Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' \n"));
        thr18_ipl2ipl_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Get slow path indication */
        slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
        /*#pragma no_reorder*/

        __LOG_PARAM_FW(slowPathInd);
        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/

            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr18_ipl2ipl_desc_ptr,EN_REC_THRD18_ECN,REC_THRD_ID_18);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr18_ipl2ipl_desc_ptr,EN_REC_THRD18_PTP,REC_THRD_ID_18);
            }
        }
    }

#if (EN_REC_THRD18_OUTPUT)
    record_data_thread_18_IPL2IPL(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(18);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/*************************************************************************************************************************//**
 * funcname        THR19_E2U_Untagged
 * inparam         None
 * return          None
 * description     Extended Port to Upstream Port for Untagged VLAN packets (E2U-Untagged) Thread:
 *                 New thread added to PIPE A1 package!!!
 *                 1. Pkt in includes MAC DA and MAC SA
 *                 2. Config HA table includes E TAG header type + VLAN_Tag_TPID. Source table includes also default vlan tag
 *                 Move MAC left 12 bytes and add TPID, E TAG and vlan tag
 *                 Copy Uplink Port indication bit to IPL Direction bit in E-tag
 *****************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR19_E2U_Untagged () {
    /* xt_iss_client_command("profile","disable"); */
    /* _exit(0);                                   */
    /* xt_iss_client_command("profile","enable");    */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr19_e2u_untagged_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD19)
    init_record_ptr();
    #if (EN_REC_THRD19_INPUT)
    record_data_thread_19_E2U_Untagged(REC_INPUT);
    #endif
#endif

    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
    /*#pragma no_reorder*/

    __LOG_FW((" Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 12 bytes left \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR19_E2U_Untagged_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET, 0);
    __LOG_FW((" Copy Source Port Table[0:11] - PCID to Template<E-CID_base> \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__PCID_LEN12_TO_CFG_HA_Table_Upstream_Ports__E_CID_base_OFFSET, 0);
    __LOG_FW((" Copy Desc<UP> and Desc<DEI> into Template<E-PCP> and Template<E-DEI> respectively \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_E2U_Untagged_DESC_nonQcn_desc__up0_LEN4_TO_CFG_HA_Table_Upstream_Ports__E_PCP_OFFSET, 0);
    __LOG_FW((" Copy Target Port Table[15:0] - TPID after New_Header<MAC SA>  \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_E2U_Untagged_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,0);
    __LOG_FW((" Copy Source Port Table[28] - Uplink Port to Old_Header<E-Tag>[IPL Direction]"
       "Setting <IPL Direction> for CPU traffic arriving from uplink port. Single bit indicating whether the source port is an uplink port. \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_HA_Table_Upstream_Ports__IPL_Direction_OFFSET, 0);
    /*#pragma no_reorder*/
    __LOG_FW((" Copy Template into New_Header<E-Tag value>  (etag 6B + VLAN TPID 2B) \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR19_E2U_Untagged_CFG_HA_Table_Upstream_Ports__E_PCP_LEN8_TO_PKT_etag__E_PCP_OFFSET, 0);
    __LOG_FW((" Copy Source Port Table[27:12] - Default VLAN Tag value after the VLAN Tag TPID \n"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Default_VLAN_Tag_LEN16_TO_PKT_vlan__up_OFFSET, 0);

    __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 12 bytes therefore the size was increment in 12 bytes. \n"));
    thr19_e2u_untagged_desc_ptr->fw_bc_modification = 12;

    __LOG_PARAM_FW(slowPathInd);
    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /*#pragma frequency_hint NEVER*/

        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr19_e2u_untagged_desc_ptr,EN_REC_THRD19_ECN,REC_THRD_ID_19);
        }
        /*#pragma no_reorder*/

        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr19_e2u_untagged_desc_ptr,EN_REC_THRD19_PTP,REC_THRD_ID_19);
        }
    }

#if (EN_REC_THRD19_OUTPUT)
    record_data_thread_19_E2U_Utagged(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(19);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR20_U2E_M4
 * inparam      None
 * return       None
 * description  Upstream Port to Extended Port for Multicast with 4 PCIDs (U2E-M4) Thread:
 *              1. Pkt in includes MAC DA and MAC SA, ETAG and VLAN tag
 *              2. Config includes target port table and source port table and HA table with 7 additional Extended Ports PCIDs
 *              New thread added to PIPE A1 package!!!
 *              Remove E-Tag (8B) + check if to remove vlan tag (4B) according to Egress Tag state
 *              1. Egress Tag State: Read data shared ram to hold vlan ports bits map to check if vlan should be removed as well.
 *              2. MultiCast SRC filtering with 4 PCIDs
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR20_U2E_M4 () {
     /*xt_iss_client_command("profile","disable"); */
     /*_exit(0);                                   */
     /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    uint32_t VlanEgressTagState,TargetPortVlanBitSelect;

    /* Get pointers to header, cfg and descriptor */
    struct thr20_u2e_m4_in_hdr* thr20_u2e_m4_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr20_u2e_m4_in_hdr);
    struct thr20_u2e_m4_cfg* thr20_u2e_m4_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr20_u2e_m4_cfg);
    struct nonQcn_desc* thr20_u2e_m4_desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    __LOG_FW((" Egress Tag State -> Read from data shared ram vlan ports bits map. Use vlan id to point to the right entry. \n"));
    __LOG_PARAM_FW(thr20_u2e_m4_pkt_in_ptr->vlan.vid);
    VlanEgressTagState = PPA_FW_SHARED_DRAM_SHORT_READ(2*thr20_u2e_m4_pkt_in_ptr->vlan.vid);
    __LOG_PARAM_FW(VlanEgressTagState);
    /*#pragma no_reorder*/

#if (EN_REC_THRD20)
    init_record_ptr();
    #if (EN_REC_THRD20_INPUT)
    record_data_thread_20_U2E_M4(REC_INPUT);
    #endif
#endif

    __LOG_FW((" check for MultiCast SRC filtering with 4 PCIDs  \n"));
    __LOG_PARAM_FW(thr20_u2e_m4_cfg_ptr->targetPortEntry.PCID);
    __LOG_PARAM_FW(thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1);
    __LOG_PARAM_FW(thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2);
    __LOG_PARAM_FW(thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3);
    __LOG_PARAM_FW(thr20_u2e_m4_pkt_in_ptr->etag.Ingress_E_CID_base);
    if ( (thr20_u2e_m4_cfg_ptr->targetPortEntry.PCID                   == thr20_u2e_m4_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1 == thr20_u2e_m4_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2 == thr20_u2e_m4_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr20_u2e_m4_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3 == thr20_u2e_m4_pkt_in_ptr->etag.Ingress_E_CID_base) )
    {
        /*#pragma frequency_hint NEVER*/

        __LOG_FW(("Trunk src filtering \n"));
        __LOG_FW((" Drop the packet. Indicates it by setting fw_drop field in descriptor to '1' (Ingress_E_CID_base==PCID/ExtPort_PCID_1/ExtPort_PCID_2/ExtPort_PCID_3)\n"));
        thr20_u2e_m4_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Get slow path indication value */
        slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
        __LOG_FW((" Set vlan bit select according to target port number that is used by this thread to extract the specific target port. NOTE: CPU port (port 16) is not used by the U2E thread \n"));
        TargetPortVlanBitSelect =  thr20_u2e_m4_cfg_ptr->targetPortEntry.TargetPort;
        __LOG_PARAM_FW(TargetPortVlanBitSelect);

        __LOG_FW((" Update vlan egress tag state according to vlan ports bits map in shared ram \n"));
        VlanEgressTagState =  (0x1 << TargetPortVlanBitSelect) & VlanEgressTagState ;
        __LOG_PARAM_FW(VlanEgressTagState);

        __LOG_PARAM_FW(thr20_u2e_m4_pkt_in_ptr->vlan.TPID);
        __LOG_PARAM_FW(thr20_u2e_m4_cfg_ptr->targetPortEntry.TPID);
        __LOG_FW((" If Old_Header<TPID after E-Tag>= Target Port Table[15:0] - TPID and VlanEgressTagState == 0 \n"));
        if ( (thr20_u2e_m4_pkt_in_ptr->vlan.TPID == thr20_u2e_m4_cfg_ptr->targetPortEntry.TPID) && (VlanEgressTagState == 0) )
        { __LOG_FW((" Remove E-Tag and VLAN Tag \n"));

            __LOG_FW((" Move Old_Header<MAC DA> and Old_Header<MAC SA> 12 bytes such that ETAG(8B) and VLAN-Tag(4B) are removed \n"));
            ACCEL_CMD_TRIG(COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET, 0);
            __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right total 12 bytes therefore the size was reduced in 12 bytes. \n"));
            thr20_u2e_m4_desc_ptr->fw_bc_modification = -12;
        }
        else
        { __LOG_FW((" Remove E-Tag and leave VLAN Tag \n"));

            __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 8 bytes therefore the size was reduced in 8 bytes. \n"));
            thr20_u2e_m4_desc_ptr->fw_bc_modification = -8;
            __LOG_FW((" Remove E-Tag: Move Old_Header<MAC DA> and Old_Header<MAC SA> 8 bytes such that Old_Header<E-Tag> is removed; \n"));
            ACCEL_CMD_TRIG(COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET, 0);
        }

        __LOG_PARAM_FW(slowPathInd);
        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/
            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr20_u2e_m4_desc_ptr,EN_REC_THRD20_ECN,REC_THRD_ID_20);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr20_u2e_m4_desc_ptr,EN_REC_THRD20_PTP,REC_THRD_ID_20);
            }
        }
    }

#if (EN_REC_THRD20_OUTPUT)
    record_data_thread_20_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(20);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR21_U2E_M8
 * inparam      None
 * return       None
 * description  Upstream Port to Extended Port for Multicast with 8 PCIDs (U2E-M8) Thread:
 *              1. Pkt in includes MAC DA and MAC SA, ETAG and VLAN tag
 *              2. Config includes target port table and source port table and HA table with 7 additional Extended Ports PCIDs
 *              New thread added to PIPE A1 package!!!
 *              Remove E-Tag (8B) + check if to remove vlan tag (4B) according to Egress Tag state
 *              1. Egress Tag State: Read data shared ram to hold vlan ports bits map to check if vlan should be removed as well.
 *              2. MultiCast SRC filtering with 8 PCIDs
 *              Does bot have to apply
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR21_U2E_M8 () {
     /*xt_iss_client_command("profile","disable"); */
     /*_exit(0);                                   */
     /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    uint32_t VlanEgressTagState,TargetPortVlanBitSelect;

    /* Get pointers to header, cfg and descriptor */
    struct thr21_u2e_m8_in_hdr* thr21_u2e_m8_pkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr21_u2e_m8_in_hdr);
    struct thr21_u2e_m8_cfg* thr21_u2e_m8_cfg_ptr       = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr21_u2e_m8_cfg);
    struct nonQcn_desc* thr21_u2e_m8_desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Addition for PIPE A1!!!:*/__LOG_FW((" Egress Tag State -> Read from data shared ram vlan ports bits map. Use vlan id to point to the right entry. \n"));
    VlanEgressTagState = PPA_FW_SHARED_DRAM_SHORT_READ(2*thr21_u2e_m8_pkt_in_ptr->vlan.vid);
    __LOG_PARAM_FW(VlanEgressTagState);
    /*#pragma no_reorder*/

#if (EN_REC_THRD21)
    init_record_ptr();
    #if (EN_REC_THRD21_INPUT)
    record_data_thread_21_U2E_M8(REC_INPUT);
    #endif
#endif

    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->targetPortEntry.PCID          );
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_4);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_5);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_6);
    __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_7);
    __LOG_PARAM_FW(thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base);
    /* Addition for PIPE A1 !!! MultiCast SRC filtering  */
    if ( (thr21_u2e_m8_cfg_ptr->targetPortEntry.PCID                   == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_4 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_5 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_6 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) || \
         (thr21_u2e_m8_cfg_ptr->HA_Table_Extended_Ports.ExtPort_PCID_7 == thr21_u2e_m8_pkt_in_ptr->etag.Ingress_E_CID_base) )
    {
        /*#pragma frequency_hint NEVER*/

        __LOG_FW(("Trunk src filtering \n"));
        __LOG_FW((" Drop the packet. Indicates it by setting fw_drop field in descriptor to '1' (Ingress_E_CID_base==PCID/ExtPort_PCID_1/ExtPort_PCID_2/ExtPort_PCID_3/..7)\n"));
        thr21_u2e_m8_desc_ptr->fw_drop = 1;
    }
    else
    {
        /* Get slow path indication value */
        slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);
        /* Addition for PIPE A1 !!! */ __LOG_FW(("Set vlan bit select according to target port number that is used by this thread to extract the specific target port"
           "NOTE: CPU port (port 16) is not used by the U2E thread \n"));
        TargetPortVlanBitSelect =  thr21_u2e_m8_cfg_ptr->targetPortEntry.TargetPort;
        __LOG_PARAM_FW(TargetPortVlanBitSelect);
        __LOG_FW((" Update vlan egress tag state according to vlan ports bits map in shared ram \n"));
        VlanEgressTagState =  (0x1 << TargetPortVlanBitSelect) & VlanEgressTagState ;
        __LOG_PARAM_FW(VlanEgressTagState);

        __LOG_PARAM_FW(thr21_u2e_m8_pkt_in_ptr->vlan.TPID);
        __LOG_PARAM_FW(thr21_u2e_m8_cfg_ptr->targetPortEntry.TPID);
        __LOG_FW((" If Old_Header<TPID after E-Tag>= Target Port Table[15:0] - TPID and VlanEgressTagState == 0 \n"));
        if ( (thr21_u2e_m8_pkt_in_ptr->vlan.TPID == thr21_u2e_m8_cfg_ptr->targetPortEntry.TPID) && (VlanEgressTagState == 0) )
        { __LOG_FW((" Remove E-Tag and VLAN Tag \n"));

            __LOG_FW((" Move Old_Header<MAC DA> and Old_Header<MAC SA> 12 bytes such that E-Tag amd VLAN-Tag are removed \n"));
            ACCEL_CMD_TRIG(COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET, 0);
            __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right total 12 bytes therefore the size was reduced in 12 bytes. \n"));
            thr21_u2e_m8_desc_ptr->fw_bc_modification = -12;
        }
        else
        { __LOG_FW((" Remove E-Tag only and leave VLAN Tag \n"));

            __LOG_FW((" Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 8 bytes therefore the size was reduced in 8 bytes. \n"));
            thr21_u2e_m8_desc_ptr->fw_bc_modification = -8;
            __LOG_FW((" Remove E-Tag: Move Old_Header<MAC DA> and Old_Header<MAC SA> 8 bytes such that Old_Header<E-Tag> is removed; \n"));
            ACCEL_CMD_TRIG(COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET, 0);
        }

        __LOG_PARAM_FW(slowPathInd);
        /*---------------------------------------------------
         * Check if slow path is required
         * --------------------------------------------------*/
        /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
        if (slowPathInd != 0)
        {
            /*#pragma frequency_hint NEVER*/
            /* Check if this is ECN case */
            if(ECN_INDICATION_IS_ON(slowPathInd))
            {   /* Handle ECN case */
                ECN_handler(thr21_u2e_m8_desc_ptr,EN_REC_THRD21_ECN,REC_THRD_ID_21);
            }
            /* Check if this is PTP case */
            if(PTP_INDICATION_IS_ON(slowPathInd))
            {   /* Handle PTP event */
                PTP_handler(thr21_u2e_m8_desc_ptr,EN_REC_THRD21_PTP,REC_THRD_ID_21);
            }
        }
    }

#if (EN_REC_THRD21_OUTPUT)
    record_data_thread_21_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(21);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname        THR22_Discard_pkt
 * inparam         None
 * return          None
 * description     Mark the descriptor to drop packet
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR22_Discard_pkt () {
    /*xt_iss_client_command("profile","disable");*/
    /*_exit(0);                                  */
    /*xt_iss_client_command("profile","enable"); */

    /* Get pointer to descriptor */
    struct nonQcn_desc* thr22_Discard_pkt_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD22)
    init_record_ptr();
    #if (EN_REC_THRD22_INPUT)
    record_data_thread_22_Discard_pkt(REC_INPUT);
    #endif
#endif

    __LOG_FW((" Drop the packet. Indicates it by setting fw_drop field in descriptor to '1' \n"));
    thr22_Discard_pkt_desc_ptr->fw_drop = 1;

#if (EN_REC_THRD22_OUTPUT)
    record_data_thread_22_Discard_pkt(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(22);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR23_EVB_E2U
 * inparam      None
 * return       None
 * description  EVB (Edge Virtual Bridging) use case. VLAN tag as forwarding tag is described in IEEE 802.1Qbg
 *              Extended Port to Upstream Port (E2U) Thread:
 *              1. Pkt in includes MAC DA and MAC SA
 *              2. Config includes target port table and source port table
 *              New thread added for demo for Ciena !!!
 *              Add VLAN tag.
 *              Ciena is very interested in PIPE for a optical port extender application.
 *              The catch is that it must work with a Broadcom device that does not support  802.1br (Tomahawk3).
 *              We are thinking a crude VLAN scheme would suffice.
 *              10G->100G- The PIPE should push a VLAN tag indicating which 10G port a packet came in on (1-12)
 *              100G-> 10G port - The PIPE should read/pop the outer tag and send to that PORT  (1-12)
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR23_EVB_E2U () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0);                                   */
    /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr23_evb_e2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD23)
    init_record_ptr();
    #if (EN_REC_THRD23_INPUT)
    record_data_thread_23_evb_E2U(REC_INPUT);
    #endif
#endif

    /* Get slow path indication value */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);

    /* Copy Desc<UP> and Desc<DEI> into New Header<PCP> and New Header<DEI> respectevely */
    ACCEL_CMD_TRIG(COPY_BITS_THR23_EVB_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_vlan__up_OFFSET, 0);
    /* Copy Source Port Table[0:11] - VID into New Header<VID>; */
    ACCEL_CMD_TRIG(COPY_BITS_THR23_EVB_E2U_CFG_srcPortEntry__PVID_LEN12_TO_CFG_vlan__vid_OFFSET, 0);
    /* Copy Target Port Table[15:0] - TPID into New_Header<TPID> */
    ACCEL_CMD_TRIG(COPY_BITS_THR23_EVB_E2U_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET, 0);
    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 4 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR23_EVB_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET, 0);
    /* #pragma no_reorder */
    /* Copy vlan template into new output header packet */
    ACCEL_CMD_TRIG(COPY_BYTES_THR23_EVB_E2U_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 4 bytes therefore the size was increased by 4 bytes */
    thr23_evb_e2u_desc_ptr->fw_bc_modification = 4;

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /* #pragma frequency_hint NEVER */
        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr23_evb_e2u_desc_ptr,EN_REC_THRD23_ECN,REC_THRD_ID_23);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr23_evb_e2u_desc_ptr,EN_REC_THRD23_PTP,REC_THRD_ID_23);
        }
    }

#if (EN_REC_THRD23_OUTPUT)
    record_data_thread_23_evb_E2U(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(23);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname     THR24_EVB_U2E
 * inparam      None
 * return       None
 * description  EVB (Edge Virtual Bridging) use case. VLAN tag as forwarding tag is described in IEEE 802.1Qbg
 *              Upstream Port to Extended Port (U2E) Thread
 *              1. Pkt in includes MAC DA and MAC SA and VLAN tag
 *              2. Config includes target port table and source port table
 *              New thread added for demo for Ciena !!!
 *              Remove VLAN tag.
 *              Ciena is very interested in PIPE for a optical port extender application.
 *              The catch is that it must work with a Broadcom device that does not support  802.1br (Tomahawk3).
 *              We are thinking a crude VLAN scheme would suffice.
 *              10G->100G- The PIPE should push a VLAN tag indicating which 10G port a packet came in on (1-12)
 *              100G-> 10G port - The PIPE should read/pop the outer tag and send to that PORT  (1-12)
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR24_EVB_U2E () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0);                                   */
    /*xt_iss_client_command("profile","enable");  */

    uint8_t slowPathInd;
    /* Get pointer to descriptor */
    struct nonQcn_desc* thr24_evb_u2e_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

#if (EN_REC_THRD24)
    init_record_ptr();
    #if (EN_REC_THRD24_INPUT)
    record_data_thread_24_evb_U2E(REC_INPUT);
    #endif
#endif

    /* Get slow path indication value */
    slowPathInd = PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET);

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift left 4 bytes therefore the size was increased by 4 bytes */
    thr24_evb_u2e_desc_ptr->fw_bc_modification = -4;

    /* Copy Old_Header<MAC DA> and Old_Header<MAC SA> into New_Header<MAC DA> and New_Header<MAC SA> respectively with offset of 4 bytes right */
    ACCEL_CMD_TRIG(COPY_BYTES_THR24_EVB_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET, 0);

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (slowPathInd != 0)
    {
        /* #pragma frequency_hint NEVER */
        /* Check if this is ECN case */
        if(ECN_INDICATION_IS_ON(slowPathInd))
        {   /* Handle ECN case */
            ECN_handler(thr24_evb_u2e_desc_ptr,EN_REC_THRD24_ECN,REC_THRD_ID_24);
        }
        /* Check if this is PTP case */
        if(PTP_INDICATION_IS_ON(slowPathInd))
        {   /* Handle PTP event */
            PTP_handler(thr24_evb_u2e_desc_ptr,EN_REC_THRD24_PTP,REC_THRD_ID_24);
        }
    }


#if (EN_REC_THRD24_OUTPUT)
    record_data_thread_24_evb_U2E(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(24);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}

/********************************************************************************************************************//**
 * funcname     THR25_EVB_QCN
 * inparam      None
 * return       None
 * description  EVB (Edge Virtual Bridging) QCN (Quantized Congestion Notification) use case. QCN over VLAN tag.
 *              1. Pkt in includes MAC DA and MAC SA
 *              2. Config includes
 *                 - HA table: MAC SA (6B), QCN EtherType(2B), reserved_0(2B), reserved_1(2B)
 *                 - source port table: not in use
 *                 - target port table: VLAN tag TPID(2B)
 *              Description:
 *              This thread builds a CNM message over 4 bytes VLAN tag. Provide a means for a bridge to notify a source
 *              of congestion causing the source to reduce the flow rate.
 *              Packet output format:
 *               ------------------------------------------------------
 *              | MAC DA & MAC SA (12B) | VLAN tag(4B) | QCN header(25)|
 *               ------------------------------------------------------
 *              MAC DA:             get it from origin MAC SA
 *              MAC SA:             get it from HA table
 *              VLAN TPID:          get it from target port table
 *              VLAN UP, CFI & VID: get it from data shared memory
 *              QCN[EtherType]:     get it from HA table
 *              QCN[qfb]:           get it from Descriptor<QCN Q FB>
 *              QCN[CPID[2:0]]:     get it from Descriptor<Congested Queue Number>
 *              QCN rest fields should all be cleared to zero
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR25_EVB_QCN () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0);                                   */
    /*xt_iss_client_command("profile","enable");  */

    /* VLAN tag value should be set with {UP[2:0],CFI,VID[11:0]} */
    uint16_t  qcnVlanTagVal;
    uint16_t  qcnVlanTagTblEntry;
    uint32_t  qcnVlanTagAddr;

    /* Get pointer to QCN descriptor */
    struct Qcn_desc* thr25_evb_qcn_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,Qcn_desc);

#if (EN_REC_THRD25)
    init_record_ptr();
    #if (EN_REC_THRD25_INPUT)
    record_data_thread_25_evb_QCN(REC_INPUT);
    #endif
#endif

    /*
    For building the QCN vlan tag, a new table should be added in the PHA shared memory space QcnVlanTbl.
    The table is indexed by the PHA using the Descriptor<Congested Port Number>.
    The entry content is as follows: ENTRY[15:0] = {UP[2:0],CFI,VID[11:0]}
    The table is 17 entires (ports 0-16), thus total 17x16b = 272b
    */
    __LOG_FW(("Read 16 bits of vlan tag fields from table in data shared memory while Desc<congested_port_number> is used as the index "));
    __LOG_PARAM_FW(thr25_evb_qcn_desc_ptr->congested_port_number);
    qcnVlanTagTblEntry =  2*thr25_evb_qcn_desc_ptr->congested_port_number; /* Each entry 2 bytes */
    qcnVlanTagVal = PPA_FW_SHARED_DRAM_SHORT_READ(DRAM_QCN_VLAN_TBL_OFST + qcnVlanTagTblEntry);
    __LOG_PARAM_FW(qcnVlanTagVal);
    /*#pragma no_reorder*/

    /* Should be placed in packet out after VLAN TPID field: packet register file address:0x2200 + expansion space:20B + mac DA SA:12B + vlan TPID field:2B  */
    qcnVlanTagAddr = PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + MAC_ADDR_SIZE + 2 ;
    __LOG_PARAM_FW(qcnVlanTagAddr);
    __LOG_FW(("Write vlan tag fields (16bits) into packet. Should be placed after VLAN TPID field"));
    PPA_FW_SP_SHORT_WRITE(qcnVlanTagAddr, qcnVlanTagVal);

    /* Set packet MAC DA, take it from MAC SA of the original packet */
    __LOG_FW(("Set packet MAC DA, take it from MAC SA of the original packet"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR25_EVB_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Set packet MAC SA, take it from HA table[127:80] */
    __LOG_FW(("Set packet MAC SA, take it from HA table[127:80]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__sa_mac_47_32_LEN6_TO_PKT_mac_header__mac_sa_47_32_OFFSET, 0);

    /* Set packet VLAN TPID, take it from Target Port Table[63:48] */
    __LOG_FW(("Set packet VLAN TPID, take it from Target Port Table[63:48]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_EVB_QCN_CFG_targetPortEntry__TPID_LEN16_TO_PKT_vlan__TPID_OFFSET, 0);

    /* Set packet QCN[EtherType], take it from HA table[79:64] + 8 bytes of zeros taken from HA table[63:0]  */
    __LOG_FW(("Set packet QCN[EtherType], take it from HA table[79:64] + 8 bytes of zeros taken from HA table[63:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__EtherType_LEN10_TO_PKT_qcn_tag__EtherType_OFFSET, 0);

    /* Clear to zero next 8 bytes of QCN tag */
    __LOG_FW(("Clear to zero next 8 bytes of QCN tag "));
    ACCEL_CMD_TRIG(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN8_TO_PKT_qcn_tag__EtherType_PLUS10_OFFSET, 0);

    /* Clear to zero the last 7 bytes of QCN tag */
    __LOG_FW(("Clear to zero the last 7 bytes of QCN tag "));
    ACCEL_CMD_TRIG(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN7_TO_PKT_qcn_tag__EtherType_PLUS18_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Set packet QCN[Qfb], take it from Descriptor<QCN Q FB>. Fb is a measure of the congestion level. qFb is a quantization of Fb (feedback)*/
    __LOG_FW(("Set packet QCN[Qfb], take it from Descriptor<QCN Q FB>. Fb is a measure of the congestion level. qFb is a quantization of Fb (feedback)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET, 0);

    /* Set packet QCN[ CPID[2:0] ], take it from Descriptor<Congested Queue Number> */
    __LOG_FW(("Set packet QCN[ CPID[2:0] ], take it from Descriptor<Congested Queue Number>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET, 0);


#if (EN_REC_THRD25_OUTPUT)
    record_data_thread_25_evb_QCN(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(25);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR26_PRE_DA_U2E
 * inparam      None
 * return       None
 * description  Support 2 bytes pre tag MAC DA Upstream to Extended use case
 *              Cisco is using a proprietary 2B tag before the MACDA to specify the source/target info between PIPE and their NPU
 *              On the Downstream (NPU->PIPE), port represents “target”
 *              HA template: | reserved(11B) | ip_protocol(1B) | udp_dest_port(2B) | domain_number(1B) | ptp_version(4b) | message_type(4b) |
 *              src port entry table(4B):  | reserved(1B) | pre_da_port_info(1B) | reserved(13b) | ptpOverMplsEn(1b) | ptp_mode(2b) |
 *              target port entry table(8B): | TPID 2B | PVID 12b | VlanEgrTagState 1b | reserved 3b | PCID 12b | Egress_Pipe_Delay 20b |
 *              hdr in:  | Extension space(20B) | pre_da_tag(2B) | MAC DA(6B) | MAC SA (6B).....
 *              hdr out: | Extension space(22B) | MAC DA(6B) | MAC SA (6B).....
 *              Firmware removes the 2 bytes pre da tag part
 *              Firmware completes PTP packet classification and check if PTP should be executed
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR26_PRE_DA_U2E () {
    /*xt_iss_client_command("profile","enable"); */

    /* Get pointers to descriptor and cfg */
    struct nonQcn_desc* thr26_pre_da_u2e_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
    struct thr26_pre_da_u2e_cfg* thr26_pre_da_u2e_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr26_pre_da_u2e_cfg);

    uint32_t    ptp_addr;
    uint32_t    udp_addr;
    uint32_t    ip_addr;
    uint16_t   pkt_ptp_first_2_bytes;
    uint8_t    pkt_ptp_version;
    uint8_t    pkt_ptp_messageType;
    uint8_t    pkt_ptp_domain_number;
    uint16_t   pkt_udp_dest_port;
    uint8_t    pkt_ip_protocol;
    uint32_t   pkt_ptp_fields;
    uint32_t   ha_ptp_fields;


#if (EN_REC_THRD26)
    init_record_ptr();
    #if (EN_REC_THRD26_INPUT)
    record_data_thread_26_pre_da_u2e(REC_INPUT);
    #endif
#endif


    /* Need to remove the 2 bytes before MAC DA. Do it by setting Desc<fw_bc_modification> to -2B indicating HW about the new starting point of the packet header */
    __LOG_FW(("Need to remove the 2 bytes before MAC DA. Do it by setting Desc<fw_bc_modification> to -2B indicating HW about the new starting point of the packet header \n"));
    thr26_pre_da_u2e_desc_ptr->fw_bc_modification = -2;

    /* Check if PTP indication is on */
    __LOG_FW(("Check if PTP indication is on \n"));
    __LOG_PARAM_FW(thr26_pre_da_u2e_desc_ptr->is_ptp);
    __LOG_PARAM_FW(thr26_pre_da_u2e_desc_ptr->outer_l3_offset);
    if (thr26_pre_da_u2e_desc_ptr->is_ptp)
    {
        /* desc<is_ptp> is on therefore check ptp packet type */
        __LOG_FW(("desc<is_ptp> is on therefore check ptp packet type \n"));
        switch(thr26_pre_da_u2e_desc_ptr->ptp_pkt_type_idx)
        {
            case PTP_OVER_L2_TYPE:  /* #1 | L2(12B) | ET(2B) | PTP(34B) | */
                 __LOG_FW((" #1 | L2(12B) | ET(2B) | PTP(34B) |  \n"));

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 2);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Extract the first nibble which should hold versionPtp field */
                __LOG_FW(("Extract the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Extract the third nibble which should hold messageType field */
                __LOG_FW(("Extract the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Arrange the packet fields together: | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get ptp_version & ptp_messageType from HA table for comparison*/
                __LOG_FW(("Get ptp_version & ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_BYTE_READ(PIPE_CFG_REGs_lo + 15);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ptp_version & ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare ptp_version & ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_MPLS_PW_TYPE:  /* #2  | L2(12B) | ET(2B) | MPLS(4B) | MPLS PW(4B) | L2 | ET |  PTP(34B) | */
                 __LOG_FW((" #2  | L2(12B) | ET(2B) | MPLS(4B) | MPLS PW(4B) | L2 | ET |  PTP(34B) |  \n"));

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Extract the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Extract the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison*/
                __LOG_FW(("Get ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_SHORT_READ(PIPE_CFG_REGs_lo + 14);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_TYPE:  /*  #8 | L2(12B) | ET(2B) | IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #8 | L2(12B) | ET(2B) | IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 22);
                __LOG_PARAM_FW(udp_addr);

                /* Get packet UDP destination port */
                __LOG_FW(("Get packet UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 30);
                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field  \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field  \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message.  \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:   /* #9  | L2(12B) | ET(2B) | MPLS(4B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #9  | L2(12B) | ET(2B) | MPLS(4B)| IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of packet IPv4 header */
                __LOG_FW(("Get address of start of IPv4 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 6);
                __LOG_PARAM_FW(ip_addr);

                /* Get packet IPv4 protocol */
                __LOG_FW(("Get packet IPv4 protocol \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 9);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of packet UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 26);
                __LOG_PARAM_FW(udp_addr);

                /* Get packet UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 34);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:    /*  #12  | L2(12B) | ET(2B) | MPLS(4B)| MPLS CW(4B) | L2 (12B)| ET (2B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #12  | L2(12B) | ET(2B) | MPLS(4B)| MPLS CW(4B) | L2 (12B)| ET (2B)| IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of IPv4 header */
                __LOG_FW(("Get address of start of IPv4 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ip_addr);

                /*  Get packet IPv4 protocol */
                __LOG_FW(("Get packet IPv4 protocol \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 9);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 44);
                __LOG_PARAM_FW(udp_addr);

                /*  Get packet UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 52);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_TYPE:    /*  #18   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #18   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 42);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 50);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:    /*  #19   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #19   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 46);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 54);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:    /*  #22   | L2(12B) | ET(2B) | MPLS(4B) | MPLS CW(4B) | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #22   | L2(12B) | ET(2B) | MPLS(4B) | MPLS CW(4B) | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of IPv6 header */
                __LOG_FW(("Get address of start of IPv6 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ip_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 6);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 64);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr26_pre_da_u2e_desc_ptr->outer_l3_offset + 72);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field  \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr26_pre_da_u2e_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr26_pre_da_u2e_desc_ptr,EN_REC_THRD26_PTP,REC_THRD_ID_26);
                }
                break;

            default:
                 /* desc<ptp_pkt_type_idx> is none of the following: 1,2,8,9,12,18,19,22 therefore exit and don't try to classify it as PTP */
                 __LOG_FW((" desc<ptp_pkt_type_idx> is none of the following: 1,2,8,9,12,18,19,22 therefore exit and don't try to classify it as PTP  \n"));

                break;
        }
    }




#if (EN_REC_THRD26_OUTPUT)
    record_data_thread_26_pre_da_u2e(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(26);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR27_PRE_DA_E2U
 * inparam      None
 * return       None
 * description  Support 2 bytes pre tag MAC DA Extended to Upstream use case
 *              Cisco is using a proprietary 2B tag before the MACDA to specify the source/target info between PIPE and their NPU
 *              On the Upstream (PIPE->NPU), port represents source
 *              HA template: | reserved(11B) | ip_protocol(1B) | udp_dest_port(2B) | domain_number(1B) | ptp_version(4b) | message_type(4b) |
 *              src port entry table(4B):  | reserved(1B) | pre_da_port_info(1B) | reserved(13b) | ptpOverMplsEn(1b) | ptp_mode(2b) |
 *              target port entry table(8B): | TPID 2B | PVID 12b | VlanEgrTagState 1b | reserved 3b | PCID 12b | Egress_Pipe_Delay 20b |
 *              hdr in:  Extension space(20B) , MAC DA(6B) , MAC SA (6B).....
 *              hdr out: Extension space(18B) , pre_da_tag(2B) , MAC DA(6B) , MAC SA (6B).....
 *              Firmware adds the 2 bytes pre da tag part just before MAC DA.
 *              It copies the pre da tag from source port entry table.
 *              Firmware completes PTP packet classification and check if PTP should be executed.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR27_PRE_DA_E2U () {
    /*xt_iss_client_command("profile","enable"); */

    /* Get pointer to descriptor */
    struct nonQcn_desc* thr27_pre_da_e2u_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    struct thr27_pre_da_e2u_cfg* thr27_pre_da_e2u_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr27_pre_da_e2u_cfg);

    uint32_t    ptp_addr;
    uint32_t    udp_addr;
    uint32_t    ip_addr;
    uint16_t   pkt_ptp_first_2_bytes;
    uint8_t    pkt_ptp_version;
    uint8_t    pkt_ptp_messageType;
    uint8_t    pkt_ptp_domain_number;
    uint16_t   pkt_udp_dest_port;
    uint8_t    pkt_ip_protocol;
    uint32_t   pkt_ptp_fields;
    uint32_t   ha_ptp_fields;



#if (EN_REC_THRD27)
    init_record_ptr();
    #if (EN_REC_THRD27_INPUT)
    record_data_thread_27_pre_da_e2u(REC_INPUT);
    #endif
#endif


    /* Copy 2 bytes pre tag from source port entry table and placed it just before MAC DA */
    __LOG_FW(("Copy 2 bytes pre tag from source port entry table and placed it just before MAC DA \n"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR27_PRE_DA_E2U_CFG_srcPortEntry__reserved_0_LEN2_TO_PKT_pre_da_tag__reserved_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we increased the packet by 2B */
    __LOG_FW(("Need to remove the 2 bytes before MAC DA. Do it by setting Desc<fw_bc_modification> to -2B indicating HW about the new starting point of the packet header \n"));
    thr27_pre_da_e2u_desc_ptr->fw_bc_modification = 2;

    /* Check if PTP indication is on */
    __LOG_FW(("Check if PTP indication is on \n"));
    __LOG_PARAM_FW(thr27_pre_da_e2u_desc_ptr->is_ptp);
    __LOG_PARAM_FW(thr27_pre_da_e2u_desc_ptr->outer_l3_offset);
    if (thr27_pre_da_e2u_desc_ptr->is_ptp)
    {
        switch(thr27_pre_da_e2u_desc_ptr->ptp_pkt_type_idx)
        {
            case PTP_OVER_L2_TYPE:  /* #1 | L2(12B) | ET(2B) | PTP(34B) | */
                __LOG_FW((" #1 | L2(12B) | ET(2B) | PTP(34B) |  \n"));

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 2);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Extract the first nibble which should hold versionPtp field */
                __LOG_FW(("Extract the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Extract the third nibble which should hold messageType field */
                __LOG_FW(("Extract the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Arrange the packet fields together: | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get ptp_version & ptp_messageType from HA table for comparison*/
                __LOG_FW(("Get ptp_version & ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_BYTE_READ(PIPE_CFG_REGs_lo + 15);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ptp_version & ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare ptp_version & ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_MPLS_PW_TYPE:  /* #2  | L2(12B) | ET(2B) | MPLS(4B) | MPLS PW(4B) | L2 | ET |  PTP(34B) | */
                 __LOG_FW((" #2  | L2(12B) | ET(2B) | MPLS(4B) | MPLS PW(4B) | L2 | ET |  PTP(34B) |  \n"));

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Extract the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Extract the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison*/
                __LOG_FW(("Get ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_SHORT_READ(PIPE_CFG_REGs_lo + 14);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_TYPE:  /*  #8 | L2(12B) | ET(2B) | IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #8 | L2(12B) | ET(2B) | IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 22);
                __LOG_PARAM_FW(udp_addr);

                /* Get packet UDP destination port */
                __LOG_FW(("Get packet UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 30);
                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2B from start of PTP message which should include ptp version and ptp message type => | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field  \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field  \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message.  \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:   /* #9  | L2(12B) | ET(2B) | MPLS(4B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #9  | L2(12B) | ET(2B) | MPLS(4B)| IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of packet IPv4 header */
                __LOG_FW(("Get address of start of IPv4 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 6);
                __LOG_PARAM_FW(ip_addr);

                /* Get packet IPv4 protocol */
                __LOG_FW(("Get packet IPv4 protocol \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 9);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of packet UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 26);
                __LOG_PARAM_FW(udp_addr);

                /* Get packet UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 34);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:    /*  #12  | L2(12B) | ET(2B) | MPLS(4B)| MPLS CW(4B) | L2 (12B)| ET (2B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #12  | L2(12B) | ET(2B) | MPLS(4B)| MPLS CW(4B) | L2 (12B)| ET (2B)| IPv4(20B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of IPv4 header */
                __LOG_FW(("Get address of start of IPv4 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ip_addr);

                /*  Get packet IPv4 protocol */
                __LOG_FW(("Get packet IPv4 protocol \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 9);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 44);
                __LOG_PARAM_FW(udp_addr);

                /*  Get packet UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 52);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_TYPE:    /*  #18   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #18   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 42);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 50);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:    /*  #19   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #19   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 46);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 54);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | (pkt_ptp_messageType) );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison */
                __LOG_FW(("Get udp_dest_port, ptp_domain_number, ptp_version , ptp_messageType from HA table for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( pkt_ptp_fields == ha_ptp_fields )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:    /*  #22   | L2(12B) | ET(2B) | MPLS(4B) | MPLS CW(4B) | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                 __LOG_FW((" #22   | L2(12B) | ET(2B) | MPLS(4B) | MPLS CW(4B) | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) |  \n"));

                /* Get address of start of IPv6 header */
                __LOG_FW(("Get address of start of IPv6 header \n"));
                ip_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 24);
                __LOG_PARAM_FW(ip_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_ip_protocol = PPA_FW_SP_BYTE_READ(ip_addr + 6);
                __LOG_PARAM_FW(pkt_ip_protocol);

                /* Get address of start of UDP header */
                __LOG_FW(("Get address of start of UDP header \n"));
                udp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 64);
                __LOG_PARAM_FW(udp_addr);

                /* Get UDP destination port */
                __LOG_FW(("Get UDP destination port \n"));
                pkt_udp_dest_port = PPA_FW_SP_SHORT_READ(udp_addr + 2);
                __LOG_PARAM_FW(pkt_udp_dest_port);

                /* Get address of start of PTP message */
                __LOG_FW(("Get address of start of PTP message \n"));
                ptp_addr = (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + thr27_pre_da_e2u_desc_ptr->outer_l3_offset + 72);
                __LOG_PARAM_FW(ptp_addr);

                /* Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| */
                __LOG_FW(("Read 2 bytes from start of PTP message which should include ptp version and ptp message type | transportSpecific 4b | messageType 4b | reserved0 4b | versionPTP 4b| \n"));
                pkt_ptp_first_2_bytes = PPA_FW_SP_SHORT_READ(ptp_addr);
                __LOG_PARAM_FW(pkt_ptp_first_2_bytes);

                /* Take the first nibble which should hold versionPtp field */
                __LOG_FW(("Take the first nibble which should hold versionPtp field \n"));
                pkt_ptp_version = pkt_ptp_first_2_bytes & 0xf;
                __LOG_PARAM_FW(pkt_ptp_version);

                /* Take the third nibble which should hold messageType field */
                __LOG_FW(("Take the third nibble which should hold messageType field  \n"));
                pkt_ptp_messageType = (pkt_ptp_first_2_bytes >> 8) & 0xf;
                __LOG_PARAM_FW(pkt_ptp_messageType);

                /* Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. */
                __LOG_FW(("Read 8 bits of domain number field from PTP message. Placed in offset of 4 bytes from start of PTP message. \n"));
                pkt_ptp_domain_number = PPA_FW_SP_BYTE_READ(ptp_addr + 4);
                __LOG_PARAM_FW(pkt_ptp_domain_number);

                /* Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | */
                __LOG_FW(("Arrange the packet fields together: | pkt_udp_dest_port | pkt_ptp_domain_number | pkt_ptp_version | pkt_ptp_messageType | \n"));
                pkt_ptp_fields = ( (pkt_udp_dest_port<<16) | (pkt_ptp_domain_number<<8) | (pkt_ptp_version<<4) | pkt_ptp_messageType );
                __LOG_PARAM_FW(pkt_ptp_fields);

                /* Get PTP fields from HA table. Use them for comparison  */
                __LOG_FW(("Get PTP fields from HA table. Use them for comparison \n"));
                ha_ptp_fields = PPA_FW_SP_READ(PIPE_CFG_REGs_lo + 12);
                __LOG_PARAM_FW(ha_ptp_fields);

                /* Check if PTP handler should be called. Compare ip_protocol, udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields */
                __LOG_FW(("Check if PTP handler should be called. Compare udp_dest_port, ptp_domain_number, ptp_version, ptp_messageType fields \n"));
                __LOG_PARAM_FW(pkt_ip_protocol);
                __LOG_PARAM_FW(thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol);
                __LOG_PARAM_FW(pkt_ptp_fields);
                __LOG_PARAM_FW(ha_ptp_fields);
                if ( (pkt_ip_protocol == thr27_pre_da_e2u_cfg_ptr->HA_Table_ptp_classification_fields.ip_protocol) && (pkt_ptp_fields == ha_ptp_fields) )
                {
                    /* Handle PTP event */
                    __LOG_FW(("Handle PTP event \n"));
                    PTP_handler(thr27_pre_da_e2u_desc_ptr,EN_REC_THRD27_PTP,REC_THRD_ID_27);
                }
                break;

            default:
                 /* desc<ptp_pkt_type_idx> is none of the following: 1,2,8,9,12,18,19,22 therefore exit and don't try to classify it as PTP */
                 __LOG_FW((" desc<ptp_pkt_type_idx> is none of the following: 1,2,8,9,12,18,19,22 therefore exit and don't try to classify it as PTP  \n"));

                break;
        }
    }


#if (EN_REC_THRD27_OUTPUT)
    record_data_thread_27_pre_da_e2u(REC_OUTPUT);
#endif

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(27);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname      THR45_VariableCyclesLengthWithAcclCmd
 * inparam       None
 * return        None
 * description   Thread made for FC verification to be used for power measurements. It gives the user the option to control
 *               the thread time duration by using the first 4 bytes of the configuration table as the number of iterations in
 *               the loop. Each loop's iteration includes a dummy bit field operation which will be done on the configuration
 *               table. Outside the loop there is one more dummy byte field operation changing the extension space in header.
 *               NOTE: No change is done in header itself and descriptor !!!
 *               1.  Read the first 4 bytes of the configuration data.
 *               2.  Get into loop and execute bits copy accelerator command on the configuration table.
 *               3.  Run the loop as the number of iterations the user configured in the HA table.
 *               4.  Exit loop and execute byte copy accelerator command on the extension space.
 *               5.  No support for PTP and ECN cases.
 *               6.  Cycle measurements: From iteration number 1 and above each iteration takes about ~6 cycles.
 *                                       3 for branch delay and 3 for execution.
 *                   Number of iterations | Cycles
 *                              0         |   28
 *                              1         |   28
 *                              2         |   34
 *                              3         |   40
 *                              4         |   46
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR45_VariableCyclesLengthWithAcclCmd () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable"); */

    uint32_t delay,i;
    struct thr45_VariableCyclesLengthWithAcclCmd_cfg* thr45_VariableCyclesLengthWithAcclCmd_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr45_VariableCyclesLengthWithAcclCmd_cfg);

    /* Get the number of iterations */
    delay = thr45_VariableCyclesLengthWithAcclCmd_cfg_ptr->HA_Table_reserved_space.reserved_0;

    for ( i=0;i<delay;i++)
    {
        /* Copy 3 bits field (reserved from cfg target port table) to reserved_2 field in cfg HA table */
        ACCEL_CMD_TRIG(COPY_BITS_THR45_VariableCyclesLengthWithAcclCmd_CFG_targetPortEntry__reserved_LEN3_TO_CFG_HA_Table_reserved_space__reserved_2_OFFSET, 0);
        /*#pragma no_reorder*/
    }

    /* Copy MAC DA (6 bytes) to expansion space */
    ACCEL_CMD_TRIG(COPY_BYTES_THR45_VariableCyclesLengthWithAcclCmd_PKT_mac_da_47_32_LEN6_TO_MACSHIFTLEFT_6_OFFSET, 0);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(45);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}



/********************************************************************************************************************//**
 * funcname        THR46_RemoveAddBytes
 * inparam         None
 * return          None
 * description     Thread made for FC verification for debug only to be used on emulator for stress tests.
 *                 1. Decrement the packets by 0-40 bytes according to configuration.
 *                 2. Increment the packets by 0-20 bytes according to configuration.
 *                 3. Incase of zero bytes do nothing
 *                 Use first 2 words of HA table for user configuration:
 *                    1. First word should contain the number of bytes to add or remove
 *                    2. Second word should contain the action: add or remove while add is '1' and remove is '0'
 *                 Update byte count field in the descriptor with the number of bytes that was added or removed.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR46_RemoveAddBytes () {

    uint32_t numOfBytesUserConfig,removeOrAddUserConfig,src,dest,offset,length,command_value;

    /* Get pointers to cfg and descriptor */
    struct nonQcn_desc* thr46_RemoveAddBytes_desc_ptr             = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
    struct thr46_RemoveAddBytes_cfg* thr46_RemoveAddBytes_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr46_RemoveAddBytes_cfg);

    /* Get user configuration: number of bytes to add or remove */
    numOfBytesUserConfig = thr46_RemoveAddBytes_cfg_ptr->HA_Table_reserved_space.reserved_0;

    /* If number of bytes is zero do nothing */
    if (numOfBytesUserConfig>0)
    {
        /* Get user configuration: remove or add bytes */
        removeOrAddUserConfig = thr46_RemoveAddBytes_cfg_ptr->HA_Table_reserved_space.reserved_1;
        /* Get the offset to the entry in table in which the accelerator command that we need to use will be placed */
        offset = ACCELERATOR_COMMAND_OFFSET_LAST_ENTRY + 4;

        /* Check if remove or add bytes */
        if (removeOrAddUserConfig == ADD_BYTES_CASE)
        {   /* Add bytes case */
            /* Check if number of bytes is in the valid range */
            if (numOfBytesUserConfig>20)
            {
                /* In case user config number of bytes more than the maximum valid number(HW limitation), number of bytes will be the maximum 20 bytes. */
                numOfBytesUserConfig = 20;
            }
            /*#pragma no_reorder*/
            /*************************************************************************************************
             *  Build accelerator command for moving MAC addresses left
             *************************************************************************************************/
            src    = 20;                          /* Get the src position in bytes which is MAC DA (20 bytes right after the expansion area) */
            dest   = src - numOfBytesUserConfig;  /* Get the destination position according to the number of bytes to move */
            length = 12;                          /* Length of MAC DA and MAC SA 12 bytes */
            /* length = 16; */                    /* Length of MAC DA and MAC SA 12 bytes + 2bytes of EthernetType + 2bytes of zero paddings */

            /* Build header to header accelerator command that we need to run in order to shift left the number of bytes according to configuration */
            command_value = BUILD_ACCELERATOR_COMMAND_H2H_COPY_BYTE(length,dest,src);
            /*#pragma no_reorder*/
            /* Save the accelerator command value in the right entry of the table */
            ACCEL_CMD_LOAD(offset, command_value);
            /*#pragma no_reorder*/

            /*************************************************************************************************
             *  Build accelerator command for copy cfg tables to packet header right after MAC SA address
             *************************************************************************************************/
            length = numOfBytesUserConfig ;      /* length of bytes to copy */
            dest   = 32 - numOfBytesUserConfig;  /* Right after MAC SA address: 32 => 20 bytes of extension space + 12 bytes of MAC addresses */
            /* dest   = 36 - numOfBytesUserConfig; */ /* Right after MAC SA address: 32 + 4 => 20 bytes of extension space + 12 bytes of MAC addresses + 2bytes of EthernetType + 2bytes of zero paddings */
            src    = 8;                          /* Third bye of cfg tables which in this case is third byte of HA table */
            /* Build cfg to header accelerator command that we need to run in order to add/remove the number of bytes according to configuration */
            command_value = BUILD_ACCELERATOR_COMMAND_C2H_COPY_BYTE(length,dest,src);
            /*#pragma no_reorder*/
            /* Save the accelerator command value in table */
            ACCEL_CMD_LOAD(offset + 4, command_value);
            /*#pragma no_reorder*/

            /*************************************************************************************************
             *  Trigger the accelerator commands
             *************************************************************************************************/
            /* Trigger the command that shift left MAC addresses */
            ACCEL_CMD_TRIG(offset, 0);
            /*#pragma no_reorder*/
            /* Trigger the command that copy cfg tables to packet header */
            ACCEL_CMD_TRIG(offset + 4, 0);

            /* Indicates HW about the new starting point of the packet header (two's complement). */
            thr46_RemoveAddBytes_desc_ptr->fw_bc_modification = numOfBytesUserConfig;
        }
        else
        {   /* Remove bytes case */
            /* Check if number of bytes is in valid range. The official valid range should be 20 bytes as well however here there is no HW limitation and FW can reduce the header even more. */
            if (numOfBytesUserConfig>40)
            {
                /* In case user config number of bytes more than the maximum valid number(40 bytes), number of bytes will be the maximum 40 bytes. */
                numOfBytesUserConfig = 40;
            }
            /*************************************************************************************************
             *  Build accelerator command for moving MAC addresses left
             *************************************************************************************************/
            src    = 20;                          /* Get the src position in bytes which is MAC DA (20 bytes right after the expansion area) */
            dest   = src + numOfBytesUserConfig;  /* Get the destination position according to the number of bytes to move */
            length = 12;                          /* Length of MAC DA and MAC SA 12 bytes */
            /* length = 16; */                    /* Length of MAC DA and MAC SA 12 bytes + 2bytes of EthernetType + 2bytes of zero paddings */
            /* Build header to header accelerator command that we need to run in order to shift right the number of bytes according to configuration */
            command_value = BUILD_ACCELERATOR_COMMAND_H2H_COPY_BYTE(length,dest,src);
            /*#pragma no_reorder*/
            /* Save the accelerator command value in table */
            ACCEL_CMD_LOAD(offset, command_value);
            /*#pragma no_reorder*/

            /*************************************************************************************************
             *  Trigger the accelerator command
             *************************************************************************************************/
            /* Trigger the command that shift right MAC addresses */
            ACCEL_CMD_TRIG(offset, 0);

            /* Indicates HW about the new starting point of the packet header (two's complement). */
            thr46_RemoveAddBytes_desc_ptr->fw_bc_modification = 0 - numOfBytesUserConfig;
        }

    }

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(46);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        THR47_Add20Bytes
 * inparam         None
 * return          None
 * description     Thread made for FC verification for debug only to check use case in which we add the maximum possible
 *                 bytes (20B) from template to header after L2.
 *                 1.   MAC addresses are shifted left by 20 bytes.
 *                 2.   Copy 20 bytes from configuration data (HA table:16 bytes + source port table:4 bytes) and put them after MAC addresses.
 *                 3.   Update byte count field in the descriptor with + 20 bytes.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR47_Add20Bytes () {
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */

    /* Get pointer to descriptor */
    struct nonQcn_desc* thr47_Add20Bytes_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 20 bytes left */
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_Add20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_20_OFFSET, 0);
    /*#pragma no_reorder*/

    /* Copy 16 bytes configuration HA table to output packet */
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_Add20Bytes_CFG_HA_Table_reserved_space__reserved_0_LEN16_TO_PKT_HA_Table_reserved_space__reserved_0_OFFSET, 0);

    /* Copy 4 bytes configuration source port table to output packet */
    ACCEL_CMD_TRIG(COPY_BYTES_THR47_Add20Bytes_CFG_srcPortEntry__PVID_LEN4_TO_PKT_srcPortEntry__PVID_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 4 bytes therefore the size was reduced in 4 bytes. */
    thr47_Add20Bytes_desc_ptr->fw_bc_modification = 20;

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET) != 0)
    {
        /* Check if this is ECN case */
        if (thr47_Add20Bytes_desc_ptr->mark_ecn == 1)
        {   /* Handle ECN case */
            ECN_handler(thr47_Add20Bytes_desc_ptr,EN_REC_THRD47_ECN,REC_THRD_ID_47);
        }
        /* Check if this is PTP case */
        if (thr47_Add20Bytes_desc_ptr->is_ptp == 1)
        {   /* Handle PTP event */
            PTP_handler(thr47_Add20Bytes_desc_ptr,EN_REC_THRD47_PTP,REC_THRD_ID_47);
        }
    }

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(47);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        THR48_Remove20Bytes
 * inparam         None
 * return          None
 * description     Thread made for FC verification for debug only to check use case in which we remove the maximum possible bytes (20B) after L2.
 *                 1.   Packet is in with some template (I used extended DSA tag + vlan tag) placed after MAC addresses.
 *                 2.   MAC addresses are shifted right by 20 bytes.
 *                 3.   Update byte count field in the descriptor with - 20 bytes.
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR48_Remove20Bytes () {

    /* Get pointer to descriptor */
    struct nonQcn_desc* thr48_Remove20Bytes_desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Move Old_Header<MAC DA> and Old_Header<MAC SA> 20 bytes right */
    ACCEL_CMD_TRIG(COPY_BYTES_THR48_Remove20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_20_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). In this case we shift right 20 bytes therefore the size was reduced in 20 bytes. */
    thr48_Remove20Bytes_desc_ptr->fw_bc_modification = -20;

    /*---------------------------------------------------
     * Check if slow path is required
     * --------------------------------------------------*/
    /* Read one byte which includes is_ptp(1bit) and mark_ecn(1bit) fields from descriptor header. The 6 remaining bits are kept zero by HW */
    if (PPA_FW_SP_BYTE_READ(PIPE_DESC_REGs_lo + SLOW_PATH_INDICATION_OFFSET) != 0)
    {
        /* Check if this is ECN case */
        if (thr48_Remove20Bytes_desc_ptr->mark_ecn == 1)
        {   /* Handle ECN case */
            ECN_handler(thr48_Remove20Bytes_desc_ptr,EN_REC_THRD48_ECN,REC_THRD_ID_48);
        }
        /* Check if this is PTP case */
        if (thr48_Remove20Bytes_desc_ptr->is_ptp == 1)
        {   /* Handle PTP event */
            PTP_handler(thr48_Remove20Bytes_desc_ptr,EN_REC_THRD48_PTP,REC_THRD_ID_48);
        }
    }

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(48);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        THR49_VariableCyclesLength
 * inparam         None
 * return          None
 * description     Thread made for FC verification for debug only. No header alteration here however it gives you the option
 *                 to control the thread time duration by using the first 4 bytes of configuration data as a counter.
 *                 1.   Read the first 4 bytes of the configuration data.
 *                 2.   Get into loop and use this parameter as the number of iterations.
 *                 3.   Each iteration takes about ~5 cycles. Only the first iteration takes more. Around 14 cycles.
 *                 NOTE: delay parameter must not be of value zero since compiler decrements it by one and in this case 0-1=0xffffffff
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void THR49_VariableCyclesLength () {
#ifndef ASIC_SIMULATION
    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
    /*xt_iss_client_command("profile","enable");  */

    uint32_t delay;
    struct thr49_VariableCyclesLength_cfg* thr49_VariableCyclesLength_cfg_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr49_VariableCyclesLength_cfg);

    /* Get the number of iterations */
    delay = thr49_VariableCyclesLength_cfg_ptr->HA_Table_reserved_space.reserved_0;


    asm volatile("        j dloop               \n\t"
                ".align 16                     \n\t"
                "dloop:  addi     %0, %0, -1   \n\t"
                "        bnez     %0, dloop    \n\t" : "+a"(delay) ::"memory" );

    /*xt_iss_client_command("profile","disable"); */
    /*_exit(0); */
#endif  /* !ASIC_SIMULATION */

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    stack_overflow_checker(49);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        PTP_handler (Precision Time Protocol)
 * inparam         None
 * return          None
 * description     PTP packet handler which performs the following steps:    1. PTP type detection
 *                                                                           2. Updating the correctionField
 *                                                                           3. UDP Checksum update for IPv4 or IPv6
 *                                                                           4. Updating the descriptor
 *                 NOTE: CFG=>cfgReservedSpace=>reserved_0 are 4 bytes zero set by HW. We use them for zero padding.
 *
 *   Supports legacy and new PTP modes:
 *   PTP legacy mode (PTP-o-L2/L3-udp):
 *   - Packet type is detected according to packet outer Ethernet type.
 *   - PHA classification is done fully by HW TCAM
 *   - It supports 3 packet types:
 *                                  ------------------------------
 *     PTP_OVER_L2_TYPE:            | L2 | ET | PTP header | PTP |
 *                                  ------------------------------
 *                                  -------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV4_TYPE: | L2 | ET | IPv4 | UDP | PTP header | PTP |
 *                                  -------------------------------------------
 *                                  -------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV6_TYPE: | L2 | ET | IPv6 | UDP | PTP header | PTP |
 *                                  -------------------------------------------
 *
 *   PTP new mode (PTP-o-L2/L3-udp/MPLS):
 *    - Packet type is detected according to desc<ptp_pkt_type_idx>.
 *    - PHA classification is done by HW TCAM and also by fw using parameters located in HA table.
 *      This table is already populated in a few forwarding tag cases (eDSA, ETAG…) so for these cases new mode should not be applied !!!
 *    - It supports the above 3 types plus 5 MPLS types:
 *                                               ------------------------------
 *     PTP_OVER_L2_TYPE:                         | L2 | ET | PTP header | PTP |
 *                                               ------------------------------
 *                                               ---------------------------------------------------------
 *     PTP_OVER_MPLS_PW_TYPE:                    | L2 | ET | MPLS | MPLS CW | L2 | ET | PTP header | PTP |
 *                                               ---------------------------------------------------------
 *                                               -------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV4_TYPE:              | L2 | ET | IPv4 | UDP | PTP header | PTP |
 *                                               -------------------------------------------
 *                                               --------------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:    | L2 | ET | MPLS | IPv4 | UDP | PTP header | PTP |
 *                                               --------------------------------------------------
 *                                               ---------------------------------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE: | L2 | ET | MPLS | MPLS CW | L2 | ET |IPv4 | UDP | PTP header | PTP |
 *                                               ---------------------------------------------------------------------
 *                                               -------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV6_TYPE:              | L2 | ET | IPv6 | UDP | PTP header | PTP |
 *                                               -------------------------------------------
 *                                               --------------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:    | L2 | ET | MPLS | IPv6 | UDP | PTP header | PTP |
 *                                               --------------------------------------------------
 *                                               ---------------------------------------------------------------------
 *     PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE: | L2 | ET | MPLS | MPLS CW | L2 | ET |IPv6 | UDP | PTP header | PTP |
 *                                               ---------------------------------------------------------------------
 *
 *****************************************************************************************************************************************/
void PTP_handler (struct nonQcn_desc *desc_ptr, uint32_t enable_record,uint32_t thread_id)
{
    uint16_t  not_cs_of_current_CF,cs_of_new_CF,new_udp_cs,current_udp_cs;
    uint32_t  sumTo32bits,current_CF_low,new_CF_low,timestamp_in_CF_format_low,timestamp_in_CF_format_high;
    int32_t   local_udp_cs_offset,current_CF_high,new_CF_high;
    uint32_t  reserved_field_nano, reserved_field_sec;
    uint32_t  ErrParameters[6];
    uint8_t   ptp_ofst;
    uint8_t   ptp_over_mpls_en;
    uint16_t  ptp_pkt_type;

#ifndef ASIC_SIMULATION
    uint16_t*           pkt_in_ethernet_type_ptr;
    struct ptp_header* pkt_in_ptp_msg_ptr;
    uint16_t*           pkt_in_udp_checksum_ptr;

#else /* !ASIC_SIMULATION */
    uint32_t            pkt_in_ethernet_type_offset;
    struct ptp_header* pkt_in_ptp_msg_ptr;
    uint16_t*           pkt_in_udp_checksum_ptr;
#endif  /* ASIC_SIMULATION */


    __LOG_FW((" PTP handler \n"));

    /* Check which mode should be applied: legacy mode or new mode (MPLS support) */
    __LOG_FW((" Check which mode should be applied: legacy mode or new mode (MPLS support) \n"));
    ptp_over_mpls_en = ( PPA_FW_SP_BYTE_READ(PIPE_CFG_REGs_lo + PTP_SOURCE_PORT_MODE_OFFSET) >> PTP_OVER_MPLS_EN_OFFSET) & PTP_OVER_MPLS_EN_MASK ;
    __LOG_PARAM_FW(ptp_over_mpls_en);

    if(ptp_over_mpls_en)
    {  /* New ptp mode in which ptp packet type is detected by desc<ptp_pkt_type_idx>  */

        /* Get PTP packet type index from descriptor */
        ptp_pkt_type = desc_ptr->ptp_pkt_type_idx ;
        __LOG_PARAM_FW(ptp_pkt_type);

        /* Set ptp offset (from l3_offset till start of ptp message) according to ptp packet type */
        switch(ptp_pkt_type)
        {
            case PTP_OVER_L2_TYPE:  /* #1 | L2(12B) | ET(2B) | PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  2;
                break;
            case PTP_OVER_MPLS_PW_TYPE:  /* #2  | L2(12B) | ET(2B) | MPLS(4B) | MPLS PW(4B) | L2 | ET |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  24;
                break;
            case PTP_OVER_UDP_OVER_IPV4_TYPE:  /*  #8 | L2(12B) | ET(2B) | IPv4(20B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  30;
                break;
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:   /* #9  | L2(12B) | ET(2B) | MPLS(4B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  34;
                break;
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:    /*  #12  | L2(12B) | ET(2B) | MPLS(4B)| MPLS CW(4B) | L2 (12B)| ET (2B)| IPv4(20B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  52;
                break;
            case PTP_OVER_UDP_OVER_IPV6_TYPE:    /*  #18   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  50;
                break;
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:    /*  #19   | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  54;
                break;
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:    /*  #22   | L2(12B) | ET(2B) | MPLS(4B) | MPLS CW(4B) | L2(12B) | ET(2B) | IPv6(40B) | UDP(8B) |  PTP(34B) | */
                /* Set offset from desc_ptr->outer_l3_offset to start of ptp message */
                ptp_ofst =  72;
                break;
            default:
                /* Invalid ptp packet type index. */

                /* Notify MG about this failure. Save error parameters in SP !!! */
                ErrParameters[0] = thread_id;
                ErrParameters[2] = ptp_pkt_type;
                ppn2mg_failure_event(PPN2MG_PTP_INVALID_ETHERNET_TYPE_EVENT,ErrParameters,2,DONT_STOP_PPN);
                /* Exit PTP handler */
                return;
        }
     }
     else
     {  /* Legacy mode in which PTP packet type is detected by Ethernet type */

        /* Read packet Ethernet type to get ptp packet type  */
        ptp_pkt_type = PPA_FW_SP_SHORT_READ(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + desc_ptr->outer_l3_offset);

        /* Set ptp offset (from l3_offset till start of ptp message) according to desc<ptp_offset> */
        ptp_ofst = desc_ptr->ptp_offset;
     }

    __LOG_PARAM_FW(ptp_pkt_type);
    __LOG_PARAM_FW(ptp_ofst);



/*-----------------------------------------------------------------------------------------------------------
 1. Set pointers to Ether type, PTP message and UDP checksum.
 -----------------------------------------------------------------------------------------------------------*/
#ifndef ASIC_SIMULATION /* the ASIC_SIMULATION 64bit compilation not like the cast to (uint16_t*) */
    /* Get the PTP transport type. outer_l3_offset is the offset from beginning of input packet (mac da) to start of Ethernet field. */
    pkt_in_ethernet_type_ptr    = (uint16_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + desc_ptr->outer_l3_offset);
    /* Get pointer to PTP message. ptp_offset is the offset in bytes from outer_l3_offset. */
    pkt_in_ptp_msg_ptr = (struct ptp_header*)( (uint8_t*)pkt_in_ethernet_type_ptr + ptp_ofst);
    /* Get pointer to UDP checksum. The location of the UDP Checksum is the 2 bytes preceding the PTP header. */
    pkt_in_udp_checksum_ptr     = (uint16_t*)((uint8_t*)pkt_in_ptp_msg_ptr - UDP_CS_SIZE);
#else /* !ASIC_SIMULATION */
    pkt_in_ethernet_type_offset = PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + desc_ptr->outer_l3_offset;
    pkt_in_ptp_msg_ptr          = PIPE_MEM_CAST(pkt_in_ethernet_type_offset + ptp_ofst,ptp_header);
    {
        uint32_t addr = pkt_in_ethernet_type_offset + ptp_ofst - UDP_CS_SIZE;
        uint32_t newAddr = addr & 0xFFFFFFFC;/* the shift hold compensation about the address */
        pkt_in_udp_checksum_ptr = (uint16_t*)pha_findMem(newAddr);
        pkt_in_udp_checksum_ptr++;
    }
#endif  /* ASIC_SIMULATION */
    /* Prevent warning */
    enable_record = enable_record;

#if EN_REC_PTP
    /* Record PTP input data if PTP record flag is set to on */
    if (enable_record)
    {
        record_data_PTP_handler(pkt_in_ethernet_type_ptr, pkt_in_ptp_msg_ptr, pkt_in_udp_checksum_ptr, REC_INPUT, thread_id );
    }
#endif

    /* Get current correction field parameters */
    current_CF_high = ( (pkt_in_ptp_msg_ptr->correctionField_63_48<<16) | (pkt_in_ptp_msg_ptr->correctionField_47_32&0xFFFF));
    current_CF_low  = ( (pkt_in_ptp_msg_ptr->correctionField_31_16<<16) | (pkt_in_ptp_msg_ptr->correctionField_15_0&0xFFFF) );
    /*#pragma no_reorder*/

    /*  Check if need to update CF */
    if ( (current_CF_high == 0x7FFFFFFF) && (current_CF_low == 0xFFFFFFFF) )
    {
        /* Indicates that the correction is too big to be represented therefore do nothing. */
        /* Disable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 0). */
        ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
        /*#pragma no_reorder*/
    }
    else
    {
        /*  Check type of processing: TC or BC/OC */
        switch ((PPA_FW_SP_BYTE_READ(PIPE_CFG_REGs_lo + PTP_SOURCE_PORT_MODE_OFFSET)) & PTP_SOURCE_PORT_MODE_MASK)
        {
        case PTP_SOURCE_PORT_MODE_OC_BC_INGRESS:

            /* OC/BC ingress - Set the Reserved field to the ingress timestamp */
            /*-----------------------------------------------------------------------------------------------------------
             -----------------------------------------------------------------------------------------------------------*/
            /* Convert timestamp to fit RF format. */
            reserved_field_nano = desc_ptr->timestamp_NanoSec;
            if (desc_ptr->timestamp_Sec & 0x01)
            {
                reserved_field_nano += BILLION;
            }


            /* Check which ptp packet type */
            switch (ptp_pkt_type)
            {
            case IPV4_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:

                __LOG_FW((" IPv4 Sync message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                /* Update the new reserved field */
                pkt_in_ptp_msg_ptr->reserved2 = (uint16_t)(reserved_field_nano >> 16);
                pkt_in_ptp_msg_ptr->reserved3 = (uint16_t)(reserved_field_nano & 0xFFFF);

                /*-----------------------------------------------------------------------------------------------------------
                3. UDP Checksum update for IPv4
                 -----------------------------------------------------------------------------------------------------------*/
                /* For IPv4: clear the UDP Checksum to zero */
                *pkt_in_udp_checksum_ptr = 0;

                break;

            case IPV6_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:

                __LOG_FW((" IPv6 Sync message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                /*-----------------------------------------------------------------------------------------------------------
                3. UDP Checksum update for IPv6
                 -----------------------------------------------------------------------------------------------------------*/
                /* Read origin udp checksum */
                current_udp_cs = *pkt_in_udp_checksum_ptr;

                /* Calculate UDP checksum only if origin cs is not zero (from RFC768) */
                if(current_udp_cs != 0)
                {
                    /* For IPv6: update the UDP Checksum to an updated value (incremental computation), due to the correctionField update. */
                    /* New checksum = ~(  ~(current checksum) + ~(current CF) + (new CF) ) */

                    /* Checksum calculation of 64 bits of current CF => ~(current CF) */
                    /* Note: current_CF_high is defined as signed since initial CF can be either positive or negative
                       however for the cs calculation we must use it as usigned type therefore the casting */
                    sumTo32bits  = pkt_in_ptp_msg_ptr->reserved2;
                    sumTo32bits += pkt_in_ptp_msg_ptr->reserved3;

                    /* Add also the carry to the sum */
                    sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);

                    /* Perform not and save it for later */
                    not_cs_of_current_CF = ~sumTo32bits ;

                    /* Checksum calculation of 64 bits of new CF => (new CF) */
                    /* Note: new_CF_high is defined as signed since updated CF can be either positive or negative
                       however for the cs calculation we must use it as usigned type therefore the casting */
                    sumTo32bits  = (reserved_field_nano >> 16) ;
                    sumTo32bits += (reserved_field_nano  & 0xFFFF) ;

                    /* Add also the carry to the sum */
                    sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);

                    /* Save it for later */
                    cs_of_new_CF = sumTo32bits ;

                    /* Now calculate the new CS according to formula =>  ~( ~(current checksum) + ~(current CF) + (new CF) ) */
                    sumTo32bits  = (~current_udp_cs) & 0xFFFF;

                    sumTo32bits += not_cs_of_current_CF;
                    sumTo32bits += cs_of_new_CF;

                    /* Check for carry */
                    while (sumTo32bits > 0xFFFF)
                    {
                        /* Add also the carry to the sum */
                        sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                    }

                    /* Perform not and update the new UDP CS */
                    new_udp_cs = ~sumTo32bits ;

                    /* From RFC 768: "If the computed checksum is zero,it is transmitted as all ones (the equivalent in one's complement arithmetic). */
                    /*               An all zero transmitted checksum value means that the transmitter generated no checksum (for debugging or for higher level protocols that don't care)." */
                    if(new_udp_cs == 0)
                    {
                        *pkt_in_udp_checksum_ptr = 0xFFFF ;
                    }
                    else
                    {
                        *pkt_in_udp_checksum_ptr = new_udp_cs ;
                    }

                }

                /* Fall-through */

            case ETHERNET_TRANSPORT_TYPE:
            case PTP_OVER_L2_TYPE:
            case PTP_OVER_MPLS_PW_TYPE:

                __LOG_FW((" Eth Sync message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                /* Update the new reserved field */
                pkt_in_ptp_msg_ptr->reserved2 = (uint16_t)(reserved_field_nano >> 16);
                pkt_in_ptp_msg_ptr->reserved3 = (uint16_t)(reserved_field_nano & 0xFFFF);

                break;

            default:
                /* Invalid ptp packet type !!! */

                /* Notify MG about this failure. Save error parameters in SP !!! */
                ErrParameters[0] = thread_id;
                ErrParameters[1] = desc_ptr->outer_l3_offset;
                ErrParameters[2] = ptp_pkt_type;
                ppn2mg_failure_event(PPN2MG_PTP_INVALID_ETHERNET_TYPE_EVENT,ErrParameters,3,DONT_STOP_PPN);
                break;

            }

            /* Skip the TSU configuration code - No need to trigger TSU modifications */
            goto exit_nicely;

        case PTP_SOURCE_PORT_MODE_OC_BC_EGRESS:

            /* OC/BC ingress - CF field is updated from Reserved field, TSU is programmed to set CF += <EgressTime> */
            /*-----------------------------------------------------------------------------------------------------------
             -----------------------------------------------------------------------------------------------------------*/
            /*-----------------------------------------------------------------------------------------------------------
            2. Updating the correctionField.
               C: correctionField value which is the value of the correction measured in nanoseconds and multiplied by 2^16
               T: The value of the 30 least significant bits of Desc<Timestamp> after convert it to CF format
               If required then update correction field by performing C - T
             -----------------------------------------------------------------------------------------------------------*/
            reserved_field_sec  = 0;
            reserved_field_nano = ((uint32_t)pkt_in_ptp_msg_ptr->reserved2 << 16) | pkt_in_ptp_msg_ptr->reserved3;
            if (reserved_field_nano >= BILLION)
            {
                reserved_field_sec   = 1;
                reserved_field_nano -= BILLION;
            }
            __LOG_PARAM_FW(reserved_field_sec);
            __LOG_PARAM_FW(reserved_field_nano);
            /* Convert timestamp to fit CF format. Shift it left by 16 bits (change from 30 bits to maximum 46 bits representation) */
            timestamp_in_CF_format_low  = (reserved_field_nano << 16);
            timestamp_in_CF_format_high = (reserved_field_nano >> 16) & (0x3FFF);
            __LOG_PARAM_FW(timestamp_in_CF_format_low);
            __LOG_PARAM_FW(timestamp_in_CF_format_high);

            /* Update correction field by performing C - T  */
            /* C is signed 64-bit integer, therefore it may be either positive or negative. Similarly, C-T may be either positive or negative */
            new_CF_low  = current_CF_low  - timestamp_in_CF_format_low;
            new_CF_high = current_CF_high - timestamp_in_CF_format_high;

            /* Check for underflow of low 32 bits, if this is the case subtract one from high part */
            if (new_CF_low > current_CF_low)
            {
                new_CF_high = new_CF_high - 1 ;
            }
            __LOG_PARAM_FW(new_CF_low);
            __LOG_PARAM_FW(new_CF_high);

            /* Update the new correction field */
            pkt_in_ptp_msg_ptr->correctionField_63_48 =  (new_CF_high >> 16) ;
            pkt_in_ptp_msg_ptr->correctionField_47_32 =  (new_CF_high & 0xFFFF) ;
            pkt_in_ptp_msg_ptr->correctionField_31_16 =  (new_CF_low >> 16) ;
            pkt_in_ptp_msg_ptr->correctionField_15_0  =  (new_CF_low & 0xFFFF) ;

            /* Use the two Desc<Timestamp> bits representing Seconds to detect a wraparound */
            if ((desc_ptr->timestamp_Sec & 0x01) == reserved_field_sec)
            {
                desc_ptr->ingress_timestamp_seconds = desc_ptr->timestamp_Sec;
            }
            else
            {
                /* wraparound has occurred */
                if (reserved_field_nano >= HALF_BILLION)
                {
                    desc_ptr->ingress_timestamp_seconds = desc_ptr->timestamp_Sec - 1;
                }
                else
                {
                    desc_ptr->ingress_timestamp_seconds = desc_ptr->timestamp_Sec + 1;
                }
            }

            /* Check which IP version */
            switch (ptp_pkt_type)
            {
            case IPV4_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:

                /*-----------------------------------------------------------------------------------------------------------
                3. UDP Checksum update for IPv4
                 -----------------------------------------------------------------------------------------------------------*/
                /* For IPv4: clear the UDP Checksum to zero */
                __LOG_FW((" IPv4 DReq message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                *pkt_in_udp_checksum_ptr = 0;

                /*-----------------------------------------------------------------------------------------------------------
                4. Updating the descriptor. At this point only mac_timestamping_en field
                 -----------------------------------------------------------------------------------------------------------*/
                /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
/*#pragma no_reorder*/

                break;

            case IPV6_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:

                __LOG_FW((" IPv6 DReq message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                /* Point to last two bytes of packet(end of PTP message right before 2 bytes of udp checksum trailer). The offset is in bytes from the beginning of the outgoing packet. */
                /* pkt_in_ptp_msg_ptr->messageLength holds the full length of the PTP message i.e. including the header, body and any suffix */
                /* fw_bc_modification is added as well to reflect the header alteration that was done (add/remove bytes) */
                local_udp_cs_offset = desc_ptr->outer_l3_offset + ptp_ofst + pkt_in_ptp_msg_ptr->messageLength + desc_ptr->fw_bc_modification ;
                __LOG_PARAM_FW(local_udp_cs_offset);

                /* Check if offset to udp checksum trailer is valid. If it exceeds the first 256 bytes we don't want to modify the packet at all */
                if ( local_udp_cs_offset > UDP_CS_TRAILER_MAX_OFFSET)
                {
                    /* Disable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 0). */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
/*#pragma no_reorder*/

                    /* Notify MG about this failure. Save error parameters in SP !!! */
                    ErrParameters[0] = thread_id;
                    ErrParameters[1] = local_udp_cs_offset;
                    ErrParameters[2] = desc_ptr->outer_l3_offset;
                    ErrParameters[3] = ptp_ofst;
                    ErrParameters[4] = pkt_in_ptp_msg_ptr->messageLength;
                    ErrParameters[5] = desc_ptr->fw_bc_modification;
                    ppn2mg_failure_event(PPN2MG_PTP_INVALID_UDP_CS_OFFSET_EVENT,ErrParameters,6,DONT_STOP_PPN);
                }
                else
                {
                    /*-----------------------------------------------------------------------------------------------------------
                    3. UDP Checksum update for IPv6
                     -----------------------------------------------------------------------------------------------------------*/
                    /* Read origin udp checksum */
                    current_udp_cs = *pkt_in_udp_checksum_ptr;
                    __LOG_PARAM_FW(current_udp_cs);

                    /* Calculate UDP checksum only if origin cs is not zero (from RFC768) */
                    if(current_udp_cs != 0)
                    {
                        /* For IPv6: update the UDP Checksum to an updated value (incremental computation), due to the correctionField update. */
                        /* New checksum = ~(  ~(current checksum) + ~(current CF) + (new CF) ) */

                        /* Checksum calculation of 64 bits of current CF => ~(current CF) */
                        /* Note: current_CF_high is defined as signed since initial CF can be either positive or negative
                           however for the cs calculation we must use it as usigned type therefore the casting */
                        sumTo32bits  = ((uint32_t)current_CF_high >> 16);

                        sumTo32bits += ((uint32_t)current_CF_high & 0xFFFF);
                        sumTo32bits += (current_CF_low >> 16);
                        sumTo32bits += (current_CF_low & 0xFFFF);
                        sumTo32bits += pkt_in_ptp_msg_ptr->reserved2;
                        sumTo32bits += pkt_in_ptp_msg_ptr->reserved3;

                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }

                        /* Perform not and save it for later */
                        not_cs_of_current_CF = ~sumTo32bits ;
                        __LOG_PARAM_FW(not_cs_of_current_CF);

                        /* Checksum calculation of 64 bits of new CF => (new CF) */
                        /* Note: new_CF_high is defined as signed since updated CF can be either positive or negative
                           however for the cs calculation we must use it as usigned type therefore the casting */
                        sumTo32bits  = ((uint32_t)new_CF_high >> 16);

                        sumTo32bits += ((uint32_t)new_CF_high & 0xFFFF);
                        sumTo32bits += (new_CF_low >> 16) ;
                        sumTo32bits += (new_CF_low & 0xFFFF) ;

                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }

                        /* Save it for later */
                        cs_of_new_CF = sumTo32bits ;
                        __LOG_PARAM_FW(cs_of_new_CF);

                        /* Now calculate the new CS according to formula =>  ~( ~(current checksum) + ~(current CF) + (new CF) ) */
                        sumTo32bits  = (~current_udp_cs) & 0xFFFF;

                        sumTo32bits += not_cs_of_current_CF;
                        sumTo32bits += cs_of_new_CF;

                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }

                        /* Perform not and update the new UDP CS */
                        new_udp_cs = ~sumTo32bits ;
                        __LOG_PARAM_FW(new_udp_cs);

                        /* From RFC 768: "If the computed checksum is zero,it is transmitted as all ones (the equivalent in one's complement arithmetic). */
                        /*               An all zero transmitted checksum value means that the transmitter generated no checksum (for debugging or for higher level protocols that don't care)." */
                        if(new_udp_cs == 0)
                        {
                            *pkt_in_udp_checksum_ptr = 0xFFFF ;
                        }
                        else
                        {
                            *pkt_in_udp_checksum_ptr = new_udp_cs ;
                        }
                    }

                    /*----------------------------------------------------------------------------------------------------------------------
                    4. Updating the descriptor. At this point only mac_timestamping_en, udp_checksum_offset and udp_checksum_update_en fields
                     -----------------------------------------------------------------------------------------------------------------------*/
                    /* Update udp checksum offset */
                    desc_ptr->udp_checksum_offset = local_udp_cs_offset ;
                    /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
                    /* UDP Checksum Update En = 1 */
                    /* According to TalM this field will be ignored by the MAC when Desc<MAC Timestamping En> is disabled, so in this case its value is insignificant */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET, 0);
/*#pragma no_reorder*/

                }

                break;

            case ETHERNET_TRANSPORT_TYPE:
            case PTP_OVER_L2_TYPE:
            case PTP_OVER_MPLS_PW_TYPE:

                __LOG_FW((" IPv6 DReq message arriving to slave OC - Set the Reserved field to the ingress timestamp \n"));
                /*----------------------------------------------------------------------------------------------------------------------
                4. Updating the descriptor. At this point only mac_timestamping_en field
                 -----------------------------------------------------------------------------------------------------------------------*/
                /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
/*#pragma no_reorder*/

                break;

            default:
                /* Invalid ptp packet type !!! */

                /* Notify MG about this failure. Save error parameters in SP !!! */
                ErrParameters[0] = thread_id;
                ErrParameters[1] = desc_ptr->outer_l3_offset;
                ErrParameters[2] = ptp_pkt_type;
                ppn2mg_failure_event(PPN2MG_PTP_INVALID_ETHERNET_TYPE_EVENT,ErrParameters,3,DONT_STOP_PPN);
                break;

            }

            pkt_in_ptp_msg_ptr->reserved2 = 0;
            pkt_in_ptp_msg_ptr->reserved3 = 0;

            break;


        case PTP_OC_BC_EGRESS_TIMESTAMP_IN_CF:

            /* OC/BC egress - apply egress timestamp in CF. Don't insert ingress time to CF, don’t modify the PTP header at all. 
               Usually fw sets CF = -Ingress Timestamp and set TSU to add the egress timestamp (nanosec portion) + Egress delay  
               so CF = Egress timestamp + Egress delay - Ingress Timestamp. In this case no Ingress timestamp so CF = Egress timestamp + Egress delay*/
            /*-----------------------------------------------------------------------------------------------------------*/

    		/* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
    	    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
            /* Copy Timestamp[31:30] to Desc<Ingress Timestamp Seconds>. */
            ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_OFFSET, 0);
           	/*#pragma no_reorder*/

            /* Check if it is IPv6 packet type */
            if ( (ptp_pkt_type == IPV6_TRANSPORT_TYPE) ||
                 (ptp_pkt_type == PTP_OVER_UDP_OVER_IPV6_TYPE) ||
                 (ptp_pkt_type == PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE) ||
                 (ptp_pkt_type == PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE) )
            {
    			/* Point to last two bytes of packet(end of PTP message right before 2 bytes of udp checksum trailer). The offset is in bytes from the beginning of the outgoing packet. */
    			/* pkt_in_ptp_msg_ptr->messageLength holds the full length of the PTP message i.e. including the header, body and any suffix */
    			/* fw_bc_modification is added as well to reflect the header alteration that was done (add/remove bytes) */
    			local_udp_cs_offset = desc_ptr->outer_l3_offset + ptp_ofst + pkt_in_ptp_msg_ptr->messageLength + desc_ptr->fw_bc_modification ;
            	/*#pragma no_reorder*/

    			/* Check if offset to udp checksum trailer is valid. If it exceeds the first 256 bytes we don't want to modify the packet at all */
    			if ( local_udp_cs_offset > UDP_CS_TRAILER_MAX_OFFSET)
    			{
    				/* Disable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 0). */
    	    		ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
    			}

    		    /* Update udp checksum offset */
   				desc_ptr->udp_checksum_offset = local_udp_cs_offset ;
   				/* UDP Checksum Update En = 1. This field will be ignored by the MAC when Desc<MAC Timestamping En> is disabled, so in this case its value is insignificant */
   				ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET, 0);
            }

            break;


        default:

            /* TC */
            /* Check which IP version */
            switch (ptp_pkt_type)
            {
            case IPV4_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV4_OVER_MPLS_PW_TYPE:

                /*-----------------------------------------------------------------------------------------------------------
                2. Updating the correctionField.
                   C: correctionField value which is the value of the correction measured in nanoseconds and multiplied by 2^16
                   T: The value of the 30 least significant bits of Desc<Timestamp> after convert it to CF format
                   If required then update correction field by performing C - T
                 -----------------------------------------------------------------------------------------------------------*/
                /* Convert timestamp to fit CF format. Shift it left by 16 bits (change from 30 bits to maximum 46 bits representation) */
                timestamp_in_CF_format_low  = (desc_ptr->timestamp_NanoSec << 16);
                timestamp_in_CF_format_high = (desc_ptr->timestamp_NanoSec >>16) & (0x3FFF);

                /* Update correction field by performing C - T  */
                /* C is signed 64-bit integer, therefore it may be either positive or negative. Similarly, C-T may be either positive or negative */
                new_CF_low  = current_CF_low  - timestamp_in_CF_format_low;
                new_CF_high = current_CF_high - timestamp_in_CF_format_high ;

                /* Check for underflow of low 32 bits, if this is the case subtract one from high part */
                if (new_CF_low > current_CF_low)
                {
                    new_CF_high = new_CF_high - 1 ;
                }

                /* Update the new correction field */
                pkt_in_ptp_msg_ptr->correctionField_63_48 =  (new_CF_high >> 16) ;
                pkt_in_ptp_msg_ptr->correctionField_47_32 =  (new_CF_high & 0xFFFF) ;
                pkt_in_ptp_msg_ptr->correctionField_31_16 =  (new_CF_low >> 16) ;
                pkt_in_ptp_msg_ptr->correctionField_15_0  =  (new_CF_low & 0xFFFF) ;
                /*#pragma no_reorder*/

                /*-----------------------------------------------------------------------------------------------------------
                3. UDP Checksum update for IPv4
                 -----------------------------------------------------------------------------------------------------------*/
                /* For IPv4: clear the UDP Checksum to zero */
                *pkt_in_udp_checksum_ptr = 0;

                /*-----------------------------------------------------------------------------------------------------------
                4. Updating the descriptor. At this point only mac_timestamping_en field
                 -----------------------------------------------------------------------------------------------------------*/
                /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
                /*#pragma no_reorder*/
                break;

            case IPV6_TRANSPORT_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_TYPE:
            case PTP_OVER_UDP_OVER_IPV6_OVER_MPLS_PW_TYPE:

                /* Point to last two bytes of packet(end of PTP message right before 2 bytes of udp checksum trailer). The offset is in bytes from the beginning of the outgoing packet. */
                /* pkt_in_ptp_msg_ptr->messageLength holds the full length of the PTP message i.e. including the header, body and any suffix */
                /* fw_bc_modification is added as well to reflect the header alteration that was done (add/remove bytes) */
                local_udp_cs_offset = desc_ptr->outer_l3_offset + ptp_ofst + pkt_in_ptp_msg_ptr->messageLength + desc_ptr->fw_bc_modification ;
                /*#pragma no_reorder*/

                /* Check if offset to udp checksum trailer is valid. If it exceeds the first 256 bytes we don't want to modify the packet at all */
                if ( local_udp_cs_offset > UDP_CS_TRAILER_MAX_OFFSET)
                {
                    /* Disable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 0). */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
                    /*#pragma no_reorder*/

                    /* Notify MG about this failure. Save error parameters in SP !!! */
                    ErrParameters[0] = thread_id;
                    ErrParameters[1] = local_udp_cs_offset;
                    ErrParameters[2] = desc_ptr->outer_l3_offset;
                    ErrParameters[3] = ptp_ofst;
                    ErrParameters[4] = pkt_in_ptp_msg_ptr->messageLength;
                    ErrParameters[5] = desc_ptr->fw_bc_modification;
                    ppn2mg_failure_event(PPN2MG_PTP_INVALID_UDP_CS_OFFSET_EVENT,ErrParameters,6,DONT_STOP_PPN);
                }
                else
                {
                    /*-----------------------------------------------------------------------------------------------------------
                    2. Updating the correctionField.
                       C: correctionField value which is the value of the correction measured in nanoseconds and multiplied by 2^16
                       T: The value of the 30 least significant bits of Desc<Timestamp> after convert it to CF format
                       If required then update correction field by performing C - T
                     -----------------------------------------------------------------------------------------------------------*/
                    /* Convert timestamp to fit CF format. Shift it left by 16 bits (change from 30 bits to maximum 46 bits representation) */
                    timestamp_in_CF_format_low  = (desc_ptr->timestamp_NanoSec << 16);
                    timestamp_in_CF_format_high = (desc_ptr->timestamp_NanoSec >>16) & (0x3FFF);

                    /* Update correction field by performing C - T  */
                    /* C is signed 64-bit integer, therefore it may be either positive or negative. Similarly, C-T may be either positive or negative */
                    new_CF_low  = current_CF_low  - timestamp_in_CF_format_low;
                    new_CF_high = current_CF_high - timestamp_in_CF_format_high ;

                    /* Check for underflow of low 32 bits, if this is the case subtract one from high part */
                    if (new_CF_low > current_CF_low)
                    {
                        new_CF_high = new_CF_high - 1 ;
                    }

                    /* Update the new correction field */
                    pkt_in_ptp_msg_ptr->correctionField_63_48 =  (new_CF_high >> 16) ;
                    pkt_in_ptp_msg_ptr->correctionField_47_32 =  (new_CF_high & 0xFFFF) ;
                    pkt_in_ptp_msg_ptr->correctionField_31_16 =  (new_CF_low >> 16) ;
                    pkt_in_ptp_msg_ptr->correctionField_15_0  =  (new_CF_low & 0xFFFF) ;
                    /*#pragma no_reorder*/

                    /*-----------------------------------------------------------------------------------------------------------
                    3. UDP Checksum update for IPv6
                     -----------------------------------------------------------------------------------------------------------*/
                    /* Read origin udp checksum */
                    current_udp_cs = *pkt_in_udp_checksum_ptr;

                    /* Calculate UDP checksum only if origin cs is not zero (from RFC768) */
                    if(current_udp_cs != 0)
                    {
                        /* For IPv6: update the UDP Checksum to an updated value (incremental computation), due to the correctionField update. */
                        /* New checksum = ~(  ~(current checksum) + ~(current CF) + (new CF) ) */

                        /* Checksum calculation of 64 bits of current CF => ~(current CF) */
                        /* Note: current_CF_high is defined as signed since initial CF can be either positive or negative
                           however for the cs calculation we must use it as usigned type therefore the casting */
                        sumTo32bits  = ((uint32_t)current_CF_high >> 16);
                        /*#pragma no_reorder*/
                        sumTo32bits += ((uint32_t)current_CF_high & 0xFFFF);
                        sumTo32bits += (current_CF_low >> 16);
                        sumTo32bits += (current_CF_low & 0xFFFF);
                        /*#pragma no_reorder*/
                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }
                        /*#pragma no_reorder*/
                        /* Perform not and save it for later */
                        not_cs_of_current_CF = ~sumTo32bits ;
                        /*#pragma no_reorder*/

                        /* Checksum calculation of 64 bits of new CF => (new CF) */
                        /* Note: new_CF_high is defined as signed since updated CF can be either positive or negative
                           however for the cs calculation we must use it as usigned type therefore the casting */
                        sumTo32bits  = ((uint32_t)new_CF_high >> 16);
                        /*#pragma no_reorder*/
                        sumTo32bits += ((uint32_t)new_CF_high & 0xFFFF);
                        sumTo32bits += (new_CF_low >> 16) ;
                        sumTo32bits += (new_CF_low & 0xFFFF) ;
                        /*#pragma no_reorder*/
                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }
                        /*#pragma no_reorder*/
                        /* Save it for later */
                        cs_of_new_CF = sumTo32bits ;
                        /*#pragma no_reorder*/

                        /* Now calculate the new CS according to formula =>  ~( ~(current checksum) + ~(current CF) + (new CF) ) */
                        sumTo32bits  = (~current_udp_cs) & 0xFFFF;

                        /*#pragma no_reorder*/
                        sumTo32bits += not_cs_of_current_CF;
                        sumTo32bits += cs_of_new_CF;
                        /*#pragma no_reorder*/
                        /* Check for carry */
                        while (sumTo32bits > 0xFFFF)
                        {
                            /* Add also the carry to the sum */
                            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
                        }
                        /*#pragma no_reorder*/
                        /* Perform not and update the new UDP CS */
                        new_udp_cs = ~sumTo32bits ;
                        /*#pragma no_reorder*/
                        /* From RFC 768: "If the computed checksum is zero,it is transmitted as all ones (the equivalent in one's complement arithmetic). */
                        /*               An all zero transmitted checksum value means that the transmitter generated no checksum (for debugging or for higher level protocols that don't care)." */
                        if(new_udp_cs == 0)
                        {
                            *pkt_in_udp_checksum_ptr = 0xFFFF ;
                        }
                        else
                        {
                            *pkt_in_udp_checksum_ptr = new_udp_cs ;
                        }
                        /*#pragma no_reorder*/
                    }

                    /*----------------------------------------------------------------------------------------------------------------------
                    4. Updating the descriptor. At this point only mac_timestamping_en, udp_checksum_offset and udp_checksum_update_en fields
                     -----------------------------------------------------------------------------------------------------------------------*/
                    /* Update udp checksum offset */
                    desc_ptr->udp_checksum_offset = local_udp_cs_offset ;
                    /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
                    /* UDP Checksum Update En = 1 */
                    /* According to TalM this field will be ignored by the MAC when Desc<MAC Timestamping En> is disabled, so in this case its value is insignificant */
                    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET, 0);
                    /*#pragma no_reorder*/
                }
                break;

            case ETHERNET_TRANSPORT_TYPE:
            case PTP_OVER_L2_TYPE:
            case PTP_OVER_MPLS_PW_TYPE:

                /*----------------------------------------------------------------------------------------------------------------------
                4. Updating the descriptor. At this point only mac_timestamping_en field
                 -----------------------------------------------------------------------------------------------------------------------*/
                /* Enable MAC Timestamping En field(desc_ptr->mac_timestamping_en = 1). Use is_ptp field which must be '1' to set this field to 1. */
                ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET, 0);
                /*#pragma no_reorder*/

                /*-----------------------------------------------------------------------------------------------------------
                2. Updating the correctionField.
                   C: correctionField value which is the value of the correction measured in nanoseconds and multiplied by 2^16
                   T: The value of the 30 least significant bits of Desc<Timestamp> after convert it to CF format
                   If required then update correction field by performing C - T
                 -----------------------------------------------------------------------------------------------------------*/
                /* Convert timestamp to fit CF format. Shift it left by 16 bits (change from 30 bits to maximum 46 bits representation) */
                timestamp_in_CF_format_low  = (desc_ptr->timestamp_NanoSec << 16);
                timestamp_in_CF_format_high = (desc_ptr->timestamp_NanoSec >>16) & (0x3FFF);

                /* Update correction field by performing C - T  */
                /* C is signed 64-bit integer, therefore it may be either positive or negative. Similarly, C-T may be either positive or negative */
                new_CF_low  = current_CF_low  - timestamp_in_CF_format_low;
                new_CF_high = current_CF_high - timestamp_in_CF_format_high ;

                /* Check for underflow of low 32 bits, if this is the case subtract one from high part */
                if (new_CF_low > current_CF_low)
                {
                    new_CF_high = new_CF_high - 1 ;
                }

                /* Update the new correction field */
                pkt_in_ptp_msg_ptr->correctionField_63_48 =  (new_CF_high >> 16) ;
                pkt_in_ptp_msg_ptr->correctionField_47_32 =  (new_CF_high & 0xFFFF) ;
                pkt_in_ptp_msg_ptr->correctionField_31_16 =  (new_CF_low >> 16) ;
                pkt_in_ptp_msg_ptr->correctionField_15_0  =  (new_CF_low & 0xFFFF) ;
                /*#pragma no_reorder*/
                break;

            default:

                /* Invalid ptp packet type !!! */

                /* Notify MG about this failure. Save error parameters in SP !!! */
                ErrParameters[0] = thread_id;
                ErrParameters[1] = desc_ptr->outer_l3_offset;
                ErrParameters[2] = ptp_pkt_type;
                ppn2mg_failure_event(PPN2MG_PTP_INVALID_ETHERNET_TYPE_EVENT,ErrParameters,3,DONT_STOP_PPN);
                break;
            }

            /* Copy Timestamp[31:30] to Desc<Ingress Timestamp Seconds>. */
            ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_OFFSET, 0);
        }
    }

    /* Must have it to pass PTP tests on board */
    /*#pragma flush_memory*/
    /*-----------------------------------------------------------------------------------------------------------
    4. Updating the descriptor.   Rest of the fields
     -----------------------------------------------------------------------------------------------------------*/
    /* PTP Packet Format = PTPv2 */
    desc_ptr->ptp_packet_format = PTP_PKT_FORMAT_PTPv2;
    /* Timestamp Offset = point to correctionField (The offset is in bytes from the beginning of the outgoing packet) */
    /* fw_bc_modification is added as well to reflect the header alteration that was done (add/remove bytes) */
    desc_ptr->timestamp_offset  = (desc_ptr->outer_l3_offset + ptp_ofst + CF_OFST) + (desc_ptr->fw_bc_modification);
    /* PTP Action = AddCorrectedTime        According to TalM this field will be ignored by the MAC when Desc<MAC Timestamping En> is disabled, so in this case its value is insignificant */
    desc_ptr->ptp_action = PTP_ACTION_ADD_CORRECTION_TIME;
    /*#pragma no_reorder*/

    /* PTP Dispatching En = 1 */
    ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_dispatching_en_OFFSET, 0);

    /* Check type of PTP processing in order to know how to set Desc<ptp_cf_wraparound_check_en> field
       Set it to '1' for PTP_OC_BC_EGRESS_TIMESTAMP_IN_CF otherwise it should be cleared to '0' */
    if (((PPA_FW_SP_BYTE_READ(PIPE_CFG_REGs_lo + PTP_SOURCE_PORT_MODE_OFFSET)) & PTP_SOURCE_PORT_MODE_MASK) == PTP_OC_BC_EGRESS_TIMESTAMP_IN_CF)
    {
        /* PTP CF Wraparound Check En = 1 */
        ACCEL_CMD_TRIG(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET, 0);
    }
    else
    {
		/* PTP CF Wraparound Check En = 0 */
		ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET, 0);
    }


    /* PTP Timestamp Queue Entry ID = 0 */
    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_entry_id_OFFSET, 0);
    /* PTP Egress TAI Sel = 0 */
    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_egress_tai_sel_OFFSET, 0);
    /* PTP Timestamp Queue Select = 0 */
    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_select_OFFSET, 0);
    /* Egress Pipe Delay = 10 msbits are 0, and 20 lsbits are the Egress Pipe Delay from the Target */
    /* Do it using 2 accelerator commands copy 10 bits each time. */
    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS10_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_PLUS10_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS20_OFFSET, 0);

exit_nicely:
#if EN_REC_PTP
    /* Record PTP output data if PTP record flag is set to on */
    if (enable_record)
    {
        record_data_PTP_handler(pkt_in_ethernet_type_ptr, pkt_in_ptp_msg_ptr, pkt_in_udp_checksum_ptr, REC_OUTPUT, thread_id );
    }
#endif

    return;
}

/*************************************************************************************************************************************************************//**
 * funcname        ECN_handler
 * inparam         None
 * return          None
 * description     Each forwarding thread can be followed by ECN marking.
 *                 Updating the ECN field in IP header and if required update the IP checksum as well.
 *                 ECN (Explicit Congestion Notification) is an extension to the TCP protocol. It allows end-to-end notification of network congestion without dropping packets.
 *                 CE (Congestion Encountered): This act is referred to as “marking” and its purpose is to inform the receiving endpoint of impending congestion.
 *                 At the receiving endpoint, this congestion indication is handled by the upper layer protocol (transport layer protocol) and needs to be echoed
 *                 back to the transmitting node in order to signal it to reduce its transmission rate.
 ******************************************************************************************************************************************************************/
inline void ECN_handler (struct nonQcn_desc *desc_ptr, uint32_t enable_record,uint32_t thread_id)
{
    uint32_t sumTo32bits;
    uint16_t new_ecn,current_ecn,current_cs;
    uint32_t ErrParameters[3];

    /* Get pointer to IPv4 and IPv6 headers. */
    struct IPv4_Header* pkt_in_IPv4_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + desc_ptr->outer_l3_offset + ETHERNET_SIZE,IPv4_Header);
    struct IPv6_Header* pkt_in_IPv6_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE + desc_ptr->outer_l3_offset + ETHERNET_SIZE,IPv6_Header);

    /* Prevent warning */
    enable_record = enable_record;

#if EN_REC_ECN
    /* Record ECN input data if ECN record flag is set to on */
    if (enable_record)
    {
        record_data_ECN_handler(pkt_in_IPv4_ptr,pkt_in_IPv6_ptr,REC_INPUT,thread_id);
    }
#endif

    if (pkt_in_IPv4_ptr->version == IPv4_VER)
    {   /* IPv4 version */

        /* Perform checksum update by calculation of the following formula = ~(~current_checksum + ~current_ecn + new_ecn) */
        new_ecn     = IP_ECN_CE;    /* Congestion Encountered */
        current_ecn = pkt_in_IPv4_ptr->ecn;
        current_cs  = pkt_in_IPv4_ptr->header_checksum;
        /*#pragma no_reorder*/

        /* Implement the formula above */
        sumTo32bits  = ( (~current_ecn) & 0xFFFF) ;
        /*#pragma no_reorder*/
        sumTo32bits += ( (~current_cs) & 0xFFFF);
        sumTo32bits += new_ecn;
        /*#pragma no_reorder*/

        /* Check for carry */
        while (sumTo32bits > 0xFFFF)
        {
            /* Add also the carry to the sum */
            sumTo32bits = (sumTo32bits>>16) + (sumTo32bits&0xFFFF);
        }
        /*#pragma no_reorder*/
        /* Update IPv4 ecn field with new value */
        pkt_in_IPv4_ptr->ecn = new_ecn;
        /* Perform not and update IPv4 header checksum field with the new value */
        pkt_in_IPv4_ptr->header_checksum = ~sumTo32bits ;
    }
    else if (pkt_in_IPv6_ptr->version == IPv6_VER)
    {   /* IPv6 version */

        /* Update IPv6 ecn field with new value */
        pkt_in_IPv6_ptr->ecn = IP_ECN_CE ;    /* Congestion Encountered */
    }
    else
    {   /* Invalid IP version !!! */

        /* Notify MG about this failure. Save error parameters in SP !!! */
        ErrParameters[0] = thread_id;
        ErrParameters[1] = desc_ptr->outer_l3_offset;
        ErrParameters[2] = pkt_in_IPv4_ptr->version;
        ppn2mg_failure_event(PPN2MG_ECN_INVALID_IP_VERSION_EVENT,ErrParameters,3,DONT_STOP_PPN);
    }

#if EN_REC_ECN
    /* Record ECN output data if ECN record flag is set to on */
    if (enable_record)
    {
        record_data_ECN_handler(pkt_in_IPv4_ptr,pkt_in_IPv6_ptr,REC_OUTPUT,thread_id);
    }
#endif

}

#if RECOVER_FROM_EXCEPTION
/********************************************************************************************************************//**
 * funcname        pipeRecoveFromException
 * inparam         None
 * return          None
 * description     Recover from core exception by executing packet swap sequence to send current packet and get the next one.
 *                 fw_drop bit is set in order to indicate that this packet should be dropped.
 *                 NOTE: can use the following instruction to generate an exception for testing => asm volatile ("syscall");
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void pipeRecoveFromException ()
{
    /* Get pointer to descriptor */
    struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
    desc_ptr->fw_drop = 1;

    /* Notify MG that PPN is recovered from an exception event  */
    ppn2mg_failure_event(PPN2MG_PPN_EXCEPTION_RECOVER_EVENT,0,0,DONT_STOP_PPN);

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}
#endif

/* ------------ */
/* --- main --- */
/* ------------ */
#ifndef ASIC_SIMULATION
int main()  {
    /* xt_iss_client_command("profile","disable"); */

    /* Initialization of PIPE project */
    pipe_init();

    /* Execute the packet swap sequence to jump to the first Thread */
    packet_swap_sequence();

    /* Infinite loop (should not stuck here) */
    __asm__ __volatile__(
            "main_swap_Loop:                  \n\t"
            "         J main_swap_Loop        \n\t" );

    /* Call all supported Threads. Should not get in here. Only done for compiler to build the Threads routines so we can get their PC addresses */
    call_threads();

    return 0;
}
#endif /*!ASIC_SIMULATION*/

/********************************************************************************************************************//**
 * funcname        pipe_init
 * inparam         None
 * return          None
 * description     Execute PIPE initialization
 *                 1. Init SHARED DRAM memory
 *                 2. Load all accelerators instructions data into SP memory
 *                 3. Call swap packet sequence to get the first thread
 ************************************************************************************************************************/
void    pipe_init()
{
    uint32_t ppn_id;

#ifdef ASIC_SIMULATION
    ppn_id = 0;
#else /*ASIC_SIMULATION*/
    /* get PPN ID register */
    asm("RSR.PRID %0" : "=a"(ppn_id));
#endif /*!ASIC_SIMULATION*/

    /* Set the ppn id so we will know which PPN is using this SP memory (third word from the beginning of SP) */
    PPA_FW_SP_WRITE(PPN_ID_ADDR,0x55000000 + ppn_id);

    /* Init the last 2 words of the stack with constant known pattern */
    stack_overflow_checker_power_up();

    /* Initialize shared data memory (SRAM) */
    /* PIPE A1 additions !!!: do not call this function !!!
       Due to PIPE A1 requests the whole 8KB memory is now set by MG for Egress Tag State.
       Memory contains vlan ports bits map which are now part of condition to check if vlan should be removed.  */
    /* init_shared_data_memory(ppn_id); */


    /* Load accelerator commands data into SP memory. NOTE: Can be loaded by host as part of the FW image. */
    load_accelerator_commands();

    /* Set ePort parameters */
    set_ePort_params();

    /* PPN exception power up sequence */
    exception_power_up();

#if ENABLE_RECORDING
    /* Call recording power up sequence */
    recording_powerup();
#endif

#ifndef ASIC_SIMULATION
    /* Call PPA fw print power up sequence */
    logging_powerup();
#endif
    /* PPN to MG IF initialization   */
    ppn2mg_powerup();

    /* Get pipe fw version and save it into fixed address in SP memory */
    get_fw_version();
}

/********************************************************************************************************************//**
 * funcname        exception_power_up
 * inparam         None
 * return          None
 * description     PPN exception power up sequence.
 ************************************************************************************************************************/
void exception_power_up()
{
    /* Initialize exception status to indicate no exception occured */
    exception_dump_buf[EXCEPTION_STATUS] = NO_EXCEPTION_TILL_NOW;
}

/********************************************************************************************************************//**
 * funcname        init_shared_data_memory
 * inparam         ppn_num: ppn id
 * return          None
 * description     Initialize shared ram memory which is common to all PPNs.
 *                 Use PPN 0 to initialize variables in shared memory and only when done set free the rest of the PPNs
 ************************************************************************************************************************/
/* Force not to optimize this function since it seems that the compiler optimized away the "else" and "while"  */
__attribute__((optimize ("-O0")))
void init_shared_data_memory(uint32_t ppn_num)
{
    uint32_t dram_init_done_val;

    /* Initializations performed by PPN_0 only */
    if (ppn_num == 0)
    {
        /* Mainly for initialization of tables and global variables in DRAM memory */
        /* */
        /* */

        /* Mark for all the rest of PPNs in SHARED DRAM memory that init procedure is done. */
        /* NOTE: According to Doron Schupper the problem is that the first read that most ppns will get whatever is found in address 0 of the shared dmem. */
        /*       In Some cases this data is undefined and theoretically can cause a "false" "900d_900d". */
        PPA_FW_SHARED_DRAM_WRITE(DRAM_INIT_DONE_OFST, DRAM_INIT_DONE_VALUE);
    }
    else
    { /* Wait for ppn0 to finish */
        dram_init_done_val = PPA_FW_SHARED_DRAM_READ(DRAM_INIT_DONE_OFST);
        while (dram_init_done_val!= DRAM_INIT_DONE_VALUE)
        {
            dram_init_done_val = PPA_FW_SHARED_DRAM_READ(DRAM_INIT_DONE_OFST);
        }
    }
}

/********************************************************************************************************************//**
 * funcname        set_ePort_params
 * inparam         None
 * return          None
 * description     Store some values relevant for ePort. Constant during run time but can be different between clients
 *                 Values were taken from Ilan Yerushalmi. Can also be loaded by host.
 ************************************************************************************************************************/
void set_ePort_params()
{
    PPA_FW_SP_SHORT_WRITE(MIN_E_PORT_MINUS_1_SP_ADDR,(11000-1) );
    PPA_FW_SP_SHORT_WRITE(MAX_E_PORT_SP_ADDR,        12000);
    PPA_FW_SP_SHORT_WRITE(MIN_E_PORT_SP_ADDR,        11000);
    PPA_FW_SP_SHORT_WRITE(MULTICAST_OFFSET_SP_ADDR,  4000);
}


/********************************************************************************************************************//**
 * funcname        get_fw_version
 * inparam         None
 * return          None
 * description     Get pipe fw version and save it into SP memory
 ************************************************************************************************************************/
void get_fw_version ()
{
    struct pipe_fw_version *ver_p = PIPE_MEM_CAST(PIPE_FW_VERSION_ADDR,pipe_fw_version);

    ver_p->name[0] = VER_CHAR_0;   /* 'P' */
    ver_p->name[1] = VER_CHAR_1;   /* 'I' */
    ver_p->name[2] = VER_CHAR_2;   /* 'P' */
    ver_p->name[3] = VER_CHAR_3;   /* 'E' */
    ver_p->major_x = VER_MAJOR;    /* Year */
    ver_p->minor_y = VER_MINOR;    /* Month */
    ver_p->local_z = VER_LOCAL;    /* Version number */
    ver_p->debug_d = VER_DEBUG;    /* Used for private versions, should be zero for official version */
}


/********************************************************************************************************************//**
 * funcname        stack_overflow_checker_power_up
 * inparam         None
 * return          None
 * description     Get the stack end address (goes from high address to low address).
 *                 Set the last 2 words with constant and known pattern.
 ************************************************************************************************************************/
void stack_overflow_checker_power_up()
{
#ifndef ASIC_SIMULATION

    /* Get the last sp address and save it in global variable */
    asm volatile("movi  a14, stack_end_addr \n\t"
                 "movi  a15, _stack_sentry \n\t"    /* Known symbol from memory map which points to the end of the stack */
                 "s32i  a15,  a14, 0 " );           /* Store _stack_sentry inside stack_end_addr  */


    /* Set the last 2 entries with constant and known pattern (0x900dCAFE) */
    PPA_FW_SP_WRITE(stack_end_addr-4,STACK_INITIAL_VAL);
    PPA_FW_SP_WRITE(stack_end_addr,STACK_INITIAL_VAL);

#endif
}


/********************************************************************************************************************//**
 * funcname        stack_overflow_checker
 * inparam         None
 * return          None
 * description     Check if stack was overflowed by reading the last 2 words.
 *                 If known pattern was overrun notify MG by triggering doorbell interrupt.
 ************************************************************************************************************************/
void stack_overflow_checker(uint32_t thrd_id)
{

    /* Prevent warning */
    thrd_id = thrd_id;

#ifndef ASIC_SIMULATION

    uint32_t    stack_val[2];

    /* Read the last 2 words of the stack */
    stack_val[0] = PPA_FW_SP_READ(stack_end_addr-4);
    stack_val[1] = PPA_FW_SP_READ(stack_end_addr);

    if ( (stack_val[0] != STACK_INITIAL_VAL) || (stack_val[1] != STACK_INITIAL_VAL) )
    {
        /* Notify MG that PPN stack is overflowed and add in which thread it occured  */
        stack_val[0] = thrd_id;
        ppn2mg_failure_event(PPN2MG_PPN_STACK_OVERFLOW_EVENT,stack_val,1, STOP_PPN);
    }

#endif
}

