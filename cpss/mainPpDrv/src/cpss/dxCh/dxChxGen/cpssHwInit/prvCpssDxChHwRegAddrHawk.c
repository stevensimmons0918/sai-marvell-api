/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChHwRegAddrHawk.c
*
* @brief This file implement DB of units base addresses for Hawk.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct{
    PRV_CPSS_DXCH_UNIT_ENT pipe0Id;
    GT_U32                 pipe0baseAddr;
    GT_U32                 sizeInBytes;
}UNIT_IDS_STC;

#define MG_SIZE (1   * _1M)
#define NUM_MG_PER_CNM   4

/* base address of the CNM unit that is single for 2 tiles */
#define CNM_OFFSET_CNS       0x3C000000
#define RUNIT_OFFSET_CNS     0x3D000000

/* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
#define DP0_GOP_CPU_NETWORK_PORT    104
#define DP1_GOP_CPU_NETWORK_PORT    105

#define GOP_INDEX_CPU_NETWORK_PORT    1/* GOP index DP index = 1 */
#define GOP_NUM_OF_50G_PORTS_PER_DP   8

/* There are up to 7 LED servers/units in Hawk and only one LED interface */
#define PRV_CPSS_DXCH_AC5P_LED_UNIT_NUM_CNS                           7
/* There are up to 32 LED ports per one LED server.
   Only 27 LED ports (26 network ports + 1 CPU) are in use */
#define PRV_CPSS_DXCH_AC5P_LED_UNIT_PORTS_NUM_CNS                     32

#define GM_STEP_BETWEEN_MIB_PORTS   0x400
#define GM_STEP_BETWEEN_DPS         _64K

