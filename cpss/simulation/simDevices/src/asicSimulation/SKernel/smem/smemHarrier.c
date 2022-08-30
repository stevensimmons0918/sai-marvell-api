/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemHarrier.c
*
* DESCRIPTION:
*       Harrier memory mapping implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemHarrier.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <common/Utils/Math/sMath.h>

/* 2 MG units */
#define HARRIER_NUM_MG_UNITS  2
/* 3 DP units */
#define HARRIER_NUM_DP_UNITS  3


/* the size in bytes of the MG unit */
#define MG_SIZE             _1M
/* base address of the CNM unit that is single for 2 tiles */
#define CNM_OFFSET_CNS       0x3C000000
/* base address of the MG 0_0 unit . MG_0_0 to MG_0_3 serve tile 0 */
#define MG_0_0_OFFSET_CNS    (CNM_OFFSET_CNS + 0x00200000)
/* base address of the MG 0_1 unit . MG_0_1 is part of tile 0 */
#define MG_0_1_OFFSET_CNS    (MG_0_0_OFFSET_CNS + 1*MG_SIZE)
/* used for GM devices */
GT_U32   simHarrierMgBaseAddr = MG_0_0_OFFSET_CNS;

#define LMU_0_BASE_ADDR 0x0e100000
#define LMU_1_BASE_ADDR 0x10100000
#define LMU_2_BASE_ADDR 0x00500000

#define TSU_0_BASE_ADDR 0x0e200000
#define TSU_1_BASE_ADDR 0x10200000
#define TSU_2_BASE_ADDR 0x00600000

#define MAC_MTIP_MAC_0_BASE_ADDR  0x0d600000
#define MAC_MTIP_MAC_1_BASE_ADDR  0x0f600000
#define MAC_MTIP_MAC_2_BASE_ADDR  0x01200000

#define MAC_MTIP_MAC_PCS_0_BASE_ADDR  0x0d900000
#define MAC_MTIP_MAC_PCS_1_BASE_ADDR  0x0f900000
#define MAC_MTIP_MAC_PCS_2_BASE_ADDR  0x01500000


#define MIF_0_BASE_ADDR           0x0d480000 /*400G_0*/
#define MIF_1_BASE_ADDR           0x0f480000 /*400G_1*/
#define MIF_2_BASE_ADDR           0x01080000 /*400G_2*/

#define ANP_0_BASE_ADDR           0x0d800000
#define ANP_1_BASE_ADDR           0x0f800000
#define ANP_2_BASE_ADDR           0x01400000

#define NUM_PORTS_PER_UNIT        8
#define NUM_PORTS_PER_DP_UNIT     26 /*+1 for CPU ---> NOTE: DP unit kept 'as as' from Hawk !!! */
#define NUM_PORTS_FOR_NUMBERRING_DP_UNIT     16 /*DP0-16 ports , DP1-16 ports , DP2-8 ports */
#define RS_FEC_STATISTIC_CAPTURED_COUNTER_INDEX 20

/* offsets within the : Hawk/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x0000C000 /* moved to allow larger unit */
#define MAC_200G_400G_OFFSET      0x00002000
#define PORT0_100G_OFFSET         0x00004000
#define EMAC_OFFSET               0x00000100

#define START_MIB_RX_PER_PORT 0x00000100
#define SIZE_MIB_RX_PER_PORT  (43*4) /*decimal*/
#define SIZE_MIB_RX  (SIZE_MIB_RX_PER_PORT*NUM_PORTS_PER_UNIT*2/*EMAC/PMAC*/)
#define START_MIB_TX_PER_PORT (START_MIB_RX_PER_PORT + SIZE_MIB_RX)
#define SIZE_MIB_TX_PER_PORT  (34*4) /*decimal*/
#define SIZE_MIB_TX  (SIZE_MIB_TX_PER_PORT*NUM_PORTS_PER_UNIT*2/*EMAC/PMAC*/)
#define MAC_MIB_UNIT_SIZE       0x2000


#define MAC_STEP_PORT_OFFSET      0x00001000

/********** start : PCS for 400G *********/
#define PCS_400G_OFFSET           0x00000000
#define PCS_200G_OFFSET           0x00001000
#define PCS_PORT0_100G_OFFSET     0x00002000
#define PCS_PORT1_50G_OFFSET      (PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET)
#define PCS_RS_FEC_OFFSET         0x0000a000
#define PCS_LPCS_OFFSET           0x0000b000
#define PCS_RS_FEC_STATISTICS_OFFSET 0x0000c000

#define PCS_STEP_PORT_OFFSET      0x00001000
#define LPCS_STEP_PORT_OFFSET     0x00000080
/********** end  : PCS for 400G *********/

#define MIF_GLOBAL_OFFSET             0x00000000
#define MIF_RX_OFFSET                 0x00000800
#define MIF_TX_OFFSET                 0x00001000

#define ANP_OFFSET                    0x00000000
#define ANP_STEP_PORT_OFFSET          0x000002a4
#define AN_OFFSET                     0x00004000
#define ANP_STEP_UNIT_OFFSET          0x00200000

#define HARRIER_PSI_SCHED_OFFSET         (GT_U32)0x00000300

static void smemHarrierUnitEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
);
static void smemHarrierUnitPreq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
);
static void smemHarrierSpecificDeviceUnitAlloc_DIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemHarrierSpecificDeviceUnitAlloc_TXQ_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/
Projects/Harrier/Shared%20Documents/Design/Address%20Space/
Harrier_Address_Space_CC_200715.xlsm
*/
#define UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName)                                , size , 0}

/* the units of  */
static SMEM_GEN_UNIT_INFO_STC harrier_units[] =
{

    /* packet buffer subunits */
     UNIT_INFO_MAC(0x0b000000, UNIT_PB_CENTER_BLK              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0ba00000, UNIT_PB_COUNTER_BLK             ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0b100000, UNIT_PB_WRITE_ARBITER_0         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b200000, UNIT_PB_WRITE_ARBITER_1         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b300000, UNIT_PB_WRITE_ARBITER_2         ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0b400000, UNIT_PB_GPC_GRP_PACKET_WRITE_0  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b500000, UNIT_PB_GPC_GRP_PACKET_WRITE_1  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b600000, UNIT_PB_GPC_GRP_PACKET_WRITE_2  ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0b700000, UNIT_PB_GPC_GRP_CELL_READ_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b800000, UNIT_PB_GPC_GRP_CELL_READ_1     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0b900000, UNIT_PB_GPC_GRP_CELL_READ_2     ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0ad00000, UNIT_PB_GPC_GRP_PACKET_READ_0   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0ae00000, UNIT_PB_GPC_GRP_PACKET_READ_1   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0af00000, UNIT_PB_GPC_GRP_PACKET_READ_2   ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0ab00000, UNIT_PB_NEXT_POINTER_MEMO_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0ac00000, UNIT_PB_NEXT_POINTER_MEMO_1     ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0a800000, UNIT_PB_SHARED_MEMO_BUF_0       ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0a900000, UNIT_PB_SHARED_MEMO_BUF_1       ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0aa00000, UNIT_PB_SHARED_MEMO_BUF_2       ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0c700000,UNIT_TXQ_PDX            ,64 *     _1K)
    ,UNIT_INFO_MAC(0x0c800000,UNIT_TXQ_PFCC           ,64    * _1K)
    ,UNIT_INFO_MAC(0x0c900000,UNIT_TXQ_PSI            , 1  * _1M)

    ,UNIT_INFO_MAC(0x0c100000,UNIT_TXQ_SDQ0  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x0c000000,UNIT_TXQ_PDS0  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x0c200000,UNIT_TXQ_QFC0  ,64   *    _1K)
    ,UNIT_INFO_MAC(0x0c500000,UNIT_TXQ_SDQ1  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x0c400000,UNIT_TXQ_PDS1  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x0c600000,UNIT_TXQ_QFC1  ,64   *    _1K)
    ,UNIT_INFO_MAC(0x0cb00000,UNIT_TXQ_SDQ2  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x0ca00000,UNIT_TXQ_PDS2  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x0cc00000,UNIT_TXQ_QFC2  ,64   *    _1K)

    ,UNIT_INFO_MAC(0x0d000000,UNIT_RX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0f000000,UNIT_RX_DMA_1          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0be00000,UNIT_RX_DMA_2          ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0cf00000,UNIT_IA                ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0d200000,UNIT_TX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0d300000,UNIT_TX_FIFO           ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0f200000,UNIT_TX_DMA_1          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0f300000,UNIT_TX_FIFO_1         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0bc00000,UNIT_TX_DMA_2          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0bd00000,UNIT_TX_FIFO_2         ,64  *     _1K)

    ,UNIT_INFO_MAC(0x0da00000,UNIT_PCA_BRG_0              ,16  * _1K )
    ,UNIT_INFO_MAC(0x0dc00000,UNIT_PCA_PZ_ARBITER_I_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x0db00000,UNIT_PCA_PZ_ARBITER_E_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x0e000000,UNIT_PCA_SFF_0              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_0_BASE_ADDR, UNIT_PCA_LMU_0        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_0_BASE_ADDR, UNIT_PCA_CTSU_0       ,64  * _1K )
    ,UNIT_INFO_MAC(0x0fa00000,UNIT_PCA_BRG_1              ,16  * _1K )
    ,UNIT_INFO_MAC(0x0fc00000,UNIT_PCA_PZ_ARBITER_I_1     ,16  * _1K )
    ,UNIT_INFO_MAC(0x0fb00000,UNIT_PCA_PZ_ARBITER_E_1     ,16  * _1K )
    ,UNIT_INFO_MAC(0x10000000,UNIT_PCA_SFF_1              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_1_BASE_ADDR, UNIT_PCA_LMU_1        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_1_BASE_ADDR, UNIT_PCA_CTSU_1       ,64  * _1K )
    ,UNIT_INFO_MAC(0x00100000,UNIT_PCA_BRG_2              ,16  * _1K )
    ,UNIT_INFO_MAC(0x00300000,UNIT_PCA_PZ_ARBITER_I_2     ,16  * _1K )
    ,UNIT_INFO_MAC(0x00200000,UNIT_PCA_PZ_ARBITER_E_2     ,16  * _1K )
    ,UNIT_INFO_MAC(0x00400000,UNIT_PCA_SFF_2              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_2_BASE_ADDR, UNIT_PCA_LMU_2        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_2_BASE_ADDR, UNIT_PCA_CTSU_2       ,64  * _1K )

    ,UNIT_INFO_MAC(0x0e300000,UNIT_PCA_MACSEC_EXT_E_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x0dd00000,UNIT_PCA_MACSEC_EXT_I_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x0e800000,UNIT_PCA_MACSEC_EXT_E_164_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x0e600000,UNIT_PCA_MACSEC_EXT_I_164_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x10300000,UNIT_PCA_MACSEC_EXT_E_163_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x0fd00000,UNIT_PCA_MACSEC_EXT_I_163_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x10800000,UNIT_PCA_MACSEC_EXT_E_164_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x10600000,UNIT_PCA_MACSEC_EXT_I_164_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x00800000,UNIT_PCA_MACSEC_EXT_E_163_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x00700000,UNIT_PCA_MACSEC_EXT_I_163_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x00a00000,UNIT_PCA_MACSEC_EXT_E_164_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x00900000,UNIT_PCA_MACSEC_EXT_I_164_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x0de00000,UNIT_PCA_MACSEC_EIP_163_I_0 ,256 * _1K )
    ,UNIT_INFO_MAC(0x0e380000,UNIT_PCA_MACSEC_EIP_163_E_0 ,256 * _1K )
    ,UNIT_INFO_MAC(0x0fe00000,UNIT_PCA_MACSEC_EIP_163_I_1 ,256 * _1K )
    ,UNIT_INFO_MAC(0x10380000,UNIT_PCA_MACSEC_EIP_163_E_1 ,256 * _1K )
    ,UNIT_INFO_MAC(0x0e700000,UNIT_PCA_MACSEC_EIP_164_I_0 ,256 * _1K )
    ,UNIT_INFO_MAC(0x0e900000,UNIT_PCA_MACSEC_EIP_164_E_0 ,256 * _1K )
    ,UNIT_INFO_MAC(0x10700000,UNIT_PCA_MACSEC_EIP_164_I_1 ,256 * _1K )
    ,UNIT_INFO_MAC(0x10900000,UNIT_PCA_MACSEC_EIP_164_E_1 ,256 * _1K )

    /* DFX */
    ,UNIT_INFO_MAC(0x00000000, UNIT_DFX_SERVER       ,1  * _1M)

    ,UNIT_INFO_MAC(0x01800000,UNIT_TTI               ,4   * _1M)

    /* TAI 0,1 */
    ,UNIT_INFO_MAC(0x07C00000,UNIT_TAI             ,      64  * _1K )
    ,UNIT_INFO_MAC(0x07C80000,UNIT_TAI_1           ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0E400000,UNIT_PCA2_TAI0_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0E480000,UNIT_PCA2_TAI1_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x10400000,UNIT_PCA0_TAI0_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x10480000,UNIT_PCA0_TAI1_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x00A80000,UNIT_PCA1_TAI0_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x00AC0000,UNIT_PCA1_TAI1_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0D400000,UNIT_400G0_TAI0_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0D440000,UNIT_400G0_TAI1_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0F400000,UNIT_400G1_TAI0_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0F440000,UNIT_400G1_TAI1_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x01000000,UNIT_400G2_TAI0_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x01040000,UNIT_400G2_TAI1_E    ,      64  * _1K )
    ,UNIT_INFO_MAC(0x01C00000,UNIT_TTI_TAI0        ,      64  * _1K )
    ,UNIT_INFO_MAC(0x01C80000,UNIT_TTI_TAI1        ,      64  * _1K )
    ,UNIT_INFO_MAC(0x08B00000,UNIT_PHA_TAI0_E      ,      64  * _1K )
    ,UNIT_INFO_MAC(0x08B80000,UNIT_PHA_TAI1_E      ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0A300000,UNIT_EPCL_HA_TAI0_E  ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0A380000,UNIT_EPCL_HA_TAI1_E  ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0C280000,UNIT_TXQS_TAI0_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0C680000,UNIT_TXQS_TAI1_E     ,      64  * _1K )
    ,UNIT_INFO_MAC(0x0CC80000,UNIT_TXQS1_TAI0_E    ,      64  * _1K )

    ,UNIT_INFO_MAC(0x01d00000,UNIT_PPU               ,1   * _1M)

    ,UNIT_INFO_MAC(0x08000000,UNIT_PHA               ,8   * _1M)
    ,UNIT_INFO_MAC(0x09400000,UNIT_EPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x0a000000,UNIT_EOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x09000000,UNIT_EPCL              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x0a200000,UNIT_PREQ              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x08c00000,UNIT_ERMRK             ,4   * _1M)
    ,UNIT_INFO_MAC(0x08900000,UNIT_EREP              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x08a00000,UNIT_BMA               ,1   * _1M)
    ,UNIT_INFO_MAC(0x08800000,UNIT_HBU               ,64   *    _1K)
    ,UNIT_INFO_MAC(0x09800000,UNIT_HA                ,6   * _1M)

    ,UNIT_INFO_MAC(0x04800000,UNIT_TCAM              ,8   * _1M)
    ,UNIT_INFO_MAC(0x05000000,UNIT_EM                ,4   * _1M)
    ,UNIT_INFO_MAC(0x04000000,UNIT_FDB               ,64  *     _1K)
/*  ,UNIT_INFO_MAC(0x--------,UNIT_SHM               ,64  *     _1K) no such unit */

    ,UNIT_INFO_MAC(0x01e00000,UNIT_IPCL              ,512 *     _1K)

    ,UNIT_INFO_MAC(0x01f00000,UNIT_CNC               ,256 *     _1K)

    ,UNIT_INFO_MAC(0x07900000,UNIT_EGF_QAG           ,1   * _1M)
    ,UNIT_INFO_MAC(0x07A00000,UNIT_EGF_SHT           ,2   * _1M)
    ,UNIT_INFO_MAC(0x07800000,UNIT_EGF_EFT           ,64  *     _1K)

    ,UNIT_INFO_MAC(0x07000000,UNIT_IPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x07400000,UNIT_IPLR1             ,4   * _1M)

    ,UNIT_INFO_MAC(0x06000000,UNIT_EQ                ,16  * _1M)

    ,UNIT_INFO_MAC(0x05800000,UNIT_IOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x05C00000,UNIT_MLL               ,2   * _1M)

    ,UNIT_INFO_MAC(0x03000000,UNIT_IPVX              ,8   * _1M)
    ,UNIT_INFO_MAC(0x02000000,UNIT_LPM               ,16  * _1M)
    ,UNIT_INFO_MAC(0x03800000,UNIT_L2I               ,8   * _1M)

    ,UNIT_INFO_MAC(0x0d500000,UNIT_LED_0             ,4 * _1K)
    ,UNIT_INFO_MAC(0x0f500000,UNIT_LED_1             ,4 * _1K)
    ,UNIT_INFO_MAC(0x01100000,UNIT_LED_2             ,4 * _1K)

    /*NOTE: MG0 is part of the CnM : 0x3D000000 size 1M */
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG    /*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_0_1/*CNM*/, MG_SIZE)

    /* RFU is in the CnM section */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00000000,UNIT_CNM_RFU    ,64   *     _1K) /*in CnM */

    /* SMI is in the CnM section */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00010000,UNIT_GOP_SMI_0  ,64   *     _1K) /*in CnM */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00020000,UNIT_GOP_SMI_1  ,64   *     _1K) /*in CnM */
     /* AAC is in the CnM section */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00030000,UNIT_CNM_AAC    ,64   *     _1K) /*in CnM */

    /* SRAM  is in the CnM section */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00070000,UNIT_CNM_SRAM    ,64   *     _1K) /*in CnM */

     /* PEX MAC is in the CnM section */
    ,UNIT_INFO_MAC(CNM_OFFSET_CNS + 0x00170000,UNIT_CNM_PEX_MAC   ,64   *     _1K)/*in CnM */

    /* LMU - moved into PCA */
    /* TSU (PTP) - moved into PCA */

    ,UNIT_INFO_MAC(MAC_MTIP_MAC_0_BASE_ADDR,UNIT_MAC_400G_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_1_BASE_ADDR,UNIT_MAC_400G_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_2_BASE_ADDR,UNIT_MAC_400G_2              ,64   *     _1K)

    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_0_BASE_ADDR,UNIT_PCS_400G_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_1_BASE_ADDR,UNIT_PCS_400G_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_2_BASE_ADDR,UNIT_PCS_400G_2              ,64   *     _1K)

    /*MIF */
    ,UNIT_INFO_MAC(MIF_0_BASE_ADDR, UNIT_MIF_0              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_1_BASE_ADDR, UNIT_MIF_1              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_2_BASE_ADDR, UNIT_MIF_2              ,16   *     _1K)

    /*ANP */
    ,UNIT_INFO_MAC(ANP_0_BASE_ADDR, UNIT_ANP_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_1_BASE_ADDR, UNIT_ANP_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_2_BASE_ADDR, UNIT_ANP_2              ,64   *     _1K)

    ,UNIT_INFO_MAC(0x0D100000,      UNIT_SERDES_0_0         ,512  *     _1K)     /* 56G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x0D180000,      UNIT_SERDES_0_1         ,512  *     _1K)     /* 56G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x0F100000,      UNIT_SERDES_0_2         ,512  *     _1K)     /* 56G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x0F180000,      UNIT_SERDES_0_3         ,512  *     _1K)     /* 56G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x00B00000,      UNIT_SERDES_0_4         ,512  *     _1K)     /* 56G Quad SD Wrapper */


    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

