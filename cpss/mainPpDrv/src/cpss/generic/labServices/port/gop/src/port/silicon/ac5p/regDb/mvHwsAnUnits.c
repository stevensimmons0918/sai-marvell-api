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
* @file mvHwsAnUnits.c
*
* @brief AN register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC anUnitsDb[] = {
    /* 802_3AP_AUTO_NEGOTIATION_CONTROL */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*CONTROL_RESERVED */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RESTART_802_3AP_AUTO_NEGOTIATION */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*EXTENDED_NEXT_PAGE_CONTROL */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MY_RESET */

    /* 802_3AP_AUTO_NEGOTIATION_STATUS */
  /*5*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LINK_PARTNER_AUTO_NEGOTIATION_ABILITY */
  /*6*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FIELD_802_3AP_LINK_STATUS */
  /*7*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_ABILITY */
  /*8*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*STAT_REMOTE_FAULT */
  /*9*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_COMPLETE */
  /*10*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PAGE_RECEIVED */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EXTENDED_NEXT_PAGE_STATUS */
  /*12*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FIELD_802_3AP_PARALLEL_DETECTION_FAULT */

    /* AUTO_NEGOTIATION_DEVICE_IDENTIFIER_1 */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_318 */

    /* AUTO_NEGOTIATION_DEVICE_IDENTIFIER_2 */
  /*14*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REVISION_NUMBER */
  /*15*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 6 }, /*MODEL_NUMBER */
  /*16*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924 */

    /* AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1 */
  /*17*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE_22_REGISTERS_PRESENT */
  /*18*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMDPMA_PRESENT */
  /*19*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRESENT */
  /*20*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRESENT */
  /*21*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS_PRESENT */
  /*22*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS_PRESENT */
  /*23*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRESENT */
  /*24*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AUTO_NEGOTIATION_PRESENT */
  /*25*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEPARATED_PMA_1 */
  /*26*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEPARATED_PMA_2 */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEPARATED_PMA_3 */
  /*28*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEPARATED_PMA_4 */

    /* AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_2 */
  /*29*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE_22_EXTENSION_PRESENT */
  /*30*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MARVELL_SPECIFIC_DEVICE_1_PRESENT */
  /*31*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MARVELL_SPECIFIC_DEVICE_2_PRESENT */

    /* AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_1 */
  /*32*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ORGANIZATIONALLY_UNIQUE_PKG_IDENTIFIER_BIT_318 */

    /* AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_2 */
  /*33*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PKG_REVISION_NUMBER */
  /*34*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 6 }, /*PKG_MODEL_NUMBER */
  /*35*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*PKG_ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924 */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_1 */
  /*36*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LP_SELECTOR_FIELD */
  /*37*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*LP_ECHOED_NONCE_FIELD */
  /*38*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_PAUSE_CAPABLE */
  /*39*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_ASYMMETRIC_PAUSE */
  /*40*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_REMOTE_FAULT */
  /*41*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_ACKNOWLEDGE */
  /*42*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_NEXT_PAGE */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_2 */
  /*43*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LP_LINK_PARTNER_TRANSMITTED_NONCE_FIELD */
  /*44*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LP_1000BASE_KX_ */
  /*45*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LP_10GBASE_KX4 */
  /*46*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LP_10GBASE_KR */
  /*47*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LP_40GBASE_KR4 */
  /*48*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LP_40GBASE_CR4 */
  /*49*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_100GBASE_CR10 */
  /*50*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_100GBASE_KP4 */
  /*51*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_100GBASE_KR4 */
  /*52*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_100GBASE_CR4 */
  /*53*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*54*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_25GBASE_KR_OR_25GBASE_CR */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_3 */
  /*55*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LP_2_5G_KX */
  /*56*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LP_5G_KR */
  /*57*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LP_50G_KRCR */
  /*58*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LP_100G_KRCR2 */
  /*59*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LP_200G_KRCR4 */
  /*60*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LP_25G_R2_MARVELL_MODE_ */
  /*61*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_200G_KRCR8 */
  /*62*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_50G_R4_MARVELL_MODE */
  /*63*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_25G_RS_FEC_REQUESTEDF2 */
  /*64*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_25G_BASE_R_REQUESTEDF3 */
  /*65*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_LINK_PARTNER_FEC_ABILITY */
  /*66*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_LINK_PARTNER_REQUESTING_FEC_ENABLE */

    /* 802_3AP_NEXT_PAGE_TRANSMIT_EXTENDED_NEXT_PAGE_TRANSMIT */
  /*67*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*LD_NP_MESSAGEUNFORMATTED_FIELD */
  /*68*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_NP_TOGGLE */
  /*69*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_NP_ACKNOWLEDGE_2 */
  /*70*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_NP_MESSAGE_PAGE_MODE */
  /*71*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_NP_NEXT_PAGE */

    /* 802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_UNFORMATTED_CODE_FIELD_U0_TO_U15 */
  /*72*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LD_UNFORMATTED_FIELD0_15 */

    /* 802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_UNFORMATTED_CODE_FIELD_U16_TO_U31 */
  /*73*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LD_UNFORMATTED_FIELD16_31 */

    /* 802_3AP_LINK_PARTNER_NEXT_PAGE_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY */
  /*74*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*LP_NP_MESSAGEUNFORMATTED_FIELD */
  /*75*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_NP_TOGGLE */
  /*76*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_NP_ACKNOWLEDGE2 */
  /*77*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_NP_MESSAGE_PAGE */
  /*78*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_NP_ACKNOWLEDGE */
  /*79*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_NP_NEXT_PAGE */

    /* 802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_UNFORMATTED_CODE_FIELD_U0_TO_U15 */
  /*80*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LP_UNFORMATTED_FIELD0_15 */

    /* 802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_UNFORMATTED_CODE_FIELD_U16_TO_U31 */
  /*81*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LP_UNFORMATTED_FIELD16_31 */

    /* BACKPLANE_ETHERNET_STATUS */
  /*82*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*STAT_BP_AN_ABILITY */
  /*83*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*STAT_FIELD_1000BASE_KX */
  /*84*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*STAT_FIELD_10GBASE_KX4 */
  /*85*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*STAT_FIELD_10GBASE_KR */
  /*86*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*STAT_BASE_R_FEC_NEGOTIATED */
  /*87*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*STAT_FIELD_40GBASE_KR4 */
  /*88*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*STAT_FIELD_40GBASE_CR4 */
  /*89*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*STAT_RS_FEC_NEGOTIATED */
  /*90*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_CR10 */
  /*91*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_KP4 */
  /*92*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_KR4 */
  /*93*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_CR4 */
  /*94*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*STAT_FIELD_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*95*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*STAT_FIELD_25GBASE_KR_OR_25GBASE_CR */
  /*96*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*STAT_FIELD_2_5GBASE_KX */
  /*97*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*STAT_FIELD_5GBASE_KR */

    /* BACKPLANE_ETHERNET_STATUS2 */
  /*98*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FIELD_50GBASE_KRCR */
  /*99*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FIELD_100GBASE_KRCR2 */
  /*100*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FIELD_200GBASE_KRCR4 */
  /*101*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FIELD_100GBASE_KRCR */
  /*102*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FIELD_200GBASE_KRCR2 */
  /*103*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FIELD_400GBASE_KRCR4 */

    /* ANEG_LANE_0_CONTROL_0 */
  /*104*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*OVERRIDE_CTRL_S */
  /*105*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AP_ANEG_AMDISAM_S */
  /*106*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AP_ANEG_BP_REACHED_S */
  /*107*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 11 }, /*AP_ANEG_STATE_S10_0 */
  /*108*/  {/*baseAddr*/ 0x800,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_EEE_XNP_SEL_S */

    /* ANEG_LANE_0_CONTROL_1 */
  /*109*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*AG_LGTH_MATCH_CNT_S3_0 */
  /*110*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*CG_ANEG_TEST0_S */
  /*111*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*CG_ANEG_TEST3_S */
  /*112*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RG_NO_RINGOSC_S */
  /*113*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*OW_AS_NONCE_MATCH_S */
  /*114*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RG_AS_NONCE_MATCH_S */
  /*115*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL1500_S */
  /*116*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL500_S */
  /*117*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL50_S */
  /*118*/  {/*baseAddr*/ 0x804,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_LINK_FAIL_TIMER_OFF_S */

    /* ANEG_LANE_0_CONTROL_2 */
  /*119*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLEAR_HCD_OW_S */
  /*120*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_CLEAR_HCD_S */
  /*121*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ANEG_ENABLE_OW_S */
  /*122*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_ANEG_ENABLE_S */
  /*123*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RESTART_ANEG_OW_S */
  /*124*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RG_RESTART_ANEG_S */
  /*125*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LINK_OW_S */
  /*126*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_LINK_S */
  /*127*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*HCD_RESOLVED_OW_S */
  /*128*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_HCD_RESOLVED_S */
  /*129*/  {/*baseAddr*/ 0x808,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*REG2_RESERVED10 */

    /* ANEG_LANE_0_CONTROL_3 */
  /*130*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG3_RESERVED */
  /*131*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_SFEC_ENABLE_S */
  /*132*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_FEC_ENABLE_S */
  /*133*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*AA_RSFEC_ENABLE_OVERWRITE_VALUE */
  /*134*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*AP_RSFEC_ENABLE_OVERWRITE */
  /*135*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*AA_FEC_ENABLE_OVERWRITE_VALUE */
  /*136*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AP_FEC_ENABLE_OVERWRITE */
  /*137*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_PAUSE_ENABLE_OW_S */
  /*138*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_TX_PAUSE_ENABLE_S */
  /*139*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_PAUSE_ENABLE_OW_S */
  /*140*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*RG_RX_PAUSE_ENABLE_S */
  /*141*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PCS_LOCK_OW_S */
  /*142*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RG_PCS_LOCK_S */
  /*143*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DSP_LOCK_OW_S */
  /*144*/  {/*baseAddr*/ 0x80c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_DSP_LOCK_S */

    /* ANEG_LANE_0_CONTROL_4 */
  /*145*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AA_PWRUP_G_S */
  /*146*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*AA_PWRUP_2P5G_S */
  /*147*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_PWRUP_5GR_S */
  /*148*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_PWRUP_10GKR_S */
  /*149*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*AA_PWRUP_CON25GKR_S */
  /*150*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*AA_PWRUP_CON25GCR_S */
  /*151*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*AA_PWRUP_IEEE25GS_S */
  /*152*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AA_PWRUP_IEEE25GRCR_S */
  /*153*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*AA_PWRUP_40GKR4_S */
  /*154*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*AA_PWRUP_40GCR4_S */
  /*155*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*AA_PWRUP_CON50GKR2_S */
  /*156*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*AA_PWRUP_CON50GCR2_S */
  /*157*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*AA_PWRUP_100GKR4_S */
  /*158*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*AA_PWRUP_100GCR4_S */
  /*159*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*AA_PWRUP_50GRCR_S */
  /*160*/  {/*baseAddr*/ 0x810,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*AA_PWRUP_100GRCR2_S */

    /* ANEG_LANE_0_CONTROL_5 */
  /*161*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AA_PWRUP_200GRCR4_S */
  /*162*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*AA_PWRUP_25GKR4_S */
  /*163*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_PWRUP_50GKR4_S */
  /*164*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_PWRUP_25GKR2_S */
  /*165*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*AA_PWRUP_40GR2 */
  /*166*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*MY_RESERVED5 */
  /*167*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 5 }, /*SELECTED_MODES_TO_OVERWRITE */
  /*168*/  {/*baseAddr*/ 0x814,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PWRUP_MODES_OW_S */

    /* ANEG_LANE_0_CONTROL_6 */
  /*169*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*OW_PM_NORM_X_STATE_S */
  /*170*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_PM_NORM_X_STATE_S */
  /*171*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*OW_PM_LOOPBACK_S */
  /*172*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_PM_LOOPBACK_S */
  /*173*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*OW_FAR_CLEAR_RESET_ALL_S */
  /*174*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RG_FAR_CLEAR_RESET_ALL_S */
  /*175*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*OW_FAR_SET_RESTART_ALL_S */
  /*176*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_FAR_SET_RESTART_ALL_S */
  /*177*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*OW_RING_OSC_S */
  /*178*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_RING_OSC_S */
  /*179*/  {/*baseAddr*/ 0x818,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*REG6_RESERVED10 */

    /* ANEG_LANE_0_CONTROL_7 */
  /*180*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_VALUE_S */
  /*181*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_OW_S */
  /*182*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_SEL_S */
  /*183*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_TIMER2_LENGTH_S_OW_S */
  /*184*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*RG_TIMER2_LENGTH_SEL_S */
  /*185*/  {/*baseAddr*/ 0x81c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 10 }, /*MY_RESERVED6 */

    /* ANEG_LANE_0_CONTROL_8 */
  /*186*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*DET_DATA_TIMER_WINDOW_SELECT_ */
  /*187*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_DET_DAT_TIMER_OW_S */
  /*188*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*DET_CLOCK_TIMER_WINDOW_SELECT_ */
  /*189*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_DET_CLK_TIMER_OW_S */
  /*190*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RG_IGNORE_PULSE_TOO_SHORT_S */
  /*191*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*BREAK_LINK_LENGTH_OVERRIDE_VALUE_SELECT */
  /*192*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*RG_BREAK_LINK_LENGTH_OW_S */
  /*193*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RG_DISABLE_HW_NP_S */
  /*194*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RG_CHK_CLK_TRAN_S */
  /*195*/  {/*baseAddr*/ 0x820,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*MY_RESERVED14 */

    /* ANEG_LANE_0_CONTROL_9 */
  /*196*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*SW_AG_MODE */
  /*197*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SW_FEC_ENABLE_S */
  /*198*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SW_RSFEC_ENABLE_S */
  /*199*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SW_RESOLVED_S */
  /*200*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SW_LLFEC_ENABLE_S */
  /*201*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*MY_RESERVED9 */
  /*202*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SW_LP_SELECTOR_ENABLE_S */
  /*203*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*Z80_IO_RESOLVED */
  /*204*/  {/*baseAddr*/ 0x824,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SW_AP_ENABLE */

    /* ANEG_LANE_0_CONTROL_10 */
  /*205*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*ADVERTISED_CON25GKRCR_LOCATION */
  /*206*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*ADVERTISED_CON50GKRCR_LOCATION */
  /*207*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 3 }, /*ADVERTISED_CON400GRCR8_LOCATION */
  /*208*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*REG10_RESERVED13 */
  /*209*/  {/*baseAddr*/ 0x828,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_AP_LOCAL_RESET_S */

    /* ANEG_LANE_0_CONTROL_11 */
  /*210*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*REG11_RESERVED */
  /*211*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_OVERRIDE_VALUE */
  /*212*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*REG11_RESERVED4 */
  /*213*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 4 }, /*REG11_RESERVED7 */
  /*214*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*CHK_DSP_LOCK_S */
  /*215*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 3 }, /*REG11_RESERVED12 */
  /*216*/  {/*baseAddr*/ 0x82c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_SELECT */

    /* ANEG_LANE_0_CONTROL_12 */
  /*217*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*REG12_RESERVED */
  /*218*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_OVERRIDE_ENABLE */
  /*219*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*REG12_MY_RESERVED4 */
  /*220*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 5 }, /*N_AG_MODE_S */
  /*221*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*ADV_USE_N_AGMODE_S */
  /*222*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RE_AGSTART_USE_N_LINK_S */
  /*223*/  {/*baseAddr*/ 0x830,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*INTEL_FEC_MODE */

    /* ANEG_LANE_0_CONTROL_13 */
  /*224*/  {/*baseAddr*/ 0x834,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AP_INTERRUPT */

    /* ANEG_LANE_0_CONTROL_14 */
  /*225*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*ADVERTISED_200G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*226*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*ADVERTISED_25R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*227*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*ADVERTISED_800G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*228*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*ADVERTISED_50G_R4_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*229*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 3 }, /*ADVERTISED_40G_R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*230*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*CFG_40GR2_PRIO_HIGHER_THAN_40GR4_S */
  /*231*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*AP_INTERRUPT_TYPE */
  /*232*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*INTERRUPT_TIME_SELECT */
  /*233*/  {/*baseAddr*/ 0x838,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*INTERRUPT_MASK */

    /* ANEG_LANE_0_CONTROL_15 */
  /*234*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PM_NORM_X_STATE_S */
  /*235*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FAR_SET_RESTART_ALL_S */
  /*236*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FAR_CLEAR_RESET_ALL_S */
  /*237*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PCS_LINK */
  /*238*/  {/*baseAddr*/ 0x83c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SYMBOL_LOCK */

    /* ANEG_LANE_0_CONTROL_16_CONSORTIUM_MP5_0 */
  /*239*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG16_MESSAGEUNFORMATTED_FIELD */
  /*240*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG16_TOGGLE */
  /*241*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG16_ACKNOWLEDGE2 */
  /*242*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG16_MESSAGE_PAGE */
  /*243*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG16_ACKNOWLEDGE */
  /*244*/  {/*baseAddr*/ 0x840,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG16_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_17_CONSORTIUM_MP5_1 */
  /*245*/  {/*baseAddr*/ 0x844,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG17_OUI_23_13 */
  /*246*/  {/*baseAddr*/ 0x844,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG17_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_18_CONSORTIUM_MP5_2 */
  /*247*/  {/*baseAddr*/ 0x848,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG18_OUI_12_2 */
  /*248*/  {/*baseAddr*/ 0x848,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG18_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_19_CONSORTIUM_ET_0 */
  /*249*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG19_MESSAGEUNFORMATTED_FIELD */
  /*250*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG19_OUI_1_0 */
  /*251*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG19_TOGGLE */
  /*252*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG19_ACKNOWLEDGE2 */
  /*253*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG19_MESSAGE_PAGE */
  /*254*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG19_ACKNOWLEDGE */
  /*255*/  {/*baseAddr*/ 0x84c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG19_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_20_CONSORTIUM_ET_1 */
  /*256*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG20_RESERVED */
  /*257*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_25G_KR1 */
  /*258*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_25G_CR1 */
  /*259*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG20_CONSORTIUM_RESERVED */
  /*260*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_50G_KR2 */
  /*261*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_50G_CR2 */
  /*262*/  {/*baseAddr*/ 0x850,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*REG20_RESERVED10 */

    /* ANEG_LANE_0_CONTROL_21_CONSORTIUM_ET_2 */
  /*263*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG21_MARVELL_CONSORTIUM_40GR2 */
  /*264*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG21_RESERVED1 */
  /*265*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG21_MARVELL_CONSORTIUM_400GR8 */
  /*266*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*REG21_RESERVED3 */
  /*267*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF1 */
  /*268*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF2 */
  /*269*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF3 */
  /*270*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F1 */
  /*271*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F2 */
  /*272*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F3 */
  /*273*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F4 */
  /*274*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LFR */
  /*275*/  {/*baseAddr*/ 0x854,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 3 }, /*REG21_RESERVED13 */

    /* ANEG_LANE_0_CONTROL_22_LINK_PARTNER_CONSORTIUM_MP5_0 */
  /*276*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG22_MESSAGEUNFORMATTED_FIELD */
  /*277*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG22_TOGGLE */
  /*278*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG22_ACKNOWLEDGE2 */
  /*279*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG22_MESSAGE_PAGE */
  /*280*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG22_ACKNOWLEDGE */
  /*281*/  {/*baseAddr*/ 0x858,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG22_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_23_LINK_PARTNER_CONSORTIUM_MP5_1 */
  /*282*/  {/*baseAddr*/ 0x85c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG23_OUI_23_13 */

    /* ANEG_LANE_0_CONTROL_24_LINK_PARTNER_CONSORTIUM_MP5_2 */
  /*283*/  {/*baseAddr*/ 0x860,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG24_OUI_12_2 */
  /*284*/  {/*baseAddr*/ 0x860,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG24_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_25_LINK_PARTNER_CONSORTIUM_ET_0 */
  /*285*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG25_MESSAGEUNFORMATTED_FIELD */
  /*286*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG25_OUI_1_0 */
  /*287*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG25_TOGGLE */
  /*288*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG25_ACKNOWLEDGE2 */
  /*289*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG25_MESSAGE_PAGE */
  /*290*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG25_ACKNOWLEDGE */
  /*291*/  {/*baseAddr*/ 0x864,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG25_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_26_LINK_PARTERN_CONSORTIUM_ET_1 */
  /*292*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG26_RESERVED */
  /*293*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_25G_KR1 */
  /*294*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_25G_CR1 */
  /*295*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG26_CONSORTIUM_RESERVED */
  /*296*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_50G_KR2 */
  /*297*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_50G_CR2 */
  /*298*/  {/*baseAddr*/ 0x868,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*REG26_RESERVED10 */

    /* ANEG_LANE_0_CONTROL_27_LINK_PARTNER_CONSORTIUM_ET_2 */
  /*299*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG27_MARVELL_CONSORTIUM_40GR2 */
  /*300*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG27_RESERVED1 */
  /*301*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG27_MARVELL_CONSORTIUM_400GR8 */
  /*302*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*REG27_RESERVED3 */
  /*303*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF1 */
  /*304*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF2 */
  /*305*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF3 */
  /*306*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F1 */
  /*307*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F2 */
  /*308*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F3 */
  /*309*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F4 */
  /*310*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LFR */
  /*311*/  {/*baseAddr*/ 0x86c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 3 }, /*REG27_RESERVED13 */

    /* ANEG_Z80_MESSAGE_0 */
  /*312*/  {/*baseAddr*/ 0x880,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE0 */

    /* ANEG_Z80_MESSAGE_1 */
  /*313*/  {/*baseAddr*/ 0x884,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE1 */

    /* ANEG_Z80_MESSAGE_2 */
  /*314*/  {/*baseAddr*/ 0x888,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE2 */

    /* ANEG_Z80_MESSAGE_3 */
  /*315*/  {/*baseAddr*/ 0x88c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE3 */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_1 */
  /*316*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LD_SELECTOR_FIELD */
  /*317*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*LD_ECHOED_NONCE_FIELD */
  /*318*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_PAUSE_CAPABLE */
  /*319*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_ASYMMETRIC_PAUSE */
  /*320*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_REMOTE_FAULT */
  /*321*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_ACKNOWLEDGE */
  /*322*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_NEXT_PAGE */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_2 */
  /*323*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LD_LINK_PARTNER_TRANSMITTED_NONCE_FIELD */
  /*324*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LD_1000BASE_KX_ */
  /*325*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LD_10GBASE_KX4 */
  /*326*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LD_10GBASE_KR */
  /*327*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LD_40GBASE_KR4 */
  /*328*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LD_40GBASE_CR4 */
  /*329*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_100GBASE_CR10 */
  /*330*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_100GBASE_KP4 */
  /*331*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_100GBASE_KR4 */
  /*332*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_100GBASE_CR4 */
  /*333*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*334*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_25GBASE_KR_OR_25GBASE_CR */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_3 */
  /*335*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LD_2_5G_KX */
  /*336*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LD_5G_KR */
  /*337*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LD_50G_KRCR */
  /*338*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LD_100G_KRCR2 */
  /*339*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LD_200G_KRCR4 */
  /*340*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LD_25G_R2_MARVELL_MODE_ */
  /*341*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_200G_KRCR8 */
  /*342*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_50G_R4_MARVELL_MODE */
  /*343*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_25G_RS_FEC_REQUESTEDF2 */
  /*344*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_25G_BASE_R_REQUESTEDF3 */
  /*345*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_LINK_PARTNER_FEC_ABILITY */
  /*346*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_LINK_PARTNER_REQUESTING_FEC_ENABLE */
};