/*
NOTE: last alignment according to excel file in:

http://webilsites/sites/EBUSites/Switching/VLSIDesign/ChipDesign/Projects/Aldrin3/
Shared Documents/Design/AXI/Hawk_Address_Space_190730.xlsm

*/
static const UNIT_IDS_STC hawkUnitsIdsInPipes[] =
{
    /****************************************************************/
    /* NOTE: the table is sorted according to the base address ...  */
    /* BUT excluding the 'dummy' GOP addresses                      */
    /****************************************************************/

    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
                                     /*pipe0baseAddr*//*sizeInBytes*/
     {PRV_CPSS_DXCH_UNIT_LPM_E           ,0x18000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPVX_E          ,0x17800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_L2I_E           ,0x19000000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_CNC_0_E         ,0x13400000,     256 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNC_1_E         ,0x13800000,     256 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNC_2_E         ,0x13C00000,     256 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNC_3_E         ,0x14000000,     256 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCL_E           ,0x13000000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPLR_E          ,0x0DC00000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EOAM_E          ,0x0F600000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_E          ,0x0E400000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PREQ_E          ,0x0E600000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_ERMRK_E         ,0x0E000000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IOAM_E          ,0x17000000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_MLL_E           ,0x17400000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_E          ,0x15800000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,0x15C00000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EQ_E            ,0x16000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,0x14800000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,0x14C00000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,0x15000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TTI_E           ,0x0C800000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PPU_E           ,0x15400000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_EREP_E          ,0x0F000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_BMA_E           ,0x0F200000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_HBU_E           ,0x0F400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_IA_E            ,0x09000000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E         ,0x08000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA1_E        ,0x08400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA2_E        ,0x08800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA3_E        ,0x08C00000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E         ,0x09800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,0x09C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA1_E        ,0x0A000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO1_E      ,0x0A400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA2_E        ,0x0A800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO2_E      ,0x0AC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA3_E        ,0x0B000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO3_E      ,0x0B400000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,0x04000000,     128  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E,0x04C00000,     128  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,0x04400000,     512  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E,0x05000000,     512  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,0x04800000,     64   * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E,0x05400000,     64   * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E,0x06000000,     128  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E,0x06C00000,     128  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E,0x06400000,     512  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E,0x07000000,     512  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E,0x06800000,     64   * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E,0x07400000,     64   * _1K }

    ,{PRV_CPSS_DXCH_UNIT_HA_E            ,0x0E800000,     6   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PHA_E           ,0x0D000000,     8   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_TCAM_E                       ,0x10800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,0x03800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                   ,0x03C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_SHM_E                        ,0x12C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EM_E                         ,0x12400000,      4  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_FDB_E                        ,0x12800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,0x03E00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x00000000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_COUNTER_E                 ,0x00200000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,0x00400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E     ,0x00600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_2_E     ,0x00800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_3_E     ,0x00A00000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x00C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,0x01400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E     ,0x01600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x01C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E   ,0x01E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E  ,0x00E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_2_E  ,0x01000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_3_E  ,0x01200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E     ,0x01800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E     ,0x01A00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E   ,0x02000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E   ,0x02200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,0x02400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E     ,0x02600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x02800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E     ,0x02C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E     ,0x03000000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E                  ,0x34A00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E                  ,0x35200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E                  ,0x32400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E                  ,0x33400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E                 ,0x34B00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_1_E                 ,0x35300000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_2_E                 ,0x32500000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_3_E                 ,0x33500000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E                  ,0x34000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_1_E                  ,0x34400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_2_E                  ,0x32000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_3_E                  ,0x33000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  ,0x34800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E                  ,0x35000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E                  ,0x32300000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E                  ,0x33300000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         ,0x36000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E         ,0x34200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E         ,0x36400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_1_E         ,0x34600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E         ,0x32100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_2_E         ,0x32200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E         ,0x33100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_3_E         ,0x33200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     ,0x34C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     ,0x36100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     ,0x35800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     ,0x36800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E     ,0x35400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E     ,0x36500000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E     ,0x35C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E     ,0x36C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E     ,0x32600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E     ,0x32A00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E     ,0x32800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E     ,0x32C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E     ,0x33600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E     ,0x33A00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E     ,0x33800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E     ,0x33C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E     ,0x36200000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E     ,0x34D00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E     ,0x36A00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E     ,0x35A00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E     ,0x36600000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E     ,0x35500000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E     ,0x36E00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E     ,0x35E00000,      1  * _1M }

    ,{PRV_CPSS_DXCH_UNIT_DFX_SERVER_E    ,PRV_CPSS_AC5P_DFX_BASE_ADDRESS_CNS,      1  * _1M }



    ,{PRV_CPSS_DXCH_UNIT_TXQS_TAI0_E          ,0x05800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQS_TAI1_E          ,0x05C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQS1_TAI0_E          ,0x07800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQS1_TAI1_E          ,0x07C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TAI_E           ,0x0BC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TAI1_E          ,0x0BE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TTI_TAI0_E      ,0x0CC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TTI_TAI1_E      ,0x0CE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PHA_TAI0_E          ,0x0D800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PHA_TAI1_E          ,0x0DA00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI0_E          ,0x0F800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI1_E          ,0x0FA00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI0_E          ,0x1AC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI1_E          ,0x1AE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G0_TAI0_E          ,0x1BC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G0_TAI1_E          ,0x1BE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G1_TAI0_E          ,0x1CC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G1_TAI1_E          ,0x1CE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G2_TAI0_E          ,0x1DC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G2_TAI1_E          ,0x1DE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G3_TAI0_E          ,0x1EC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_400G3_TAI1_E          ,0x1EE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX0_TAI0_E          ,0x25C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX0_TAI1_E          ,0x25E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX1_TAI0_E      ,0x27C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX1_TAI1_E          ,0x27E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA2_TAI0_E          ,0x32E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA2_TAI1_E          ,0x32F00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA3_TAI0_E          ,0x33E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA3_TAI1_E          ,0x33F00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA0_TAI0_E          ,0x34E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA0_TAI1_E          ,0x34F00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA1_TAI0_E          ,0x35600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA1_TAI1_E          ,0x35700000,     64  * _1K }



    ,{PRV_CPSS_DXCH_UNIT_MG_E            ,PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS+MG_SIZE*0,MG_SIZE }
    ,{PRV_CPSS_DXCH_UNIT_MG_0_1_E        ,PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS+MG_SIZE*1,MG_SIZE }
    ,{PRV_CPSS_DXCH_UNIT_MG_0_2_E        ,PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS+MG_SIZE*2,MG_SIZE }
    ,{PRV_CPSS_DXCH_UNIT_MG_0_3_E        ,PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS+MG_SIZE*3,MG_SIZE }

    ,{PRV_CPSS_DXCH_UNIT_CNM_RFU_E       ,CNM_OFFSET_CNS + 0x00000000,     64  * _1K }
    /* SMI is in the CnM section */
    ,{PRV_CPSS_DXCH_UNIT_SMI_0_E         ,CNM_OFFSET_CNS + 0x00010000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_SMI_1_E         ,CNM_OFFSET_CNS + 0x00020000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNM_AAC_E       ,CNM_OFFSET_CNS + 0x00030000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_CNM_RUNIT_E     ,RUNIT_OFFSET_CNS           ,     64  * _1K }
    /* units that where in 'Raven' and now part of the 'device'  */

    /* LED CPU */
    ,{PRV_CPSS_DXCH_UNIT_LED_CPU_0_E      ,0x1A000000     , 2   *     _1K}
    /* LED 400G */
    ,{PRV_CPSS_DXCH_UNIT_LED_0_E          ,0x1B000000     , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_LED_1_E          ,0x1C000000     , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_LED_2_E          ,0x1D000000     , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_LED_3_E          ,0x1E000000     , 4   *     _1K}

    /* LED USX */
    ,{PRV_CPSS_DXCH_UNIT_LED_4_E          ,0x24000000     , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_LED_5_E          ,0x26000000     , 4   *     _1K}

    /* 400G mac */
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_0_E     ,0x1B800000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_1_E     ,0x1C800000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_2_E     ,0x1D800000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_3_E     ,0x1E800000     ,64   *     _1K}


    /* 400G pcs */
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_0_E     ,0x1F200000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_1_E     ,0x1F600000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_2_E     ,0x1FA00000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_3_E     ,0x1FE00000     ,64   *     _1K}

    /* cpu mac*/
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E      ,0x1A800000     ,16   *     _1K}
    /* cpu pcs */
    ,{PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E      ,0x1AA00000     ,16   *     _1K}


    ,{PRV_CPSS_DXCH_UNIT_SERDES_E         ,0x2E800000     ,512   * _1K     }    /* 112G SD Wrapper (8 instances)  */
    ,{PRV_CPSS_DXCH_UNIT_SERDES_1_E       ,0x1A400000     ,512   * _1K     }    /* 28G SD Wrapper (1 instances)  */

    /* mif 400G */
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_0_E     ,0x1B400000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_1_E     ,0x1C400000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_2_E     ,0x1D400000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_3_E     ,0x1E400000     ,16   *     _1K}

        /* mif USX*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_0_E      ,0x24400000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_1_E      ,0x26400000     ,16   *     _1K}

    /* mif CPU*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E      ,0x26600000     ,16   *     _1K}

    /* anp 400G */
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_0_E     ,0x1F000000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_1_E     ,0x1F400000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_2_E     ,0x1F800000     ,64   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_3_E     ,0x1FC00000     ,64   *     _1K}

    /* anp USX*/
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_0_E      ,0x2A000000     ,12   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_1_E      ,0x2C000000     ,12   *     _1M}

    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E     ,0x24800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_0_E     ,0x25000000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_0_E     ,0x25800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_1_E     ,0x26800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_1_E     ,0x27000000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_1_E     ,0x27800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E     ,0x2AC00000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_0_E     ,0x2B000000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_0_E     ,0x2B800000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_1_E     ,0x2CC00000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_1_E     ,0x2D000000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_1_E     ,0x2D800000     ,32  *  _1K}


    /* anp CPU*/
    ,{PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E      ,0x1A600000     ,64   *     _1K}

    ,{PRV_CPSS_DXCH_UNIT_SDW0_0_E         ,0x2E800000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW1_0_E         ,0x2EC00000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW0_1_E         ,0x2F000000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW1_1_E         ,0x2F400000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW0_2_E         ,0x2F800000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW1_2_E         ,0x2FC00000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW0_3_E         ,0x30000000       , 4   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_SDW1_3_E         ,0x30400000       , 4   *     _1K}


    /* dummy for MG in Ravens - for BWC interrupts */
    /* see use in file : cpssDriver\pp\interrupts\dxExMx\cpssDrvPpIntDefDxChHawk.c*/
    ,{PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS + PRV_CPSS_DXCH_UNIT_MG_E, 0x40000000, 4   * _1M     }

#ifdef GM_USED
    /********************************/
    /* dummy for GM device only !!! */
    /********************************/
    ,{PRV_CPSS_DXCH_UNIT_MIB_E,0x50000000, GM_STEP_BETWEEN_DPS * 4     }/* 4 chunks , each supports 27 ports */

#endif /*GM_USED*/

     /* must be last */
    ,{PRV_CPSS_DXCH_UNIT_LAST_E, 0,0 }
};

/*
    structure to help convert Global to Local for DMA and for GOP ports
*/
typedef struct{
    GT_U32    dpIndex;
    GT_U32    localPortInDp;
    GT_U32    globalMac;
}SPECIAL_DMA_PORTS_STC;

#define MG0_SERVED_DP   0
#define MG1_SERVED_DP   1
#define MG2_SERVED_DP   2
#define MG3_SERVED_DP   3

/* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
static const SPECIAL_DMA_PORTS_STC hawk_SpecialDma_ports[]= {
   /*104*/{GOP_INDEX_CPU_NETWORK_PORT,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,DP0_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/
   /*105*/{GOP_INDEX_CPU_NETWORK_PORT,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,DP1_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/

   /*106*/{MG0_SERVED_DP,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*107*/{MG1_SERVED_DP,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*108*/{MG2_SERVED_DP,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 2*/
   /*109*/{MG3_SERVED_DP,HAWK_PORTS_PER_DP_CNS/*localPortInDp*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 3*/
   };
static const GT_U32 num_ports_hawk_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(hawk_SpecialDma_ports);

/**
* @internal prvCpssHawkUnitIdSizeInByteGet function
* @endinternal
*
* @brief   To get the size of the unit in bytes
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] prvUnitId                - Id of DxCh unit(one of the PRV_CPSS_DXCH_UNIT_ENT)
*
* @param[out] unitIdSize               - size of the unit in bytes
*/
static GT_STATUS prvCpssHawkUnitIdSizeInByteGet
(
    IN  GT_U8                      devNum,
    IN  PRV_CPSS_DXCH_UNIT_ENT     prvUnitId,
    OUT GT_U32                    *unitIdSizePtr
)
{
    const UNIT_IDS_STC   *unitIdInPipePtr = &hawkUnitsIdsInPipes[0];
    GT_U32 ii;

    devNum = devNum;

    for(ii = 0 ; unitIdInPipePtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,unitIdInPipePtr++)
    {
        if(unitIdInPipePtr->pipe0Id == prvUnitId)
        {
            *unitIdSizePtr = unitIdInPipePtr->sizeInBytes;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "unit [%d] not supported in Hawk",
        prvUnitId);
}

typedef enum{
    MAC_CAPABILITY_50G_100G_E ,/* 50G ,100G (also lower than  50G)  */
    MAC_CAPABILITY_400G_E,/* 400G (also lower than 400G)  */
    MAC_CAPABILITY_200G_E,/* 200G (also lower than 200G)  */
    MAC_CAPABILITY_1G_E ,/* 1G ,2.5G , 5G */
    MAC_CAPABILITY_CPU_E,/* cpu port*/
    MAC_CAPABILITY_UNKNOWN_E /* error */
}MAC_CAPABILITY_ENT;

typedef struct{
    MAC_CAPABILITY_ENT macCapability;
    GT_U32             ciderUnit;
    GT_U32             ciderIndexInUnit;
    GT_U32             ciderSubUnit;

    GT_U32             dpIndex;
    GT_U32             localPortInDp;
}MAC_INFO_STC;

static GT_STATUS   macInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN HWS_UNIT_BASE_ADDR_TYPE_ENT unitId,
    OUT MAC_INFO_STC               *macInfoPtr
)
{
    devNum = devNum;

    macInfoPtr->ciderSubUnit = 0;

    if(portMacNum > DP1_GOP_CPU_NETWORK_PORT)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is unknown",
            portMacNum);
    }

    if(portMacNum >= DP0_GOP_CPU_NETWORK_PORT)
    {
        /* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
        macInfoPtr->ciderUnit        = GOP_INDEX_CPU_NETWORK_PORT;
        macInfoPtr->macCapability    = MAC_CAPABILITY_CPU_E;
        macInfoPtr->ciderIndexInUnit = 0;

        macInfoPtr->dpIndex          = macInfoPtr->ciderUnit;
        macInfoPtr->localPortInDp    = HAWK_PORTS_PER_DP_CNS;

        return GT_OK;
    }

    macInfoPtr->dpIndex       = portMacNum / HAWK_PORTS_PER_DP_CNS;
    macInfoPtr->localPortInDp = portMacNum % HAWK_PORTS_PER_DP_CNS;

    macInfoPtr->ciderUnit    = macInfoPtr->dpIndex;
    macInfoPtr->macCapability = MAC_CAPABILITY_UNKNOWN_E;

    switch (unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Unit[%d] is not supported", unitId);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E:
            switch(macInfoPtr->localPortInDp)
            {
                case 0:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_400G_E;
                    macInfoPtr->ciderIndexInUnit = 0;
                    break;
                case 1:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 1;
                    break;
                case 2:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 2;
                    break;
                case 6:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 3;
                    break;
                case 10:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_400G_E;
                    macInfoPtr->ciderIndexInUnit = 4;
                    break;
                case 14:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 5;
                    break;
                case 18:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 6;
                    break;
                case 22:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = 7;
                    break;
                default:
                    macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
                    return /* this is not error */GT_OUT_OF_RANGE;
            }
            break;


        case HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E:

            if (macInfoPtr->localPortInDp < 2)
            {
                return /* this is not error */GT_OUT_OF_RANGE;
            }
            macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp - 2;
            switch(macInfoPtr->ciderIndexInUnit%8)
            {
                case 0:
                    macInfoPtr->ciderIndexInUnit = 0;
                    break;
                case 1:
                    macInfoPtr->ciderIndexInUnit = 2;
                    break;
                case 2:
                    macInfoPtr->ciderIndexInUnit = 1;
                    break;
                case 3:
                    macInfoPtr->ciderIndexInUnit = 3;
                    break;
                case 4:
                    macInfoPtr->ciderIndexInUnit = 4;
                    break;
                case 5:
                    macInfoPtr->ciderIndexInUnit = 6;
                    break;
                case 6:
                    macInfoPtr->ciderIndexInUnit = 5;
                    break;
                case 7:
                    macInfoPtr->ciderIndexInUnit = 7;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is not implemented",
                        portMacNum);
            }
            macInfoPtr->ciderIndexInUnit += ((macInfoPtr->localPortInDp - 2)/8)*8;
            break;

        default:
            break;
    }



    return GT_OK;
}

/* offsets within the : Hawk/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_200G_400G_OFFSET      0x00002000
#define PORT0_100G_OFFSET         0x00004000

#define CPU_MAC_EXT_BASE_OFFSET   0x00000000
#define CPU_MAC_PORT_OFFSET       0x00002000
#define CPU_MAC_MIB_OFFSET        0x00001000

#define PCS_400G_OFFSET           0x00000000
#define PCS_200G_OFFSET           0x00001000
#define PCS_PORT0_100G_OFFSET     0x00002000
#define PCS_PORT1_50G_OFFSET      (PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET)
#define PCS_RS_FEC_OFFSET         0x0000a000
#define PCS_LPCS_OFFSET           0x0000b000
#define PCS_RS_FEC_STATISTICS_OFFSET 0x0000c000

#define ANP_STEP_UNIT_OFFSET      0x00200000

/**
* @internal hawkUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS to call to cpss to use for Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] unitId                   - the unit : MIB/SERDES/GOP
* @param[in] portNum                  - the global port num (MAC number)
*
* @return - The address of the port in the unit
*/
static GT_U32 /*GT_UREG_DATA*/   hawkUnitBaseAddrCalc(
    GT_U8                                   devNum,
    GT_U32/*HWS_UNIT_BASE_ADDR_TYPE_ENT*/   unitId,
    GT_U32/*GT_UOPT*/                       portNum
)
{
    GT_STATUS   rc;
    GT_U32 baseAddr;
    GT_U32 relativeAddr;
    GT_U32 dpIndex;
    MAC_INFO_STC    macInfo;
    MAC_CAPABILITY_ENT macCapability;
    PRV_CPSS_DXCH_UNIT_ENT   cpssUnitId;

    if(unitId == HWS_UNIT_BASE_ADDR_TYPE_SERDES_E)
    {
        /* the 'portNum' is actually 'SERDES number' : 0..32 (not MAC port 0..104) */
        if(portNum == 32)
        {
            /*CPU Serdes (28G unit)*/
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_SERDES_1_E), NULL);
            relativeAddr = 0;
        }
        else
        {
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_SERDES_E), NULL);

            relativeAddr = (portNum/4) * 0x400000;
        }
        return baseAddr + relativeAddr;
    }

    rc =  macInfoGet(devNum,portNum,unitId,&macInfo);
    if(rc != GT_OK)
    {
        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }
    macCapability  = macInfo.macCapability;
    dpIndex        = macInfo.dpIndex;

    switch(unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
            /* note : the 32 local ports share the same 'base' address !!! */
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E + dpIndex/*0..3*/), NULL);

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E:  /* 0..15 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E nor supported in Hawk");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E nor supported in Hawk");/* not supported !!! */
        case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E nor supported in Hawk");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:   /* 0..255 */
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:   /* 0..255 */
            if(MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = MAC_200G_400G_OFFSET + (macInfo.ciderIndexInUnit / 4) * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E), NULL);

            relativeAddr = 0;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
            baseAddr = hawkUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_PORT_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
            baseAddr = hawkUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_EXT_BASE_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
            baseAddr = hawkUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_MIB_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);

            relativeAddr = 0;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);

            relativeAddr = 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E ), NULL);

            relativeAddr = 0x2000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = 0xc000;/* shared to 8 ports */

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIB_E:
            if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            /********************************/
            /* dummy for GM device only !!! */
            /********************************/
            baseAddr     = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MIB_E, NULL);
            relativeAddr = GM_STEP_BETWEEN_DPS       * dpIndex +
                           GM_STEP_BETWEEN_MIB_PORTS * macInfo.localPortInDp;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
            if(MAC_CAPABILITY_CPU_E == macCapability)
            {
                baseAddr = hawkUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E , portNum);
                if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                {
                    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                }

                return baseAddr;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = MAC_EXT_BASE_OFFSET;/* shared to 8 ports */

            return baseAddr + relativeAddr;


        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E    :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E   :
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E   :
            if(MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_200G_OFFSET;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E   :
            if(MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_400G_OFFSET;

            return baseAddr + relativeAddr;
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_LPCS_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_RS_FEC_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E:
            if(MAC_CAPABILITY_50G_100G_E != macCapability &&
               MAC_CAPABILITY_200G_E     != macCapability &&
               MAC_CAPABILITY_400G_E     != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_RS_FEC_STATISTICS_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_USX_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit), NULL);
            return (baseAddr +  0x4000 + macInfo.ciderIndexInUnit * 0x1000);

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E), NULL);
            return (baseAddr + 0x4000);

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E   :
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E   :
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            /* There are 2 ANP units in each USX. we return the address to the first ANP in the USX */
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if (unitId == HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E)
            {
                baseAddr += (macInfo.ciderIndexInUnit/8) * 2 * ANP_STEP_UNIT_OFFSET;
            }
            else
            {
                baseAddr +=  (macInfo.ciderIndexInUnit/4) *  ANP_STEP_UNIT_OFFSET;
            }

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E   :
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            /* There are 2 ANP units in each USX. we return the address to the first ANP in the USX */
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr +=  (macInfo.ciderIndexInUnit/4) *  ANP_STEP_UNIT_OFFSET;

            return (baseAddr + 0x4000);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            relativeAddr = 0x2000 + (macInfo.ciderIndexInUnit % 8) * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x4000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x3000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E:
            if(macInfo.ciderUnit >= 2)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderIndexInUnit / 8 + 3 * macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x2000;
            return baseAddr;

        default:
            break;
    }
    /* indicate to skip this unit */
    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
}