/* info about the 50/100/200/400G MACs */
MAC_NUM_INFO_STC harrier100GPortsArr[] = {
    /*global*/      /*mac*/   /* channelInfo */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {0  ,           {0, 0} ,    {0,0 }       ,0    }
    ,{2  ,           {0, 1} ,    {0,2 }       ,0    }
    ,{4  ,           {0, 2} ,    {0,4 }       ,0    }
    ,{6  ,           {0, 3} ,    {0,6 }       ,0    }
    ,{8  ,           {0, 4} ,    {0,8 }       ,0    }
    ,{10 ,           {0, 5} ,    {0,10}       ,0    }
    ,{12 ,           {0, 6} ,    {0,12}       ,0    }
    ,{14 ,           {0, 7} ,    {0,14}       ,0    }

    ,{16 ,           {1, 0} ,    {1,0 }       ,1    }
    ,{18 ,           {1, 1} ,    {1,2 }       ,1    }
    ,{20 ,           {1, 2} ,    {1,4 }       ,1    }
    ,{22 ,           {1, 3} ,    {1,6 }       ,1    }
    ,{24 ,           {1, 4} ,    {1,8 }       ,1    }
    ,{26 ,           {1, 5} ,    {1,10}       ,1    }
    ,{28 ,           {1, 6} ,    {1,12}       ,1    }
    ,{30 ,           {1, 7} ,    {1,14}       ,1    }

    ,{32 ,           {2, 0} ,    {2,0 }       ,2    }
    ,{34 ,           {2, 1} ,    {2,2 }       ,2    }
    ,{36 ,           {2, 2} ,    {2,4 }       ,2    }
    ,{38 ,           {2, 3} ,    {2,6 }       ,2    }


    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* number of units in the device */
#define HARRIER_NUM_UNITS sizeof(harrier_units)/sizeof(harrier_units[0])

static SMEM_GEN_UNIT_INFO_STC SORTED___harrier_units[HARRIER_NUM_UNITS] =
{
    /* sorted and build during smemHarrierInit(...) from harrier_units[] */
    {SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};


/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from falcon_units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC harrier_duplicatedUnits[] =
{
    /* those explicitly listed here need unit allocation as are not duplicated within each pipe */
    {STR(UNIT_RX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_RX_DMA_1)},
        {STR(UNIT_RX_DMA_2)},

    {STR(UNIT_TX_FIFO) ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_FIFO_1)},
        {STR(UNIT_TX_FIFO_2)},

    {STR(UNIT_TX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_DMA_1)},
        {STR(UNIT_TX_DMA_2)},

    {STR(UNIT_TXQ_PDS0)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TXQ_PDS1)},
        {STR(UNIT_TXQ_PDS2)},

    {STR(UNIT_TXQ_SDQ0)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TXQ_SDQ1)},
        {STR(UNIT_TXQ_SDQ2)},

    {STR(UNIT_TXQ_QFC0)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TXQ_QFC1)},
        {STR(UNIT_TXQ_QFC2)},

    {STR(UNIT_MG)        ,1}, /* 1 duplication of this unit */
        {STR(UNIT_MG_0_1)},


    {STR(UNIT_PB_WRITE_ARBITER_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_WRITE_ARBITER_1)},
        {STR(UNIT_PB_WRITE_ARBITER_2)},

    {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_1)},
        {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_2)},

    {STR(UNIT_PB_GPC_GRP_CELL_READ_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_GPC_GRP_CELL_READ_1)},
        {STR(UNIT_PB_GPC_GRP_CELL_READ_2)},

    {STR(UNIT_PB_GPC_GRP_PACKET_READ_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_GPC_GRP_PACKET_READ_1)},
        {STR(UNIT_PB_GPC_GRP_PACKET_READ_2)},

    {STR(UNIT_PB_NEXT_POINTER_MEMO_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PB_NEXT_POINTER_MEMO_1)},

    {STR(UNIT_PB_SHARED_MEMO_BUF_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_SHARED_MEMO_BUF_1)},
        {STR(UNIT_PB_SHARED_MEMO_BUF_2)},

    {STR(UNIT_MAC_400G_0)  ,2},  /* 2 more per device */
        {STR(UNIT_MAC_400G_1)},
        {STR(UNIT_MAC_400G_2)},

    {STR(UNIT_PCS_400G_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PCS_400G_1)},
        {STR(UNIT_PCS_400G_2)},

    {STR(UNIT_PCA_LMU_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PCA_LMU_1)},
        {STR(UNIT_PCA_LMU_2)},

    {STR(UNIT_PCA_CTSU_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PCA_CTSU_1)},
        {STR(UNIT_PCA_CTSU_2)},

    {STR(UNIT_PCA_BRG_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PCA_BRG_1)},
        {STR(UNIT_PCA_BRG_2)},

    {STR(UNIT_PCA_SFF_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PCA_SFF_1)},
        {STR(UNIT_PCA_SFF_2)},

    {STR(UNIT_PCA_PZ_ARBITER_I_0)  ,5},  /* 5 more per device */
        {STR(UNIT_PCA_PZ_ARBITER_I_1)},
        {STR(UNIT_PCA_PZ_ARBITER_I_2)},
        {STR(UNIT_PCA_PZ_ARBITER_E_0)},
        {STR(UNIT_PCA_PZ_ARBITER_E_1)},
        {STR(UNIT_PCA_PZ_ARBITER_E_2)},

    {STR(UNIT_PCA_MACSEC_EXT_I_163_0)  ,11},  /* 11 more per device */
        {STR(UNIT_PCA_MACSEC_EXT_I_163_1)},
        {STR(UNIT_PCA_MACSEC_EXT_I_163_2)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_1)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_2)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_1)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_2)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_1)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_2)},

    {STR(UNIT_PCA_MACSEC_EIP_163_E_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_163_E_1) },

    {STR(UNIT_PCA_MACSEC_EIP_164_E_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_164_E_1) },

    {STR(UNIT_PCA_MACSEC_EIP_163_I_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_163_I_1) },

    {STR(UNIT_PCA_MACSEC_EIP_164_I_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_164_I_1) },

    {STR(UNIT_LED_0)  ,2},  /* 2 more per device */
        {STR(UNIT_LED_1)},
        {STR(UNIT_LED_2)},

    {STR(UNIT_MIF_0)  ,2},  /* 2 more per device */
        {STR(UNIT_MIF_1)},
        {STR(UNIT_MIF_2)},

    {STR(UNIT_ANP_0)  ,2},  /* 2 more per device */
        {STR(UNIT_ANP_1)},
        {STR(UNIT_ANP_2)},

    {STR(UNIT_TAI)  ,22},  /* 22 more per device */
        {STR(UNIT_TAI_1          )},
        {STR(UNIT_PCA2_TAI0_E    )},
        {STR(UNIT_PCA2_TAI1_E    )},
        {STR(UNIT_PCA0_TAI0_E    )},
        {STR(UNIT_PCA0_TAI1_E    )},
        {STR(UNIT_PCA1_TAI0_E    )},
        {STR(UNIT_PCA1_TAI1_E    )},
        {STR(UNIT_400G0_TAI0_E   )},
        {STR(UNIT_400G0_TAI1_E   )},
        {STR(UNIT_400G1_TAI0_E   )},
        {STR(UNIT_400G1_TAI1_E   )},
        {STR(UNIT_400G2_TAI0_E   )},
        {STR(UNIT_400G2_TAI1_E   )},
        {STR(UNIT_TTI_TAI0       )},
        {STR(UNIT_TTI_TAI1       )},
        {STR(UNIT_PHA_TAI0_E     )},
        {STR(UNIT_PHA_TAI1_E     )},
        {STR(UNIT_EPCL_HA_TAI0_E )},
        {STR(UNIT_EPCL_HA_TAI1_E )},
        {STR(UNIT_TXQS_TAI0_E    )},
        {STR(UNIT_TXQS_TAI1_E    )},
        {STR(UNIT_TXQS1_TAI0_E   )},

    {STR(UNIT_SERDES_0_0)  ,4},  /* 4 more per device */
        {STR(UNIT_SERDES_0_1)},
        {STR(UNIT_SERDES_0_2)},
        {STR(UNIT_SERDES_0_3)},
        {STR(UNIT_SERDES_0_4)},


    {NULL,0} /* must be last */
};

/* the DP that hold MG0_0*/
#define CNM0_DP_FOR_MG_0   0
/* the DP that hold MG0_1*/
#define CNM0_DP_FOR_MG_1   1

#define FIRST_CPU_SDMA    40 /* DP ports :40,41 SDMA ports */
/* DMA : special ports mapping {global,local,DP}    */
static SPECIAL_PORT_MAPPING_CNS harrier_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
/*40*/ {FIRST_CPU_SDMA+0/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_0/*DP[0]*/}
/*41*/,{FIRST_CPU_SDMA+1/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_1/*DP[1]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/**
* @internal smemHarrierGetMgUnitIndexFromAddress function
* @endinternal
*
* @brief   Get MG unit index from the regAddr
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] regAddress               - register address
*
* @retval the mg unit Index.
*         if the register is not MG register , it returns SMAIN_NOT_VALID_CNS
*/
static GT_U32  smemHarrierGetMgUnitIndexFromAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  regAddress
)
{
    if(regAddress >= MG_0_0_OFFSET_CNS &&
       regAddress < (MG_0_0_OFFSET_CNS + (MG_SIZE*HARRIER_NUM_MG_UNITS)))
    {
        return (regAddress - MG_0_0_OFFSET_CNS)/MG_SIZE;
    }

    return SMAIN_NOT_VALID_CNS;
}

