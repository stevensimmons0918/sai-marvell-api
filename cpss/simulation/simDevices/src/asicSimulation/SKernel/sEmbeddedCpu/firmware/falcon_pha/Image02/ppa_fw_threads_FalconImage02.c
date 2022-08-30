/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * ppa_fw_threads_FalconImage02.c
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
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image02/ppa_fw_image_info.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_base_types.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image02/ppa_fw_accelerator_commands.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image02/ppa_fw_threads_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_exception.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/wm_asic_sim_defs.h"
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
* funcname        FalconImage02_get_fw_version
* inparam         fwImageId    PHA firmware image ID
* return          None
* description     Get PHA fw version and save it into SP memory (used in WM)
************************************************************************************************************************/
void FalconImage02_get_fw_version(GT_U32 fwImageId)
{
    /* Get falcon fw version and save it into fixed address in SP memory */
    falcon_get_fw_version(fwImageId, ((VER_YEAR<<24) | (VER_MONTH<<16) | (VER_IN_MONTH<<8) | VER_DEBUG ));
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
    falcon_stack_overflow_checker(0);
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
    struct ppa_in_desc*  thr1_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

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
        segments_left = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + segments_left_ofst) - 1;
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
            PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + segments_left_ofst,segments_left);
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
    falcon_stack_overflow_checker(1);
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
    falcon_stack_overflow_checker(2);
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
    falcon_stack_overflow_checker(3);
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
    struct ppa_in_desc* thr4_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr4_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr4_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122);
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
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116, ipv6NextHeader ); 
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. 
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117, geneircTsDataEthertype ); 

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */ 
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119, geneircTsDataVid ); 

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
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + 4);
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
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 116 + 4, ipv6NextHeader ); 
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. 
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 117 + 4, geneircTsDataEthertype ); 

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */ 
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 119 + 4, geneircTsDataVid ); 

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
    falcon_stack_overflow_checker(4);
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
    struct ppa_in_desc* thr5_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Read Desc<egress_tag_state> to know if vlan exist or not */
    __LOG_FW(("Read Desc<egress_tag_state> to know if vlan exist or not"));
    egressTagState = thr5_desc_ptr->phal2ppa.egress_tag_state;
    __LOG_PARAM_FW(thr5_desc_ptr->phal2ppa.egress_tag_state);

    /* Get Generic_TS_Data.vid[7:0] */
    __LOG_FW(("Get Generic_TS_Data.vid[7:0]"));
    geneircTsDataVid = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 63);
    __LOG_PARAM_FW(geneircTsDataVid);

    /* Get Generic_TS_Data.ethertype  */
    __LOG_FW(("Get Generic_TS_Data.ethertype"));
    geneircTsDataEthertype = PPA_FW_SP_SHORT_READ(FALCON_PKT_REGs_lo + 64);
    __LOG_PARAM_FW(geneircTsDataEthertype);


    /* Check if packet is with or without vlan */
    __LOG_FW(("Check if packet is with or without vlan"));
    if(egressTagState == 0)
    {   /* no vlan */
        __LOG_FW(("no vlan"));

        /* Get IPv6.Next Header */
        __LOG_FW(("Get IPv6.Next Header"));
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122);
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
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132, ipv6NextHeader ); 
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. 
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133, geneircTsDataEthertype ); 

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */ 
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135, geneircTsDataVid ); 

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
        ipv6NextHeader = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo + 122 + 4);
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
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 132 + 4, ipv6NextHeader ); 
        __PRAGMA_NO_REORDER

        /* Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. 
           Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type. */
        __LOG_FW(("Copy Generic TS L2 Header EtherType[15:8] to SRH Base Header.Hdr Ext Len. Copy Generic TS L2 Header EtherType[7:0] to SRH Base Header.Routing Type."));
        PPA_FW_SP_SHORT_WRITE(FALCON_PKT_REGs_lo + 133 + 4, geneircTsDataEthertype ); 

        /* Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left */ 
        __LOG_FW(("Copy Generic TS L2 Header VLAN-ID[7:0] to SRH Base Header.Segment Left"));
        PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo + 135 + 4, geneircTsDataVid ); 

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
    falcon_stack_overflow_checker(5);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is (LC) Line Card port   
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - In this case the Egress port of the ERSPAN packet is LC (Line Card) port
 * 
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              hdr out: | Extension space (22B) | MAC addresses(12B) | TO_ANALYZER extended DSA tag(8B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag. 
 *              Update IPv4 length and calculate IPv4 cs. 
 *              Update HW that 42 bytes are added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;
    uint8_t     VoQThread;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr6_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg* thr6_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg);
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr* thr6_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr);


    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr6_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr6_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr6_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr6_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr6_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr6_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 20B of IPv4 header from shared memory
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 20B of IPv4 header from shared memory"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));
    __PRAGMA_NO_REORDER

    
    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr6_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set extended TO_ANALYZER 8B DSA values
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set extended TO_ANALYZER 8B DSA values"));

    /* Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits */
    __LOG_FW(("Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_extended_DSA_w0_ToAnalyzer__TagCommand_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

   /* Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1 */ 
   __LOG_FW(("Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1"));
   if(vlanVid != 0)
   {
       /* Set SrcTrg_Tagged to '1' */ 
       __LOG_FW(("Set SrcTrg_Tagged to '1'"));
       ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrg_Tagged_OFFSET, 0);
   }

    /* Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)*/ 
    __LOG_FW(("Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__Extend0_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Copy CFI field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy CFI field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__CFI_OFFSET, 0);

    /* Copy up field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy UP field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_up_LEN3_TO_PKT_extended_DSA_w0_ToAnalyzer__UP_OFFSET, 0);

    /* Copy vid field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy vid field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_LEN12_TO_PKT_extended_DSA_w0_ToAnalyzer__VID_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Tunnel MAC addresses 
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Tunnel MAC addresses"));

    /* Set Tunnel MAC addresses from Thread Template */ 
    __LOG_FW(("Copy the Tunnel MAC addresses from Thread Template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_mac_header_outer__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template 
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template.
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template"));

    /* Extract from Global parameters (taken from shared memory) the VoQ Thread indication */
    __LOG_FW(("Extract from Global parameters (taken from shared memory) the VoQ Thread indication"));
    VoQThread = (smemGlobalVal >> 8) & 0xFF;

    __LOG_PARAM_FW(VoQThread);

    if (VoQThread == 1) 
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS6_LEN4_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_PLUS1_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS10_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS11_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }
    else
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_15_0_PLUS13_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_cfi_PLUS1_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }


    /* extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits) */
    __LOG_FW(("extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS13_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__RxSniff_OFFSET, 0);

    /* extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits) */
    __LOG_FW(("extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_15_0_PLUS2_LEN2_TO_PKT_extended_DSA_w1_ToAnalyzer__DropOnSource_OFFSET, 0);
    

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    /* EthType 0x0800. Set 0x08, LSB 0x00 was already cleared by previous commands with SET_REMAIN_FIELDS_TO_ZERO */
    __LOG_FW(("EthType 0x0800. Set 0x08, LSB 0x00 was already cleared by previous commands with SET_REMAIN_FIELDS_TO_ZERO "));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
     __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */ 
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );       
     }
     __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */ 
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else 
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  ); 
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */ 
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else 
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));
        
            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
    __PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv4 length field                         
     - Calculate IPv4 checksum 
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv4 length field + calculate IPv4 checksum"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=20 */
    __LOG_FW(("IPv4 total Length +=20"));
    ACCEL_CMD_TRIG(ADD_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x14_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Desc<Egress Byte Count> +=42 */
    __LOG_FW(("Desc<Egress Byte Count> +=42"));
    ACCEL_CMD_TRIG(ADD_BITS_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_CONST_0x2A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+42 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+42 bytes)."));
    thr6_desc_ptr->ppa_internal_desc.fw_bc_modification = 42;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(6);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is (LC) Line Card port   
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - In this case the Egress port of the ERSPAN packet is LC (Line Card) port
 * 
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              hdr out: | Extension space (2B)  | MAC addresses(12B) | TO_ANALYZER extended DSA tag(8B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag. 
 *              Update IPv6 length. 
 *              Update HW that 62 bytes are added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6)() {
     /*xt_iss_client_command("profile","enable");             */
     /*xt_iss_client_command("profile","disable"); _exit(0);   */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;
    uint8_t     VoQThread;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr7_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg* thr7_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg);
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr* thr7_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr7_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr7_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr7_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr7_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr7_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr7_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 40B of IPv6 header from shared memory
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 40B of IPv6 header from shared memory"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | MAC DA SA | reserved | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));
    __PRAGMA_NO_REORDER

    
    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr7_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set extended TO_ANALYZER 8B DSA values
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set extended TO_ANALYZER 8B DSA values"));

    /* Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits */
    __LOG_FW(("Set TagCommand field to 2. Plus clear to zero all other fields in the range of 32 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_extended_DSA_w0_ToAnalyzer__TagCommand_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

   /* Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1 */ 
   __LOG_FW(("Set SrcTrg_Tagged field to 0 if VID taken from the template is 0 otherwise set it to 1"));
   if(vlanVid != 0)
   {
       /* Set SrcTrg_Tagged to '1' */ 
       __LOG_FW(("Set SrcTrg_Tagged to '1'"));
       ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrg_Tagged_OFFSET, 0);
   }

    /* Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)*/ 
    __LOG_FW(("Set extendedDSATagW0.Extend0 field to '1' plus clear other bits in 32 bits range to zero (clear extendedDSATagW0.Extend1 bit)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__Extend0_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Copy CFI field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy CFI field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__CFI_OFFSET, 0);

    /* Copy up field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy UP field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_up_LEN3_TO_PKT_extended_DSA_w0_ToAnalyzer__UP_OFFSET, 0);

    /* Copy vid field from Thread Template into packet extended TO_ANALYZER tag */ 
    __LOG_FW(("Copy vid field from Thread Template into packet extended TO_ANALYZER tag"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_LEN12_TO_PKT_extended_DSA_w0_ToAnalyzer__VID_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Tunnel MAC addresses 
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Tunnel MAC addresses"));

    /* Set Tunnel MAC addresses from Thread Template */ 
    __LOG_FW(("Copy the Tunnel MAC addresses from Thread Template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_mac_header_outer__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template 
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template.
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Continue to set extended TO_ANALYZER 8B DSA values assuming eDSA tag is now placed in template"));

    /* Extract from Global parameters (taken from shared memory) the VoQ Thread indication */
    __LOG_FW(("Extract from Global parameters (taken from shared memory) the VoQ Thread indication"));
    VoQThread = (smemGlobalVal >> 8) & 0xFF;
    __LOG_PARAM_FW(VoQThread);

    if (VoQThread == 1) 
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[9:6] (take it from template byte 6 plus 6 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS6_LEN4_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_PLUS1_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[5] (take it from template byte 6 plus 10 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS10_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer Trg Phy Port)[4:0] (take it from template byte 6 plus 11 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS11_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }
    else
    {
        /* extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerDev = eDSA TO_ANALYZER(Analyzer Trg Device)[4:0] (take it from template byte 4 plus 13 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_15_0_PLUS13_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Dev_OFFSET, 0);

        /* extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[5] = eDSA TO_ANALYZER(Analyzer ePort)[5] (take it from template byte 14 plus 3 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_LEN1_TO_PKT_extended_DSA_w1_ToAnalyzer__SrcTrgAnalyzer_Port_5_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

        /* extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits) */
        __LOG_FW(("extendedDSATag.AnalyzerPort[4:0] = eDSA TO_ANALYZER(Analyzer ePort)[4:0] (take it from template byte 14 plus 4 bits)"));
        ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_cfi_PLUS1_LEN5_TO_PKT_extended_DSA_w0_ToAnalyzer__SrcTrgAnalyzer_Port_4_0_OFFSET, 0);
    }


    /* extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits) */
    __LOG_FW(("extendedDSATag.rx sniff = eDSA TO_ANALYZER(rx sniff) (take it from template byte 0 plus 13 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS13_LEN1_TO_PKT_extended_DSA_w0_ToAnalyzer__RxSniff_OFFSET, 0);

    /* extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits) */
    __LOG_FW(("extendedDSATag.DropOnSource,PacketIsLooped = eDSA TO_ANALYZER(DropOnSource,PacketIsLooped) (take it from template byte 4 plus 2 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_15_0_PLUS2_LEN2_TO_PKT_extended_DSA_w1_ToAnalyzer__DropOnSource_OFFSET, 0);
    

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    /* EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits  */
    __LOG_FW(("EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits "));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0xdd_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
     __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */ 
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );       
     }
     __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */ 
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else 
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  ); 
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */ 
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else 
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));
        
            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
    __PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv6 length field                         
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv6 length field"));


    /* Set IPv6 Header<payload length> = Desc<Egress Byte Count> */
    __LOG_FW(("Set IPv6 Header<payload length> = Desc<Egress Byte Count>"));

    /* First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b) plus clear to zero 2 MS bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Desc<Egress Byte Count> +=62 */
    __LOG_FW(("Desc<Egress Byte Count> +=62"));
    ACCEL_CMD_TRIG(ADD_BITS_THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_CONST_0x3E_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+62 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+62 bytes)."));
    thr7_desc_ptr->ppa_internal_desc.fw_bc_modification = 62;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(7);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is Falcon direct port   
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - In this case the Egress port of the ERSPAN packet is Falcon direct port
 * 
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              2 options: with vlan tag and without vlan tag
 *              hdr out(with vlan): | Extension space (26B) | MAC addresses(12B) | VLAN tag(4B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              hdr out(no vlan):   | Extension space (30B) | MAC addresses(12B) | ETH type(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag. 
 *              Update IPv4 length and calculate IPv4 cs. 
 *              Update HW that 34B(no vlan) or 38B(with vlan) are added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr8_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg* thr8_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg);
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr* thr8_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr8_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr8_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr8_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr8_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr8_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr8_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |
      - Since L2 location in packet output is not 32 bits align it is copied first by core to template and then by accelerator to packet output   
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 20B of IPv4 header from shared memory into packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 20B of IPv4 header from shared memory into packet output"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));

    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr8_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    /* EthType 0x0800. Set 0x08, LSB 0x00 is cleared by SET_REMAIN_FIELDS_TO_ZERO data */
    __LOG_FW(("EthType 0x0800. Set 0x08, LSB 0x00 is cleared by SET_REMAIN_FIELDS_TO_ZERO data "));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv4 length field     
     - Set MAC DA SA and vlan tag if exist                    
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv4 length field"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+20"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b) plus clear to zero 2 MS bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=20 */
    __LOG_FW(("IPv4 total Length +=20"));
    ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x14_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);

    if(vlanVid == 0)
    { /* no vlan */
      __LOG_FW(("no vlan"));

        /* Desc<Egress Byte Count> +=34 */
        __LOG_FW(("Desc<Egress Byte Count> +=34"));
        ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x22_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+34 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+34 bytes)."));
        thr8_desc_ptr->ppa_internal_desc.fw_bc_modification = 34;

        /* Set tunnel MAC addresses from template (copied earlier from shared memory to template) */ 
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_no_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }
    else
    { /* with vlan */
      __LOG_FW(("with vlan"));

        /* Desc<Egress Byte Count> +=38 */
        __LOG_FW(("Desc<Egress Byte Count> +=38"));
        ACCEL_CMD_TRIG(ADD_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x26_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+38 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+38 bytes)."));
        thr8_desc_ptr->ppa_internal_desc.fw_bc_modification = 38;

        /* Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template) */ 
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_LEN16_TO_PKT_with_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template 
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
     __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */ 
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );       
     }
     __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */ 
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else 
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  ); 
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */ 
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else 
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));
        
            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
    __PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Calculate IPv4 checksum
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate IPv4 checksum"));


    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(8);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel for ePort-based CC (Centralized Chassis) while Egress port of the ERSPAN packet is Falcon direct port   
 *              Encapsulated Remote Switch Port Analyzer (ERSPAN) allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - In this case the Egress port of the ERSPAN packet is Falcon direct port
 * 
 *              cfg template:    Not in use (16B). It is used by fw to first hold L2 parameters and then eDSA tag.
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | TO_ANALYZER eDSA tag(16B) | payload |
 *              2 options: with vlan tag and without vlan tag
 *              hdr out(with vlan): | Extension space (06B) | MAC addresses(12B) | VLAN tag(4B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              hdr out(no vlan):   | Extension space (10B) | MAC addresses(12B) | ETH type(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | MAC addresses(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              Remove eDSA tag. 
 *              Update IPv6 length 
 *              Update HW that 54B(no vlan) or 58B(with vlan) are added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */


    uint32_t    srcTrgDev;
    uint32_t    sessionId;
    uint32_t    smemAnalyzerTemplateBaseAddr;
    uint32_t    deviceIdVal;
    uint32_t    vlanVid;
    uint8_t     getDeviceIdFromPkt;
    uint32_t    smemAnalayzerTemplateOfst;
    uint8_t     descRxSniff;
    uint32_t    smemGlobalVal;


    /* Get pointers to descriptor,cfg & packet */
    struct ppa_in_desc* thr9_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg* thr9_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg);
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr* thr9_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr);

    /* Get source or target device field value from packet input in eDSA tag */
    __LOG_FW(("Get source or target device field value from packet input in eDSA tag"));
    srcTrgDev = (thr9_in_hdr_ptr->eDSA_w3_ToAnalyzer.SrcTrg_Dev_11_5<<5) | (thr9_in_hdr_ptr->eDSA_w0_ToAnalyzer.SrcTrg_Dev_4_0);
    __LOG_PARAM_FW(srcTrgDev);

    /* Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory */
    __LOG_FW(("Get packet input eDSA.W2 bit 21 which indicates if Device ID should be taken from packet or from shared memory"));
    getDeviceIdFromPkt = thr9_in_hdr_ptr->eDSA_w2_ToAnalyzer.Reserved & 0x1;
    __LOG_PARAM_FW(getDeviceIdFromPkt);

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr9_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr9_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr9_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |
      - Since L2 location in packet output is not 32 bits align it is copied first by core to template and then by accelerator to packet output   
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 16B of L2 parameters from shared memory into CFG template: | MAC DA SA | TPID | UP | CFI | VID |"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Copy 40B of IPv6 header from shared memory into packet output
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Copy 40B of IPv6 header from shared memory into packet output"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 60) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));

    /* Get vlan vid from template. Was copied earlier from shared memory */
    __LOG_FW(("Get vlan vid from template. Was copied earlier from shared memory"));
    vlanVid = thr9_cfg_ptr->cc_erspan_template.vlan_vid;
    __LOG_PARAM_FW(vlanVid);

    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    /* EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits  */
    __LOG_FW(("EthType 0x86dd. Do it with 2 accelerators since maximum bits for const command is 11 bits "));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0xdd_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update Desc<Egress Byte Count>
     - Update packet IPv6 length field     
     - Set MAC DA SA and vlan tag if exist                    
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update Desc<Egress Byte Count> + update IPv6 length field"));

    /* Set IPv6 Header<Payload Length> = Desc<Egress Byte Count> */
    __LOG_FW(("Set IPv6 Header<Payload Length>= Desc<Egress Byte Count>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);

    if(vlanVid == 0)
    { /* no vlan */
      __LOG_FW(("no vlan"));

        /* Desc<Egress Byte Count> +=54 */
        __LOG_FW(("Desc<Egress Byte Count> +=54"));
        ACCEL_CMD_TRIG(ADD_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x36_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+54 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+54 bytes)."));
        thr9_desc_ptr->ppa_internal_desc.fw_bc_modification = 54;

        /* Set tunnel MAC addresses from template (copied earlier from shared memory to template) */ 
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN12_TO_PKT_no_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }
    else
    { /* with vlan */
      __LOG_FW(("with vlan"));

        /* Desc<Egress Byte Count> +=58 */
        __LOG_FW(("Desc<Egress Byte Count> +=58"));
        ACCEL_CMD_TRIG(ADD_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x3a_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);

        /* Indicates HW about the new starting point of the packet header (+58 bytes). */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+58 bytes)."));
        thr9_desc_ptr->ppa_internal_desc.fw_bc_modification = 58;

        /* Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template) */ 
        __LOG_FW(("Set tunnel MAC addresses + vlan tag from template (copied earlier from shared memory to template)"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_LEN16_TO_PKT_with_vlan__mac_header_outer__mac_da_47_32_OFFSET, 0);
    }


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template 
     - Remove packet eDSA tag (16B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Since copy bits accelerators can not be used from packet to packet, copy 16 bytes of packet eDSA tag into thread's template"));
    __LOG_FW(("Remove packet eDSA tag (16B)"));

    /* Copy 16 bytes of packet eDSA tag into template */
    __LOG_FW(("Copy 16 bytes of packet eDSA tag into template"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_PKT_eDSA_w0_ToAnalyzer__TagCommand_LEN16_TO_CFG_cc_erspan_template__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Remove the TO_ANALYZER eDSA tag by moving right the original MAC addresses by 16 bytes */
    ACCEL_CMD_TRIG(SHIFTRIGHT_16_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Assign 0x1000 into GRE Header Flags fields. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Assign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits */
    __LOG_FW(("AAssign 0x88BE into GRE Header Protocol field. Do it with 2 accelerators since maximum bits for const command is 11 bits"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Assign Desc<LM Counter> into GRE Header Sequence Number field */
    __LOG_FW(("Assign Desc<LM Counter> into the <Sequence Number> field in the GRE header"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

     /* Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0 */
     __LOG_FW(("Set ERSPAN.Index[17:14] (Falcon Device ID). Plus ERSPAN.Reserved = 0"));

     /* First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7) */
     __LOG_FW(("First take it from packet eDSA.word2.reserved2 (template byte 8 bits offset 7)"));
     ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS7_LEN3_TO_PKT_ERSPAN_type_II_header__Index_PLUS3_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
     __PRAGMA_NO_REORDER

     /* Check if need to overrun it since need to take it from shared memory */ 
     if (getDeviceIdFromPkt == 0)
     {
         /* Extract from Global parameters (taken from shared memory) the Falcon Device ID */
         __LOG_FW(("Extract from Global parameters (taken from shared memory) the Falcon Device ID"));
         deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>2;
         __LOG_PARAM_FW(deviceIdVal);

         PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 76) , deviceIdVal  );       
     }
     __PRAGMA_NO_REORDER

    /* Check if RX or TX mirroring */
    __LOG_FW(("Assign ERSPAN.Index"));
    if (descRxSniff == 1)
    {
        /* RX mirroring */ 
        __LOG_FW(("RX mirroring"));

        if ( srcTrgDev <= 7 )
        {   /* RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7) */
            __LOG_FW(("RX Mirroring (type 1) in case packet was mirrored from Falcon (0<= SrcDev <= 7)"));

            /* Set Index Bits [19:18] (type) to '1' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '1'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x1_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits) */
            __LOG_FW(("Set Index Bits [13:0] (ePort). Take it from packet eDSA.src_ePort (take it from template byte 8 plus 15 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS15_LEN14_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);
        }
        else 
        {   /* RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15) */
            __LOG_FW(("RX Mirroring (type 0) in case packet was mirrored from LC (8<= SrcDev <= 15)"));

            /* Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits) */
            __LOG_FW(("Set Index Bits [13:9] (src device) from packet eDSA.srcDev[4:0] (take it from template byte 0 plus 3 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Src Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Src Phy Port)"));

            /* Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits) */
            __LOG_FW(("Set Index Bits [8:7] (Src Phy Port) from packet 0,eDSA.Src_Phy_Port_7 (take it from template byte 8 plus 11 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits) */
            __LOG_FW(("Set Index Bits [6:5] (Src Phy Port) from packet eDSA.Src_Phy_Port_6,eDSA.Src_Phy_Port_5 (take it from template byte 6 plus 4 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)*/
            __LOG_FW(("Set Index Bits [4:0] (Src Phy Port) from packet eDSA.Src_Phy_Port_4_0 (take it from template byte 0 plus 8 bits)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero */
        __LOG_FW(("Write Ingress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Ingress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , (sessionId >>10) & 0x3FF  ); 
        __LOG_PARAM_FW((sessionId >>10) & 0x3FF);
    }
    else
    {
        /* TX mirroring */ 
        __LOG_FW(("TX mirroring"));

        if (srcTrgDev <= 15)
        {   /* TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15) */
            __LOG_FW(("TX Mirroring (type 0) in case packet was mirrored from Falcon or LC without VoQ (TrgDev <= 15)"));

            /* Set Index Bits [13:9] (target device). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3) */
            __LOG_FW(("Take it from packet eDSA.word2.reserved2 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS6_OFFSET, 0);

            /* Set Index Bits [8:0] (Trg Phy Port)*/
            __LOG_FW(("Set Index Bits [8:0] (Trg Phy Port)"));

            /* Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)*/
            __LOG_FW(("Set Index Bits [8:7] (Trg Phy Port). Take it from packet 0,eDSA.SrcTrg_Phy_Port_7 (template byte 0 bits offset 11)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_31_16_PLUS11_LEN1_TO_PKT_ERSPAN_type_II_header__Index_PLUS12_OFFSET, 0);
            /* Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4) */
            __LOG_FW(("Set Index Bits [6:5] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_6,eDSA.SrcTrg_Phy_Port_5 (template byte 6 bits offset 4)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_sa_47_32_PLUS4_LEN2_TO_PKT_ERSPAN_type_II_header__Index_PLUS13_OFFSET, 0);
            /* Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Phy Port). Take it from packet eDSA.SrcTrg_Phy_Port_4_0 (template byte 0 bits offset 5)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS8_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }
        else 
        {   /* TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16) */
            __LOG_FW(("TX Mirroring (type 2) in case packet was mirrored from LC with VoQ (TrgDev >= 16)"));
        
            /* Set Index Bits [19:18] (type) to '2' */
            __LOG_FW(("Set Index Bits [19:18] (type) to '2'"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x2_LEN2_TO_PKT_ERSPAN_type_II_header__Index_OFFSET, 0);

            /* Set Index Bits [13:0] (Trg Dev)*/
            __LOG_FW(("Set Index Bits [13:0] (Trg Dev)"));

            /* Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9) */
            __LOG_FW(("Set Index Bits [13:5] (Trg Dev). Take it from packet 0,0,eDSA.SrcTrgDev_11_5 (template byte 14 bits offset 9)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__vlan_vid_PLUS5_LEN7_TO_PKT_ERSPAN_type_II_header__Index_PLUS8_OFFSET, 0);

            /* Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)*/
            __LOG_FW(("Set Index Bits [4:0] (Trg Dev). Take it from packet eDSA.SrcTrgDev_4_0 (template byte 0 bits offset 3)"));
            ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_47_32_PLUS3_LEN5_TO_PKT_ERSPAN_type_II_header__Index_PLUS15_OFFSET, 0);
        }

        /* Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero */
        __LOG_FW(("Write Egress session ID to packet ERSPAN header.sessionID. Write it as 4B while other bits than Egress sessionID are cleared to zero"));
        PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 72) , sessionId & 0x3FF  );
        __LOG_PARAM_FW(sessionId & 0x3FF);

    }
    __PRAGMA_NO_REORDER

    /* Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Assign ERSPAN.Ver with 1(4b) and ERSPAN.VLAN(12b) with 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits) */
    __LOG_FW(("Set ERSPAN.COS value from eDSA.UP (take it from template byte 2 plus 0 bits)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CFG_cc_erspan_template__mac_da_31_16_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0 */
    __LOG_FW(("Assign ERSPAN.En with 3(2b) and ERSPAN.T(1b) with 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(9);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR10_Cc_Erspan_TypeII_SrcDevMirroring
 * inparam      None
 * return       None
 * description  ePort-based CC (Centralized Chassis) for source device mirroring.
 *              Thread to process ERSPAN Type II Source Device Mirroring.   
 *              
 *              cfg template:    Not in use (16B). It is used by fw to hold device ID value. 
 *              src & trg entries: Not is use (4B)
 *              hdr in:  | Extension space (64B) | MAC addresses(12B) | eDSA tag(16B) | payload |
 *              hdr out: | Extension space (64B) | MAC addresses(12B) | eDSA tag(16B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *              Firmware fill eDSA.W2.reserved bits 22:24 with device ID value 
 *              Also set eDSA.W2.reserved bit 21 to '1' to indicate that Falcon Device ID was set.
 *              The header remains with the same size no addition here.
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR10_Cc_Erspan_TypeII_SrcDevMirroring)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    deviceIdVal;
    uint32_t    smemGlobalVal ;


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get Global parameters from shared memory. Should includes | Falcon Device ID(2B) | VoQ Thread (1B) | reserved(1B) |"));

    smemGlobalVal = PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR);
    __LOG_PARAM_FW(PPA_FW_SP_READ(DRAM_CC_ERSPAN_GLOBAL_ADDR));


    /* Extract from Global parameters the Falcon Device ID */
    __LOG_FW(("Extract from Global parameters the Falcon Device ID"));
    deviceIdVal = (smemGlobalVal & 0xFFFF0000)>>16;
    __LOG_PARAM_FW(deviceIdVal);

    /* Write it first to template */
    __LOG_FW(("Write it first to template"));
    PPA_FW_SP_WRITE( FALCON_CFG_REGs_lo , deviceIdVal  ); 
    __PRAGMA_NO_REORDER

    /* Now set eDSA.word2 bits 24:22 with the Falcon Device ID which is now placed in template */
    __LOG_FW(("Now set eDSA.word2 bits 24:22 with the Falcon Device ID which is now placed in template"));
    ACCEL_CMD_TRIG(COPY_BITS_THR10_Cc_Erspan_TypeII_SrcDevMirroring_CFG_HA_Table_reserved_space__reserved_0_PLUS29_LEN3_TO_PKT_eDSA_w2_ToAnalyzer__Reserved_PLUS6_OFFSET, 0);

    /* Set '1' in eDSA.word2 bit 21 to indicate that Falcon Device ID was set */
    __LOG_FW(("Set '1' in eDSA.word2 bit 21 to indicate that Falcon Device ID was set"));
    ACCEL_CMD_TRIG(COPY_BITS_THR10_Cc_Erspan_TypeII_SrcDevMirroring_CONST_0x1_LEN1_TO_PKT_eDSA_w2_ToAnalyzer__Reserved_PLUS9_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(10);
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
 *                               - Applicable values: For Falcon it should be 2:19. It should be checked by the CPSS API and not by fw.
 *                               CopyReservedLSB: 
 *                               - Indicates fw the LS bit of the Group Policy ID value inside Desc<copy reserved> field.
 *                               - Meaning from which bit the Group Policy value starts inside Desc<copy reserved> field.
 *                               - Applicable values: For Falcon it should be 2:19. It should be checked by the CPSS API and not by fw.
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
    struct thr11_vxlan_gpb_cfg* thr11_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr11_vxlan_gpb_cfg);
    struct thr11_vxlan_gpb_in_hdr_ipv4* thr11_ipv4_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv4);
    struct thr11_vxlan_gpb_in_hdr_ipv6* thr11_ipv6_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr11_vxlan_gpb_in_hdr_ipv6);


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
   
    __LOG_PARAM_FW(PPA_FW_SP_READ(FALCON_DESC_REGs_lo + 12));
    groupPolicyIdValue = (PPA_FW_SP_READ(FALCON_DESC_REGs_lo + 12) >> (12 + thr11_cfg_ptr->vxlan_gpb_template.CopyReservedLSB) ) & groupPolicyIdmask ;
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
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 62) , groupPolicyIdValue );
    }
    else if (thr11_ipv6_in_hdr_ptr->IPv6_Header.version == 6)
    {
        /* Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID */
        __LOG_FW(("Packet is IPv6. Set VXLAN header<Group_Policy_ID> with Group Policy ID"));
        PPA_FW_SP_SHORT_WRITE( (FALCON_PKT_REGs_lo + 62 + 20) , groupPolicyIdValue );
    }
    else
    {
        /* WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet */
        __LOG_FW(("WARNING WARNING WARNING: Invalid packet since no IPv4 and no IPv6 !!! Do not change the packet"));
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(11);
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
    struct ppa_in_desc* thr12_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr12_MPLS_NO_EL_in_hdr* thr12_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr12_MPLS_NO_EL_in_hdr);

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
    falcon_stack_overflow_checker(12);
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
    struct ppa_in_desc* thr13_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr13_MPLS_ONE_EL_in_hdr* thr13_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr13_MPLS_ONE_EL_in_hdr);

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
    falcon_stack_overflow_checker(13);
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
    struct ppa_in_desc* thr14_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr14_MPLS_TWO_EL_in_hdr* thr14_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr14_MPLS_TWO_EL_in_hdr);

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
    falcon_stack_overflow_checker(14);
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
    struct ppa_in_desc* thr15_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);
    struct thr15_MPLS_THREE_EL_in_hdr* thr15_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr15_MPLS_THREE_EL_in_hdr);

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
    falcon_stack_overflow_checker(15);
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
    struct thr53_SRV6_End_Node_GSID_COC32_in_hdr *thr53_in_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_in_hdr);
    struct thr53_SRV6_End_Node_GSID_COC32_cfg *thr53_cfg_ptr = FALCON_MEM_CAST(FALCON_CFG_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_cfg);
    struct thr53_SRV6_End_Node_GSID_COC32_out_hdr *thr53_out_hdr_ptr = FALCON_MEM_CAST(FALCON_PKT_REGs_lo,thr53_SRV6_End_Node_GSID_COC32_out_hdr);

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

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src); 
    __PRAGMA_NO_REORDER
    
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest,NextGSID);     
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+1); 
    __PRAGMA_NO_REORDER
    
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+1,NextGSID);     
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+2); 
    __PRAGMA_NO_REORDER
    
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+2,NextGSID);     
    __PRAGMA_NO_REORDER

    NextGSID = PPA_FW_SP_BYTE_READ(FALCON_PKT_REGs_lo+src+3); 
    __PRAGMA_NO_REORDER
    
    PPA_FW_SP_BYTE_WRITE(FALCON_PKT_REGs_lo+dest+3,NextGSID);     
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
    falcon_stack_overflow_checker(53);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR54_IPv4_TTL_Increment
 * inparam      None
 * return       None
 * description  This thread handles specific request.
 *              For selected IPv4 routed flows and if router next hop is configured to "decrement TTL" 
 *              HA unit will decrement TTL and PHA fw will increment it back so TTL number will stay the same.
 *
 *              cfg template: Not in use (16B) 
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv4(20B)
 *              hdr out: Extension space(32B),IPv4(20B)
 *
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 * 
 *              Firmware actions:
 *              - if Desc<dec ttl> == 1  
 *                 - Increment IPv4 TTL
 *                 - Incremental Checksum Update
 *              - if Desc<dec ttl> == 0 do nothing
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR54_IPv4_TTL_Increment) () {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr54_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Check Desc<dec ttl> */
    __LOG_FW(("Check Desc<dec ttl>"));
    __LOG_PARAM_FW(thr54_desc_ptr->phal2ppa.dec_ttl);
    if (thr54_desc_ptr->phal2ppa.dec_ttl == 1)
    {
        /*-----------------------------------------------------------------------------------------------------------------
          - Increment IPv4.ttl field by '1' 
          - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old ttl) + (new ttl) )
            Since CS accelerators work with 2 bytes resolution we will do CS of ttl(8b) + protocol(8b)
          ----------------------------------------------------------------------------------------------------------------*/

        /* Sum old IPv4.ttl together with IPv4.protocol (since CS acc work with 2B resolution) */
        __LOG_FW(("Sum old IPv4.ttl with IPv4.protocol (since CS acc work with 2B resolution)"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR54_IPv4_TTL_Increment_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);
        __PRAGMA_NO_REORDER
        /* Increment IPv4.ttl field by '1' */
        __LOG_FW(("Increment IPv4.ttl field by '1'"));
        ACCEL_CMD_TRIG(ADD_BITS_THR54_IPv4_TTL_Increment_CONST_0x1_LEN8_TO_PKT_IPv4_Header__ttl_OFFSET, 0);
        __PRAGMA_NO_REORDER
        /* Sum new IPv4.ttl together with IPv4.protocol (since CS acc work with 2B resolution) */
        __LOG_FW(("Sum new Total Length using accelerator cmd"));
        ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR54_IPv4_TTL_Increment_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);
        /* Sum current/old IPv4 CS */
        __LOG_FW(("Sum current/old IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR54_IPv4_TTL_Increment_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
        __PRAGMA_NO_REORDER
        /* Store the new IPv4 CS */
        __LOG_FW(("Sum current/old IPv4 CS"));
        ACCEL_CMD_TRIG(CSUM_STORE_IP_THR54_IPv4_TTL_Increment_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);
    }
    

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(54);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR55_IPv6_HopLimit_Increment
 * inparam      None
 * return       None
 * description  This thread handles specific request.
 *              For selected IPv6 routed flows and if router next hop is configured to "decrement TTL" 
 *              HA unit will decrement Hop Limit and PHA fw will increment it back so Hop Limit number will stay the same.
 *
 *              cfg template: Not in use (16B) 
 *              src & trg entries: Not in use (4B)
 *              hdr in:  Extension space(32B),IPv6(40B)
 *              hdr out: Extension space(32B),IPv6(40B)
 *
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 128 bytes + 32 bytes Header increment space
 * 
 *              Firmware actions:
 *              - if Desc<dec ttl> == 1  
 *                 - Increment IPv6 Hop limit
 *              - if Desc<dec ttl> == 0 do nothing
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR55_IPv6_HopLimit_Increment) () {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr55_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Check Desc<dec ttl> */
    __LOG_FW(("Check Desc<dec ttl>"));
    __LOG_PARAM_FW(thr55_desc_ptr->phal2ppa.dec_ttl);
    if (thr55_desc_ptr->phal2ppa.dec_ttl == 1)
    {
        /* Increment IPv6.hop_limit field by '1' */
        __LOG_FW(("Increment IPv6.hop_limit field by '1'"));
        ACCEL_CMD_TRIG(ADD_BITS_THR55_IPv6_HopLimit_Increment_CONST_0x1_LEN8_TO_PKT_IPv6_Header__hop_limit_OFFSET, 0);
    }

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(55);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/********************************************************************************************************************//**
 * funcname     THR56_Clear_Outgoing_Mtag_Cmd
 * inparam      None
 * return       None
 * description  Handles a case in which need to apply EPCL rule to trap packet that was already trapped by an ingress 
 *              pipeline engine, e.g. router exception. In such a case the request is to assign a new CPU Code which
 *              to change the TC assignment.
 *              In the design, EREP will not perform Egress Trap replication to the EQ if Desc<Outgoing Mtag Cmd>=TO_CPU
 *              This is the case when it was trapped already in the ingress pipeline. Solution is for EPCL rule to assign
 *              action of TRAP and to trigger PHA thread that will set Desc<Outgoing Mtag Cmd> to FORWARD.

 *              cfg template: Not in use (16B) 
 *              src & trg entries: Not in use (4B)
 *              hdr in:  don't care (change is only done in descriptor field)
 *              hdr out: don't care (change is only done in descriptor field)
 *
 *              Header Window anchor= don't care let's set it to Outer Layer 3
 *              Header Window size= don't care   let's set it to 128 bytes + 32 bytes Header increment space
 * 
 *              Firmware actions:
 *              - Sets Desc<outgoing_mtag_cmd> to 0x3 (FORWARD)
 *              - Set Desc<Flow-ID> = InDesc<Local Dev Src Port> 
 *              - Set Desc<Egress Tag State> = 0x7 ("Do_Not_Modify")
 *                The packet is egressed as it was received without modifying its original VLAN tags. 
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR56_Clear_Outgoing_Mtag_Cmd) () {
    /*xt_iss_client_command("profile","enable");            */
    /*xt_iss_client_command("profile","disable"); _exit(0); */

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr56_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Set Desc<egress_tag_state> field to 0x7 ("Do_Not_Modify") */
    __LOG_FW(("Set Desc<egress_tag_state> field to 7 (Do_Not_Modify)"));
    thr56_desc_ptr->phal2ppa.egress_tag_state = 0x7; 

    /* Sets Desc<outgoing_mtag_cmd> to 0x3 (FORWARD) */
    __LOG_FW(("Sets Desc<outgoing_mtag_cmd> to 0x3 (FORWARD)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR56_Clear_Outgoing_Mtag_Cmd_CONST_0x3_LEN2_TO_DESC_phal2ppa__outgoing_mtag_cmd_OFFSET, 0);
    
    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Sets OutDesc<Flow-ID> = InDesc<Local Dev Src Port> 
      - Since Desc<Flow-ID> is 13 bits and Desc<Local Dev Src Port> is 10 bits only, need to to it in 2 commands
      - First to copy 10 bits of Desc<Local Dev Src Port> to the 10 LS bits of Desc<Flow-ID>
      - Second to clear 3 MS bits of Desc<Flow-ID> with zeros
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Copy 10 bits of Desc<Local Dev Src Port> to the 10 LS bits of Desc<Flow-ID> */
    __LOG_FW(("Copy 10 bits of Desc<Local Dev Src Port> to the 10 LS bits of Desc<Flow-ID>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR56_Clear_Outgoing_Mtag_Cmd_DESC_phal2ppa__local_dev_src_port_LEN10_TO_DESC_phal2ppa__flow_id_PLUS3_OFFSET, 0);

    /* Clear 3 MS bits of Desc<Flow-ID> with zeros */
    __LOG_FW(("Clear 3 MS bits of Desc<Flow-ID> with zeros"));
    ACCEL_CMD_TRIG(COPY_BITS_THR56_Clear_Outgoing_Mtag_Cmd_CONST_0x0_LEN3_TO_DESC_phal2ppa__flow_id_OFFSET, 0);


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(56);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR57_SFLOW_V5_IPv4
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers. 
 *              It requires ingress/egress mirrored packets to be sent over an IPv4-UDP tunnel to a collector in the network.
 *              Following the UDP header there is SFLOW header which is followed by the mirrored packet as the payload.
 *              The L2-IPv4-UDP tunnel encapsulation is added by the Falcon Tunnel-start in the HA unit.
 *              The SFLOW header is inserted by the PHA firmware after the UDP header.
 * 
 *              cfg template: 4 LSB holds IPv4 agent address rest 12 bytes are reserved 
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (64B) | IPv4(20B) | UDP(8B) | TS place holder (16B)| payload |
 *              hdr out:   | Extension space (12) | IPv4(20B) | UDP(8B) | sFlow header(28B) | Data Format Header (8B) | Flow Sample Header (32B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares actions:
 *              - Adds sFlow header + Data Format Header + Flow Sample Header and sets it according to architect's definitions.  
 *              - Updates IPv4 length to + 52 bytes
 *              - Updates UDP length to + 52 bytes  
 *              - Calculates IPv4 CS  
 *              - Updates HW that 52 bytes were added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR57_SFLOW_V5_IPv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    ts_sec;                             /* timestamp in seconds */
    uint32_t    ts_nano_sec;                        /* timestamp in nano seconds */
    uint32_t    ts_sec_new;                         /* holds timestamp in seconds after reading it for the second time to check wraparound */  
    uint32_t    ts_sec_to_msec;                     /* holds seconds value in milliseconds units */
    uint32_t    ts_nano_sec_to_msec;                /* holds nano seconds value in milliseconds units */
    uint32_t    uptime;                             /* holds uptime which is the time (in ms) since system boot */ 
    uint8_t     tai_sel;                            /* TAI select index 0 or 1 */
    uint32_t    sampleSeqNum;                       /* holds sample Sequence Number taken from shared memory */
    uint16_t    dropsNum;                           /* holds drops number taken from shared memory */
    uint32_t    smemSrcPortSampleSeqTableEntryAddr; /* holds address of specific entry in src port sample sequence number table in shared memory */
    uint32_t    smemSrcPortDropsTableEntryAddr;     /* holds address of specific entry in src port drops table in shared memory */
    uint32_t    smemSrcPortIndx;                    /* holds index (0:127) to src port table in shared memory */ 


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr57_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr57_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Shift packet left to make room for sFlow headers
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move left IPv4(20B) + UDP(8B) by 52 bytes to make room for sFlow headers */
    __LOG_FW(("Move left IPv4(20B) + UDP(8B) by 52 bytes to make room for sFlow headers"));
    ACCEL_CMD_TRIG(SHIFTLEFT_52_BYTES_THR57_SFLOW_V5_IPv4_LEN28_FROM_PKT_IPv4_Header__version_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Move left TS place holder (16B) by 52 bytes to make room for sFlow headers */
    __LOG_FW(("Move left TS place holder (16B) by 52 bytes to make room for sFlow headers"));
    ACCEL_CMD_TRIG(SHIFTLEFT_52_BYTES_THR57_SFLOW_V5_IPv4_LEN16_FROM_PKT_IPv4_Header__version_PLUS28_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow header (28B). 7 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow header. 7 words each of 32 bits."));

    /* Word0: sFlow Version. Set to fixed value 0x5 */                                                    
    __LOG_FW(("Word0: sFlow Version. Set to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x5_LEN3_TO_PKT_sflow_ipv4_header__version_PLUS29_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word1: IP version. Set to fixed value 0x1 (IPv4) */                                                    
    __LOG_FW(("Word1: IP version. Set to fixed value 0x1 (IPv4)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__agent_ip_version_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word2: Agent IPv4 Address. Set to template[31:0] */
    __LOG_FW(("Word2: Agent IPv4 Address. Set to template[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_IPv4_CFG_sflow_ipv4_template__agent_ip_address_LEN4_TO_PKT_sflow_ipv4_header__agent_ip_address_OFFSET, 0);

    /* Word3: Sub Agent ID. Set to 0 */
    __LOG_FW(("Word3: Sub Agent ID. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x0_LEN1_TO_PKT_sflow_ipv4_header__sub_agent_id_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word4: Sequence number. Set to Desc<LM Counter[31:0]> (sequence per target analyzer) */
    __LOG_FW(("Word4: Sequence number. Set to Desc<LM Counter[31:0]> (sequence per target analyzer)"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_IPv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_sflow_ipv4_header__sequence_number_OFFSET, 0);

    /*------------------------------------------------------------------------------------------------------------------------------------------------
      Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns.
      - The uptime is the time (in ms) since system boot. 
      - In our case we have 18 bits of seconds and 30 bits of ns that is ~3 days
      - Since 18b for seconds we assume that multiplication of 1000 to convert to milliseconds should not be bigger than 32b size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns."));

    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);
    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
    __PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);

    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* Convert seconds to milliseconds units */
    __LOG_FW(("Convert seconds to milliseconds units"));
    ts_sec_to_msec = ts_sec * 1000 ;

    /* Convert nano seconds to milliseconds units */
    __LOG_FW(("Convert nano seconds to milliseconds units"));
    ts_nano_sec_to_msec = ts_nano_sec / 1000000 ;

    /* Calculate uptime in milliseconds */
    __LOG_FW(("Calculate uptime in milliseconds"));
    uptime = ts_sec_to_msec + ts_nano_sec_to_msec ;

    /* Set uptime in sFlow header */
    __LOG_FW(("Set uptime in sFlow header"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 60 , uptime );

    /* Word6: Number of samples. Set to fixed value 1 */                                                    
    __LOG_FW(("Word6: Number of samples. Set to fixed value 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__samples_number_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits.     
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits."));

    /* Word7: Data Format. Set to template[63:32] */                                                    
    __LOG_FW(("Word7: Data Format. Set to template[63:32]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_IPv4_CFG_sflow_ipv4_template__data_format_header_LEN4_TO_PKT_sflow_data_format_header__smi_private_enterprise_code_OFFSET, 0);

    /* Word8: Set Sample Length to 32 (decimal) */                                                    
    __LOG_FW(("Word8: Set Sample Length to 32 (decimal)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x20_LEN8_TO_PKT_sflow_data_format_header__sample_length_PLUS24_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow Flow Sample Header (32B). 8 words each of 32 bits.      
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow Flow Sample Header (32B). 8 words each of 32 bits."));

    /* Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0] */                                                    
    __LOG_FW(("Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0]"));

    /* Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127. */
    __LOG_FW(("Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127."));
    smemSrcPortIndx = (thr57_desc_ptr->phal2ppa.local_dev_src_port & 0x7F);
    __LOG_PARAM_FW(thr57_desc_ptr->phal2ppa.local_dev_src_port);
    __LOG_PARAM_FW(smemSrcPortIndx);

    /* Get shared memory address compatible to packet's source port sample sequence */
    __LOG_FW(("Get shared memory address compatible to packet's source port sample sequence"));
    smemSrcPortSampleSeqTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR + smemSrcPortIndx*4;
    __LOG_PARAM_FW(smemSrcPortSampleSeqTableEntryAddr);

    /* Read sample sequence number from shared memory */
    __LOG_FW(("Read sample sequence number from shared memory"));
    sampleSeqNum = PPA_FW_SP_READ(smemSrcPortSampleSeqTableEntryAddr);
    __LOG_PARAM_FW(sampleSeqNum);
    __PRAGMA_NO_REORDER

    /* Set sample sequence number in sFlow Flow Sample Header (word 9) */
    __LOG_FW(("Set sample sequence number in sFlow Flow Sample Header (word 9)"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 76 , sampleSeqNum );

    /* Increment sample sequence number by 1 and save it in shared memory source port table */
    __LOG_FW(("Increment sample sequence number by 1 and save it in shared memory source port table"));
    PPA_FW_SP_WRITE( smemSrcPortSampleSeqTableEntryAddr , sampleSeqNum + 1 );
    __LOG_PARAM_FW(sampleSeqNum + 1);

    /* Word10: Source ID Type + Index. Set to LocalDevSrcPort. */
    __LOG_FW(("Word10: Source ID Type + Index. Set to LocalDevSrcPort."));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_sflow_flow_sample_header__source_id_type_PLUS22_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word11: Sampling Rate. Set to EPCL Metadata[31:0] */
    __LOG_FW(("Word11: Sampling Rate. Set to EPCL Metadata[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR57_SFLOW_V5_IPv4_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_sflow_flow_sample_header__sampling_rate_OFFSET, 0);

    /* Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops */
    __LOG_FW(("Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 88 , sampleSeqNum );

    /* Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port] */
    __LOG_FW(("Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port]"));

    /* Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number */
    __LOG_FW(("Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number"));
    smemSrcPortDropsTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR + smemSrcPortIndx*2;
    __LOG_PARAM_FW(smemSrcPortDropsTableEntryAddr);

    /* Read Drops counter number from shared memory */
    __LOG_FW(("Read Drops counter number from shared memory"));
    dropsNum = PPA_FW_SP_SHORT_READ(smemSrcPortDropsTableEntryAddr);
    __LOG_PARAM_FW(dropsNum);

    /* Check if packet Hard/Soft drop */
    __LOG_FW(("Check if packet Hard/Soft drop"));
    __LOG_PARAM_FW(thr57_desc_ptr->phal2ppa.egress_packet_cmd);
    if( (thr57_desc_ptr->phal2ppa.egress_packet_cmd == 3/*HARD DROP*/) || (thr57_desc_ptr->phal2ppa.egress_packet_cmd == 4/*SOFT DROP*/))
    {
        /* Packet is dropped therefore increment Drops number by 1 */
        __LOG_FW(("Packet is dropped therefore increment Drops number by 1"));
        dropsNum +=1; 
 
        /* Save it in shared memory */
        __LOG_FW(("Save it in shared memory"));
        PPA_FW_SP_SHORT_WRITE(smemSrcPortDropsTableEntryAddr, dropsNum);
    }
    /* Set Drops number in output packet in sFlow Flow Sample Header.drops (word 13) */
    __LOG_FW(("Set sample sequence number in output packet in sFlow Flow Sample Header.drops (word 13)"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 92 , dropsNum );
    __LOG_PARAM_FW(dropsNum);
    
    /* Word14: Input Interface. Set to LocalDevSrcPort */
    __LOG_FW(("Word14: Input Interface. Set to LocalDevSrcPort"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_sflow_flow_sample_header__input_PLUS22_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word15: Output Interface. Set to 0 */
    __LOG_FW(("Word15: Output Interface. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x0_LEN1_TO_PKT_sflow_flow_sample_header__output_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word16: Set Number of records to 0 */
    __LOG_FW(("Word16: Set Number of records to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR57_SFLOW_V5_IPv4_CONST_0x0_LEN1_TO_PKT_sflow_flow_sample_header__number_of_records_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field:                         IPv4 Header<Total Length>= IPv4 Header<Total Length> + 52
      - Calculate IPv4 checksum (incremental update):   New checksum = ~( ~(old checksum) + ~(old Total Length) + (new Total Length) )
      - Update UDP length field:                        UDP<length> = UDP<length> + 52 
      - Update HW that packet was increased by 52B      Desc<fw_bc_modification> = 52
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IP length(+52B), calculate new IPv4 CS, update udp length(+52B) and update HW packet was increased by 52B"));

    /* Sum old IP Total Length */
    __LOG_FW(("Sum old IP Total Length using accelerator cmd"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR57_SFLOW_V5_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Update new IP header total length: IPv4<Total Length> + 52 */
    __LOG_FW(("Update new IP header total length: IPv4<Total Length> + 52"));
    ACCEL_CMD_TRIG(ADD_BITS_THR57_SFLOW_V5_IPv4_CONST_0x34_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Sum new Total Length */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR57_SFLOW_V5_IPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET, 0);
    /* Sum old IPv4 CS */
    __LOG_FW(("Sum old IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_LOAD_OLD_THR57_SFLOW_V5_IPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET, 0);
    __PRAGMA_NO_REORDER
    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR57_SFLOW_V5_IPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 52 */
    __LOG_FW(("Update new UDP length. UDP<length> + 52"));
    ACCEL_CMD_TRIG(ADD_BITS_THR57_SFLOW_V5_IPv4_CONST_0x34_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+52 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+52 bytes)."));
    thr57_desc_ptr->ppa_internal_desc.fw_bc_modification = 52;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(57);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR58_SFLOW_V5_IPv6
 * inparam      None
 * return       None
 * description  sFlow is a technology for monitoring traffic in data networks containing switches and routers. 
 *              It requires ingress/egress mirrored packets to be sent over an IPv6-UDP tunnel to a collector in the network.
 *              Following the UDP header there is SFLOW header which is followed by the mirrored packet as the payload.
 *              The L2-IPv6-UDP tunnel encapsulation is added by the Falcon Tunnel-start in the HA unit.
 *              The SFLOW header is inserted by the PHA firmware after the UDP header.
 * 
 *              cfg template: 4 LSB holds IPv4 agent address rest 12 bytes are reserved 
 *              src & trg entries: Not in use (4B)
 *              hdr in:    | Extension space (64B) | IPv6(40B) | UDP(8B) | TS place holder (16B)| payload |
 *              hdr out:   | Extension space (12B) | IPv6(40B) | UDP(8B) | sFlow header(28B) | Data Format Header (8B) | Flow Sample Header (32B) | payload |
 *              Header Window anchor= Outer Layer 3
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmwares actions:
 *              - Adds sFlow header + Data Format Header + Flow Sample Header and sets it according to architect's definitions.  
 *              - Updates IPv6 length to + 52 bytes
 *              - Updates UDP length to + 52 bytes  
 *              - Updates HW that 52 bytes were added 
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16)))
void PPA_FW(THR58_SFLOW_V5_IPv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    ts_sec;                             /* timestamp in seconds */
    uint32_t    ts_nano_sec;                        /* timestamp in nano seconds */
    uint32_t    ts_sec_new;                         /* holds timestamp in seconds after reading it for the second time to check wraparound */  
    uint32_t    ts_sec_to_msec;                     /* holds seconds value in milliseconds units */
    uint32_t    ts_nano_sec_to_msec;                /* holds nano seconds value in milliseconds units */
    uint32_t    uptime;                             /* holds uptime which is the time (in ms) since system boot */ 
    uint8_t     tai_sel;                            /* TAI select index 0 or 1 */
    uint32_t    sampleSeqNum;                       /* holds sample Sequence Number taken from shared memory */
    uint16_t    dropsNum;                           /* holds drops number taken from shared memory */
    uint32_t    smemSrcPortSampleSeqTableEntryAddr; /* holds address of specific entry in src port sample sequence number table in shared memory */
    uint32_t    smemSrcPortDropsTableEntryAddr;     /* holds address of specific entry in src port drops table in shared memory */
    uint32_t    smemSrcPortIndx;                    /* holds index (0:127) to src port table in shared memory */ 

    /* Get pointer to descriptor */
    struct ppa_in_desc* thr58_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Get ptp TAI IF select */
    __LOG_FW(("Get ptp TAI IF select"));
    tai_sel = thr58_desc_ptr->phal2ppa.ptp_tai_select;
    __LOG_PARAM_FW(tai_sel);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Shift packet left to make room for sFlow headers
      ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move left first 32B of IPv6 header by 52 bytes to make room for sFlow headers */
    __LOG_FW(("Move left first 32B of IPv6 header by 52 bytes to make room for sFlow headers"));
    ACCEL_CMD_TRIG(SHIFTLEFT_52_BYTES_THR58_SFLOW_V5_IPv6_LEN32_FROM_PKT_IPv6_Header__version_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Move left last 8B of IPv6 header + UDP(8B) + TS place holder (16B) by 52 bytes to make room for sFlow headers */
    __LOG_FW(("Move left last 8B of IPv6 header + UDP(8B) + TS place holder (16B) by 52 bytes to make room for sFlow headers"));
    ACCEL_CMD_TRIG(SHIFTLEFT_52_BYTES_THR58_SFLOW_V5_IPv6_LEN32_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow header (28B). 7 words each of 32 bits.
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow header. 7 words each of 32 bits."));

    /* Word0: sFlow Version. Set to fixed value 0x5 */                                                    
    __LOG_FW(("Word0: sFlow Version. Set to fixed value 0x5"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x5_LEN3_TO_PKT_sflow_ipv4_header__version_PLUS29_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word1: IP version. Set to fixed value 0x1 (IPv4) */                                                    
    __LOG_FW(("Word1: IP version. Set to fixed value 0x1 (IPv4)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__agent_ip_version_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word2: Agent IPv4 Address. Set to template[31:0] */
    __LOG_FW(("Word2: Agent IPv4 Address. Set to template[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR58_SFLOW_V5_IPv6_CFG_sflow_ipv4_template__agent_ip_address_LEN4_TO_PKT_sflow_ipv4_header__agent_ip_address_OFFSET, 0);

    /* Word3: Sub Agent ID. Set to 0 */
    __LOG_FW(("Word3: Sub Agent ID. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x0_LEN1_TO_PKT_sflow_ipv4_header__sub_agent_id_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /* Word4: Sequence number. Set to Desc<LM Counter[31:0]> (sequence per target analyzer) */
    __LOG_FW(("Word4: Sequence number. Set to Desc<LM Counter[31:0]> (sequence per target analyzer)"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR58_SFLOW_V5_IPv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_sflow_ipv4_header__sequence_number_OFFSET, 0);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns.
      - The uptime is the time (in ms) since system boot. 
      - In our case we have 18 bits of seconds and 30 bits of ns that is ~3 days
      - Since 18b for seconds we assume that multiplication of 1000 to convert to milliseconds should not be bigger than 32b size
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Word5: Uptime in milliseconds. Set according to TAI.sec and TAI.ns."));

    /* Read TOD word 1 which represents seconds field [31:0] */
    __LOG_FW(("Read TOD word 1 which represents seconds field [31:0]"));
    ts_sec = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec);
    /* Read TOD word 0 which represents nano seconds field */
    __LOG_FW(("Read TOD word 0 which represents nano seconds field"));
    ts_nano_sec = READ_TOD_IF_REG(TOD_WORD_0, tai_sel);
    __LOG_PARAM_FW(ts_nano_sec);
    __PRAGMA_NO_REORDER

    /* Check for wraparound */
    __LOG_FW(("Read again seconds (TOD word 1) to check for wraparound"));
    ts_sec_new = READ_TOD_IF_REG(TOD_WORD_1, tai_sel);
    __LOG_PARAM_FW(ts_sec_new);

    /* < 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound */
    __LOG_FW(("< 500000000 (half a second) indicates that the nanosecond value is small, and there was a recent wraparound"));
    if ( (ts_sec_new != ts_sec) && (ts_nano_sec < 500000000) )
    {
        __LOG_FW(("Update ts_sec with new value since wraparound occurred"));
        ts_sec = ts_sec_new ;
        __LOG_PARAM_FW(ts_sec);
    }

    /* Convert seconds to milliseconds units */
    __LOG_FW(("Convert seconds to milliseconds units"));
    ts_sec_to_msec = ts_sec * 1000 ;

    /* Convert nano seconds to milliseconds units */
    __LOG_FW(("Convert nano seconds to milliseconds units"));
    ts_nano_sec_to_msec = ts_nano_sec / 1000000 ;

    /* Calculate uptime in milliseconds */
    __LOG_FW(("Calculate uptime in milliseconds"));
    uptime = ts_sec_to_msec + ts_nano_sec_to_msec ;

    /* Set uptime in sFlow header */
    __LOG_FW(("Set uptime in sFlow header"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 80 , uptime );

    /* Word6: Number of samples. Set to fixed value 1 */                                                    
    __LOG_FW(("Word6: Number of samples. Set to fixed value 1"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x1_LEN1_TO_PKT_sflow_ipv4_header__samples_number_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits.     
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set Sample Data with sFlow Data Format Header (4B) and Sample Length. 2 words each of 32 bits."));

    /* Word7: Data Format. Set to template[63:32] */                                                    
    __LOG_FW(("Word7: Data Format. Set to template[63:32]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR58_SFLOW_V5_IPv6_CFG_sflow_ipv4_template__data_format_header_LEN4_TO_PKT_sflow_data_format_header__smi_private_enterprise_code_OFFSET, 0);

    /* Word8: Set Sample Length to 32 (decimal) */                                                    
    __LOG_FW(("Word8: Set Sample Length to 32 (decimal)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x20_LEN8_TO_PKT_sflow_data_format_header__sample_length_PLUS24_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Set sFlow Flow Sample Header (32B). 8 words each of 32 bits.      
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set sFlow Flow Sample Header (32B). 8 words each of 32 bits."));

    /* Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0] */                                                    
    __LOG_FW(("Word9: Sample Sequence Number. Set to SharedMem.SrcPort[31:0]. Increment SharedMem.SrcPort[31:0]"));

    /* Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127. */
    __LOG_FW(("Get index to shared memory sample sequence source port table. Get it from Desc.local_dev_src_port. Applicable values are 0:127."));
    smemSrcPortIndx = (thr58_desc_ptr->phal2ppa.local_dev_src_port & 0x7F);
    __LOG_PARAM_FW(thr58_desc_ptr->phal2ppa.local_dev_src_port);
    __LOG_PARAM_FW(smemSrcPortIndx);

    /* Get shared memory address compatible to packet's source port sample sequence */
    __LOG_FW(("Get shared memory address compatible to packet's source port sample sequence"));
    smemSrcPortSampleSeqTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR + smemSrcPortIndx*4;
    __LOG_PARAM_FW(smemSrcPortSampleSeqTableEntryAddr);

    /* Read sample sequence number from shared memory */
    __LOG_FW(("Read sample sequence number from shared memory"));
    sampleSeqNum = PPA_FW_SP_READ(smemSrcPortSampleSeqTableEntryAddr);
    __LOG_PARAM_FW(sampleSeqNum);
    __PRAGMA_NO_REORDER

    /* Set sample sequence number in sFlow Flow Sample Header (word 9) */
    __LOG_FW(("Set sample sequence number in sFlow Flow Sample Header (word 9)"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 96 , sampleSeqNum );

    /* Increment sample sequence number by 1 and save it in shared memory source port table */
    __LOG_FW(("Increment sample sequence number by 1 and save it in shared memory source port table"));
    PPA_FW_SP_WRITE( smemSrcPortSampleSeqTableEntryAddr , sampleSeqNum + 1 );
    __LOG_PARAM_FW(sampleSeqNum + 1);

    /* Word10: Source ID Type + Index. Set to LocalDevSrcPort.*/
    __LOG_FW(("Word10: Source ID Type + Index. Set to LocalDevSrcPort."));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_sflow_flow_sample_header__source_id_type_PLUS22_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word11: Sampling Rate. Set to EPCL Metadata[31:0] */
    __LOG_FW(("Word11: Sampling Rate. Set to EPCL Metadata[31:0]"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR58_SFLOW_V5_IPv6_DESC_phal2ppa__pha_metadata_LEN4_TO_PKT_sflow_flow_sample_header__sampling_rate_OFFSET, 0);

    /* Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops */
    __LOG_FW(("Word12: Sampling Pool. Set to SharedMem.SrcPort[31:0]. Same as sample sequence number, includes drops"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 108 , sampleSeqNum );

    /* Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port] */
    __LOG_FW(("Word13: Drops. If <egress packet command> == HARD/SOFT DROP then Increment SharedMem.DropTable[Local Dev Src Port]"));

    /* Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number */
    __LOG_FW(("Get shared memory address of drops counter compatible to packet's source port. Use the same index for sequence number"));
    smemSrcPortDropsTableEntryAddr = DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR + smemSrcPortIndx*2;
    __LOG_PARAM_FW(smemSrcPortDropsTableEntryAddr);

    /* Read Drops counter number from shared memory */
    __LOG_FW(("Read Drops counter number from shared memory"));
    dropsNum = PPA_FW_SP_SHORT_READ(smemSrcPortDropsTableEntryAddr);
    __LOG_PARAM_FW(dropsNum);

    /* Check if packet Hard/Soft drop */
    __LOG_FW(("Check if packet Hard/Soft drop"));
    __LOG_PARAM_FW(thr58_desc_ptr->phal2ppa.egress_packet_cmd);
    if( (thr58_desc_ptr->phal2ppa.egress_packet_cmd == 3/*HARD DROP*/) || (thr58_desc_ptr->phal2ppa.egress_packet_cmd == 4/*SOFT DROP*/))
    {
        /* Packet is dropped therefore increment Drops number by 1 */
        __LOG_FW(("Packet is dropped therefore increment Drops number by 1"));
        dropsNum +=1; 
 
        /* Save it in shared memory */
        __LOG_FW(("Save it in shared memory"));
        PPA_FW_SP_SHORT_WRITE(smemSrcPortDropsTableEntryAddr, dropsNum);
    }
    /* Set Drops number in output packet in sFlow Flow Sample Header.drops (word 13) */
    __LOG_FW(("Set sample sequence number in output packet in sFlow Flow Sample Header.drops (word 13)"));
    PPA_FW_SP_WRITE( FALCON_PKT_REGs_lo + 112 , dropsNum );
    __LOG_PARAM_FW(dropsNum);
    
    /* Word14: Input Interface. Set to LocalDevSrcPort */
    __LOG_FW(("Word14: Input Interface. Set to LocalDevSrcPort"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_DESC_phal2ppa__local_dev_src_port_LEN10_TO_PKT_sflow_flow_sample_header__input_PLUS22_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word15: Output Interface. Set to 0 */
    __LOG_FW(("Word15: Output Interface. Set to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x0_LEN1_TO_PKT_sflow_flow_sample_header__output_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);

    /* Word16: Set Number of records to 0 */
    __LOG_FW(("Word16: Set Number of records to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR58_SFLOW_V5_IPv6_CONST_0x0_LEN1_TO_PKT_sflow_flow_sample_header__number_of_records_PLUS31_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*------------------------------------------------------------------------------------------------------------------------------------------------
      - Update IP length field:                         IPv6 Header<Payload Length>= IPv6 Header<Payload Length> + 52
      - Update UDP length field:                        UDP<length> = UDP<length> + 52 
      - Update HW that packet was increased by 52B      Desc<fw_bc_modification> = 52
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IP length(+52B), update udp length(+52B) and update HW packet was increased by 52B"));

    /* Update new IPv6 header payload length: IPv6<Payload Length> + 52 */
    __LOG_FW(("Update new IPv6 header payload length: IPv6<Payload Length> + 52"));
    ACCEL_CMD_TRIG(ADD_BITS_THR58_SFLOW_V5_IPv6_CONST_0x34_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);

    /* Update new UDP length. UDP<length> + 52 */
    __LOG_FW(("Update new UDP length. UDP<length> + 52"));
    ACCEL_CMD_TRIG(ADD_BITS_THR58_SFLOW_V5_IPv6_CONST_0x34_LEN16_TO_PKT_udp_header__Length_OFFSET, 0);

    /* Indicates HW about the new starting point of the packet header (+52 bytes). */
    __LOG_FW(("Indicates HW about the new starting point of the packet header (+52 bytes)."));
    thr58_desc_ptr->ppa_internal_desc.fw_bc_modification = 52;


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(58);
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
    struct ppa_in_desc*  thr60_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

    /* Set Desc<egress_packet_cmd> to 0x3 (Hard Drop) */    
    __LOG_FW(("Set Desc<egress_packet_cmd> to 0x3 (Hard Drop)"));
    thr60_desc_ptr->phal2ppa.egress_packet_cmd = 0x3;

    /* Set Desc<CPU code> to 183 */    
    __LOG_FW(("Set Desc<CPU code> to 183"));
    thr60_desc_ptr->phal2ppa.cpu_code = 183;

#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(60);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR64_Erspan_TypeII_SameDevMirroring_Ipv4
 * inparam      None
 * return       None
 * description  ERSPAN over IPv4 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN) 
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv4 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - Support handling vlan tagged or untagged
 * 
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries: PHA Target port table holds original packet target physical port for Egress mirroring (2B)  
 *                                 PHA Source port table holds original packet source physical port for Ingress mirroring (2B)
 *
 *              hdr in:  | Extension space (64B) | payload |
 *              hdr out: | Extension space (14B or 10B) | MAC addresses(12B) | VLAN(4B) or not | ET(2B) | IPv4(20B) | GRE(8B) | ERSPAN Type II(8B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Add IPv4 and L2 headers which are copied from shared memory 
 *              - Add GRE & ERSPAN Type II and set them according to Architect's definitions
 *              - Set IPv4 length and calculate IPv4 cs. 
 *              - Update Desc<egress_byte_count> +=54(with vlan) or 50(without vlan) 
 *              - Update Desc<fw_bc_modification> +=54(with vlan) or 50(without vlan)  
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16))) 
void PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    sessionId;                      /* holds Ingress or Egress session ID (10 bits)                   */ 
    uint32_t    smemAnalyzerTemplateBaseAddr;   /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;      /* holds base address of specific sFlow template in shared memory */
    uint32_t    descRxSniff;                    /* holds value of Desc<rx_sniff> field                            */
    uint32_t    vlanVid;                        /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr64_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);


    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr64_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr64_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr64_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 10 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV4_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 20B of IPv4 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 20B of IPv4 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV4_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);

     /* Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field */
     __LOG_FW(("Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field"));
     sessionId = (sessionId >>(10*descRxSniff)) & 0x3FF;
     __LOG_PARAM_FW(sessionId);
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x0800 (IPv4)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x0800 (IPv4)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x08_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Set GRE.Protocol to 0x88BE */
    __LOG_FW(("Set GRE.Protocol to 0x88BE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Set GRE.Sequence Number to Desc<LM Counter> */
    __LOG_FW(("Set GRE.Sequence Number to Desc<LM Counter>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

    /* Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>).
       Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero */
    __LOG_FW(("Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>)."
              "Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , sessionId ); 

    /* Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero.
       if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset) */
    __LOG_FW(("Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero."
              "if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_srcTrgPortEntry__targetPortEntry_LEN16_TO_PKT_ERSPAN_type_II_header__Index_PLUS4_OFFSET + 4*descRxSniff, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS to Desc<QoS Mapped UP> */
    __LOG_FW(("Set ERSPAN.COS to Desc<QoS Mapped UP>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__qos_mapped_up_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Set ERSPAN.En to 3(2b) and ERSPAN.T(1b) to 0 */
    __LOG_FW(("Assign ERSPAN.En to 3(2b) and ERSPAN.T(1b) to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv4 length field 
     - Calculate IPv4 checksum 
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv4 length field + calculate IPv4 checksum"));


    /* Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+36 */
    __LOG_FW(("Set IPv4 Header<Total Length>= Desc<Egress Byte Count>+36"));

    /* First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv4<Total length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv4.Total Length(16b)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv4_Header__total_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv4 total Length +=36 */
    __LOG_FW(("IPv4 total Length +=36"));
    ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x24_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Compute IPv4 CS */
    __LOG_FW(("Compute IPv4 CS"));

    /* Sum first 8 bytes of IPv4 */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__version_OFFSET, 0);

    /* Skip cs field and add to sum 2B of ttl and protocol */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN2_FROM_PKT_IPv4_Header__ttl_OFFSET, 0);

    /* Sum the last 8 bytes of IPv4 (src and dest IP addresses fields) */
    __LOG_FW(("Sum new Total Length"));
    ACCEL_CMD_TRIG(CSUM_LOAD_NEW_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_LEN8_FROM_PKT_IPv4_Header__sip_high_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* Store the new IPv4 CS */
    __LOG_FW(("Store the new IPv4 CS"));
    ACCEL_CMD_TRIG(CSUM_STORE_IP_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_TO_PKT_IPv4_Header__header_checksum_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=50 or 54(with vlan)                         
     - Update Desc<fw_bc_modification> +=50 or 54(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with or without vlan tag, Update Desc<Egress Byte Count> +=50, Update Desc<fw_bc_modification> +=50"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));    
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+50 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+50 bytes)"));
        thr64_desc_ptr->ppa_internal_desc.fw_bc_modification = 50;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv4_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=50 */
        __LOG_FW(("Desc<Egress Byte Count> +=50"));
        ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x32_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+54 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+54 bytes)"));
        thr64_desc_ptr->ppa_internal_desc.fw_bc_modification = 54;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv4_Header__version_MINUS18_OFFSET, 0);
 
        /* Desc<Egress Byte Count> +=54 */
        __LOG_FW(("Desc<Egress Byte Count> +=54"));
        ACCEL_CMD_TRIG(ADD_BITS_THR64_Erspan_TypeII_SameDevMirroring_Ipv4_CONST_0x36_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(64);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


/*************************************************************************************************************************************//**
 * funcname     THR65_Erspan_TypeII_SameDevMirroring_Ipv6
 * inparam      None
 * return       None
 * description  ERSPAN over IPv6 tunnel. Encapsulated Remote Switch Port Analyzer (ERSPAN) 
 *              allows packets to be mirrored to a remote analyzer over a tunnel encapsulation. 
 *              - ERSPAN Type II is used
 *              - ERSPAN is sent over an IPv6 GRE tunnel
 *              - Support handling of Ingress and Egress mirroring   
 *              - Support handling vlan tagged or untagged
 *              NOTE: since in case of IPv6 tunneling need to add 74 bytes and PHA fw can only add maximum 64 bytes 
 *                    HA adds dummy RSPAN (4B) and dummy E-Tag(8B) to be used as place holder when packet arrives to PHA fw.
 *                    PHA fw will then overrun this area with GRE(4LSB) & ERSPAN II headers.    
 * 
 *              cfg template:    Not in use (16B). It is used by fw to hold L2 parameters copied from shared memory.
 *              src & trg entries: PHA Target port table holds original packet target physical port for Egress mirroring (2B)  
 *                                 PHA Source port table holds original packet source physical port for Ingress mirroring (2B)
 *
 *              hdr in:  | Extension space (64B) | origin MAC(12B) | dummy RSPAN tag(4B) | dummy E-Tag (8B) | payload |
 *              hdr out: | Extension space (6B or 2B) | MAC(12B) | VLAN(4B) or not | ET(2B) | IPv6(40B) | GRE(8B) | ERSPAN Type II(8B) | origin MAC(12B) | payload |
 *              Header Window anchor= Outer Layer 2
 *              Header Window size= 96 bytes + 64 bytes Header increment space
 *
 *              Firmware adds the tunnel and sets the values according to architect's definitions.
 *              - Copy origin MAC into dummy E-tag & RSPAN  
 *              - Add IPv6 and L2 headers which are copied from shared memory 
 *              - Add GRE & ERSPAN Type II and set them according to Architect's definitions
 *              - Set IPv6 payload length  
 *              - Update Desc<egress_byte_count> +=62(with vlan) or 58(without vlan) 
 *              - Update Desc<fw_bc_modification> +=62(with vlan) or 58(without vlan)  
 *****************************************************************************************************************************************/
__attribute__ ((aligned (16))) 
void PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)() {
    /* xt_iss_client_command("profile","enable");             */
    /* xt_iss_client_command("profile","disable"); _exit(0);  */

    uint32_t    sessionId;                     /* holds Ingress or Egress session ID (10 bits)                   */
    uint32_t    smemAnalyzerTemplateBaseAddr;  /* holds base address of sFlow templates in shared memory         */
    uint32_t    smemAnalayzerTemplateOfst;     /* holds base address of specific sFlow template in shared memory */
    uint32_t    descRxSniff;                   /* holds value of Desc<rx_sniff> field                            */
    uint32_t    vlanVid;                       /* holds value of vlan vid taken from shared memory template      */


    /* Get pointer to descriptor */
    struct ppa_in_desc* thr65_desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

#ifdef ASIC_SIMULATION
    /* Print out the input packet */
    __LOG_FW(("Print out the input packet"));
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 4) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 8) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 12) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 16) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 20) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 24) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 28) );
    __LOG_PARAM_FW( PPA_FW_SP_READ(FALCON_PKT_REGs_lo + 64 + 32) );
#endif /* ASIC_SIMULATION */

    /* Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring */
    __LOG_FW(("Get Rx Sniff value from descriptor to tell us if Ingress mirroring or Egress mirroring"));
    descRxSniff = thr65_desc_ptr->phal2ppa.rx_sniff;
    __LOG_PARAM_FW(descRxSniff);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index 
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Calculate offset to the right Analyzer ePort template in shared memory according to shared memory index"));

    /* Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6. */
    __LOG_FW(("Desc.pha_metadata[0:2] contains shared memory index. Applicable values are 0:6."));
    smemAnalayzerTemplateOfst = (thr65_desc_ptr->phal2ppa.pha_metadata & 0x7)*64;
    __LOG_PARAM_FW(thr65_desc_ptr->phal2ppa.pha_metadata);
    __LOG_PARAM_FW(smemAnalayzerTemplateOfst);

    /* Get base address of the required Analyzer ePort template in shared memory */
    __LOG_FW(("Get base address of the required Analyzer ePort template in shared memory"));
    smemAnalyzerTemplateBaseAddr = DRAM_CC_ERSPAN_ANALYZER_EPORT_1_TEMPLATE_ADDR + smemAnalayzerTemplateOfst;
    __LOG_PARAM_FW(smemAnalyzerTemplateBaseAddr);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template
      - To avoid 32 bits alignment issues (packet starts after 2 bytes since Ethertype included), copy it first to CFG template and then to packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 16B of L2 parameters from shared memory: | mac da sa(12B) | vlan tag(4B) | and placed them in thread's template"));

    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 0) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 4)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 4) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 8)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 8) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 12)  );
    PPA_FW_SP_WRITE( (FALCON_CFG_REGs_lo + 12) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE + 16)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + IPV6_BASIC_HEADER_SIZE + 16));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get 40B of IPv6 header from shared memory and place them in output packet
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Get 40B of IPv6 header from shared memory and place them in output packet"));

    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 20) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  0)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 24) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  4)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 28) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  8)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 32) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 36) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 40) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 44) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 48) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 52) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32)  );
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 56) , PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36)  );
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 0));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 4));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 8));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 12));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 16));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 20));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 24));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 28));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 32));
    __LOG_PARAM_FW(PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr + 36));


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
      - Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |
      ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Get session ID from shared memory. This address contains  | reserved(12b) | Ingress session ID(10b) | Egress session ID(10b) |"));

     sessionId = PPA_FW_SP_READ(smemAnalyzerTemplateBaseAddr +  IPV6_BASIC_HEADER_SIZE) ;
     __LOG_PARAM_FW(sessionId);

     /* Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field */
     __LOG_FW(("Extract 10 bits of Ingress or Egress session ID according to Desc<rx_sniff> field"));
     sessionId = (sessionId >>(10*descRxSniff)) & 0x3FF;
     __LOG_PARAM_FW(sessionId);
     __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Get VLAN vid
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Get vlan vid from template */
    __LOG_FW(("Get vlan vid from template"));
    vlanVid = PPA_FW_SP_READ(FALCON_CFG_REGs_lo + 12) & 0xFFF;
    __LOG_PARAM_FW(vlanVid);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Move origin MAC addresses into dummy RSPAN & E-Tag section
     ------------------------------------------------------------------------------------------------------------------------------------------------*/

    /* Move right origin MAC addresses by 12 bytes and overrun dummy RSPAN & E-tag */
    __LOG_FW(("Move right origin MAC addresses by 12 bytes and overrun dummy RSPAN & E-tag"));
    ACCEL_CMD_TRIG(SHIFTRIGHT_12_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_LEN12_FROM_PKT_mac_header__mac_da_47_32_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set Ether Type to 0x86dd (IPv6)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set Ether Type to 0x86dd (IPv6)"));

    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x86_LEN8_TO_PKT_ethertype_header__ethertype_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0xDD_LEN8_TO_PKT_ethertype_header__ethertype_PLUS8_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet GRE header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet GRE header (8B)"));

    /* Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value */
    __LOG_FW(("Set GRE.flags to 0x1000. Clear all other bits in range of 32 bits to zero to get 0x1000 value"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x10_LEN8_TO_PKT_GRE_header__C_OFFSET, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Set GRE.Protocol to 0x88BE */
    __LOG_FW(("Set GRE.Protocol to 0x88BE"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x88_LEN8_TO_PKT_GRE_header__NP_OFFSET, 0);
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0xBE_LEN8_TO_PKT_GRE_header__NP_PLUS8_OFFSET, 0);

    /* Set GRE.Sequence Number to Desc<LM Counter> */
    __LOG_FW(("Set GRE.Sequence Number to Desc<LM Counter>"));
    ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__lm_counter_LEN4_TO_PKT_GRE_header__Seq_num_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set packet ERSPAN header (8B)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
     __LOG_FW(("Set packet ERSPAN header (8B)"));

    /* Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>).
       Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero */
    __LOG_FW(("Set Ingress or Egress ERSPAN header.sessionID (extracted earlier according to Desc<rx_sniff>)."
              "Write it as 4B so rest fields (Ver,VLAN,COS,En,T) are cleared to zero"));
    PPA_FW_SP_WRITE( (FALCON_PKT_REGs_lo + 68) , sessionId ); 

    /* Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero.
       if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset) */
    __LOG_FW(("Set 20  bits of ERSPAN.index with PHA target/Source Physical Port bits [15:0], rest 4 bits are cleared to zero."
              "if descRxSniff = 0 trigger target port accelerator if 1 trigger source port accelerator (next command 4B offset)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_srcTrgPortEntry__targetPortEntry_LEN16_TO_PKT_ERSPAN_type_II_header__Index_PLUS4_OFFSET + 4*descRxSniff, SET_REMAIN_FIELDS_TO_ZERO);
    __PRAGMA_NO_REORDER

    /* Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) */
    __LOG_FW(("Set ERSPAN.Ver to 1(4b) and ERSPAN.VLAN(12b) to 0 (cleared already when sessionID field was set) "));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x1_LEN4_TO_PKT_ERSPAN_type_II_header__Ver_OFFSET, 0);

    /* Set ERSPAN.COS to Desc<QoS Mapped UP> */
    __LOG_FW(("Set ERSPAN.COS to Desc<QoS Mapped UP>"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__qos_mapped_up_LEN3_TO_PKT_ERSPAN_type_II_header__COS_OFFSET, 0);

    /* Set ERSPAN.En to 3(2b) and ERSPAN.T(1b) to 0 */
    __LOG_FW(("Assign ERSPAN.En to 3(2b) and ERSPAN.T(1b) to 0"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x6_LEN3_TO_PKT_ERSPAN_type_II_header__En_OFFSET, 0);


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Update packet IPv6 payload length field                         
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Update IPv6 payload length field"));


    /* Set IPv6 Header<Payload Length> = Desc<Egress Byte Count>+16 */
    __LOG_FW(("Set IPv6 Header<Payload Length>= Desc<Egress Byte Count>+16"));

    /* First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b).
       Assumption 2 MS bits were already cleared by application when setting IPv6<Payload length> field in shared memory */
    __LOG_FW(("First copy Desc.Egress Byte Count(14b) to IPv6.Payload Length(16b)"));
    ACCEL_CMD_TRIG(COPY_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_DESC_phal2ppa__egress_byte_count_LEN14_TO_PKT_IPv6_Header__payload_length_PLUS2_OFFSET, 0);
    __PRAGMA_NO_REORDER

    /* IPv6 payload Length +=16 */
    __LOG_FW(("IPv6 payload Length +=16"));
    ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x10_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET, 0);
    __PRAGMA_NO_REORDER


    /*-----------------------------------------------------------------------------------------------------------------------------------------------
     - Set L2 header: MAC DA SA + vlan tag or no vlan tag
     - Update Desc<Egress Byte Count> +=58 or 62(with vlan)                         
     - Update Desc<fw_bc_modification> +=58 or 62(with vlan)
     ------------------------------------------------------------------------------------------------------------------------------------------------*/
    __LOG_FW(("Set L2 header with or without vlan tag, Update Desc<Egress Byte Count> +=58 or 62(with vlan), Update Desc<fw_bc_modification> +=58 or 62(with vlan)"));

    /* Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan */
    __LOG_FW(("Check if there is vlan or not. If thread's template<vlanVid>=0 then there is no vlan"));    
    if (vlanVid == 0)
    { /* No vlan */

        /* Indicates HW about the new starting point of the packet header (+58 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+58 bytes)"));
        thr65_desc_ptr->ppa_internal_desc.fw_bc_modification = 58;

        /* No vlan, just copy MAC 12 bytes from thread's template */
        __LOG_FW(("No vlan, just copy MAC 12 bytes from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_erspan_template__mac_da_47_32_LEN12_TO_PKT_IPv6_Header__version_MINUS14_OFFSET, 0);

        /* Desc<Egress Byte Count> +=58 */
        __LOG_FW(("Desc<Egress Byte Count> +=58"));
        ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x3A_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }
    else
    {/* With vlan */

        /* Indicates HW about the new starting point of the packet header (+62 bytes) */
        __LOG_FW(("Indicates HW about the new starting point of the packet header (+62 bytes)"));
        thr65_desc_ptr->ppa_internal_desc.fw_bc_modification = 62;

        /* With vlan, copy MAC(12B) + vlan tag(4B) from thread's template */
        __LOG_FW(("With vlan, copy MAC(12B) + vlan tag(4B) from thread's template"));
        ACCEL_CMD_TRIG(COPY_BYTES_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CFG_erspan_template__mac_da_47_32_LEN16_TO_PKT_IPv6_Header__version_MINUS18_OFFSET, 0);
 
        /* Desc<Egress Byte Count> +=62 */
        __LOG_FW(("Desc<Egress Byte Count> +=62"));
        ACCEL_CMD_TRIG(ADD_BITS_THR65_Erspan_TypeII_SameDevMirroring_Ipv6_CONST_0x3E_LEN14_TO_DESC_phal2ppa__egress_byte_count_OFFSET, 0);
    }


#if (CHECK_STACK)
    /* Check if stack was overflowed */
    __LOG_FW(("Check if stack was overflowed"));
    falcon_stack_overflow_checker(65);
#endif

    /* Execute the packet swap sequence to jump to the next Thread */
    __LOG_FW(("Execute the packet swap sequence to jump to the next Thread"));
    packet_swap_sequence();
}


#if RECOVER_FROM_EXCEPTION
/********************************************************************************************************************//**
 * funcname        falconRecoveFromException
 * inparam         None
 * return          None
 * description     Recover from core exception by executing packet swap sequence to send current packet and get the next one.
 *                 fw_drop bit is set in order to indicate that this packet should be dropped.
 *                 NOTE: can use the following instruction to generate an exception for testing => asm volatile ("syscall");
 ************************************************************************************************************************/
__attribute__ ((aligned (16)))
void falconRecoveFromException ()
{
    /* Get pointer to descriptor */
    struct ppa_in_desc* desc_ptr = FALCON_MEM_CAST(FALCON_DESC_REGs_lo,ppa_in_desc);

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

THREAD_TYPE phaThreadsTypeFalconImage02[] = {
    /* 0*/  PPA_FW(THR0_DoNothing)
    /* 1*/ ,PPA_FW(THR1_SRv6_End_Node)
    /* 2*/ ,PPA_FW(THR2_SRv6_Source_Node_1_segment)
    /* 3*/ ,PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)
    /* 4*/ ,PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)
    /* 5*/ ,PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)
    /* 6*/ ,PPA_FW(THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4)
    /* 7*/ ,PPA_FW(THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6)
    /* 8*/ ,PPA_FW(THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4)
    /* 9*/ ,PPA_FW(THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6)
    /*10*/ ,PPA_FW(THR10_Cc_Erspan_TypeII_SrcDevMirroring)
    /*11*/ ,PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)
    /*12*/ ,PPA_FW(THR12_MPLS_SR_NO_EL)
    /*13*/ ,PPA_FW(THR13_MPLS_SR_ONE_EL)
    /*14*/ ,PPA_FW(THR14_MPLS_SR_TWO_EL)
    /*15*/ ,PPA_FW(THR15_MPLS_SR_THREE_EL)
    /*16*/ ,invalidFirmwareThread
    /*17*/ ,invalidFirmwareThread
    /*18*/ ,invalidFirmwareThread
    /*19*/ ,invalidFirmwareThread
    /*20*/ ,invalidFirmwareThread
    /*21*/ ,invalidFirmwareThread
    /*22*/ ,invalidFirmwareThread
    /*23*/ ,invalidFirmwareThread
    /*24*/ ,invalidFirmwareThread
    /*25*/ ,invalidFirmwareThread
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
    /*54*/ ,PPA_FW(THR54_IPv4_TTL_Increment)
    /*55*/ ,PPA_FW(THR55_IPv6_HopLimit_Increment)
    /*56*/ ,PPA_FW(THR56_Clear_Outgoing_Mtag_Cmd)
    /*57*/ ,PPA_FW(THR57_SFLOW_V5_IPv4)
    /*58*/ ,PPA_FW(THR58_SFLOW_V5_IPv6)
    /*59*/ ,invalidFirmwareThread
    /*60*/ ,PPA_FW(THR60_DropAllTraffic)
    /*61*/ ,invalidFirmwareThread
    /*62*/ ,invalidFirmwareThread
    /*63*/ ,invalidFirmwareThread
    /*64*/ ,PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)
    /*65*/ ,PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)
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