/**
* @internal prvDxChHwRegAddrHawkDbInit function
* @endinternal
*
* @brief   init the base address manager of the Hawk device.
*         prvDxChHawkUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrHawkDbInit(GT_VOID)
{
    const UNIT_IDS_STC   *currEntryPtr = &hawkUnitsIdsInPipes[0];
    GT_U32                  globalIndex = 0;/* index into prvDxChHawkUnitsIdUnitBaseAddrArr */
    GT_U32                  ii;
    GT_U32                  errorCase = 0;
    GT_U32                  maxUnits = HAWK_MAX_UNITS;

    /* bind HWS with function that calc base addresses of units */
    /* NOTE: bind was already done by 'Falcon'
       hwsFalconUnitBaseAddrCalcBind(hawkUnitBaseAddrCalc); */

    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        if(globalIndex >= maxUnits)
        {
            errorCase = 1;
            goto notEnoughUnits_lbl;
        }

        /* add the unit in pipe 0 */
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChHawkUnitsIdUnitBaseAddrArr)[globalIndex].unitId = currEntryPtr->pipe0Id;
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChHawkUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr;
        globalIndex++;
    }

    if(globalIndex >= maxUnits)
    {
        errorCase = 1;
        goto notEnoughUnits_lbl;
    }

    /* set the 'last index as invalid' */
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChHawkUnitsIdUnitBaseAddrArr)[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChHawkUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;
    globalIndex++;

    if(globalIndex >= maxUnits)
    {
        errorCase = 4;
        goto notEnoughUnits_lbl;
    }

    /* avoid warning if 'CPSS_LOG_ENABLE' not defined
      (warning: variable 'errorCase' set but not used [-Wunused-but-set-variable])
    */
    if(errorCase == 0)
    {
        errorCase++;
    }

    return GT_OK;

notEnoughUnits_lbl:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :HAWK_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,maxUnits , globalIndex+1);
}
/**
* @internal prvCpssDxChHawkHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
static PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChHawkHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    const UNIT_IDS_STC   *currEntryPtr;
    GT_U32  ii;

    devNum = devNum;
    currEntryPtr = &hawkUnitsIdsInPipes[0];

    /****************************************/
    /* search for addrInPipe0 in the ranges */
    /****************************************/
    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        /* add the unit in pipe 0 */
        if(regAddr >= currEntryPtr->pipe0baseAddr  &&
           regAddr <  (currEntryPtr->pipe0baseAddr + currEntryPtr->sizeInBytes))
        {
            /* found the proper range */
            return currEntryPtr->pipe0Id;
        }
    }

    /* not found ! */
    return PRV_CPSS_DXCH_UNIT_LAST_E;
}

/* number of rx/tx DMA and txfifo 'per DP unit' */
#define HAWK_NUM_PORTS_DMA   (HAWK_PORTS_PER_DP_CNS + 1)

/*check if register address is per port in RxDMA unit */
static GT_BOOL  hawk_mustNotDuplicate_rxdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {

         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.cutThrough.channelCTConfig                ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelToLocalDevSourcePort ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.PCHConfig                   ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelGeneralConfigs       ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelCascadePort          ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg), HAWK_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x400000; /* offset between units */

    /* next array hold addresses in DP[0] that are global , and can't be used with addrOffset_NON_FIRST_UNIT_INDEX_CNS !!! */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalAddrArr[]=
    {
        /* next registers are accessed 'per DP index' explicitly (DP[0]) */
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[0].debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_ref  ),   1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[0].debug.rxIngressDropCounter[0].rx_ingress_drop_count_type_mask ),   1}

        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds0),  1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds1),  1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* check global */
    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    /* check per port */
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}