/**
* @internal smemHarrierGopPortByAddrGet function
* @endinternal
*
* @brief   get global MAC port id according to associated address (in GOP unit).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the  space of the port
*
* @note the port id
*
*/
static GT_U32 smemHarrierGopPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32 unitIndex;
    GT_U32 ii;
    GT_U32 localPortInUnit = 0;
    GT_U32 unitBase;
    GT_U32 unitSize;
    GT_U32 startTxCounters;
    GT_U32 portDivider;

    for(ii = 0; ii < SIM_MAX_ANP_UNITS; ii++)
    {
        if(address >=  devObjPtr->memUnitBaseAddrInfo.anp[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.anp[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.anp[ii] + ANP_OFFSET;
            if(address >= unitBase + 0x00000100 &&
               address < (unitBase + 0x00000104 + 8*0x2a4))
            {
                /* 0x00000094 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Cause */
                /* 0x00000098 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Mask */
                localPortInUnit = (address - (unitBase + 0x00000100)) / 0x2a4;
                break;
            }
        }
    }
    unitIndex = ii;

    if(ii < 3)
    {
        /* matched as address in MTI_ANP  */
        return (localPortInUnit*2)+(ii*16);
    }
    else if(ii < SIM_MAX_ANP_UNITS)
    {
        skernelFatalError("smemHarrierGopPortByAddrGet : in ANP: unknown port for address [0x%8.8x] as 'Gop unit' \n",
            address);
    }
    else
    {
        /* match not found - continue */
    }
    /**********************/
    /* End of ANP section */
    /**********************/



    for(ii = 0 ; ii < HARRIER_NUM_DP_UNITS; ii++)
    {
        if(address >=  devObjPtr->memUnitBaseAddrInfo.macWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + _1M) )
        {
            unitSize = MAC_MIB_UNIT_SIZE;
            startTxCounters = START_MIB_TX_PER_PORT;
            /* support the dual MIB counters per 'port' (EMAC,PMAC) */
            portDivider = 2;

            unitBase = devObjPtr->memUnitBaseAddrInfo.macMib[ii];

            if(address >= unitBase &&
               address < (unitBase + unitSize))
            {
                /* mib unit */
                /* calculate the local port in the unit */
                if(address >= (unitBase + START_MIB_RX_PER_PORT) &&
                   address <  (unitBase + startTxCounters))
                {
                    /* rx registers */
                    localPortInUnit = (address - unitBase) / (43 * 0x4);

                    /* support the dual MIB counters per 'port' (EMAC,PMAC) */
                    localPortInUnit /= portDivider;
                    break;
                }

                if(address >= (unitBase + startTxCounters))
                {
                    /* tx registers */
                    localPortInUnit = (address - unitBase) / (34 * 0x4);

                    /* support the dual MIB counters per 'port' (EMAC,PMAC) */
                    localPortInUnit /= portDivider;
                    break;
                }

                break;/* common to the unit */
            }

            /* MTI 400G mac */
            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_200G_400G_OFFSET;

            if(address >= unitBase &&
               address < (unitBase + (PORT0_100G_OFFSET-MAC_200G_400G_OFFSET)))
            {
                /* 200G/400G port */
                localPortInUnit = 4 * ((address - unitBase) / MAC_STEP_PORT_OFFSET);
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] +  PORT0_100G_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (8*MAC_STEP_PORT_OFFSET)))
            {
                /* 100G/50G port */
                localPortInUnit = (address - unitBase) / MAC_STEP_PORT_OFFSET;
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_EXT_BASE_OFFSET;

            if(address >= unitBase + 0x00000094 &&
               address < (unitBase + 0x00000098 + (8 * 0x18)))
            {
                /* 0x00000094 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
                /* 0x00000098 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
                localPortInUnit = (address - (unitBase + 0x00000094)) / 0x18;
                break;
            }

            if(address == (unitBase + 0x00000068) ||
               address == (unitBase + 0x0000006c) )
            {
                /* 0x00000068 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause */
                /* 0x0000006c : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask */
                localPortInUnit = 0;
                break;
            }

            if(address == (unitBase + 0x00000068 + 0x14) ||
               address == (unitBase + 0x0000006c + 0x14) )
            {
                /* 0x00000068 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause */
                /* 0x0000006c : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask */
                localPortInUnit = 4;
                break;
            }

            break; /* common to the unit */
        }

        if(address >=  devObjPtr->memUnitBaseAddrInfo.macPcs[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + _1M) )
        {

            unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_400G_OFFSET;

            if(address >= unitBase &&
               address < (unitBase + (PCS_PORT0_100G_OFFSET-PCS_400G_OFFSET)))
            {
                /* 200G/400G port */
                localPortInUnit = 4 * ((address - unitBase) / PCS_STEP_PORT_OFFSET);
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_PORT0_100G_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (8*PCS_STEP_PORT_OFFSET)))
            {
                /* 100G/50G port */
                localPortInUnit = (address - unitBase) / PCS_STEP_PORT_OFFSET;
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_LPCS_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (8*LPCS_STEP_PORT_OFFSET)))
            {
                /* LPCS port */
                localPortInUnit = (address - unitBase) / LPCS_STEP_PORT_OFFSET;
                break;
            }
            break;
        }
    }

    if(ii == HARRIER_NUM_DP_UNITS)
    {
        skernelFatalError("smemGopPortByAddrGet : unknown address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }

    unitIndex = ii;

    for(ii = 0 ; harrier100GPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
    {
        if(harrier100GPortsArr[ii].macInfo.unitIndex   == unitIndex &&
           harrier100GPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
        {
            return harrier100GPortsArr[ii].globalMacNum;
        }
    }

    skernelFatalError("smemHarrierGopPortByAddrGet : unknown port for address [0x%8.8x] as 'Gop unit' \n",
        address);
    return 0;
}

/*******************************************************************************
*   smemHarrierPortInfoGet
*
* DESCRIPTION:
*       get port info relate to the portNum .
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       unitType    - the unit type
*       portNum     - the port num
* OUTPUTS:
*       portInfoPtr - (pointer to) the port info.
*
* RETURNS:
*      GT_OK       - the port is valid for the 'unitInfo'
*      GT_NOT_FOUND - the port is not valid for the 'unitInfo'
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS smemHarrierPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SMEM_UNIT_TYPE_ENT      unitType,
    IN GT_U32                  portNum,
    OUT ENHANCED_PORT_INFO_STC *portInfoPtr
)
{
    GT_U32  ii,jj,jjMax = 1;
    MAC_NUM_INFO_STC   *tmpMacInfoPtr;
    GT_U32      sip6_MTI_bmpPorts = 0xFFFFFFFF;
    GT_U32      specific_sip6_MTI_EXTERNAL_representativePortIndex;

    if(unitType == SMEM_UNIT_TYPE_MTI_MAC_USX_E ||
       unitType == SMEM_UNIT_TYPE_MTI_MAC_CPU_E)
    {
        /* support Hawk generic code , that check if port supports USX or CPU port */
        return GT_NOT_FOUND;
    }

    /*special case*/
    if(unitType == SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E)
    {
        GT_U32  mifUnitId = portNum;/* !!! the parameter used as mifUnitId !!! */
        GT_U32  dpUnitIndex = 0;
        GT_U32  IN_indexInUnit = portInfoPtr->simplePortInfo.indexInUnit;/* !!! the parameter used as input !!! */
        GT_U32  OUT_indexInUnit=0;
        static  GT_U32  localPortIn400GMapArr[] = {0,2,4,6,8,10,12,14};
        GT_BIT  isPreemptiveChannel = 0;
        #define _400G_MIF_TO_CHANNEL                                            \
            OUT_indexInUnit     = localPortIn400GMapArr[IN_indexInUnit & 0x7];  \
            isPreemptiveChannel = IN_indexInUnit >> 3

        switch(mifUnitId)
        {
            case 0:/*400G_0*/dpUnitIndex = 0;_400G_MIF_TO_CHANNEL;break;
            case 1:/*400G_1*/dpUnitIndex = 1;_400G_MIF_TO_CHANNEL;break;
            case 2:/*400G_2*/dpUnitIndex = 2;_400G_MIF_TO_CHANNEL;break;
            default:
                skernelFatalError("smemHarrierPortInfoGet : unknown mifUnitId [%d] \n",mifUnitId);
                break;
        }

        portInfoPtr->simplePortInfo.unitIndex   = dpUnitIndex;/* the 'dpUnitIndex' */
        portInfoPtr->simplePortInfo.indexInUnit = OUT_indexInUnit;/* the converted Cider index of port in DP to 'local MAC' */
        portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = 0;/*not used*/
        portInfoPtr->sip6_MTI_bmpPorts[0] = (isPreemptiveChannel << 0);/*bit 0 is 'isPreemptiveChannel'*/
        portInfoPtr->sip6_MTI_bmpPorts[1] = 0;/*not used*/
        return GT_OK;
    }

    switch(unitType)
    {
        case SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E:
        case SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E:
        case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
        case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
            jjMax = HARRIER_NUM_DP_UNITS;
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
            sip6_MTI_bmpPorts = 0x00005555;/* 8 ports in the group : bits : 0,2,4,6,8,10,12,14 */
            break;
        default:
            skernelFatalError("smemHarrierPortInfoGet : unknown type [%d] \n",unitType);
            return GT_NOT_FOUND;

    }

    for(jj = 0 ; jj < jjMax ; jj++)
    {
        switch(unitType)
        {
            case SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E:
            case SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E:
            case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
            case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
                tmpMacInfoPtr = harrier100GPortsArr;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
                tmpMacInfoPtr = harrier100GPortsArr;
                break;
            default:
                return GT_NOT_FOUND;
        }

        for(ii = 0 ;
            tmpMacInfoPtr->globalMacNum != SMAIN_NOT_VALID_CNS ;
            ii++,tmpMacInfoPtr++)
        {
            if(tmpMacInfoPtr->globalMacNum != portNum)
            {
                continue;
            }

            switch(unitType)
            {
                case SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E:
                    portInfoPtr->simplePortInfo = tmpMacInfoPtr->channelInfo;
                    break;
                case SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E:
                    portInfoPtr->simplePortInfo.unitIndex   = tmpMacInfoPtr->channelInfo.unitIndex;
                    /* Harrier use channel 0 in LMU for all channels */
                    portInfoPtr->simplePortInfo.indexInUnit = 0;
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
                    {
                        GT_U32  is_mti_segmented = snetChtPortMacFieldGet(devObjPtr, portNum,
                            SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E);

                        /* can be 32 or 128 depend on 'segmented' */
                        portInfoPtr->simplePortInfo.unitIndex   =
                            is_mti_segmented ? SMEM_CHT_PORT_MTI_MIF_TYPE_128_E :
                                               SMEM_CHT_PORT_MTI_MIF_TYPE_32_E;
                    }
                    portInfoPtr->simplePortInfo.indexInUnit = 0;/* not used */
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
                    {
                        /*100G mac */
                        portInfoPtr->simplePortInfo.unitIndex   = tmpMacInfoPtr->macInfo.unitIndex;/*0..3*/ /*400G_MAC_INST*/
                        portInfoPtr->simplePortInfo.indexInUnit = tmpMacInfoPtr->macInfo.indexInUnit;/*0..7*/
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = portInfoPtr->simplePortInfo.unitIndex;/*0..3*/
                    }

                    /* set specific sip6_MTI_EXTERNAL_representativePort */
                    portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = specific_sip6_MTI_EXTERNAL_representativePortIndex;
                    portInfoPtr->sip6_MTI_bmpPorts[0] = sip6_MTI_bmpPorts;
                    return GT_OK;

                default:
                    portInfoPtr->simplePortInfo = tmpMacInfoPtr->macInfo;
                    break;
            }

            portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex =  tmpMacInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex;
            portInfoPtr->sip6_MTI_bmpPorts[0] = sip6_MTI_bmpPorts;

            return GT_OK;
        }/* loop on ii*/
    } /* loop on jj*/


    return GT_NOT_FOUND;
}

/**
* @internal internalHarrierUnitPipeOffsetGet function
* @endinternal
*
* @brief   Get pipe offset for unit register address in data base.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] regAddress               - unit register lookup address
* @param[in,out] currentPipeIdPtr         - (pointer to) allow indication
*                                      ignored if NULL
*
* @param[out] pipeIdOfAddressIndexPtr  - (pointer to) the global pipe index that hold the 'regAddress' (!!! without !!! the 'current pipe Id')
*                                      ignored if NULL
* @param[in,out] currentPipeIdPtr         - (pointer to) the current pipe Id (from smemGetCurrentPipeId(...))
*                                      ignored if NULL
*
* @retval Pipe offset !== 0        - pipe1 offset
* @retval == 0                     - pipe1 offset not found (should not happen)
*/
static GT_U32 internalHarrierUnitPipeOffsetGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   regAddress,
    OUT GT_U32                  *pipeIdOfAddressIndexPtr,
    INOUT GT_U32                *currentPipeIdPtr,
    IN GT_BOOL                  needPipeOffsetFromPipe0
)
{
    if(pipeIdOfAddressIndexPtr)
    {
        *pipeIdOfAddressIndexPtr = 0;
    }

    if (regAddress == SMAIN_NOT_VALID_CNS)
    {
        /* support NOT valid address if expected */
        return 0;
    }

    if(currentPipeIdPtr)
    {
        *currentPipeIdPtr = 0;
    }

    /******************************************************************/
    /* the address maybe in 'MG 0' but we need to convert to other MG */
    /******************************************************************/
    if(regAddress >= MG_0_0_OFFSET_CNS &&
       regAddress <  MG_0_1_OFFSET_CNS)
    {
        GT_U32 mgUnitIndex;

        mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);

        *pipeIdOfAddressIndexPtr = 0; /* make sure that caller update the address with the needed offset */

        return mgUnitIndex * MG_SIZE;
    }

    return 0;
}

static SMEM_UNIT_NAME_AND_INDEX_STC harrierUnitNameAndIndexArr[HARRIER_NUM_UNITS]=
{
    /* filled in runtime from harrier_units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};
/* the addresses of the units that the harrier uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   harrierUsedUnitsAddressesArray[HARRIER_NUM_UNITS]=
{
    {0,0}    /* filled in runtime from harrier_units[] */
};

