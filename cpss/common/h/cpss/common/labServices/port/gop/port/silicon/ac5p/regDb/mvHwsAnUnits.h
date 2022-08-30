/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAnUnits.h
*
* @brief AN interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsAnUnits_H
#define __mvHwsAnUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_CONTROL_RESERVED_E,
  /*1*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_RESTART_802_3AP_AUTO_NEGOTIATION_E,
  /*2*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE_E,
  /*3*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_EXTENDED_NEXT_PAGE_CONTROL_E,
  /*4*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_MY_RESET_E,
  /*5*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_LINK_PARTNER_AUTO_NEGOTIATION_ABILITY_E,
  /*6*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_FIELD_802_3AP_LINK_STATUS_E,
  /*7*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_FIELD_802_3AP_AUTO_NEGOTIATION_ABILITY_E,
  /*8*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_STAT_REMOTE_FAULT_E,
  /*9*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_FIELD_802_3AP_AUTO_NEGOTIATION_COMPLETE_E,
  /*10*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_PAGE_RECEIVED_E,
  /*11*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_EXTENDED_NEXT_PAGE_STATUS_E,
  /*12*/  AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_FIELD_802_3AP_PARALLEL_DETECTION_FAULT_E,
  /*13*/  AN_UNITS_AUTO_NEGOTIATION_DEVICE_IDENTIFIER_1_ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_318_E,
  /*14*/  AN_UNITS_AUTO_NEGOTIATION_DEVICE_IDENTIFIER_2_REVISION_NUMBER_E,
  /*15*/  AN_UNITS_AUTO_NEGOTIATION_DEVICE_IDENTIFIER_2_MODEL_NUMBER_E,
  /*16*/  AN_UNITS_AUTO_NEGOTIATION_DEVICE_IDENTIFIER_2_ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924_E,
  /*17*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_CLAUSE_22_REGISTERS_PRESENT_E,
  /*18*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_PMDPMA_PRESENT_E,
  /*19*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_WIS_PRESENT_E,
  /*20*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_PCS_PRESENT_E,
  /*21*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_PHY_XS_PRESENT_E,
  /*22*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_DTE_XS_PRESENT_E,
  /*23*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_TC_PRESENT_E,
  /*24*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_AUTO_NEGOTIATION_PRESENT_E,
  /*25*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_SEPARATED_PMA_1_E,
  /*26*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_SEPARATED_PMA_2_E,
  /*27*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_SEPARATED_PMA_3_E,
  /*28*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1_SEPARATED_PMA_4_E,
  /*29*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_2_CLAUSE_22_EXTENSION_PRESENT_E,
  /*30*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_2_MARVELL_SPECIFIC_DEVICE_1_PRESENT_E,
  /*31*/  AN_UNITS_AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_2_MARVELL_SPECIFIC_DEVICE_2_PRESENT_E,
  /*32*/  AN_UNITS_AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_1_ORGANIZATIONALLY_UNIQUE_PKG_IDENTIFIER_BIT_318_E,
  /*33*/  AN_UNITS_AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_2_PKG_REVISION_NUMBER_E,
  /*34*/  AN_UNITS_AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_2_PKG_MODEL_NUMBER_E,
  /*35*/  AN_UNITS_AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_2_PKG_ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924_E,
  /*36*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_SELECTOR_FIELD_E,
  /*37*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_ECHOED_NONCE_FIELD_E,
  /*38*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_PAUSE_CAPABLE_E,
  /*39*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_ASYMMETRIC_PAUSE_E,
  /*40*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_REMOTE_FAULT_E,
  /*41*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_ACKNOWLEDGE_E,
  /*42*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_NEXT_PAGE_E,
  /*43*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_LINK_PARTNER_TRANSMITTED_NONCE_FIELD_E,
  /*44*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_1000BASE_KX__E,
  /*45*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_10GBASE_KX4_E,
  /*46*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_10GBASE_KR_E,
  /*47*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_40GBASE_KR4_E,
  /*48*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_40GBASE_CR4_E,
  /*49*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_100GBASE_CR10_E,
  /*50*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_100GBASE_KP4_E,
  /*51*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_100GBASE_KR4_E,
  /*52*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_100GBASE_CR4_E,
  /*53*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_25GBASE_KR_S_OR_25GBASE_CR_S_E,
  /*54*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_25GBASE_KR_OR_25GBASE_CR_E,
  /*55*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_2_5G_KX_E,
  /*56*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_5G_KR_E,
  /*57*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_50G_KRCR_E,
  /*58*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_100G_KRCR2_E,
  /*59*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_200G_KRCR4_E,
  /*60*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_25G_R2_MARVELL_MODE__E,
  /*61*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_200G_KRCR8_E,
  /*62*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_50G_R4_MARVELL_MODE_E,
  /*63*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_25G_RS_FEC_REQUESTEDF2_E,
  /*64*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_25G_BASE_R_REQUESTEDF3_E,
  /*65*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_LINK_PARTNER_FEC_ABILITY_E,
  /*66*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_LINK_PARTNER_REQUESTING_FEC_ENABLE_E,
  /*67*/  AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_MESSAGEUNFORMATTED_FIELD_E,
  /*68*/  AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_TOGGLE_E,
  /*69*/  AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_ACKNOWLEDGE_2_E,
  /*70*/  AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_MESSAGE_PAGE_MODE_E,
  /*71*/  AN_UNITS_REG_802_3AP_NEXT_PAGE_TRANSMIT_REGISTER_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_LD_NP_NEXT_PAGE_E,
  /*72*/  AN_UNITS_REG_802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_UNFORMATTED_CODE_FIELD_U0_TO_U15_LD_UNFORMATTED_FIELD0_15_E,
  /*73*/  AN_UNITS_REG_802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_REGISTER_UNFORMATTED_CODE_FIELD_U16_TO_U31_LD_UNFORMATTED_FIELD16_31_E,
  /*74*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_MESSAGEUNFORMATTED_FIELD_E,
  /*75*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_TOGGLE_E,
  /*76*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_ACKNOWLEDGE2_E,
  /*77*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_MESSAGE_PAGE_E,
  /*78*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_ACKNOWLEDGE_E,
  /*79*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_NEXT_PAGE_REGISTER_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_LP_NP_NEXT_PAGE_E,
  /*80*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_UNFORMATTED_CODE_FIELD_U0_TO_U15_LP_UNFORMATTED_FIELD0_15_E,
  /*81*/  AN_UNITS_REG_802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_REGISTER_UNFORMATTED_CODE_FIELD_U16_TO_U31_LP_UNFORMATTED_FIELD16_31_E,
  /*82*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_BP_AN_ABILITY_E,
  /*83*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_1000BASE_KX_E,
  /*84*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_10GBASE_KX4_E,
  /*85*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_10GBASE_KR_E,
  /*86*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_BASE_R_FEC_NEGOTIATED_E,
  /*87*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_40GBASE_KR4_E,
  /*88*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_40GBASE_CR4_E,
  /*89*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_RS_FEC_NEGOTIATED_E,
  /*90*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_100GBASE_CR10_E,
  /*91*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_100GBASE_KP4_E,
  /*92*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_100GBASE_KR4_E,
  /*93*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_100GBASE_CR4_E,
  /*94*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_25GBASE_KR_S_OR_25GBASE_CR_S_E,
  /*95*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_25GBASE_KR_OR_25GBASE_CR_E,
  /*96*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_2_5GBASE_KX_E,
  /*97*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS_REGISTER_STAT_FIELD_5GBASE_KR_E,
  /*98*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_50GBASE_KRCR_E,
  /*99*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_100GBASE_KRCR2_E,
  /*100*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_200GBASE_KRCR4_E,
  /*101*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_100GBASE_KRCR_E,
  /*102*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_200GBASE_KRCR2_E,
  /*103*/  AN_UNITS_BACKPLANE_ETHERNET_STATUS2_REGISTER_FIELD_400GBASE_KRCR4_E,
  /*104*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_OVERRIDE_CTRL_S_E,
  /*105*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_AMDISAM_S_E,
  /*106*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_BP_REACHED_S_E,
  /*107*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_STATE_S10_0_E,
  /*108*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_RG_EEE_XNP_SEL_S_E,
  /*109*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_AG_LGTH_MATCH_CNT_S3_0_E,
  /*110*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_CG_ANEG_TEST0_S_E,
  /*111*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_CG_ANEG_TEST3_S_E,
  /*112*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_NO_RINGOSC_S_E,
  /*113*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_OW_AS_NONCE_MATCH_S_E,
  /*114*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_AS_NONCE_MATCH_S_E,
  /*115*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E,
  /*116*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E,
  /*117*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E,
  /*118*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_OFF_S_E,
  /*119*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_CLEAR_HCD_OW_S_E,
  /*120*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_CLEAR_HCD_S_E,
  /*121*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_ANEG_ENABLE_OW_S_E,
  /*122*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_ANEG_ENABLE_S_E,
  /*123*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RESTART_ANEG_OW_S_E,
  /*124*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_RESTART_ANEG_S_E,
  /*125*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_LINK_OW_S_E,
  /*126*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_LINK_S_E,
  /*127*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_HCD_RESOLVED_OW_S_E,
  /*128*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_RG_HCD_RESOLVED_S_E,
  /*129*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_2_REG2_RESERVED10_E,
  /*130*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_REG3_RESERVED_E,
  /*131*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AA_SFEC_ENABLE_S_E,
  /*132*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AA_FEC_ENABLE_S_E,
  /*133*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AA_RSFEC_ENABLE_OVERWRITE_VALUE_E,
  /*134*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AP_RSFEC_ENABLE_OVERWRITE_E,
  /*135*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AA_FEC_ENABLE_OVERWRITE_VALUE_E,
  /*136*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_AP_FEC_ENABLE_OVERWRITE_E,
  /*137*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_TX_PAUSE_ENABLE_OW_S_E,
  /*138*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_RG_TX_PAUSE_ENABLE_S_E,
  /*139*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_RX_PAUSE_ENABLE_OW_S_E,
  /*140*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_RG_RX_PAUSE_ENABLE_S_E,
  /*141*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_PCS_LOCK_OW_S_E,
  /*142*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_RG_PCS_LOCK_S_E,
  /*143*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_DSP_LOCK_OW_S_E,
  /*144*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_3_RG_DSP_LOCK_S_E,
  /*145*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_G_S_E,
  /*146*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_2P5G_S_E,
  /*147*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_5GR_S_E,
  /*148*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_10GKR_S_E,
  /*149*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_CON25GKR_S_E,
  /*150*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_CON25GCR_S_E,
  /*151*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_IEEE25GS_S_E,
  /*152*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_IEEE25GRCR_S_E,
  /*153*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_40GKR4_S_E,
  /*154*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_40GCR4_S_E,
  /*155*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_CON50GKR2_S_E,
  /*156*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_CON50GCR2_S_E,
  /*157*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_100GKR4_S_E,
  /*158*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_100GCR4_S_E,
  /*159*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_50GRCR_S_E,
  /*160*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_4_AA_PWRUP_100GRCR2_S_E,
  /*161*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_200GRCR4_S_E,
  /*162*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_25GKR4_S_E,
  /*163*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_50GKR4_S_E,
  /*164*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_25GKR2_S_E,
  /*165*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_AA_PWRUP_40GR2_E,
  /*166*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_MY_RESERVED5_E,
  /*167*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_SELECTED_MODES_TO_OVERWRITE_E,
  /*168*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_5_PWRUP_MODES_OW_S_E,
  /*169*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_PM_NORM_X_STATE_S_E,
  /*170*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_RG_PM_NORM_X_STATE_S_E,
  /*171*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_PM_LOOPBACK_S_E,
  /*172*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_RG_PM_LOOPBACK_S_E,
  /*173*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_FAR_CLEAR_RESET_ALL_S_E,
  /*174*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_RG_FAR_CLEAR_RESET_ALL_S_E,
  /*175*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_FAR_SET_RESTART_ALL_S_E,
  /*176*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_RG_FAR_SET_RESTART_ALL_S_E,
  /*177*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_OW_RING_OSC_S_E,
  /*178*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_RG_RING_OSC_S_E,
  /*179*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_6_REG6_RESERVED10_E,
  /*180*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_LOSTLOCK_VALUE_S_E,
  /*181*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_LOSTLOCK_OW_S_E,
  /*182*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_LOSTLOCK_SEL_S_E,
  /*183*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_TIMER2_LENGTH_S_OW_S_E,
  /*184*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_RG_TIMER2_LENGTH_SEL_S_E,
  /*185*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_7_MY_RESERVED6_E,
  /*186*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_DET_DATA_TIMER_WINDOW_SELECT__E,
  /*187*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_DET_DAT_TIMER_OW_S_E,
  /*188*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_DET_CLOCK_TIMER_WINDOW_SELECT__E,
  /*189*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_DET_CLK_TIMER_OW_S_E,
  /*190*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_IGNORE_PULSE_TOO_SHORT_S_E,
  /*191*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_BREAK_LINK_LENGTH_OVERRIDE_VALUE_SELECT_E,
  /*192*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_BREAK_LINK_LENGTH_OW_S_E,
  /*193*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_DISABLE_HW_NP_S_E,
  /*194*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_RG_CHK_CLK_TRAN_S_E,
  /*195*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_8_MY_RESERVED14_E,
  /*196*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_AG_MODE_E,
  /*197*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_FEC_ENABLE_S_E,
  /*198*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_RSFEC_ENABLE_S_E,
  /*199*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_RESOLVED_S_E,
  /*200*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_LLFEC_ENABLE_S_E,
  /*201*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_MY_RESERVED9_E,
  /*202*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_LP_SELECTOR_ENABLE_S_E,
  /*203*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_Z80_IO_RESOLVED_E,
  /*204*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_9_SW_AP_ENABLE_E,
  /*205*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON25GKRCR_LOCATION_E,
  /*206*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON50GKRCR_LOCATION_E,
  /*207*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_ADVERTISED_CON400GRCR8_LOCATION_E,
  /*208*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_REG10_RESERVED13_E,
  /*209*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_10_RG_AP_LOCAL_RESET_S_E,
  /*210*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_REG11_RESERVED_E,
  /*211*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_SYMBOL_LOCK_OVERRIDE_VALUE_E,
  /*212*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_REG11_RESERVED4_E,
  /*213*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_REG11_RESERVED7_E,
  /*214*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_CHK_DSP_LOCK_S_E,
  /*215*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_REG11_RESERVED12_E,
  /*216*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_11_SYMBOL_LOCK_SELECT_E,
  /*217*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_REG12_RESERVED_E,
  /*218*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_SYMBOL_LOCK_OVERRIDE_ENABLE_E,
  /*219*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_REG12_MY_RESERVED4_E,
  /*220*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_N_AG_MODE_S_E,
  /*221*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_ADV_USE_N_AGMODE_S_E,
  /*222*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_RE_AGSTART_USE_N_LINK_S_E,
  /*223*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_12_INTEL_FEC_MODE_E,
  /*224*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_13_AP_INTERRUPT_E,
  /*225*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_200G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E,
  /*226*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_25R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E,
  /*227*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_800G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E,
  /*228*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_50G_R4_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E,
  /*229*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_ADVERTISED_40G_R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT_E,
  /*230*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_CFG_40GR2_PRIO_HIGHER_THAN_40GR4_S_E,
  /*231*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_AP_INTERRUPT_TYPE_E,
  /*232*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_INTERRUPT_TIME_SELECT_E,
  /*233*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_14_INTERRUPT_MASK_E,
  /*234*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_PM_NORM_X_STATE_S_E,
  /*235*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_FAR_SET_RESTART_ALL_S_E,
  /*236*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_FAR_CLEAR_RESET_ALL_S_E,
  /*237*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_PCS_LINK_E,
  /*238*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_15_SYMBOL_LOCK_E,
  /*239*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_MESSAGEUNFORMATTED_FIELD_E,
  /*240*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_TOGGLE_E,
  /*241*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_ACKNOWLEDGE2_E,
  /*242*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_MESSAGE_PAGE_E,
  /*243*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_ACKNOWLEDGE_E,
  /*244*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_16_CONSORTIUM_MP5_REGISTER_0_REG16_NEXT_PAGE_E,
  /*245*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_17_CONSORTIUM_MP5_REGISTER_1_REG17_OUI_23_13_E,
  /*246*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_17_CONSORTIUM_MP5_REGISTER_1_REG17_RESERVED11_E,
  /*247*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_18_CONSORTIUM_MP5_REGISTER_2_REG18_OUI_12_2_E,
  /*248*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_18_CONSORTIUM_MP5_REGISTER_2_REG18_RESERVED11_E,
  /*249*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_MESSAGEUNFORMATTED_FIELD_E,
  /*250*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_OUI_1_0_E,
  /*251*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_TOGGLE_E,
  /*252*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_ACKNOWLEDGE2_E,
  /*253*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_MESSAGE_PAGE_E,
  /*254*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_ACKNOWLEDGE_E,
  /*255*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_19_CONSORTIUM_ET_REGISTER_0_REG19_NEXT_PAGE_E,
  /*256*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_RESERVED_E,
  /*257*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_25G_KR1_E,
  /*258*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_25G_CR1_E,
  /*259*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_RESERVED_E,
  /*260*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_50G_KR2_E,
  /*261*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_CONSORTIUM_50G_CR2_E,
  /*262*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_20_CONSORTIUM_ET_REGISTER_1_REG20_RESERVED10_E,
  /*263*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_MARVELL_CONSORTIUM_40GR2_E,
  /*264*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_RESERVED1_E,
  /*265*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_MARVELL_CONSORTIUM_400GR8_E,
  /*266*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_RESERVED3_E,
  /*267*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_LF1_E,
  /*268*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_LF2_E,
  /*269*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_LF3_E,
  /*270*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F1_E,
  /*271*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F2_E,
  /*272*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F3_E,
  /*273*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_F4_E,
  /*274*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_CONSORTIUM_LFR_E,
  /*275*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_21_CONSORTIUM_ET_REGISTER_2_REG21_RESERVED13_E,
  /*276*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_MESSAGEUNFORMATTED_FIELD_E,
  /*277*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_TOGGLE_E,
  /*278*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_ACKNOWLEDGE2_E,
  /*279*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_MESSAGE_PAGE_E,
  /*280*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_ACKNOWLEDGE_E,
  /*281*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_22_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_0_REG22_NEXT_PAGE_E,
  /*282*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_23_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_1_REG23_OUI_23_13_E,
  /*283*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_24_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_2_REG24_OUI_12_2_E,
  /*284*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_24_LINK_PARTNER_CONSORTIUM_MP5_REGISTER_2_REG24_RESERVED11_E,
  /*285*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_MESSAGEUNFORMATTED_FIELD_E,
  /*286*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_OUI_1_0_E,
  /*287*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_TOGGLE_E,
  /*288*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_ACKNOWLEDGE2_E,
  /*289*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_MESSAGE_PAGE_E,
  /*290*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_ACKNOWLEDGE_E,
  /*291*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_25_LINK_PARTNER_CONSORTIUM_ET_REGISTER_0_REG25_NEXT_PAGE_E,
  /*292*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_RESERVED_E,
  /*293*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_CONSORTIUM_25G_KR1_E,
  /*294*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_CONSORTIUM_25G_CR1_E,
  /*295*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_CONSORTIUM_RESERVED_E,
  /*296*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_CONSORTIUM_50G_KR2_E,
  /*297*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_CONSORTIUM_50G_CR2_E,
  /*298*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_26_LINK_PARTERN_CONSORTIUM_ET_REGISTER_1_REG26_RESERVED10_E,
  /*299*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_MARVELL_CONSORTIUM_40GR2_E,
  /*300*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_RESERVED1_E,
  /*301*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_MARVELL_CONSORTIUM_400GR8_E,
  /*302*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_RESERVED3_E,
  /*303*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_LF1_E,
  /*304*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_LF2_E,
  /*305*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_LF3_E,
  /*306*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_F1_E,
  /*307*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_F2_E,
  /*308*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_F3_E,
  /*309*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_F4_E,
  /*310*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_CONSORTIUM_LFR_E,
  /*311*/  AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_27_LINK_PARTNER_CONSORTIUM_ET_REGISTER_2_REG27_RESERVED13_E,
  /*312*/  AN_UNITS_ANEG_Z80_MESSAGE_REG_0_ANEG_Z80_MESSAGE0_E,
  /*313*/  AN_UNITS_ANEG_Z80_MESSAGE_REG_1_ANEG_Z80_MESSAGE1_E,
  /*314*/  AN_UNITS_ANEG_Z80_MESSAGE_REG_2_ANEG_Z80_MESSAGE2_E,
  /*315*/  AN_UNITS_ANEG_Z80_MESSAGE_REG_3_ANEG_Z80_MESSAGE3_E,
  /*316*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_SELECTOR_FIELD_E,
  /*317*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_ECHOED_NONCE_FIELD_E,
  /*318*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_PAUSE_CAPABLE_E,
  /*319*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_ASYMMETRIC_PAUSE_E,
  /*320*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_REMOTE_FAULT_E,
  /*321*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_ACKNOWLEDGE_E,
  /*322*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_NEXT_PAGE_E,
  /*323*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_LINK_PARTNER_TRANSMITTED_NONCE_FIELD_E,
  /*324*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_1000BASE_KX__E,
  /*325*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_10GBASE_KX4_E,
  /*326*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_10GBASE_KR_E,
  /*327*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_40GBASE_KR4_E,
  /*328*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_40GBASE_CR4_E,
  /*329*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_CR10_E,
  /*330*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_KP4_E,
  /*331*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_KR4_E,
  /*332*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_CR4_E,
  /*333*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_25GBASE_KR_S_OR_25GBASE_CR_S_E,
  /*334*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_25GBASE_KR_OR_25GBASE_CR_E,
  /*335*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_2_5G_KX_E,
  /*336*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_5G_KR_E,
  /*337*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_50G_KRCR_E,
  /*338*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_100G_KRCR2_E,
  /*339*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_200G_KRCR4_E,
  /*340*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_25G_R2_MARVELL_MODE__E,
  /*341*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_200G_KRCR8_E,
  /*342*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_50G_R4_MARVELL_MODE_E,
  /*343*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_25G_RS_FEC_REQUESTEDF2_E,
  /*344*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_25G_BASE_R_REQUESTEDF3_E,
  /*345*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_FEC_ABILITY_E,
  /*346*/  AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_REQUESTING_FEC_ENABLE_E,
    AN_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_AN_UNITS;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAnUnits_H */