/*check if register address is per port in TxDMA unit */
static GT_BOOL  hawk_mustNotDuplicate_txdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile                 ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.channelReset                 ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase                 ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.interGapConfigitation        ), HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.rateLimiterEnable            ), HAWK_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x800000; /* offset between units */

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  hawk_mustNotDuplicate_txfifoAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile ),  HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase ),  HAWK_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.tagFIFOBase  ),  HAWK_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* next array hold addresses in DP[0] that are global , and can't be used with addrOffset_NON_FIRST_UNIT_INDEX_CNS !!! */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalAddrArr[]=
    {
        /* hold bit of <enable/Disable SDMA Port> that must not be duplicated */
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[0].configs.globalConfigs.globalConfig1 ) , 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x800000; /* offset between units */

    /* check global */
    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}



/* check if register address is one 'per MG' SDMA , and allow to convert address
   of MG 0 to address of other MG  */
static GT_BOOL  hawk_convertPerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         /* cover full array of : PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs */
         /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */

                    /* 4 bits per queue , no global bits */
/*per queue*/        {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxQCmdReg ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaCdp)}

                    /* 2 bits per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQCmdReg             ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txDmaCdp)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaResErrCnt)}/* the same as rxSdmaResourceErrorCountAndMode[0,1]*/
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaPcktCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaByteCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txQWrrPrioConfig)}

                    /* 1 bit per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQFixedPrioConfig    ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueConfig)}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketGeneratorConfigQueue)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketCountConfigQueue)}
                    /* the rxSdmaResourceErrorCountAndMode[0..7] not hold consecutive addresses !
                       need to split to 2 ranges [0..1] and [2..7] */
/*per queue: 0..1*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[0]) , 2}
/*per queue: 2..7*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[2]) , 6}



         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
}

/* check if register address is one MG 'per MG' for SDMA that need duplication  ... need special treatment */
static GT_BOOL  hawk_allowDuplicatePerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    /* using registers from regDb */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.sdmaCfgReg)                          ,1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txSdmaWrrTokenParameters )           ,1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* using registers from regDb1 */
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr_regDb1[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[0])        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[0] )        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[1])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[1] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[2])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[2] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[3])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[3] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[4])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[4] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[5])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[5] )        ,1}

        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.highAddressRemap)  }/*array of 6 consecutive addresses*/
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.windowControl)     }/*array of 6 consecutive addresses*/

        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.UnitDefaultID)         ,1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalRegAddrArr_regDb1,regAddr);
}

/* check if register address is one of FUQ related ... need special treatment */
static GT_BOOL  hawk_fuqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC fuqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQBaseAddr), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_generalControl), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_hostConfig    ), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,fuqRegAddrArr,regAddr);
}


static GT_BOOL mgUnitDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    INOUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT  unitId,
    INOUT PRV_CPSS_DXCH_UNIT_ENT  usedUnits[/*PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS*/],
    INOUT GT_U32                  *unitsIndexPtr,
    INOUT GT_U32                  *additionalBaseAddrPtr,
    INOUT GT_U32                  *dupIndexPtr,
    OUT   GT_BOOL                  *dupWasDonePtr,
    OUT   GT_BOOL                  *unitPerMgPtr
)
{
    GT_U32  ii;
    GT_U32  relativeRegAddr;
    GT_U32  unitsIndex    = *unitsIndexPtr;
    GT_U32  dupIndex      = *dupIndexPtr;
    GT_BOOL dupWasDone    = GT_FALSE;
    GT_BOOL unitPerMg     = GT_FALSE;
    GT_U32  portGroupIndex = 0;
    GT_U32  mgId,isSdmaRegister = 0,isSdmaRegisterAllowDup = 0;


    if(GT_TRUE == hawk_fuqSupport_mgAddr(devNum, regAddr))
    {
        /* FUQ registers need special manipulations */

        /* NOTE: we assume that every access to WRITE to those register is done
           using 'port group' indication that is not 'unaware' !!!! */
        /* we allow 'read' / 'check status' on all port groups */

        /* we get here with the address of 'MG_0_0' but we may need to
           convert it to address of 'MG_0_1'
        */
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*for MG[1]*/

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        mgId = 1;
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++; /*per MG*/
        }

        dupWasDone  = GT_TRUE;
    }
    else
    if (GT_TRUE == hawk_convertPerMg_mgSdmaAddr(devNum, regAddr))
    {
        /* allow MG[1] till MG[3] to be able to use MG[0] addresses !        */
        isSdmaRegister = 1;
    }
    else
    if (GT_TRUE == hawk_allowDuplicatePerMg_mgSdmaAddr(devNum, regAddr))
    {
        isSdmaRegister = 1;
        isSdmaRegisterAllowDup = 1;
    }
    else
    {
        return GT_FALSE;
    }

    unitPerMg = GT_TRUE;
    if(isSdmaRegister)
    {

        if(!isSdmaRegisterAllowDup &&
           portGroupId >= PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)
        {
            /* for those registers we NOT allow 'unaware mode' and NOT allow
               duplications.

               the caller must be specific about the needed 'MG unit' ...
               otherwise it will duplicate it to unneeded places !!!
            */
            /* ERROR */

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
            additionalBaseAddrPtr[dupIndex++] = GT_NA;/* will cause fatal error */
            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = portGroupId;

            dupWasDone  = GT_TRUE;

            *unitsIndexPtr = unitsIndex;
            *dupIndexPtr   = dupIndex;
            *dupWasDonePtr = dupWasDone;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TRUE, "ERROR : the MG register [0x%x] must not be duplicated 'unaware' to all MG units",
                regAddr);
        }


        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_2_E;/*MG2*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_3_E;/*MG3*/

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        mgId = 1;
        /* support the tiles 0,1 : MG 1..3 */
        /* the additionalBaseAddrPtr[] not holding the 'MG 0' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++; /*per MG*/
        }

        dupWasDone  = GT_TRUE;
    }

    *unitsIndexPtr = unitsIndex;
    *dupIndexPtr   = dupIndex;
    *dupWasDonePtr = dupWasDone;
    *unitPerMgPtr= unitPerMg;


    return GT_TRUE;
}