/* build once the sorted memory for the falcon .. for better memory search performance

    that use by :

    devObjPtr->devMemUnitNameAndIndexPtr      = harrierUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = harrierUsedUnitsAddressesArray;


*/
static void build_SORTED___harrier_units(void)
{
    GT_U32  numValidElem,ii;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &harrier_units[0];

    if(SORTED___harrier_units[0].base_addr != SMAIN_NOT_VALID_CNS)
    {
        /* already initialized */
        return;
    }

    memcpy(SORTED___harrier_units,harrier_units,sizeof(harrier_units));

    numValidElem = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        numValidElem++;
    }

    qsort(SORTED___harrier_units, numValidElem, sizeof(SMEM_GEN_UNIT_INFO_STC),
          sim_sip6_units_cellCompare);

}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        harrierUsedUnitsAddressesArray - the addresses of the units that the Falcon uses
        harrierUnitNameAndIndexArr - name of unit and index in harrierUsedUnitsAddressesArray */
    GT_U32  ii;
    GT_U32  index;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &SORTED___harrier_units[0];

    /* build once the sorted memory for the falcon .. for better memory search performance */
    build_SORTED___harrier_units();

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        harrierUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
        harrierUsedUnitsAddressesArray[index].unitSizeInBytes = unitInfoPtr->size;
        harrierUnitNameAndIndexArr[index].unitNameIndex = index;
        harrierUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        index++;
    }

    if(index >= (sizeof(harrierUnitNameAndIndexArr) / sizeof(harrierUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    harrierUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    harrierUnitNameAndIndexArr[index].unitNameStr = NULL;
    harrierUsedUnitsAddressesArray[index].unitBaseAddr = SMAIN_NOT_VALID_CNS;
    harrierUsedUnitsAddressesArray[index].unitSizeInBytes = 0;

    devObjPtr->devMemUnitNameAndIndexPtr = harrierUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = harrierUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;/* no pipe offset */
    devObjPtr->support_memoryRanges = 1;

#if 0 /* check that the array is ascending ! (harrierUsedUnitsAddressesArray) */
    for(ii = 0 ; ii < (index+1) ; ii++)
    {
        printf("unitBaseAddr = [0x%8.8x] \n",
            harrierUsedUnitsAddressesArray[ii].unitBaseAddr);

        if(ii &&
            (harrierUsedUnitsAddressesArray[ii].unitBaseAddr <=
             harrierUsedUnitsAddressesArray[ii-1].unitBaseAddr))
        {
            printf("Error: at index[%d] prev index higher \n",ii);
            break;
        }
    }
#endif /*0*/
}

/**
* @internal smemHarrierActiveRead_pm_ctrl_tx_lane_reg1_lane function
* @endinternal
*
* @brief   Read pm_ctrl_tx_lane_reg1_lane value.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output counter value and reset it.
*
*/
static void smemHarrierActiveRead_pm_ctrl_tx_lane_reg1_lane (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* output const value */
    * outMemPtr = (* memPtr) | (1<<18)/*pin_pll_ready_tx_lane*/;

    return;
}

/**
* @internal smemHarrierActiveRead_pm_ctrl_rx_lane_reg1_lane function
* @endinternal
*
* @brief   Read pm_ctrl_rx_lane_reg1_lane value.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output counter value and reset it.
*
*/
static void smemHarrierActiveRead_pm_ctrl_rx_lane_reg1_lane (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* output const value */
    * outMemPtr = (* memPtr) | (1<<22)/*pin_pll_ready_rx_lane*/;

    return;
}

/**
* @internal smemHarrierActiveRead_tx_train_if_reg3 function
* @endinternal
*
* @brief   Read tx_train_if_reg3 value.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output counter value and reset it.
*
*/
static void smemHarrierActiveRead_tx_train_if_reg3 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* output value */
    * outMemPtr = (* memPtr) | (1<<4)/*RX_TRAIN_COMPLETE_LANE*/;

    return;
}



/**
* @internal smemHarrierUnitSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories - SERDESes
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHarrierUnitSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
#define SMEM_PER_LANE_MASK_CNS (0xFFFFFFF3)/* support 4 lanes */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {0x00000080, SMEM_PER_LANE_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwTxRxTraining,0},

    {0x00000220, SMEM_PER_LANE_MASK_CNS, smemChtActiveReadConst, 0xc ,NULL,0},
    {0x00000240, SMEM_PER_LANE_MASK_CNS, smemChtActiveReadConst, 0x5 ,NULL,0},

    /* bit 18 TxPLL for the logic in mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl */
    {0x00023000, SMEM_PER_LANE_MASK_CNS, smemHarrierActiveRead_pm_ctrl_tx_lane_reg1_lane, 0 ,NULL,0},
    /* bit 22 RxPLL for the logic in mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl */
    {0x00023200, SMEM_PER_LANE_MASK_CNS, smemHarrierActiveRead_pm_ctrl_rx_lane_reg1_lane, 0 ,NULL,0},
    /* bit 4 RX_TRAIN_COMPLETE_LANE for the logic in mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort */
    {0x00025020, SMEM_PER_LANE_MASK_CNS, smemHarrierActiveRead_tx_train_if_reg3         , 0 ,NULL,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* DSW IP */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x0000004C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x0000006C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E0, 0x000000EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000010C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x0000012C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x0000014C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x0000022C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000240, 0x0000024C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x0000028C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002A0, 0x000002AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002E0, 0x000002EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}

            /* DSW COMPHY 56 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020000, 0x00020100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020400, 0x000204FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00021000, 0x000210F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000210FC, 0x000211DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00021400, 0x000214B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00021800, 0x00021AF0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022000, 0x000220BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022400, 0x000224DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022800, 0x000228B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022C00, 0x00022CDC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023000, 0x00023034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002303C, 0x00023044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002304C, 0x0002304C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002305C, 0x00023070)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002307C, 0x00023090)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023098, 0x000230E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000230F4, 0x00023100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023200, 0x0002324C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023258, 0x00023270)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023280, 0x00023360)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023400, 0x00023528)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023530, 0x00023538)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00023540, 0x00023548)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x00024068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024070, 0x000241F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024200, 0x00024214)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024220, 0x00024244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025000, 0x0002504C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025054, 0x000250C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025100, 0x00025148)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025200, 0x00025248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025300, 0x000253AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025400, 0x00025464)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025500, 0x00025558)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025600, 0x00025678)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025700, 0x00025708)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025800, 0x00025808)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00025900, 0x00025950)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026000, 0x00026084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002608C, 0x000261AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026200, 0x00026344)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026600, 0x00026658)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026D00, 0x00026E20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026E2C, 0x00026E2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00028000, 0x0002819C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00028400, 0x00028420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A200, 0x0002A208)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A214, 0x0002A23C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A254, 0x0002A2A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A300, 0x0002A340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A348, 0x0002A34C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A354, 0x0002A380)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A394, 0x0002A3B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A3F8, 0x0002A43C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002E600, 0x0002E650)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002E658, 0x0002E65C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002E668, 0x0002E710)}

            /* SDW PRAM */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 65536)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
                {
            /* SDW IP */
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x07280000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x01650000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00030000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x80000400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0x03a00000,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0x03000000,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000500,         0xffff0000,      1,    0x4 }

            /* DSW COMPHY 56 */
            ,{DUMMY_NAME_PTR_CNS,            0x00021000,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021004,         0x00000051,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002101c,         0x00000013,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021024,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021028,         0x000000d5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021040,         0x000000f5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021044,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021050,         0x0000000f,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021058,         0x00000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021060,         0x0000000f,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021080,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021084,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021088,         0x0000001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002108c,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021090,         0x00000024,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021094,         0x00000001,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210a0,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210ac,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210b0,         0x000000bb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210b4,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210b8,         0x00000097,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210bc,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210c0,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210c4,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210c8,         0x000000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210cc,         0x00000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210d0,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210d4,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210dc,         0x0000001e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210e0,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210e4,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000210e8,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021100,         0x00000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021104,         0x0000003c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021108,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002110c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021114,         0x00000015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021120,         0x0000000f,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021128,         0x00000030,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002112c,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021134,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021144,         0x0000000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002114c,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021158,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000211a4,         0x0000003f,      6,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000211bc,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000211c0,         0x000000aa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000211c4,         0x0000009c,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000211d8,         0x0000009c,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021404,         0x000000a2,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021408,         0x00000098,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002140c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021410,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021414,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021448,         0x000000a8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002144c,         0x000000f8,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021460,         0x000000a2,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021464,         0x00000098,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021468,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002146c,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021470,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000214a4,         0x000000a8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000214a8,         0x000000f8,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021808,         0x000000c5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021810,         0x00000041,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021814,         0x00000018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021818,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002181c,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021820,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021830,         0x000000ca,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021834,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021838,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002183c,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021840,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002184c,         0x00000090,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021850,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021854,         0x00000016,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021858,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002185c,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021864,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021870,         0x00000090,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021874,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021878,         0x00000016,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002187c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021880,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021888,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021894,         0x00000094,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021898,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002189c,         0x00000014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218a0,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218a4,         0x000000f8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218ac,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218b4,         0x0000009a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218b8,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218bc,         0x00000014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218c0,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218c4,         0x000000f8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218cc,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218d4,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218d8,         0x00000050,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218dc,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218e0,         0x00000012,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218e4,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218e8,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218ec,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218f4,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218f8,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000218fc,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021900,         0x00000012,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021904,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021908,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002190c,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021924,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021928,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002192c,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021930,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021934,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021938,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021940,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021944,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021950,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021954,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021958,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002195c,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021960,         0x00000034,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021964,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021968,         0x000000f4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002196c,         0x0000001a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021970,         0x000000fc,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021984,         0x00000081,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021988,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002198c,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021994,         0x00000024,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021998,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002199c,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219a0,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219a4,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219b0,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219b4,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219b8,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219bc,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219c0,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219c4,         0x000000a4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219c8,         0x000000fc,      2,    0xc }
            ,{DUMMY_NAME_PTR_CNS,            0x000219d0,         0x000000a0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219dc,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219e0,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219e4,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219e8,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219ec,         0x00000034,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219f0,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000219f4,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a08,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a0c,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a10,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a14,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a18,         0x00000034,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a1c,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a20,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a44,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a48,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a4c,         0x0000000a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a50,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a54,         0x000000bc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a58,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a5c,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a60,         0x000000a1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a64,         0x000000fc,      2,    0xc }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a6c,         0x000000a0,      2,    0x10}
            ,{DUMMY_NAME_PTR_CNS,            0x00021a80,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a84,         0x00000030,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a88,         0x000000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021a8c,         0x00000080,      2,    0x18}
            ,{DUMMY_NAME_PTR_CNS,            0x00021a98,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ab0,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ab4,         0x00000041,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ab8,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021abc,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ac4,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ac8,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ad8,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021adc,         0x00000012,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ae0,         0x00000014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ae4,         0x0000003e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00021ae8,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022000,         0x00000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022004,         0x00000008,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022008,         0x000000c8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022010,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022014,         0x00000030,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002201c,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022020,         0x0000000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022024,         0x00000076,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022028,         0x000000a9,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002202c,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022030,         0x000000a5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022034,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022038,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002203c,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022044,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002204c,         0x00000048,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022050,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002205c,         0x00000083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022064,         0x00000042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022068,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002206c,         0x000000f8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022070,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022094,         0x00000083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022098,         0x0000003e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002209c,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000220a0,         0x000000f0,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x000220a4,         0x0000001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000220ac,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000220b0,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000220b4,         0x0000009c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022408,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022410,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022414,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022418,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002241c,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022420,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022430,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022434,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022438,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002243c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022440,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022444,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022448,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022454,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022458,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002245c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022460,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022464,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022468,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002246c,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022474,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022478,         0x00000050,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002247c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022480,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022484,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022488,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002248c,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022494,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022498,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002249c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224a0,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224a4,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224a8,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224ac,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224bc,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224c4,         0x0000002a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224c8,         0x00000005,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224cc,         0x000000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000224d8,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022800,         0x00000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022804,         0x00000008,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022808,         0x000000c8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022810,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022814,         0x00000030,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002281c,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022820,         0x0000000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022824,         0x00000076,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022828,         0x000000a9,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002282c,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022830,         0x000000a5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022834,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022838,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002283c,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022844,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002284c,         0x00000048,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022850,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002285c,         0x00000083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022864,         0x00000042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022868,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002286c,         0x000000f8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022870,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022894,         0x00000083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022898,         0x0000003e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002289c,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000228a0,         0x000000f0,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x000228a4,         0x0000001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000228ac,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000228b0,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000228b4,         0x0000009c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c08,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c10,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c14,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c18,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c1c,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c20,         0x000000fe,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c30,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c34,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c38,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c3c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c40,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c44,         0x0000007c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c48,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c54,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c58,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c5c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c60,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c64,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c68,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c6c,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c74,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c78,         0x00000050,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c7c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c80,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c84,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c88,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c8c,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c94,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c98,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022c9c,         0x00000021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022ca0,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022ca4,         0x0000002c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022ca8,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022cac,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022cbc,         0x000000c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022cc4,         0x0000002a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022cc8,         0x00000005,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022ccc,         0x000000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00022cd8,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023000,         0x29004280,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023004,         0x00002000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002301c,         0x00000700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023020,         0x20000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023034,         0x41009500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002303c,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002304c,         0x0000ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002306c,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002307c,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023080,         0x00180092,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023088,         0xd2142800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002308c,         0x1f4900b3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023098,         0x10000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230a8,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230bc,         0x00003010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230c0,         0x00000042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230c4,         0x00978002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230c8,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230cc,         0x0000007e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230d0,         0x00000014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230dc,         0x12305674,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230e0,         0x00010002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230f4,         0x00000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230f8,         0x0c21202d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000230fc,         0x00800825,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023200,         0x00302a54,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023204,         0x40000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023208,         0x6f0011f4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002321c,         0x68000082,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023220,         0xf1800c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023238,         0x000000f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023260,         0x0604e800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023264,         0x02020000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023268,         0x0000864f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002326c,         0x0b680000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023270,         0x00070e66,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023280,         0x10030040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232a0,         0xffff0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232a4,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232b8,         0x00010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232c0,         0x741ffe04,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232c4,         0x040fea99,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000232c8,         0x2b9a2789,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002335c,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023360,         0x00a00000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023400,         0x04000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023408,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023410,         0x001b0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023414,         0x00003fff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023428,         0x00030e0b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000234ac,         0x00010001,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000234c4,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000234d0,         0x0000000a,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00023514,         0x00000400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002351c,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024000,         0x040014f0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024004,         0x00a00c04,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024008,         0x02000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002400c,         0x203f00a1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024010,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024018,         0x03f00401,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002401c,         0x00040415,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024024,         0x80000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002402c,         0x68222291,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024030,         0x666a6888,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024034,         0x2469a162,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024038,         0x24624668,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002403c,         0x00244246,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024040,         0xe6122423,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024044,         0x080822aa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024048,         0x30000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002404c,         0x0f0b0804,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024050,         0xffd38d47,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024054,         0x00210021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024058,         0x01200003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002405c,         0xb0c08400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024068,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024078,         0x00246268,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00024240,         0x0000000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025000,         0x00000840,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002500c,         0x0c700000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025014,         0x27108503,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025018,         0x0bb70013,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025020,         0x0083d800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025024,         0x40000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025028,         0xbf3fa53f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002502c,         0x000f000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025030,         0x00260006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025034,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002503c,         0x002f2f3f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025040,         0x000f0900,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025044,         0x00000600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002504c,         0x00007700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025054,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002505c,         0x00000041,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025064,         0x00030000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002507c,         0x00420000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025080,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025084,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002509c,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250a8,         0x00000042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250ac,         0x00000189,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250b0,         0x2f2f2f3f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250b4,         0x0f0f0900,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250b8,         0x00000600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000250c4,         0x000000a0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025100,         0x0000004f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025104,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025108,         0x0000a556,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002510c,         0x0002fffd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025110,         0x22004000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025114,         0x33000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025118,         0x11a40000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002511c,         0x08000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025120,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025124,         0x000005cf,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025128,         0x00000018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002512c,         0x00800000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025138,         0x00400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025144,         0x0000ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025200,         0x0000004f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025204,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025208,         0x0000a556,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002520c,         0x0002fffd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025210,         0x22004000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025214,         0x33000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025218,         0x11a40000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002521c,         0x08000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025220,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025224,         0x000005cf,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025228,         0x00000018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002522c,         0x00200000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025238,         0x00400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025244,         0x0000ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025300,         0x00410000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025308,         0x4a810000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002530c,         0xcc082601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025310,         0x00840013,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025314,         0x68140000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025318,         0x80400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002531c,         0x0cf80000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025320,         0x00000b68,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025324,         0x8a200000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025328,         0x02015041,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002532c,         0x1c1a1b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025330,         0x171c1d20,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025334,         0x00151b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025338,         0x05000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002533c,         0x04000600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025340,         0x00030000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025344,         0x06030004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025348,         0x00050404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002534c,         0x00000b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025350,         0x05bf4fff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025354,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025358,         0x00004a20,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002535c,         0x1c1a1b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025360,         0x171c1d20,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025364,         0x00151b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025368,         0x05000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002536c,         0x04000600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025370,         0x00030000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025374,         0x06030004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025378,         0x00050404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002537c,         0x00000b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025380,         0x00008a20,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025384,         0x1c1a1b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025388,         0x171c1d20,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002538c,         0x00151b18,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025390,         0x05000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025394,         0x04000600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025398,         0x00030000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002539c,         0x06030004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000253a0,         0x00050404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000253a4,         0x00000b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000253a8,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025400,         0x00291000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025404,         0x40110000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025408,         0x00000087,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002540c,         0x162820c5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025410,         0x01001000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025414,         0x101f0818,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025418,         0x02021f1f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002541c,         0x031afcfa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025420,         0x3014011e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002542c,         0x19300000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025430,         0x08180100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025434,         0x301f101f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025438,         0x00000205,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002543c,         0x00000042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025444,         0x00010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025448,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025460,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025464,         0x00001654,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025924,         0x00002000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00025930,         0x00004000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002600c,         0x00000300,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026010,         0x05550000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026018,         0x37080c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002601c,         0x37083708,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002602c,         0xe0000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026034,         0xf0000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002603c,         0x0c000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026044,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026048,         0x04050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002604c,         0x7c010220,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026050,         0x0c000c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026054,         0x07600000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026058,         0x0300c008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026064,         0x00000405,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026074,         0x05550000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002607c,         0x00fe0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026080,         0x00003215,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026084,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002608c,         0x0f0f0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026090,         0xffff3f05,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026094,         0x00c88307,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026098,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002609c,         0x03020000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260a0,         0x000000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260a8,         0x00323232,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260e4,         0x0003ffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260ec,         0x000b1e00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260f0,         0x2c152c15,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260f8,         0x3a0a0632,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000260fc,         0x37372814,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002619c,         0x0f0732bf,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000261a0,         0x283207e0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000261a4,         0x1f190c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000261a8,         0x00080000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000261ac,         0x02083200,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026200,         0x0301000a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026220,         0x05000202,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026234,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026338,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026dd0,         0x00009300,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026dd4,         0x00163209,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026dd8,         0x03ffff12,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026de4,         0x00000f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00026e20,         0x00001f40,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028000,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028004,         0x000000cc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028008,         0x00000068,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002800c,         0x0000006c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028010,         0x0000008e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028014,         0x0000005e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028018,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002801c,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028020,         0x0000006c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028024,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028028,         0x0000004e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002802c,         0x0000000e,     15,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028068,         0x00000018,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028078,         0x00000012,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028088,         0x0000000d,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028098,         0x0000000e,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280a4,         0x0000000d,      5,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280b8,         0x000000cc,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280c0,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280c4,         0x000000cc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280c8,         0x0000000e,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280d8,         0x00000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280dc,         0x0000000e,      8,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000280fc,         0x00000012,      8,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002811c,         0x00000044,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028120,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028128,         0x00000090,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002812c,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028130,         0x000000e0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028134,         0x0000000b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028138,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028150,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028154,         0x0000008c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028158,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002815c,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028160,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028168,         0x00000080,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028178,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002818c,         0x00000090,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028198,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028408,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002840c,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028410,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00028414,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002841c,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a220,         0x20000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a23c,         0x0000001b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a308,         0x0000ff01,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a318,         0x08000000,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a320,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a32c,         0x00020000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a330,         0x0000004f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a334,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a338,         0x00800001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a340,         0x00310a00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a35c,         0xfff80000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a364,         0x00010001,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a374,         0x80000014,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a3a0,         0x00000400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a3a4,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a3f8,         0x74008a10,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002a3fc,         0x40100000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e608,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e60c,         0x03bac000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e610,         0x00061400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e614,         0x00000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e618,         0x80000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e620,         0x06000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e624,         0x00000005,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e628,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e62c,         0x0002003a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e634,         0x20202020,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e658,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e668,         0x040d0404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e66c,         0x03030c04,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e670,         0x0902020b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e674,         0x060b0404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e678,         0x05050c06,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e67c,         0x0a04040c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e680,         0x060e0505,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e684,         0x00000c06,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6a4,         0x0000090f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6a8,         0x00000fff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6ac,         0x00c3dfbf,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6b4,         0x0042cf78,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6b8,         0x00c3dfbf,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6bc,         0x00420f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6c0,         0x0042cf78,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6c4,         0x00c3dfbf,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6c8,         0x00420f31,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6cc,         0x00420f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6d4,         0x0042cf78,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6e0,         0x00420f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6e4,         0x00807f80,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6e8,         0x0002c100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6ec,         0x00c3df48,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6f0,         0x00021d3f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6f4,         0x00061d3f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6f8,         0x00803980,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0002e6fc,         0x00000f03,      1,    0x4 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal smemHarrierSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHarrierSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
          {STR(UNIT_EGF_QAG)      ,smemHarrierUnitEgfQag },
          {STR(UNIT_PREQ)      ,smemHarrierUnitPreq },
          {STR(UNIT_SERDES_0_0)    ,smemHarrierUnitSerdes },

        /* must be last */
         {NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}

/**
* @internal smemHarrierUnitBar0 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM BAR0 unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHarrierUnitBar0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* active memory     from smemFalconUnitBar0(...) */

    /* memory allocation from smemFalconUnitBar0(...) */

    /* only different default values */
    if(unitPtr->unitDefaultRegistersPtr == NULL)
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
            /* lower 0x00 - 0x3c addresses comes from the 'PCI configuration space'  */
            /*              values from 12.8T HW device after cpssInitSystem is done */
            /* register 0 is set by function smemChtInitPciRegistres in runtime with the 'current device type' */
              {DUMMY_NAME_PTR_CNS,            0x00000000 ,             0x11ab,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000004 ,         0x00100006,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000008 ,         0x02000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000000C ,         0x00000008,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000010 ,         0x00000004,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000014 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000018 ,         0x00000004,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000001C ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000020 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000024 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000028 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000002C ,         0x11ab11ab,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000030 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000034 ,         0x00000040,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000038 ,         0x00000000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000003C ,         0x000001ff,      1,    0x0  }

             /* values from Cider of Harrier */
             ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0xdbc35001,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000008,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x00807005,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x00020010,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000074,         0x00008fc0,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000078,         0x00002010,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000007c,         0x00435413,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0x10110000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000094,         0x0001001f,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000009c,         0x0000000e,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000000a0,         0x00010003,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x14820001,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x00400000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x00462030,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x0000e000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x000000a0,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000148,         0x15810019,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x00000700,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000158,         0x0001000b,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000015c,         0x10040002,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000168,         0x00000100,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x00180000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00000200,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000214,         0x00fff000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x0c23040b,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0xffffffff,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x00800004,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x1b60c000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x00010120,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x0000c000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00080140,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x00000008,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000724,         0x00000001,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000740,         0x0000000f,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000748,         0x45218038,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000074c,         0x05218006,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000750,         0x05400000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000080c,         0x00020178,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000081c,         0x0000007e,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000088c,         0x00000003,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000890,         0x00002000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008a8,         0x04039060,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008b8,         0x00000001,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008bc,         0x000bff41,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008c4,         0x00000044,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008cc,         0xff000001,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008d0,         0x00009c00,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008d4,         0x00000132,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008f8,         0x3534302a,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x000008fc,         0x67612a2a,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000b40,         0x0000000a,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000b48,         0x00000220,      1,    0x4   }
             /*oATU*/
             ,{DUMMY_NAME_PTR_CNS,            ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000010, 0x00007fff,     8,    0x200}
             /*iATU*/
             ,{DUMMY_NAME_PTR_CNS,            ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000110, 0x00007fff,    64,    0x200}

             ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

        unitPtr->unitDefaultRegistersPtr = &list;

    }
}

