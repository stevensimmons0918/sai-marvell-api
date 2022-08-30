/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 *  ppa_fw_accelerator_commands.c
 *
 *  PPA fw accelerator commands function
 *
 */

/* ==========================
  Include headers section
  =========================== */
#include "ppa_fw_defs.h"
#include "ppa_fw_base_types.h"
#include "ppa_accelerator_commands.h"



/********************************************************************************************************************//**
 * funcname        load_accelerator_commands
 * inparam         None
 * return          None
 * description     Load accelerator commands into SP memory
 ************************************************************************************************************************/
void load_accelerator_commands () {

    /* Load thread_1 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR1_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET,             COPY_BYTES_THR1_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR1_E2U_CFG_srcPortEntry__PCID_LEN12_TO_CFG_etag__E_CID_base_OFFSET,  COPY_BITS_THR1_E2U_CFG_srcPortEntry__PCID_LEN12_TO_CFG_etag__E_CID_base_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR1_E2U_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,                                             COPY_BITS_THR1_E2U_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR1_E2U_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET,                                                      COPY_BYTES_THR1_E2U_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR1_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_etag__E_PCP_OFFSET,         COPY_BITS_THR1_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR1_E2U_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_etag__IPL_Direction_OFFSET,                                 COPY_BITS_THR1_E2U_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_etag__IPL_Direction_VALUE);
    /* Load thread_2 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_PLUS8_LEN12_TO_MACSHIFTRIGHT_12_OFFSET,     COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_PLUS8_LEN12_TO_MACSHIFTRIGHT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET,            COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_VALUE);
    /* Load thread_3 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR3_U2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET,                      COPY_BYTES_THR3_U2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR3_U2C_CFG_cfgReservedSpace__reserved_0_LEN1_TO_PKT_etag__IPL_Direction_OFFSET,                              COPY_BITS_THR3_U2C_CFG_cfgReservedSpace__reserved_0_LEN1_TO_PKT_etag__IPL_Direction_VALUE);
    /* Load thread_4 commands */
    ACCEL_CMD_LOAD(COPY_BITS_THR4_ET2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET,  COPY_BITS_THR4_ET2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_ET2U_PKT_vlan__vid_LEN12_TO_CFG_DSA_fwd__vid_OFFSET,                                COPY_BITS_THR4_ET2U_PKT_vlan__vid_LEN12_TO_CFG_DSA_fwd__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_ET2U_PKT_vlan__up_LEN3_TO_CFG_DSA_fwd__up_OFFSET,                                   COPY_BITS_THR4_ET2U_PKT_vlan__up_LEN3_TO_CFG_DSA_fwd__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR4_ET2U_PKT_vlan__cfi_LEN1_TO_CFG_DSA_fwd__cfi_OFFSET,                                 COPY_BITS_THR4_ET2U_PKT_vlan__cfi_LEN1_TO_CFG_DSA_fwd__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR4_ET2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET,               COPY_BYTES_THR4_ET2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_VALUE);
    /* Load thread_5 commands */
    ACCEL_CMD_LOAD(COPY_BITS_THR5_EU2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET,  COPY_BITS_THR5_EU2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_EU2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET,                              COPY_BYTES_THR5_EU2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR5_EU2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET,               COPY_BYTES_THR5_EU2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR5_EU2U_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_DSA_fwd__vid_OFFSET,             COPY_BITS_THR5_EU2U_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_DSA_fwd__vid_VALUE);
    /* Load thread_6 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET,                                                    COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__vid_LEN12_TO_CFG_vlan__vid_OFFSET,                                  COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__vid_LEN12_TO_CFG_vlan__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__up_LEN3_TO_CFG_vlan__up_OFFSET,                                     COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__up_LEN3_TO_CFG_vlan__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__cfi_LEN1_TO_CFG_vlan__cfi_OFFSET,                                   COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__cfi_LEN1_TO_CFG_vlan__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR6_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET,                              COPY_BITS_THR6_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR6_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET,                                         COPY_BYTES_THR6_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                                     COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    /* Load thread_7 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR7_Mrr2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                  COPY_BYTES_THR7_Mrr2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR7_Mrr2E_PKT_DSA_from_cpu__Extend_LEN1_TO_DESC_nonQcn_desc__fw_drop_OFFSET,  COPY_BITS_THR7_Mrr2E_PKT_DSA_from_cpu__Extend_LEN1_TO_DESC_nonQcn_desc__fw_drop_VALUE);
    /* Load thread_8 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR8_E_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET,                                      COPY_BYTES_THR8_E_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET,                              COPY_BITS_THR8_E_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET,                                        COPY_BITS_THR8_E_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET,                                      COPY_BITS_THR8_E_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET,  COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET,          COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET,          COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET,           COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET,         COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR8_E_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET,               COPY_BYTES_THR8_E_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_VALUE);
    /* Load thread_9 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR9_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_16_OFFSET,                                        COPY_BYTES_THR9_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_16_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET,    COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET,            COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET,            COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR9_E2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET,             COPY_BITS_THR9_E2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR9_E2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET,           COPY_BITS_THR9_E2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR9_E2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET,                 COPY_BYTES_THR9_E2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_VALUE);
    /* Load thread_10 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR10_C_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET,                                      COPY_BYTES_THR10_C_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET,      COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET,       COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET,      COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET,                             COPY_BITS_THR10_C_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET,                                       COPY_BITS_THR10_C_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET,                                     COPY_BITS_THR10_C_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET,          COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET,        COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR10_C_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET,              COPY_BYTES_THR10_C_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR10_C_V2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET,       COPY_BITS_THR10_C_V2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_VALUE);
    /* Load thread_11 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR11_C2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET,                                        COPY_BYTES_THR11_C2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET,        COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET,         COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET,        COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET,            COPY_BITS_THR11_C2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET,          COPY_BITS_THR11_C2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR11_C2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET,                COPY_BYTES_THR11_C2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR11_C2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET,         COPY_BITS_THR11_C2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_VALUE);
    /* Load thread_12 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET,                                   COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET,                                  COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET,                                COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET,                            COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET,                                      COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET,                                    COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR12_U_UC2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET,                                                  COPY_BYTES_THR12_U_UC2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR12_U_UC2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET,                                                    COPY_BYTES_THR12_U_UC2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                                    COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET,               COPY_BITS_THR12_U_UC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR12_U_UC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,                                         COPY_BITS_THR12_U_UC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_VALUE);
    /* Load thread_13 commands */
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w3__eVIDX_15_12_LEN4_TO_CFG_etag_u_mc2c__eVIDX_full_OFFSET,                  COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w3__eVIDX_15_12_LEN4_TO_CFG_etag_u_mc2c__eVIDX_full_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__eVIDX_11_0_LEN12_TO_CFG_etag_u_mc2c__eVIDX_full_PLUS4_OFFSET,            COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__eVIDX_11_0_LEN12_TO_CFG_etag_u_mc2c__eVIDX_full_PLUS4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag_u_mc2c__E_PCP_OFFSET,                               COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag_u_mc2c__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag_u_mc2c__E_DEI_OFFSET,                             COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag_u_mc2c__E_DEI_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_src_ePort__src_ePort_full_OFFSET,            COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_src_ePort__src_ePort_full_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_src_ePort__src_ePort_full_PLUS10_OFFSET,       COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_src_ePort__src_ePort_full_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_src_ePort__src_ePort_full_PLUS12_OFFSET,       COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_src_ePort__src_ePort_full_PLUS12_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan_eDSA__eVLAN_OFFSET,                         COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan_eDSA__eVLAN_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan_eDSA__up_OFFSET,                                     COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan_eDSA__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan_eDSA__cfi_OFFSET,                                   COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan_eDSA__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_U_MC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                                        COPY_BYTES_THR13_U_MC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_U_MC2C_CFG_etag_u_mc2c__E_PCP_LEN6_TO_PKT_etag_u_mc2c__E_PCP_OFFSET,                                    COPY_BYTES_THR13_U_MC2C_CFG_etag_u_mc2c__E_PCP_LEN6_TO_PKT_etag_u_mc2c__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR13_U_MC2C_CFG_vlan_eDSA__TPID_LEN4_TO_PKT_vlan_eDSA__TPID_OFFSET,                                          COPY_BYTES_THR13_U_MC2C_CFG_vlan_eDSA__TPID_LEN4_TO_PKT_vlan_eDSA__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET,     COPY_BITS_THR13_U_MC2C_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET,    COPY_BITS_THR13_U_MC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR13_U_MC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag_u_mc2c__TPID_OFFSET,                                  COPY_BITS_THR13_U_MC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag_u_mc2c__TPID_VALUE);
    /* Load thread_14 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET,                                       COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET,                                      COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET,                                    COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET,                         COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__UP_LEN3_TO_CFG_vlan__up_OFFSET,                                   COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__UP_LEN3_TO_CFG_vlan__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET,                                 COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__CFI_LEN1_TO_CFG_vlan__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,                                         COPY_BITS_THR14_U_MR2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_U_MR2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET,                                                  COPY_BYTES_THR14_U_MR2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_U_MR2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET,                                                    COPY_BYTES_THR14_U_MR2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                                        COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR14_U_MR2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET,                   COPY_BITS_THR14_U_MR2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_VALUE);
    /* Load thread_15 commands */
    ACCEL_CMD_LOAD(COPY_BITS_THR15_QCN_CFG_targetPortEntry__zero_0_LEN10_TO_PKT_qcn_tag__Version_OFFSET,                            COPY_BITS_THR15_QCN_CFG_targetPortEntry__zero_0_LEN10_TO_PKT_qcn_tag__Version_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN12_TO_PKT_qcn_tag__CPID_63_32_OFFSET,                        COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN12_TO_PKT_qcn_tag__CPID_63_32_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN9_TO_PKT_qcn_tag__Encapsulated_priority_OFFSET,              COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN9_TO_PKT_qcn_tag__Encapsulated_priority_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET,                                            COPY_BYTES_THR15_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_port_number_LEN5_TO_CFG_qcn_ha_table__DSA_w0_SrcPort_4_0_OFFSET,    COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_port_number_LEN5_TO_CFG_qcn_ha_table__DSA_w0_SrcPort_4_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET,                                     COPY_BITS_THR15_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET,                  COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR15_QCN_CFG_qcn_ha_table__sa_mac_47_32_LEN16_TO_PKT_mac_header__mac_sa_47_32_OFFSET,                COPY_BYTES_THR15_QCN_CFG_qcn_ha_table__sa_mac_47_32_LEN16_TO_PKT_mac_header__mac_sa_47_32_VALUE);
    /* Load thread_16 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET,                                          COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET,                                   COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET,                                             COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET,                                           COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET,                                    COPY_BITS_THR16_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR16_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET,                                               COPY_BYTES_THR16_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET,                                                  COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS27_OFFSET, COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS27_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS25_OFFSET, COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS25_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS15_OFFSET, COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS15_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_10_0_LEN11_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_PLUS3_OFFSET, COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_10_0_LEN11_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_PLUS3_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_13_11_LEN3_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_OFFSET,     COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_13_11_LEN3_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_VALUE);
	/* Load thread_19 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR19_E2U_Untagged_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET,                                          COPY_BYTES_THR19_E2U_Untagged_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__PCID_LEN12_TO_CFG_HA_Table_Upstream_Ports__E_CID_base_OFFSET,             COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__PCID_LEN12_TO_CFG_HA_Table_Upstream_Ports__E_CID_base_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_E2U_Untagged_DESC_nonQcn_desc__up0_LEN4_TO_CFG_HA_Table_Upstream_Ports__E_PCP_OFFSET,                    COPY_BITS_THR19_E2U_Untagged_DESC_nonQcn_desc__up0_LEN4_TO_CFG_HA_Table_Upstream_Ports__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_HA_Table_Upstream_Ports__IPL_Direction_OFFSET,    COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_HA_Table_Upstream_Ports__IPL_Direction_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_E2U_Untagged_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET,                                   COPY_BITS_THR19_E2U_Untagged_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR19_E2U_Untagged_CFG_HA_Table_Upstream_Ports__E_PCP_LEN8_TO_PKT_etag__E_PCP_OFFSET,                         COPY_BYTES_THR19_E2U_Untagged_CFG_HA_Table_Upstream_Ports__E_PCP_LEN8_TO_PKT_etag__E_PCP_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Default_VLAN_Tag_LEN16_TO_PKT_vlan__up_OFFSET,                            COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Default_VLAN_Tag_LEN16_TO_PKT_vlan__up_VALUE);
	/* Load thread_20 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET,                                               COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET,                                                COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_VALUE);
	/* Load thread_21 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET,                                               COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET,                                                COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_VALUE);
	/* Load thread_23 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR23_EVB_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET,                                                COPY_BYTES_THR23_EVB_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_EVB_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_vlan__up_OFFSET,                                               COPY_BITS_THR23_EVB_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_vlan__up_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_EVB_E2U_CFG_srcPortEntry__PVID_LEN12_TO_CFG_vlan__vid_OFFSET,                                            COPY_BITS_THR23_EVB_E2U_CFG_srcPortEntry__PVID_LEN12_TO_CFG_vlan__vid_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR23_EVB_E2U_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET,                                        COPY_BITS_THR23_EVB_E2U_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR23_EVB_E2U_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET,                                                   COPY_BYTES_THR23_EVB_E2U_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_VALUE);
	/* Load thread_24 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR24_EVB_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET,                                               COPY_BYTES_THR24_EVB_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_VALUE);
    /* Load thread_25 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR25_EVB_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET,                                                COPY_BYTES_THR25_EVB_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__sa_mac_47_32_LEN6_TO_PKT_mac_header__mac_sa_47_32_OFFSET,                 COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__sa_mac_47_32_LEN6_TO_PKT_mac_header__mac_sa_47_32_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_EVB_QCN_CFG_targetPortEntry__TPID_LEN16_TO_PKT_vlan__TPID_OFFSET,                                        COPY_BITS_THR25_EVB_QCN_CFG_targetPortEntry__TPID_LEN16_TO_PKT_vlan__TPID_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__EtherType_LEN10_TO_PKT_qcn_tag__EtherType_OFFSET,                         COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__EtherType_LEN10_TO_PKT_qcn_tag__EtherType_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN8_TO_PKT_qcn_tag__EtherType_PLUS10_OFFSET,                  COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN8_TO_PKT_qcn_tag__EtherType_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN7_TO_PKT_qcn_tag__EtherType_PLUS18_OFFSET,                  COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN7_TO_PKT_qcn_tag__EtherType_PLUS18_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET,                                         COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET,                      COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_VALUE);
    /* Load thread_27 commands */
	ACCEL_CMD_LOAD(COPY_BYTES_THR27_PRE_DA_E2U_CFG_srcPortEntry__reserved_0_LEN2_TO_PKT_pre_da_tag__reserved_OFFSET,                        COPY_BYTES_THR27_PRE_DA_E2U_CFG_srcPortEntry__reserved_0_LEN2_TO_PKT_pre_da_tag__reserved_VALUE);
	/* Load thread_45 commands */
	ACCEL_CMD_LOAD(COPY_BITS_THR45_VariableCyclesLengthWithAcclCmd_CFG_targetPortEntry__reserved_LEN3_TO_CFG_HA_Table_reserved_space__reserved_2_OFFSET, COPY_BITS_THR45_VariableCyclesLengthWithAcclCmd_CFG_targetPortEntry__reserved_LEN3_TO_CFG_HA_Table_reserved_space__reserved_2_VALUE);
	ACCEL_CMD_LOAD(COPY_BYTES_THR45_VariableCyclesLengthWithAcclCmd_PKT_mac_da_47_32_LEN6_TO_MACSHIFTLEFT_6_OFFSET,                                      COPY_BYTES_THR45_VariableCyclesLengthWithAcclCmd_PKT_mac_da_47_32_LEN6_TO_MACSHIFTLEFT_6_VALUE);
	/* Load thread_47 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR47_Add20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_20_OFFSET,                                                 COPY_BYTES_THR47_Add20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_20_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR47_Add20Bytes_CFG_HA_Table_reserved_space__reserved_0_LEN16_TO_PKT_HA_Table_reserved_space__reserved_0_OFFSET,  COPY_BYTES_THR47_Add20Bytes_CFG_HA_Table_reserved_space__reserved_0_LEN16_TO_PKT_HA_Table_reserved_space__reserved_0_VALUE);
    ACCEL_CMD_LOAD(COPY_BYTES_THR47_Add20Bytes_CFG_srcPortEntry__PVID_LEN4_TO_PKT_srcPortEntry__PVID_OFFSET,                                     COPY_BYTES_THR47_Add20Bytes_CFG_srcPortEntry__PVID_LEN4_TO_PKT_srcPortEntry__PVID_VALUE);
    /* Load thread_48 commands */
    ACCEL_CMD_LOAD(COPY_BYTES_THR48_Remove20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_20_OFFSET,    COPY_BYTES_THR48_Remove20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_20_VALUE);
    /* Load PTP commands */
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET,                     COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET,                             COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_egress_tai_sel_OFFSET,                      COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_egress_tai_sel_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_select_OFFSET,              COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_select_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_entry_id_OFFSET,            COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_entry_id_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_dispatching_en_OFFSET,                              COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_dispatching_en_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET,              COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS10_OFFSET,         COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS10_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_PLUS10_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS20_OFFSET,  COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_PLUS10_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS20_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_OFFSET,                COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_VALUE);
    ACCEL_CMD_LOAD(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET,                          COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_VALUE);
	ACCEL_CMD_LOAD(COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET,                      COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_VALUE);
    /* #pragma flush_memory */
}

