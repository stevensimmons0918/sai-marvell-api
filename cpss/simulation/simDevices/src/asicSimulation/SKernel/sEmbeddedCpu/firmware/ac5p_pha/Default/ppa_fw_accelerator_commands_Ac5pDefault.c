/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * ppa_fw_accelerator_commands_Ac5pDefault.c
 *
 *  PPA fw accelerator commands function
 *
 */

/* ==========================
  Include headers section
  =========================== */
#ifdef ASIC_SIMULATION
#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
    #pragma warning(disable: 4245) /* 'function' : conversion from 'int' to 'uint32_t', signed/unsigned mismatch */
#endif /*_VISUALC*/
#endif /*ASIC_SIMULATION*/

#ifdef ASIC_SIMULATION
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_image_info.h"
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_base_types.h"
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_defs.h"
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_accelerator_commands.h"
#else
    #include "ppa_fw_image_info.h"
    #include "ppa_fw_base_types.h"
    #include "ppa_fw_defs.h"
    #include "ppa_fw_accelerator_commands.h"
#endif /*ASIC_SIMULATION*/

#ifndef ASIC_SIMULATION
/********************************************************************************************************************//**
* funcname        pragma_flush_memory
* inparam         None
* return          None
* description     Activate compiler pragma flush_memory command.
*                 Command to flush memory (~5 cycles) 
 ************************************************************************************************************************/
INLINE void pragma_flush_memory(){  
    #pragma flush_memory 
}
#endif /*!ASIC_SIMULATION*/

/************************************************************************************************************************
 * funcname        load_accelerator_commands
 * inparam         None
 * return          None
 * description     Load accelerator commands into SP memory
 ************************************************************************************************************************/