/**
* @internal prvCpssHawkDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Hawk device.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssHawkDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;/* index in additionalBaseAddrPtr */
    GT_U32  ii;             /*iterator*/
    GT_BOOL unitPerMg;      /*indication to use 'per MG'      units */
    GT_BOOL dupWasDone;     /*indication that dup per pipe/tile/2 tiles was already done */
    PRV_CPSS_DXCH_UNIT_ENT  usedUnits[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];
    GT_U32  unitsIndex; /* index in usedUnits */
    GT_U32  pipeId;/* pipeId iterator */
    GT_U32  relativeRegAddr;/* relative register address to it's unit */

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    unitPerMg = GT_FALSE;
    dupWasDone = GT_FALSE;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];
    dupIndex = 0;

    /* call direct to prvCpssDxChHawkHwRegAddrToUnitIdConvert ... not need to
       get to it from prvCpssDxChHwRegAddrToUnitIdConvert(...) */
    unitId = prvCpssDxChHawkHwRegAddrToUnitIdConvert(devNum, regAddr);
    /* set the 'orig' unit at index 0 */
    unitsIndex = 0;
    usedUnits[unitsIndex++] = unitId;

    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == hawk_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA3_E;

            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == hawk_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA3_E;

            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == hawk_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E;

            break;


        case  PRV_CPSS_DXCH_UNIT_MG_E                         :
            if(GT_FALSE == mgUnitDuplicatedMultiPortGroupsGet(
                devNum,portGroupId,regAddr,additionalRegDupPtr,unitId,
                usedUnits,&unitsIndex,additionalBaseAddrPtr,&dupIndex,
                &dupWasDone,&unitPerMg))
            {
                return GT_FALSE;
            }

            break;

        default:
            /* we get here for 'non first instance' of duplicated units */

            /* we not need to duplicate the address */
            return GT_FALSE;
    }

    if(unitsIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,unitsIndex);
    }

    if(dupWasDone == GT_TRUE)
    {
        /********************************/
        /* no extra duplication needed  */
        /* complex logic already applied*/
        /********************************/
    }
    else
    if(unitsIndex)
    {
        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        /****************************/
        /* handle the duplications  */
        /****************************/
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        pipeId = 0;
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId;
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */
        }
    }

    if(dupIndex == 0 && unitPerMg == GT_FALSE)
    {
        return GT_FALSE;
    }

    /* support multi-pipe awareness by parameter <portGroupId> */
    switch(portGroupId)
    {
        case CPSS_PORT_GROUP_UNAWARE_MODE_CNS:
            /* allow the loops on the 2 addresses (orig+additional) to be
                accessed , each one in different iteration in the loop of :
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(...)
                or
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(...)
            */

            additionalRegDupPtr->use_originalAddressPortGroup = GT_TRUE;
            additionalRegDupPtr->originalAddressPortGroup = 0; /* pipe 0 */

            additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
            *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            break;
        default:
            if(unitPerMg == GT_TRUE  && portGroupId < PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)
            {
                if(portGroupId == 0)
                {
                    *portGroupsBmpPtr = BIT_0;
                    dupIndex = 0;/* access only to MG 0 --> no duplications */
                }
                else
                {
                    *portGroupsBmpPtr = (BIT_0 << portGroupId);
                    /* access only to this pipe (1/2/3/4/5/6/7) */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    /* the only valid address is the one in additionalBaseAddrPtr[0] */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                }
            }
            else
            {
                /* should not get here */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : unsupported port group id [%d] ",
                    portGroupId);
            }
            break;
    }

    if(dupIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,dupIndex);
    }

    if(unitPerMg == GT_TRUE)
    {
        /******************************************/
        /* mask the MGs BMP with existing MGs BMP */
        /******************************************/
        *portGroupsBmpPtr &= (1<<PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)-1;

        /* state the caller not to do more mask */
        additionalRegDupPtr->skipPortGroupsBmpMask = GT_TRUE;
    }
    else
    {
        *portGroupsBmpPtr &= 0x1;
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;
    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}


/**
* @internal prvCpssHawkOffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
static GT_U32   prvCpssHawkOffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
)
{
    GT_U32  numOfDpPerPipe;
    GT_U32  relativeInstanceInPipe0;
    GT_U32  pipe0UnitId;
    GT_U32  instance0BaseAddr,instanceInPipe0BaseAddr;

    /* use 'PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]' because it may be called from the cpssDriver
       before the 'PRV_CPSS_PP_MAC(devNum)' initialized ! */
    switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            numOfDpPerPipe = 4;
            break;
        default:
            /* was not implemented */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA, LOG_ERROR_NO_MSG);
    }

    switch(instance0UnitId)
    {
        default:
            pipe0UnitId = instance0UnitId;
            break;

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E               :
        {
            /* tile 0 : MG 0,1,2,3 - serve 'tile 0' located at tile 0 memory */
            GT_U32  tileOffset        = instanceId / NUM_MG_PER_CNM;
            GT_U32  internalCnmOffset = (MG_SIZE            * (instanceId % NUM_MG_PER_CNM));

            return tileOffset + internalCnmOffset;
        }

        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_RXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXDMA_E            :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO_E          :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E; break;
            }
            break;

         case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E; break;
            }
            break;
    }

    /* we got here for units that are per DP */
    /* 1. get the offset of the unit from first instance in Pipe 0 */
    /*  1.a get address of first  instance */
    instance0BaseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,instance0UnitId,NULL);
    /*  1.b get address of needed instance (in pipe 0)*/
    instanceInPipe0BaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,pipe0UnitId,NULL);

    return  instanceInPipe0BaseAddr - instance0BaseAddr;
}


/**
* @internal hawkSpecialPortsMapGet function
* @endinternal
*
* @brief   Hawk : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS hawkSpecialPortsMapGet
(
    OUT const SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    *specialDmaPortsPtrPtr   =           hawk_SpecialDma_ports;
    *numOfSpecialDmaPortsPtr = num_ports_hawk_SpecialDma_ports;

    return GT_OK;
}

/**
* @internal prvCpssHawkDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Hawk : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssHawkDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    if(globalDmaNum < numRegularPorts)
    {
        if(dpIndexPtr)
        {
            *dpIndexPtr     = globalDmaNum / HAWK_PORTS_PER_DP_CNS;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr = globalDmaNum % HAWK_PORTS_PER_DP_CNS;
        }
    }
    else
    {
        rc = hawkSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(globalDmaNum >= (numRegularPorts + numOfSpecialDmaPorts))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global DMA number [%d] is >= [%d] (the max)",
                globalDmaNum,
                numRegularPorts + numOfSpecialDmaPorts);
        }

        if(dpIndexPtr)
        {
            *dpIndexPtr       = specialDmaPortsPtr[globalDmaNum-numRegularPorts].dpIndex;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr   = specialDmaPortsPtr[globalDmaNum-numRegularPorts].localPortInDp;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssHawkDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Hawk : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
static GT_STATUS prvCpssHawkDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    if(localDmaNum > HAWK_PORTS_PER_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "local DMA number [%d] > [%d] (the max)",
            localDmaNum,HAWK_PORTS_PER_DP_CNS);
    }

    if(localDmaNum < HAWK_PORTS_PER_DP_CNS)
    {
        *globalDmaNumPtr = (HAWK_PORTS_PER_DP_CNS*dpIndex) + localDmaNum;
        return GT_OK;
    }

    rc = hawkSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex == dpIndex)
        {
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    CPSS_LOG_INFORMATION_MAC("dpIndex[%d] and localDmaNum[%d] not found as existing DMA",
    dpIndex,localDmaNum);

    return /*do not log*/GT_NOT_FOUND;
}

