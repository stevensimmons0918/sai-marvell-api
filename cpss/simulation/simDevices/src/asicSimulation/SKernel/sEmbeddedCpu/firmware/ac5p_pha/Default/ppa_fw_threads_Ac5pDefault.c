/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * ppa_fw_threads_Ac5pDefault.c
 *
 *  PPA fw threads functions
 *
 */

#ifdef ASIC_SIMULATION
    #ifdef _VISUALC
        #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
        #pragma warning(disable: 4244) /* conversion from 'uint32_t' to 'uint16_t', possible loss of data */
        #pragma warning(disable: 4146) /* unary minus operator applied to unsigned type, result still unsigned */

    #endif /*_VISUALC*/
#endif /*ASIC_SIMULATION*/

/* ==========================
  Include headers section
  =========================== */
#ifdef ASIC_SIMULATION
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_image_info.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_base_types.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_accelerator_commands.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_threads_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_exception.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/wm_asic_sim_defs.h"
static SKERNEL_DEVICE_OBJECT * devObjPtr;
#else
#include "ppa_fw_image_info.h"
#include "ppa_fw_base_types.h"
#include "ppa_fw_defs.h"
#include "ppa_fw_accelerator_commands.h"
#include "ppa_fw_threads_defs.h"
#include "ppa_fw_ppn2mg.h"
#include "ppa_fw_exception.h"
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include <xtensa/sim.h>


/********************************************************************************************************************//**
 * funcname        pragma_no_reorder
 * inparam         None
 * return          None
 * description     Activate compiler pragma no_reorder command.
 *                 Telling the compiler not to change order of instructions at this point.
 ************************************************************************************************************************/
INLINE void pragma_no_reorder(){
            #pragma no_reorder
    }

/********************************************************************************************************************//**
 * funcname        pragma_frequency_hint_never
 * inparam         None
 * return          None
 * description     Activate compiler pragma frequency_hint command.
 *                 Telling the compiler that this code is hardly executed
 ************************************************************************************************************************/
INLINE void pragma_frequency_hint_never(){
            #pragma frequency_hint NEVER
    }

/********************************************************************************************************************//**
 * funcname        pragma_frequency_hint_frequent
 * inparam         None
 * return          None
 * description     Activate compiler pragma frequency_hint command.
 *                 Telling the compiler that this code is most likely to be executed
 ************************************************************************************************************************/
INLINE void pragma_frequency_hint_frequent(){
            #pragma frequency_hint FREQUENT
    }

/********************************************************************************************************************//**
 * funcname        first_packet_swap_sequence
 * inparam         None
 * return          None
 * description     Activate the below sequence: send the processing packet, gets new one, load the new PC thread and jump to it
 ************************************************************************************************************************/
INLINE void packet_swap_sequence()
{
    uint32_t   ppa_deq  ;

    /* step 1 - swap instruction, activate packet Out & packet In machines (Enqueue packet) */
    PPA_FW_SP_WRITE(PKT_SWAP_INST_ADDR,0);
    /* step 2 - flush store before activating the blocking load */
    #pragma flush_memory
    /* step 3 - initialize a15 for pc blocking load.  load start of new packet scenario PC. */
    ppa_deq=PPA_FW_SP_READ(PKT_LOAD_PC_INST_ADDR);
    /* steps 4  - jump to start of scenario - start execution */
    asm volatile("JX %0" :: "a"(ppa_deq));
}

#endif /*!ASIC_SIMULATION*/


/* ==========================
  Threads section
  =========================== */

#ifdef ASIC_SIMULATION
/********************************************************************************************************************//**
* funcname        Ac5pDefault_get_fw_version
* inparam         fwImageId    PHA firmware image ID
* return          None
* description     Get PHA fw version and save it into SP memory (used in WM)
************************************************************************************************************************/
void Ac5pDefault_get_fw_version(GT_U32 fwImageId)
{
    /* Get ac5p fw version and save it into fixed address in SP memory */
    ac5p_get_fw_version(fwImageId, ((VER_YEAR<<24) | (VER_MONTH<<16) | (VER_IN_MONTH<<8) | VER_DEBUG ));
}
#endif /*ASIC_SIMULATION*/



/********************************************************************************************************************//**
 * funcname        THR0_DoNothing
 * inparam         None
 * return          None
 * description     No modifications to the packet and no additions or remove of any tags or headers.
 *                 Packet is out as it is in.
 *                 Note:
 *                 - this thread is mapped to a fix address and should not be changed.
 *                 - allows to execute the thread while fw version is being upgraded.
 *                 - do not use accelerator commands since it will change thread's code
 *                   since in each build accelerator commands might be in different location
 ************************************************************************************************************************/