void PPA_FW(load_accelerator_commands)() {
    ACCEL_CMD_LOAD(COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET,              COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN32_FROM_PKT_IPv6_Header__version_OFFSET,                           SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN32_FROM_PKT_IPv6_Header__version_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET,                     SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_OFFSET, COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_OFFSET, COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_OFFSET,                         COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_OFFSET,               COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_OFFSET,                          COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR2_SRv6_Source_Node_1_segment_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET,      COPY_BYTES_THR2_SRv6_Source_Node_1_segment_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET,                     ADD_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_OFFSET,                        COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,          COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET,         SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET,  SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET,   SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_OFFSET, COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET, COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_OFFSET,        ADD_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_OFFSET,     COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET, SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET, COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET, COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET, ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET, ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET, COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET, COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET, COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET, COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_MPLS_SR_NO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                            COPY_BITS_THR12_MPLS_SR_NO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                     SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                     SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                           COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET,                                    COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET,                             COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                           COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET,             COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET,                                     COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                     COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR13_MPLS_SR_ONE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,         SUB_BITS_THR13_MPLS_SR_ONE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET,          COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_OFFSET, COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                     SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                     SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                    SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                              SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                              SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                             SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                           COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET,                                    COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET,                             COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                           COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET,             COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET,                                     COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                     COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR14_MPLS_SR_TWO_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,         SUB_BITS_THR14_MPLS_SR_TWO_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET,          COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_OFFSET, COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                   SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                   SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET,                                  SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                            SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                            SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                           SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET,                          SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS16_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                         COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET,                                  COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET,                           COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET,                                         COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET,           COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET,                                   COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                   COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR15_MPLS_SR_THREE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,       SUB_BITS_THR15_MPLS_SR_THREE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET,            COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS4_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS8_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS8_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS12_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS16_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS16_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS20_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS20_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS24_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS24_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS28_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS28_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS32_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS36_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS36_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS40_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS40_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS44_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS44_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS48_OFFSET, COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS48_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_OFFSET,                                                SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN32_FROM_PKT_mac_da_47_32_OFFSET,                                                SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN32_FROM_PKT_mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET,                                         SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR16_SGT_NetAddMSB_CFG_sgt_tag_template__ethertype_LEN6_TO_PKT_sgt_tag__ethertype_OFFSET,                    COPY_BYTES_THR16_SGT_NetAddMSB_CFG_sgt_tag_template__ethertype_LEN6_TO_PKT_sgt_tag__ethertype_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_SGT_NetAddMSB_CONST_0x4_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                       COPY_BITS_THR16_SGT_NetAddMSB_CONST_0x4_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR17_SGT_NetFix_CFG_sgt_tag_template__ethertype_LEN4_TO_PKT_sgt_tag__ethertype_OFFSET,                       COPY_BYTES_THR17_SGT_NetFix_CFG_sgt_tag_template__ethertype_LEN4_TO_PKT_sgt_tag__ethertype_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_OFFSET,                                               SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET,                                        SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET,                                               SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN32_FROM_PKT_mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR18_SGT_NetRemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                      COPY_BITS_THR18_SGT_NetRemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_SGT_eDSAFix_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w3__Tag1SrcTagged_OFFSET,                                     COPY_BITS_THR19_SGT_eDSAFix_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w3__Tag1SrcTagged_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN4_FROM_PKT_mac_da_47_32_PLUS32_OFFSET,                                        SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN4_FROM_PKT_mac_da_47_32_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET,                                              SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN32_FROM_PKT_mac_da_47_32_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR20_SGT_eDSARemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                     COPY_BITS_THR20_SGT_eDSARemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR21_SGT_GBPFixIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,                                         COPY_BITS_THR21_SGT_GBPFixIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,        COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET, COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR22_SGT_GBPFixIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,                                         COPY_BITS_THR22_SGT_GBPFixIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,        COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET, COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,                                      COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,     COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET, COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN16_FROM_PKT_version_PLUS32_OFFSET,                                         SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN16_FROM_PKT_version_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN20_FROM_PKT_version_PLUS32_OFFSET,                                         SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN20_FROM_PKT_version_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN32_FROM_PKT_version_OFFSET,                                                SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN32_FROM_PKT_version_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET,                                    CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET,                                SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_IPv4_Header__total_length_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_NEW_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET,                                    CSUM_LOAD_NEW_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET,                                 CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_VALUE);
    ACCEL_CMD_LOAD(CSUM_STORE_IP_THR23_SGT_GBPRemoveIPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET,                                        CSUM_STORE_IP_THR23_SGT_GBPRemoveIPv4_TO_PKT_IPv4_Header__header_checksum_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET,                                       SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_udp_header__Length_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                  COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET,                                      COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET,     COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET, COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN4_FROM_PKT_version_PLUS64_OFFSET,                                          SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN4_FROM_PKT_version_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN8_FROM_PKT_version_PLUS64_OFFSET,                                          SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN8_FROM_PKT_version_PLUS64_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_PLUS32_OFFSET,                                         SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_PLUS32_VALUE);
    ACCEL_CMD_LOAD(SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_OFFSET,                                                SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET,                              SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_IPv6_Header__payload_length_VALUE);
    ACCEL_CMD_LOAD(SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET,                                       SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_udp_header__Length_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET,                  COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_PLUS1_LEN1_TO_CFG_ptp_phy_tag__ingTimeSecLsb_OFFSET,             COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_PLUS1_LEN1_TO_CFG_ptp_phy_tag__ingTimeSecLsb_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__udp_checksum_update_en_LEN1_TO_CFG_ptp_phy_tag__checksumUpdate_OFFSET,     COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__udp_checksum_update_en_LEN1_TO_CFG_ptp_phy_tag__checksumUpdate_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__ptp_tai_select_LEN1_TO_CFG_ptp_phy_tag__tai_sel_PLUS1_OFFSET,              COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__ptp_tai_select_LEN1_TO_CFG_ptp_phy_tag__tai_sel_PLUS1_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_offset_LEN7_TO_CFG_ptp_phy_tag__cf_offset_OFFSET,                COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_offset_LEN7_TO_CFG_ptp_phy_tag__cf_offset_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_LEN1_TO_DESC_phal2ppa__ptp_timestamp_queue_select_OFFSET,      COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_LEN1_TO_DESC_phal2ppa__ptp_timestamp_queue_select_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_PLUS1_LEN10_TO_DESC_phal2ppa__ptp_timestamp_queue_entry_id_OFFSET, COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_PLUS1_LEN10_TO_DESC_phal2ppa__ptp_timestamp_queue_entry_id_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_PTP_Phy_1_Step_CONST_0x3_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET,                                       COPY_BITS_THR25_PTP_Phy_1_Step_CONST_0x3_LEN4_TO_DESC_phal2ppa__ptp_action_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_OFFSET,               COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_PLUS2_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_PLUS2_OFFSET,   COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_PLUS2_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_PLUS2_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_OFFSET,                                                  CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_NEW_THR59_SLS_Test_LEN2_FROM_PKT_IPv4_Header__version_PLUS8_OFFSET,                                            CSUM_LOAD_NEW_THR59_SLS_Test_LEN2_FROM_PKT_IPv4_Header__version_PLUS8_VALUE);
    ACCEL_CMD_LOAD(CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_PLUS12_OFFSET,                                           CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_PLUS12_VALUE);
    ACCEL_CMD_LOAD(CSUM_STORE_IP_THR59_SLS_Test_TO_PKT_IPv4_Header__header_checksum_OFFSET,                                                 CSUM_STORE_IP_THR59_SLS_Test_TO_PKT_IPv4_Header__header_checksum_VALUE);
__PRAGMA_FLUSH_MEMORY

}