/**
* @internal smemHarrierPexAndBar0DeviceUnitAlloc function
* @endinternal
*
* @brief   allocate 'PEX config space' and 'BAR0' -- if not allocated already
* @param[in] devObjPtr                - pointer to device object.
*
*/
void smemHarrierPexAndBar0DeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;

    currUnitChunkPtr = &devMemInfoPtr->BAR0_UnitMem;
    /* generate memory space for BAR0 */
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemHarrierUnitBar0(devObjPtr,currUnitChunkPtr);
    }

    /*allocate 'PEX config space' and 'BAR0' -- if not allocated already*/
    smemFalconPexAndBar0DeviceUnitAlloc(devObjPtr);
}



/**
* @internal smemHarrierSpecificDeviceUnitAlloc_main function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHarrierSpecificDeviceUnitAlloc_main
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /*allocate 'PEX config space' and 'BAR0' -- if not allocated already*/
    smemHarrierPexAndBar0DeviceUnitAlloc(devObjPtr);

    /*Harrier specific */
    smemHarrierSpecificDeviceUnitAlloc_SIP_units(devObjPtr);

    /*Harrier specific */
    smemHarrierSpecificDeviceUnitAlloc_DIP_units(devObjPtr);

    /* call it before smemHawkSpecificDeviceUnitAlloc_DP_units() ,
        so the TXQ units are from this file and not from 'Hawk' */
    smemHarrierSpecificDeviceUnitAlloc_TXQ_units(devObjPtr);
    /*DP from Hawk*/
    smemHawkSpecificDeviceUnitAlloc_DP_units(devObjPtr);
    /*CP from Phoenix*/
    smemPhoenixSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

/**
* @internal smemHarrierSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHarrierSpecificDeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[0];

    for (unitIndex = 0 ; unitIndex < SMEM_CHT_NUM_UNITS_MAX_CNS;
        unitIndex+=1,currUnitChunkPtr += 1)
    {
        currUnitChunkPtr->chunkType = SMEM_UNIT_CHUNK_TYPE_FLEX_BASE_E;
    }

    {
        GT_U32  ii,jj;
        SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &harrier_units[0];

        for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
        {
            jj = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,unitInfoPtr->base_addr);

            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[jj];

            if(currUnitChunkPtr->chunkIndex != jj)
            {
                skernelFatalError("smemFalconSpecificDeviceUnitAlloc : not matched index");
            }
            currUnitChunkPtr->numOfUnits = 1;/*unitInfoPtr->size;*/
        }
    }

    smemHarrierSpecificDeviceUnitAlloc_main(devObjPtr);
}


/**
* @internal smemHarrierConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Harrier : Convert (dev,address) to new (dev,address).
*         needed for multi-MG device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void smemHarrierConvertDevAndAddrToNewDevAndAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT * *newDevObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
)
{
    GT_U32  newAddress;
    GT_U32 currentPipeId;
    GT_U32 pipeOffset , pipeIdOfAddress = 0;

    *newDevObjPtrPtr = devObjPtr;

    if(0 == IS_SKERNEL_OPERATION_MAC(accessType))
    {
        /* the CPU access 'pipe 0/1' explicitly */
        /* so no address modifications */
        *newAddressPtr = address;
        return;
    }

    pipeOffset = internalHarrierUnitPipeOffsetGet(devObjPtr, address,
        &pipeIdOfAddress,&currentPipeId,GT_FALSE);

    newAddress = address + pipeOffset;

    *newAddressPtr = newAddress;

    return;
}

/**
* @internal smemHarrierPrepareMultiMgRecognition function
* @endinternal
*
* @brief   prepare multi MG units recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void smemHarrierPrepareMultiMgRecognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    /* NOTE: the function smemHarrierConvertDevAndAddrToNewDevAndAddr will help
       any memory access to 'MG0' address to access proper MG unit */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        smemHarrierConvertDevAndAddrToNewDevAndAddr;

}

#define ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(target_unitName,regDbUnitName, dpIndex) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,target_unitName))           \
    {                                                                                                         \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,target_unitName)]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                       \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName[dpIndex]),                                   \
            currUnitChunkPtr);                                                                                \
                                                                                                              \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,target_unitName);                                    \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName[dpIndex]           ,unitBaseAddress); \
    }

#define ALIGN_REG_DB_TO_UNIT_MAC(target_unitName,regDbUnitName) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,target_unitName))           \
    {                                                                                                         \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,target_unitName)]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                       \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName),                                   \
            currUnitChunkPtr);                                                                                \
                                                                                                              \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,target_unitName);                                    \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName,unitBaseAddress); \
    }


/**
* @internal smemHarrierInitRegDbDpUnits function
* @endinternal
*
* @brief   Init RegDb for DP units
*/
static void smemHarrierInitRegDbDpUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    /* set register addresses for SDQ[0..2] */
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ1 , SIP6_TXQ_SDQ ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ2 , SIP6_TXQ_SDQ ,2);/*DP[2]*/

    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC1 , SIP6_TXQ_QFC ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC2 , SIP6_TXQ_QFC ,2);/*DP[2]*/

    /* set register addresses for sip6_rxDMA[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA   , sip6_rxDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_1 , sip6_rxDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_2 , sip6_rxDMA ,2);/*DP[2]*/

    /* set register addresses for sip6_txDMA[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA   , sip6_txDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_1 , sip6_txDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_2 , sip6_txDMA ,2);/*DP[2]*/

    /* set register addresses for sip6_txFIFO[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_1 , sip6_txFIFO ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_2 , sip6_txFIFO ,2);/*DP[2]*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_TXQ_PDX , SIP6_TXQ_PDX);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PREQ,PREQ);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EREP,EREP);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EM,EXACT_MATCH);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PHA,PHA);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_SHM,SHM);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PPU,PPU);

    /* support CP units too : CNC2,3 */
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_CNC_2 , CNC ,2);/*CNC[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_CNC_3 , CNC ,3);/*CNC[3]*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_AAC, CNM.AAC);

}

/**
* @internal internal_InterruptTreeInit function
* @endinternal
*
* @brief   Init interrupt tree for specific device.
*
* @param[in] devObjPtr                - (pointer to) device object.
*                                      currDbPtrPtr - (pointer ti) pointer to device interrupt tree to be initialized.
*/
static GT_VOID  internal_InterruptTreeInit
(
     IN SKERNEL_DEVICE_OBJECT *devObjPtr,
     INOUT SKERNEL_INTERRUPT_REG_INFO_STC ** currDbPtrPtr
)
{
    smemHarrierInterruptTreeInit(devObjPtr);
}