/**
* @internal prvCpssHawkDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Hawk : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*                                       if 'current' is GT_NA (0xFFFFFFFF) --> meaning need to 'get first'
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
static GT_STATUS prvCpssHawkDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    GT_U32  globalDmaNum;

    /* check validity and get 'first globalDmaNum' */
    rc = prvCpssHawkDmaLocalNumInDpToGlobalNumConvert(devNum,
        dpIndex,localDmaNum,&globalDmaNum);

    if(rc != GT_OK)
    {
        /* there is error with {dpIndex,localDmaNum} ... so no 'first' and no 'next' ...*/
        return rc;
    }

    if(GT_NA == (*globalDmaNumPtr))
    {
        /* get first global DMA that match {dpIndex,localDmaNum} */
        *globalDmaNumPtr = globalDmaNum;
        return GT_OK;
    }

    /* check if there is 'next' */
    if(localDmaNum < HAWK_PORTS_PER_DP_CNS)
    {
        /* regular port without muxing */

        return /* not error for the LOG */ GT_NO_MORE;
    }

    /* use the pointer as 'IN' parameter */
    globalDmaNum = *globalDmaNumPtr;

    rc = hawkSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    /*************************************************/
    /* start looking from 'next' index after current */
    /*************************************************/
    ii = (globalDmaNum-numRegularPorts) + 1;

    for(/*already init*/ ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex == dpIndex)
        {
            /****************/
            /* found 'next' */
            /****************/
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    /********************/
    /* NOT found 'next' */
    /********************/

    return /* not error for the LOG */ GT_NO_MORE;
}

/* return the BMP of local macs that may effect the local mac */
static GT_U32   macBmpEclipseSerdesBehindGet(IN GT_U32 ciderIndexInUnit)
{
    /*0,1,2,6,10,14,18,22*/
    /*0,-,2,-,10,--,18,--*/
    static GT_U32  macBmpEclipseSerdesBehind[4] = {
        BIT_0,
        BIT_0 | BIT_2,
        BIT_0 | BIT_2 | BIT_10,
        BIT_0 | BIT_2 | BIT_10 | BIT_18
    };

    return macBmpEclipseSerdesBehind[ciderIndexInUnit / 2];
}

/* return the BMP of local macs that may be effected by the local mac */
static GT_U32   macBmpEclipseSerdesAHeadGet(IN GT_U32 ciderIndexInUnit , IN GT_U32 numOfActLanes)
{
    GT_U32  ii;
    GT_U32  bmp = 0;
    static GT_U32   macBmpEclipseSerdesAHead[8] = {0,1,2,6,10,14,18,22};

    for(ii = ciderIndexInUnit ; (ii < (ciderIndexInUnit+numOfActLanes)) && (ii < 8) ; ii++)
    {
        bmp |= 1 << macBmpEclipseSerdesAHead[ii];
    }

    return bmp;
}
/* return the SERDES global id for the global MAC */
static GT_U32  macSerdesGet(
    IN GT_U32                      portMacNum
)
{
    GT_U32  startGlobalSerdes;
    GT_U32  localSerdes;
    if(portMacNum >= DP0_GOP_CPU_NETWORK_PORT)/* DP0_GOP_CPU_NETWORK_PORT or DP1_GOP_CPU_NETWORK_PORT*/
    {
        /* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
        return 32;
    }

    startGlobalSerdes = (portMacNum / HAWK_PORTS_PER_DP_CNS) * 8;

    switch(portMacNum % HAWK_PORTS_PER_DP_CNS)
    {
        case 0:     localSerdes = 0; break;
        case 1:     localSerdes = 1; break;
        case 2:
        case 3:
        case 4:
        case 5:     localSerdes = 2; break;
        case 6:
        case 7:
        case 8:
        case 9:     localSerdes = 3; break;
        case 10:
        case 11:
        case 12:
        case 13:    localSerdes = 4; break;
        case 14:
        case 15:
        case 16:
        case 17:    localSerdes = 5; break;
        case 18:
        case 19:
        case 20:
        case 21:    localSerdes = 6; break;
        /*22..25*/
        default:    localSerdes = 7; break;
    }

    return startGlobalSerdes + localSerdes;
}


/**
* @internal prvCpssHawkRegDbInfoGet function
* @endinternal
*
* @brief   function to get the info to index in 'reg DB'
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum          - The PP's device number.
* @param[in] portMacNum      - the global mac port number.
* @param[out] regDbType      - the type of regDbInfo.
* @param[out] regDbInfoPtr   - (pointer to) the reg db info
* @return - GT_OK on success
*/
static GT_STATUS   prvCpssHawkRegDbInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN PRV_CPSS_REG_DB_TYPE_ENT    regDbType,
    IN PRV_CPSS_REG_DB_INFO_STC   *regDbInfoPtr
)
{
    GT_STATUS       rc;
    MAC_INFO_STC    macInfo;
    HWS_UNIT_BASE_ADDR_TYPE_ENT     unitId;

    switch(regDbType)
    {
        case PRV_CPSS_REG_DB_TYPE_MTI_MIB_COUNTER_SHARED_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
        case PRV_CPSS_REG_DB_TYPE_PB_CHANNEL_ID_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_FIRST_SERDES_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_SERDES_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_USX_MTI_MIB_COUNTER_SHARED_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_USX_EXT_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(regDbType);
    }
    rc =  macInfoGet(devNum,portMacNum,unitId,&macInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    regDbInfoPtr->macBmpEclipseSerdes = 0;/* don't care */
    switch(regDbType)
    {
        case PRV_CPSS_REG_DB_TYPE_MTI_MIB_COUNTER_SHARED_E:
            /* shared per 8 ports , and index : */
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit;
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_USX_MTI_MIB_COUNTER_SHARED_E:
            /* shared per 8 ports , and index : */
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit * 3 + (macInfo.ciderIndexInUnit / 8);
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit % 8;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_USX_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit; /* index in PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[_lmu] */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesBehindGet(regDbInfoPtr->ciderIndexInUnit);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes =
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[macInfo.ciderUnit].dataPathFirstPort;
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesAHeadGet(regDbInfoPtr->ciderIndexInUnit ,
                regDbInfoPtr->startMacIn_macBmpEclipseSerdes/*numOfActLanes*/);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes =
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[macInfo.ciderUnit].dataPathFirstPort;
            break;
        case PRV_CPSS_REG_DB_TYPE_PB_CHANNEL_ID_E:
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit;       /* not used */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            break;

        case PRV_CPSS_REG_DB_TYPE_FIRST_SERDES_E:
            regDbInfoPtr->regDbIndex       = 0;       /* not used */
            regDbInfoPtr->ciderIndexInUnit = 0;       /* not used */
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = macSerdesGet(portMacNum);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(regDbType);
    }

    return GT_OK;
}

extern GT_U32 debug_force_numOfDp_get(void);

GT_VOID prvCpssHawkNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
)
{
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssHwsUnitBaseAddrCalcFunc)        = hawkUnitBaseAddrCalc;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssRegDbInfoGetFunc)               = prvCpssHawkRegDbInfoGet;
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssHawkDmaGlobalNumToLocalNumInDpConvert;
    DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(devNum)= prvCpssHawkDmaLocalNumInDpToGlobalNumConvert;
}