#ifdef ASIC_SIMULATION
/************************************************************************************************************************
 * AC5P PHA threads accelerator commands table
 * Includes name, offset and thread ID for each accelerator command
 ************************************************************************************************************************/

#define AC5P_ENUM_NAME_AND_VALUE(var) STR(enum_##var),var
ACCEL_INFO_STC accelInfoArrAc5pDefault[] =
{

/*************** THR1_SRv6_End_Node *******************/
    { AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR1_SRv6_End_Node_OFFSET32_PKT_srv6_seg0__dip0_high_LEN16_TO_PKT_IPv6_Header__dip0_high_OFFSET),             1}

/*************** THR2_SRv6_Source_Node_1_segment *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN32_FROM_PKT_IPv6_Header__version_OFFSET),                          2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR2_SRv6_Source_Node_1_segment_LEN8_FROM_PKT_IPv6_Header__version_PLUS32_OFFSET),                    2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__sip0_high_MINUS24_LEN16_TO_PKT_SRv6_Segment0_Header__dip0_high_OFFSET), 2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR2_SRv6_Source_Node_1_segment_PKT_IPv6_Header__next_header_MINUS24_LEN2_TO_PKT_SRv6_Header__next_header_OFFSET), 2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2_LEN8_TO_PKT_SRv6_Header__hdr_ext_len_OFFSET),                        2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x401_LEN11_TO_PKT_SRv6_Header__routing_type_PLUS5_OFFSET),              2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x0_LEN8_TO_PKT_SRv6_Header__last_entry_OFFSET),                         2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR2_SRv6_Source_Node_1_segment_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET),     2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET),                    2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x2B_LEN8_TO_PKT_IPv6_Header__next_header_OFFSET),                       2}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR2_SRv6_Source_Node_1_segment_CONST_0x18_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),         2}

/*************** THR3_SRv6_Source_Node_First_Pass_2_3_segments *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_OFFSET),        3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN32_FROM_PKT_mac_header__mac_da_47_32_PLUS32_OFFSET), 3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_16_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_LEN6_FROM_PKT_mac_header__mac_da_47_32_PLUS64_OFFSET),  3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_PKT_IPv6_Header__sip0_high_MINUS16_LEN16_TO_PKT_SRv6_Segment2_Header__dip0_high_OFFSET), 3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CFG_ipv6_sip_template__val_0_LEN16_TO_PKT_IPv6_Header__sip0_high_OFFSET), 3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x1_LEN16_TO_PKT_eDSA_fwd_w3__Trg_ePort_15_0_OFFSET),       3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x10_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET), 3}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR3_SRv6_Source_Node_First_Pass_2_3_segments_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w2__IsTrgPhyPortValid_OFFSET),    3}

/*************** THR4_SRv6_Source_Node_Second_Pass_3_segments *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_40_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS40_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x38_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_no_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR4_SRv6_Source_Node_Second_Pass_3_segments_OFFSET8_PKT_with_vlan__Generic_TS_Data__data_31_28_MINUS40_LEN32_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET), 4}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR4_SRv6_Source_Node_Second_Pass_3_segments_CONST_0xF2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET), 4}

/*************** THR5_SRv6_Source_Node_Second_Pass_2_segments *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS32_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN12_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN32_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS76_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_LEN16_FROM_PKT_no_vlan__Generic_TS_Data__mac_da_47_32_PLUS64_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_24_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_LEN32_FROM_PKT_with_vlan__Generic_TS_Data__mac_da_47_32_PLUS80_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_no_vlan__SRv6_Header__last_entry_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_35_32_MINUS24_LEN4_TO_PKT_with_vlan__SRv6_Header__last_entry_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_no_vlan__IPv6_Header__payload_length_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( ADD_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x28_LEN16_TO_PKT_with_vlan__IPv6_Header__payload_length_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_no_vlan__IPv6_Header__next_header_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_CONST_0x2B_LEN8_TO_PKT_with_vlan__IPv6_Header__next_header_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_no_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_no_vlan__SRv6_Segment0_Header__dip0_high_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_SRv6_Source_Node_Second_Pass_2_segments_OFFSET24_PKT_with_vlan__Generic_TS_Data__data_15_12_MINUS24_LEN16_TO_PKT_with_vlan__SRv6_Segment0_Header__dip0_high_OFFSET), 5}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR5_SRv6_Source_Node_Second_Pass_2_segments_CONST_0xE2_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET), 5}

/*************** THR12_MPLS_SR_NO_EL *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR12_MPLS_SR_NO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                           12}

/*************** THR13_MPLS_SR_ONE_EL *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                    13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                    13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR13_MPLS_SR_ONE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                          13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET),                                   13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET),                            13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                          13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET),            13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET),                                    13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR13_MPLS_SR_ONE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                    13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR13_MPLS_SR_ONE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),        13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET),         13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_OFFSET), 13}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_MPLS_SR_ONE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_OFFSET), 13}

/*************** THR14_MPLS_SR_TWO_EL *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                    14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                    14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                             14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                             14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR14_MPLS_SR_TWO_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                          14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET),                                   14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET),                            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                          14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET),            14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET),                                    14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR14_MPLS_SR_TWO_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                    14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR14_MPLS_SR_TWO_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),        14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_expansion_space__reserved_8_OFFSET),         14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_expansion_space__reserved_8_PLUS4_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_expansion_space__reserved_8_PLUS8_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_expansion_space__reserved_8_PLUS12_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_expansion_space__reserved_8_PLUS16_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_expansion_space__reserved_8_PLUS20_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_expansion_space__reserved_8_PLUS24_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_expansion_space__reserved_8_PLUS28_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_expansion_space__reserved_8_PLUS32_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_expansion_space__reserved_8_PLUS36_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_expansion_space__reserved_8_PLUS40_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_expansion_space__reserved_8_PLUS44_OFFSET), 14}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_MPLS_SR_TWO_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_expansion_space__reserved_8_PLUS48_OFFSET), 14}

/*************** THR15_MPLS_SR_THREE_EL *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                  15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                  15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                           15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                           15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS8_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN4_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN8_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN12_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN16_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN20_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN24_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN28_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_8_BYTES_THR15_MPLS_SR_THREE_EL_LEN32_FROM_PKT_MPLS_label_1__label_val_MINUS16_OFFSET),                         15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                        15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN10_TO_PKT_MPLS_label_8__label_val_OFFSET),                                 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x7_LEN10_TO_PKT_MPLS_label_8__label_val_PLUS10_OFFSET),                          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x0_LEN1_TO_PKT_MPLS_label_8__BoS_OFFSET),                                        15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_DESC_phal2ppa__packet_hash_LEN12_TO_PKT_MPLS_label_8__label_val_PLUS8_OFFSET),          15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x1_LEN1_TO_PKT_MPLS_label_8__label_val_OFFSET),                                  15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR15_MPLS_SR_THREE_EL_CONST_0x30_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                  15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR15_MPLS_SR_THREE_EL_PKT_MPLS_data__EL1_ofst_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),      15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_LEN4_TO_PKT_MPLS_label_2__label_val_OFFSET),           15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS4_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS4_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS8_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS8_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS12_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS12_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS16_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS16_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS20_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS20_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS24_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS24_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS28_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS28_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS32_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS32_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS36_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS36_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS40_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS40_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS44_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS44_OFFSET), 15}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_MPLS_SR_THREE_EL_PKT_expansion_space__reserved_7_PLUS48_LEN4_TO_PKT_MPLS_label_2__label_val_PLUS48_OFFSET), 15}

/*************** THR16_SGT_NetAddMSB *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_OFFSET),                                               16}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN32_FROM_PKT_mac_da_47_32_OFFSET),                                               16}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTLEFT_4_BYTES_THR16_SGT_NetAddMSB_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET),                                        16}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR16_SGT_NetAddMSB_CFG_sgt_tag_template__ethertype_LEN6_TO_PKT_sgt_tag__ethertype_OFFSET),                   16}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR16_SGT_NetAddMSB_CONST_0x4_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                      16}

/*************** THR17_SGT_NetFix *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR17_SGT_NetFix_CFG_sgt_tag_template__ethertype_LEN4_TO_PKT_sgt_tag__ethertype_OFFSET),                      17}

/*************** THR18_SGT_NetRemove *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_OFFSET),                                              18}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN12_FROM_PKT_mac_da_47_32_PLUS32_OFFSET),                                       18}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR18_SGT_NetRemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET),                                              18}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR18_SGT_NetRemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                     18}

/*************** THR19_SGT_eDSAFix *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR19_SGT_eDSAFix_CONST_0x0_LEN1_TO_PKT_eDSA_fwd_w3__Tag1SrcTagged_OFFSET),                                    19}

/*************** THR20_SGT_eDSARemove *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN4_FROM_PKT_mac_da_47_32_PLUS32_OFFSET),                                       20}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR20_SGT_eDSARemove_LEN32_FROM_PKT_mac_da_47_32_OFFSET),                                             20}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR20_SGT_eDSARemove_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                    20}

/*************** THR21_SGT_GBPFixIPv4 *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR21_SGT_GBPFixIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET),                                        21}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET),       21}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR21_SGT_GBPFixIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET), 21}

/*************** THR22_SGT_GBPFixIPv6 *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR22_SGT_GBPFixIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET),                                        22}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET),       22}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR22_SGT_GBPFixIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET), 22}

/*************** THR23_SGT_GBPRemoveIPv4 *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET),                                     23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET),    23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR23_SGT_GBPRemoveIPv4_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET), 23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN16_FROM_PKT_version_PLUS32_OFFSET),                                        23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN20_FROM_PKT_version_PLUS32_OFFSET),                                        23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR23_SGT_GBPRemoveIPv4_LEN32_FROM_PKT_version_OFFSET),                                               23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET),                                   23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_IPv4_Header__total_length_OFFSET),                               23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_NEW_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__total_length_OFFSET),                                   23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_OLD_THR23_SGT_GBPRemoveIPv4_LEN2_FROM_PKT_IPv4_Header__header_checksum_OFFSET),                                23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_STORE_IP_THR23_SGT_GBPRemoveIPv4_TO_PKT_IPv4_Header__header_checksum_OFFSET),                                       23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET),                                      23}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR23_SGT_GBPRemoveIPv4_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                 23}

/*************** THR24_SGT_GBPRemoveIPv6 *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x1_LEN1_TO_PKT_vxlan_gbp_header__G_OFFSET),                                     24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata3_LEN4_TO_PKT_vxlan_gbp_header__Group_Policy_ID_OFFSET),    24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR24_SGT_GBPRemoveIPv6_DESC_phal2ppa__pha_metadata2_PLUS4_LEN12_TO_PKT_vxlan_gbp_header__Group_Policy_ID_PLUS4_OFFSET), 24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN4_FROM_PKT_version_PLUS64_OFFSET),                                         24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN8_FROM_PKT_version_PLUS64_OFFSET),                                         24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_PLUS32_OFFSET),                                        24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SHIFTRIGHT_4_BYTES_THR24_SGT_GBPRemoveIPv6_LEN32_FROM_PKT_version_OFFSET),                                               24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_IPv6_Header__payload_length_OFFSET),                             24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( SUB_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0x4_LEN16_TO_PKT_udp_header__Length_OFFSET),                                      24}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR24_SGT_GBPRemoveIPv6_CONST_0xFC_LEN8_TO_DESC_ppa_internal_desc__fw_bc_modification_OFFSET),                 24}

/*************** THR25_PTP_Phy_1_Step *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_PLUS1_LEN1_TO_CFG_ptp_phy_tag__ingTimeSecLsb_OFFSET),            25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__udp_checksum_update_en_LEN1_TO_CFG_ptp_phy_tag__checksumUpdate_OFFSET),    25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__ptp_tai_select_LEN1_TO_CFG_ptp_phy_tag__tai_sel_PLUS1_OFFSET),             25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_DESC_phal2ppa__timestamp_offset_LEN7_TO_CFG_ptp_phy_tag__cf_offset_OFFSET),               25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_LEN1_TO_DESC_phal2ppa__ptp_timestamp_queue_select_OFFSET),     25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_CFG_ptp_phy_tag__cf_offset_PLUS1_LEN10_TO_DESC_phal2ppa__ptp_timestamp_queue_entry_id_OFFSET), 25}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BITS_THR25_PTP_Phy_1_Step_CONST_0x3_LEN4_TO_DESC_phal2ppa__ptp_action_OFFSET),                                      25}

/*************** THR59_SLS_Test *******************/
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_OFFSET),              59}
    ,{ AC5P_ENUM_NAME_AND_VALUE( COPY_BYTES_THR59_SLS_Test_PKT_IPv4_Header__version_PLUS2_LEN2_TO_CFG_HA_Table_reserved_space__reserved_0_PLUS2_OFFSET),  59}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_OFFSET),                                                 59}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_NEW_THR59_SLS_Test_LEN2_FROM_PKT_IPv4_Header__version_PLUS8_OFFSET),                                           59}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_LOAD_NEW_THR59_SLS_Test_LEN8_FROM_PKT_IPv4_Header__version_PLUS12_OFFSET),                                          59}
    ,{ AC5P_ENUM_NAME_AND_VALUE( CSUM_STORE_IP_THR59_SLS_Test_TO_PKT_IPv4_Header__header_checksum_OFFSET),                                                59}
/*************** Last Entry *******************/
    /*must be last*/
    ,{NULL , 0 , 0}
};
#endif /*ASIC_SIMULATION*/