static void regAddr_anp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
)
{
    GT_U32  unitOffset;
    SMEM_CHT_PORT_ANP_UNIT_REG_STC  *unitDbPtr;
    GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_50G_E;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32 unitIndex = 0;

    is_SMEM_UNIT_TYPE_MTI_MAC_50G_E = (GT_OK == smemHarrierPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_50G_E,portNum,&portInfo)) ? 1 : 0;

    if(is_SMEM_UNIT_TYPE_MTI_MAC_50G_E)
    {
        unitIndex = portNum / 16;
    }
    else
        return;


    unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[unitIndex].ANP;
    unitOffset = devObjPtr->memUnitBaseAddrInfo.anp[unitIndex];

    unitDbPtr->portInterruptCause[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x100; /*0x00000100*/
    unitDbPtr->portInterruptMask[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x104; /*0x00000104*/
    unitDbPtr->interruptSummaryCause = unitOffset + 0x94; /*0x00000094*/
    unitDbPtr->interruptSummaryMask = unitOffset + 0x98; /*0x00000098*/
}

/**
* @internal smemHawkGopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Hawk.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHarrierGopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 globalPort;

    smemHawkGopRegDbInit(devObjPtr);

    for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
    {
        regAddr_anp(devObjPtr, globalPort);
    }

    return;
}

/**
* @internal internal_smemHarrierInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
static void internal_smemHarrierInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isHarrier = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP6_20_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP6_15_GET(devObjPtr) = 1;/* as not call to Phoenix init */

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isHarrier = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = harrier_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = harrier_duplicatedUnits;
    }

    if (isHarrier == GT_TRUE)
    {
        devObjPtr->devMemGopRegDbInitFuncPtr = smemHarrierGopRegDbInit;
        devObjPtr->devIsOwnerMemFunPtr = NULL;
        devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;
        devObjPtr->devMemPortInfoGetPtr =  smemHarrierPortInfoGet;

        devObjPtr->devMemGetMgUnitIndexFromAddressPtr = smemHarrierGetMgUnitIndexFromAddress;
        devObjPtr->devMemGopPortByAddrGetPtr = smemHarrierGopPortByAddrGet;
        devObjPtr->devMemMibPortByAddrGetPtr = smemHarrierGopPortByAddrGet;

        devObjPtr->numOfPipesPerTile = 0;/* no pipes */
        devObjPtr->numOfTiles        = 0;/* no tiles */
        devObjPtr->tileOffset        = 0;
        devObjPtr->mirroredTilesBmp  = 0;

        devObjPtr->numOfMgUnits = HARRIER_NUM_MG_UNITS;/* 2 MG units */

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = HARRIER_NUM_DP_UNITS;
        /* !!! there is NO TXQ-dq in Harrier (like in Hawk) !!! */
        devObjPtr->multiDataPath.numTxqDq           = 0;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 0;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = FIRST_CPU_SDMA;

        /* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = 0;
        devObjPtr->numOfPortsPerPipe = SMAIN_NOT_VALID_CNS;/* no 'pipes' */

        devObjPtr->txqNumPorts = 128;/* the limit on the 10 bits in SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM */

        devObjPtr->multiDataPath.maxIa = 1;/* single IA */

        {/* support the multi DP units */
            GT_U32  index;

            for(index = 0 ; index < devObjPtr->multiDataPath.maxDp ; index++)/* ALL 4 DPs duplicated although not evenly used! */
            {
                devObjPtr->multiDataPath.info[index].dataPathFirstPort  = NUM_PORTS_FOR_NUMBERRING_DP_UNIT*index;/* without the 'CPU port' */
                devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = NUM_PORTS_FOR_NUMBERRING_DP_UNIT;      /* without the 'CPU port' */
                devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = NUM_PORTS_PER_DP_UNIT;                 /* the 'CPU port' number  in the DP */
            }

            devObjPtr->multiDataPath.info[2].dataPathNumOfPorts = 8;/* half the ports */
        }

        /* MIF support */
        devObjPtr->memUnitBaseAddrInfo.mif[0] =  MIF_0_BASE_ADDR;/*400G_0*/
        devObjPtr->memUnitBaseAddrInfo.mif[1] =  MIF_1_BASE_ADDR;/*400G_1*/
        devObjPtr->memUnitBaseAddrInfo.mif[2] =  MIF_2_BASE_ADDR;/*400G_2*/

        /* LMU support */
        devObjPtr->memUnitBaseAddrInfo.lmu[0] = LMU_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.lmu[1] = LMU_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.lmu[2] = LMU_2_BASE_ADDR;

        devObjPtr->numOfLmus = 3;

        devObjPtr->memUnitBaseAddrInfo.tsu[0] = TSU_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.tsu[1] = TSU_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.tsu[2] = TSU_2_BASE_ADDR;

        /* support for the 400G MAC */
        devObjPtr->memUnitBaseAddrInfo.macMib[0]  = MAC_MTIP_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[0] = MAC_MTIP_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[0]  = MAC_MTIP_MAC_PCS_0_BASE_ADDR;

        devObjPtr->memUnitBaseAddrInfo.macMib[1]  = MAC_MTIP_MAC_1_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[1] = MAC_MTIP_MAC_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[1]  = MAC_MTIP_MAC_PCS_1_BASE_ADDR;

        devObjPtr->memUnitBaseAddrInfo.macMib[2]  = MAC_MTIP_MAC_2_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[2] = MAC_MTIP_MAC_2_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[2]  = MAC_MTIP_MAC_PCS_2_BASE_ADDR;

        devObjPtr->memUnitBaseAddrInfo.anp[0] =  ANP_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.anp[1] =  ANP_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.anp[2] =  ANP_2_BASE_ADDR;

        devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);

        devObjPtr->dma_specialPortMappingArr = harrier_DMA_specialPortMappingArr;
        devObjPtr->gop_specialPortMappingArr    = NULL;
        devObjPtr->ravens_specialPortMappingArr = NULL;
        devObjPtr->cpuPortSdma_specialPortMappingArr = harrier_DMA_specialPortMappingArr;


        devObjPtr->tcam_numBanksForHitNumGranularity = 2; /* like BC2 */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->support_remotePhysicalPortsTableMode = 1;

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);/*1024 in Falcon , 512 in BC3, 128 in Aldrin2*/
        /*NOTE: support 8K eports like Falcon.
        */

        devObjPtr->cncNumOfUnits = 1;/* must set 1 CNC units otherwise smemLion3Init(...) will set it to 2 */

        devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emMaxNumEntries  = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emAutoLearnNumEntries = 16 * _1K;           /* 64K in Hawk*/
        devObjPtr->limitedResources.eVid  = 6 * _1K;           /*  8K in Hawk*/
        devObjPtr->limitedResources.stgId = 1 * _1K;           /*  4K in Hawk*/
        devObjPtr->limitedResources.nextHop     =  8*_1K;      /* 16K in Hawk*/

        devObjPtr->policerSupport.iplrTableSize =  2*_1K;      /*  4K in Hawk*/
        devObjPtr->policerSupport.numOfIpfix    =  4*_1K;      /* 64K in Hawk*/
        devObjPtr->tcamNumOfFloors   = 3; /*(16 in Hawk) support 3 floors : each 3K of 10B = total 9K of 10B = 4.5K@20B */
        devObjPtr->limitedResources.mllPairs    =  4*_1K;        /*  8K in Hawk*/
        devObjPtr->limitedResources.l2LttMll    = 12*_1K;       /* 16K in Hawk*/
        devObjPtr->limitedResources.l3LttMll    =  4*_1K;       /*  4K in Hawk (the same)*/
        devObjPtr->limitedResources.l2Ecmp      =  8*_1K;       /*  8K in Hawk (the same)*/
        devObjPtr->limitedResources.l2LttEcmp   =  8*_1K;       /* like Hawk according to number of eports(8K) */
        devObjPtr->limitedResources.numOfArps   = 16*_1K;       /*256K in Hawk*/
        devObjPtr->limitedResources.ipvxEcmp    = 2*_1K;          /* 6K in Hawk : each line 2 entries */

        devObjPtr->numofTcamClients = HARRIER_TCAM_NUM_OF_GROUPS_CNS;
    }

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemHarrierSpecificDeviceUnitAlloc;
    }

    /* function will be called from inside smemLion3Init(...) */
    if(devObjPtr->devMemSpecificDeviceMemInitPart2 == NULL)
    {
        /* let Falcon set it's function */
        /*devObjPtr->devMemSpecificDeviceMemInitPart2 = smemBobcat3SpecificDeviceMemInitPart2;*/
    }

    if(devObjPtr->devMemInterruptTreeInit == NULL)
    {
        devObjPtr->devMemInterruptTreeInit = internal_InterruptTreeInit;
    }

    if(devObjPtr->registersDfxDefaultsPtr == NULL)
    {
        /* let Falcon set it's logic */
    }

    smemPhoenixInit(devObjPtr);

    if (isHarrier == GT_TRUE)
    {

        /* Harrier - use the one of Hawk */
        smemHawkBindFindMemoryFunc(devObjPtr);

        /* Init RegDb for DP units */
        smemHarrierInitRegDbDpUnits(devObjPtr);

        /* prepare multi MG units recognition */
        smemHarrierPrepareMultiMgRecognition(devObjPtr);

        /* check that no unit exceed the size of it's unit */
        smemGenericUnitSizeCheck(devObjPtr,harrier_units);

        /* init the FIRMAWARE */
        simulationAc5pFirmwareInit(devObjPtr);
    }

}

/**
* @internal smemHarrierInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemHarrierInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    internal_smemHarrierInit(devObjPtr);
}


/**
* @internal smemHarrierInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHarrierInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* let hawk init it's logic for traffic */
    smemHawkInit_debug_allowTraffic(devObjPtr);

    /* set vlan#1 with members of only : 0,2,4,6,8...38 */
    /* set vlan#1 with fid = 1                          */
    {
        GT_U32  ii;
        GT_U32  address;
        GT_U32  value[32];

        memset(value,0,sizeof(value));
        /* set egress vlan members */
        value[0] = 0x55555555;
        value[1] = 0x00000055;

        address = 0x03b40080;
        for(ii = 0 ; ii < 32; ii++)
        {
            smemRegSet(devObjPtr , address + 4 * ii , value[ii]);
        }

        /* set ingress vlan members */
        address = 0x07a00010;
        for(ii = 0 ; ii < 4; ii++)
        {
            smemRegSet(devObjPtr , address + 4 * ii , value[ii]);
        }

        /* set ingress vlan fid = 1 */
        address = 0x03c40010;

        value[0] = 0x00000003;
        value[1] = 0x003ffc00;
        value[2] = 0xffe00004;
        value[3] = 0x00000001;
        for(ii = 0 ; ii < 4; ii++)
        {
            smemRegSet(devObjPtr , address + 4 * ii , value[ii]);
        }
    }


}
extern GT_U32 smemHawkInit_debug_enable_rx_tx_all_ports_get(void);
extern GT_STATUS smemHawkInit_debug_enable_rx_tx_all_ports_set(IN GT_U32    enable);
extern GT_STATUS smemHawkInit_debug_mru_set(IN GT_U32 mru);
/* debug function to allow test with traffic before CPSS initialization */
GT_STATUS smemHarrierAllowTraffic
(
    IN GT_U32   devNum
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    GT_U32  ii;

    smemHarrierInit_debug_allowTraffic(devObjPtr);
    /* as this call back relevant to init stage only , lets call it for the relevant ports :
       0,2,4,6...38

       this sets the 'EGF force link up' on the ports.
    */
    if(smemHawkInit_debug_enable_rx_tx_all_ports_get())
    {
        for(ii = 0 ; ii < 40 ; ii +=2)
        {
            devObjPtr->devSlanBindPostFuncPtr(devObjPtr,ii,GT_TRUE,GT_TRUE);
        }
    }

    return GT_OK;
}

/* debug function to allow test with traffic before CPSS initialization */
GT_STATUS smemAldrin3MAllowTraffic
(
    IN GT_U32   devNum
)
{
    return smemHarrierAllowTraffic(devNum);
}

extern void smemHawkEgfLinkUpOnSlanBind
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx
);
void snetHawkMifMacEnableTraffic
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  macPort ,
    IN GT_U32                  local_dma_port_number
);

GT_STATUS smemHarrierRxTxEnableSet
(
    IN GT_U32   devNum,
    IN GT_U32   portNum, /*macNum*/
    IN GT_U32   enable,  /*0 or 1*/
    IN GT_U32   isSegmentedPort /*0 or 1*/,
    IN GT_U32   numSerdeses,
    IN GT_U32   fecMode,
    IN GT_U32   sgmii_speed
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    GT_U32  value , serdesBmp;
    SIM_MV_HWS_PORT_FEC_MODE wmFecMode = fecMode;
    GT_U32  portForLink;

    if(portNum >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS ||
       devObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
    {
        simForcePrintf("WM : portNum[%d] not exists \n",
            portNum);
        return GT_OK;
    }

    simForcePrintf("WM : [%s] portNum[%d] for Rx,Tx \n",
        enable ? "Enable":"Disable" ,
        portNum);

    if(enable)
    {
        snetChtPortMacFieldSet(devObjPtr, portNum,
            SNET_CHT_PORT_MAC_FIELDS_segmented_E,
            isSegmentedPort);
    }

    snetChtPortMacFieldSet(devObjPtr, portNum,
        SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E,enable);
    snetChtPortMacFieldSet(devObjPtr, portNum,
        SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E,enable);
    snetChtPortMacFieldSet(devObjPtr, portNum,
        SNET_CHT_PORT_MAC_FIELDS_clock_Enable_E,enable);

    if(sgmii_speed)
    {
        snetChtPortMacFieldSet(devObjPtr, portNum,
            SNET_CHT_PORT_MAC_FIELDS_sgmii_speed_E,
                sgmii_speed);
    }



    if(numSerdeses == 0) numSerdeses = 1;


    if(wmFecMode != SIM_RS_FEC && wmFecMode != SIM_RS_FEC_544_514)
    {
        serdesBmp = 0;
    }
    else
    if(enable)
    {
        serdesBmp = (1 << numSerdeses) - 1;
    }
    else
    {
        serdesBmp = 0;
    }
    value = serdesBmp << 16 | numSerdeses;

    snetChtPortMacFieldSet(devObjPtr, portNum,
        SNET_CHT_PORT_MAC_FIELDS_fec_RS_FEC_Enable_Lane_E,value);

    if(wmFecMode != SIM_RS_FEC_544_514)
    {
        serdesBmp = 0;
    }
    else
    if(enable)
    {
        serdesBmp = (1 << numSerdeses) - 1;
    }
    else
    {
        serdesBmp = 0;
    }
    value = serdesBmp << 16 | numSerdeses;
    snetChtPortMacFieldSet(devObjPtr, portNum,
        SNET_CHT_PORT_MAC_FIELDS_fec_KP_Mode_Enable_E,value);

    if(!enable)
    {
        snetChtPortMacFieldSet(devObjPtr, portNum,
            SNET_CHT_PORT_MAC_FIELDS_segmented_E,
            isSegmentedPort);
    }

    if(enable)
    {
        snetHawkMifMacEnableTraffic(devObjPtr,portNum,
            portNum % devObjPtr->multiDataPath.info[0].dataPathNumOfPorts);
    }

    /* support a port change link on segmented port */
    portForLink = snetChtExtendedPortMacGet(devObjPtr,portNum,GT_TRUE);

    /* update link up/down */
    snetLinkStateNotify(devObjPtr, portForLink, devObjPtr->portsArr[portNum].linkStateWhenNoForce);

    /* needed for EGF link up/down (with accordance to link status) */
    /* set with flag 0x80000000 , meaning that need to use map to RxDma virtualPortNum */
    smemHawkEgfLinkUpOnSlanBind(devObjPtr ,0x80000000 | portNum, enable , enable);

    return GT_OK;
}


/* init a port : mvHwsMifInit() */
static void smemHarrierMifInit
(
    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   macPort
)
{
    GT_U32  regValue , regAddr;
    GT_U32  local_dma_port_number;
    SMEM_CHT_PORT_MTI_MIF_TYPE_ENT  mifType;

    local_dma_port_number = macPort % devObjPtr->multiDataPath.info[0].dataPathNumOfPorts;

    for(mifType = SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ; mifType < SMEM_CHT_PORT_MTI_MIF_TYPE___last___E ; mifType++)
    {
        regAddr = MTI_PORT_MIF(devObjPtr,macPort).mif_channel_mapping_register[mifType];
        if(regAddr == SMAIN_NOT_VALID_CNS)
        {
            continue;
        }

        /*update the register via SCIB for active memory */
        scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
        SMEM_U32_SET_FIELD(regValue,18,1,1); /*clock enable*/
        SMEM_U32_SET_FIELD(regValue, 0,6,local_dma_port_number); /*Tx*/
        SMEM_U32_SET_FIELD(regValue, 8,6,local_dma_port_number); /*Rx*/
        scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);
    }


}

/* init a port : hwsHarrierIfInit() */
GT_STATUS smemHarrierPortInit
(
    IN GT_U32   devNum,
    IN GT_U32   portNum
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    GT_U32  macPort = portNum;

    if(portNum >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS ||
       devObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
    {
        /* ignore the 'error' allow iterations on range without device's port list */
        return GT_OK;
    }

    smemHarrierMifInit(devObjPtr,macPort);

    return GT_OK;
}




/**
* @internal smemHarrierInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHarrierInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    static int my_dummy = 0;

    smemPhoenixInit2(devObjPtr);

    if(my_dummy)
    {
        static GT_U32   allowAllPortsRxTx = 0; /*when generate MI for WM , may want to change '1' to '0' */

        smemHawkInit_debug_enable_rx_tx_all_ports_set(allowAllPortsRxTx);
        smemHawkInit_debug_mru_set(9220);

        smemHarrierInit_debug_allowTraffic(devObjPtr);
    }
}


