/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsAc5xPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifndef  MV_HWS_FREE_RTOS

typedef struct{
    GT_U32 regAddr;
    GT_U32 expectedData;
    GT_U32 mask;
}MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC;

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_100G_R4_RegDb[] = {

    {0xA7800004, 0x000f0000, 0xFFFFFFFF},   /* m_100.m_EXT.MTIP_Global_FEC_Control */
    {0xA7800008, 0x5a780c04, 0xFFF7FFFF},   /* m_100.m_EXT.MTIP_Global_Channel_Control*/
    {0xA7800010, 0x00031c01, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x28010f0f, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */
    {0xA7800018, 0x00000003, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control2*/
    {0xA7800084, 0x004b0110, 0xFFFe03FF},   /* m_100.m_EXT.mtip_ext_units_RegFile.MTIP_Port_Control[0]*/

    {0xA7802008, 0x00102813, 0xFFeFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802080, 0x00000131, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/


    /*{0xA7C00808, 0x000000ff, 0xFFFFFFFF},  */ /* m_100.m_PCS100.VENDOR_VL_INTVL */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_25G_R1_RegDb[] = {

    {0xA7800008, 0x5a780c00, 0xFFF7FFFF},   /* m_100.m_EXT.MTIP_Global_Channel_Control*/
    {0xA7800018, 0x00000003, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control2*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/

    {0xA7800010, 0x00031c01, 0xFFFFF7FF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x28010101, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */
    {0xA7800004, 0x00010000, 0xFFFFFFFF},   /* m_100.m_EXT.MTIP_Global_FEC_Control */

    {0xA7C01808, 0x0000009f, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_VL_INTVL */
    {0xA7C01820, 0x000068c1, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL0_0 */
    {0xA7C01824, 0x00000021, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL0_1 */
    {0xA7C01840, 0x00000305, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_PCS_MODE */
    {0xA7C01828, 0x0000c4f0, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_0*/
    {0xA7C0182c, 0x000000e6, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_1*/
    {0xA7C01830, 0x000065c5, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_0*/
    {0xA7C01834, 0x0000009b, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_1*/
    {0xA7C01838, 0x000079a2, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_0*/
    {0xA7C0183c, 0x0000003d, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_1*/

    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802080, 0x00000121, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA7802008, 0x00302813, 0xFFFFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMtiUsx_25G_R1_RegDb[] = {

    {0x9D800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9D800004, 0x0003303f, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/

    {0xA3800000, 0x00008000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0]*/
    {0xA3800048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0]*/
    {0xA380004C, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0]*/

    {0xA380200C, 0x0000009f, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL_INTVL*/

    {0xA38010F8, 0x00000005, 0xFFFFFFFF},   /* m_USX0.m_PCS.p_VENDOR_PCS_MODE[0]*/
    {0xA3801000, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_PCS.p_CONTROL1[0]*/

    {0x9D802044, 0x0000000c, 0x0000003F},   /* m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x9D802048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.CRC_MODE*/
    {0x9D802008, 0x00302813, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x9D802020, 0x00000015, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x9D80201c, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x9D802080, 0x00000121, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.XIF_MODE*/
    {0x9D802014, 0x00003178, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.FRM_LENGTH*/

    {0xA3802044, 0x00003321, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL0_1*/
    {0xA380204c, 0x000033e6, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL1_1*/
    {0xA3802054, 0x0000339b, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL2_1*/
    {0xA380205c, 0x0000333d, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL3_1*/

    {0xA3803000, 0x00000004, 0xFFFFFFFF}    /* m_USX0.m_RSFEC_0.RSFEC_CONTROL*/

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMtiUsx_20G_OXGMII_RegDb[] = {

    {0x9D800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9D800004, 0x0003303f, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/

    {0xA3800000, 0x00008000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0]*/
    {0xA3800048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0]*/
    {0xA380004C, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0]*/
    {0xA3800058, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_USXGMII_REP[0]*/

    {0xA3802008, 0x00000008, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.PORTS_ENA*/

    {0x9D802044, 0x0000000c, 0x0000003F},   /* m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x9D802048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.CRC_MODE*/
    {0x9D802008, 0x00202813, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x9D802020, 0x00000015, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x9D80201c, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x9D802080, 0x00000121, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.XIF_MODE*/
    {0x9D802014, 0x00003178, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.FRM_LENGTH*/

    {0xA3803000, 0x00000004, 0xFFFFFFFF}    /* m_USX0.m_RSFEC_0.RSFEC_CONTROL*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixCpu_25G_R1_RegDb[] = {

    {0xA500001C, 0x0000003f, 0xFFFFFFFF},  /* m_CPU0.m_EXT.mtip_cpu_ext_units_RegFile.Port_Clocks*/
    {0xA5000018, 0x00000007, 0xFFFFFFFF},  /* m_CPU0.m_EXT.mtip_cpu_ext_units_RegFile.Port_Reset*/

    {0xA1000840, 0x00000007, 0xFFFFFCFF},  /* m_CPU0.m_PCS.PORT_VENDOR_PCS_MODE*/
    {0xA1000808, 0x00004fff, 0xFFFFFFFF},  /* m_CPU0.m_PCS.PORT_VL_INTVL*/
    {0xA1000000, 0x00002040, 0xFFFF7FFF},  /* m_RAL.m_CPU0.m_PCS.port_CONTROL1*/

    {0xA5002044, 0x0000000c, 0x0000003F},   /* m_CPU0.m_MAC_0.TX_IPG_LENGTH*/
    {0xA5002008, 0x00102813, 0xFFEFFFFF},   /* m_CPU0.m_MAC_0.COMMAND_CONFIG*/
    {0xA5002020, 0x00000015, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA500201c, 0x00000001, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA5002080, 0x00000121, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.XIF_MODE*/
    {0xA5002014, 0x00003178, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.FRM_LENGTH*/

    {0xA1001200, 0x00000004, 0xFFFFFFFF}    /* m_CPU0.m_RSFEC.RSFEC_VENDOR_CONTROL*/

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixCpu_10G_R1_RegDb[] = {

    {0xA500001C, 0x0000003f, 0xFFFFFFFF},  /* m_CPU0.m_EXT.mtip_cpu_ext_units_RegFile.Port_Clocks*/
    {0xA5000018, 0x00000007, 0xFFFFFFFF},  /* m_CPU0.m_EXT.mtip_cpu_ext_units_RegFile.Port_Reset*/

    {0xA1000840, 0x00000003, 0xFFFFFCFF},  /* m_CPU0.m_PCS.PORT_VENDOR_PCS_MODE*/
    {0xA1000808, 0x00004fff, 0xFFFFFFFF},  /* m_CPU0.m_PCS.PORT_VL_INTVL*/
    {0xA1000000, 0x00002040, 0xFFFF7FFF},  /* m_RAL.m_CPU0.m_PCS.port_CONTROL1*/

    {0xA5002044, 0x0000000c, 0x0000003F},   /* m_CPU0.m_MAC_0.TX_IPG_LENGTH*/
    {0xA5002008, 0x00102813, 0xFFEFFFFF},   /* m_CPU0.m_MAC_0.COMMAND_CONFIG*/
    {0xA5002020, 0x00000015, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA500201c, 0x00000001, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA5002080, 0x00000121, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.XIF_MODE*/
    {0xA5002014, 0x00003178, 0xFFFFFFFF},   /* m_CPU0.m_MAC_0.FRM_LENGTH*/

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_50G_R2_RegDb[] =
{
    {0xA7800008, 0x5a780800, 0xFFF7FFFF},   /* m_100.m_EXT.MTIP_Global_Channel_Control*/
    {0xA7800018, 0x00000003, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control2*/
    {0xA7800084, 0x00430110, 0xFFFFFFFF},   /* m_100.m_EXT.MTIP_Port_Control[0]*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/

    {0xA7800010, 0x00031c01, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x28010303, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */
    {0xA7800004, 0x00030000, 0xFFFFFFFF},   /* m_100.m_EXT.MTIP_Global_FEC_Control */

    {0xA7C01808, 0x0000009f, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_VL_INTVL */
    {0xA7C01820, 0x00007690, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL0_0 */
    {0xA7C01824, 0x00000047, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL0_1 */
    {0xA7C01840, 0x00000300, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_PCS_MODE */
    {0xA7C01828, 0x0000c4f0, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_0*/
    {0xA7C0182c, 0x000000e6, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_1*/
    {0xA7C01830, 0x000065c5, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_0*/
    {0xA7C01834, 0x0000009b, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_1*/
    {0xA7C01838, 0x000079a2, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_0*/
    {0xA7C0183c, 0x0000003d, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_1*/

    {0xA7C02808, 0x0000009f, 0xFFFFFFFF},   /* m_100.m_PCS25_0.VENDOR_VL_INTVL */

    {0xA7C05000, 0x00000200, 0xFFFFFFFF},   /* m_100.m_RSFEC.p_RSFEC_CONTROL[0] */
    {0xA7C05020, 0x00000200, 0xFFFFFFFF},   /* m_100.m_RSFEC.p_RSFEC_CONTROL[1] */

    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802080, 0x00000120, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA7802008, 0x00302813, 0xFFeFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_SGMII_RegDb[] =
{
    {0xA7800084, 0x000b0110, 0xFFFE03FF},   /* m_100.m_EXT.MTIP_Port_Control[0]*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/

    {0xA7800010, 0x00039401, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x48010101, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */

    {0xA7C063e0, 0x00000001, 0xFFFFFFFF},   /* m_100.m_LPCS.lpcs_units_RegFile.GMODE */
    {0xA7C06050, 0x00000009, 0xFFFFFFFF},   /* m_100.m_LPCS.lpcs_units_RegFile.p_IF_MODE[0] */
    {0xA7C06000, 0x00000140, 0xFFFF7FFF},   /* m_100.m_LPCS.lpcs_units_RegFile.p_CONTROL[0] */

    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802080, 0x00000130, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA7802008, 0x00302813, 0xFFeFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMtiUSX_SGMII_RegDb[] =
{
    {0x9D80002c, 0x00030110, 0xFFFE03FF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.MTIP_Port_Control[0] */

    {0x9e000000, 0x00040000, 0xFFFFFFFF},   /* m_MIF0.mif_t8_channel_mapping_register[0]*/
    {0x9e000800, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t8_rx_control_register*/
    {0x9e001000, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t8_tx_control_registers*/
    {0x9e001200, 0x000001b0, 0xFFFFFFFF},   /* m_MIF0.mif_t8_tx_credit_register[0]*/

    {0x9D800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9D800004, 0x0003c009, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/

    {0xA3800000, 0x00008000, 0xFFFFFEBF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0]*/
    {0xA3800048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0]*/
    {0xA380004C, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0]*/
    {0xA38003e0, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.GMODE*/
    {0xA3800050, 0x00000009, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_IF_MODE[0]*/

    {0x9D802044, 0x0000000c, 0x0000003F},   /* m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x9D802048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.CRC_MODE*/
    {0x9D802008, 0x00202813, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x9D802020, 0x00000015, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x9D80201c, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x9D802080, 0x00000131, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.XIF_MODE*/
    {0x9D802014, 0x00003178, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.FRM_LENGTH*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_1000BaseX_RegDb[] =
{
    {0xA7800084, 0x001b0110, 0xFFFE03FF},   /* m_100.m_EXT.MTIP_Port_Control[0]*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/

    {0xA7800010, 0x00039401, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x48010101, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */

    {0xA7C063e0, 0x00000001, 0xFFFFFFFF},   /* m_100.m_LPCS.lpcs_units_RegFile.GMODE */
    {0xA7C06050, 0x00000008, 0xFFFFFFFF},   /* m_100.m_LPCS.lpcs_units_RegFile.p_IF_MODE[0] */
    {0xA7C06000, 0x00000140, 0xFFFF7FFF},   /* m_100.m_LPCS.lpcs_units_RegFile.p_CONTROL[0] */

    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802080, 0x00000130, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA7802008, 0x00302813, 0xFFeFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMti100_10G_R1_RegDb[] = {

    {0xA7800008, 0x5a780c00, 0xFFF7FFFF},   /* m_100.m_EXT.MTIP_Global_Channel_Control*/
    {0xA7800018, 0x00000003, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control2*/

    {0xA8400080, 0x00043232, 0xFFFFFFFF},   /* m_MIF4.mif_t32_channel_mapping_register[0]*/
    {0xA8400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_rx_control_register*/
    {0xA8401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_control_registers*/
    {0xA8401290, 0x00000180, 0xFFFFFFFF},   /* m_MIF4.mif_t32_tx_credit_register[0]*/

    {0xA7800010, 0x00031401, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Clock_Enable */
    {0xA7800014, 0x08010101, 0xFFFFFFFF},   /* m_100.m_EXT.Global_Reset_Control */
    {0xA7800004, 0x00000000, 0xFFFFFFFF},   /* m_100.m_EXT.MTIP_Global_FEC_Control */
    {0xA7800084, 0x002b0110, 0xFFFE03FF},   /* m_100.m_EXT.MTIP_Port_Control[0] */

    {0xA7C01840, 0x00000303, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_PCS_MODE */
    {0xA7C01828, 0x0000c4f0, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_0*/
    {0xA7C0182c, 0x000000e6, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL1_1*/
    {0xA7C01830, 0x000065c5, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_0*/
    {0xA7C01834, 0x0000009b, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL2_1*/
    {0xA7C01838, 0x000079a2, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_0*/
    {0xA7C0183c, 0x0000003d, 0xFFFFFFFF},   /* m_100.m_PCS50_0.VENDOR_MIRROR_VL3_1*/

    {0xA7802044, 0x0000000c, 0x0000003F},   /*m_100.m_MAC_0.TX_IPG_LENGTH*/
    {0xA7802048, 0x00000000, 0xFFFFFFFF},   /*m_100.m_MAC_0.CRC_MODE*/
    {0xA7802020, 0x00000009, 0xFFFFFFFF},   /*m_100.m_MAC_0.TX_FIFO_SECTIONS*/
    {0xA780201c, 0x00000001, 0xFFFFFFFF},   /*m_100.m_MAC_0.RX_FIFO_SECTIONS*/
    {0xA7802080, 0x00000121, 0xFFFFFFFF},   /*m_100.m_MAC_0.XIF_MODE*/
    {0xA7802014, 0x00003178, 0xFFFFFFFF},   /*m_100.m_MAC_0.FRM_LENGTH*/
    {0xA7802008, 0x00302813, 0xFFeFFFFF},   /*m_100.m_MAC_0.COMMAND_CONFIG*/
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMtiUsx_10G_OUSGMII_RegDb[] = {

    {0x9D800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9D800004, 0x0003c009, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/

    {0xA3800000, 0x00008000, 0x0000FFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0]*/
    {0xA3800048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0]*/
    {0xA380004C, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0]*/
    {0xA38003e0, 0x00000c00, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.GMODE*/
    {0xA3800050, 0x00000009, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_IF_MODE[0]*/

    {0x9D802044, 0x0000000c, 0x0000003F},   /* m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x9D802048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.CRC_MODE*/
    {0x9D802008, 0x00202813, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x9D802020, 0x00000015, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x9D80201c, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x9D802080, 0x00000131, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.XIF_MODE*/
    {0x9D802014, 0x00003178, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.FRM_LENGTH*/
#if 0
    {0xA3803000, 0x00000004, 0xFFFFFFFF}    /* m_USX0.m_RSFEC_0.RSFEC_CONTROL*/
#endif
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC phoenixMtiUsx_10G_R1_RegDb[] = {

    {0x9D800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x9D800004, 0x0003003f, 0xFFFFFFFF},   /* m_USX0.m_EXT.mtip_usx_ext_units_RegFile.Global_PCS_Clock_and_Reset_Control*/

    {0xA3800000, 0x00000000, 0x0000DC00},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0]*/
    {0xA3800048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0]*/
    {0xA380004C, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0]*/

    {0xA380200C, 0x00001004, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL_INTVL*/

    {0xA38010F8, 0x00000007, 0x0000000F},   /* m_USX0.m_PCS.p_VENDOR_PCS_MODE[0]*/
    {0xA3801000, 0x00000000, 0x0000C800},   /* m_USX0.m_PCS.p_CONTROL1[0]*/

    {0x9D802044, 0x0000000c, 0x0000003F},   /* m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x9D802048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.CRC_MODE*/
    {0x9D802008, 0x00302813, 0xFFEFFFFF},   /* m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x9D802020, 0x00000015, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x9D80201c, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x9D802080, 0x00000121, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.XIF_MODE*/
    {0x9D802014, 0x00003178, 0xFFFFFFFF},   /* m_USX0.m_MAC_0.FRM_LENGTH*/

    {0xA3802044, 0x00000021, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL0_1*/
    {0xA380204c, 0x000000e6, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL1_1*/
    {0xA3802054, 0x0000009b, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL2_1*/
    {0xA380205c, 0x0000003d, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL3_1*/

    {0xA3803000, 0x00000000, 0xFFFFFFFF}    /* m_USX0.m_RSFEC_0.RSFEC_CONTROL*/

};


GT_STATUS mvHwsPhoenixPunktUsxMacCheck(GT_U32 portType)
{
    GT_U32 i;
    GT_U32 iterNum;
    GT_U32 regData;
    const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC *regPtr;

    if(portType == 0)
    {
        hwsOsPrintf(" 1 - MTI_100 - 25G_R1         \n");
        hwsOsPrintf(" 2 - MTI_USX - 25G_R1         \n");
        hwsOsPrintf(" 3 - MTI_USX - 20G_OXGMII     \n");
        hwsOsPrintf(" 4 - MTI_CPU - 25G_R1         \n");
        hwsOsPrintf(" 5 - MTI_CPU - 10G_R1         \n");
        hwsOsPrintf(" 6 - MTI_100 - 50G_R2         \n");
        hwsOsPrintf(" 7 - MTI_100 - 100G_R4        \n");
        hwsOsPrintf(" 8 - MTI_100 - SGMII          \n");
        hwsOsPrintf(" 9 - MTI_USX - SGMII          \n");
        hwsOsPrintf(" 10- MTI_USX - 1000_BaseX     \n");
        hwsOsPrintf(" 11- MTI_100 - 10G_R1         \n");
        hwsOsPrintf(" 12- MTI_USX - 10G_OUSGMII    \n");
        hwsOsPrintf(" 13- MTI_USX - 10G_R1         \n");
        hwsOsPrintf("\n");
        return GT_OK;
    }
    else if(portType == 1) /* MTI100 25R port */
    {
        iterNum = sizeof(phoenixMti100_25G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_25G_R1_RegDb;
     }
    else if(portType == 2) /* USX 25G port*/
    {
        iterNum = sizeof(phoenixMtiUsx_25G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_25G_R1_RegDb;
    }
    else if(portType == 3) /* USX 20G_OXGMII port*/
    {
        iterNum = sizeof(phoenixMtiUsx_20G_OXGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_20G_OXGMII_RegDb;
    }
    else if(portType == 4) /* CPU 25G port*/
    {
        iterNum = sizeof(phoenixCpu_25G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixCpu_25G_R1_RegDb;
    }
    else if(portType == 5) /* CPU 10G port*/
    {
        iterNum = sizeof(phoenixCpu_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixCpu_10G_R1_RegDb;
    }
    else if(portType == 6) /* MTI100 50G_R2 */
    {
        iterNum = sizeof(phoenixMti100_50G_R2_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_50G_R2_RegDb;
    }
    else if(portType == 7) /* MTI100 100G_R4 port */
    {
        iterNum = sizeof(phoenixMti100_100G_R4_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_100G_R4_RegDb;
    }
    else if(portType == 8) /* MTI100 SGMII*/
    {
        iterNum = sizeof(phoenixMti100_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_SGMII_RegDb;
    }
    else if(portType == 9) /* MTIUSX SGMII*/
    {
        iterNum = sizeof(phoenixMtiUSX_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUSX_SGMII_RegDb;
    }
    else if(portType == 10) /* MTI100 1000BaseX*/
    {
        iterNum = sizeof(phoenixMti100_1000BaseX_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_1000BaseX_RegDb;
    }
    else if(portType == 11) /* MTI100 10G_R1*/
    {
        iterNum = sizeof(phoenixMti100_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_10G_R1_RegDb;
    }
    else if(portType == 12) /* MTI USX 10G OUSGMII */
    {
        iterNum = sizeof(phoenixMtiUsx_10G_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_10G_OUSGMII_RegDb;
    }
    else if(portType == 13) /* USX 10G port*/
    {
        iterNum = sizeof(phoenixMtiUsx_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_10G_R1_RegDb;
    }
    else
    {
        hwsOsPrintf("ILLEGAL PORT TYPE\n");
        return GT_FAIL;
    }
    hwsOsPrintf("iterNum = %d\n\n", iterNum);

    for(i = 0; i < iterNum; i++)
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(0, 0, regPtr[i].regAddr, &regData, 0xFFFFFFFF));
        if ((regData & regPtr[i].mask) != (regPtr[i].expectedData & regPtr[i].mask))
        {
            hwsOsPrintf("Found mismatch: iter = %d, regAddr = 0x%x, data = 0x%x, expectedData = 0x%x\n", i, regPtr[i].regAddr, (regData& regPtr[i].mask), (regPtr[i].expectedData& regPtr[i].mask));
        }
    }

    return GT_OK;
}

GT_STATUS mvHwsPhoenixPunktUnitBaseGet
(
    GT_U32                  macPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS rc;
    MV_HWS_UNITS_ID unitId,unitId1;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          unitAddr = 0;
    GT_U32          localUnitNum = 0;
    GT_U32          unitIndex = 0;

    switch (portMode)
    {
        case _50GBase_KR:
        case _10GBase_KR:
        case _25GBase_KR:
        case _100GBase_KR4:
        case _107GBase_KR4:
        case _100GBase_KR2:
        case _106GBase_KR2:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10GBase_KR /*NON SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_400_UNIT;
            break;

        case _2_5G_SXGMII:
        case _5G_SXGMII:
        case _10G_SXGMII:
        case _5G_DXGMII:
        case _10G_DXGMII:
        case _20G_DXGMII:
        case _5G_QUSGMII:
        case _10G_QXGMII:
        case _20G_QXGMII:
        case _10G_OUSGMII:
        case _20G_OXGMII:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10G_OUSGMII /*USX PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_USX_UNIT;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }
    if(mvHwsMtipIsReducedPort(0,macPortNum) == GT_TRUE)
    {
        unitId = MIF_CPU_UNIT;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK))
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    hwsOsPrintf("MIF UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    if (mvHwsUsxModeCheck(0, macPortNum, portMode))
    {
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            unitId = ANP_USX_O_UNIT;
        }
        else
        {
            unitId = ANP_USX_UNIT;
        }
        unitId1 = AN_USX_UNIT;
    }
    else if(mvHwsMtipIsReducedPort(0,macPortNum) == GT_TRUE)
    {
        unitId = ANP_CPU_UNIT;
        unitId1 = AN_CPU_UNIT;
    }
    else
    {
        unitId = ANP_400_UNIT;
        unitId1 = AN_400_UNIT;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
    {
        return GT_FAIL;
    }

    hwsOsPrintf("ANP UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    rc = mvUnitExtInfoGet(0, unitId1, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
    {
        return GT_FAIL;
    }

    hwsOsPrintf("AN UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    return GT_OK;
}

#endif