/**
* @internal prvCpssHawkInitParamsSet function
* @endinternal
*
* @brief  Hawk  : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssHawkInitParamsSet
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  ii,index;
    GT_U32  debug_force_numOfDp    = debug_force_numOfDp_get();
    GT_U32  lanesNumInDev;
    GT_U32  numMgUnitsPerTile = 4;
    GT_U32  numMgUnits = numMgUnitsPerTile;
    GT_U32  frameLatencyArrHawk[]        =  {0x798,0x798,0x798,0x798,0x798,0x798,0x798,0x798,0x798,0x798,0x79C,0x79C,0x798,0x798,0x798,0x798};
    GT_U32  frameLatencyFracArrHawk[]    =  { 0x40, 0x36, 0x36, 0x40, 0x40, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xCE, 0xCE, 0xF4, 0xF4, 0xF4, 0xF4};


    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits = numMgUnits;

    /* single AUQ  */
    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp     |= BIT_0;
    /* single FUQ  */
    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp     |= BIT_0;

    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgCoreClock         = 312500000;


    devPtr->multiPipe.numOfPipesPerTile = 0;/* non - tile device */
    devPtr->multiPipe.tileOffset        = 0;/* non - tile device */
    devPtr->multiPipe.mirroredTilesBmp  = 0;/* non - tile device */

    devPtr->multiPipe.numOfPipes        = 0;/* non - pipes device */
    devPtr->multiPipe.numOfPortsPerPipe = 0;/* non - pipes device */

    dxDevPtr->hwInfo.multiDataPath.maxDp = 4;/* 4 DP[] in the device */

    if(debug_force_numOfDp)
    {
        dxDevPtr->hwInfo.multiDataPath.maxDp = debug_force_numOfDp;
        devPtr->multiPipe.numOfPortsPerPipe  = 0;/* non - pipes device */
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts =
        dxDevPtr->hwInfo.multiDataPath.maxDp * 2;/* 2 segmented port per DP */

    /* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts = 1;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts =
        dxDevPtr->hwInfo.multiDataPath.maxDp * HAWK_PORTS_PER_DP_CNS;


    lanesNumInDev = 1/*10G*/ + dxDevPtr->hwInfo.multiDataPath.maxDp * GOP_NUM_OF_50G_PORTS_PER_DP/*50G*/;
    PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = lanesNumInDev;

    /* led info */
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum = PRV_CPSS_DXCH_AC5P_LED_UNIT_NUM_CNS;
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts   = PRV_CPSS_DXCH_AC5P_LED_UNIT_PORTS_NUM_CNS;


    /* get info for 'SDMA CPU' port numbers */
    rc = hawkSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    index = 0;
    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].globalMac == GT_NA)
        {
            /* this is valid 'SDMA CPU' port */
            if(index >= NUM_MG_PER_CNM)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "too many SDMA ports in specialDmaPortsPtr[]");
            }
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum = numRegularPorts + ii;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].valid  = GT_TRUE;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].usedAsCpuPort = GT_FALSE;/* the 'port mapping' should bind it */
            /* parameter called 'dqNum' but should be considered as 'DP index' */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dqNum  = specialDmaPortsPtr[ii].dpIndex;
            index++;
        }
    }

    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = ii * HAWK_PORTS_PER_DP_CNS;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = HAWK_PORTS_PER_DP_CNS + 1;/* support CPU port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum      = HAWK_PORTS_PER_DP_CNS;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks = 30;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 10*_1K;/*14K  in Falcon */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap = _32K;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = 640;/* hawk : 640 lines , in non shared block */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  = 20;/* hawk : up to 20 blocks from non shared (to fill to total of 30) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = 30;/* hawk : 30 blocks supported */

    if(CPSS_AC5P_98DX4504_DEVICES_CHECK_MAC(PRV_CPSS_PP_MAC(devNum)->devType))
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = 28;
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL2Ecmp = _2K;
    }

    /* PHA info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg =  4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn = 10;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts = 108;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues = CPSS_DXCH_SIP6_10_SDQ_QUEUE_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups = CPSS_DXCH_SIP_6_10_MAX_Q_GROUP_SIZE_MAC;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxPizzaSize =  92;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxNumOfSpeedProfiles = MAX_SPEED_PROFILE_NUM_HAWK_CNS;

    /* SBM info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm = 10;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm = 4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.bmpSbmSupportMultipleArp = BIT_8 | BIT_9;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm = _16K;/*64K in 4 SBMs*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort = 8;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId = 5;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors = 16; /* 16*3K == 48K@10B == 24K@20B */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits = 4;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum = _64K;

    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits = 4;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart  =  _64K;/*32K Falcon*//* used only by ENTRY_TYPE_TUNNEL_START_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb          = _256K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm           = _256K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesPortIsolation= _4K+128; /* used only by CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E/CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx         = _12K; /* used only by CPSS_DXCH_TABLE_MULTICAST_E */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam          =  _2K; /* Hawk  2K */

    /* only DP/PCA 0,1 have MACSec */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp = 0x3;

    /* state that the devices supports the preemption */
    PRV_CPSS_PP_MAC(devNum)->preemptionSupported = GT_TRUE;

    /* state which MACs are capable of supporting the 'preemption' */
    {
        static GT_U32  localMti100Macs[8] = {0,1,2,6,10,14,18,22};
        GT_U32 globalMac;

        for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
        {
            for(index = 0; index < 8; index++)
            {
                globalMac = HAWK_PORTS_PER_DP_CNS * ii + localMti100Macs[index] ;
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[globalMac].preemptionSupported = GT_TRUE;
            }
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits = PRV_CPSS_NUM_OF_TAI_IN_HAWK_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr = cpssOsMalloc((PRV_CPSS_NUM_OF_TAI_IN_HAWK_CNS-1)*sizeof(GT_U32));
    cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr, frameLatencyArrHawk, sizeof(frameLatencyArrHawk));
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr = cpssOsMalloc((PRV_CPSS_NUM_OF_TAI_IN_HAWK_CNS-1)*sizeof(GT_U32));
    cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr, frameLatencyFracArrHawk, sizeof(frameLatencyFracArrHawk));
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz = 800000;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssUnitIdSizeInByteGetFunc        = prvCpssHawkUnitIdSizeInByteGet;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssOffsetFromFirstInstanceGetFunc = prvCpssHawkOffsetFromFirstInstanceGet;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc = prvCpssHawkDmaLocalNumInDpToGlobalNumConvert_getNext;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssHwRegAddrToUnitIdConvertFunc   = prvCpssDxChHawkHwRegAddrToUnitIdConvert;
    prvCpssHawkNonSharedHwInfoFuncPointersSet(devNum);

    return GT_OK;
}