/**
* @internal smemHarrierUnitTxqSdq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq sdq unit
*/
static void smemHarrierUnitTxqSdq
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x00000004)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x00000214)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500,0x0000056c)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700,0x00000768)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000,0x000013fc)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000,2048)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000,0x00004068)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200,0x00004268)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400,0x00004468)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004600,0x00004668)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004800,0x00004868)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000,0x000050d4)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005600,0x000056d4)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300,0x000063d4)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000,0x0000700c)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100,0x0000710c)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007140,0x0000714c)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007180,0x0000718c)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000071c0,0x000071cc)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000,0x00008024)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008030,0x00008030)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008200,216)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008400,108)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008600,216)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000,2048)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000b000,1024)},
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PortConfig*/
        {
            GT_U32  pt;
            for(pt = 0; pt <= NUM_PORTS_PER_DP_UNIT; pt++)
            {
                /*0x00004000 + pt*0x4: where pt (0-26) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortConfig[pt] =
                    0x00004000+ pt*0x4;

                /*0x00004200 + pt*0x4: where pt (0-26) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeLow[pt] =
                    0x00004200+ pt*0x4;

                /*0x00004400  + pt*0x4: where pt (0-26) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeHigh[pt] =
                    0x00004400 + pt*0x4;
            }
        }
    }

   {
  static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
  {
         {DUMMY_NAME_PTR_CNS,           0x00000004,         0x00000006,      1,    0x4    }/*global_config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x4    }/*SDQ_Metal_Fix*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000004,      27,    0x4    }/*Port_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000500,         0x00000003,      1,    0x4    }/*QCN_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008000,         0x000201ff,      1,    0x4    }/*Sdq_Idle*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007000,         0x00000005,      1,    0x4    }/*debug_config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007008,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_lsb*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000700c,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_msb*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002000,         0x60000000,      256,    0x8    }/*queue_cfg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b000,         0x00000381,      256,    0x4    }/*queue_elig_state*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
  };
  static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
  unitPtr->unitDefaultRegistersPtr = &list;
}
}

static void smemHarrierUnitTxqQfc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address */
    {0x0000080C , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteTxqQfcCounterTable1IndirectReadAddress, 17/*bit 17*/},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x00000070)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100,0x0000013C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x00000260)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400,0x0000048C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500,0x00000568)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600,0x00000638)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800,0x00000814)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000,0x000013FC)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800,0x0000187C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900,0x00001984)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000,0x0000235C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000,0x00003068)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300,0x0000331C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400,0x00003400)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500,0x00003568)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600,0x0000361C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000,0x0000406C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100,0x00004108)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
         {DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x4    }/*qfc_metal_fix*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x30000000,      1,    0x4    }/*Global_PFC_conf*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000608,         0x0000007f,      1,    0x4    }/*Uburst_Event_FIFO_Min_Peak*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000610,         0x00000078,      1,    0x4    }/*Xoff_Size_Indirect_Read_Access*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000800,         0x00020000,      1,    0x0    }/*manual entry in order to get 0 */
        ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000078,      1,    0x4    }/*HR_Counters_Indirect_Read_Access*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004004,         0x0001ffff,      27,    0x4    }/*Port_HR_Counters_Threshold*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004108,         0x0001ffff,      1,    0x4    }/*HR_Counter_Min_Peak*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
    };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data    = 0x00000808;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address = 0x0000080C;
}

/**
* @internal smemHarrierUnitTxqPds function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
 static void smemHarrierUnitTxqPds
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000,32640)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000,35776)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000,8944)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037000,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037800,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038800,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039000,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039800,1024)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000,2048)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041000,0x00041034)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041040,0x0004183C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00042000,0x00042018)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00043000,0x00043010)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00043020,0x0004341C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000440F8,0x00044118)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044120,0x00044120)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044128,0x00044128)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044130,0x00044130)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004413C,0x0004413C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044144,0x00044144)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004414C,0x00044158)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044164,0x00044164)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004416C,0x0004416C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044174,0x0004417C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044200,0x00044204)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044300,0x0004430C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044400,0x0004440C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044440,0x0004444C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044480,0x0004448C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000444C0,0x000444CC)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044500,0x0004450C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044540,0x0004454C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045000,0x0004503C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045100,0x0004513C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045200,0x0004523C)},
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045300,0x0004533C)},

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
       {
         {DUMMY_NAME_PTR_CNS,           0x00041008,         0x00000f94,      1,    0x4    }/*Max_PDS_size_limit_for_pdx*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041000,         0x0000ffff,      1,    0x4    }/*PDS_Metal_Fix*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041004,         0x00000040,      1,    0x4    }/*Global_PDS_CFG*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041028,         0x00000022,      1,    0x4    }/*FIFOs_Limits*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004100c,         0x00000fd4,      1,    0x4    }/*Max_PDS_size_limit_for_PB*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041010,         0x00000100,      1,    0x4    }/*Max_Num_Of_Long_Queues*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041014,         0x0000000f,      1,    0x4    }/*Tail_Size_for_PB_Wr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041018,         0x00000014,      1,    0x4    }/*PID_Empty_Limit_for_PDX*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004101c,         0x00000014,      1,    0x4    }/*PID_Empty_Limit_for_PB*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041020,         0x0000000c,      1,    0x4    }/*NEXT_Empty_Limit_for_PDX*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041024,         0x0000000c,      1,    0x4    }/*NEXT_Empty_Limit_for_PB*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041030,         0x000000be,      1,    0x4    }/*PB_Full_Limit*/
        ,{DUMMY_NAME_PTR_CNS,           0x00041034,         0x0000000c,      1,    0x4    }/*PB_Read_Req_FIFO_Limit*/
        ,{DUMMY_NAME_PTR_CNS,           0x00042004,         0x00000002,      1,    0x4    }/*PDS_Interrupt_Summary_Mask*/
        ,{DUMMY_NAME_PTR_CNS,           0x00043000,         0x0000001f,      1,    0x4    }/*Idle_Register*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004410c,         0x000007ff,      1,    0x4    }/*Free_Next_FIFO_Min_Peak*/
        ,{DUMMY_NAME_PTR_CNS,           0x00044114,         0x00000fff,      1,    0x4    }/*Free_PID_FIFO_Min_Peak*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004417c,         0x000003ff,      1,    0x4    }/*FIFO_State_Latency_Min_Peak*/
        ,{DUMMY_NAME_PTR_CNS,           0x00045100,         0x0000002a,      16,    0x4    }/*Profile_Long_Queue_Limit*/
        ,{DUMMY_NAME_PTR_CNS,           0x00045200,         0x00000026,      16,    0x4    }/*Profile_Head_Empty_Limit*/
        ,{DUMMY_NAME_PTR_CNS,           0x00045000,         0x00000001,      16,    0x4    }/*Profile_Long_Queue_Enable*/
        ,{DUMMY_NAME_PTR_CNS,           0x00044300,         0x00000005,      1,    0x4    }/*pds_debug_configurations*/
        ,{DUMMY_NAME_PTR_CNS,           0x00044308,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_lsb*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004430c,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_msb*/
        ,{DUMMY_NAME_PTR_CNS,           0x00044508,         0x00000400,      1,    0x4    }/*pds_pb_write_req_count_type_ref*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004450c,         0x00003c00,      1,    0x4    }/*pds_pb_write_req_count_type_mask*/
        ,{DUMMY_NAME_PTR_CNS,           0x00044548,         0x00000400,      1,    0x4    }/*pds_pb_read_req_count_type_ref*/
        ,{DUMMY_NAME_PTR_CNS,           0x0004454c,         0x00003c00,      1,    0x4    }/*pds_pb_read_req_count_type_mask*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
    };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

static void smemHarrierUnitTxqPsi
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         /*PSI_REG*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x00000104)},
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x00000238)}
         /*PDQ*/
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000000, HARRIER_PSI_SCHED_OFFSET+0x00000004)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000008, HARRIER_PSI_SCHED_OFFSET+0x0000000C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000010, HARRIER_PSI_SCHED_OFFSET+0x00000014)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000018, HARRIER_PSI_SCHED_OFFSET+0x0000001C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000020, HARRIER_PSI_SCHED_OFFSET+0x00000024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000028, HARRIER_PSI_SCHED_OFFSET+0x0000002C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000030, HARRIER_PSI_SCHED_OFFSET+0x00000034)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000038, HARRIER_PSI_SCHED_OFFSET+0x0000003C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000040, HARRIER_PSI_SCHED_OFFSET+0x00000044)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000048, HARRIER_PSI_SCHED_OFFSET+0x0000004C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000050, HARRIER_PSI_SCHED_OFFSET+0x00000054)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000800, HARRIER_PSI_SCHED_OFFSET+0x00000804)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000808, HARRIER_PSI_SCHED_OFFSET+0x0000080C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000810, HARRIER_PSI_SCHED_OFFSET+0x00000814)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000818, HARRIER_PSI_SCHED_OFFSET+0x0000081C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00000A00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00001A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00001E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00002200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00002600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00002A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00002E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00003200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00003600, HARRIER_PSI_SCHED_OFFSET+0x00003604)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00003608, HARRIER_PSI_SCHED_OFFSET+0x0000360C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00003800, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00004800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00004C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00005000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00005400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00005800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00005C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00006000, HARRIER_PSI_SCHED_OFFSET+0x00006004)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00006008, HARRIER_PSI_SCHED_OFFSET+0x0000600C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00006200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00007200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00007600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00007A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00007E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00008200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00008600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00008A00, HARRIER_PSI_SCHED_OFFSET+0x00008A04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00008A08, HARRIER_PSI_SCHED_OFFSET+0x00008A0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00008C00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00009C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000A000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000A400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000A800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000AC00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000B000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000B400, HARRIER_PSI_SCHED_OFFSET+0x0000B404)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000B408, HARRIER_PSI_SCHED_OFFSET+0x0000B40C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000B600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000C000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0000E000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00010000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00012000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00014000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00016000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00016400, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00016600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00016A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00016E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00017200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00017600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00017A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00017E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00018200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00018600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00018A00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00018C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00019000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00019400, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00019600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00019A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00019E00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001A000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001B000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001D000, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001E000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001F000, HARRIER_PSI_SCHED_OFFSET+0x0001F004)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001F008, HARRIER_PSI_SCHED_OFFSET+0x0001F00C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001F010, HARRIER_PSI_SCHED_OFFSET+0x0001F014)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001F200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001F600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001FA00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0001FE00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020A00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020A10, HARRIER_PSI_SCHED_OFFSET+0x00020A14)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020A18, HARRIER_PSI_SCHED_OFFSET+0x00020A1C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020A20, HARRIER_PSI_SCHED_OFFSET+0x00020A24)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020B00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020C00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020D00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020D10, HARRIER_PSI_SCHED_OFFSET+0x00020D14)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00020E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00021200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00021600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00021A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00021E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022640, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x000226C0, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022740, HARRIER_PSI_SCHED_OFFSET+0x00022744)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022748, HARRIER_PSI_SCHED_OFFSET+0x0002274C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022750, HARRIER_PSI_SCHED_OFFSET+0x00022754)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022760, HARRIER_PSI_SCHED_OFFSET+0x00022764)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022770, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022C00, HARRIER_PSI_SCHED_OFFSET+0x00022C04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00022E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00023200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00023600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00023A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00023E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024680, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024780, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024880, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x000248C0, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024900, HARRIER_PSI_SCHED_OFFSET+0x00024904)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024908, HARRIER_PSI_SCHED_OFFSET+0x0002490C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024910, HARRIER_PSI_SCHED_OFFSET+0x00024914)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024920, HARRIER_PSI_SCHED_OFFSET+0x00024924)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024930, HARRIER_PSI_SCHED_OFFSET+0x00024934)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00024E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00025200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00025600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00025A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00025E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026680, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026780, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026880, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x000268C0, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026900, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026920, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026940, HARRIER_PSI_SCHED_OFFSET+0x00026944)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026948, HARRIER_PSI_SCHED_OFFSET+0x0002694C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026950, HARRIER_PSI_SCHED_OFFSET+0x00026954)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026960, HARRIER_PSI_SCHED_OFFSET+0x00026964)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026970, HARRIER_PSI_SCHED_OFFSET+0x00026974)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00026A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00027000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029000, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029A00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029B00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029C00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029C40, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029C80, HARRIER_PSI_SCHED_OFFSET+0x00029C84)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029C88, HARRIER_PSI_SCHED_OFFSET+0x00029C8C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029CA0, HARRIER_PSI_SCHED_OFFSET+0x00029CA4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029CA8, HARRIER_PSI_SCHED_OFFSET+0x00029CAC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029CB0, HARRIER_PSI_SCHED_OFFSET+0x00029CB4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x00029CB8, HARRIER_PSI_SCHED_OFFSET+0x00029CBC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002A000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C000, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C048, HARRIER_PSI_SCHED_OFFSET+0x0002C04C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C050, HARRIER_PSI_SCHED_OFFSET+0x0002C054)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C058, HARRIER_PSI_SCHED_OFFSET+0x0002C05C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C060, HARRIER_PSI_SCHED_OFFSET+0x0002C064)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C068, HARRIER_PSI_SCHED_OFFSET+0x0002C06C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C070, HARRIER_PSI_SCHED_OFFSET+0x0002C074)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C078, HARRIER_PSI_SCHED_OFFSET+0x0002C07C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C080, HARRIER_PSI_SCHED_OFFSET+0x0002C084)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C088, HARRIER_PSI_SCHED_OFFSET+0x0002C08C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C090, HARRIER_PSI_SCHED_OFFSET+0x0002C094)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C098, HARRIER_PSI_SCHED_OFFSET+0x0002C09C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0A0, HARRIER_PSI_SCHED_OFFSET+0x0002C0A4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0A8, HARRIER_PSI_SCHED_OFFSET+0x0002C0AC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0B0, HARRIER_PSI_SCHED_OFFSET+0x0002C0B4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0B8, HARRIER_PSI_SCHED_OFFSET+0x0002C0BC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0C0, HARRIER_PSI_SCHED_OFFSET+0x0002C0C4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0C8, HARRIER_PSI_SCHED_OFFSET+0x0002C0CC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0D0, HARRIER_PSI_SCHED_OFFSET+0x0002C0D4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0D8, HARRIER_PSI_SCHED_OFFSET+0x0002C0DC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0E0, HARRIER_PSI_SCHED_OFFSET+0x0002C0E4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0E8, HARRIER_PSI_SCHED_OFFSET+0x0002C0EC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0F0, HARRIER_PSI_SCHED_OFFSET+0x0002C0F4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C0F8, HARRIER_PSI_SCHED_OFFSET+0x0002C0FC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C100, HARRIER_PSI_SCHED_OFFSET+0x0002C104)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C108, HARRIER_PSI_SCHED_OFFSET+0x0002C10C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C110, HARRIER_PSI_SCHED_OFFSET+0x0002C114)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C118, HARRIER_PSI_SCHED_OFFSET+0x0002C11C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C120, HARRIER_PSI_SCHED_OFFSET+0x0002C124)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C128, HARRIER_PSI_SCHED_OFFSET+0x0002C12C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C130, HARRIER_PSI_SCHED_OFFSET+0x0002C134)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C138, HARRIER_PSI_SCHED_OFFSET+0x0002C13C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C140, HARRIER_PSI_SCHED_OFFSET+0x0002C144)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C148, HARRIER_PSI_SCHED_OFFSET+0x0002C14C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C150, HARRIER_PSI_SCHED_OFFSET+0x0002C154)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C158, HARRIER_PSI_SCHED_OFFSET+0x0002C15C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C160, HARRIER_PSI_SCHED_OFFSET+0x0002C164)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C168, HARRIER_PSI_SCHED_OFFSET+0x0002C16C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C170, HARRIER_PSI_SCHED_OFFSET+0x0002C174)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (HARRIER_PSI_SCHED_OFFSET+0x0002C178, HARRIER_PSI_SCHED_OFFSET+0x0002C17C)}





    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

   {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
           {DUMMY_NAME_PTR_CNS,           0x00000208,         0x00001000,      1,    0x4    }/*Credit_Value*/
          ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x0000ffff,      1,    0x4    }/*PSI_Metal_Fix_Register*/
          ,{DUMMY_NAME_PTR_CNS,           0x00000200,         0x00000002,      1,    0x4    }/*QmrSchPLenBursts*/
          ,{DUMMY_NAME_PTR_CNS,           0x00000204,         0x00000002,      1,    0x4    }/*QmrSchBurstsSent*/
          ,{DUMMY_NAME_PTR_CNS,           0x0000021c,         0x000003ff,      1,    0x4    }/*MSG_FIFO_Min_Peak*/
         ,{DUMMY_NAME_PTR_CNS,           0x00000228,         0x00000001,      1,    0x4    }/*Queue_Status_Read_Reply*/
         ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000007,      1,    0x4    }/*Psi_Idle*/
          ,{DUMMY_NAME_PTR_CNS,           0x00000000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrorStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000008+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*FirstExcp_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000010+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrCnt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000018+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ExcpCnt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000020+HARRIER_PSI_SCHED_OFFSET,         0x00000037,      1,    0x8    }/*ExcpMask_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00000024+HARRIER_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*ExcpMask_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000028+HARRIER_PSI_SCHED_OFFSET,         0x00001033,      1,    0x8    }/*Identity_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000030+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ForceErr_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000038+HARRIER_PSI_SCHED_OFFSET,         0x04081020,      1,    0x8    }/*ScrubSlots_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000003c+HARRIER_PSI_SCHED_OFFSET,         0x00000004,      1,    0x8    }/*ScrubSlots_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000040+HARRIER_PSI_SCHED_OFFSET,         0x00000007,      1,    0x8    }/*BPMsgFIFO_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00000044+HARRIER_PSI_SCHED_OFFSET,         0x00000008,      1,    0x8    }/*BPMsgFIFO_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000048+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*TreeDeqEn_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000050+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PDWRREnReg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000800+HARRIER_PSI_SCHED_OFFSET,         0x00500000,      1,    0x8    }/*PPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000808+HARRIER_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*PPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000080c+HARRIER_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*PPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000810+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PortExtBPEn_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000818+HARRIER_PSI_SCHED_OFFSET,         0x00001001,      1,    0x8    }/*PBytePerBurstLimit_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003600+HARRIER_PSI_SCHED_OFFSET,         0x071c0000,      1,    0x8    }/*CPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003608+HARRIER_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*CPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000360c+HARRIER_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*CPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006000+HARRIER_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*BPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006008+HARRIER_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*BPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000600c+HARRIER_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*BPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008a00+HARRIER_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*APerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008a08+HARRIER_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*APerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00008a0c+HARRIER_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*APerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b400+HARRIER_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*QPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b408+HARRIER_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*QPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000b40c+HARRIER_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*QPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f008+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ScrubDis_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f010+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      1,    0x8    }/*Plast_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001f014+HARRIER_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*Plast_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a10+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00020a14+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a18+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a20+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020d10+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022740+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00022744+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022748+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022750+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022760+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022c00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024900+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00024904+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024908+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024910+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024920+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024930+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026940+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00026944+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026948+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026950+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026960+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ABnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026970+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*APerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c80+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c88+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00029c8c+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029ca0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QBnkEccErrStatus_StartAddr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029ca8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank1EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029cb0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank2EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029cb8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank3EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c048+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccConfig_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c050+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters0*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c058+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters1*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c060+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters2*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c068+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters3*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c070+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters4*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c078+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters5*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c080+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters6*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c088+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters7*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c090+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters8*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c098+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters9*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0a0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters10*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0a8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters11*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0b0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters12*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0b8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters13*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0c0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters14*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0c8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters15*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0d0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters16*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0d8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters17*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0e0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters18*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0e8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters19*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0f0+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters20*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0f8+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters21*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c100+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters22*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c108+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters23*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c110+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters24*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c118+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters25*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c120+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters26*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c128+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters27*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c130+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters28*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c138+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters29*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c140+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters30*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c148+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters31*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c150+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters32*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c158+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters33*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c160+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters34*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c168+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters35*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c170+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters36*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c178+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters37*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000a00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*PortEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001a00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001e00+HARRIER_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00001e04+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002200+HARRIER_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002204+HARRIER_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002a00+HARRIER_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosLo*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002a04+HARRIER_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002e00+HARRIER_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosHi*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002e04+HARRIER_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003800+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*ClvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004800+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004c00+HARRIER_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*ClvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005000+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00005004+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005400+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005800+HARRIER_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*ClvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005c00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvltoPortAndBlvlRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*BlvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007600+HARRIER_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*BlvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007a00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00007a04+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007e00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008200+HARRIER_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*BlvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BLvltoClvlAndAlvlRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008c00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00009c00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a000+HARRIER_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*AlvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a400+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000a404+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a800+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000ac00+HARRIER_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*AlvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ALvltoBlvlAndQueueRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*QueueEligPrioFunc*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000c000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000e000+HARRIER_PSI_SCHED_OFFSET,         0x0fff0fff,      512,    0x8    }/*QueueTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00010000+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00010004+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00012000+HARRIER_PSI_SCHED_OFFSET,         0x00000040,      512,    0x8    }/*QueueQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00014000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueAMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016000+HARRIER_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00016004+HARRIER_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016400+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00016404+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016600+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016a00+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016e00+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio2*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017200+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017600+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio4*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017a00+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio5*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017e00+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio6*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018200+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio7*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018600+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00018604+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018a00+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00018a04+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018c00+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*CLvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019000+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019004+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019400+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019404+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019600+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*BlvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019a00+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019a04+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019e00+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019e04+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001a000+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*AlvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001b000+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001b004+HARRIER_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001d000+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001d004+HARRIER_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001e000+HARRIER_PSI_SCHED_OFFSET,         0x00000001,      512,    0x8    }/*QueueDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f200+HARRIER_PSI_SCHED_OFFSET,         0x000001fe,      64,    0x8    }/*PortNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001fa00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001fe00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020200+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020600+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*PortWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020b00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PortBPFromSTF*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020c00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PortBPFromQMgr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020d00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*TMtoTMPortBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020e00+HARRIER_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*ClvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021600+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021a00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021e00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022200+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*ClvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022640+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000226c0+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022770+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ClvlBPFromSTF*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022800+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMClvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022e00+HARRIER_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*BlvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023600+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023a00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023e00+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024200+HARRIER_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*BlvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024680+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024780+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024880+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000248c0+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024a00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMBlvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024e00+HARRIER_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*AlvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025200+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025600+HARRIER_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025a00+HARRIER_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025e00+HARRIER_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026200+HARRIER_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026600+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*AlvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026680+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026780+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026880+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000268c0+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026900+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026920+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026a00+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMAlvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00027000+HARRIER_PSI_SCHED_OFFSET,         0x00000002,      512,    0x8    }/*QueueNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      16,    0x8    }/*QueueWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029200+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029600+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029a00+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029b00+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c00+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c40+HARRIER_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002a000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*TMtoTMQueueBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c000+HARRIER_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*QueuePerStatus*/
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