__attribute__ ((section(".iram0.text")))
__attribute__ ((aligned (16)))
void PPA_FW(THR0_DoNothing)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(0);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR1_SRv6_End_Node
 * inparam      None
 * return       None
 * description  Covers the IPv6 Segment Routing (SR) End Node use case
 *              cfg template:    not in use(16B)
 *              targetPortTable: not in use(4B)
 *              Incoming packet: Expansion space(32B),IPv6(40B),IPv6 Ext. hdrs(n*8B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Output packet:   Expansion space(32B),IPv6(40B),IPv6 Ext. hdrs(n*8B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 32B Expansion space + 128B packet
 *              Use Desc<SR EH Offset> to find the location of the fields.
 *              Decrement and update the SR Header<Segments Left> field
 *              Update Ipv6 destination IP with the selected segment list IP.
 *              Do it only if packet size is still under the maximum allowed size of 128 bytes otherwise drop.
 *              NOTE: Since required Segment list can reach to location beyond 127B need to set offset register
 *                    in order for the accelerator command to work correctly
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR1_SRv6_End_Node)() {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t segments_left,ipv6_eh_size,segments_left_ofst;
    uint32_t selected_segment_list_ofst;

    /* Get pointer to descriptor */
    struct ppa_in_desc*  thr1_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /* Indicates, in 8 bytes units, where the Segment Routing Extension Header is located relative to end of IPv6 Base header.
       Basically it tells us if there are IPv6 Extension headers or not and their sizes. 0xn - SR routing header is 8*n bytes after the base header */
    __LOG_FW(("Indicates, in 8 bytes units, where the Segment Routing Extension Header is located relative to end of IPv6 Base header."));
    ipv6_eh_size = thr1_desc_ptr->phal2ppa.sr_eh_offset*EIGHT_BYTES;
    __LOG_PARAM_FW(ipv6_eh_size);

    /* Calculate offset to SRH<Segment left> field */
    __LOG_FW(("Calculate offset to SRH<Segment left> field"));
    segments_left_ofst = IPV6_SR_EXPANSION_SPACE_SIZE + IPV6_BASIC_HEADER_SIZE + ipv6_eh_size + SEGMENT_LEFT_OFST_IN_SR_HDR ;
    __LOG_PARAM_FW(segments_left_ofst);

    /* Check that SRH<Segment left> is in the range of PPA buffer (160B = Exp space:32B + input packet:128B) otherwise drop */
    __LOG_FW(("Check that SRH<Segment left> is in the range of PPA buffer (160B = Exp space:32B + input packet:128B) otherwise drop"));
    if ( segments_left_ofst < (IPV6_SR_EXPANSION_SPACE_SIZE+IPv6_SR_PKT_MAX_LENGTH) )
    {
        /* Read SRH<Segment left> index. Decrement by 1 to point to the right Segment list */
        __LOG_FW(("Read SRH<Segment left> index. Decrement by 1 to point to the right Segment list"));
        segments_left = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + segments_left_ofst) - 1;
        __LOG_PARAM_FW(segments_left);

        /* Calculates the offset to the start of the required Segment list */
        __LOG_FW(("Calculates the offset to the start of the required Segment list"));
        selected_segment_list_ofst = IPV6_SR_EXPANSION_SPACE_SIZE + IPV6_BASIC_HEADER_SIZE + ipv6_eh_size + SR_BASIC_HEADER_SIZE + (SEGMENT_LIST_SIZE*segments_left);
        __LOG_PARAM_FW(selected_segment_list_ofst);

        /* Check Segment list is in the range of PPA buffer. Since it's 16B long max valid offset is 144B (160-16) otherwise drop */
        __LOG_FW(("Check Segment list is in the range of PPA buffer. Since it's 16B long max valid offset is 144B (160-16) otherwise drop"));
        if (selected_segment_list_ofst <= (IPV6_SR_EXPANSION_SPACE_SIZE+IPv6_SR_PKT_MAX_LENGTH-SEGMENT_LIST_SIZE))
        {
            /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
               Since in this use case might be a case in which Segment list location exceeds 127 bytes need to use PPN offset register.
               By default this register is used all the time and the default value is zero.
               This time we will set it to point to IPv6 which means that the pkt src and dst will be relative to 32B
               HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
            __LOG_FW(("Set offset register to point to IPv6"));
            PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 32);
            __PRAGMA_NO_REORDER

            /* Update IPv6 destination address from the segment list */
            __LOG_FW(("Update IPv6 destination address from the segment list"));
            ACCEL_CMD_LOAD_PKT_SRC(COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET, ( (PKT<<7) | (selected_segment_list_ofst-32) ));
            __PRAGMA_NO_REORDER
            ACCEL_CMD_TRIG(COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET, 0);

            /* Update SRH<Segment left> field after decrement it by 1 */
            __LOG_FW(("Update SRH<Segment left> field after decrement it by 1"));
            PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + segments_left_ofst,segments_left);
            __LOG_PARAM_FW(segments_left);

            /* Set offset register back to zero (default value)  */
            __LOG_FW(("Set offset register back to zero (default value)"));
            PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);
        }
        else
        {   /* Drop the packet */
            __LOG_FW(("Drop the packet use case"));
            __PRAGMA_FREQUENCY_HINT_NEVER

            /* Packet command allocated by FW set to Hard drop  */
            __LOG_FW(("Packet command allocated by FW set to Hard drop "));
            thr1_desc_ptr->ppa_internal_desc.fw_packet_command = HARD_DROP;
            __LOG_PARAM_FW(thr1_desc_ptr->ppa_internal_desc.fw_packet_command);
        }
    }
    else
    {   /* Drop the packet */
        __LOG_FW(("Drop the packet use case"));
        __PRAGMA_FREQUENCY_HINT_NEVER

        /* Packet command allocated by FW set to Hard drop  */
        __LOG_FW(("Packet command allocated by FW set to Hard drop "));
        thr1_desc_ptr->ppa_internal_desc.fw_packet_command = HARD_DROP;
        __LOG_PARAM_FW(thr1_desc_ptr->ppa_internal_desc.fw_packet_command);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(1);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR2_SRv6_Source_Node_1_segment
 * inparam      None
 * return       None
 * description  Support generation of SRv6 Source node 1 segment
 *              cfg template:    new ipv6 sip (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv6(40B),UDP(8B),VXLAN(8B), Passenger
 *              hdr out: Extension space(08B),IPv6(40B),SRH(8B),Segment0(16B),UDP(8B),VXLAN(8B),Passenger
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *              Firmware adds 24 bytes of SRH(8B) + Segment0(16B)
 *              Update IPv6 SIP with new value taken from the CFG template
 *              Updates IPv6<Payload Length> += 24
 *              Copy IPv6 SIP to Segment0
 *              Copy Thread<Template> to IPv6 SIP
 *              Set SRH header with specific values defined by architect
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR2_SRv6_Source_Node_1_segment)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move IPv6 header left by 24 bytes to make room for SRH(8B) + SR segment(16B)
      - Need to do it in 2 commands since need to move more than 32B (IPv6 is 40B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes of IPv6 */
    __LOG_FW(("Move left the first 32 bytes using accelerator cmd"));
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN32_FROM_PKT_IPv6_Header__version_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Move left the last 8 bytes of IPv6 */
    ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Segment with origin IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6 SIP to Segment0 */
    __LOG_FW(("Copy IPv6 SIP to Segment0 using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero. */
    __LOG_FW(("Use max constant 11 bits command to set SRH<Routing Type>=4 and SRH<Segments Left>=1. Clear rest bit to zero."));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B
       therefore copy also IPv6<hope_limit> to SRH<hdr_ext_len> which will later be overrun with correct value */
    __LOG_FW(("SRH<Next Header>= IPv6<Next Header> Can not use copy bits command to copy bits from pkt to pkt therefore use copy bytes which must copy minimum 2B"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* SRH<Hdr Ext Len>=2 */
    __LOG_FW(("SRH<Hdr Ext Len>=2"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_OFFSET, 0);

    /* SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) */
    __LOG_FW(("SRH<Last Entry>=0, SRH<Flags>=0, SRH<Tag>=0 (clear 4 LS bytes to zero) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update origin IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy Thread<Template> to IPv6 SIP */
    __LOG_FW(("Copy Thread<Template> to IPv6 SIP using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);

    /* Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 */
    __LOG_FW(("Increment IP header payload length field by 24B.  IPv6 Header<payload length>= IPv6 Header<payload Length> + 24 using accelerator cmd"));
    ACCEL_CMD_TRIG(ADD_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);

    /* IPv6<Next Header>=43 (0x2B) */
    __LOG_FW(("IPv6<Next Header>=43 (0x2B) using accelerator cmd"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Updates Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 24 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);
    __PRAGMA_NO_REORDER


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(2);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR3_SRv6_Source_Node_First_Pass_2_3_segments
 * inparam      None
 * return       None
 * description  Handles first pass thread for 2 or 3 segments.
 *              Adds one segment of 16B. SRH will be added in second pass with the option to add maximum of 2 more segments.
 *              cfg template:      new ipv6 sip (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),L2(12B),eDSA(16B),ET(2B),IPv6(40B),Passenger
 *              hdr out: Extension space(16B),L2(12B),eDSA(16B),ET(2B),IPv6(40B),Segment2(16B),Passenger
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *              Firmware adds 16B after IPv6. Those bytes are marked as Segment2 (deepest segment).
 *              It copies IPv6 SIP to Segment2
 *              It copies Thread<Template> to IPv6 SIP
 *              Sets eDSA<target ePort>++
 *              eDSA<Is Trg Phy Port Valid>=0
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move L2,eDSA,ET and IPv6 headers left by 16 bytes to make room for Segment2(16B)
      - Need to do it in 3 commands since need to move more than 32B (L2(12B) + eDSA(16B) + ET(2B) + IPv6(40B) is 70B)
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left the first 32 bytes */
    __LOG_FW(("Move left the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Move the next 32 bytes */
    __LOG_FW(("Move left the next 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Move the last 6 bytes */
    __LOG_FW(("Move left the last 6 bytes"));
    ACCEL_CMD_TRIG(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update SR Segment with origin IPv6 SIP
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy IPv6 SIP to Segment2 (MINUS16 in command since we already moved the packet left) */
    __LOG_FW(("Copy IPv6 SIP to Segment2 (MINUS16 in command since we already moved the packet left)."));
    ACCEL_CMD_TRIG(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update origin IPv6 header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy CFG<Template> to IPv6 SIP */
    __LOG_FW(("Copy CFG<Template> to IPv6 SIP"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update eDSA tag
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* eDSA<target ePort>++ */
    __LOG_FW(("eDSA<target ePort>++"));
    ACCEL_CMD_TRIG(ADD_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_OFFSET, 0);
    /* eDSA<Is Trg Phy Port Valid>=0 */
    __LOG_FW(("eDSA<Is Trg Phy Port Valid>=0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). In this case plus 16 bytes"));
    ACCEL_CMD_TRIG(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);
    __PRAGMA_NO_REORDER


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(3);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR4_SRv6_Source_Node_Second_Pass_3_segments
 * inparam      None
 * return       None
 * description  Handles second pass thread for 3 segments.
 *              Basically adds SRH(8B) + Segment0(16B) + Segment1(16B).
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Segment2(16B)
 *              hdr out: Extension space(62B),L2(14B or 18B),IPv6(40B),SRH(8B),Segment0(16B),Segment1(16B),Segment2(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              Use Desc<Egress Tag state> to determine if Layer2 header is 14 bytes or 18 bytes
 *              Add 40 Bytes after IPv6 Base Header for SRH, Segment0 and Segment1
 *              IPv6BaseHeader<Payload Length>+= 56 (It includes the 16B added in the first pass + 40B added here)
 *              Set SRH Base, Segment0 and Segment1 from Generic TS<data (36B),EthType(2B) & VLAN(1B)>
 *              Update Desc<fw_bc_modification> field to -14B (0xF2) since we add 40B but remove 54B of Generic TS
 *              NOTE: - Since this thread handles packet with and without vlan there are 2 types of accelerators to support both cases
 *                      these accelerators are placed one after the other. First one handles no vlan and second (next 4B) handles with vlan
 *                    - Require set of offset register at some point to enable accelerator command to reach location beyond 127 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  egressTagState;          /* Holds Desc<egress_tag_state> which indicates if vlan exist */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr4_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr4_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr4_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(AC5P_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 122);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 116, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(AC5P_PKT_REGs_lo + 117, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 119, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
        __PRAGMA_NO_REORDER
     }
     else if(egressTagState == 1)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 122 + 4);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 40B but remove 54B so total minus 14B (0xF2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Move Generic TS(54B),L2(14B or 18B) and IPv6(40B) headers left by 40 bytes to make room for SRH(8B) + Container0(16B) + Container1(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B or 112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 40B which means that the pkt src and dst will be relative to 8B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 8);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 116 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(AC5P_PKT_REGs_lo + 117 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 119 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 56 (16B in pass1 and 40B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0 and Container1
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy Generic TS< Data Bytes [31:16]> to Container 0
           Copy Generic TS< Data Bytes [15:0]> to Container 1 */
        __LOG_FW(("Copy Generic TS< Data Bytes [31:16]> to Container 0. Copy Generic TS< Data Bytes [15:0]> to Container 1"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
        __PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr4_desc_ptr->phal2ppa.egress_tag_state);
     }

     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(4);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR5_SRv6_Source_Node_Second_Pass_2_segments
 * inparam      None
 * return       None
 * description  Handles second pass thread for 2 segments.
 *              Basically adds SRH(8B) + Segment0.
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),Generic TS(54B),L2(14B or 18B),IPv6(40B),Segment1(16B)
 *              hdr out: Extension space(78B),L2(14B or 18B),IPv6(40B),SRH(8B),Segment0(16B),Segment1(16B)
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              Use Desc<Egress Tag state> to determine if Layer2 header is 14 bytes or 18 bytes
 *              Add 8 Bytes after IPv6 Base Header for SRH and Segment0
 *              IPv6BaseHeader<Payload Length>+= 40 (It includes the 16B added in the first pass and 24B added here)
 *              Set SRH and Segment0 according to Generic TS.
 *              Update Desc<fw_bc_modification> field to -30B (0xE2) since we add 24B but remove 54B of Generic TS
 *              NOTE: - Since this thread handles packet with and without vlan there are 2 types of accelerators to support both cases
 *                      these accelerators are placed one after the other. First one handles no vlan and second (next 4B) handles with vlan
 *                    - Require set of offset register at some point to enable accelerator command to reach location beyond 127 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint8_t  egressTagState;          /* Holds Desc<egress_tag_state> which indicates if vlan exist */
    uint8_t  ipv6NextHeader;          /* Holds IPv6 next header field value */
    uint16_t geneircTsDataEthertype;  /* Holds Generic TS Data Ethertype field value */
    uint8_t  geneircTsDataVid;        /* Holds Generic TS Data Vlan ID LS byte value */

    /* Get pointers to descriptor */
    struct ppa_in_desc* thr5_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr5_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr5_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(AC5P_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 122);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Move Generic TS(54B),L2(14B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (108B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 12 bytes */
        __LOG_FW(("Move left the next 12 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case the location of some of the packet's fields exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 132, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(AC5P_PKT_REGs_lo + 133, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 135, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
        __PRAGMA_NO_REORDER
     }
     else if(egressTagState == 1)
     {  /* with vlan */
        __LOG_FW(("with vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo + 122 + 4);
        __LOG_PARAM_FW(ipv6NextHeader);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update Desc<fw_bc_modification> field
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header. In this case we add 24B but remove 54B so total minus 30B (0xE2)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Move Generic TS(54B),L2(18B) and IPv6(40B) headers left by 24 bytes to make room for SRH(8B) + Container0(16B)
          - Need to do it in 4 commands since need to move more than 32B (112B)
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
        /* Move left the first 32 bytes */
        __LOG_FW(("Move left the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 32 bytes */
        __LOG_FW(("Move left the next 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Move the next 16 bytes */
        __LOG_FW(("Move left the next 16 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Pkt dst field in accelerator command is only 7 bits which means can hold packet offset up to 127 bytes.
           Since in this use case location of last 32B of IPv6 exceed 127 bytes need to use PPN offset register.
           By default this register is used all the time and the default value is zero.
           This time we will set it to point to start of Generic TS after shift left by 24B which means that the pkt src and dst will be relative to 24B
           HW accelerator will add the offset to the current pkt src and dst values and so will point to the exact location */
        __LOG_FW(("Set offset register to point to L2"));
        PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 24);
        __PRAGMA_NO_REORDER

        /* Move the last 32 bytes */
        __LOG_FW(("Move the last 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set SRH
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy IPv6.Next Header to SRH Base Header.Next Header */
        __LOG_FW(("Copy IPv6.Next Header to SRH Base Header.Next Header"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 132 + 4, ipv6NextHeader );
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len.
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(AC5P_PKT_REGs_lo + 133 + 4, geneircTsDataEthertype );

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo + 135 + 4, geneircTsDataVid );

        /* Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0] */
        __LOG_FW(("Copy Generic TS< Data Bytes [35:32]> to SRH Bytes [3:0]"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Update IPv6
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2) */
        __LOG_FW(("IPv6.Payload Length += 40 (16B in pass1 and 24B in pass2)"));
        ACCEL_CMD_TRIG(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, 0);

        /* IPv6<Next Header>=43 (0x2B) */
        __LOG_FW(("IPv6<Next Header>=43 (0x2B)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, 0);

        /*------------------------------------------------------------------------------------------------------------------------------------------------
          - Set Container0
          ------------------------------------------------------------------------------------------------------------------------------------------------*/
       /* Copy Generic TS< Data Bytes [15:0]> to Container 0 */
        __LOG_FW(("Copy Generic TS< Data Bytes [15:0]> to Container 0"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, 0);
        __PRAGMA_NO_REORDER
     }
     else
     {  /* not valid: packet should not be changed */
        __LOG_FW(("not valid: packet should not be changed"));

        __LOG_FW((" Desc<Egress Tag state> is not '0' nor '1' "));
        __LOG_PARAM_FW(thr5_desc_ptr->phal2ppa.egress_tag_state);
     }

     /* Set offset register back to zero (default value) */
     __LOG_FW(("Set offset register back to zero (default value)"));
     PPA_FW_SP_WRITE(PPN_IO_REG_PKT_HDR_OFST_ADDR, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(5);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR11_VXLAN_GPB_SourceGroupPolicyID
 * inparam      None
 * return       None
 * description  Thread to support VXLAN-GPB (Group Policy Based) when the Source Group Policy ID is carried in the Desc<Copy Reserved> field.
 *              The <Copy Reserved> bits that are allocated for the Source Group Policy ID should be copied to the VXLAN-GBP field <Group Policy ID>.
 *              Support IPv4 and IPv6 packets.
 *
 *              cfg template:    | reserved 0 (4B)|
 *                               | reserved 1 (4B)|
 *                               | reserved 2 (4B)|
 *                               | reserved 3 (22b) | CopyReservedMSB (5b) | CopyReservedLSB (5b) |
 *
 *                               CopyReservedMSB:
 *                               - Indicates fw the MS bit of the Group Policy ID value inside Desc<copy reserved> field.
 *                               - Meaning which bit is the last of Group Policy value inside Desc<copy reserved> field.
 *                               - Applicable values: For AC5P it should be 1:18. It should be checked by the CPSS API and not by fw.
 *                               CopyReservedLSB:
 *                               - Indicates fw the LS bit of the Group Policy ID value inside Desc<copy reserved> field.
 *                               - Meaning from which bit the Group Policy value starts inside Desc<copy reserved> field.
 *                               - Applicable values: For AC5P it should be 1:18. It should be checked by the CPSS API and not by fw.
 *
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space (32B) | IPv4/IPv6(20B/40B) | UDP(8B) | VXLAN-GPB(8B) | original packet |
 *              hdr out: Extension space (32B) | IPv4/IPv6(20B/40B) | UDP(8B) | VXLAN-GPB(8B) | original packet |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment
 *
 *              Firmware actions:
 *              - Extracts Group Policy ID value out from Desc<copy reserved>
 *              - Copies it into packet VXLAN header<Group Policy ID>
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint32_t    groupPolicyIdLength;
    uint32_t    groupPolicyIdmask;
    uint16_t    groupPolicyIdValue;

    /* Get pointers to descriptor & input packet (can be either IPv4 or IPv6) */
    struct thr11_vxlan_gpb_cfg* thr11_cfg_ptr = AC5P_MEM_CAST(AC5P_CFG_REGs_lo,thr11_vxlan_gpb_cfg);
    struct thr11_vxlan_gpb_in_hdr_ipv4* thr11_ipv4_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv4);
    struct thr11_vxlan_gpb_in_hdr_ipv6* thr11_ipv6_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv6);


    /* Use CopyReservedMSB and CopyReservedLSB from template to check number of bits of Group Policy ID value */
    __LOG_FW(("Use CopyReservedMSB and CopyReservedLSB from template to check number of bits of Group Policy ID value"));
    __LOG_PARAM_FW(thr11_cfg_ptr->vxlan_gpb_template.CopyReservedMSB);
    __LOG_PARAM_FW(thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB);
    groupPolicyIdLength = (thr11_cfg_ptr->vxlan_gpb_template.CopyReservedMSB - thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB) + 1;
    __LOG_PARAM_FW(groupPolicyIdLength);

    /* Set Group Policy ID mask compatible to number of bits */
    __LOG_FW(("Set Group Policy ID mask compatible to number of bits"));
    groupPolicyIdmask = (1 << groupPolicyIdLength) - 1 ;
    __LOG_PARAM_FW(groupPolicyIdmask);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Extract Group Policy ID value from Desc<copy_reserved> field
      -
      -  1. Read 32 bits from descriptor while the 20 MS bits are copy_reserved field
      -  2. Move right by 12 bits so copy_reserved field starts from bit#0
      -  3. Now move right by CopyReservedLSB bits so the Group Policy ID value from copy_reserved field will start from bit #0
      -  4. Mask to get Group Policy ID value
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
      __LOG_FW(("Extract Group Policy ID value from Desc<copy_reserved> field"));

    __LOG_PARAM_FW(PPA_FW_SP_READ(AC5P_DESC_REGs_lo + 12));
    groupPolicyIdValue = (PPA_FW_SP_READ(AC5P_DESC_REGs_lo + 12) >> (12 + thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB) ) & groupPolicyIdmask ;
    __LOG_PARAM_FW(groupPolicyIdValue);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Write Group Policy ID value into packet VXLAN header<Group_Policy_ID>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
      __LOG_FW(("Write Group Policy ID value into packet VXLAN header<Group_Policy_ID>"));

    /* Check packet IP version (IPv4 or IPv6) */
    __LOG_FW(("Check packet IP version (IPv4 or IPv6)"));
    __LOG_PARAM_FW(thr11_ipv4_in_hdr_ptr->IPv4_Header.version);
    if (thr11_ipv4_in_hdr_ptr->IPv4_Header.version == 4)
    {
        /* Packet is IPv4. Set VXLAN header<Group_Policy_ID> with Group Policy ID */
        __LOG_FW(("Packet is IPv4. Set VXLAN header<Group_Policy_ID> with Group Policy ID"));
        PPA_FW_SP_SHORT_WRITE( (AC5P_PKT_REGs_lo + 62) , groupPolicyIdValue );
    }
    else if (thr11_ipv6_in_hdr_ptr->IPv6_Header.version == 6)
    {
        /* Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID */
        __LOG_FW(("Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID"));
        PPA_FW_SP_SHORT_WRITE( (AC5P_PKT_REGs_lo + 62 + 20) , groupPolicyIdValue );
    }
    else
    {
        /* WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet */
        __LOG_FW(("WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet"));
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(11);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR12_MPLS_SR_NO_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),data(4B),L1(4B),L2(4B),L3(4B),L4(4B),L5(4B),L6(4B),L7(4B),L8(4B)
 *              hdr out: Extension space(52B),L1(4B),L2(4B),L3(4B),L4(4B),L5(4B),L6(4B),L7(4B),L8(4B)
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread no EL (Entropy Label) is added however MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR12_MPLS_SR_NO_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr12_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);
    struct thr12_MPLS_NO_EL_in_hdr* thr12_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr12_MPLS_NO_EL_in_hdr);

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR12_MPLS_SR_NO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). 48B is the incoming L3 offset. */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement). 48B is the incoming L3 offset."));
    thr12_desc_ptr->ppa_internal_desc.fw_bc_modification = (48 - thr12_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr12_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr12_desc_ptr->ppa_internal_desc.fw_bc_modification);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed "));
    ac5p_stack_overflow_checker(12);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR13_MPLS_SR_ONE_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(44B),L1,L2,L3,L4,L5,L6,ELI_1,EL_1,L7,L8
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread one pair of ELI(Entropy Label Indicator) and EL(Entropy Label) is added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR13_MPLS_SR_ONE_EL)() {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t  eli1_cmd_byte_position ;

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr13_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);
    struct thr13_MPLS_ONE_EL_in_hdr* thr13_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr13_MPLS_ONE_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement)  */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr13_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr13_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr13_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr13_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    eli1_cmd_byte_position = ( (PKT<<7)|thr13_in_hdr_ptr->MPLS_data.EL1_ofst ) ;
    __LOG_PARAM_FW(thr13_in_hdr_ptr->MPLS_data.EL1_ofst);
    __LOG_PARAM_FW(eli1_cmd_byte_position);

    /*--------------------------------------------------------------------------------------------------------------------------------
      - Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending label's offset) to make room for EL & ELI labels"));

    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             is after first label and maximum should be after origin label 8 */
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (thr13_in_hdr_ptr->MPLS_data.EL1_ofst-48), 0);
    __PRAGMA_NO_REORDER

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Trigger the command: Copy 4B of preceding label into ELI label */
    __LOG_FW(("Trigger the command: Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_LOAD_PKT_DEST_AND_PKT_SRC(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, \
                                           eli1_cmd_byte_position,(eli1_cmd_byte_position-4) ) ;
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]>
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]>"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*--------------------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      --------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(13);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR14_MPLS_SR_TWO_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not is use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(36B),L1,L2,L3,L4,L5,L6,ELI_2,EL_2,L7,L8,ELI_1,EL_1
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread two pairs of ELI(Entropy Label Indicator) and EL(Entropy Label) are added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR14_MPLS_SR_TWO_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint8_t  eli1_cmd_byte_position, eli2_cmd_byte_position, eli1_ofst, eli2_ofst, cpy_preceding_lbl1_cmd_ofst, cpy_preceding_lbl2_cmd_ofst ;

    /* Get pointer to descriptor  */
    struct ppa_in_desc* thr14_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);
    struct thr14_MPLS_TWO_EL_in_hdr* thr14_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr14_MPLS_TWO_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement)  */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr14_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr14_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr14_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr14_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI1 label. Set MS bit to 1 to define pkt type for accelerator command usage
       Get the offset to the command of copy preceding label according to ELI offset minus 4.
       First copy command starts from offset 28 since first location to add ELI is after L1 therefore L1 is the preceding label to copy from.
       L1 can be shifted left maximum 8*3 for case of 3 MPLS labels in this case its offset is 28B so this is the first preceding copy command */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    __LOG_FW(("Get the offset to the command of copy preceding label according to ELI offset minus 4. First copy command starts from offset 28"));
    eli1_cmd_byte_position = ( (PKT<<7)|thr14_in_hdr_ptr->MPLS_data.EL1_ofst ) ;
    eli1_ofst = thr14_in_hdr_ptr->MPLS_data.EL1_ofst;
    cpy_preceding_lbl1_cmd_ofst = (eli1_ofst - 4) - 28 ;
    __LOG_PARAM_FW(eli1_cmd_byte_position);
    __LOG_PARAM_FW(eli1_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl1_cmd_ofst);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli2_cmd_byte_position = ( (PKT<<7)|thr14_in_hdr_ptr->MPLS_data.EL2_ofst ) ;
    eli2_ofst = thr14_in_hdr_ptr->MPLS_data.EL2_ofst;
    cpy_preceding_lbl2_cmd_ofst = (eli2_ofst - 4) - 28 ;
    __LOG_PARAM_FW(eli2_cmd_byte_position);
    __LOG_PARAM_FW(eli2_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl2_cmd_ofst);


    /*------------------------------------------------------------------------------------------------------------------
      -  Handle first EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle first EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             after first label and maximum should be after origin label 8 */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (eli1_ofst-48), 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl1_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET + cpy_preceding_lbl1_cmd_ofst, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0, Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------
      -  Handle second EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET + (eli2_ofst-40), 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET + cpy_preceding_lbl2_cmd_ofst, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli2_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli2_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label: TC, TTL, BoS =0, Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli2_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(14);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR15_MPLS_SR_THREE_EL
 * inparam      None
 * return       None
 * description  Ingress packets are bridged and associated with Generic TS (Contains labels and data how to generate the MPLS tunnel)
 *              PHA process the Generic Tunnel to generate the required MPLS SR header
 *              cfg template:    Not in use (16B)
 *              src & trg entries: Not is use (4B)
 *              hdr in:  Extension space(48B),data,L1,L2,L3,L4,L5,L6,L7,L8
 *              hdr out: Extension space(28B),L1,L2,L3,ELI_3,EL_3,L4,L5,L6,ELI_2,EL_2,L7,L8,ELI_1,EL_1
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 112 bytes + 48 bytes Header increment
 *              MPLS data includes information regarding the offsets to the new labels.
 *              In this specific thread three pairs of ELI(Entropy Label Indicator) and EL(Entropy Label) are added.
 *              MPLS data is removed and origin labels might be removed as well.
 *              Also BoS bit of last label (the one preceding the passenger) is set to '1' to indicate last label.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR15_MPLS_SR_THREE_EL)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint8_t eli1_cmd_byte_position, eli2_cmd_byte_position, eli3_cmd_byte_position, eli1_ofst, eli2_ofst, eli3_ofst, cpy_preceding_lbl1_cmd_ofst, cpy_preceding_lbl2_cmd_ofst, cpy_preceding_lbl3_cmd_ofst ;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr15_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);
    struct thr15_MPLS_THREE_EL_in_hdr* thr15_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr15_MPLS_THREE_EL_in_hdr);

    /* Indicates HW about the new starting point of the packet header (two's complement) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (two's complement) "));
    thr15_desc_ptr->ppa_internal_desc.fw_bc_modification = (48-thr15_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr15_in_hdr_ptr->MPLS_data.New_L3_ofst);
    __LOG_PARAM_FW(thr15_desc_ptr->ppa_internal_desc.fw_bc_modification);

    /* Get the destination position which points to ELI1 label. Set MS bit to 1 to define pkt type for accelerator command usage
       Get the offset to the command of copy preceding label according to ELI offset minus 4.
       First copy command starts from offset 28 since first location to add ELI is after L1 therefore L1 is the preceding label to copy from.
       L1 can be shifted left maximum 8*3 for case of 3 MPLS labels in this case its offset is 28B so this is the first preceding copy command */
    __LOG_FW(("Get the destination position which points to ELI label. Set MS bit to 1 to define pkt type for accelerator command usage"));
    __LOG_FW(("Get the offset to the command of copy preceding label according to ELI offset minus 4. First copy command starts from offset 28"));
    eli1_ofst = thr15_in_hdr_ptr->MPLS_data.EL1_ofst;
    cpy_preceding_lbl1_cmd_ofst = (eli1_ofst - 4) - 28 ;
    eli1_cmd_byte_position = ( (PKT<<7)|eli1_ofst) ;
    __LOG_PARAM_FW(eli1_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl1_cmd_ofst);
    __LOG_PARAM_FW(eli1_cmd_byte_position);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli2_ofst = thr15_in_hdr_ptr->MPLS_data.EL2_ofst;
    cpy_preceding_lbl2_cmd_ofst = (eli2_ofst - 4) - 28 ;
    eli2_cmd_byte_position = ( (PKT<<7)|eli2_ofst) ;
    __LOG_PARAM_FW(eli2_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl2_cmd_ofst);
    __LOG_PARAM_FW(eli2_cmd_byte_position);

    /* Get offsets same as done for EL1 */
    __LOG_FW(("Get offsets same as done for EL1"));
    eli3_ofst = thr15_in_hdr_ptr->MPLS_data.EL3_ofst;
    cpy_preceding_lbl3_cmd_ofst = (eli3_ofst - 4) - 28 ;
    eli3_cmd_byte_position = ( (PKT<<7)|eli3_ofst) ;
    __LOG_PARAM_FW(eli3_ofst);
    __LOG_PARAM_FW(cpy_preceding_lbl3_cmd_ofst);
    __LOG_PARAM_FW(eli3_cmd_byte_position);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle first EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle first EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    /* Note: To save cycles I prepared predefined commands of 8 options. All options move the labels to the same destination
             but each option with different size from 4B (1 label) till 32B (all 8 labels).  Minimum offset to add the new labels
             after first label and maximum should be after origin label 8 */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET + (eli1_ofst-48), 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl1_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl1_cmd_ofst, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli1_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli1_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1  Label value[18:0] = Desc<Hash[11:0]"));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli1_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli1_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle second EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET + (eli2_ofst-40), 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl2_cmd_ofst, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli2_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli2_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1 Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1 Label value[18:0] = Desc<Hash[11:0] "));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli2_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli2_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Handle third EL label
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Handle second EL label"));

    /* Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels */
    __LOG_FW(("Move 8B left size of 4,8,12,16,20,24,28 or 32B (pending offset) to make room for EL & ELI labels"));
    ACCEL_CMD_TRIG(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET + (eli3_ofst-32), 0);
    __PRAGMA_NO_REORDER

    /*------------------------------------------------------------------------------------------------------------------
      -  Update ELI label. Since TC and TTL values should be taken from preceding label
      -  I first copy the whole 4B of preceding label then change the other fields as required (label value=7,BoS=0)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update ELI label: TC & TTL from preceding label,label value=7,BoS=0"));

    /* Copy 4B of preceding label into ELI label. cpy_preceding_lbl2_cmd_ofst points to the compatible command */
    __LOG_FW(("Copy 4B of preceding label into ELI label"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET + cpy_preceding_lbl3_cmd_ofst, 0);
    __PRAGMA_NO_REORDER

    /* Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)*/
    __LOG_FW(("Set ELI label value to 7 (since label value is 20 bits long and set constant cmd can supports no more than 11 bits I need to split it to 2 cmds)"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, eli3_cmd_byte_position );
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, (eli3_cmd_byte_position+1) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET, 0);

    /* Clear BoS bit to zero */
    __LOG_FW(("Clear BoS bit to zero "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, (eli3_cmd_byte_position+2) );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update EL label:  TC, TTL, BoS =0 , Label value[19]=1   Label value[18:0] = Desc<Hash[11:0]  "));

    /* Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well */
    __LOG_FW(("Label value[19]=1 + clear all bits within 32bits window to zero therefore clear TC, TTL and BoS fields as well"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, eli3_cmd_byte_position+4 );
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Label value[18:0] = Desc<Hash[11:0]> Zero padding */
    __LOG_FW(("Label value[18:0] = Desc<Hash[11:0]> Zero padding"));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, (eli3_cmd_byte_position+1) + 4);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------
      -  Update last label (BoS=1)
      ------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update last label (BoS=1) "));

    /* Set <BoS> in last label (adjacent to passenger) */
    __LOG_FW(("Set <BoS> in last label (adjacent to passenger)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET, 0);

    /* Return this accelerator command to its origin value so it can be used also in SGT thread */
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET, COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(15);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR16_SGT_NetAddMSB
 * inparam      None
 * return       None
 * description  Handles Network Port Add MSB SGT (Security Group Tag) Thread
 *              cfg template:    ethertype, version, length (4B)  6MSB of SGT tag 0x89090101
 *                               len, option_type, reserved_0 (4B)                0x0001rrrr
 *                               reserved_1 (4B):
 *                               reserved_2 (4B):
 *              Incoming packet: Expansion space(32B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B),SGT tag LSB(4B), Payload
 *              Output packet:   Expansion space(28B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B),SGT tag(8B), Payload
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              PHA fw actions:
 *              - Calculates Timestamp size and VLANs size in order to know where SGT tag should be placed
 *              - Adds 4 bytes
 *              - Copies Template[47:0] to the 6 MSBs bytes of the SGT tag
 *              - Update Desc<fw_bc_modifications> by 4 to indicate addition of 4 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR16_SGT_NetAddMSB)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint8_t vlans_size,timestamp_size,pkt_size_to_move ;
    uint8_t egress_tag0_exists,egress_inner_special_tag,egress_timestamp_tagged_0,egress_timestamp_tagged_1;
    uint8_t out_sgt_tag_ofst, out_sgt_tag_position ;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr16_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get PHA metadata<egress_tag0_exists> and PHA metadata<egress_inner_special_tag>
      - Get Desc<egress_timestamp_tagged_0> and Desc<egress_timestamp_tagged_1>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_PARAM_FW(thr16_desc_ptr->phal2ppa.pha_metadata0);


    /* Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2 */
    __LOG_FW(("Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2"));
    egress_tag0_exists = (thr16_desc_ptr->phal2ppa.pha_metadata0>>2) & 0x1 ;
    __LOG_PARAM_FW(egress_tag0_exists);

    /* Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3) */
    __LOG_FW(("Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3)"));
    egress_inner_special_tag = (thr16_desc_ptr->phal2ppa.pha_metadata0>>4) & 0x7 ;
    __LOG_PARAM_FW(egress_inner_special_tag);

    /* Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long) */
    __LOG_FW(("Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long)"));
    egress_timestamp_tagged_0 = thr16_desc_ptr->phal2ppa.egress_timestamp_tagged & 0x1;
    egress_timestamp_tagged_1 = (thr16_desc_ptr->phal2ppa.egress_timestamp_tagged >> 1) & 0x1;
    __LOG_PARAM_FW(thr16_desc_ptr->phal2ppa.egress_timestamp_tagged);
    __LOG_PARAM_FW(egress_timestamp_tagged_0);
    __LOG_PARAM_FW(egress_timestamp_tagged_1);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate timestamp size
      - Calculate VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Calculate timestamp size (applicable values:0,8 or 16)
       a= Desc<Egress Timestamp Tagged>[0]
       timestamp_size = (Desc<Egress Timestamp Tagged>[1]&~a)<<4+a<<3 */
    __LOG_FW(("Calculate timestamp size (applicable values:0,8,16 or 24)"));
    timestamp_size = ((egress_timestamp_tagged_1 & ((~egress_timestamp_tagged_0) & 0x1))<<4) + (egress_timestamp_tagged_0<<3) ;
    __LOG_PARAM_FW(timestamp_size);

    /* Calculate VLANs size (applicable values:0,4,8,12 or 16)
       VLANs_size= (Metadata<Egress Tag0 Exists> + Metadata<Egress Inner Special tag>)<<2 */
    __LOG_FW(("Calculate VLANs size (applicable values:0,4,8,12 or 16)"));
    vlans_size = (egress_tag0_exists + egress_inner_special_tag)<<2;
    __LOG_PARAM_FW(vlans_size);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate SGT tag location within output packet according to timestamp size and VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* MAC size(12B) + Timestamp size + VLANs size */
    __LOG_FW(("MAC size(12B) + Timestamp size + VLANs size"));
    pkt_size_to_move = 12 + timestamp_size + vlans_size ;
    /* Expansion space size(32B) - 4B (that will be added) = 28B + MAC size + Timestamp size + VLANs size */
    __LOG_FW(("Expansion space size(32B) - 4B (that will be added) = 28B + MAC size + Timestamp size + VLANs size"));
    out_sgt_tag_ofst = 28 + pkt_size_to_move;
    out_sgt_tag_position = ( (PKT<<7) | out_sgt_tag_ofst );
    __LOG_PARAM_FW(pkt_size_to_move);
    __LOG_PARAM_FW(out_sgt_tag_ofst);
    __LOG_PARAM_FW(out_sgt_tag_position);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move left the packet by 4 bytes to make room for SGT tag
      - Maximum size to move in accelerator command is 32B and since packet size to move left here can be up to 48B might need to do it in 2 commands
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move left 4B packet size of 12B+timestamp_size(0,8,16B)+VLANs_size(0,4,8,12,16B)  */
    __LOG_FW(("Move left 4B packet size of 12B+timestamp_size(0,8,16B)+VLANs_size(0,4,8,12,16B) "));
    if (pkt_size_to_move <= 32)
    {   /* Packet size to move is less than 32B, move the packet in one command */
        __LOG_FW(("Packet size to move is less than 32B, move the packet in one command "));
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_OFFSET, (pkt_size_to_move-1));
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }
    else
    {   /* Packet size to move is more than 32B, move the packet in two commands */
        __LOG_FW(("Packet size to move is more than 32B, move the packet in two commands"));
        ACCEL_CMD_LOAD_LENGTH(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, ((pkt_size_to_move-32)-1) );
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN32_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set 6MSB of SGT tag in packet
      - Set Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy Template[47:0] to the 6 MSBs bytes of the SGT tag */
    __LOG_FW(("Copy Template[47:0] to the 6 MSBs bytes of the SGT tag "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR16_SGT_NetAddMSB_CFG_sgt_tag_template__ethertype_LEN6_TO_PKT_sgt_tag__ethertype_OFFSET, out_sgt_tag_position);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BYTES_THR16_SGT_NetAddMSB_CFG_sgt_tag_template__ethertype_LEN6_TO_PKT_sgt_tag__ethertype_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (two's complement). Set Desc<fw_bc_modification> to 4B */
    __LOG_FW(("Indicates HW about the new starting point of the packet header"));
    ACCEL_CMD_TRIG(COPY_BITS_THR16_SGT_NetAddMSB_CONST_0x4_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(16);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR17_SGT_NetFix
 * inparam      None
 * return       None
 * description  Handles Network Port Fix SGT (Security Group Tag) Thread
 *              cfg template:    ethertype, version, length (4B)  6MSB of SGT tag 0x89090101
 *                               len, option_type, reserved_0 (4B)                0x0001rrrr
 *                               reserved_1 (4B):
 *                               reserved_2 (4B):
 *              Incoming packet: Expansion space(32B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B),SGT tag(8B), Payload
 *              Output packet:   Expansion space(32B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B),SGT tag(8B), Payload
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              PHA fw actions:
 *              - Calculates Timestamp size and VLANs size in order to know where SGT tag should be placed
 *              - Copies Template[47:16] to the 4 MSBs bytes of the SGT tag
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR17_SGT_NetFix)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint8_t vlans_size,timestamp_size,pkt_size_to_move ;
    uint8_t egress_tag0_exists,egress_inner_special_tag,egress_timestamp_tagged_0,egress_timestamp_tagged_1;
    uint8_t out_sgt_tag_ofst, out_sgt_tag_position ;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr17_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get PHA metadata<egress_tag0_exists> and PHA metadata<egress_inner_special_tag>
      - Get Desc<egress_timestamp_tagged_0> and Desc<egress_timestamp_tagged_1>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_PARAM_FW(thr17_desc_ptr->phal2ppa.pha_metadata0);

    /* Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2 */
    __LOG_FW(("Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2"));
    egress_tag0_exists = (thr17_desc_ptr->phal2ppa.pha_metadata0>>2) & 0x1 ;
    __LOG_PARAM_FW(egress_tag0_exists);

    /* Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3) */
    __LOG_FW(("Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3)"));
    egress_inner_special_tag = (thr17_desc_ptr->phal2ppa.pha_metadata0>>4) & 0x7 ;
    __LOG_PARAM_FW(egress_inner_special_tag);

    /* Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long) */
    __LOG_FW(("Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long)"));
    egress_timestamp_tagged_0 = thr17_desc_ptr->phal2ppa.egress_timestamp_tagged & 0x1;
    egress_timestamp_tagged_1 = (thr17_desc_ptr->phal2ppa.egress_timestamp_tagged >> 1) & 0x1;
    __LOG_PARAM_FW(thr17_desc_ptr->phal2ppa.egress_timestamp_tagged);
    __LOG_PARAM_FW(egress_timestamp_tagged_0);
    __LOG_PARAM_FW(egress_timestamp_tagged_1);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate timestamp size
      - Calculate VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Calculate timestamp size (applicable values:0,8 or 16)
       a= Desc<Egress Timestamp Tagged>[0]
       timestamp_size = (Desc<Egress Timestamp Tagged>[1]&~a)<<4+a<<3 */
    __LOG_FW(("Calculate timestamp size (applicable values:0,8 or 16)"));
    timestamp_size = ((egress_timestamp_tagged_1 & ((~egress_timestamp_tagged_0) & 0x1))<<4) + (egress_timestamp_tagged_0<<3) ;
    __LOG_PARAM_FW(timestamp_size);

    /* Calculate VLANs size (applicable values:0,4,8,12 or 16)
       VLANs_size= (Metadata<Egress Tag0 Exists> + Metadata<Egress Inner Special tag>)<<2 */
    __LOG_FW(("Calculate VLANs size (applicable values:0,4,8,12 or 16)"));
    vlans_size = (egress_tag0_exists + egress_inner_special_tag)<<2;
    __LOG_PARAM_FW(vlans_size);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate SGT tag location within output packet according to timestamp size and VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* MAC size(12B) + Timestamp size + VLANs size */
    __LOG_FW(("MAC size(12B) + Timestamp size + VLANs size"));
    pkt_size_to_move = 12 + timestamp_size + vlans_size ;
    /* Expansion space size(32B) + MAC size + Timestamp size + VLANs size */
    __LOG_FW(("Expansion space size(32B) + MAC size + Timestamp size + VLANs size"));
    out_sgt_tag_ofst = 32 + pkt_size_to_move;
    out_sgt_tag_position = ( (PKT<<7) | out_sgt_tag_ofst );
    __LOG_PARAM_FW(pkt_size_to_move);
    __LOG_PARAM_FW(out_sgt_tag_ofst);
    __LOG_PARAM_FW(out_sgt_tag_position);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set 4MSB of SGT tag in packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Copy Template[47:16] to the 4 MSBs bytes of the SGT tag */
    __LOG_FW(("Copy Template[47:16] to the 4 MSBs bytes of the SGT tag "));
    ACCEL_CMD_LOAD_PKT_DEST(COPY_BYTES_THR17_SGT_NetFix_CFG_sgt_tag_template__ethertype_LEN4_TO_PKT_sgt_tag__ethertype_OFFSET, out_sgt_tag_position);
    __PRAGMA_NO_REORDER
    ACCEL_CMD_TRIG(COPY_BYTES_THR17_SGT_NetFix_CFG_sgt_tag_template__ethertype_LEN4_TO_PKT_sgt_tag__ethertype_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(17);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR18_SGT_NetRemove
 * inparam      None
 * return       None
 * description  Handles Network Port Remove SGT (Security Group Tag) Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B),SGT tag(4B), Payload
 *              Output packet:   Expansion space(36B),MAC DA SA(12B),Timestamp(0,8,16B),VLANs(0,4,8,12,16B), Payload
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              PHA fw actions:
 *              - Calculates Timestamp size and VLANs size in order to know where SGT tag should be placed
 *              - Remove 4 bytes of the SGT tag
 *              - Updates Desc<fw_bc_modification> to -4 to indicate removal of 4 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR18_SGT_NetRemove)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint8_t vlans_size,timestamp_size,pkt_size_to_move ;
    uint8_t egress_tag0_exists,egress_inner_special_tag,egress_timestamp_tagged_0,egress_timestamp_tagged_1;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr18_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get PHA metadata<egress_tag0_exists> and PHA metadata<egress_inner_special_tag>
      - Get Desc<egress_timestamp_tagged_0> and Desc<egress_timestamp_tagged_1>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_PARAM_FW(thr18_desc_ptr->phal2ppa.pha_metadata0);

    /* Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2 */
    __LOG_FW(("Extract egress_tag0_exists -> placed in PHA metadata Byte 0 bit 2"));
    egress_tag0_exists = (thr18_desc_ptr->phal2ppa.pha_metadata0>>2) & 0x1 ;
    __LOG_PARAM_FW(egress_tag0_exists);

    /* Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3) */
    __LOG_FW(("Extract egress_inner_special_tag -> placed in PHA metadata Byte 0 bits 4:6 (supported values 0-3)"));
    egress_inner_special_tag = (thr18_desc_ptr->phal2ppa.pha_metadata0>>4) & 0x7 ;
    __LOG_PARAM_FW(egress_inner_special_tag);

    /* Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long) */
    __LOG_FW(("Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long)"));
    egress_timestamp_tagged_0 = thr18_desc_ptr->phal2ppa.egress_timestamp_tagged & 0x1;
    egress_timestamp_tagged_1 = (thr18_desc_ptr->phal2ppa.egress_timestamp_tagged >> 1) & 0x1;
    __LOG_PARAM_FW(thr18_desc_ptr->phal2ppa.egress_timestamp_tagged);
    __LOG_PARAM_FW(egress_timestamp_tagged_0);
    __LOG_PARAM_FW(egress_timestamp_tagged_1);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate timestamp size
      - Calculate VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Calculate timestamp size (applicable values:0,8,16)
       a= Desc<Egress Timestamp Tagged>[0]
       timestamp_size = (Desc<Egress Timestamp Tagged>[1]&~a)<<4+a<<3 */
    __LOG_FW(("Calculate timestamp size (applicable values:0,8,16)"));
    timestamp_size = ((egress_timestamp_tagged_1 & ((~egress_timestamp_tagged_0) & 0x1))<<4) + (egress_timestamp_tagged_0<<3) ;
    __LOG_PARAM_FW(timestamp_size);

    /* Calculate VLANs size (applicable values:0,4,8,12 or 16)
       VLANs_size= (Metadata<Egress Tag0 Exists> + Metadata<Egress Inner Special tag>)<<2 */
    __LOG_FW(("Calculate VLANs size (applicable values:0,4,8,12 or 16)"));
    vlans_size = (egress_tag0_exists + egress_inner_special_tag)<<2;
    __LOG_PARAM_FW(vlans_size);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate SGT tag location within output packet according to timestamp size and VLANs size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* MAC size(12B) + Timestamp size + VLANs size */
    __LOG_FW(("MAC size(12B) + Timestamp size + VLANs size"));
    pkt_size_to_move = 12 + timestamp_size + vlans_size ;
    __LOG_PARAM_FW(pkt_size_to_move);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move right the packet by 4 bytes to remove SGT tag
      - Maximum size to move in accelerator command is 32B and since packet size to move right here can be up to 52B
        might need to do it in 2 accelerator commands
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Move right 4B packet size of 12B+timestamp_size(0,8,16B)+VLANs_size(0,4,8,12,16B)  */
    __LOG_FW(("Move right 4B packet size of 12B+timestamp_size(0,8,16B)+VLANs_size(0,4,8,12,16B) "));
    if (pkt_size_to_move <= 32)
    {   /* Packet size to move is less than 32B, move the packet in one command */
        __LOG_FW(("Packet size to move is less than 32B, move the packet in one command "));
        ACCEL_CMD_LOAD_LENGTH(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_OFFSET, (pkt_size_to_move-1));
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }
    else
    {   /* Packet size to move is more than 32B, move the packet in two commands */
        __LOG_FW(("Packet size to move is more than 32B, move the packet in two commands"));
        ACCEL_CMD_LOAD_LENGTH(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, ((pkt_size_to_move-32)-1) );
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). Set Desc<fw_bc_modification> to -4B (0xFC) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header"));
    ACCEL_CMD_TRIG(COPY_BITS_THR18_SGT_NetRemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,0);

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(18);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR19_SGT_eDSAFix
 * inparam      None
 * return       None
 * description  Handles eDSA Fix Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B),MAC DA SA(12B),eDSA(16B), Timestamp(0,8,16B),VLAN(4B),SGT LSB tag(4B), Payload
 *              Output packet:   Expansion space(32B),MAC DA SA(12B),eDSA(16B), Timestamp(0,8,16B),VLAN(4B),SGT LSB tag(4B), Payload
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              PHA fw actions:
 *              - Set eDSA<Tag1 Src Tagged>=0
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR19_SGT_eDSAFix)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    /* Set eDSA<Tag1 Src Tagged>=0 */
    __LOG_FW(("Set eDSA<Tag1 Src Tagged>=0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR19_SGT_eDSAFix_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w3__Tag1SrcTagged_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(19);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR20_SGT_eDSARemove
 * inparam      None
 * return       None
 * description  Handles eDSA remove Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B),MAC DA SA(12B),eDSA(16B), Timestamp(0,8,16B),VLAN1(4B),SGT LSB tag(4B), Payload
 *              Output packet:   Expansion space(36B),MAC DA SA(12B),eDSA(16B), Timestamp(0,8,16B),VLAN1(4B), Payload
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              PHA fw actions:
 *              - Remove SGT LSB tag (4B)
 *              - Updates Desc<fw_bc_modification> to -4 to indicate removal of 4 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR20_SGT_eDSARemove)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    uint8_t timestamp_size ;
    uint8_t egress_timestamp_tagged_0,egress_timestamp_tagged_1;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr20_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Get Desc<egress_timestamp_tagged_0> and Desc<egress_timestamp_tagged_1>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long) */
    __LOG_FW(("Extract egress_timestamp_tagged[0] and egress_timestamp_tagged[1] from Desc<egress_timestamp_tagged> (3 bits long)"));
    egress_timestamp_tagged_0 = thr20_desc_ptr->phal2ppa.egress_timestamp_tagged & 0x1;
    egress_timestamp_tagged_1 = (thr20_desc_ptr->phal2ppa.egress_timestamp_tagged >> 1) & 0x1;
    __LOG_PARAM_FW(egress_timestamp_tagged_0);
    __LOG_PARAM_FW(egress_timestamp_tagged_1);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate timestamp size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Calculate timestamp size (applicable values:0,8 or 16)
       a= Desc<Egress Timestamp Tagged>[0]
       timestamp_size = (Desc<Egress Timestamp Tagged>[1]&~a)<<4+a<<3 */
    __LOG_FW(("Calculate timestamp size (applicable values:0,8,16 or 24)"));
    timestamp_size = ((egress_timestamp_tagged_1 & ((~egress_timestamp_tagged_0) & 0x1))<<4) + (egress_timestamp_tagged_0<<3) ;
    __LOG_PARAM_FW(timestamp_size);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Move right the packet by 4 bytes to remove SGT tag
      - Need to do it with 2 commands if packet size is bigger than 32 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    if (timestamp_size == 0)
    {   /* There is no Timestamp therefore packet size is 32 bytes */
        __LOG_FW(("There is no Timestamp therefore packet size is 32 bytes"));

        /* Move right the packet by 4 bytes to remove SGT tag */
        __LOG_FW(("Move right the packet by 4 bytes to remove SGT tag"));
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }
    else
    {   /* Timestamp exists therefore packet size is bigger than 32 bytes which means we need to shift the packet by 2 accelerator commands */
        __LOG_FW(("Timestamp exists therefore packet size is bigger than 32 bytes which means we need to shift the packet by 2 accelerator commands"));

        ACCEL_CMD_LOAD_LENGTH(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN4_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, (timestamp_size-1) );
        __PRAGMA_NO_REORDER
        /* First move right the last bytes (Timestamp bytes) */
        __LOG_FW(("First move right the last bytes (Timestamp bytes)"));
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN4_FROM_PKT_mac_da_47_32_PLUS32_OFFSET, 0);
        __PRAGMA_NO_REORDER
        /* Then move right the first 32 bytes */
        __LOG_FW(("Then move right the first 32 bytes"));
        ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET, 0);
        __PRAGMA_NO_REORDER
    }


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). Set Desc<fw_bc_modification> to -4B (0xFC) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header"));
    ACCEL_CMD_TRIG(COPY_BITS_THR20_SGT_eDSARemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(20);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR21_SGT_GBPFixIPv4
 * inparam      None
 * return       None
 * description  Handles VXLAN GBP Fix IPv4 Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B), IPv4(20B), UDP(8B), VXLAN(8B), MAC DA SA(12B), Payload
 *              Output packet:   Expansion space(32B), IPv4(20B), UDP(8B), VXLAN(8B), MAC DA SA(12B), Payload
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              Desc<PHA metadata>:  -----------------------------------------------------------------------------------------------
 *                                  |     Byte 5    |     Byte 4    |     Byte 3    |     Byte 2    |     Byte 1    |     Byte 0    |
 *                                   -----------------------------------------------------------------------------------------------
 *                                  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *                                   -----------------------------------------------------------------------------------------------
 *                                                                  |v v v v        |        v v v v|v v v v v v v v|  v v v   v    |
 *                                                                     Desc.Tag1up                Desc.Tag1vid                 Desc.EgressTag0Exists
 *                                                                   Des.Tag1cfi                                       Desc.EgressInnerSpecialTag
 *
 *              PHA fw actions:
 *              - Set <G> bit in the VXLAN header
 *              - Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1>  to <Group Policy ID> in the VXLAN header
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR21_SGT_GBPFixIPv4)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /* Set <G> bit in the VXLAN header */
    __LOG_FW(("Set <G> bit in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR21_SGT_GBPFixIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,0);

    /* Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header */
    __LOG_FW(("Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,0);
    ACCEL_CMD_TRIG(COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(21);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR22_SGT_GBPFixIPv6
 * inparam      None
 * return       None
 * description  Handles VXLAN GBP Fix IPv6 Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B), IPv6(40B), UDP(8B), VXLAN(8B), MAC DA SA(12B), Payload
 *              Output packet:   Expansion space(32B), IPv6(40B), UDP(8B), VXLAN(8B), MAC DA SA(12B), Payload
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              Desc<PHA metadata>:  -----------------------------------------------------------------------------------------------
 *                                  |     Byte 5    |     Byte 4    |     Byte 3    |     Byte 2    |     Byte 1    |     Byte 0    |
 *                                   -----------------------------------------------------------------------------------------------
 *                                  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *                                   -----------------------------------------------------------------------------------------------
 *                                                                  |v v v v        |        v v v v|v v v v v v v v|  v v v   v    |
 *                                                                     Desc.Tag1up                Desc.Tag1vid                 Desc.EgressTag0Exists
 *                                                                   Des.Tag1cfi                                       Desc.EgressInnerSpecialTag
 *
 *              PHA fw actions:
 *              - Set <G> bit in the VXLAN header
 *              - Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR22_SGT_GBPFixIPv6)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /* Set <G> bit in the VXLAN header */
    __LOG_FW(("Set <G> bit in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR22_SGT_GBPFixIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,0);

    /* Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header */
    __LOG_FW(("Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,0);
    ACCEL_CMD_TRIG(COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(22);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR23_SGT_GBPRemoveIPv4
 * inparam      None
 * return       None
 * description  Handles VXLAN-GBP Remove SGT (Security Group Tag) IPv4 Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B), IPv4(20B), UDP(8B), VXLAN(8B), MAC DA SA(12B), VLAN (4B optional), SGT LSB Tag(4B), Payload
 *              Output packet:   Expansion space(36B), IPv4(20B), UDP(8B), VXLAN(8B), MAC DA SA(12B), VLAN (4B optional), Payload
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              Desc<PHA metadata>:  -----------------------------------------------------------------------------------------------
 *                                  |     Byte 5    |     Byte 4    |     Byte 3    |     Byte 2    |     Byte 1    |     Byte 0    |
 *                                   -----------------------------------------------------------------------------------------------
 *                                  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *                                   -----------------------------------------------------------------------------------------------
 *                                                                  |v v v v        |        v v v v|v v v v v v v v|  v v v   v    |
 *                                                                     Desc.Tag1up                Desc.Tag1vid                 Desc.EgressTag0Exists
 *                                                                   Des.Tag1cfi                                       Desc.EgressInnerSpecialTag
 *              PHA fw actions:
 *              - Set <G> bit in the VXLAN header
 *              - Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header
 *              - If Metadata<Egress tag state>=tag0 then remove 4 bytes after the passenger VLAN
 *                Else remove 4 bytes after the passenger MAC SA
 *              - Update IP length
 *              - Update IPv4 checksum
 *              - Update UDP length
 *              - Update Desc<fw_bc_modification> by -4 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR23_SGT_GBPRemoveIPv4)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint32_t    vlan_tag_offset;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr23_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Check if packet includes VLAN tag
      - Remove 4 bytes of SGT LSB TAG by right shift taking into consideration existence or nonexistence of VLAN TAG
        NOTE: Since accelerator command can shift maximum 32B and size of packet till SGT tag is more than 32B need to do it using 2 accelerators
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get Desc<pha metadata>[Egress Tag0 Exists] field. 1 bit size placed in PHA metadata[2] (byte 0 bit 2)
       Calculate vlan tag offset: 0 if there is no vlan, 4 if vlan exists */
    __LOG_FW(("Get Desc<pha metadata>[Egress Tag0 Exists] field and calculates vlan offset"));
    vlan_tag_offset = thr23_desc_ptr->phal2ppa.pha_metadata0 & 0x4 ;
    __LOG_PARAM_FW(thr23_desc_ptr->phal2ppa.pha_metadata0);
    __LOG_PARAM_FW(vlan_tag_offset);

    /* First move right the last 16 bytes(no vlan) or 20 bytes(with vlan)
       SHIFTRIGHT LEN16 & LEN20 commands are placed in memory one after the other and are triggered pending vlan_tag_offset  */
    __LOG_FW(("First move right the last 16(no vlan) or 20(with vlan) bytes"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN16_FROM_PKT_version_PLUS32_OFFSET + vlan_tag_offset, 0);
    __PRAGMA_NO_REORDER

    /* Then move right the first 32 bytes */
    __LOG_FW(("Then move right the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN32_FROM_PKT_version_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set VXLAN header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set <G> bit in the VXLAN header */
    __LOG_FW(("Set <G> bit in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,0);

    /* Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header */
    __LOG_FW(("Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,0);
    ACCEL_CMD_TRIG(COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET,0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field:                         IPv4 Header<Total Length>= IPv4 Header<Total Length> - 4
      - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Sum old IP Total Length field */
    __LOG_FW(("Sum old IP Total Length using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Update new IP header total length field IPv4<Total Length>. Decrement it by 4.*/
    __LOG_FW(("Update new outer IP header total length field IPv4<Total Length. Decrement it by 4 using accelerator cmd"));
    ACCEL_CMD_TRIG(SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Sum new IP Total Length */
    __LOG_FW(("Sum new IP Total Length using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);

    /* Sum current/old IPv4 CS  */
    __LOG_FW(("Sum current/old IPv4 CS using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR23_SGT_GBPRemoveIPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update UDP header length field with -4 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Update new UDP length. Need to sub 4 bytes. */
    __LOG_FW(("Update new UDP length. Need to sub 4 bytes using accelerator cmd"));
    ACCEL_CMD_TRIG(SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). Set Desc<fw_bc_modification> to -4B (0xFC) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header. Set Desc<fw_bc_modification> to -4B (0xFC)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(23);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR24_SGT_GBPRemoveIPv6
 * inparam      None
 * return       None
 * description  Handles VXLAN-GBP Remove SGT (Security Group Tag) IPv6 Thread
 *              cfg template:    reserved_0 (4B): No usage
 *                               reserved_1 (4B): No usage
 *                               reserved_2 (4B): No usage
 *                               reserved_3 (4B): No usage
 *              Incoming packet: Expansion space(32B), IPv6(40B), UDP(8B), VXLAN(8B), MAC DA SA(12B), VLAN (4B optional), SGT LSB Tag(4B), Payload
 *              Output packet:   Expansion space(36B), IPv6(40B), UDP(8B), VXLAN(8B), MAC DA SA(12B), VLAN (4B optional), Payload
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *              Desc<PHA metadata>:  -----------------------------------------------------------------------------------------------
 *                                  |     Byte 5    |     Byte 4    |     Byte 3    |     Byte 2    |     Byte 1    |     Byte 0    |
 *                                   -----------------------------------------------------------------------------------------------
 *                                  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *                                   -----------------------------------------------------------------------------------------------
 *                                                                  |v v v v        |        v v v v|v v v v v v v v|  v v v   v    |
 *                                                                     Desc.Tag1up                Desc.Tag1vid                 Desc.EgressTag0Exists
 *                                                                   Des.Tag1cfi                                       Desc.EgressInnerSpecialTag
 *              PHA fw actions:
 *              - Set <G> bit in the VXLAN header
 *              - Copy Metadata<VID1>, Metadata<UP1> and Metadata<CFI1> to <Group Policy ID> in the VXLAN header
 *              - If Metadata<Egress tag state>=tag0 then remove 4 bytes after the passenger VLAN
 *                Else remove 4 bytes after the passenger MAC SA
 *              - Update IP length
 *              - Update UDP length
 *              - Update Desc<fw_bc_modification> by -4 bytes
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR24_SGT_GBPRemoveIPv6)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    uint32_t    vlan_tag_offset;

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr24_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Check if packet includes VLAN tag
      - Remove 4 bytes of SGT LSB TAG by right shift taking into consideration existence or nonexistence of VLAN TAG
        NOTE: Since accelerator command can shift maximum 32B and size of packet till SGT tag is more than 32B need to do it using 3 accelerators
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get Desc<pha metadata>[Egress Tag0 Exists] field. 1 bit size placed in PHA metadata[2] (byte 0 bit 2)
       Calculate vlan tag offset: 0 if there is no vlan, 4 if vlan exists */
    __LOG_FW(("Get Desc<pha metadata>[Egress Tag0 Exists] field and calculates vlan offset"));
    vlan_tag_offset = thr24_desc_ptr->phal2ppa.pha_metadata0 & 0x4 ;
    __LOG_PARAM_FW(vlan_tag_offset);

    /* First move right the last 4 bytes(no vlan) or 8 bytes(with vlan)
       SHIFTRIGHT LEN4 & LEN8 commands are placed in memory one after the other and are triggered pending vlan_tag_offset  */
    __LOG_FW(("First move right the last 4(no vlan) or 8(with vlan) bytes"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN4_FROM_PKT_version_PLUS64_OFFSET + vlan_tag_offset, 0);
    __PRAGMA_NO_REORDER

    /* Then move right the second 32 bytes */
    __LOG_FW(("Then move right the second 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_PLUS32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Then move right the first 32 bytes */
    __LOG_FW(("Then move right the first 32 bytes"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set VXLAN header
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set <G> bit in the VXLAN header */
    __LOG_FW(("Set <G> bit in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,0);

    /* Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header */
    __LOG_FW(("Copy Metadata<UP1>, Metadata<CFI1> and Metadata<VID1> to <Group Policy ID> in the VXLAN header using accelerator command"));
    ACCEL_CMD_TRIG(COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,0);
    ACCEL_CMD_TRIG(COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET,0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field with -4 bytes
      - Update UDP header length field with -4 bytes
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Update new IP header payload length field IPv6<Payload Length>. Decrement it by 4 */
    __LOG_FW(("Update new IP header payload length field IPv6<Payload Length>. Decrement it by 4 using accelerator cmd"));
    ACCEL_CMD_TRIG(SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Update new UDP length. Need to sub 4 bytes. */
    __LOG_FW(("Update new UDP length. Need to sub 4 bytes using accelerator cmd"));
    ACCEL_CMD_TRIG(SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Desc<fw_bc_modification>
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* Indicates HW about the new starting point of the packet header (two's complement). Set Desc<fw_bc_modification> to -4B (0xFC) */
    __LOG_FW(("Indicates HW about the new starting point of the packet header. Set Desc<fw_bc_modification> to -4B (0xFC)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(24);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR25_PTP_Phy_1_Step
 * inparam      None
 * return       None
 * description  This thread is part of the support for OC/BC and TC 1 step timestamping for PTPv2 in Marvell Switch-PHY system
 *              cfg template:    ptp_phy_tag (4B): fw build this tag and hold it here bits[10:0]
 *                               reserved_1  (4B): No usage
 *                               reserved_2  (4B): No usage
 *                               reserved_3  (4B): No usage
 *              Incoming packet: NA since all modifications are done in descriptor domain
 *              Output packet:   NA since all modifications are done in descriptor domain
 *              Header Window anchor= NA since all modifications are done in descriptor domain
 *              Header Window size= NA since all modifications are done in descriptor domain
 *              Desc<PHA metadata>: No usage
 *              PHA fw actions:
 *              - Build PTP PHY Tag:  [ CF_Offset | TAI_Sel | ChecksumUpdate | IngTimeSecLsb ]
 *              - Place PTP PHY Tag as follows: {desc<PTPQueueSelect>,desc<PTPQueueEntryId[9:0]>} = PTP_PHY_TAG[10:0]
 *              - Set PTP_ACTION to CAPTURE_PCH
 *
 *              Description:
 *              Industry is mainly geared towards 1 step PTP. Marvell LinkCrypt PHYs implementation includes 1 step PTP but is relatively heavy
 *              as it requires header parsing (and doesn’t support any encapsulation). The goal is to define a low cost PHY solution that works
 *              well with a Marvell Switch to allow for 1 step PTP in a low cost USX PHY. To support low cost 1 step PTP timestamping in the
 *              egress PHY, the switch should pass to the PHY all the required information on where/how to timestamp the packet.
 *
 *              Egress packet flow: The switch receives the packet and processes it as follows
 *              1. Classify the packet as a PTP packet over L2 or L3/UDP and redirect it to the target port
 *              2. The target USX MAC adds a MCH and embeds PTP PHY tag (on the same bits as in the PCH<Signature> field) as previously configured
 *                 by the application in the register (USX MAC is configured in advance to Register<USXGMII_PCH signature mode>=CONFIG_CTR)
 *              3. Send the packet to the (egress) PHY
 *              Note: The switch PTP engines apply no changes on this packet. This is done by assigning a dedicated PTP_ACTION of CapturePch
 *
 *              FW solution: Since the egress time is added to the CF by the PHY, the CTSU should not apply it by itself.
 *              There’re 2 ways to achieve this - HW change or applying FW (PHA). In order to avoid HW modifications, the following is required
 *              to disable the CTSU packet processing and generate the required PTP PHY tag:
 *              1. The PHA dedicated PTP_PHY_1_STEP thread is triggered. This is done by assigning the thread id from one of the below options:
 *                 a. EPCL: Key is based on metadata of {is_ptp, trg_phy_port}. Action is assigning the thread ID.
 *                 b. eport: assign thread ID per trg_eport (HA).
 *              2. PHA PTP_PHY_1_STEP thread will override the following desc' fields:
 *                 a. PTP_ACTION = CAPTURE: this will prevent the CTSU from further modifications on the frame
 *                 b. {PTPQueueSelect [0],PTPQueueEntryId[9:0]} = PTP_PHY_TAG[10:0]: The PTP_PHY_TAG will be taken directly from the desc fields.
 *                    this will cause the CTSU to pass the PTP_PHY_TAG to the EPI to build the MCH
 *              3. CTSU is configured for MCH enable (USX) – unaware and assigns the ctsu2epi bus from {PTPQueueSelect [0],PTPQueueEntryId[9:0]}
 *              4. EPI is configured to take the signature from the CTSU bus
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR25_PTP_Phy_1_Step)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */


    /*------------------------------------------------------------------------------------------------------------------------------------------
      - Build PTP PHY Tag which is used to pass TS information from the switch to the egress PHY (not needed in ingress)
      - Put it temporarily in CFG<reserved_0[10:0]>

        PTP PHY Tag:  | bits[10:4] | bits[3:2] |    bit[1]      |   bit[ 0]     |
                      ----------------------------------------------------------
                     | CF_Offset  | TAI_Sel   | ChecksumUpdate | IngTimeSecLsb |
                     ----------------------------------------------------------

       IngTimeSecLsb: indicates to the PHY the LSBit of the ingress time that was inserted to the CF by the host/switch.
                      The PHY uses it to check for nanosec WA (wraparound) to account for when updating CF with the egress time
       CS Update:     indicates to the PHY whether UDP incremental checksum update is required upon packet modification (only for IP6/UDP frames)
                      0 - DISABLE: no checksum update is required (packet is L2/IPv4)
                      1 - ENABLE: checksum update is required (packet is IPv6/UDP)
       TAI_Sel:       TAI domain selection indication from the switch to the PHY - support up to 4 domains.
                      In Hawk/Phx will be assigned as {1’b0,desc<PTP TAI Select>}
       CF_Offset:     The PTP header CF field offset, in 2B resolution. field range is 0-254B.
                      This field is relative to the SFD, i.e. byte 0 points to the first byte of the MACDA
      -------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Build PTP PHY Tag which is used to pass TS information from the switch to the egress PHY (not needed in ingress)"));

    /* Clear all bits to zero and set IngTimeSecLsb field according to desc<Ingress Timestamp Seconds[0]> (chop the msbit) */
    __LOG_FW(("Clear all bits to zero and set IngTimeSecLsb field according to desc<Ingress Timestamp Seconds[0]> (chop the msbit)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_PLUS1_LEN1_TO_CFG_ptp_phy_tag__ingTimeSecLsb_OFFSET,SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Set Checksum Update field according to desc<udp_checksum_update_en> */
    __LOG_FW(("Set Checksum Update field according to desc<udp_checksum_update_en>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__udp_checksum_update_en_LEN1_TO_CFG_ptp_phy_tag__checksumUpdate_OFFSET,0);

    /* Set TAI_Sel field as {1’b0,desc<PTP TAI Select>} */
    __LOG_FW(("Set TAI_Sel field as {1’b0,desc<PTP TAI Select>}"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__ptp_tai_select_LEN1_TO_CFG_ptp_phy_tag__tai_sel_PLUS1_OFFSET,0);

    /* Set CF_Offset field according to desc<timestamp_offset[7:1]> (chop the lsbit - field is in 1B resolution in the desc’) */
    __LOG_FW(("Set CF_Offset field according to desc<timestamp_offset[7:1]> (chop the lsbit - field is in 1B resolution in the desc’)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_offset_LEN7_TO_CFG_ptp_phy_tag__cf_offset_OFFSET,0);
    __PRAGMA_NO_REORDER


    /*-------------------------------------------------------------------------------------------------------------------------------------------------
      Move PTP PHY Tag to the final location in descriptor => Set {desc<PTPQueueSelect>,desc<PTPQueueEntryId[9:0]>} = PTP_PHY_TAG[10:0]
      The PTP_PHY_TAG will be taken directly from the desc fields. This will cause the CTSU (Timestamp Unit) to pass the PTP_PHY_TAG
      to the EPI to build the MCH (Marvell Control Header, added as preamble before the packet)
      - desc<PTPQueueSelect>:       Indicates which queue, 0 or 1, should be used by the TSU to store the egress timestamp.
      - desc<PTPQueueEntryId[9:0]>: The TS queue entry ID is stored in the TS queue in the ERMRK (for the PSU) and in the TS queue in the TSU.
                                    The application then reads the 2 entries from ERMRK and from TSU and uses the queue entry ID to match the two
      -------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Move PTP PHY Tag to the final location in descriptor => Set {desc<PTPQueueSelect>,desc<PTPQueueEntryId[9:0]>} = PTP_PHY_TAG[10:0]"));

    /* Set desc<PTPQueueSelect> = PTP_PHY_TAG[10] */
    __LOG_FW(("Set desc<PTPQueueSelect> = PTP_PHY_TAG[10]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_LEN1_TO_DESC_phal2ppa__ptp_timestamp_queue_select_OFFSET,0);

    /* Set desc<PTPQueueEntryId[9:0]> = PTP_PHY_TAG[9:0] */
    __LOG_FW(("Set desc<PTPQueueEntryId[9:0]> = PTP_PHY_TAG[9:0]"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_PLUS1_LEN10_TO_DESC_phal2ppa__ptp_timestamp_queue_entry_id_OFFSET,0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      Set PTP_ACTION to CAPTURE: this will prevent the CTSU from further modifications on the frame
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set PTP_ACTION to CAPTURE: this will prevent the CTSU from further modifications on the frame"));

    /* Set desc<PTP_ACTION> = CAPTURE (0x3) */
    __LOG_FW(("Set desc<PTP_ACTION> = CAPTURE (0x3)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR25_PTP_Phy_1_Step_CONST_0x3_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET,0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(25);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}





/********************************************************************************************************************//**
 * funcname     THR53_SRV6_End_Node_GSID_COC32
 * inparam      None
 * return       None
 * description  This thread covers G-SID CoC32 end node processing
 *              It modifies the IPv6 DIP with the next G-SID from SRH segment list.
 *              The thread is triggered by ePort configuration.
 *
 *              cfg template: Byte 0 should hold DIP Common Prefix length (in bits units)
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B),Container2(16B)
 *              hdr out: Extension space(32B),IPv6(40B),SRH(8B),Container0(16B),Container1(16B),Container2(16B)
 *
 *              While IPv6 packet with DIP containing Common Prefix(variable length) + G-SID(4B) + SI(2b)
 *
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmware actions:
 *              - Updates SI and SL indexes
 *              - Updates IPv6 DIP with Next G-SID
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR53_SRV6_End_Node_GSID_COC32) () {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    uint8_t sl;                    /* Indicates the required container from SRH container list */
    uint8_t si;                    /* Indicates the next G-SID to be used from the required SRH container */
    uint8_t NextGSID;              /* Holds G-SID value */
    uint32_t commonPrefixLength;   /* Holds Common Prefix Length in bytes */
    uint32_t src;                  /* Holds source offset (next G-SID from SRH container list) */
    uint32_t dest;                 /* Holds destination offset (where to copy next G-SID) */


    /* Get pointer to cfg & packet */
    struct thr53_SRV6_End_Node_GSID_COC32_in_hdr *thr53_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_in_hdr);
    struct thr53_SRV6_End_Node_GSID_COC32_cfg *thr53_cfg_ptr = AC5P_MEM_CAST(AC5P_CFG_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_cfg);
    struct thr53_SRV6_End_Node_GSID_COC32_out_hdr *thr53_out_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_out_hdr);

    /*-------------------------------------------------------------------------------------------------------------------------
      - Get SI & SL parameters
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Get SI[1:0] = IPv6 DIP[1:0] (2 lsbits) */
    __LOG_FW(("Get SI[1:0] = IPv6 DIP[1:0] (2 lsbits)"));
    si = thr53_in_hdr_ptr->IPv6_Header.dip3_low&0x3;
    __LOG_PARAM_FW(si);

    /* Get SL[7:0] = SL field in SRH */
    __LOG_FW(("Get SL[7:0] = SL field in SRH"));
    sl = thr53_in_hdr_ptr->SRv6_Header.segments_left;
    __LOG_PARAM_FW(sl);

    /* Check SI value */
    __LOG_FW(("Check SI value"));
    if (si==0)
    {   /* SI=0 therefore no more G-SIDs in current container */
        __LOG_FW(("SI=0 therefore no more G-SIDs in current container"));

        /* Decrement SL, the next container */
        __LOG_FW(("Decrement SL, the next container"));
        sl--;

        /* Set SI to 3 */
        __LOG_FW(("Set SI to 3"));
        si=3;

        /* Copy updated SL into SRH SL field */
        __LOG_FW(("Copy updated SL into SRH SL field"));
        thr53_out_hdr_ptr->SRv6_Header.segments_left = sl;
    }
    else
    {   /* SI!=0 therefore current container still has G-SIDs */
        __LOG_FW(("SI!=0 therefore current container still has G-SIDs"));

        /* Decrement SI by 1 */
        __LOG_FW(("Decrement SI by 1"));
        si--;
    }


    /*-------------------------------------------------------------------------------------------------------------------------
      - Update IPv6 DIP with Next G-SID
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Current_Container_Offset = extension space(32B) + IPv6(40B) + SRH(8B) + (SL*16) + Current_Container_Offset[SI] -- SI selects the 32b G-SID */
    __LOG_FW(("Current_Container_Offset = extension space(32B) + IPv6(40B) + SRH(8B) + (SL*16) + Current_Container_Offset[SI] -- SI selects the 32b G-SID "));
    src = 80 + sl*16 + si*4;
    __LOG_PARAM_FW(src);

    /* Get common prefix length from template byte 0 (in bits units) */
    __LOG_FW(("Get common prefix length from template byte 0 (in bits units)"));
    commonPrefixLength = thr53_cfg_ptr->SRV6_GSID_COC32_template.commonPrefixLength/8;
    __LOG_PARAM_FW(thr53_cfg_ptr->SRV6_GSID_COC32_template.commonPrefixLength);
    __LOG_PARAM_FW(commonPrefixLength);

    /* Get destination offset for G-SID location: extension space(32B) + IPv6.DIP(24B) + common prefix length (bytesoffset) */
    __LOG_FW(("Get destination offset for G-SID location: extension space(32B) + IPv6.DIP(24B) + common prefix length (bytesoffset)"));
    dest =  56 + commonPrefixLength;
    __LOG_PARAM_FW(dest);


    /* Copy 32b Next_G-SID into IPv6 DIP at offset Common_Prefix_Length */
    __LOG_FW(("Copy 32b Next_G-SID into IPv6 DIP at offset Common_Prefix_Length"));

    NextGSID = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo+src);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo+dest,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo+src+1);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo+dest+1,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo+src+2);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo+dest+2,NextGSID);
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(AC5P_PKT_REGs_lo+src+3);
    __PRAGMA_NO_REORDER

    PPA_FW_SP_BYTE_WRITE(AC5P_PKT_REGs_lo+dest+3,NextGSID);
    __PRAGMA_NO_REORDER


    /*-------------------------------------------------------------------------------------------------------------------------
      - Update new SI value
      -------------------------------------------------------------------------------------------------------------------------*/

    /* Updates new SI value into IPv6 DIP[1:0] */
    __LOG_FW(("Updates new SI value into IPv6 DIP[1:0]"));
    thr53_out_hdr_ptr->IPv6_Header.dip3_low = (thr53_out_hdr_ptr->IPv6_Header.dip3_low&0xFFFC)|si;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(53);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR59_SLS_Test
 * inparam      None
 * return       None
 * description  Debug thread that is used by CV team.
 *              Request from CV to add PHA engine to their SLS test.
 *
 *              cfg template: Not in use (16B)
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (32B) | IPv4(20B) | UDP(8B) |
 *              hdr out:   | Extension space (32B) | IPv4(20B) | UDP(8B) |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 *
 *              Firmwares actions:
 *              - Check IPv4 version
 *              - Execute 2 different dummy accelerators
 *                - Copy first 2 bytes of IP header into CFG template domain [127:112]
 *                - Copy next 2 bytes of IP header into CFG template domain[111:96]
 *                - Do it in loop to get enough cycles
 *              - Calculate IPv4 CS
 *              - Set PHA CNC counter to indicate CV that the thread indeed run
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR59_SLS_Test)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    i;   /* for loop counter */

    /* Get pointers to desc & packet */
    struct ppa_in_desc* thr59_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);
    struct thr59_sls_test_in_hdr* thr59_ipv4_in_hdr_ptr = AC5P_MEM_CAST(AC5P_PKT_REGs_lo,thr59_sls_test_in_hdr);


    /* Check packet is IPv4 */
    __LOG_FW(("Check packet is IPv4"));
    __LOG_PARAM_FW(thr59_ipv4_in_hdr_ptr->IPv4_Header.version);
    if (thr59_ipv4_in_hdr_ptr->IPv4_Header.version == 4)
    {   /* Packet is IPv4 */

        /* Activate some dummy accelerators (copy first 4B of IP into CFG template) */
        __LOG_FW(("Activate some dummy accelerators (copy first 4B of IP into CFG template)"));
        for (i=0;i<7;i++)
        {
           ACCEL_CMD_TRIG(COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_OFFSET, 0);
           ACCEL_CMD_TRIG(COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_PLUS2_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_PLUS2_OFFSET, 0);
        }

        /* Calculate IPv4 checksum */
        __LOG_FW(("Calculate IPv4 checksum"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR59_SLS_Test_LEN2_FROM_PKT_IPv4_Header__version_PLUS8_OFFSET, 0);
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_PLUS12_OFFSET, 0);
        __PRAGMA_NO_REORDER
        /* Store IPv4 checksum */
        __LOG_FW(("Store IPv4 checksum"));
        ACCEL_CMD_TRIG(CSUM_STORE_IP_THR59_SLS_Test_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);
        __PRAGMA_NO_REORDER

        /* Set PHA CNC client */
        __LOG_FW(("Set PHA CNC client"));
        thr59_desc_ptr->ppa_internal_desc.pha_cnc_client_trigger = 0x1;
    }
    else
    {
        /* WARNING WARNING WARNING: Invalid packet since it is not IPv4 !!! Do nothing */
        __LOG_FW(("WARNING WARNING WARNING: Invalid packet since it is not IPv4 !!! Do nothing"));
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(59);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname        THR60_DropAllTraffic
 * inparam         None
 * return          None
 * description     Thread that is used to drop any packet that goes through PPA
 *                 Firmware actions:
 *                 - Set Desc<egress_packet_cmd> to 0x3 (Hard Drop)
 *                 - Set Desc<CPU code> to 183
 *                 Note:
 *                 - this thread is mapped to a fix address and should not be changed.
 *                 - allows to execute the thread while fw version is being upgraded.
 *                 - do not use accelerator commands since it will change thread's code
 *                   since in each build accelerator commands might be in different location
 ************************************************************************************************************************/
__attribute__ ((section(".iram0.text")))
__attribute__ ((aligned (16)))
void PPA_FW(THR60_DropAllTraffic)() {
    /*xt_iss_client_command("profile","enable");             */
    /*xt_iss_client_command("profile","disable");  _exit(0); */

    /* Get pointer to descriptor */
    struct ppa_in_desc*  thr60_desc_ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    /* Set Desc<egress_packet_cmd> to 0x3 (Hard Drop) */
    __LOG_FW(("Set Desc<egress_packet_cmd> to 0x3 (Hard Drop)"));
    thr60_desc_ptr->phal2ppa.egress_packet_cmd = 0x3;

    /* Set Desc<CPU code> to 183 */
    __LOG_FW(("Set Desc<CPU code> to 183"));
    thr60_desc_ptr->phal2ppa.cpu_code = 183;

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    ac5p_stack_overflow_checker(60);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


#if RECOVER_FROM_EXCEPTION
/********************************************************************************************************************//**
 * funcname        ac5pRecoveFromException
 * inparam         None
 * return          None
 * description     Recover from core exception by executing packet swap sequence to send current packet and get the next one.
 *                 fw_drop bit is set in order to indicate that this packet should be dropped.
 *                 NOTE: can use the following instruction to generate an exception for testing => asm volatile ("syscall");
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void ac5pRecoveFromException ()
{
    /* Get pointer to descriptor */
    struct ppa_in_desc* desc_ptr = AC5P_MEM_CAST(Ac5P_DESC_REGs_lo,ppa_in_desc);

    /* Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' */
    __LOG_FW(("Discard the packet. Indicates it by setting fw_drop field in descriptor to '1' "));
    desc_ptr->ppa_internal_desc.fw_drop_code = 0x01;
    __LOG_PARAM_FW(desc_ptr->internal_desc.fw_drop_code);

    /* Notify MG that PPN is recovered from an exception event  */
    __LOG_FW(("Notify MG that PPN is recovered from an exception event"));
    ppn2mg_failure_event(PPN2MG_PPN_EXCEPTION_RECOVER_EVENT,0,0,DONT_STOP_PPN);

    /* Execute the packet swap sequence to jump to the next Thread */
    packet_swap_sequence();
}
#endif  /* RECOVER_FROM_EXCEPTION */



extern void invalidFirmwareThread();

THREAD_TYPE phaThreadsTypeAc5pDefault[] = {
    /* 0*/  PPA_FW(THR0_DoNothing)
    /* 1*/ ,PPA_FW(THR1_SRv6_End_Node)
    /* 2*/ ,PPA_FW(THR2_SRv6_Source_Node_1_segment)
    /* 3*/ ,PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)
    /* 4*/ ,PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)
    /* 5*/ ,PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)
    /* 6*/ ,invalidFirmwareThread
    /* 7*/ ,invalidFirmwareThread
    /* 8*/ ,invalidFirmwareThread
    /* 9*/ ,invalidFirmwareThread
    /*10*/ ,invalidFirmwareThread
    /*11*/ ,PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)
    /*12*/ ,PPA_FW(THR12_MPLS_SR_NO_EL)
    /*13*/ ,PPA_FW(THR13_MPLS_SR_ONE_EL)
    /*14*/ ,PPA_FW(THR14_MPLS_SR_TWO_EL)
    /*15*/ ,PPA_FW(THR15_MPLS_SR_THREE_EL)
    /*16*/ ,PPA_FW(THR16_SGT_NetAddMSB)
    /*17*/ ,PPA_FW(THR17_SGT_NetFix)
    /*18*/ ,PPA_FW(THR18_SGT_NetRemove)
    /*19*/ ,PPA_FW(THR19_SGT_eDSAFix)
    /*20*/ ,PPA_FW(THR20_SGT_eDSARemove)
    /*21*/ ,PPA_FW(THR21_SGT_GBPFixIPv4)
    /*22*/ ,PPA_FW(THR22_SGT_GBPFixIPv6)
    /*23*/ ,PPA_FW(THR23_SGT_GBPRemoveIPv4)
    /*24*/ ,PPA_FW(THR24_SGT_GBPRemoveIPv6)
    /*25*/ ,PPA_FW(THR25_PTP_Phy_1_Step)
    /*26*/ ,invalidFirmwareThread
    /*27*/ ,invalidFirmwareThread
    /*28*/ ,invalidFirmwareThread
    /*29*/ ,invalidFirmwareThread
    /*30*/ ,invalidFirmwareThread
    /*31*/ ,invalidFirmwareThread
    /*32*/ ,invalidFirmwareThread
    /*33*/ ,invalidFirmwareThread
    /*34*/ ,invalidFirmwareThread
    /*35*/ ,invalidFirmwareThread
    /*36*/ ,invalidFirmwareThread
    /*37*/ ,invalidFirmwareThread
    /*38*/ ,invalidFirmwareThread
    /*39*/ ,invalidFirmwareThread
    /*40*/ ,invalidFirmwareThread
    /*41*/ ,invalidFirmwareThread
    /*42*/ ,invalidFirmwareThread
    /*43*/ ,invalidFirmwareThread
    /*44*/ ,invalidFirmwareThread
    /*45*/ ,invalidFirmwareThread
    /*46*/ ,invalidFirmwareThread
    /*47*/ ,invalidFirmwareThread
    /*48*/ ,invalidFirmwareThread
    /*49*/ ,invalidFirmwareThread
    /*50*/ ,invalidFirmwareThread
    /*51*/ ,invalidFirmwareThread
    /*52*/ ,invalidFirmwareThread
    /*53*/ ,PPA_FW(THR53_SRV6_End_Node_GSID_COC32)
    /*54*/ ,invalidFirmwareThread
    /*55*/ ,invalidFirmwareThread
    /*56*/ ,invalidFirmwareThread
    /*57*/ ,invalidFirmwareThread
    /*58*/ ,invalidFirmwareThread
    /*59*/ ,PPA_FW(THR59_SLS_Test)
    /*60*/ ,PPA_FW(THR60_DropAllTraffic)
    /*61*/ ,invalidFirmwareThread
    /*62*/ ,invalidFirmwareThread
    /*63*/ ,invalidFirmwareThread
    /*64*/ ,invalidFirmwareThread
    /*65*/ ,invalidFirmwareThread
    /*66*/ ,invalidFirmwareThread
    /*67*/ ,invalidFirmwareThread
    /*68*/ ,invalidFirmwareThread
    /*69*/ ,invalidFirmwareThread
    /*70*/ ,invalidFirmwareThread
    /*71*/ ,invalidFirmwareThread
    /*72*/ ,invalidFirmwareThread
    /*73*/ ,invalidFirmwareThread
    /*74*/ ,invalidFirmwareThread
    /*75*/ ,invalidFirmwareThread
    /*76*/ ,invalidFirmwareThread
    /*77*/ ,invalidFirmwareThread
    /*78*/ ,invalidFirmwareThread
    /*79*/ ,invalidFirmwareThread
};