static void smemHarrierUnitTxqPdx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(txqPdxQueueGroupMap)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000200,512)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400,512)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800,512)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00,0x00000C0C)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000,0x00001008)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001010,0x00001018)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020,0x00001028)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030,0x00001038)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200,0x00001208)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001210,0x00001218)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001220,0x00001228)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300,0x00001310)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400,0x00001414)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001420,0x00001420)},
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500,0x00001500)}

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PreemptionEnable*/
        {
            GT_U32  i;
            for(i = 0; i < 4; i++)
            {
                /*0x00000c00 + p*0x4: where pt (0-1) represents preemp_en_regs */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_PDX.PreemptionEnable[i] =
                   0x00000c00+4*i;
            }
        }
    }

    {
  static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
    {
         {DUMMY_NAME_PTR_CNS,           0x00001500,         0x0000ffff,      1,    0x4    }/*pdx_metal_fix*/
        ,{DUMMY_NAME_PTR_CNS,           0x00001220,         0x00000f94,      3,    0x4    }/*pfcc_burst_fifo_thr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00001210,         0x00000f94,      3,    0x4    }/*global_burst_fifo_thr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00001200,         0x00000f94,      3,    0x4    }/*global_burst_fifo_available_entries*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
    };
  static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
  unitPtr->unitDefaultRegistersPtr = &list;
   }
}


/**
* @internal smemHarrierSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemHarrierSpecificDeviceUnitAlloc_TXQ_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr []= {
         {STR(UNIT_TXQ_QFC0)      ,smemHarrierUnitTxqQfc }
        ,{STR(UNIT_TXQ_SDQ0)      ,smemHarrierUnitTxqSdq }
        ,{STR(UNIT_TXQ_PDS0)      ,smemHarrierUnitTxqPds }
        ,{STR(UNIT_TXQ_PSI)       ,smemHarrierUnitTxqPsi }
        ,{STR(UNIT_TXQ_PDX)       ,smemHarrierUnitTxqPdx }

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

}



/**
* @internal smemHarrierUnitEgfQag function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-QAG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHarrierUnitEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 65536 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(44, 8),SMEM_BIND_TABLE_MAC(egfQagEgressEPort)}
            /* TC_DP_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfQagTcDpMapper)}
            /* VOQ_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00094000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(7, 4)}
            /* Port_Target_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00096000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(egfQagPortTargetAttribute)}
            /* Port_Enq_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00097000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(egfQagTargetPortMapper)}
            /* Port_Source_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00098000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(9, 4) ,SMEM_BIND_TABLE_MAC(egfQagPortSourceAttribute)}
            /* EVIDX_Activity_Status_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00099000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}
            /* Cpu_Code_To_Loopback_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0009A000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4), SMEM_BIND_TABLE_MAC(egfQagCpuCodeToLbMapper)}

            /*VLAN Q Offset Mapping Table  Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A8000,24576), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4), SMEM_BIND_TABLE_MAC(egfQagVlanQOffsetMappingTable)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000a0000, 0x000a0008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000a0010, 0x000a0018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000a0020, 0x000a0020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000a0170, 0x000a019C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000a0A00, 0x000a0A04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000b0000, 0x000b0000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000b0010, 0x000b0010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000b0020, 0x000b0020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000b0100, 0x000b0114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000b0200, 0x000b0200)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 524288),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8),SMEM_BIND_TABLE_MAC(egfQagEVlanDescriptorAssignmentAttributes)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  index = 0;

        /* support Falcon , Hawk , Phoenix */
        /* 8 lines for single vlan */
        chunksMem[index].numOfRegisters = (numEVlans*8) * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Cut Through Configuration */
            {DUMMY_NAME_PTR_CNS,    0x000A0A04,         0x00007FFF,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,    0x00097000,         0x00010000,      128,  0x4}
           ,{NULL,                  0,                  0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit EGF_qag */
        {/*start of unit uniMultiArb */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.uniMultiCtrl = 0x000b0100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.multiShaperCtrl = 0x000b010c;

        }/*end of unit uniMultiArb */


        {/*start of unit miscellaneous */
            /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.miscellaneous.badAddrLatchReg = 0x000b0200;*/

        }/*end of unit miscellaneous */


        {/*start of unit global */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.global.QAGInterruptsCause = 0x000b0000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.global.QAGInterruptsMask = 0x000b0010;

        }/*end of unit global */


        {/*start of unit distributor */
            {/*c9b0170+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackEnableConfig[n] =
                        0x000a0170+0x4*n;
                }/* end of loop n */
            }/*000a0170+0x4*n*/
            {/*000a0180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.toAnalyzerLoopback[n] =
                        0x000a0180+0x4*n;
                }/* end of loop n */
            }/*000a0180+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.distributorGeneralConfigs = 0x000a0000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.TCLoopbackConfig = 0x000a0004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.eVIDXActivityStatusWriteMask = 0x000a0008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.voQAssignmentGeneralConfig0 = 0x000a0010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.pktHashForVoQAssignment0 = 0x000a0014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.pktHashForVoQAssignment1 = 0x000a0018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.voQAssignmentGeneralConfig1 = 0x000a0020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackForwardingGlobalConfig = 0x000a0a00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.cutThroughConfig = 0x000a0a04;

        }/*end of unit distributor */


    }/*end of unit EGF_qag */

}

/**
* @internal smemHarrierUnitPreq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PREQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHarrierUnitPreq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{   /*sync to Cider 180606*/
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* ROC (read only clear) counters */
        { 0x00000710, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000714, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000720, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000724, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000730, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000734, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000750, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000754, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000760, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00000764, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000054, 0x00000054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000030C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000350)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000358, 0x000003D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000404, 0x00000414)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000041C, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000608)}

            /* Egress MIB counters*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000720, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x00000734)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000750, 0x00000754)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000760, 0x00000764)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000860, 0x00000864)}

            /*Queue Offset Profile Mapping */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000097C)}

            /* PREQ Queue Port Mapping Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC( 32,  4), SMEM_BIND_TABLE_MAC(preqQueuePortMapping)}

            /* PREQ Profiles Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00030000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(67, 16), SMEM_BIND_TABLE_MAC(preqProfiles)}
            /* PREQ Queue Configurations Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00040000, 12288),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC( 88, 16), SMEM_BIND_TABLE_MAC(preqQueueConfiguration)}
            /* PREQ Port Profile Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00050000,  256),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(108, 16), SMEM_BIND_TABLE_MAC(preqPortProfile)}
            /* PREQ Target Physical Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00060000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(  6,  4), SMEM_BIND_TABLE_MAC(preqTargetPhyPort)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        GT_U32 n;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.globalConfig = 0x00000000;
        for (n = 0; (n <= 1); n++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBCntrsPortSetConfig[n] = 0x00000708 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBCntrsSetConfig[n]     = 0x00000700 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBTailDroppedPktCntr[n] = 0x00000750 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBOutgoingUcPktCntr[n]  = 0x00000710 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBOutgoingMcPktCntr[n]  = 0x00000720 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBOutgoingBcPktCntr[n]  = 0x00000730 + (4 * n);
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.
                egrMIBCntrs.egrMIBCtrlPktCntr[n]        = 0x00000760 + (4 * n);
        }
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
              /* preqTargetPhyPort initial values */
              {DUMMY_NAME_PTR_CNS,           0x00060000,         0x00000001,      1024,    0x4     }
             ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000003,      1,    0x4    }/*Global_Config*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000054,         0x0000ffff,      1,    0x4    }/*PREQ_Metal_Fix*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000100,         0x00000028,      1,    0x4    }/*Fifos_Depth*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000828,         0x0000001f,      1,    0x4    }/*ingress_pkt_config*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000860,         0x0000003f,      1,    0x4    }/*query_fifo_min_peak*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000404,         0x000fffff,      1,    0x4    }/*global_tail_drop_limit*/
             ,{DUMMY_NAME_PTR_CNS,           0x0000041c,         0x000fffff,      4,    0x4    }/*mc_td_configuration*/
             ,{NULL,            0,           0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHarrierUnitDfxServer function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the DFX unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHarrierUnitDfxServer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG() */
    {0x000f800c, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteDfxServerResetControlReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 0x000F7FFC/* 1015804*/)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8000, 0x000F8018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8020, 0x000F8038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8040, 0x000F8044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8060, 0x000F806C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8078, 0x000F80AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80B0, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80C0, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F80D0, 0x000F80DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8100, 0x000F8114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8120, 0x000F8120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8200, 0x000F8230)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F823C, 0x000F8248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8250, 0x000F82AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F82CC, 0x000F82D8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8340, 0x000F8368)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F83C0, 0x000F841C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8450, 0x000F849C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8500, 0x000F8508)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8510, 0x000F8520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8C80, 0x000F8C90)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8D00, 0x000F8D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8DA0, 0x000F8DF0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8E18, 0x000F8E4C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F8F00, 64)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F9000, 1024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000FC000, 12288)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
             {DUMMY_NAME_PTR_CNS,            0x000f8004,         0x02040053,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8008,         0x2ff92830,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f800c,         0x007020de,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8010,         0x0000000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8014,         0xfffffffb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8018,         0x04000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8020,         0x0000019f,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8030,         0x0000039f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8034,         0x0000019f,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8040,         0x0000019f,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8060,         0x0000019f,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f807c,         0x8011e214,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8080,         0x30a88019,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8084,         0x33981000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8088,         0x08c01a09,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8090,         0x0000019f,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f809c,         0xff8000ff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80a4,         0x000001f8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80ac,         0x00aa9000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80d0,         0xf0f01032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80d4,         0x00000781,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80d8,         0x0184e140,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8120,         0x0d08007f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f823c,         0x00000357,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8248,         0x0000ffff,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f825c,         0xffffffc2,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8264,         0x1c000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8268,         0x25001000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f826c,         0xe1048200,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8270,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8274,         0x208046e9,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8278,         0x00007a49,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f827c,         0x01ba4000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8280,         0x2ff82000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8284,         0x00700700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8288,         0x00705701,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f828c,         0x00700704,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8290,         0x00710719,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8294,         0x00700707,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8298,         0x0070070a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f829c,         0x00700700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82a0,         0x00705700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82a4,         0x0070470b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82a8,         0x00701705,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82ac,         0x00700701,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82d0,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8340,         0x00001018,      9,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8364,         0x0000001c,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8498,         0x00400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8500,         0x0000001e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8504,         0x00000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8510,         0x00800000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8514,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d00,         0x00176101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d04,         0x2d49d809,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d08,         0x003c20c5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d0c,         0x00010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d14,         0x2c000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d18,         0x10c84000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d1c,         0xcba98765,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d24,         0xc6c34f30,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d28,         0x012401a5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d2c,         0x01f82008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e20,         0x4000404c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e24,         0x78240297,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e28,         0x81e1ffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e2c,         0x8003e0fd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e3c,         0x203a1172,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e40,         0x00009508,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e44,         0x00007fc7,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8e48,         0x00001000,      1,    0x4 } /* Temp readout is valid */
            ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }


}

/**
* @internal smemHarrierSpecificDeviceUnitAlloc_DIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemHarrierSpecificDeviceUnitAlloc_DIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
          {STR(UNIT_DFX_SERVER)      ,smemHarrierUnitDfxServer },

        /* must be last */
         {NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}

