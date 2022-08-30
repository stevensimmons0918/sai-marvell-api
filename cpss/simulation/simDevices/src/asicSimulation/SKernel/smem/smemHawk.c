/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemHawk.c
*
* DESCRIPTION:
*       Hawk memory mapping implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <common/Utils/Math/sMath.h>
#include <asicSimulation/SKernel/smem/smemHarrier.h>
#include <asicSimulation/SKernel/suserframes/snetHawkExactMatch.h>

/* was 1K in Falcon */
#define SIP6_10_NUM_DEFAULT_E_PORTS_CNS         128

/* 4 MG units */
#define HAWK_NUM_MG_UNITS  4
/* the size in bytes of the MG unit */
#define MG_SIZE             _1M
/* base address of the CNM unit that is single for 2 tiles */
#define CNM_OFFSET_CNS       0x3C000000
/* base address of the MG 0_0 unit . MG_0_0 to MG_0_3 serve tile 0 */
#define MG_0_0_OFFSET_CNS    (CNM_OFFSET_CNS + 0x00200000)
/* base address of the MG 0_1 unit . MG_0_1 is part of tile 0 */
#define MG_0_1_OFFSET_CNS    (MG_0_0_OFFSET_CNS + 1*MG_SIZE)
/* used for GM devices */
GT_U32   simHawkMgBaseAddr = MG_0_0_OFFSET_CNS;

#define LMU_1_BASE_ADDR 0x35200000 /*PCA*/
#define LMU_2_BASE_ADDR 0x32400000 /*PCA*/
#define LMU_3_BASE_ADDR 0x33400000 /*PCA*/
#define LMU_0_BASE_ADDR 0x34A00000 /*PCA with MACSEC*/

#define TSU_1_BASE_ADDR 0x35300000 /*PCA*/
#define TSU_2_BASE_ADDR 0x32500000 /*PCA*/
#define TSU_3_BASE_ADDR 0x33500000 /*PCA*/
#define TSU_0_BASE_ADDR 0x34B00000 /*PCA with MACSEC*/

#define MAC_MTIP_MAC_0_BASE_ADDR  0x1B800000
#define MAC_MTIP_MAC_1_BASE_ADDR  0x1C800000
#define MAC_MTIP_MAC_2_BASE_ADDR  0x1D800000
#define MAC_MTIP_MAC_3_BASE_ADDR  0x1E800000

#define MAC_MTIP_MAC_PCS_0_BASE_ADDR  0x1F200000
#define MAC_MTIP_MAC_PCS_1_BASE_ADDR  0x1F600000
#define MAC_MTIP_MAC_PCS_2_BASE_ADDR  0x1FA00000
#define MAC_MTIP_MAC_PCS_3_BASE_ADDR  0x1FE00000


#define MTIP_USX_0_MAC_0_BASE_ADDR      0x24800000
#define MTIP_USX_1_MAC_0_BASE_ADDR      0x25000000
#define MTIP_USX_2_MAC_0_BASE_ADDR      0x25800000
#define MTIP_USX_0_MAC_1_BASE_ADDR      0x26800000
#define MTIP_USX_1_MAC_1_BASE_ADDR      0x27000000
#define MTIP_USX_2_MAC_1_BASE_ADDR      0x27800000

#define MTIP_UNIT_USX_0_PCS_0_BASE_ADDR 0x2AC00000
#define MTIP_UNIT_USX_1_PCS_0_BASE_ADDR 0x2B000000
#define MTIP_UNIT_USX_2_PCS_0_BASE_ADDR 0x2B800000
#define MTIP_UNIT_USX_0_PCS_1_BASE_ADDR 0x2CC00000
#define MTIP_UNIT_USX_1_PCS_1_BASE_ADDR 0x2D000000
#define MTIP_UNIT_USX_2_PCS_1_BASE_ADDR 0x2D800000

#define MIF_0_BASE_ADDR           0x1B400000 /*400G_0*/
#define MIF_1_BASE_ADDR           0x1C400000 /*400G_1*/
#define MIF_2_BASE_ADDR           0x1D400000 /*400G_2*/
#define MIF_3_BASE_ADDR           0x1E400000 /*400G_3*/
#define MIF_4_BASE_ADDR           0x24400000 /*USX0*/
#define MIF_5_BASE_ADDR           0x26400000 /*USX1*/
#define MIF_6_BASE_ADDR           0x26600000 /*CPU*/

#define ANP_0_BASE_ADDR           0x1A600000
#define ANP_1_BASE_ADDR           0x1F000000
#define ANP_2_BASE_ADDR           0x1F400000
#define ANP_3_BASE_ADDR           0x1F800000
#define ANP_4_BASE_ADDR           0x1FC00000
#define ANP_5_BASE_ADDR           0x2A000000
#define ANP_6_BASE_ADDR           0x2A200000
#define ANP_7_BASE_ADDR           0x2A400000
#define ANP_8_BASE_ADDR           0x2A600000
#define ANP_9_BASE_ADDR           0x2A800000
#define ANP_10_BASE_ADDR          0x2AA00000
#define ANP_11_BASE_ADDR          0x2C000000
#define ANP_12_BASE_ADDR          0x2C200000
#define ANP_13_BASE_ADDR          0x2C400000
#define ANP_14_BASE_ADDR          0x2C600000
#define ANP_15_BASE_ADDR          0x2C800000
#define ANP_16_BASE_ADDR          0x2CA00000

#define NUM_PORTS_PER_UNIT        8
#define NUM_PORTS_PER_DP_UNIT     26 /*+1 for CPU */
#define HAWK_MAX_PROFILE_CNS      12
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


/* offsets within the : Hawk/Core/EPI/<USX_MAC>MTIP IP USX MAC WRAPPER/<USX_MAC> <MTIP IP USX MAC>MTIP_USX_MAC %a_%b/<MTIP IP USX MAC> MAC/MAC Units %i */
#define USX_MAC_EXT_BASE_OFFSET       0x00000000
#define USX_MAC_MIB_OFFSET            0x00001000
#define USX_PORT0_100G_OFFSET         0x00002000

#define MAC_STEP_PORT_OFFSET      0x00001000
#define USX_START_MIB_TX_PER_PORT 0x0660
#define USX_SIZE_MIB_TX_PER_PORT  (34*4) /*decimal*/
#define USX_SIZE_MIB_TX  (USX_SIZE_MIB_TX_PER_PORT*NUM_PORTS_PER_UNIT)
#define USX_MAC_MIB_UNIT_SIZE           0x1000


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

/********** start : PCS for usx *********/
#define USX_PCS_PORTS_OFFSET      0x00001000
#define USX_PCS_RS_FEC_OFFSET     0x00003000
#define USX_PCS_LPCS_OFFSET       0x00000000
#define USX_PCS_USXM_OFFSET       0x00002000

#define USX_PCS_SERDES_OFFSET     0x00003000 /* for RS_FEC,RS_FEC_STAT,USXM */
#define USX_PCS_STEP_PORT_OFFSET  0x00000100
#define USX_LPCS_STEP_PORT_OFFSET  0x00000080
/********** end  : PCS for usx *********/


#define CPU_MAC_MTIP_MAC_BASE_ADDR      0x1A800000
#define CPU_MAC_MTIP_MAC_PCS_BASE_ADDR  0x1AA00000

#define CPU_MAC_EXT_BASE_OFFSET       0x00000000
#define CPU_MAC_PORT_OFFSET           0x00002000
#define CPU_MAC_MIB_OFFSET            0x00001000

#define CPU_PCS_PORT_OFFSET           0x00000000
#define CPU_PCS_RS_FEC_OFFSET         0x00001000
#define CPU_PCS_LPCS_OFFSET           0x00002000
#define HAWK_PSI_SCHED_OFFSET         (GT_U32)0x00000300

#define MIF_GLOBAL_OFFSET             0x00000000
#define MIF_RX_OFFSET                 0x00000800
#define MIF_TX_OFFSET                 0x00001000

#define ANP_OFFSET                    0x00000000
#define ANP_STEP_PORT_OFFSET          0x000002a4
#define AN_OFFSET                     0x00004000
#define ANP_STEP_UNIT_OFFSET          0x00200000
#define AN_STEP_PORT_OFFSET           0x00001000

#define RS_STATISTIC_CONTER_CAPTURE_OFFSET    0x00000004
#define RS_STATISTIC_CONTER_CODEWORDS_OFFSET  0x00000050

static void smemHawkActiveWriteExactMatchMsg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
static void smemHawkActiveWriteExactMatchGlobalCfg1Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
/**
* @internal smemHawkActiveWriteEMFlowIdAllocationConfiguration1 function
* @endinternal
*
* @brief   Activate CM3.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter - global interrupt bit number.
*
* @param[out] outMemPtr               - Pointer to the memory to copy register's content.
*
*
*/
static void smemHawkActiveWriteEMFlowIdAllocationConfiguration1
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
);

static void smemHawkActiveWriteMacSecCountersControl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);

static void smemHawkActiveWriteMacSecNextPnCtrl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);

/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/Projects/Aldrin3/
_layouts/15/WopiFrame2.aspx?sourcedoc=%2Fsites%2FEBUSites%2FSwitching%2FVLSIDesign%2FChipDesign%2FProjects%2FAldrin3%2FShared%20
Documents%2FDesign%2FAXI%2FHawk_Address_Space_190303%2Exlsm&action=view&activeCell=%27CORE%27!B47
*/
#define UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName)                                , size , 0}

/* the units of HAWK */
static SMEM_GEN_UNIT_INFO_STC hawk_units[] =
{

    /* packet buffer subunits */
     UNIT_INFO_MAC(0x00000000, UNIT_PB_CENTER_BLK              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x00200000, UNIT_PB_COUNTER_BLK             ,64  *     _1K)

    ,UNIT_INFO_MAC(0x00400000, UNIT_PB_WRITE_ARBITER_0         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x00600000, UNIT_PB_WRITE_ARBITER_1         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x00800000, UNIT_PB_WRITE_ARBITER_2         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x00A00000, UNIT_PB_WRITE_ARBITER_3         ,64  *     _1K)

    ,UNIT_INFO_MAC(0x00C00000, UNIT_PB_GPC_GRP_PACKET_WRITE_0  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x00E00000, UNIT_PB_GPC_GRP_PACKET_WRITE_1  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01000000, UNIT_PB_GPC_GRP_PACKET_WRITE_2  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01200000, UNIT_PB_GPC_GRP_PACKET_WRITE_3  ,64  *     _1K)

    ,UNIT_INFO_MAC(0x01400000, UNIT_PB_GPC_GRP_CELL_READ_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01600000, UNIT_PB_GPC_GRP_CELL_READ_1     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01800000, UNIT_PB_GPC_GRP_CELL_READ_2     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01A00000, UNIT_PB_GPC_GRP_CELL_READ_3     ,64  *     _1K)

    ,UNIT_INFO_MAC(0x01C00000, UNIT_PB_GPC_GRP_PACKET_READ_0   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x01E00000, UNIT_PB_GPC_GRP_PACKET_READ_1   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x02000000, UNIT_PB_GPC_GRP_PACKET_READ_2   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x02200000, UNIT_PB_GPC_GRP_PACKET_READ_3   ,64  *     _1K)

    ,UNIT_INFO_MAC(0x02400000, UNIT_PB_NEXT_POINTER_MEMO_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x02600000, UNIT_PB_NEXT_POINTER_MEMO_1     ,64  *     _1K)

    ,UNIT_INFO_MAC(0x02800000, UNIT_PB_SHARED_MEMO_BUF_0       ,64  *     _1K)
    ,UNIT_INFO_MAC(0x02C00000, UNIT_PB_SHARED_MEMO_BUF_1       ,64  *     _1K)
    ,UNIT_INFO_MAC(0x03000000, UNIT_PB_SHARED_MEMO_BUF_2       ,64  *     _1K)

    ,UNIT_INFO_MAC(0x03800000,UNIT_TXQ_PDX            ,64 *     _1K)
    ,UNIT_INFO_MAC(0x03C00000,UNIT_TXQ_PFCC           ,64    * _1K)
    ,UNIT_INFO_MAC(0x03E00000,UNIT_TXQ_PSI            , 1  * _1M)

    ,UNIT_INFO_MAC(0x04000000,UNIT_TXQ_SDQ0  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x04400000,UNIT_TXQ_PDS0  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x04800000,UNIT_TXQ_QFC0  ,64   *    _1K)
    ,UNIT_INFO_MAC(0x04C00000,UNIT_TXQ_SDQ1  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x05000000,UNIT_TXQ_PDS1  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x05400000,UNIT_TXQ_QFC1  ,64   *    _1K)
    ,UNIT_INFO_MAC(0x06000000,UNIT_TXQ_SDQ2  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x06400000,UNIT_TXQ_PDS2  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x06800000,UNIT_TXQ_QFC2  ,64   *    _1K)
    ,UNIT_INFO_MAC(0x06C00000,UNIT_TXQ_SDQ3  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x07000000,UNIT_TXQ_PDS3  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x07400000,UNIT_TXQ_QFC3  ,64   *    _1K)

    ,UNIT_INFO_MAC(0x08000000,UNIT_RX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x08400000,UNIT_RX_DMA_1          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x08800000,UNIT_RX_DMA_2          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x08C00000,UNIT_RX_DMA_3          ,64  *     _1K)

    ,UNIT_INFO_MAC(0x09000000,UNIT_IA                ,64  *     _1K)

    ,UNIT_INFO_MAC(0x09800000,UNIT_TX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x09C00000,UNIT_TX_FIFO           ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0A000000,UNIT_TX_DMA_1          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0A400000,UNIT_TX_FIFO_1         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0A800000,UNIT_TX_DMA_2          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0AC00000,UNIT_TX_FIFO_2         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0B000000,UNIT_TX_DMA_3          ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0B400000,UNIT_TX_FIFO_3         ,64  *     _1K)

    ,UNIT_INFO_MAC(0x34000000,UNIT_PCA_BRG_0              ,16  * _1K )
    ,UNIT_INFO_MAC(0x36000000,UNIT_PCA_PZ_ARBITER_I_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x34200000,UNIT_PCA_PZ_ARBITER_E_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x34800000,UNIT_PCA_SFF_0              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_0_BASE_ADDR, UNIT_PCA_LMU_0        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_0_BASE_ADDR, UNIT_PCA_CTSU_0       ,64  * _1K )
    ,UNIT_INFO_MAC(0x34400000,UNIT_PCA_BRG_1              ,16  * _1K )
    ,UNIT_INFO_MAC(0x36400000,UNIT_PCA_PZ_ARBITER_I_1     ,16  * _1K )
    ,UNIT_INFO_MAC(0x34600000,UNIT_PCA_PZ_ARBITER_E_1     ,16  * _1K )
    ,UNIT_INFO_MAC(0x35000000,UNIT_PCA_SFF_1              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_1_BASE_ADDR, UNIT_PCA_LMU_1        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_1_BASE_ADDR, UNIT_PCA_CTSU_1       ,64  * _1K )
    ,UNIT_INFO_MAC(0x32000000,UNIT_PCA_BRG_2              ,16  * _1K )
    ,UNIT_INFO_MAC(0x32100000,UNIT_PCA_PZ_ARBITER_I_2     ,16  * _1K )
    ,UNIT_INFO_MAC(0x32200000,UNIT_PCA_PZ_ARBITER_E_2     ,16  * _1K )
    ,UNIT_INFO_MAC(0x32300000,UNIT_PCA_SFF_2              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_2_BASE_ADDR, UNIT_PCA_LMU_2        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_2_BASE_ADDR, UNIT_PCA_CTSU_2       ,64  * _1K )
    ,UNIT_INFO_MAC(0x33000000,UNIT_PCA_BRG_3              ,16  * _1K )
    ,UNIT_INFO_MAC(0x33100000,UNIT_PCA_PZ_ARBITER_I_3     ,16  * _1K )
    ,UNIT_INFO_MAC(0x33200000,UNIT_PCA_PZ_ARBITER_E_3     ,16  * _1K )
    ,UNIT_INFO_MAC(0x33300000,UNIT_PCA_SFF_3              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_3_BASE_ADDR ,UNIT_PCA_LMU_3        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_3_BASE_ADDR, UNIT_PCA_CTSU_3       ,64  * _1K )
    ,UNIT_INFO_MAC(0x34C00000,UNIT_PCA_MACSEC_EXT_E_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x36100000,UNIT_PCA_MACSEC_EXT_I_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x35800000,UNIT_PCA_MACSEC_EXT_E_164_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x36800000,UNIT_PCA_MACSEC_EXT_I_164_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x35400000,UNIT_PCA_MACSEC_EXT_E_163_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x36500000,UNIT_PCA_MACSEC_EXT_I_163_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x35C00000,UNIT_PCA_MACSEC_EXT_E_164_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x36C00000,UNIT_PCA_MACSEC_EXT_I_164_1 ,16  * _1K )
    ,UNIT_INFO_MAC(0x32600000,UNIT_PCA_MACSEC_EXT_E_163_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x32A00000,UNIT_PCA_MACSEC_EXT_I_163_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x32800000,UNIT_PCA_MACSEC_EXT_E_164_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x32C00000,UNIT_PCA_MACSEC_EXT_I_164_2 ,16  * _1K )
    ,UNIT_INFO_MAC(0x33600000,UNIT_PCA_MACSEC_EXT_E_163_3 ,16  * _1K )
    ,UNIT_INFO_MAC(0x33A00000,UNIT_PCA_MACSEC_EXT_I_163_3 ,16  * _1K )
    ,UNIT_INFO_MAC(0x33800000,UNIT_PCA_MACSEC_EXT_E_164_3 ,16  * _1K )
    ,UNIT_INFO_MAC(0x33C00000,UNIT_PCA_MACSEC_EXT_I_164_3 ,16  * _1K )
    ,UNIT_INFO_MAC(0x36200000,UNIT_PCA_MACSEC_EIP_163_I_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x34D00000,UNIT_PCA_MACSEC_EIP_163_E_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x36600000,UNIT_PCA_MACSEC_EIP_163_I_1 ,1  * _1M )
    ,UNIT_INFO_MAC(0x35500000,UNIT_PCA_MACSEC_EIP_163_E_1 ,1  * _1M )
    ,UNIT_INFO_MAC(0x36A00000,UNIT_PCA_MACSEC_EIP_164_I_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x35A00000,UNIT_PCA_MACSEC_EIP_164_E_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x36E00000,UNIT_PCA_MACSEC_EIP_164_I_1 ,1  * _1M )
    ,UNIT_INFO_MAC(0x35E00000,UNIT_PCA_MACSEC_EIP_164_E_1 ,1  * _1M )

    /* DFX */
    ,UNIT_INFO_MAC(0x0B800000, UNIT_DFX_SERVER       ,1  * _1M)
    /* TAI 0,1 */
    ,UNIT_INFO_MAC(0x0BC00000,UNIT_TAI               ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0BE00000,UNIT_TAI_1             ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0C800000,UNIT_TTI               ,4   * _1M)
    ,UNIT_INFO_MAC(0x0CC00000,UNIT_TTI_TAI_0         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0CE00000,UNIT_TTI_TAI_1         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x05800000,UNIT_TXQS_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x05C00000,UNIT_TXQS_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x07800000,UNIT_TXQS1_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x07C00000,UNIT_TXQS1_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0D800000,UNIT_PHA_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0DA00000,UNIT_PHA_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0F800000,UNIT_EPCL_HA_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0FA00000,UNIT_EPCL_HA_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1AC00000,UNIT_CPU_PORT_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1AE00000,UNIT_CPU_PORT_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1BC00000,UNIT_400G0_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1BE00000,UNIT_400G0_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1CC00000,UNIT_400G1_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1CE00000,UNIT_400G1_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1DC00000,UNIT_400G2_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1DE00000,UNIT_400G2_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1EC00000,UNIT_400G3_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x1EE00000,UNIT_400G3_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x25C00000,UNIT_USX0_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x25E00000,UNIT_USX0_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x27C00000,UNIT_USX1_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x27E00000,UNIT_USX1_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x32E00000,UNIT_PCA2_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x32F00000,UNIT_PCA2_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x33E00000,UNIT_PCA3_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x33F00000,UNIT_PCA3_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x34E00000,UNIT_PCA0_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x34F00000,UNIT_PCA0_TAI1_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x35600000,UNIT_PCA1_TAI0_E         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x35700000,UNIT_PCA1_TAI1_E         ,64  *     _1K)



    /* Sip 6.10 PPU */
    ,UNIT_INFO_MAC(0x15400000,UNIT_PPU               ,1   * _1M)

    ,UNIT_INFO_MAC(0x0D000000,UNIT_PHA               ,8   * _1M)
    ,UNIT_INFO_MAC(0x0DC00000,UNIT_EPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x0F600000,UNIT_EOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x0E400000,UNIT_EPCL              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x0E600000,UNIT_PREQ              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x0E000000,UNIT_ERMRK             ,4   * _1M)
    ,UNIT_INFO_MAC(0x0F000000,UNIT_EREP              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x0F200000,UNIT_BMA               ,1   * _1M)
    ,UNIT_INFO_MAC(0x0F400000,UNIT_HBU               ,64   *    _1K)
    ,UNIT_INFO_MAC(0x0E800000,UNIT_HA                ,6   * _1M)

    ,UNIT_INFO_MAC(0x10800000,UNIT_TCAM              ,8   * _1M)
    ,UNIT_INFO_MAC(0x12400000,UNIT_EM                ,4   * _1M)
    ,UNIT_INFO_MAC(0x12800000,UNIT_FDB               ,64  *     _1K)
    ,UNIT_INFO_MAC(0x12C00000,UNIT_SHM               ,64  *     _1K)

    ,UNIT_INFO_MAC(0x13000000,UNIT_IPCL              ,512 *     _1K)

    ,UNIT_INFO_MAC(0x13400000,UNIT_CNC               ,256 *     _1K)
    ,UNIT_INFO_MAC(0x13800000,UNIT_CNC_1             ,256 *     _1K)
    ,UNIT_INFO_MAC(0x13C00000,UNIT_CNC_2             ,256 *     _1K)
    ,UNIT_INFO_MAC(0x14000000,UNIT_CNC_3             ,256 *     _1K)

    ,UNIT_INFO_MAC(0x14800000,UNIT_EGF_QAG           ,1   * _1M)
    ,UNIT_INFO_MAC(0x14C00000,UNIT_EGF_SHT           ,2   * _1M)
    ,UNIT_INFO_MAC(0x15000000,UNIT_EGF_EFT           ,64  *     _1K)

    ,UNIT_INFO_MAC(0x15800000,UNIT_IPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x15C00000,UNIT_IPLR1             ,4   * _1M)

    ,UNIT_INFO_MAC(0x16000000,UNIT_EQ                ,16  * _1M)

    ,UNIT_INFO_MAC(0x17000000,UNIT_IOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x17400000,UNIT_MLL               ,2   * _1M)

    ,UNIT_INFO_MAC(0x17800000,UNIT_IPVX              ,8   * _1M)
    ,UNIT_INFO_MAC(0x18000000,UNIT_LPM               ,16  * _1M)
    ,UNIT_INFO_MAC(0x19000000,UNIT_L2I               ,8   * _1M)

    ,UNIT_INFO_MAC(0x1A000000,UNIT_CPU_PORT_LED      ,2 * _1K)
    ,UNIT_INFO_MAC(0x1B000000,UNIT_LED_0             ,4 * _1K)
    ,UNIT_INFO_MAC(0x1C000000,UNIT_LED_1             ,4 * _1K)
    ,UNIT_INFO_MAC(0x1D000000,UNIT_LED_2             ,4 * _1K)
    ,UNIT_INFO_MAC(0x1E000000,UNIT_LED_3             ,4 * _1K)
    ,UNIT_INFO_MAC(0x24000000,UNIT_USX_LED_0         ,4 * _1K)
    ,UNIT_INFO_MAC(0x26000000,UNIT_USX_LED_1         ,4 * _1K)

    /*NOTE: MG0 is part of the CnM : 0x3D000000 size 1M */
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG    /*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_0_1/*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 2*MG_SIZE ,UNIT_MG_0_2/*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 3*MG_SIZE ,UNIT_MG_0_3/*CNM*/, MG_SIZE)

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
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_3_BASE_ADDR,UNIT_MAC_400G_3              ,64   *     _1K)

    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_0_BASE_ADDR,UNIT_PCS_400G_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_1_BASE_ADDR,UNIT_PCS_400G_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_2_BASE_ADDR,UNIT_PCS_400G_2              ,64   *     _1K)
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_3_BASE_ADDR,UNIT_PCS_400G_3              ,64   *     _1K)

    ,UNIT_INFO_MAC(CPU_MAC_MTIP_MAC_BASE_ADDR    ,UNIT_MAC_CPU              ,16   *     _1K)
    ,UNIT_INFO_MAC(CPU_MAC_MTIP_MAC_PCS_BASE_ADDR,UNIT_PCS_CPU              ,16   *     _1K)

    ,UNIT_INFO_MAC(0x2E800000     ,UNIT_SERDES_0_0         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x2EC00000     ,UNIT_SERDES_0_1         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x2F000000     ,UNIT_SERDES_0_2         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x2F400000     ,UNIT_SERDES_0_3         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x2F800000     ,UNIT_SERDES_0_4         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x2FC00000     ,UNIT_SERDES_0_5         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x30000000     ,UNIT_SERDES_0_6         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x30400000     ,UNIT_SERDES_0_7         ,1   * _1M     )     /* 112G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0x1A400000     ,UNIT_SERDES_1           ,1   * _1M     )     /* 28G Single SD Wrapper */

    /*MIF */
    ,UNIT_INFO_MAC(MIF_0_BASE_ADDR, UNIT_MIF_0              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_1_BASE_ADDR, UNIT_MIF_1              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_2_BASE_ADDR, UNIT_MIF_2              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_3_BASE_ADDR, UNIT_MIF_3              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_4_BASE_ADDR, UNIT_MIF_4              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_5_BASE_ADDR, UNIT_MIF_5              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_6_BASE_ADDR, UNIT_MIF_6              ,16   *     _1K)

    /*ANP */
    ,UNIT_INFO_MAC(ANP_0_BASE_ADDR, UNIT_ANP_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_1_BASE_ADDR, UNIT_ANP_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_2_BASE_ADDR, UNIT_ANP_2              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_3_BASE_ADDR, UNIT_ANP_3              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_4_BASE_ADDR, UNIT_ANP_4              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_5_BASE_ADDR, UNIT_ANP_5              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_6_BASE_ADDR, UNIT_ANP_6              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_7_BASE_ADDR, UNIT_ANP_7              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_8_BASE_ADDR, UNIT_ANP_8              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_9_BASE_ADDR, UNIT_ANP_9              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_10_BASE_ADDR, UNIT_ANP_10            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_11_BASE_ADDR, UNIT_ANP_11            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_12_BASE_ADDR, UNIT_ANP_12            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_13_BASE_ADDR, UNIT_ANP_13            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_14_BASE_ADDR, UNIT_ANP_14            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_15_BASE_ADDR, UNIT_ANP_15            ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_16_BASE_ADDR, UNIT_ANP_16            ,64   *     _1K)

    /* USX MAC / PCS */
    ,UNIT_INFO_MAC(MTIP_USX_0_MAC_0_BASE_ADDR,  UNIT_USX_0_MAC_0              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_1_MAC_0_BASE_ADDR,  UNIT_USX_1_MAC_0              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_2_MAC_0_BASE_ADDR,  UNIT_USX_2_MAC_0              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_0_MAC_1_BASE_ADDR,  UNIT_USX_0_MAC_1              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_1_MAC_1_BASE_ADDR,  UNIT_USX_1_MAC_1              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_2_MAC_1_BASE_ADDR,  UNIT_USX_2_MAC_1              ,64 * _1K)

    ,UNIT_INFO_MAC(MTIP_UNIT_USX_0_PCS_0_BASE_ADDR,  UNIT_USX_0_PCS_0              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_1_PCS_0_BASE_ADDR,  UNIT_USX_1_PCS_0              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_2_PCS_0_BASE_ADDR,  UNIT_USX_2_PCS_0              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_0_PCS_1_BASE_ADDR,  UNIT_USX_0_PCS_1              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_1_PCS_1_BASE_ADDR,  UNIT_USX_1_PCS_1              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_2_PCS_1_BASE_ADDR,  UNIT_USX_2_PCS_1              ,32 * _1K)

    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

/* info about the 50/100/200/400G MACs */
MAC_NUM_INFO_STC hawk100GPortsArr[] = {
    /*global*/      /*mac*/   /* channelInfo */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {0  ,           {0, 0} ,    {0, 0}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{1  ,           {0, 1} ,    {0, 1}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{2  ,           {0, 2} ,    {0, 2}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{6  ,           {0, 3} ,    {0, 6}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{10 ,           {0, 4} ,    {0,10}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{14 ,           {0, 5} ,    {0,14}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{18 ,           {0, 6} ,    {0,18}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{22 ,           {0, 7} ,    {0,22}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }

    ,{26 ,           {1, 0} ,    {1, 0}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{27 ,           {1, 1} ,    {1, 1}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{28 ,           {1, 2} ,    {1, 2}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{32 ,           {1, 3} ,    {1, 6}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{36 ,           {1, 4} ,    {1,10}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{40 ,           {1, 5} ,    {1,14}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{44 ,           {1, 6} ,    {1,18}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{48 ,           {1, 7} ,    {1,22}       ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }

    ,{52 ,           {2, 0} ,    {2, 0}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{53 ,           {2, 1} ,    {2, 1}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{54 ,           {2, 2} ,    {2, 2}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{58 ,           {2, 3} ,    {2, 6}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{62 ,           {2, 4} ,    {2,10}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{66 ,           {2, 5} ,    {2,14}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{70 ,           {2, 6} ,    {2,18}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{74 ,           {2, 7} ,    {2,22}       ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }

    ,{78 ,           {3, 0} ,    {3, 0}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{79 ,           {3, 1} ,    {3, 1}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{80 ,           {3, 2} ,    {3, 2}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{84 ,           {3, 3} ,    {3, 6}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{88 ,           {3, 4} ,    {3,10}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{92 ,           {3, 5} ,    {3,14}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{96 ,           {3, 6} ,    {3,18}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }
    ,{100,           {3, 7} ,    {3,22}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* info about the 1/2.5/5G MACs */
MAC_NUM_INFO_STC hawkUsxPortsArr[] = {
    /*global*/      /*mac*/   /* tsu */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {2  ,           {0, 0} ,    {0, 2}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{4  ,           {0, 1} ,    {0, 4}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{3  ,           {0, 2} ,    {0, 3}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{5  ,           {0, 3} ,    {0, 5}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{6  ,           {0, 4} ,    {0, 6}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{8  ,           {0, 5} ,    {0, 8}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{7  ,           {0, 6} ,    {0, 7}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{9  ,           {0, 7} ,    {0, 9}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }

    ,{10 ,           {1, 0} ,    {0, 10}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{12 ,           {1, 1} ,    {0, 12}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{11 ,           {1, 2} ,    {0, 11}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{13 ,           {1, 3} ,    {0, 13}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{14 ,           {1, 4} ,    {0, 14}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{16 ,           {1, 5} ,    {0, 16}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{15 ,           {1, 6} ,    {0, 15}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{17 ,           {1, 7} ,    {0, 17}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }

    ,{18 ,           {2, 0} ,    {0, 18}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{20 ,           {2, 1} ,    {0, 20}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{19 ,           {2, 2} ,    {0, 19}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{21 ,           {2, 3} ,    {0, 21}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{22 ,           {2, 4} ,    {0, 22}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{24 ,           {2, 5} ,    {0, 24}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{23 ,           {2, 6} ,    {0, 23}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{25 ,           {2, 7} ,    {0, 25}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }


    ,{28 ,           {3, 0} ,    {1, 2}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{30 ,           {3, 1} ,    {1, 4}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{29 ,           {3, 2} ,    {1, 3}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{31 ,           {3, 3} ,    {1, 5}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{32 ,           {3, 4} ,    {1, 6}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{34 ,           {3, 5} ,    {1, 8}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{33 ,           {3, 6} ,    {1, 7}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{35 ,           {3, 7} ,    {1, 9}       ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }

    ,{36 ,           {4, 0} ,    {1, 10}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{38 ,           {4, 1} ,    {1, 12}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{37 ,           {4, 2} ,    {1, 11}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{39 ,           {4, 3} ,    {1, 13}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{40 ,           {4, 4} ,    {1, 14}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{42 ,           {4, 5} ,    {1, 16}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{41 ,           {4, 6} ,    {1, 15}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{43 ,           {4, 7} ,    {1, 17}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }

    ,{44 ,           {5, 0} ,    {1, 18}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{46 ,           {5, 1} ,    {1, 20}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{45 ,           {5, 2} ,    {1, 19}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{47 ,           {5, 3} ,    {1, 21}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{48 ,           {5, 4} ,    {1, 22}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{50 ,           {5, 5} ,    {1, 24}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{49 ,           {5, 6} ,    {1, 23}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }
    ,{51 ,           {5, 7} ,    {1, 25}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

#define CPU_PORT_DP_0     0
#define CPU_PORT_DP_1     1

#define CPU_PORT_DP_1_GLOBAL_NUM    105

/* info about the CPU MACs */
/* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
static MAC_NUM_INFO_STC hawkCpuPortsArr[] = {
    /*global*/       /*mac*/       /* DP unit */               /*sip6_MTI_EXTERNAL_representativePortIndex*/
    /* {104,           {0, 0} ,    {CPU_PORT_DP_0, 26}      , 0    }*/
    {CPU_PORT_DP_1_GLOBAL_NUM,           {0, 0} ,    {CPU_PORT_DP_1, 26}      , 0    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* number of units in the device */
#define HAWK_NUM_UNITS sizeof(hawk_units)/sizeof(hawk_units[0])

static SMEM_GEN_UNIT_INFO_STC SORTED___hawk_units[HAWK_NUM_UNITS] =
{
    /* sorted and build during smemHawkInit(...) from hawk_units[] */
    {SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};


/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from falcon_units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC hawk_duplicatedUnits[] =
{
    /* those explicitly listed here need unit allocation as are not duplicated within each pipe */
    {STR(UNIT_RX_DMA)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_RX_DMA_1)},
        {STR(UNIT_RX_DMA_2)},
        {STR(UNIT_RX_DMA_3)},

    {STR(UNIT_TX_FIFO) ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TX_FIFO_1)},
        {STR(UNIT_TX_FIFO_2)},
        {STR(UNIT_TX_FIFO_3)},

    {STR(UNIT_TX_DMA)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TX_DMA_1)},
        {STR(UNIT_TX_DMA_2)},
        {STR(UNIT_TX_DMA_3)},

    {STR(UNIT_TXQ_PDS0)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TXQ_PDS1)},
        {STR(UNIT_TXQ_PDS2)},
        {STR(UNIT_TXQ_PDS3)},

    {STR(UNIT_TXQ_SDQ0)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TXQ_SDQ1)},
        {STR(UNIT_TXQ_SDQ2)},
        {STR(UNIT_TXQ_SDQ3)},

   {STR(UNIT_TXQ_QFC0)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_TXQ_QFC1)},
        {STR(UNIT_TXQ_QFC2)},
        {STR(UNIT_TXQ_QFC3)},

   {STR(UNIT_MG)        ,3}, /* 3 duplication of this unit */
        {STR(UNIT_MG_0_1)},
        {STR(UNIT_MG_0_2)},
        {STR(UNIT_MG_0_3)},


    {STR(UNIT_PB_WRITE_ARBITER_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PB_WRITE_ARBITER_1)},
        {STR(UNIT_PB_WRITE_ARBITER_2)},
        {STR(UNIT_PB_WRITE_ARBITER_3)},

    {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_1)},
        {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_2)},
        {STR(UNIT_PB_GPC_GRP_PACKET_WRITE_3)},

    {STR(UNIT_PB_GPC_GRP_CELL_READ_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PB_GPC_GRP_CELL_READ_1)},
        {STR(UNIT_PB_GPC_GRP_CELL_READ_2)},
        {STR(UNIT_PB_GPC_GRP_CELL_READ_3)},

    {STR(UNIT_PB_GPC_GRP_PACKET_READ_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PB_GPC_GRP_PACKET_READ_1)},
        {STR(UNIT_PB_GPC_GRP_PACKET_READ_2)},
        {STR(UNIT_PB_GPC_GRP_PACKET_READ_3)},

    {STR(UNIT_PB_NEXT_POINTER_MEMO_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PB_NEXT_POINTER_MEMO_1)},

    {STR(UNIT_PB_SHARED_MEMO_BUF_0)  ,2},  /* 2 more per device */
        {STR(UNIT_PB_SHARED_MEMO_BUF_1)},
        {STR(UNIT_PB_SHARED_MEMO_BUF_2)},

    {STR(UNIT_MAC_400G_0)  ,3},  /* 3 more per device */
        {STR(UNIT_MAC_400G_1)},
        {STR(UNIT_MAC_400G_2)},
        {STR(UNIT_MAC_400G_3)},

    {STR(UNIT_PCS_400G_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCS_400G_1)},
        {STR(UNIT_PCS_400G_2)},
        {STR(UNIT_PCS_400G_3)},

    {STR(UNIT_SERDES_0_0)  ,7},  /* 3 more per device */
        {STR(UNIT_SERDES_0_1)},
        {STR(UNIT_SERDES_0_2)},
        {STR(UNIT_SERDES_0_3)},
        {STR(UNIT_SERDES_0_4)},
        {STR(UNIT_SERDES_0_5)},
        {STR(UNIT_SERDES_0_6)},
        {STR(UNIT_SERDES_0_7)},

    {STR(UNIT_CNC) ,2}, /* 2 duplication of this unit (UNIT_CNC_1 will get duplication by 'legacy device')*/
        {STR(UNIT_CNC_2)},
        {STR(UNIT_CNC_3)},

    {STR(UNIT_PCA_LMU_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_LMU_1)},
        {STR(UNIT_PCA_LMU_2)},
        {STR(UNIT_PCA_LMU_3)},

    {STR(UNIT_PCA_CTSU_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_CTSU_1)},
        {STR(UNIT_PCA_CTSU_2)},
        {STR(UNIT_PCA_CTSU_3)},

    {STR(UNIT_PCA_BRG_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_BRG_1)},
        {STR(UNIT_PCA_BRG_2)},
        {STR(UNIT_PCA_BRG_3)},

    {STR(UNIT_PCA_SFF_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_SFF_1)},
        {STR(UNIT_PCA_SFF_2)},
        {STR(UNIT_PCA_SFF_3)},

    {STR(UNIT_PCA_PZ_ARBITER_I_0)  ,7},  /* 7 more per device */
        {STR(UNIT_PCA_PZ_ARBITER_I_1)},
        {STR(UNIT_PCA_PZ_ARBITER_I_2)},
        {STR(UNIT_PCA_PZ_ARBITER_I_3)},
        {STR(UNIT_PCA_PZ_ARBITER_E_0)},
        {STR(UNIT_PCA_PZ_ARBITER_E_1)},
        {STR(UNIT_PCA_PZ_ARBITER_E_2)},
        {STR(UNIT_PCA_PZ_ARBITER_E_3)},

    {STR(UNIT_PCA_MACSEC_EXT_I_163_0)  ,15},  /* 15 more per device */
        {STR(UNIT_PCA_MACSEC_EXT_I_163_1)},
        {STR(UNIT_PCA_MACSEC_EXT_I_163_2)},
        {STR(UNIT_PCA_MACSEC_EXT_I_163_3)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_1)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_2)},
        {STR(UNIT_PCA_MACSEC_EXT_E_163_3)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_1)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_2)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_3)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_1)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_2)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_3)},

    {STR(UNIT_PCA_MACSEC_EIP_163_E_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_163_E_1) },

    {STR(UNIT_PCA_MACSEC_EIP_164_E_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_164_E_1) },

    {STR(UNIT_PCA_MACSEC_EIP_163_I_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_163_I_1) },

    {STR(UNIT_PCA_MACSEC_EIP_164_I_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_MACSEC_EIP_164_I_1) },


        {STR(UNIT_LED_0)  ,6},  /* 3 more per device */
            {STR(UNIT_LED_1)},
            {STR(UNIT_LED_2)},
            {STR(UNIT_LED_3)},
            {STR(UNIT_CPU_PORT_LED)},
            {STR(UNIT_USX_LED_0)},
            {STR(UNIT_USX_LED_1)},

        {STR(UNIT_MIF_0)  ,6},  /* 6 more per device */
            {STR(UNIT_MIF_1)},
            {STR(UNIT_MIF_2)},
            {STR(UNIT_MIF_3)},
            {STR(UNIT_MIF_4)},
            {STR(UNIT_MIF_5)},
            {STR(UNIT_MIF_6)},

        {STR(UNIT_ANP_0)  ,16},  /* 16 more per device */
            {STR(UNIT_ANP_1)},
            {STR(UNIT_ANP_2)},
            {STR(UNIT_ANP_3)},
            {STR(UNIT_ANP_4)},
            {STR(UNIT_ANP_5)},
            {STR(UNIT_ANP_6)},
            {STR(UNIT_ANP_7)},
            {STR(UNIT_ANP_8)},
            {STR(UNIT_ANP_9)},
            {STR(UNIT_ANP_10)},
            {STR(UNIT_ANP_11)},
            {STR(UNIT_ANP_12)},
            {STR(UNIT_ANP_13)},
            {STR(UNIT_ANP_14)},
            {STR(UNIT_ANP_15)},
            {STR(UNIT_ANP_16)},

        {STR(UNIT_USX_0_MAC_0),5},
            {STR(UNIT_USX_1_MAC_0)},
            {STR(UNIT_USX_2_MAC_0)},
            {STR(UNIT_USX_0_MAC_1)},
            {STR(UNIT_USX_1_MAC_1)},
            {STR(UNIT_USX_2_MAC_1)},

        {STR(UNIT_USX_0_PCS_0),5},
            {STR(UNIT_USX_1_PCS_0)},
            {STR(UNIT_USX_2_PCS_0)},
            {STR(UNIT_USX_0_PCS_1)},
            {STR(UNIT_USX_1_PCS_1)},
            {STR(UNIT_USX_2_PCS_1)},

     {STR(UNIT_TAI)  ,33},  /* 33 more per device */
        {STR(UNIT_TAI_1)            },
        {STR(UNIT_TTI_TAI_0)        },
        {STR(UNIT_TTI_TAI_1)        },
        {STR(UNIT_TXQS_TAI0_E)      },
        {STR(UNIT_TXQS_TAI1_E)      },
        {STR(UNIT_TXQS1_TAI0_E)     },
        {STR(UNIT_TXQS1_TAI1_E)     },
        {STR(UNIT_PHA_TAI0_E)       },
        {STR(UNIT_PHA_TAI1_E)       },
        {STR(UNIT_EPCL_HA_TAI0_E)   },
        {STR(UNIT_EPCL_HA_TAI1_E)   },
        {STR(UNIT_CPU_PORT_TAI0_E)  },
        {STR(UNIT_CPU_PORT_TAI1_E)  },
        {STR(UNIT_400G0_TAI0_E)     },
        {STR(UNIT_400G0_TAI1_E)     },
        {STR(UNIT_400G1_TAI0_E)     },
        {STR(UNIT_400G1_TAI1_E)     },
        {STR(UNIT_400G2_TAI0_E)     },
        {STR(UNIT_400G2_TAI1_E)     },
        {STR(UNIT_400G3_TAI0_E)     },
        {STR(UNIT_400G3_TAI1_E)     },
        {STR(UNIT_USX0_TAI0_E)      },
        {STR(UNIT_USX0_TAI1_E)      },
        {STR(UNIT_USX1_TAI0_E)      },
        {STR(UNIT_USX1_TAI1_E)      },
        {STR(UNIT_PCA2_TAI0_E)      },
        {STR(UNIT_PCA2_TAI1_E)      },
        {STR(UNIT_PCA3_TAI0_E)      },
        {STR(UNIT_PCA3_TAI1_E)      },
        {STR(UNIT_PCA0_TAI0_E)      },
        {STR(UNIT_PCA0_TAI1_E)      },
        {STR(UNIT_PCA1_TAI0_E)      },
        {STR(UNIT_PCA1_TAI1_E)      },

        {NULL,0} /* must be last */
};

/* the DP that hold MG0_0*/
#define CNM0_DP_FOR_MG_0   0
/* the DP that hold MG0_1*/
#define CNM0_DP_FOR_MG_1   1
/* the DP that hold MG0_2*/
#define CNM0_DP_FOR_MG_2   2
/* the DP that hold MG0_3*/
#define CNM0_DP_FOR_MG_3   3

#define FIRST_CPU_SDMA    ((4*NUM_PORTS_PER_DP_UNIT) + 2/*CPU ports */) /* DP ports */
/* DMA : special ports mapping {global,local,DP}    */
static SPECIAL_PORT_MAPPING_CNS hawk_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
/*106*/ {FIRST_CPU_SDMA+0/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_0/*DP[0]*/}
/*107*/,{FIRST_CPU_SDMA+1/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_1/*DP[1]*/}
/*108*/,{FIRST_CPU_SDMA+2/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_2/*DP[2]*/}
/*109*/,{FIRST_CPU_SDMA+3/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,CNM0_DP_FOR_MG_3/*DP[3]*/}

/*105*/,{FIRST_CPU_SDMA-1/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,1/*DP[]*/}
/*104*/,{FIRST_CPU_SDMA-2/*global DMA port*/,NUM_PORTS_PER_DP_UNIT/*local DMA port*/,0/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* GOP : special ports mapping {global,local,pipe}*/
static SPECIAL_PORT_MAPPING_CNS hawk_GOP_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
/*105*/ {FIRST_CPU_SDMA-1/*global GOP port*/,FIRST_CPU_SDMA-1/*local GOP port*/,0/*pipe*/} /*DP[1] port 26*/
/*104*/,{FIRST_CPU_SDMA-2/*global GOP port*/,FIRST_CPU_SDMA-2/*local GOP port*/,0/*pipe*/} /*DP[0] port 26*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* multi-CPU SDMA MG : special ports mapping {global,NA,mgUnitIndex}    */
static SPECIAL_PORT_MAPPING_CNS hawk_cpuPortSdma_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --NA;OUT       --mgUnitIndex;OUT */
/*106*/ {FIRST_CPU_SDMA+0/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,0/*mgUnitIndex[]*/}
/*107*/,{FIRST_CPU_SDMA+1/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,1/*mgUnitIndex[]*/}
/*108*/,{FIRST_CPU_SDMA+2/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,2/*mgUnitIndex[]*/}
/*109*/,{FIRST_CPU_SDMA+3/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,3/*mgUnitIndex[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/**
* @internal smemHawkGetMgUnitIndexFromAddress function
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
static GT_U32  smemHawkGetMgUnitIndexFromAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  regAddress
)
{
    if(regAddress >= MG_0_0_OFFSET_CNS &&
       regAddress < (MG_0_0_OFFSET_CNS + (MG_SIZE*HAWK_NUM_MG_UNITS)))
    {
        return (regAddress - MG_0_0_OFFSET_CNS)/MG_SIZE;
    }

    return SMAIN_NOT_VALID_CNS;
}

/**
* @internal smemHawkGopPortByAddrGet function
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
static GT_U32 smemHawkGopPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32 unitIndex;
    GT_U32 ii;
    GT_U32 isCpuPort    = 0;
    GT_U32 localPortInUnit = 0;
    GT_U32 unitBase;
    GT_U32 unitSize;
    GT_U32 startTxCounters;
    GT_U32 portDivider;
    volatile GT_U32 is100G_400G_Unit = 0;

    for(ii = 0; ii < SIM_MAX_ANP_UNITS; ii++)
    {
        if(address >=  devObjPtr->memUnitBaseAddrInfo.anp[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.anp[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.anp[ii] + ANP_OFFSET;
            if(address >= unitBase + 0x00000100 &&
               address < (unitBase + 0x00000104 + NUM_PORTS_PER_UNIT*0x2a4))
            {
                /* 0x00000094 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Cause */
                /* 0x00000098 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Mask */
                localPortInUnit = (address - (unitBase + 0x00000100)) / 0x2a4;
                break;
            }
        }
    }

    unitIndex = ii;
    if((ii >= 1) && (ii <= 4))
    {
        /* matched as address in ANPx8(MTI400)  */
        switch(localPortInUnit)
        {
            case 0:
            case 1:
            case 2:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + localPortInUnit;
            case 3:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + 6;
            case 4:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + 10;
            case 5:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + 14;
            case 6:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + 18;
            case 7:
                return (ii - 1) * NUM_PORTS_PER_DP_UNIT + 22;
            default:
                skernelFatalError("smemHawkGopPortByAddrGet : unknown port for address [0x%8.8x] as 'Gop unit' \n",
                address);
                return 0;
        }
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

    for(ii = sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr ;
        ii < sip6_MTI_EXTERNAL_representativePortIndex_base_hawk___last;
        ii++)
    {
        is100G_400G_Unit = (ii < sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr) ? 1 : 0;
        if(address >=  devObjPtr->memUnitBaseAddrInfo.macWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + _1M) )
        {
            if(is100G_400G_Unit)
            {
                unitSize = MAC_MIB_UNIT_SIZE;
                startTxCounters = START_MIB_TX_PER_PORT;
                /* support the dual MIB counters per 'port' (EMAC,PMAC) */
                portDivider = 2;
            }
            else
            {
                unitSize = USX_MAC_MIB_UNIT_SIZE;/* also for 'CPU port' */
                startTxCounters = USX_START_MIB_TX_PER_PORT;
                portDivider = 1;
            }

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

            if(is100G_400G_Unit)
            {
                /* MTI 400G mac */
                unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_200G_400G_OFFSET;

                if(address >= unitBase &&
                   address < (unitBase + (PORT0_100G_OFFSET-MAC_200G_400G_OFFSET)))
                {
                    /* 200G/400G port */
                    localPortInUnit = 4 * ((address - unitBase) / MAC_STEP_PORT_OFFSET);
                    break;
                }
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + ((ii < sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr) ? PORT0_100G_OFFSET : USX_PORT0_100G_OFFSET);
            if(address >= unitBase &&
               address < (unitBase + (8*MAC_STEP_PORT_OFFSET)))
            {
                /* 100G/50G port */
                localPortInUnit = (address - unitBase) / MAC_STEP_PORT_OFFSET;
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_EXT_BASE_OFFSET;

            if(is100G_400G_Unit)
            {
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
            }
            else
            {
                /* usx */
                if(address >= unitBase + 0x0000002c &&
                   address < (unitBase + 0x00000050 + (8 * 0x28)))
                {
                    /* 0x00000038 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
                    /* 0x0000003c : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
                    localPortInUnit = (address - (unitBase + 0x00000038)) / 0x28;
                    break;
                }

                if(address >= unitBase + 0x00000170 &&
                   address < (unitBase + 0x0000017c + (8 * 0x10)))
                {
                    localPortInUnit = (address - (unitBase + 0x0000017c)) / 0x10;
                    break;
                }

                if(address >= unitBase + 0x000001f0 &&
                   address < (unitBase + 0x00000200 + (8/4 * 0x14)))
                {
                    localPortInUnit = ((address - (unitBase + 0x0000017c)) / 0x14) * 4;
                    break;
                }

                if(address >= unitBase + 0x00000218 &&
                   address < (unitBase + 0x0000021c + (8/4 * 0x14)))
                {
                    localPortInUnit = ((address - (unitBase + 0x00000218)) / 0x14) * 4;
                    break;
                }
            }

            break; /* common to the unit */
        }

        if(is100G_400G_Unit)
        {
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
        else
        {
            if(address >=  devObjPtr->memUnitBaseAddrInfo.macPcs[ii] &&
               address <  (devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + _1M) )
            {
                unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + USX_PCS_PORTS_OFFSET;
                if(address >= unitBase &&
                   address < (unitBase + (8*USX_PCS_STEP_PORT_OFFSET)))
                {
                    /* USX ports */
                    localPortInUnit = (address - unitBase) / USX_PCS_STEP_PORT_OFFSET;
                    break;
                }

                unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + USX_PCS_LPCS_OFFSET;
                if(address >= unitBase &&
                   address < (unitBase + (8*USX_LPCS_STEP_PORT_OFFSET)))
                {
                    /* USX ports */
                    localPortInUnit = (address - unitBase) / USX_LPCS_STEP_PORT_OFFSET;
                    break;
                }

                break;
            }
        }


        if(address >=  devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[ii] + _1M) )
        {
            /* NOTE: (if matched) will always be matched on ii=0 and never on ii=1 */
            isCpuPort = 1;
            break;
        }

        if(address >=  devObjPtr->memUnitBaseAddrInfo.cpuMacPcs[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.cpuMacPcs[ii] + _1M) )
        {
            /* NOTE: (if matched) will always be matched on ii=0 and never on ii=1 */
            isCpuPort = 1;
            break;
        }
    }

    if(ii == sip6_MTI_EXTERNAL_representativePortIndex_base_hawk___last)
    {
        skernelFatalError("smemHawkGopPortByAddrGet : unknown address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }

    unitIndex = ii;
    if(isCpuPort)
    {
        for(ii = 0 ; hawkCpuPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
        {
            if(hawkCpuPortsArr[ii].macInfo.unitIndex == unitIndex)
            {
                /* NOTE: will always be matched on ii=0 and never on ii=1 */
                return hawkCpuPortsArr[ii].globalMacNum;
            }
        }

        skernelFatalError("smemHawkGopPortByAddrGet : unknown cpu port for address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }
    else
    {
        if(is100G_400G_Unit)
        {
            for(ii = 0 ; hawk100GPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
            {
                if(hawk100GPortsArr[ii].macInfo.unitIndex   == unitIndex &&
                   hawk100GPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
                {
                    return hawk100GPortsArr[ii].globalMacNum;
                }
            }
        }
        else
        {
            unitIndex -= sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr;

            for(ii = 0 ; hawkUsxPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
            {
                if(hawkUsxPortsArr[ii].macInfo.unitIndex   == unitIndex &&
                   hawkUsxPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
                {
                    return hawkUsxPortsArr[ii].globalMacNum;
                }
            }
        }

        skernelFatalError("smemHawkGopPortByAddrGet : unknown port for address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }
}

/**
* @internal smemHawkMibPortByAddrGet function
* @endinternal
*
* @brief   get global MIB port id according to associated address (in MSM unit).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the MIB  space of the port
*
* @note the port id
*
*/
static GT_U32 smemHawkMibPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    return smemHawkGopPortByAddrGet(devObjPtr,address);
}

/*******************************************************************************
*   smemHawkPortInfoGet
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
*      GT_NOT_FOND - the port is not valid for the 'unitInfo'
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS smemHawkPortInfoGet
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

    /*special case*/
    if(unitType == SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E)
    {
        GT_U32  mifUnitId = portNum;/* !!! the parameter used as mifUnitId !!! */
        GT_U32  dpUnitIndex = 0;
        GT_U32  IN_indexInUnit = portInfoPtr->simplePortInfo.indexInUnit;/* !!! the parameter used as input !!! */
        GT_U32  OUT_indexInUnit=0;
        static  GT_U32  localPortIn400GMapArr[] = {0,1,2,6,10,14,18,22};
        GT_BIT  isPreemptiveChannel = 0;
        #define _400G_MIF_TO_CHANNEL                                            \
            OUT_indexInUnit     = localPortIn400GMapArr[IN_indexInUnit & 0x7];  \
            isPreemptiveChannel = IN_indexInUnit >> 3

        switch(mifUnitId)
        {
            case 0:/*400G_0*/dpUnitIndex = 0;_400G_MIF_TO_CHANNEL;break;
            case 1:/*400G_1*/dpUnitIndex = 1;_400G_MIF_TO_CHANNEL;break;
            case 2:/*400G_2*/dpUnitIndex = 2;_400G_MIF_TO_CHANNEL;break;
            case 3:/*400G_3*/dpUnitIndex = 3;_400G_MIF_TO_CHANNEL;break;
            case 4:/*USX0*/  dpUnitIndex = 0;OUT_indexInUnit = 2 + IN_indexInUnit;/*0..23*/break;
            case 5:/*USX1*/  dpUnitIndex = 1;OUT_indexInUnit = 2 + IN_indexInUnit;/*0..23*/break;
            case 6:/*CPU*/   dpUnitIndex = CPU_PORT_DP_1; OUT_indexInUnit= 26;   /*26*/break;
            default:
                skernelFatalError("smemHawkPortInfoGet : unknown mifUnitId [%d] \n",mifUnitId);
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
            jjMax = 3;
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
            sip6_MTI_bmpPorts = 0x00444447;/* 8 ports in the group : bits : 0,1,2,6,10,14,18,22 */
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
            sip6_MTI_bmpPorts = 0x000000ff;/* 8 consecutive ports in the group */
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_CPU_E:
            break;
        default:
            skernelFatalError("smemHawkPortInfoGet : unknown type [%d] \n",unitType);
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
                tmpMacInfoPtr =
                    (jj == 0) ? hawk100GPortsArr :
                    (jj == 1) ? hawkUsxPortsArr  :
                                hawkCpuPortsArr  ;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
                tmpMacInfoPtr = hawk100GPortsArr;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
                tmpMacInfoPtr = hawkUsxPortsArr;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_CPU_E:
                tmpMacInfoPtr = hawkCpuPortsArr;
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
                    /* Hawk use channel 0 in LMU for all channels */
                    portInfoPtr->simplePortInfo.indexInUnit = 0;
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
                    if(jj == 0)/*100G mac*/
                    {
                        GT_U32  is_mti_segmented = snetChtPortMacFieldGet(devObjPtr, portNum,
                            SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E);

                        /* can be 32 or 128 depend on 'segmented' */
                        portInfoPtr->simplePortInfo.unitIndex   =
                            is_mti_segmented ? SMEM_CHT_PORT_MTI_MIF_TYPE_128_E :
                                               SMEM_CHT_PORT_MTI_MIF_TYPE_32_E;
                    }
                    else /* CPU/USX*/
                    {
                        portInfoPtr->simplePortInfo.unitIndex   = SMEM_CHT_PORT_MTI_MIF_TYPE_8_E;
                    }
                    portInfoPtr->simplePortInfo.indexInUnit = 0;/* not used */
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
                    /*
                    0x1A200000 + 0x0a200000 + (a-5)*0x200000: where a (5-5) represents USX_MAC_INST_0
                    0x1A200000 + 0x0a200000 + 0x2000000 + (a-7)*0x200000: where a (7-8) represents USX_MAC_INST_1
                    0x1A200000 + 0x1200000 + (a-1)*0x1000000: where a (1-4) represents 400G_MAC_INST
                    0x1A200000 + 0x0a200000 + (a-5)*0x200000: where a (6-6) represents Dummy only place holder
                    */
                    if(jj == 2)
                    {
                        /*The CPU port*/
                        portInfoPtr->simplePortInfo.unitIndex   = 6;
                        portInfoPtr->simplePortInfo.indexInUnit = 0;
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = 0;
                    }
                    else
                    if (jj == 1)
                    {
                        /* 3 lower USX units connected to single MIF unit */
                        /* 3 upper USX units connected to single MIF unit */
                        portInfoPtr->simplePortInfo.unitIndex   = 4 + (tmpMacInfoPtr->macInfo.unitIndex/*0..5*/ / 3);/*4/5*/
                        portInfoPtr->simplePortInfo.indexInUnit = /*0..23*/
                            8*(tmpMacInfoPtr->macInfo.unitIndex % 3) +/*0/8/16 */
                            tmpMacInfoPtr->macInfo.indexInUnit;/*0..7*/
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = portInfoPtr->simplePortInfo.unitIndex;/*4/5*/
                    }
                    else
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
* @internal internalHawkUnitPipeOffsetGet function
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
static GT_U32 internalHawkUnitPipeOffsetGet
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

static SMEM_UNIT_NAME_AND_INDEX_STC hawkUnitNameAndIndexArr[HAWK_NUM_UNITS]=
{
    /* filled in runtime from hawk_units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};
/* the addresses of the units that the hawk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   hawkUsedUnitsAddressesArray[HAWK_NUM_UNITS]=
{
    {0,0}    /* filled in runtime from hawk_units[] */
};

/**
* @internal hawk_units_cellCompare function
* @endinternal
*
* @brief   Function compare two units base address of formula array
*
* @param[in] cell1Ptr                 - pointer to device formula cell array
* @param[in] cell2Ptr                 - pointer to device formula cell array
*                                       The return value of this function represents whether cell1Ptr is considered
*                                       less than, equal, or grater than cell2Ptr by returning, respectively,
*                                       a negative value, zero or a positive value.
*
* @note function's prototype must be defined return int to avoid warnings and be
*       consistent with qsort() function prototype
*
*/
static int  hawk_units_cellCompare
(
    const GT_VOID * cell1Ptr,
    const GT_VOID * cell2Ptr
)
{
    SMEM_GEN_UNIT_INFO_STC * cellFirstPtr = (SMEM_GEN_UNIT_INFO_STC *)cell1Ptr;
    SMEM_GEN_UNIT_INFO_STC * cellNextPtr  = (SMEM_GEN_UNIT_INFO_STC *)cell2Ptr;

    return (cellFirstPtr->base_addr - cellNextPtr->base_addr);
}

/* build once the sorted memory for the falcon .. for better memory search performance

    that use by :

    devObjPtr->devMemUnitNameAndIndexPtr      = hawkUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = hawkUsedUnitsAddressesArray;


*/
static void build_SORTED___hawk_units(void)
{
    GT_U32  numValidElem,ii;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &hawk_units[0];

    if(SORTED___hawk_units[0].base_addr != SMAIN_NOT_VALID_CNS)
    {
        /* already initialized */
        return;
    }

    memcpy(SORTED___hawk_units,hawk_units,sizeof(hawk_units));

    numValidElem = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        numValidElem++;
    }

    qsort(SORTED___hawk_units, numValidElem, sizeof(SMEM_GEN_UNIT_INFO_STC),
          hawk_units_cellCompare);

}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        hawkUsedUnitsAddressesArray - the addresses of the units that the Falcon uses
        hawkUnitNameAndIndexArr - name of unit and index in hawkUsedUnitsAddressesArray */
    GT_U32  ii;
    GT_U32  index;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &SORTED___hawk_units[0];

    /* build once the sorted memory for the falcon .. for better memory search performance */
    build_SORTED___hawk_units();

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        hawkUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
        hawkUsedUnitsAddressesArray[index].unitSizeInBytes = unitInfoPtr->size;
        hawkUnitNameAndIndexArr[index].unitNameIndex = index;
        hawkUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        index++;
    }

    if(index >= (sizeof(hawkUnitNameAndIndexArr) / sizeof(hawkUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    hawkUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    hawkUnitNameAndIndexArr[index].unitNameStr = NULL;
    hawkUsedUnitsAddressesArray[index].unitBaseAddr = SMAIN_NOT_VALID_CNS;
    hawkUsedUnitsAddressesArray[index].unitSizeInBytes = 0;

    devObjPtr->devMemUnitNameAndIndexPtr = hawkUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = hawkUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;/* no pipe offset */
    devObjPtr->support_memoryRanges = 1;

#if 0 /* check that the array is ascending ! (hawkUsedUnitsAddressesArray) */
    for(ii = 0 ; ii < (index+1) ; ii++)
    {
        printf("unitBaseAddr = [0x%8.8x] \n",
            hawkUsedUnitsAddressesArray[ii].unitBaseAddr);

        if(ii &&
            (hawkUsedUnitsAddressesArray[ii].unitBaseAddr <=
             hawkUsedUnitsAddressesArray[ii-1].unitBaseAddr))
        {
            printf("Error: at index[%d] prev index higher \n",ii);
            break;
        }
    }
#endif /*0*/
}

#define HAWK_PPG_OFFSET 0x80000
#define HAWK_PPN_OFFSET  0x4000
#define HAWK_PPN_NUM     4
#define HAWK_PPG_NUM     10
#define HAWK_TSU_CHANNELS_PER_GROUP  16
#define HAWK_TSU_OFFSET 0x4

/*ppn%n 0x00000000 + 0x80000 * m + 0x4000 * p: where p (0-8) represents PPN, where m (0-2) represents PPG*/
#define HAWK_PPN_REPLICATIONS HAWK_PPN_NUM,HAWK_PPG_OFFSET , HAWK_PPG_NUM,HAWK_PPN_OFFSET

#define HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS FORMULA_TWO_PARAMETERS(HAWK_PPN_NUM,HAWK_PPG_OFFSET , HAWK_PPG_NUM,HAWK_PPN_OFFSET)

/*ppg%g 0x00000000 + 0x80000 * g : where g (0-2) represents PPG*/
#define HAWK_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER FORMULA_SINGLE_PARAMETER(HAWK_PPN_NUM,HAWK_PPG_OFFSET)

#define HAWK_PPG_REPLICATION_ADDR(ppg)   \
    (HAWK_PPG_OFFSET * (ppg))

#define HAWK_PPN_REPLICATION_ADDR(ppg,ppn)   \
    (HAWK_PPG_REPLICATION_ADDR(ppg) + (HAWK_PPN_OFFSET * (ppn)))

#define HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,ppn) \
    /*PPN Internal Error Cause Register*/         \
    {0x00003040 + HAWK_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003044 + HAWK_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define HAWK_PPG_REPLICATIONS_ACTIVE_MEM(ppg) \
    /*PPG Interrupt Cause Register*/     \
    {0x0007FF00 + HAWK_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF04 + HAWK_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
    /*PPG internal error cause Register*/                                                                                              \
    {0x0007FF10 + HAWK_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF14 + HAWK_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
                                        \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,0), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,1), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,2), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,3), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,4), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,5), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,6), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,7), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,8), \
    HAWK_PPN_REPLICATIONS_ACTIVE_MEM(ppg,9)

#define HAWK_TSU_CHANNEL_OFFSET_MAC(group, channel)  \
     (HAWK_TSU_OFFSET * ((group * HAWK_TSU_CHANNELS_PER_GROUP) + channel))

#define HAWK_TSU_CHANNEL_ACTIVE_MEM(group, channel) \
    /* Channel Interrupt Registers */         \
    {0x00003000 + HAWK_TSU_CHANNEL_OFFSET_MAC(group, channel), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003400 + HAWK_TSU_CHANNEL_OFFSET_MAC(group, channel), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define HAWK_TSU_PER_GROUP_ACTIVE_MEM(group)     \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  0),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  1),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  2),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  3),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  4),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  5),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  6),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  7),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  8),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group,  9),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 10),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 11),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 12),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 13),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 14),      \
    HAWK_TSU_CHANNEL_ACTIVE_MEM(group, 15)

/**
* @internal smemHawkUnitPha function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PHA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitPha
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    GT_U32  ppg,ppn;
    GT_U32  ppgOffset,ppnOffset,totalOffset;

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*PPA Interrupt Cause Register*/
    {0x007EFF10 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007EFF14 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /*PPA internal error cause Register*/
    {0x007EFF20 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007EFF24 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    /* 4 PPG (with 10 PPNs each) */
    HAWK_PPG_REPLICATIONS_ACTIVE_MEM(0),/*include all it's PPN*/
    HAWK_PPG_REPLICATIONS_ACTIVE_MEM(1),/*include all it's PPN*/
    HAWK_PPG_REPLICATIONS_ACTIVE_MEM(2),/*include all it's PPN*/
    HAWK_PPG_REPLICATIONS_ACTIVE_MEM(3),/*include all it's PPN*/

    /*PHA Internal Error Cause*/
    {0x007FFF80 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007FFF84 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /*PHA Interrupt Sum Cause*/
    {0x007FFF88 , SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x007FFF8c , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PHA */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F0000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(23 , 4),SMEM_BIND_TABLE_MAC(PHA_PPAThreadsConf1)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F4000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16 , 4),SMEM_BIND_TABLE_MAC(PHA_sourcePortData )}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F6000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16 , 4),SMEM_BIND_TABLE_MAC(PHA_targetPortData )}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007F8000, 8192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(149,32),SMEM_BIND_TABLE_MAC(PHA_PPAThreadsConf2)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF00, 0x007FFF04)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF20, 0x007FFF20)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF30, 0x007FFF30)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF38, 0x007FFF3C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF60, 0x007FFF68)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF70, 0x007FFF74)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFF80, 0x007FFF94)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFFA0, 0x007FFFA0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFFA8, 0x007FFFA8)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFFB0, 0x007FFFB8)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFFD0, 0x007FFFD0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007FFFD8, 0x007FFFD8)}}

            /* PPA */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x007C0000, 8192)}} /*Shared DMEM*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF00, 0x007EFF00)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF10, 0x007EFF14)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF20, 0x007EFF24)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF30, 0x007EFF30)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF40, 0x007EFF4C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x007EFF70, 0x007EFF70)}}

            /* Per PPG */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16)}, HAWK_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}  /*IMEM*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF00, 0x0007FF04)}, HAWK_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF10, 0x0007FF14)}, HAWK_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF70, 0x0007FF70)}, HAWK_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}

            /* per PPN */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2048)}      , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*scratchpad*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 20)}        , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet configuration*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002020, 4)}         , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet special registers*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002100, 64)}        , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet descriptor*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002200, 160)}       , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet header*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003008)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003010, 0x00003010)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003020, 0x00003024)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x00003044)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003050, 0x00003058)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003060, 0x00003068)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003070, 0x00003070)} , HAWK_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* PHA */
             {DUMMY_NAME_PTR_CNS,            0x007fff04,         0x00000001,      2,    0x2c         }
            ,{DUMMY_NAME_PTR_CNS,            0x007FFF64,         0x00000001,      1,    0x0          }
            ,{DUMMY_NAME_PTR_CNS,            0x007FFFD0,         0xFFFF0000,      1,    0x0          }
            /* PPA */
            ,{DUMMY_NAME_PTR_CNS,            0x007eff00,         0x00000001,      1,    0x0          }
            ,{DUMMY_NAME_PTR_CNS,            0x007eff40,         0xffff0000,      1,    0x0          }

            /* per PPN */
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      HAWK_PPN_REPLICATIONS   }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }


    {/*startof unit pha_regs */
        {/*start of unit PPAThreadConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.PPAThreadConfigs.nullProcessingInstructionPointer = 0x007fff20;

        }/*end of unit PPAThreadConfigs */


        {/*start of unit generalRegs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.generalRegs.PHACtrl = 0x007fff00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.generalRegs.PHAStatus = 0x007fff04;

        }/*end of unit generalRegs */


        {/*start of unit errorsAndInterrupts */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.headerModificationViolationConfig = 0x007fff60;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PPAClockDownErrorConfig = 0x007fff68;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.headerModificationViolationAttributes = 0x007fff70;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PHABadAddrLatch = 0x007fff74;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorCause = 0x007fff80;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorMask = 0x007fff84;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause = 0x007fff88;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask = 0x007fff8c;

        }/*end of unit errorsAndInterrupts */

        {/*start of unit debugCounters */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.debugCounters.statisticalProcessingDebugCounterConfig = 0x007FFF90;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.pha_regs.debugCounters.statisticalProcessingDebugCounter       = 0x007FFF94;

        }/*end of unit debugCounters */

    }/*end of unit pha_regs */

    {/*start of unit PPA */
        {/*start of unit ppa_regs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPACtrl = 0x007eff00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInterruptSumCause = 0x007eff10;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInterruptSumMask = 0x007eff14;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInternalErrorCause = 0x007eff20;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPAInternalErrorMask = 0x007eff24;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPA.ppa_regs.PPABadAddrLatch = 0x007eff70;

        }/*end of unit ppPHA.a_regs */


    }/*end of unit PPA */

    /*m * 0x80000: where m (0-3) represents PPG*/
    for(ppg = 0 ; ppg < 4; ppg++) /* per PPG */
    {
        ppgOffset = ppg * 0x80000;

        {/*start of unit ppg_regs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInterruptSumCause = 0x0007ff00 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInterruptSumMask = 0x0007ff04 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInternalErrorCause = 0x0007ff10 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInternalErrorMask = 0x0007ff14 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGBadAddrLatch = 0x0007ff70 + ppgOffset;

        }/*end of unit ppg_regs */

        /* 0x4000 * p: where p (0-9) represents PPN*/
        for(ppn = 0 ; ppn < 10; ppn++) /* per PPN (PER PPG) */
        {/*start of unit ppn_regs */
            ppnOffset = ppn * 0x4000 ;
            totalOffset = ppgOffset + ppnOffset;

            {/*3050+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord0[t] =
                        0x3050+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3050+0x10 * t*/
            {/*3054+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord1[t] =
                        0x3054+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3054+0x10 * t*/
            {/*3058+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord2[t] =
                        0x3058+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3058+0x10 * t*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNDoorbell = 0x00003010 + totalOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.pktHeaderAccessOffset = 0x00003024 + totalOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNInternalErrorCause = 0x00003040 + totalOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNInternalErrorMask = 0x00003044 + totalOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNBadAddrLatch = 0x00003070 + totalOffset;

        }/*end of unit ppn_regs */

    }

}

/**
* @internal smemHawkUnitPpu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PPU unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitPpu
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000002BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000004BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x000006BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000011F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x000013F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x000015F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A00, 0x00001AF8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001BF8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x00001CF8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x000021F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x000023F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x000025F4)}

            /* PPU Action Table 0 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00003000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(379, 64),SMEM_BIND_TABLE_MAC(ppuActionTable0)}
            /* PPU Action Table 1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00005000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(379, 64),SMEM_BIND_TABLE_MAC(ppuActionTable1)}
            /* PPU Action Table 2 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00007000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(379, 64),SMEM_BIND_TABLE_MAC(ppuActionTable2)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x00010010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010020, 0x00010020)}

            /* PPU DAU Profile Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00011000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(480, 64),SMEM_BIND_TABLE_MAC(ppuDauProfileTable)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000007,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000000a0,         0x0000ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000000ac,         0x00026659,      1,    0x0}
            ,{NULL,                          0x00000000,         0x00000000,      0,    0x0}
         };
         static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
         unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/* start of unit ppu_regs */
        {/* start of unit generalRegs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.generalRegs.ppuGlobalConfig = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.generalRegs.ppuInterruptCause = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.generalRegs.ppuInterruptMask = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.generalRegs.ppuLastAddrViolation = 0x0000000C;
        }/* end of unit generalRegs */

        {/* start of unit debugRegs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterIn = 0x00000060;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterOut = 0x00000064;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterKstg0 = 0x00000068;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterKstg1 = 0x0000006C;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterKstg2 = 0x00000070;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterDau = 0x00000074;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebugCounterLoopback = 0x00000078;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuMetalFixRegister = 0x000000A0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuIdleState = 0x000000A4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuDebug = 0x000000A8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.debugRegs.ppuThresholds = 0x000000AC;
        }/* end of unit debugRegs */

        {/* start of SP BUS default profile %p word %w */
            GT_U32 w, p;

            /* 0x00000010 + 0x4*w + 0x14*p */
            for(p = 0; p < 4; p++)
                for(w = 0; w<5; w++)
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.spBusDefaultProfile[p].word[w] = 0x00000010 + 0x4*w + 0x14*p;
        }/* end of SP BUS default profile %p word %w */

        {/* start of PPU error profile %p field %b */
            GT_U32 p,b;

            /* 0x000000B0 + 0x4*b + 0x10*p */
            for(p = 0; p < 2; p++)
                for(b = 0; b<4; b++)
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.errProfile[p].field[b] = 0x000000B0 + 0x4*b + 0x10*p;
        }/* end of PPU error profile %p field %b */

        {/* start of interrupt counters */
            GT_U32 i;

            /* 0x000000D0 + 0x4*i */
            for(i = 0; i < 8; i++)
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.interruptCounter[i] = 0x000000D0 + 0x4*i;
        }/* end of interrupt counters */

        {/* start of key gen Profile k_stage %k profile %i byte %b */
            GT_U32 k, i, b;

            /* 0x00000100 + 0x4*b+0x1c*i + 0x200*k */
            for(k = 0; k < 3; k++)
            {
                for(i = 0; i < 16; i++)
                    for(b = 0; b < 7; b++)
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].keyGenProfile[i].byte[b] = 0x00000100 + 0x4*b + 0x1c*i + 0x200*k;
            }

        }/* end of key gen Profile k_stage %k profile %i byte %b */

        {/* start of tcam regs */
            GT_U32 k, i;

            for(k = 0; k < 3; k++)
                for(i = 0; i < 63; i++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].tcamKey[i].keyLsb = 0x00001000 + 0x8*i + 0x200*k;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].tcamKey[i].keyMsb = 0x00001004 + 0x8*i + 0x200*k;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].tcamKey[i].maskLsb = 0x00002000 + 0x8*i + 0x200*k;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].tcamKey[i].maskMsb = 0x00002004 + 0x8*i + 0x200*k;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.kstg[k].tcamKey[i].validate = 0x00001A00 + 0x4*i + 0x100*k;
                }
        }/* end of tcam regs */

        {/* start of dau registers */
            {/*0x00010000 + 0x4*i*/
                GT_U32 i;
                for(i=0; i<5; i++)
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.dau.dauProtWin[i] = 0x00010000 + 0x4*i;
            }/*0x00010000 + 0x4*i*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PPU.dau.dauProtWinLatchData = 0x10020;

        }/* end of dau registers */
    }/* end of unit ppu_regs */
}
/**
* @internal smemHawkUnitEgfSht function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-SHT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitEgfSht
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* L2 Port Isolation Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00040000 ,67584 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(l2PortIsolation)}
            /* Egress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000B0000 , 65536 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(egressStp)}
            /* Non Trunk Members 2 Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00090000 , 65536  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(nonTrunkMembers2)}
            /* Source ID Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000F0000 , 65536 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(sst)}
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            /* NOTE: according to Design team the RAM was removed , although CIDER keep showing this table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egfShtEportEVlanFilter)}
#endif /*0*/
            /* Multicast Groups Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000C0000 , 196608 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(mcast)}
             /* Device Map Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A8000 ,  16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(deviceMapTable)}
             /* Vid Mapper Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A0000 ,  32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtVidMapper)}
             /* Designated Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000AD000 ,   2048  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(designatedPorts)}
            /* Egress EPort table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00078000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(egfShtEgressEPort)}
            /* Non Trunk Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000AC000 , 4096  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(nonTrunkMembers)}
            /* L3 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00060000 ,67584), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(l3PortIsolation)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100000, 0x00100010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100020, 0x00100020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100030, 0x00100030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100040, 0x0010004C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001000C0, 0x001000CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100140, 0x0010014C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001001C0, 0x001001CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100240, 0x0010024C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001002C0, 0x001002CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100340, 0x0010034C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001003C0, 0x001003DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001004C0, 0x001004CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100540, 0x0010054C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001005C0, 0x001005FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001007C0, 0x001008BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100FC0, 0x00100FCC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00101040, 0x0010104C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001010C0, 0x001010CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00101140, 0x00101140)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        GT_U32  numStg = devObjPtr->limitedResources.stgId;/* stp table -- support Phoenix */

        chunksMem[1].numOfRegisters = (numStg) * (chunksMem[0].enrtyNumBytesAlignement / 4);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Egress vlan table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(egressVlan)}
            /* EVlan Attribute table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00080000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanAttribute)}
            /* EVlan Spanning table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00088000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanSpanning)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  index = 0;

        /* support Hawk , Phoenix */
        chunksMem[index].numOfRegisters = numEVlans * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
        chunksMem[index].numOfRegisters = numEVlans * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
        chunksMem[index].numOfRegisters = numEVlans * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00100000,         0x01fff007,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100004,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100008,         0x0000000f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0010000c,         0x00000078,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100030,         0xffff0000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100040,         0xffffffff,      4,    0x4,      2,    0x200   }
            ,{DUMMY_NAME_PTR_CNS,            0x00100340,         0xffffffff,      4,    0x4    }

             /* Device Map Table */
            /* Default value of 'UplinkIsTrunk - 0x1 = Trunk;' like in legacy devices */
            ,{DUMMY_NAME_PTR_CNS,           0x000A8000,         0x00000002,   16384/4,    0x4   }
             /* Designated Port Table */
            ,{DUMMY_NAME_PTR_CNS,           0x000AD000,         0xffffffff,    2048/4,    0x4   }
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020000,         0xffffffff,  131072/4,    0x4   }
#endif
            /* Source ID Members Table */
            ,{DUMMY_NAME_PTR_CNS,           0x000F0000,         0xffffffff,   65536/4,    0x4   }

#if 0   /* the Design not initialize the vidx 0xFFF entry any more ! */
            /* Multicast Groups Table */
            /* vidx 0xfff - 128 members */
            ,{DUMMY_NAME_PTR_CNS, 0x000C0000 + (0xfff * 0x10),  0xffffffff,         4,    0x4   }
#endif
            /* Egress vlan table */
            /* vlan 1 members (first 128 port )*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000000 + (1 * 0x10),      0xffffffff,         4,    0x4   }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit EGF_sht */
        GT_U32  numOfPorts = 128;

        {/*start of unit global */
            {/*start of unit miscellaneous */
/*              SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.miscellaneous.badAddrLatchReg = 0x00101140;*/

            }/*end of unit miscellaneous */


            {/*01007c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/2)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportAssociatedVid1[n] =
                        0x01007c0+n*0x4;
                }/* end of loop n */
            }/*01007c0+n*0x4*/
            {/*0100040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.UCSrcIDFilterEn[n] =
                        0x0100040+0x4*n;
                }/* end of loop n */
            }/*0100040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MESHIDConfigs = 0x00100004;
            {/*01010c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MCLocalEn[n] =
                        0x01010c0+0x4*n;
                }/* end of loop n */
            }/*01010c0+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsMask = 0x00100020;
            {/*0101040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        0x0101040+0x4*n;
                }/* end of loop n */
            }/*0101040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTGlobalConfigs = 0x00100000;
            {/*01002c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpStateMode[n] =
                        0x01002c0+0x4*n;
                }/* end of loop n */
            }/*01002c0+0x4*n*/
            {/*0100340+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpState[n] =
                        0x0100340+0x4*n;
                }/* end of loop n */
            }/*0100340+0x4*n*/
            {/*01001c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        0x01001c0+0x4*n;
                }/* end of loop n */
            }/*01001c0+0x4*n*/
            {/*01003c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/16)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportPortIsolationMode[n] =
                        0x01003c0+0x4*n;
                }/* end of loop n */
            }/*01003c0+0x4*n*/
            {/*0100540+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        0x0100540+0x4*n;
                }/* end of loop n */
            }/*0100540+0x4*n*/
            {/*01005c0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/8)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ePortMeshID[n] =
                        0x01005c0+n * 0x4;
                }/* end of loop n */
            }/*01005c0+n * 0x4*/
            {/*01004c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        0x01004c0+0x4*n;
                }/* end of loop n */
            }/*01004c0+0x4*n*/
            {/*01000c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        0x01000c0+0x4*n;
                }/* end of loop n */
            }/*01000c0+0x4*n*/
            {/*0100240+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        0x0100240+0x4*n;
                }/* end of loop n */
            }/*0100240+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportVlanEgrFiltering = 0x0010000c;
            {/*0100fc0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportDropOnEportVid1Mismatch[n] =
                        0x0100fc0+n * 0x4;
                }/* end of loop n */
            }/*0100fc0+n * 0x4*/
            {/*0100140+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= (numOfPorts/32)-1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportBridgedForwardRestricted[n] =
                        0x0100140+0x4*n;
                }/* end of loop n */
            }/*0100140+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsCause = 0x00100010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTEgrFiltersEnable = 0x00100008;

        }/*end of unit global */


    }/*end of unit EGF_sht */


}

/**
* @internal smemHawkUnitIpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitIpcl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* PCL Unit Interrupt Cause */
        {0x00000004, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg,0},
        {0x00000008, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000003C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000044, 0x00000048)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000005C, 0x0000005C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000078)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000100)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000005C0, 0x000005FC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(pearsonHash)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x0000073C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000744, 0x0000077C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000850)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000860, 0x000008B0)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000CFC)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000,  512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, 32),SMEM_BIND_TABLE_MAC(crcHashMode)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(148, 32),SMEM_BIND_TABLE_MAC(crcHashMask)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl0SourcePortConfig)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl1SourcePortConfig)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl2SourcePortConfig)}
            /* next are set below as formula of 3 tables
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 49152)}
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 49152)}
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 49152)}
            */
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 14336),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(513,128),SMEM_BIND_TABLE_MAC(ipcl0UdbSelect)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000, 14336),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(513,128),SMEM_BIND_TABLE_MAC(ipcl1UdbSelect)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048000, 14336),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(513,128),SMEM_BIND_TABLE_MAC(ipcl2UdbSelect)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        if(devObjPtr->isIpcl0NotValid)/* support for ironman */
        {
            /* remove the ipcl0UdbSelect , by override with last entry */
            chunksMem[numOfChunks-3] = chunksMem[numOfChunks-1];
            /* reduce the number of entries */
            numOfChunks --;
        }

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    /* chunks with formulas */
    if(devObjPtr->isIpcl0NotValid)/* support for ironman */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IPCL0,IPCL1,IPCL2 Configuration Table */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000, 49152), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(33, 8),SMEM_BIND_TABLE_MAC(pclConfig)}, FORMULA_SINGLE_PARAMETER(2 , 0x00010000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    else
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IPCL0,IPCL1,IPCL2 Configuration Table */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010000, 49152), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(33, 8),SMEM_BIND_TABLE_MAC(pclConfig)}, FORMULA_SINGLE_PARAMETER(3 , 0x00010000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x05E00001,            }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x02801000,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x0000ffff,            }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000028,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x0000004a,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x00000fff,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x3fffffff,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x0000ffff,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000730,         0x00ff0080,            }
            ,{DUMMY_NAME_PTR_CNS,            0x00000738,         0x00080008,            }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };



        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemSip6PolicerSetTableSize function
* @endinternal
*
* @brief   update table size - support Hawk and Phoenix
*
* @param[in] numOfChunks              - pointer to device object.
*                                      unitPtr - pointer to the unit chunk
*/
void smemSip6PolicerSetTableSize
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  numOfChunks,
    INOUT SMEM_CHUNK_BASIC_STC  chunksMem[]
)
{
    GT_U32  ii;
    GT_U32  numOfMeters = devObjPtr->policerSupport.iplrTableSize;
    GT_U32  numIpfix    = devObjPtr->policerSupport.numOfIpfix;
    GT_U32  numEntries;

    for(ii = 0 ; ii < numOfChunks ; ii ++)
    {
        numEntries = 0;

        if(chunksMem[ii].tableOffsetValid == 0)
        {
            continue;
        }

         /* in sip 6 each PLR unit has it's own instance of the metering configuration table */
        switch(chunksMem[ii].tableOffsetInBytes)
        {
            case FIELD_OFFSET_IN_STC_MAC(policer        ,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(policerConfig  ,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(policer1Config ,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(egressPolicerConfig,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(policerConformanceLevelSign,SKERNEL_TABLES_INFO_STC):
                numEntries = numOfMeters;
                break;
            case FIELD_OFFSET_IN_STC_MAC(policerCounters,SKERNEL_TABLES_INFO_STC):
                numEntries = MAX(numIpfix,numOfMeters);
                break;
            case FIELD_OFFSET_IN_STC_MAC(policerIpfix1StNPackets,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(policerIpfixWaAlert,SKERNEL_TABLES_INFO_STC):
            case FIELD_OFFSET_IN_STC_MAC(policerIpfixAgingAlert,SKERNEL_TABLES_INFO_STC):
                numEntries = numIpfix / 32;
                break;
            case FIELD_OFFSET_IN_STC_MAC(policerHierarchicalQos,SKERNEL_TABLES_INFO_STC):
                numEntries = numOfMeters;
                break;
            default :
                break;
        }

        if(numEntries)
        {
            chunksMem[ii].numOfRegisters = numEntries * (chunksMem[ii].enrtyNumBytesAlignement / 4);
        }
    }
}

/**
* @internal smemHawkUnitPolicerUnify function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the any of the 3 Policers:
*         1. iplr 0
*         2. iplr 1
*         3. eplr
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] plrUnit                  - PLR unit
*/
static void smemHawkUnitPolicerUnify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN SMEM_SIP5_PP_PLR_UNIT_ENT   plrUnit
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000070)}
            /*registers -- not table/memory !! -- Policer Table Access Data<%n> */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000074 ,8*4),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTblAccessData)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000228)}
            /*Policer Timer Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000300, 36), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTimer)}
            /*Policer Management Counters Memory --> NOTE: those addresses are NOT accessible by CPU !!! */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (POLICER_MANAGEMENT_COUNTER_ADDR_CNS/*0x00000500*/, 192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74,16),SMEM_BIND_TABLE_MAC(policerManagementCounters)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002010, 0x00002014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002020, 0x00002058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x000024FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003510, 0x00003514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003520, 0x00003524)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003550, 0x00003554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003560, 0x00003564)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003604, 0x00003614)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003704, 0x00003714)}
            /* port attributes (replace -- Port%p and Packet Type Translation Table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000 , 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13,4),SMEM_BIND_TABLE_MAC(policerMeterPointer)}
            /* replace : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInitialDP*/
            /*qos Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 1024) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20,4),SMEM_BIND_TABLE_MAC(policerQosAttributes)}
            /*IPFIX wrap around alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixWaAlert)}
            /*IPFIX aging alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixAgingAlert)}
            /*Ingress Policer Re-Marking Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 8192 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(51,8),SMEM_BIND_TABLE_MAC(policerReMarking)}
            /*Metering Conformance Level Sign Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 16384 ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(2,4),SMEM_BIND_TABLE_MAC(policerConformanceLevelSign)}
            /*e Attributes Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerEPortEVlanTrigger)}
            /* IPFIX 1st N Packets Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000, 8192) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfix1StNPackets) }
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemBobkPolicerTablesSupport(devObjPtr,numOfChunks,chunksMem,plrUnit);
        smemSip6PolicerSetTableSize(devObjPtr,numOfChunks,chunksMem);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    if(plrUnit == SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E)
    {
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Hierarchical Policing Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 16384 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14,4), SMEM_BIND_TABLE_MAC(policerHierarchicalQos)}
                /*Metering Token Bucket Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 131072 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(211,32),SMEM_BIND_TABLE_MAC(policer)}
                /*Counting Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 2097152), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(229,32),SMEM_BIND_TABLE_MAC(policerCounters)}
                /*Metering Configuration Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768  ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(55,8),SMEM_BIND_TABLE_MAC(policerConfig)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemSip6PolicerSetTableSize(devObjPtr,numOfChunks,chunksMem);
            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }
    else
    if (plrUnit == SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E)
    {
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Metering Configuration Memory*/
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768  ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(55,8),SMEM_BIND_TABLE_MAC(policer1Config)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemSip6PolicerSetTableSize(devObjPtr,numOfChunks,chunksMem);
            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }
    else
    if (plrUnit == SMEM_SIP5_PP_PLR_UNIT_EPLR_E)
    {
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Metering Configuration Memory*/
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768  ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(55,8),SMEM_BIND_TABLE_MAC(egressPolicerConfig)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemSip6PolicerSetTableSize(devObjPtr,numOfChunks,chunksMem);
            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }

    {
        /* registers defaults, relevant for IPLR0, IPLR1, EPLR */
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS, 0x00000000, 0xbe830003, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000004, 0x00000008, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x0000000c, 0xffffffff, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000010, 0x00019000, 1,     0x0}/* MRU is 100K (100*1024) */
            ,{DUMMY_NAME_PTR_CNS, 0x0000002c, 0x00000020, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000030, 0x3fffffff, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000034, 0x00001000, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000038, 0x00100000, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x0000003c, 0x0000000f, 1,     0x0}
            ,{DUMMY_NAME_PTR_CNS, 0x00000220, 0xffff0000, 1,     0x0}
            ,{NULL,               0,          0x00000000, 0,     0x0}
        };

        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr_policerConfig[] =
        {                                             /*numOfRepetitions*/
             {DUMMY_NAME_PTR_CNS, 0x00060000, 0x00000030, 0x1000, 0x8}
            ,{NULL,               0,          0x00000000, 0,     0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list_next =
            {myUnit_registersDefaultValueArr_policerConfig,NULL};
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list =
            {myUnit_registersDefaultValueArr,&list_next};

        GT_U32  numOfMeters = devObjPtr->policerSupport.iplrTableSize;
        myUnit_registersDefaultValueArr_policerConfig[0].numOfRepetitions = numOfMeters;

        unitPtr->unitDefaultRegistersPtr = &list;
    }

#if 0
    /* NOTE :  the next registers replaces with new table : policerQosAttributes */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInitialDP[n] =
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].qosProfileToPriority_tab =  /* 128 regs each (8 * 4bits) */
    /* NOTE :  the next registers replaces with new table : policerMeterPointer */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerPortMeteringReg[n] =

#endif

    {/*start of unit globalConfig */
        {/*0002040+4*t*/
            GT_U32    t;
            for(t = 0 ; t <= 6 ; t++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalConfig.countingConfigTableThreshold[t] =
                    0x0002040+4*t;
            }/* end of loop t */
        }/*0002040+4*t*/
        {/*0002020+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalConfig.countingConfigTableSegment[n] =
                    0x0002020+4*n;
            }/* end of loop n */
        }/*0002020+4*n*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringBaseAddr = 0x00000028;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCountingBaseAddr = 0x00000024;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCtrl1 = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCtrl0 = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMRU = 0x00000010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].meteringAddressingModeConfiguration0 = 0x00000040;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].hierarchicalPolicerCtrl = 0x0000002c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalBillingCntrIndexingModeConfig1 = 0x0000000c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalBillingCntrIndexingModeConfig0 = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].meteringAddressingModeConfiguration1 = 0x00000044;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerOAM = 0x00000224;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalConfig.plrAnalyzerIndexes = 0x00000050;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].statisticalMeteringConfig0 = 0x00002004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].statisticalMeteringConfig1 = 0x00002008;
    }/*end of unit globalConfig */

    {/*start of unit IPFIX */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXCtrl = 0x00000014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXNanoTimerStampUpload = 0x00000018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSecondsLSbTimerStampUpload = 0x0000001c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSecondsMSbTimerStampUpload = 0x00000020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXDroppedPktCountWAThreshold = 0x00000030;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXPktCountWAThreshold = 0x00000034;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXByteCountWAThresholdLSB = 0x00000038;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXByteCountWAThresholdMSB = 0x0000003c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSampleEntriesLog0 = 0x00000048;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSampleEntriesLog1 = 0x0000004c;
        /* SIP_6_10 */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXFirstNPacketsConfig0 = 0x00000060;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXFirstNPacketsConfig1 = 0x00000064;

    }/*end of unit IPFIX */

    {/*start of unit Miscellaneous */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInterruptMask = 0x00000204;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInterruptCause = 0x00000200;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerBadAddressLatch = 0x00002000;

    }/*end of unit Miscellaneous */

    {/*start of unit Indirect Table Access */
        {/*74+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerTableAccessData[n] =
                    0x74+n*4;
            }/* end of loop n */
        }/*74+n*4*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerTableAccessCtrl = 0x00000070;

    }/*end of unit Indirect Table Access */

    {/*start of unit scan */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringRefreshScanRateLimit = 0x000000b0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringRefreshScanAddrRange = 0x000000b4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerScanCtrl = 0x000000b8;

    }/*end of unit scan */

    {/*start of unit MEF_10_3 */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
            mef10_3_bucket0_max_rate_tab = 0x00002100; /* 128 regs*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
            mef10_3_bucket1_max_rate_tab = 0x00002300; /* 128 regs */

    }/*end of unit MEF_10_3 */

}

static void smemHawkUnitIplr0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemHawkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E);
}

static void smemHawkUnitIplr1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer */
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemHawkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E);
}

static void smemHawkUnitEplr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC3_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_EPLR_E),

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    smemHawkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_EPLR_E);
}
static void smemHawkUnitInitTai
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        /* aligned to /Cider/EBU-IP/GOP/GOP LEGACY/TAI IP/TAI IP 7.0/TAI IP {7.1.6} */
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000F8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000324)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000330, 0x0000035C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000414)}


        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }
}


/**
* @internal smemHawkUnitTti function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TTI unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemHawkUnitTti
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x000001BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001E8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000218)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000350, 0x000003CC)}



            /*DSCP To DSCP Map - one entry per translation profile
              Each entry holds 64 DSCP values, one per original DSCP value. */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384 , 64),SMEM_BIND_TABLE_MAC(dscpToDscpMap)}
            /*EXP To QosProfile Map - Each entry represents Qos translation profile
              Each entry holds QoS profile for each of 8 EXP values*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000700, 192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80, 16),SMEM_BIND_TABLE_MAC(expToQoSProfile)}
            /*DSCP To QosProfile Map - holds 12 profiles, each defining a 10bit QoSProfile per each of the 64 DSCP values : 0x400 + 0x40*p: where p (0-11) represents profile*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 1536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(640 , 128),SMEM_BIND_TABLE_MAC(dscpToQoSProfile)}
            /*UP_CFI To QosProfile Map - Each entry holds a QoS profile per each value of {CFI,UP[2:0]} */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000E00, 384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, 32),SMEM_BIND_TABLE_MAC(upToQoSProfile)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000103C)} /* PPU Profile */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x0000106C)} /* Tunnel Header Length Profile */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013B0, 0x000013DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013F8, 0x00001424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001430, 0x000014D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001540)}
            /* next are registers but hold 128 'entries' each */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002700, 0x000028FC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ttiEmProfileId1Mapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002900, 0x00002AFC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ttiEmProfileId2Mapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002C00, 0x00002CFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002E00, 0x00002EFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000300C, 0x0000300C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x0000307C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003090, 0x000030AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x0000324C),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ttiPacketTypeTcamProfileIdMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x000034FC),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ttiPortAndPacketTypeTcamProfileIdMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x000065FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000703C)}

            /* Default Port Protocol eVID and QoS Configuration Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(portProtocolVidQoSConf)}
            /* PCL User Defined Bytes Configuration Memory -- 70 udb's in each entry */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(840, 128),SMEM_BIND_TABLE_MAC(ipclUserDefinedBytesConf)}
            /* TTI User Defined Bytes Configuration Memory -- TTI keys based on UDB's : 8 entries support 8 keys*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 1280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384, 64),SMEM_BIND_TABLE_MAC(ttiUserDefinedBytesConf)}
            /* VLAN Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13, 4),SMEM_BIND_TABLE_MAC(ingressVlanTranslation)}
            /* Port to Queue Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 512), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(ttiPort2QueueTranslation)}
            /*Physical Port Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(92, 16),SMEM_BIND_TABLE_MAC(ttiPhysicalPortAttribute)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110000, 0x0011012c)}/* only 75 'my physical ports' supported !!! */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110500, 0x00110500)}

            /*Physical Port Attributes 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(251, 32),SMEM_BIND_TABLE_MAC(ttiPhysicalPort2Attribute)}

            /*Default ePort Attributes (pre-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00210000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(126, 16),SMEM_BIND_TABLE_MAC(ttiPreTtiLookupIngressEPort)}
            /*ePort Attributes (post-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00240000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(ttiPostTtiLookupIngressEPort)}

            /* QCN to Pause Timer Map*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(ttiQcnToPauseTimerMap)}

         };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* TTI Unit Global Configuration ext 3 */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000001EC, 4)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* (new table in sip6) vrf_id eVlan Mapping Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ttiVrfidEvlanMapping)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  index = 0;

        /* support Hawk , Phoenix */
        chunksMem[index].numOfRegisters = numEVlans * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    { /* sip5: chunks with formulas */
        {
            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                /* MAC2ME Registers */ /* registers not table -- SMEM_BIND_TABLE_MAC(macToMe)*/
                 {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00001600,0)}, FORMULA_TWO_PARAMETERS(6 , 0x4 , 128, 0x20)}
            };

            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*                          23 LSBits of 'Addr',     val,                      */
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x30002503,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000020,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x65586558,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x0000000e,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000024,         0x1001ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000028,         0x000088e7,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x88488847,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x65586558,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000038,         0x00003232,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000003c,         0x0000000d,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0xff000000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000001,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000004c,         0xff020000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0xff000000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000054,         0xffffffff,      3,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000060,         0x00001800,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000068,         0x1b6db81b,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000006c,         0x00000007,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x00008906,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x000fff00,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000088,         0x3fffffff,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000000f0,         0xffffffff,      4,    0x4,      2,    0x10}
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x0000004b,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x00001320,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x0000001b,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x20a6c01b,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x24924924,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x00ffffff,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x0000311f,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0fffffff,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000188,         0x0a00003c,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000018c,         0x00000300,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000001e4,         0x00000002,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000001e8,         0x06000104,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000200,         0x030022f3,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000204,         0x00400000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x12492492,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00000092,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0x0180c200,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000300,         0x81008100,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000310,         0xffffffff,      2,    0x8}
            ,{DUMMY_NAME_PTR_CNS,            0x00000340,         0x0000000f,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001050,         0x000001fc,      8,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000013bc,         0x0000003f,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000013f8,         0x0000ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000013fc,         0x000001da,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001500,         0x00602492,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0x00000fff,    128,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00001608,         0xffff1fff,    128,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x0000160c,         0xffffffff,    128,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x88f788f7,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x013f013f,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000300c,         0x00000570,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00003094,         0x88b588b5,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00007000,         0x00224fd0,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00007020,         0x08002001,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00007030,         0x00050000,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00110500,         0x88a8893f,      1,    0x0}

            ,{NULL,                          0,                   0x00000000,     0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* ROC (read only clear) counters */
        /*  CT_byte_count_extarction_fail_counter */
        {0x00000184, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /* TTI Engine Interrupt Cause */
        {0x00000004, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg,0,smemChtActiveWriteIntrCauseReg,0},
        {0x00000008, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

}

/**
* @internal smemHawkUnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemHawkUnitHa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* HA Interrupt Cause Register */
        {0x00000300, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 12 /*parent summary bit*/, smemChtActiveWriteIntrCauseReg, 0},
        /* HA Interrupt Mask Register */
        {0x00000304, SMEM_FULL_MASK_CNS,
            NULL, 0, smemLion3ActiveWriteHaInterruptsMaskReg, 0},

        /* ROC (read only clear) counters */
        /*HA Oversize Tags Drop Counter*/
        { 0x0000030C, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /*HA ECC or SER Drop Counter*/
        { 0x00000310, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /*Nat Exception Drop Counter*/
        { 0x00000314, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /*HA Set<<%s>> traffic counter - 2 registers*/
        { 0x000003C0, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x000003C4, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000070)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C0, 0x000003C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003D0, 0x000003D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000410)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000420, 0x00000428)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000430, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000550, 0x00000560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000067C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000720)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000009FC)}
            /*HA Physical Port Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(58, 8), SMEM_BIND_TABLE_MAC(haEgressPhyPort1)}
             /*EPCL User Defined Bytes Configuration Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(720,128), SMEM_BIND_TABLE_MAC(haEpclUserDefinedBytesConfig)}
             /*HA Physical Port Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(95, 16), SMEM_BIND_TABLE_MAC(haEgressPhyPort2)}
            /*HA QoS Profile to EXP Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(3, 4), SMEM_BIND_TABLE_MAC(haQosProfileToExp)}
             /*HA Global MAC SA Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8), SMEM_BIND_TABLE_MAC(haGlobalMacSa)}
            /*PTP Domain table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 5120) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(35, 8), SMEM_BIND_TABLE_MAC(haPtpDomain)}
            /*Generic TS Profile table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(537,128), SMEM_BIND_TABLE_MAC(tunnelStartGenericIpProfile) }
            /*HA Egress ePort Attribute Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4), SMEM_BIND_TABLE_MAC(haEgressEPortAttr2)}
            /*HA Egress ePort Attribute Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 131072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(97,16), SMEM_BIND_TABLE_MAC(haEgressEPortAttr1)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /*EVLAN Table (was 'vlan translation' in legacy device)*/
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36, 8), SMEM_BIND_TABLE_MAC(egressVlanTranslation)}
            /*VLAN MAC SA Table (was 'VLAN/Port MAC SA Table' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4), SMEM_BIND_TABLE_MAC(vlanPortMacSa)}
            /*Router ARP DA and Tunnel Start Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 2097152),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384,64), SMEM_BIND_TABLE_MAC(arp)/*tunnelStart*/}
        };

        GT_U32 index = 0;
        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  numArps = devObjPtr->limitedResources.numOfArps;
        /* number of entries : evlans  . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters =  numEVlans     * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
#if 0 /* the VLAN MAC SA Table should hold 'numEVlans' entries but in Phoenix was 'forgoten' and keep 8K entries like in Hawk */
        /* number of entries : evlans  . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters =  numEVlans     * (chunksMem[index].enrtyNumBytesAlignement / 4);
#endif
        index++;
        /* number of entries : 8 ARPs in line . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters = (numArps/8)    * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;

        smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
            ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00101010,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x03fdd003,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000000c,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x000c0000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0xff000000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x00000001,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000008c,         0xff020000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0xffffffff,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000000a4,         0x00000003,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x000003d0,         0x00000008,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000408,         0x81000000,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000410,         0x00010000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000424,         0x00110000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000500,         0x00008100,      8,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000550,         0x88488847,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000554,         0x00008100,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000558,         0x000022f3,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000055c,         0x00006558,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x0000ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x000037bf,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x80747874,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000803,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x00000080,      1,    0x0}
            ,{NULL,                          0x00000000,         0x00000000,      0,    0x0}
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list =
                {myUnit_registersDefaultValueArr,NULL};

        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitEm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EM
*          unit
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitEm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*EM*/
    /* indirect Access Control Register */
    {0x00002000, SMEM_FULL_MASK_CNS, NULL, 0 , smemHawkActiveWriteExactMatchMsg,0},

    /* EM Global Configuration 1 register */
    {0x00000000, SMEM_FULL_MASK_CNS, NULL, 0 , smemHawkActiveWriteExactMatchGlobalCfg1Reg, 0},

    /* EM Interrupt Cause Register */
    {0x00004000, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 17, smemChtActiveWriteIntrCauseReg, 0},
    /* EM Interrupt Mask Register */
    {0x00004004, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteMacInterruptsMaskReg, 0},

    /* EM Flow ID Allocation Configuration 1*/
    {0x00001100, SMEM_FULL_MASK_CNS,NULL, 0, smemHawkActiveWriteEMFlowIdAllocationConfiguration1, 0},

    /* ROC (read only clear) counters */
    /*  EM flow Id fail counter */
    {0x00001148, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    /*  EM index fail counter */
    {0x0000114c, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001160)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002010)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004004)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005004)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006040)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006048, 0x0000604C)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
           /*Auto Learned EM Entry Index Table
                0x40000 + i: where i (0-65535) represents flowid */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 262144),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20 , 4),SMEM_BIND_TABLE_MAC(exactMatchAutoLearnedEntryIndexTable)}
        };

        /* support Hawk , Phoenix */
        chunksMem[0].numOfRegisters = devObjPtr->emAutoLearnNumEntries * (chunksMem[0].enrtyNumBytesAlignement / 4);

        smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
            ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000135,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0xffffffff,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x00001100,         0x0001fffe,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x00003000,         0x0000ffff,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x00005000,         0x0000843f,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x00005004,         0x00000009,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,           0x0000600c,         0x000007e0,      1,    0x0}
           ,{NULL,                         0x00000000,         0x00000000,      0,    0x0}

        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit Exact Match */

        {/*start of emGlobalConfiguration*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emGlobalConfiguration.emGlobalConfiguration1 = 0x0000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emGlobalConfiguration.emCRCHashInitConfiguration = 0x0004;
        }/*end of emGlobalConfiguration*/

        {/*start of emIndirectAccess*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emIndirectAccess.emIndirectAccessControl = 0x00002000;
            {/* 0x00002004 + n*3 */
                GT_U32    n;
                for (n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emIndirectAccess.emIndirectAccessData[n] =
                        0x00002004+4*n;
                }/* end of loop n */
            }/* 0x00002004 + n*3 */
        }/*end of emIndirectAccess*/

        {/*start of emInterrupt*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emInterrupt.emInterruptCauseReg = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emInterrupt.emInterruptMaskReg = 0x00004004;
        }/*end of emInterrupt*/

        {/*start of emSchedulerConfiguration*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emSchedulerConfiguration.emSchedulerSWRRArbiterWeights = 0x00005000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emSchedulerConfiguration.emSchedulerSWRRArbiterPriority = 0x00005004;
        }/*end of emSchedulerConfiguration*/

        {/*start of emAutoLearning*/ /* sip6_10 only*/

            {
                GT_U32    n;
                for (n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emProfileConfiguration1[n] =
                        0x00001000+0x10*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emProfileConfiguration2[n] =
                        0x00001004+0x10*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emProfileConfiguration3[n] =
                        0x00001008+0x10*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emProfileConfiguration4[n] =
                        0x0000100c+0x10*n;
                }/* end of loop n */
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdAllocationConfiguration1=0x00001100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdAllocationConfiguration2=0x00001104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus1=0x00001108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus2=0x0000110c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdAllocationStatus3=0x00001160;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emAutoLearningCollisionStatus1=0x00001110;
            {/* 0x00001114 + n*4 */
                GT_U32    n;
                for (n = 2 ; n <= 13 ; n++) {
                     SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emAutoLearningCollisionStatus_n[n-2] =
                        0x00001114+(n-2)*4 ;
                }/* end of loop n */
            }/* 0x00001114 + (n-2)*4: where n (2-13) represents chunk*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emSemaphore=0x00001144;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emFlowIdFailCounter=0x00001148;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emIndexFailCounter=0x0000114c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emRateLimiterFailCounter=0x00001150;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emRateLimitConfiguration=0x00001154;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emRateLimitBurstCounter=0x00001158;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emAutoLearning.emRateLimitPacketsCounter=0x0000115c;
        }/*end of emAutoLearning*/
    }
}

/**
* @internal smemHawkUnitEq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Eq unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitEq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* EQ active memories without STC interrupts */
        ACTIVE_MEM_EQ_WITHOUT_INT_STC_INT_COMMON_MAC,

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000001C, 0x00000024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000003C, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000058, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000011C)}
            #if 0 /* Ingress STC interrupts removed */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            #endif
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005014, 0x0000509C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x0000600C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000703C),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(tcpUdpDstPortRangeCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x0000783C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007C00, 0x00007C10),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ipProtCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000, 0x0000A008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A010, 0x0000A010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A020, 0x0000A034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A100, 0x0000A10C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF00, 0x0000AF08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF10, 0x0000AF10)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF30, 0x0000AF34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B000, 0x0000B000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B008, 0x0000B024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B04C, 0x0000B064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B200, 0x0000B218)}
            /*Mirroring to Analyzer Port Configurations/Port Ingress Mirror Index Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x0000B400, 64),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqPhysicalPortIngressMirrorIndexTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D010, 0x0000D014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x00010004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002000C, 0x00020010)}
            #if 0 /* Ingress STC Table was removed */
            /*Ingress STC Table/Ingress STC Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 8192)}/*SMEM_BIND_TABLE_MAC(ingrStc)*/
            #endif
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00080000, 0x000803F8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A0000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(136, 32), SMEM_BIND_TABLE_MAC(pathUtilization0)} /*Path_Utilization_Table_0*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000B0000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(136, 32), SMEM_BIND_TABLE_MAC(pathUtilization1)} /*Path_Utilization_Table_1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C0000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(136, 32), SMEM_BIND_TABLE_MAC(pathUtilization2)} /*Path_Utilization_Table_2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000D0000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(136, 32), SMEM_BIND_TABLE_MAC(pathUtilization3)} /*Path_Utilization_Table_3*/
            /*Source Port Hash Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000E0000, 512)  , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4), SMEM_BIND_TABLE_MAC(sourcePortHash)} /*Source_Port_Hash_Table*/
            /*Statistical Rate Limits Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(statisticalRateLimit)}
            /*QoSProfile to QoS Table/QoSProfile to QoS Table              */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00110000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(qosProfile)}
            /*CPU Code Table/CPU Code Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00120000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(cpuCode)}
            /*TO CPU Packet Rate Limiters/TO CPU Rate Limiter Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00130000, 1024)}
            /*L2 ECMP/Trunk LTT                                            */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqTrunkLtt)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    /* support phoenix */
    {
        GT_U32  numEPorts = devObjPtr->limitedResources.ePort;
        GT_U32  numL2Ecmp = devObjPtr->limitedResources.l2Ecmp;
        GT_U32  numL2LttEcmp = devObjPtr->limitedResources.l2LttEcmp == 0 ?
                                numEPorts : /* according to the number of eports */
                                devObjPtr->limitedResources.l2LttEcmp;/* explicit value */
        GT_U32  index = 0;
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*ePort/EQ Ingress ePort Table                                 */
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00700000, 8192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqIngressEPort)}
                /*ePort/Tx Protection Switching Table                          */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00A00000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(txProtectionSwitchingTable)}
                /*ePort/ePort to LOC Mapping Table                             */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00A80000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(ePortToLocMappingTable)}
                /*ePort/Protection LOC RX Table                                */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00B00000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamProtectionLocStatusTable)}
                /*ePort/Protection LOC TX Table                                */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00B10000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamTxProtectionLocStatusTable)}
                /*ePort/E2PHY Table                                            */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00c00000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24, 4),SMEM_BIND_TABLE_MAC(eqE2Phy)}
            };

            /* eqIngressEPort  */
            chunksMem[index].numOfRegisters = (numEPorts / 4) * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;
            /* txProtectionSwitchingTable  */
            chunksMem[index].numOfRegisters = (numEPorts / 32)  * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;
            /* ePortToLocMappingTable  */
            chunksMem[index].numOfRegisters = (numEPorts / 2) * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;
            /* oamProtectionLocStatusTable */
            chunksMem[index].numOfRegisters = (numEPorts / 32) * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;
            /* oamTxProtectionLocStatusTable */
            chunksMem[index].numOfRegisters = (numEPorts / 32)  * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;
            /* eqE2Phy */
            chunksMem[index].numOfRegisters = (numEPorts)  * (chunksMem[index].enrtyNumBytesAlignement / 4);
            index++;

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*L2 ECMP/L2ECMP Table                                         */
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00900000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(23, 4),SMEM_BIND_TABLE_MAC(eqL2Ecmp)}
                /*L2 ECMP/ePort LTT                                            */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00800000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqL2EcmpLtt)}
            };

            /* number of entries : l2Ecmp . keep alignment and use for memory size */
            /*eqL2Ecmp*/
            chunksMem[0].numOfRegisters = numL2Ecmp    * (chunksMem[0].enrtyNumBytesAlignement / 4);
            /*eqL2EcmpLtt*/
            chunksMem[1].numOfRegisters = numL2LttEcmp * (chunksMem[1].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

    }


    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
#if 0 /* Ingress STC interrupts removed */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000620 ,0)}, FORMULA_TWO_PARAMETERS(2,0x4, 32,0x10)}
#endif
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010020 ,0)}, FORMULA_TWO_PARAMETERS(2,0x4, 16,0x10)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
             {DUMMY_NAME_PTR_CNS,         0x0000001c,         0x00000498,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00000074,         0x0000003f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005000,         0x00000004,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005014,         0x00002200,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005020,         0x00000020,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005024,         0xffffffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005028,         0x000001e0,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000502c,         0x0001ffff,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000503c,         0x00002190,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005044,         0x0007ffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005050,         0x00000002,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005058,         0x0000003f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00005060,         0x000001e0,     16,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,         0x00006008,         0x00000002,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a000,         0x3ef084e2,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a004,         0x000001f2,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a010,         0x00000160,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a020,         0x000007e0,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a100,         0xffffffff,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af00,         0x1084211f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af04,         0x00004201,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af08,         0x00000011,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af10,         0x0000421f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af30,         0x0000ffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af34,         0x000002C0,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b000,         0x00004e00,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b00c,         0x00000001,      7,    0x4,      2,    0x40}
            ,{DUMMY_NAME_PTR_CNS,         0x0000b200,         0x02000000,      7,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d000,         0x00000020,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d004,         0xffffffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d010,         0x000001e0,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,         0x00020010,         0x0000003f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,         0x00080000,         0x0000ffff,    255,    0x4   }

            ,{DUMMY_NAME_PTR_CNS,         0x00100000,         0xffffffff,    256,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x00130000,         0x0fffffff,    256,    0x4  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/*******************************************************************************
*   smemHawkFindMemoryInTcamUnit
*
* DESCRIPTION:
*       Hawk - find the memory in the TCAM unit
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       unitChunksPtr - pointer to the unit chunk
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter --> used as pointer to the memory unit chunk
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemHawkFindMemoryInTcamUnit
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    GT_U32  addrOffset;
    GT_U32  lineIndex;
    GT_U32  newAddrOffset;
    GT_U32  offsetInLine;
    GT_U32  actionTableBaseAddr = devObjPtr->tablesInfo.globalActionTable.commonInfo.baseAddress;
    GT_U32  actionTableRange    = 0x00200000;/* reserved address space for this table */

    if(!IS_SKERNEL_OPERATION_MAC(accessType) && /* Relevant for CPU access only */
       address >= actionTableBaseAddr &&/* range of the TCAM Action table */
       address <  (actionTableBaseAddr + actionTableRange))
    {
        addrOffset = address - actionTableBaseAddr;/* relative offset to the action table */

        /* every line is 256 bits = 32 bytes */
        lineIndex    = addrOffset >> 5;
        offsetInLine = addrOffset & 0x1f;
        /* action table - consider single line of action for every 2 lines of action */
        /* see devObjPtr->tablesInfo.globalActionTable.paramInfo[0].divider */
        newAddrOffset = ((lineIndex >> 1) << 5) + offsetInLine;

        /* update the address */
        address += newAddrOffset - addrOffset;
    }

    return smemDevFindInUnitChunk(devObjPtr,accessType,address,memSize,param);
}

/*******************************************************************************
*   smemHawkBindFindMemoryFunc
*
* DESCRIPTION:
*       Hawk - bind special 'find memory' functions
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
void smemHawkBindFindMemoryFunc(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    GT_U32  unitIndex;

    /* get the unitIndex from the name of the unit */
    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TCAM);
    /* bind the TCAM unit to dedicated find function */
    devMemInfoPtr->common.specFunTbl[unitIndex].specFun    = smemHawkFindMemoryInTcamUnit;

    /* action table - consider single line of action for every 2 lines of action */
    devObjPtr->tablesInfo.globalActionTable.paramInfo[0].divider = 2;
}


/**
* @internal smemHawkUnitTcam function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TCAM unit
*/
static void smemHawkUnitTcam
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    GT_U32  tcamNumOfFloors = devObjPtr->tcamNumOfFloors;
    GT_U32  num10bRulesForAddressSpace  = tcamNumOfFloors * (4 * _1K);/* 4 and not 3 , because it also hold the 'jump over index 12..15' */

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* SIP5 Tcam management registers */
    {0x00502000, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteTcamMgLookup,0},

    /* next 4 lines catch addresses of : 0x005021BC..0x00502208 (20 registers)
       those are 'read only' registers.  */
    /*exact match - 0x005021BC */
    {0x005021BC, SMEM_FULL_MASK_CNS, NULL, 0, smemLion3ActiveWriteTcamMgHitGroupHitNum,0},
    /* match - 0x005021C0.. 0x005021FC , so need to ignore 5021E0 , 5021F4 */
    {0x005021C0, 0xFFFFFFC0, NULL, 0, smemLion3ActiveWriteTcamMgHitGroupHitNum,0},
    /* match - 0x00502200.. 0x0050221C , so need to ignore 502208, 50221C */
    {0x00502200, 0xFFFFFFE0, NULL,0 , smemLion3ActiveWriteTcamMgHitGroupHitNum,0},
    /* TCAMInterruptCause register */
    {0x0050100c, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00501008, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /* 16 floors : 128K lines : 64K - X lines , 64K - Y lines */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2097152),
              SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(84, 16),SMEM_BIND_TABLE_MAC(tcamMemory)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 2097152/2),/* actual size is half of the 'address range' accessed by CPU */
              SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(globalActionTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500000, 0x00500010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00501000, 0x00501010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00502000, 0x00502050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x005021A8, 0x005021A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00502308, 0x00502310)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00503000, 0x00503030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050303C, 0x0050303C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00504010, 0x00504014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00505000, 0x0050504C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00506000, 0x00506024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050606C, 0x0050606C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0050607C, 0x00506080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00507000, 0x00507008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00507010, 0x00507010)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        /* support Falcon , Hawk , Phoenix */
        chunksMem[0].numOfRegisters = 2/*X,Y*/ * num10bRulesForAddressSpace * (chunksMem[0].enrtyNumBytesAlignement / 4);
        chunksMem[1].numOfRegisters = 1        * num10bRulesForAddressSpace * (chunksMem[1].enrtyNumBytesAlignement / 4);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* TCAM Hit Number and Group Select Floor %n : 0x00500100 + n*0x8: where n (0-15) represents Floor num */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00500100 ,0)}, FORMULA_SINGLE_PARAMETER(16,0x8)}
             /*MG Hit Group %n Hit Number %i .0x005021BC + i*0x4+n*0x14: where n (0-4) represents TCAM Group Num, where i (0-3) represents Hit Number*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x005021BC, 0)}, FORMULA_TWO_PARAMETERS(4,0x4,5,0x14)}
            /*Exact Match Port Mapping %n. 0x00507080 + 4*n: where n (0-1) represents Pipe */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00507080, 0)}, FORMULA_SINGLE_PARAMETER(2,0x4)}
            /*Exact Match Profile Table %n Control. 0x00507088 + 32*4*(n-1): where n (1-15) represents Entry*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00507088, 0)}, FORMULA_SINGLE_PARAMETER(15,(32*0x4))}
            /*Exact Match Profile Table %n Mask Data %i.0x0050708C + 4*i+32*4*(n-1): where i (0-11) represents WORD, where n (1-15) represents Entry*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x0050708C, 0)}, FORMULA_TWO_PARAMETERS(12,0x4,15,(32*0x4))}
            /*Exact Match Profile Table %n Default action Data %i.0x005070C0 + 4*i+32*4*(n-1): where i (0-7) represents WORD, where n (1-15) represents Entry*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x005070C0, 0)}, FORMULA_TWO_PARAMETERS(12,0x4,15,(32*0x4))}
            /*Exact Match Action Assignment Type %n Byte %i.0x00508000 + 4*i+32*4*n: where i (0-31) represents Byte, where n (0-15) represents Type*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00508000, 0)}, FORMULA_TWO_PARAMETERS(32,0x4,16,(32*0x4))}
            /*SubKey0 Maping %n.0x00509000 + 4*4*(n-1): where n (1-63) represents Profile*/
            /*SubKey%i Maping %n.0x00509004 + 4*4*(n-1) + 4*(i-1): where n (1-63) represents Entry, where i (1-3) represents SubKey*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00509000, 0),
                    SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),
                    SMEM_BIND_TABLE_MAC(tcamProfileSubkeySizeAndMux)},
                FORMULA_TWO_PARAMETERS(63,(4*0x4),4,0x4)}
            /*Mux Configuration Table Entry %n 10B %i.0x00509800 + 4*i+6*4*(n-1): where i (0-5) represents 10B, where n (1-47) represents 60B line*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00509800, 0),
                    SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),
                    SMEM_BIND_TABLE_MAC(tcamSubkeyMux2byteUnits)},
                FORMULA_TWO_PARAMETERS(47,(6*0x4),6,0x4)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        chunksMem[0].formulaCellArr[0].size = tcamNumOfFloors;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* init the X,Y of the tcam */
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x0000000f,  (128*_1K)/2,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0x0000000f,  (128*_1K)/2,    0x20     }

            ,{DUMMY_NAME_PTR_CNS,            0x00500000,         0x0000001F,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00503000,         0x00000043,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00503008,         0x00001fef,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x0050300c,         0x0000000f,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00503014,         0x0001fff7,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00503018,         0x00000001,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x0050301c,         0xffffffff,      2,    0x4,      2,    0xc}
            ,{DUMMY_NAME_PTR_CNS,            0x00503024,         0x000fffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00503030,         0x000fffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00504010,         0x00000006,      1,    0x0   } /* numOfFloors */
            ,{DUMMY_NAME_PTR_CNS,            0x00504014,         0x00000002,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00505004,         0x00000f3f,     10,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00506000,         0x00000010,     10,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0050606c,         0x000fffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00507000,         0x00010302,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00507004,         0x00000202,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00507008,         0x00000008,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            0x00507010,         0x0000ffff,      1,    0x0   }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

        myUnit_registersDefaultValueArr[0].numOfRepetitions = num10bRulesForAddressSpace;
        myUnit_registersDefaultValueArr[1].numOfRepetitions = num10bRulesForAddressSpace;

        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitShm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Falcon SHM unit
*
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemHawkUnitShm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    /*empty */
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E0, 0x000000E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000F0, 0x00000104)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x000000a0,         0x00101111,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0xFFFF0000,      1,    0x0 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit SHM */
        {/*00000000+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 39 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.block_port_select[n] =
                    0x00000000+n*0x4;
            }/* end of loop n */
        }/*00000000+n*0x4*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.shm_engines_size_mode = 0x000000a0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.bad_address_latch = 0x000000d4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.shm_error_sum_cause = 0x000000e0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.shm_error_sum_mask = 0x000000e4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.lpm_illegal_address = 0x000000f0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.fdb_illegal_address = 0x000000f4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.em_illegal_address = 0x000000f8;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.arp_illegal_address = 0x000000fc;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SHM.lpm_aging_illegal_address = 0x00000100;

    }/*end of unit SHM */
}


/**
* @internal smemHawkUnitIpvx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the ipvx unit
*/
static void smemHawkUnitIpvx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000028)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x0000026C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000278, 0x00000294)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000360, 0x00000364)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000380)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000924)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000940, 0x00000948)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x00000954)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000968, 0x00000978)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000980, 0x00000988)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A80, 0x00000A88)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B80, 0x00000B88)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C80, 0x00000C88)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E2C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000F00, 0x00000F10)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000F20, 0x00000F20)}

        /* Router QoS Profile Offsets Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ipvxQoSProfileOffsets)}
        /* Router Acces Matrix Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 3072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(ipvxAccessMatrix)}
        /* Router EPort Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(112, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEPort)}
        /* Router ECMP pointer Table : 4 'pointers' per line */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 32768 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(60, 8),SMEM_BIND_TABLE_MAC(ipvxEcmpPointer)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Router Next Hop Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00400000, 262144),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(106, 16),SMEM_BIND_TABLE_MAC(ipvxNextHop)}
            /* Router Next Hop Table Age Bits */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000,   2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(routeNextHopAgeBits)}
            /* Router EVlan Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00100000, 131072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(66, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEVlan)}
            /* Router ECMP table : 2 entries per line */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00250000,  49152),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(58,  8),SMEM_BIND_TABLE_MAC(ipvxEcmp)}
        };
        GT_U32  numNextHop = devObjPtr->limitedResources.nextHop;/* support Hawk and Phoenix */
        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  ipvxEcmp  = devObjPtr->limitedResources.ipvxEcmp;
        GT_U32  index = 0;
        /* number of entries : numNextHop . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters = numNextHop * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
        /* number of entries : (numNextHop/32) . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters = (numNextHop/32) * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
        /* number of entries : (numEVlans) . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters = numEVlans * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;
        /* number of entries : (ipvxEcmp) . keep alignment and use for memory size */
        chunksMem[index].numOfRegisters = ipvxEcmp * (chunksMem[index].enrtyNumBytesAlignement / 4);
        index++;

        smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
            ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Router Global Control0                                     */
             {DUMMY_NAME_PTR_CNS,           0x00000000 ,0x20000047               }
            /* Router MTU Configuration Register                          */
            ,{DUMMY_NAME_PTR_CNS,           0x00000004 ,0x017705DC ,4 ,0x4       }
            /* Router Global Control1                                     */
            ,{DUMMY_NAME_PTR_CNS,           0x00000020 ,0x0380001C            }
             /* Router Global Control2                                     */
            ,{DUMMY_NAME_PTR_CNS,           0x00000024 ,0x00000FFF               }
             /* IPv4 Control Register0; IPv6 Control Register0             */
            ,{DUMMY_NAME_PTR_CNS,           0x00000100 ,0x1B79B01B ,2 ,0x100     }
             /* IPv4 Control Register1                                     */
            ,{DUMMY_NAME_PTR_CNS,           0x00000104 ,0x001B665B               }
             /* IPv6 Control Register1                                     */
            ,{DUMMY_NAME_PTR_CNS,           0x00000204 ,0x0000661B               }
             /* IPv6 Unicast Scope Prefix 0                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000250 ,0xFFC0FE80               }
             /* IPv6 Unicast Scope Prefix 1                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000254 ,0xFE00FC00               }
             /* IPv6 Unicast Scope Level 1                                 */
            ,{DUMMY_NAME_PTR_CNS,           0x00000264 ,0x00000001               }
             /* IPv6 Unicast Scope Level 2; IPv6 Unicast Scope Level 3     */
            ,{DUMMY_NAME_PTR_CNS,           0x00000268 ,0x00000003 ,2,0x4         }
             /* Routing ECMP Configurations                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000360 ,0x00006140               }
             /* Routing ECMP Seed                                          */
            ,{DUMMY_NAME_PTR_CNS,           0x00000364 ,0xFFFFFFFF               }
             /* Router FCoE Global Configuration                           */
            ,{DUMMY_NAME_PTR_CNS,           0x00000380 ,0x001B9360               }
             /* Router Metal Fix                                           */
            ,{DUMMY_NAME_PTR_CNS,           0x00000968 ,0x0000FFFF               }
             /* Router FIFOs threshold 1                                   */
            ,{DUMMY_NAME_PTR_CNS,           0x00000978 ,0x000FFFFF               }
             /* CPU Codes 0                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E00 ,0x88878685               }
             /* CPU Codes 1                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E04 ,0x8C8B8A89               }
             /* CPU Codes 2                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E08 ,0x9F8F8E8D               }
             /* CPU Codes 3                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E0C ,0xA3A2A1A0               }
             /* CPU Codes 4                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E10 ,0xA7A6A5A4               }
             /* CPU Codes 5                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E14 ,0xABAAA9A8               }
             /* CPU Codes 6                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E18 ,0xAFAEADAC               }
             /* CPU Codes 7                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E1C ,0x90B6B1B0               }
             /* CPU Codes 8                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E20 ,0x91B5B4B3               }
             /* CPU Codes 9                                                */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E24 ,0xCBCAC9C8               }
             /* CPU Codes 10                                               */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E28 ,0x9392CDCC               }
             /* CPU Codes 11                                               */
            ,{DUMMY_NAME_PTR_CNS,           0x00000E2C ,0x00000094               }
             /* FDB IPv4 Route Lookup Mask                                 */
            ,{DUMMY_NAME_PTR_CNS,           0x00000F00 ,0xFFFFFFFF              }
             /* FDB IPv6 Route Lookup Mask                                 */
            ,{DUMMY_NAME_PTR_CNS,           0x00000F04 ,0xFFFFFFFF ,4,0x4        }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitEpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitEpcl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000074, 0x00000074)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x0000015C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000164, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000214)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000254)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000041C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(epclTargetPhysicalPortMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00013000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(epclSourcePhysicalPortMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 4*5120),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(31, 4),SMEM_BIND_TABLE_MAC(epclConfigTable)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 6144),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(364, 64),SMEM_BIND_TABLE_MAC(epclUdbSelect)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00014000, 8192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(epclPortLatencyMonitoring)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00016000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(epclExactMatchProfileIdMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000, 512) ,SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(9, 4), SMEM_BIND_TABLE_MAC(queueGroupLatencyProfileConfigTable)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x01780115,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00FF0080,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000808,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000074,         0x76543210,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000A0,         0x000000ff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000C0,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0xFFFFFFFF,      5,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000220,         0x00000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000260,         0x0000FFFF,      4,    0x4      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {
        {/* start of queue offset config */
            GT_U32 i;

            /* 0x00000400 + 0x4*i */
            for(i = 0; i <= 7; i++)
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.queueOffsetLatencyProfileConfig[i] = 0x00000400 + 0x4*i;
        }/* end of queue offset config*/
    }
}

/**
* @internal smemHawkUnitHbu function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitHbu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000060)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemHawkSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHawkSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_PHA)         ,smemHawkUnitPha}
        ,{STR(UNIT_PPU)         ,smemHawkUnitPpu}
        ,{STR(UNIT_EGF_SHT)     ,smemHawkUnitEgfSht}
        ,{STR(UNIT_IPCL)        ,smemHawkUnitIpcl}
        ,{STR(UNIT_TTI)         ,smemHawkUnitTti }
        ,{STR(UNIT_HA)          ,smemHawkUnitHa  }
        ,{STR(UNIT_EQ)          ,smemHawkUnitEq  }
        ,{STR(UNIT_EM)          ,smemHawkUnitEm  }
        ,{STR(UNIT_TCAM)        ,smemHawkUnitTcam}
        ,{STR(UNIT_SHM)         ,smemHawkUnitShm}
        ,{STR(UNIT_IPVX)        ,smemHawkUnitIpvx}
        ,{STR(UNIT_EPCL)        ,smemHawkUnitEpcl}
        ,{STR(UNIT_IPLR)        ,smemHawkUnitIplr0}
        ,{STR(UNIT_IPLR1)       ,smemHawkUnitIplr1}
        ,{STR(UNIT_EPLR)        ,smemHawkUnitEplr}
        ,{STR(UNIT_TAI)         ,smemHawkUnitInitTai}
        ,{STR(UNIT_HBU)         ,smemHawkUnitHbu}

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    smemFalconSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}



/**
* @internal smemHawkActiveReadLatencyProfileStatTable function
* @endinternal
*
* @brief   Function to read latency profile table.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - pointer to the register's memory in the simulation.
* @param[in] sumBit                   - global summary interrupt bit
*
* @param[out] outMemPtr               - Pointer to the memory to copy register's content.
*/
void smemHawkActiveReadLatencyProfileStatTable
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32                  address,
    IN         GT_U32                  memSize,
    IN         GT_U32                * memPtr,
    IN         GT_UINTPTR              sumBit,
    OUT        GT_U32 *                outMemPtr
)
{
    GT_U32  regAddr;
    GT_U32  lmuNum;
    GT_U32  index;


    /* Calculate LMU and index */
    for(lmuNum = 0;lmuNum < devObjPtr->numOfLmus;lmuNum++)
    {
        if((address & 0xFFF00000) == devObjPtr->memUnitBaseAddrInfo.lmu[lmuNum])
        {
            break;
        }
    }

    index = (address >> 5) & 0x1FF;

    regAddr = SMEM_SIP6_LATENCY_MONITORING_STAT_TBL_MEM(devObjPtr, 0, lmuNum, index);

    memPtr = smemMemGet(devObjPtr, regAddr);

    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 0),
        0, 32, snetFieldValueGet(memPtr, 0, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 1),
        0, 32, snetFieldValueGet(memPtr, 32, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 2),
        0, 32, snetFieldValueGet(memPtr, 64, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 3),
        0, 32, snetFieldValueGet(memPtr, 96, 30));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 4),
        0, 32, snetFieldValueGet(memPtr, 126, 30));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, 0, lmuNum, 5),
        0, 32, snetFieldValueGet(memPtr, 156, 30));
}

/**
* @internal smemHawkUnitLmu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LMU unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitLmu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* Read only latency statistics table */
        {0x00000000, 0xFFFFC000, smemHawkActiveReadLatencyProfileStatTable, 0, NULL, 0},
        /* Read Write configuration memory */
        {0x00004000, 0xFFFFE000, NULL, 0, NULL, 0},
        /* read write registers 8000, 8010, 8020, 8024 */
        {0x00008000, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0},
        {0x00008010, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0},
        {0x00008020, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0},
        {0x00008024, SMEM_FULL_MASK_CNS, NULL, 0, NULL, 0},
        /* LMU Profile Statistics Read data 1-6 */
        {0x00008030, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00008034, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00008038, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x0000803C, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00008040, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00008044, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        /* LMU lmu_latency_over_threshold_n_cause lmu/latency_over_threshold_n_mask */
        {0x00008100, 0x00FFFF80, smemChtActiveReadIntrCauseReg, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00008180, 0x00FFFF80, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_latency_fifo_full_n_cause/lmu_latency_fifo_full_n_mask */
        {0x00008200, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00008210, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_global_interrupt_cause/lmu_global_interrupt_mask */
        {0x00008220, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00008224, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_cause_summary/lmu_mask_summary*/
        {0x00008230, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg,                    0},
        {0x00008234, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},
        {0x00008238, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg,         0},
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* chunks without formulas */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers address space */
            {
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000, 16384),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(186, 32),
               SMEM_BIND_TABLE_MAC(lmuStatTable)
            }
           ,{
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00004000, 8192),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(90, 16),
               SMEM_BIND_TABLE_MAC(lmuCfgTable)
            }
            /*Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000, 0x00008000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008010, 0x00008010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008020, 0x00008024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008030, 0x00008044)}
            /*interrupts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008100, 0x00008140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008180, 0x000081C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008200, 0x00008200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008210, 0x00008210)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008220, 0x00008224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008230, 0x00008238)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00008020,         0x00000005,    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0xF0000000,    512,    0x00020}
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x03FFFFFF,    512,    0x00020}
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0xFFFFFFFF,    512,    0x00010}
            ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0x0FFFFFFF,    512,    0x00010}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal regAddr_lmu function
* @endinternal
*
* @brief   initialize the register DB - LMU
*
* @param[in] devObjPtr - pointer to device object.
*            unitIndex - unit index
*
*/
static void regAddr_lmu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 unitIndex     /* the unit index 0..3 */
)
{
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.lmu[unitIndex];
    GT_U32    k;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_lmu : non-valid unit[%d]",unitIndex);
        return;
    }

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].averageCoefficient           = 0x00008020 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[0] = 0x00008030 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[1] = 0x00008034 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[2] = 0x00008038 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[3] = 0x0000803C + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[4] = 0x00008040 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].profileStatisticsReadData[5] = 0x00008044 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].summary_cause                = 0x00008230 + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].summary_mask                 = 0x00008234 + unitOffset;
    for(k = 0; k < 17; k++)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].latencyOverThreshold_cause[k] = 0x00008100 + 0x4 * k + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].latencyOverThreshold_mask[k]  = 0x00008180 + 0x4 * k + unitOffset;
    }
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[unitIndex].channelEnable = 0x00008000 + unitOffset;
}

/**
* @internal regAddr_mif function
* @endinternal
*
* @brief   initialize the register DB - MIF
*
* @param[in] devObjPtr - pointer to device object.
*            unitIndex - unit index
*
*/
static void regAddr_mif
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,      /* global port in the device */
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,    /* local  port in the unit */
    IN GT_U32 unitIndex     /* the unit index 0..6 */
)
{
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.mif[unitIndex];
    GT_U32  isCpuPort = 0;

    SMEM_CHT_PORT_MTI_PORT_MIF_REG_STC  *portDbPtr;
    SMEM_CHT_PORT_MTI_UNIT_MIF_REG_STC  *unitDbPtr;
    ENHANCED_PORT_INFO_STC portInfo;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_mif : non-valid unit[%d]",unitIndex);
        return;
    }

    if(GT_OK == devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E,portNum,&portInfo))
    {
        isCpuPort = 1;
    }

    if(isCpuPort)
    {
        portDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[portIndex/*0*/].MIF_CPU;
        unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex/*0*/].MIF_CPU_global;
    }
    else
    {

        portDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MIF;
        unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MIF_global;
    }

    if(unitIndex >= 4)
    {
        /* 48 USX MAC ports and CPU MAC port */
        portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E  ] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000000 + portIndex*0x4;
        portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ] = unitOffset + MIF_TX_OFFSET     + 0x00000710 + portIndex*0x4;
    }
    else
    {
        GT_U32  portAlsoMac0InUsu;

        /* 32 400G MAC ports*/
        portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E ] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000080 + portIndex*0x4;
        portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E] = unitOffset + MIF_TX_OFFSET     + 0x00000790 + portIndex*0x4;
        if(portIndex == 0 || portIndex == 4)
        {
            /* support segmented ports */
            portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_128_E] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000100 + (portIndex/4)*0x4;
            portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_128_E] = unitOffset + MIF_TX_OFFSET    + 0x000007D0 + (portIndex/4)*0x4;
        }

        portAlsoMac0InUsu  = (unitIndex >= 2) || (portIndex <= 1) ? 0: 1;

        if(portAlsoMac0InUsu)
        {
            GT_U32  patchPortIndex = (portIndex - 2)*4;
            GT_U32  patchUnitOffset = devObjPtr->memUnitBaseAddrInfo.mif[4 + unitIndex];

            if(patchUnitOffset == 0)
            {
                /* support Harrier */
            }
            else
            {
                /* patch for port index 2 that can also be USX index 0 */
                portDbPtr->mif_channel_mapping_register [SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ] = patchUnitOffset + MIF_GLOBAL_OFFSET + 0x00000000 + patchPortIndex * 0x4;
                portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ] = patchUnitOffset + MIF_TX_OFFSET     + 0x00000710 + patchPortIndex * 0x4;
                /*portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E] = patchUnitOffset + MIF_TX_OFFSET     + 0x00000790 + patchPortIndex * 0x4;*/
            }
        }

        {

            GT_U32  newPortIndex = portIndex + 8; /* the extra mif indexes for preemption are 8..15 */

            portDbPtr->preemption.mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E ] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000080 + newPortIndex*0x4;
            portDbPtr->preemption.mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E] = unitOffset + MIF_TX_OFFSET     + 0x00000790 + newPortIndex*0x4;
        }
    }

    if(portIndex == 0 ||
      (portIndex == 1 && (unitIndex == 4 || unitIndex == 5) /*patch for USX */))
    {
        /* initialize single time per representative */

        unitDbPtr->mif_rx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E  ] = unitOffset + MIF_RX_OFFSET + 0x00000000;/*0x00000800*/
        unitDbPtr->mif_rx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E ] = unitOffset + MIF_RX_OFFSET + 0x00000004;/*0x00000804*/
        unitDbPtr->mif_rx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_128_E] = unitOffset + MIF_RX_OFFSET + 0x00000008;/*0x00000808*/

        unitDbPtr->mif_tx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E  ] = unitOffset + MIF_TX_OFFSET + 0x00000000;/*0x00001000*/
        unitDbPtr->mif_tx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E ] = unitOffset + MIF_TX_OFFSET + 0x00000004;/*0x00001004*/
        unitDbPtr->mif_tx_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_128_E] = unitOffset + MIF_TX_OFFSET + 0x00000008;/*0x00001008*/
    }
}

/**
* @internal smemHawkUnitTxqPdx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pdx unit
*/
static void smemHawkUnitTxqPdx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*Alligned to  /Cider/EBU-IP/TXQ_IP/SIP7.1 (Hawk)/TXQ_PR_IP/TXQ_PR_IP {7.1.3}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(txqPdxQueueGroupMap)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000200, 512)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 512)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000600, 512)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 512)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C0C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000103C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000122C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001310)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x00001414)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001420, 0x00001420)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001500)}
    };




    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PreemptionEnable*/
        {
            GT_U32  i;
            for(i = 0; i < 4; i++)
            {
                /*0x00000c00 + p*0x4: where pt (0-3) represents preemp_en_regs */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_PDX.PreemptionEnable[i] =
                   0x00000c00+4*i;
            }
        }
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00001200,         0x00000f94,      4,    0x4,      3,    0x10  }
            ,{DUMMY_NAME_PTR_CNS,           0x00001500,         0x0000ffff,      1,    0x0  }
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUnitTxqPfcc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pfcc unit
*/
static void smemHawkUnitTxqPfcc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
          /*Alligned to  /Cider/EBU-IP/TXQ_IP/SIP7.1 (Hawk)/TXQ_PR_IP/TXQ_PR_IP {7.1.3}*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000108)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000031C)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000444)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 592)}

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

           {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000126,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x00000008,         0x00924924,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x0000000c,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000400,         0x0000003f,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}



/**
* @internal smemHawkUnitTxqPsi function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq psi unit
*/
static void smemHawkUnitTxqPsi
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         /*PSI_REG*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000001B0)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000234)}
         /*PDQ*/
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000+HAWK_PSI_SCHED_OFFSET, 0x00000004+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008+HAWK_PSI_SCHED_OFFSET, 0x0000000C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010+HAWK_PSI_SCHED_OFFSET, 0x00000014+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000018+HAWK_PSI_SCHED_OFFSET, 0x0000001C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020+HAWK_PSI_SCHED_OFFSET, 0x00000024+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000028+HAWK_PSI_SCHED_OFFSET, 0x0000002C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030+HAWK_PSI_SCHED_OFFSET, 0x00000034+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000038+HAWK_PSI_SCHED_OFFSET, 0x0000003C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040+HAWK_PSI_SCHED_OFFSET, 0x00000044+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000048+HAWK_PSI_SCHED_OFFSET, 0x0000004C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050+HAWK_PSI_SCHED_OFFSET, 0x00000054+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800+HAWK_PSI_SCHED_OFFSET, 0x00000804+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000808+HAWK_PSI_SCHED_OFFSET, 0x0000080C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000810+HAWK_PSI_SCHED_OFFSET, 0x00000814+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000818+HAWK_PSI_SCHED_OFFSET, 0x0000081C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000A00+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005400+HAWK_PSI_SCHED_OFFSET, 0x00005404+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005408+HAWK_PSI_SCHED_OFFSET, 0x0000540C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005600+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00009800+HAWK_PSI_SCHED_OFFSET, 0x00009804+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00009808+HAWK_PSI_SCHED_OFFSET, 0x0000980C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009A00+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000AC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000BC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000C400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000CC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000DC00+HAWK_PSI_SCHED_OFFSET, 0x0000DC04+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000DC08+HAWK_PSI_SCHED_OFFSET, 0x0000DC0C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000DE00+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000F000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00011000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00013000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00019000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001B000+HAWK_PSI_SCHED_OFFSET, 0x0001B004+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001B008+HAWK_PSI_SCHED_OFFSET, 0x0001B00C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001B200+HAWK_PSI_SCHED_OFFSET, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001C000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00024000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0002C000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00034000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0003C000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044800+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00045400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00045C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00046400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00046C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00047400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00047C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00049400+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00049800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004A000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004A800+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004AC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004C000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004E000+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004F000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00054000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0005C000+HAWK_PSI_SCHED_OFFSET, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064000+HAWK_PSI_SCHED_OFFSET, 0x00064004+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064008+HAWK_PSI_SCHED_OFFSET, 0x0006400C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064010+HAWK_PSI_SCHED_OFFSET, 0x00064014+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00064400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00064C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00065400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00065C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00066400+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00066C00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067400+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067420+HAWK_PSI_SCHED_OFFSET, 0x00067424+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067428+HAWK_PSI_SCHED_OFFSET, 0x0006742C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067430+HAWK_PSI_SCHED_OFFSET, 0x00067434+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067500+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067600+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067700+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067720+HAWK_PSI_SCHED_OFFSET, 0x00067724+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00068000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00068800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00069000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00069800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006A000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006A800+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006A880+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006A980+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AA80+HAWK_PSI_SCHED_OFFSET, 0x0006AA84+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AA88+HAWK_PSI_SCHED_OFFSET, 0x0006AA8C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AA90+HAWK_PSI_SCHED_OFFSET, 0x0006AA94+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AAA0+HAWK_PSI_SCHED_OFFSET, 0x0006AAA4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AAB0+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006B400+HAWK_PSI_SCHED_OFFSET, 0x0006B404+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006B800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006C000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006C800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006D000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006D800+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006E000+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006E800+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006E880+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006E980+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EA80+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EAA0+HAWK_PSI_SCHED_OFFSET, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006EAC0+HAWK_PSI_SCHED_OFFSET, 0x0006EAC4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006EAC8+HAWK_PSI_SCHED_OFFSET, 0x0006EACC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006EAD0+HAWK_PSI_SCHED_OFFSET, 0x0006EAD4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006EAE0+HAWK_PSI_SCHED_OFFSET, 0x0006EAE4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006EAF0+HAWK_PSI_SCHED_OFFSET, 0x0006EAF4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EC00+HAWK_PSI_SCHED_OFFSET, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00072000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00074000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00076000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00078000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007A000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C000+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C200+HAWK_PSI_SCHED_OFFSET, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C600+HAWK_PSI_SCHED_OFFSET, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CA00+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CB00+HAWK_PSI_SCHED_OFFSET, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CC00+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CC40+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CC80+HAWK_PSI_SCHED_OFFSET, 0x0007CC84+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CC88+HAWK_PSI_SCHED_OFFSET, 0x0007CC8C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CC90+HAWK_PSI_SCHED_OFFSET, 0x0007CC94+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CCA0+HAWK_PSI_SCHED_OFFSET, 0x0007CCA4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CCB0+HAWK_PSI_SCHED_OFFSET, 0x0007CCB4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007D000+HAWK_PSI_SCHED_OFFSET, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00088000+HAWK_PSI_SCHED_OFFSET, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00088800+HAWK_PSI_SCHED_OFFSET, 2048)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00089800+HAWK_PSI_SCHED_OFFSET, 2048)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008A800+HAWK_PSI_SCHED_OFFSET, 256)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AA00+HAWK_PSI_SCHED_OFFSET, 256)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AC00+HAWK_PSI_SCHED_OFFSET, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AC80+HAWK_PSI_SCHED_OFFSET, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD00+HAWK_PSI_SCHED_OFFSET, 0x0008AD04+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD08+HAWK_PSI_SCHED_OFFSET, 0x0008AD0C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD20+HAWK_PSI_SCHED_OFFSET, 0x0008AD24+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD28+HAWK_PSI_SCHED_OFFSET, 0x0008AD2C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD30+HAWK_PSI_SCHED_OFFSET, 0x0008AD34+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AD38+HAWK_PSI_SCHED_OFFSET, 0x0008AD3C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008C000+HAWK_PSI_SCHED_OFFSET, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00094000+HAWK_PSI_SCHED_OFFSET, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094040+HAWK_PSI_SCHED_OFFSET, 0x00094044+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094048+HAWK_PSI_SCHED_OFFSET, 0x0009404C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094050+HAWK_PSI_SCHED_OFFSET, 0x00094054+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094058+HAWK_PSI_SCHED_OFFSET, 0x0009405C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094060+HAWK_PSI_SCHED_OFFSET, 0x00094064+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094068+HAWK_PSI_SCHED_OFFSET, 0x0009406C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094070+HAWK_PSI_SCHED_OFFSET, 0x00094074+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094078+HAWK_PSI_SCHED_OFFSET, 0x0009407C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094080+HAWK_PSI_SCHED_OFFSET, 0x00094084+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094088+HAWK_PSI_SCHED_OFFSET, 0x0009408C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094090+HAWK_PSI_SCHED_OFFSET, 0x00094094+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094098+HAWK_PSI_SCHED_OFFSET, 0x0009409C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940A0+HAWK_PSI_SCHED_OFFSET, 0x000940A4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940A8+HAWK_PSI_SCHED_OFFSET, 0x000940AC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940B0+HAWK_PSI_SCHED_OFFSET, 0x000940B4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940B8+HAWK_PSI_SCHED_OFFSET, 0x000940BC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940C0+HAWK_PSI_SCHED_OFFSET, 0x000940C4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940C8+HAWK_PSI_SCHED_OFFSET, 0x000940CC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940D0+HAWK_PSI_SCHED_OFFSET, 0x000940D4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940D8+HAWK_PSI_SCHED_OFFSET, 0x000940DC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940E0+HAWK_PSI_SCHED_OFFSET, 0x000940E4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940E8+HAWK_PSI_SCHED_OFFSET, 0x000940EC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940F0+HAWK_PSI_SCHED_OFFSET, 0x000940F4+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000940F8+HAWK_PSI_SCHED_OFFSET, 0x000940FC+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094100+HAWK_PSI_SCHED_OFFSET, 0x00094104+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094108+HAWK_PSI_SCHED_OFFSET, 0x0009410C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094110+HAWK_PSI_SCHED_OFFSET, 0x00094114+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094118+HAWK_PSI_SCHED_OFFSET, 0x0009411C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094120+HAWK_PSI_SCHED_OFFSET, 0x00094124+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094128+HAWK_PSI_SCHED_OFFSET, 0x0009412C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094130+HAWK_PSI_SCHED_OFFSET, 0x00094134+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094138+HAWK_PSI_SCHED_OFFSET, 0x0009413C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094140+HAWK_PSI_SCHED_OFFSET, 0x00094144+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094148+HAWK_PSI_SCHED_OFFSET, 0x0009414C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094150+HAWK_PSI_SCHED_OFFSET, 0x00094154+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094158+HAWK_PSI_SCHED_OFFSET, 0x0009415C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094160+HAWK_PSI_SCHED_OFFSET, 0x00094164+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094168+HAWK_PSI_SCHED_OFFSET, 0x0009416C+HAWK_PSI_SCHED_OFFSET)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094170+HAWK_PSI_SCHED_OFFSET, 0x00094174+HAWK_PSI_SCHED_OFFSET)}

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

   {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x0000ffff,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x00000200,         0x00000002,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x00001000,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            0x0000021c,         0x00000fff,      1,    0x0}

            /* registers of 64 bits ,remark at the begining mean that this is high word */
           ,{DUMMY_NAME_PTR_CNS,           0x00064000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00064008+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ScrubDis_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00064010+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      1,    0x8    }/*Plast_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00064014+HAWK_PSI_SCHED_OFFSET,         0x00ffffff,      1,    0x8    }/*Plast_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067420+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00067424+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067428+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PMemsEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067430+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PBnkEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067720+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PPerStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006aa80+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0006aa84+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006aa88+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMemsEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006aa90+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMyQEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006aaa0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CBnkEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006b400+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CPerStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006eac0+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0006eac4+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006eac8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMemsEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006ead0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMyQEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006eae0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BBnkEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006eaf0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BPerStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cc80+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0007cc84+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cc88+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMemsEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cc90+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMyQEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cca0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ABnkEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007ccb0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*APerStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QMemsEccErrStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad08+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0008ad0c+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad20+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QBnkEccErrStatus_StartAddr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad28+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank1EccErrStatus*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad30+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank2EccErrStatus*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ad38+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank3EccErrStatus*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrorStatus_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000008+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*FirstExcp_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000010+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrCnt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000018+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ExcpCnt_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000020+HAWK_PSI_SCHED_OFFSET,         0x00000037,      1,    0x8    }/*ExcpMask_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00000024+HAWK_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*ExcpMask_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000028+HAWK_PSI_SCHED_OFFSET,         0x00001033,      1,    0x8    }/*Identity_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000030+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ForceErr_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000038+HAWK_PSI_SCHED_OFFSET,         0x04081020,      1,    0x8    }/*ScrubSlots_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000003c+HAWK_PSI_SCHED_OFFSET,         0x00000004,      1,    0x8    }/*ScrubSlots_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000040+HAWK_PSI_SCHED_OFFSET,         0x00000007,      1,    0x8    }/*BPMsgFIFO_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00000044+HAWK_PSI_SCHED_OFFSET,         0x00000008,      1,    0x8    }/*BPMsgFIFO_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000048+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*TreeDeqEn_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000050+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PDWRREnReg*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000800+HAWK_PSI_SCHED_OFFSET,         0x00500000,      1,    0x8    }/*PPerCtlConf_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000808+HAWK_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*PPerRateShap_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000080c+HAWK_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*PPerRateShap_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000810+HAWK_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PortExtBPEn_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000818+HAWK_PSI_SCHED_OFFSET,         0x00001001,      1,    0x8    }/*PBytePerBurstLimit_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00005400+HAWK_PSI_SCHED_OFFSET,         0x071c0000,      1,    0x8    }/*CPerCtlConf_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00005408+HAWK_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*CPerRateShap_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000540c+HAWK_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*CPerRateShap_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00009800+HAWK_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*BPerCtlConf_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00009808+HAWK_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*BPerRateShap_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000980c+HAWK_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*BPerRateShap_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000dc00+HAWK_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*APerCtlConf_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000dc08+HAWK_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*APerRateShap_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000dc0c+HAWK_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*APerRateShap_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0001b000+HAWK_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*QPerCtlConf_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0001b008+HAWK_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*QPerRateShap_Addr*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0001b00c+HAWK_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*QPerRateShap_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094040+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccConfig_Addr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094048+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters0*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094050+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters1*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094058+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters2*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094060+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters3*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094068+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters4*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094070+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters5*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094078+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters6*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094080+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters7*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094088+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters8*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094090+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters9*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094098+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters10*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940a0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters11*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940a8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters12*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940b0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters13*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940b8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters14*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940c0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters15*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940c8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters16*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940d0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters17*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940d8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters18*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940e0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters19*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940e8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters20*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940f0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters21*/
           ,{DUMMY_NAME_PTR_CNS,           0x000940f8+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters22*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094100+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters23*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094108+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters24*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094110+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters25*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094118+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters26*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094120+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters27*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094128+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters28*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094130+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters29*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094138+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters30*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094140+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters31*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094148+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters32*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094150+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters33*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094158+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters34*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094160+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters35*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094168+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters36*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094170+HAWK_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters37*/
           ,{DUMMY_NAME_PTR_CNS,           0x00064400+HAWK_PSI_SCHED_OFFSET,         0x000001fe,      128,    0x8    }/*PortNodeState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00064c00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*PortMyQ*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067400+HAWK_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*PortWFS*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067500+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*PortBPFromSTF*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067600+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*PortBPFromQMgr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067800+HAWK_PSI_SCHED_OFFSET,         0x00000002,      128,    0x8    }/*ClvlNodeState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00068000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*ClvlMyQ*/
           ,{DUMMY_NAME_PTR_CNS,           0x00068800+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*ClvlRRDWRRStatus01*/
           ,{DUMMY_NAME_PTR_CNS,           0x00069000+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*ClvlRRDWRRStatus23*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006a800+HAWK_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*ClvlWFS*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006a880+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*ClvlL0ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006aab0+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*ClvlBPFromSTF*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006ac00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*TMtoTMClvlBPState*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006b800+HAWK_PSI_SCHED_OFFSET,         0x00000002,      128,    0x8    }/*BlvlNodeState*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006c000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*BlvlMyQ*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006c800+HAWK_PSI_SCHED_OFFSET,         0x0003ffff,      128,    0x8    }/*BlvlRRDWRRStatus01*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006d000+HAWK_PSI_SCHED_OFFSET,         0x0003ffff,      128,    0x8    }/*BlvlRRDWRRStatus23*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006e800+HAWK_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*BlvlWFS*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006e880+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006ea80+HAWK_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*BlvlL1ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006eaa0+HAWK_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*BlvlL1ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x00070000+HAWK_PSI_SCHED_OFFSET,         0x00000002,      512,    0x8    }/*AlvlNodeState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00072000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlMyQ*/
           ,{DUMMY_NAME_PTR_CNS,           0x00074000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlRRDWRRStatus01*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007c000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      16,    0x8    }/*AlvlWFS*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007c200+HAWK_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*AlvlL0ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007ca00+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL1ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cb00+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL1ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cc00+HAWK_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL2ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007cc40+HAWK_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL2ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x00080000+HAWK_PSI_SCHED_OFFSET,         0x00000002,      2048,    0x8    }/*QueueNodeState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00088000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*QueueWFS*/
           ,{DUMMY_NAME_PTR_CNS,           0x00088800+HAWK_PSI_SCHED_OFFSET,         0x00040000,      256,    0x8    }/*QueueL0ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008a800+HAWK_PSI_SCHED_OFFSET,         0x00040000,      32,    0x8    }/*QueueL1ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008aa00+HAWK_PSI_SCHED_OFFSET,         0x00040000,      32,    0x8    }/*QueueL1ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ac00+HAWK_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*QueueL2ClusterStateLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008ac80+HAWK_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*QueueL2ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x0008c000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2048,    0x8    }/*TMtoTMQueueBPState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00094000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*QueuePerStatus*/
           ,{DUMMY_NAME_PTR_CNS,           0x00000a00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*PortEligPrioFunc_Entry*/
           ,{DUMMY_NAME_PTR_CNS,           0x00001c00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*PortEligPrioFuncPtr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00002400+HAWK_PSI_SCHED_OFFSET,         0x0fff0fff,      128,    0x8    }/*PortTokenBucketTokenEnDiv*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00002404+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortTokenBucketTokenEnDiv*/
           ,{DUMMY_NAME_PTR_CNS,           0x00002c00+HAWK_PSI_SCHED_OFFSET,         0x0001ffff,      128,    0x8    }/*PortTokenBucketBurstSize*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00002c04+HAWK_PSI_SCHED_OFFSET,         0x0001ffff,      128,    0x8    }/*PortTokenBucketBurstSize*/
           ,{DUMMY_NAME_PTR_CNS,           0x00003400+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*PortDWRRPrioEn*/
           ,{DUMMY_NAME_PTR_CNS,           0x00003c00+HAWK_PSI_SCHED_OFFSET,         0x00100010,      128,    0x8    }/*PortQuantumsPriosLo*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00003c04+HAWK_PSI_SCHED_OFFSET,         0x00100010,      128,    0x8    }/*PortQuantumsPriosLo*/
           ,{DUMMY_NAME_PTR_CNS,           0x00004400+HAWK_PSI_SCHED_OFFSET,         0x00100010,      128,    0x8    }/*PortQuantumsPriosHi*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00004404+HAWK_PSI_SCHED_OFFSET,         0x00100010,      128,    0x8    }/*PortQuantumsPriosHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x00004c00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*PortRangeMap*/
           ,{DUMMY_NAME_PTR_CNS,           0x00005600+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*ClvlEligPrioFunc_Entry*/
           ,{DUMMY_NAME_PTR_CNS,           0x00006800+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*ClvlEligPrioFuncPtr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00007000+HAWK_PSI_SCHED_OFFSET,         0x0fff0fff,      128,    0x8    }/*ClvlTokenBucketTokenEnDiv*/
           ,{DUMMY_NAME_PTR_CNS,           0x00007800+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      128,    0x8    }/*ClvlTokenBucketBurstSize*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00007804+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      128,    0x8    }/*ClvlTokenBucketBurstSize*/
           ,{DUMMY_NAME_PTR_CNS,           0x00008000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*ClvlDWRRPrioEn*/
           ,{DUMMY_NAME_PTR_CNS,           0x00008800+HAWK_PSI_SCHED_OFFSET,         0x00000040,      128,    0x8    }/*ClvlQuantum*/
           ,{DUMMY_NAME_PTR_CNS,           0x00009000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*ClvltoPortAndBlvlRangeMap*/
           ,{DUMMY_NAME_PTR_CNS,           0x00009a00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*BlvlEligPrioFunc_Entry*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000ac00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*BlvlEligPrioFuncPtr*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000b400+HAWK_PSI_SCHED_OFFSET,         0x0fff0fff,      128,    0x8    }/*BlvlTokenBucketTokenEnDiv*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000bc00+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      128,    0x8    }/*BlvlTokenBucketBurstSize*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0000bc04+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      128,    0x8    }/*BlvlTokenBucketBurstSize*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000c400+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*BlvlDWRRPrioEn*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000cc00+HAWK_PSI_SCHED_OFFSET,         0x00000040,      128,    0x8    }/*BlvlQuantum*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000d400+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*BLvltoClvlAndAlvlRangeMap*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000de00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlEligPrioFunc_Entry*/
           ,{DUMMY_NAME_PTR_CNS,           0x0000f000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlEligPrioFuncPtr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00011000+HAWK_PSI_SCHED_OFFSET,         0x0fff0fff,      512,    0x8    }/*AlvlTokenBucketTokenEnDiv*/
           ,{DUMMY_NAME_PTR_CNS,           0x00013000+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*AlvlTokenBucketBurstSize*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00013004+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*AlvlTokenBucketBurstSize*/
           ,{DUMMY_NAME_PTR_CNS,           0x00015000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlDWRRPrioEn*/
           ,{DUMMY_NAME_PTR_CNS,           0x00017000+HAWK_PSI_SCHED_OFFSET,         0x00000040,      512,    0x8    }/*AlvlQuantum*/
           ,{DUMMY_NAME_PTR_CNS,           0x00019000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*ALvltoBlvlAndQueueRangeMap*/
           ,{DUMMY_NAME_PTR_CNS,           0x0001b200+HAWK_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*QueueEligPrioFunc*/
           ,{DUMMY_NAME_PTR_CNS,           0x0001c000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2048,    0x8    }/*QueueEligPrioFuncPtr*/
           ,{DUMMY_NAME_PTR_CNS,           0x00024000+HAWK_PSI_SCHED_OFFSET,         0x0fff0fff,      2048,    0x8    }/*QueueTokenBucketTokenEnDiv*/
           ,{DUMMY_NAME_PTR_CNS,           0x0002c000+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      2048,    0x8    }/*QueueTokenBucketBurstSize*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0002c004+HAWK_PSI_SCHED_OFFSET,         0x00000fff,      2048,    0x8    }/*QueueTokenBucketBurstSize*/
           ,{DUMMY_NAME_PTR_CNS,           0x00034000+HAWK_PSI_SCHED_OFFSET,         0x00000040,      2048,    0x8    }/*QueueQuantum*/
           ,{DUMMY_NAME_PTR_CNS,           0x0003c000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2048,    0x8    }/*QueueAMap*/
           ,{DUMMY_NAME_PTR_CNS,           0x00044000+HAWK_PSI_SCHED_OFFSET,         0x07ffffff,      128,    0x8    }/*PortShpBucketLvls*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00044004+HAWK_PSI_SCHED_OFFSET,         0x07ffffff,      128,    0x8    }/*PortShpBucketLvls*/
           ,{DUMMY_NAME_PTR_CNS,           0x00048c00+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      128,    0x8    }/*ClvlShpBucketLvls*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00048c04+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      128,    0x8    }/*ClvlShpBucketLvls*/
           ,{DUMMY_NAME_PTR_CNS,           0x00049800+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*CLvlDef*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004a000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      128,    0x8    }/*BlvlShpBucketLvls*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0004a004+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      128,    0x8    }/*BlvlShpBucketLvls*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004ac00+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*BlvlDef*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004c000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlShpBucketLvls*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0004c004+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlShpBucketLvls*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004f000+HAWK_PSI_SCHED_OFFSET,         0x00000001,      512,    0x8    }/*AlvlDef*/
           ,{DUMMY_NAME_PTR_CNS,           0x00054000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      2048,    0x8    }/*QueueShpBucketLvls*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00054004+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      2048,    0x8    }/*QueueShpBucketLvls*/
           ,{DUMMY_NAME_PTR_CNS,           0x00060000+HAWK_PSI_SCHED_OFFSET,         0x00000001,      2048,    0x8    }/*QueueDef*/
           ,{DUMMY_NAME_PTR_CNS,           0x00044800+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*PortShaperBucketNeg*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00044804+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*PortShaperBucketNeg*/
           ,{DUMMY_NAME_PTR_CNS,           0x00049400+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*CLevelShaperBucketNeg*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x00049404+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*CLevelShaperBucketNeg*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004a800+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*BLevelShaperBucketNeg*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0004a804+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      4,    0x8    }/*BLevelShaperBucketNeg*/
           ,{DUMMY_NAME_PTR_CNS,           0x0004e000+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*ALevelShaperBucketNeg*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0004e004+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*ALevelShaperBucketNeg*/
           ,{DUMMY_NAME_PTR_CNS,           0x0005c000+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      64,    0x8    }/*QueueShaperBucketNeg*/
           /**/,{DUMMY_NAME_PTR_CNS,           0x0005c004+HAWK_PSI_SCHED_OFFSET,         0xffffffff,      64,    0x8    }/*QueueShaperBucketNeg*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006a000+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*ClvlRRDWRRStatus67*/
           ,{DUMMY_NAME_PTR_CNS,           0x00069800+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*ClvlRRDWRRStatus45*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006a980+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*ClvlL0ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006e000+HAWK_PSI_SCHED_OFFSET,         0x0003ffff,      128,    0x8    }/*BlvlRRDWRRStatus67*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006d800+HAWK_PSI_SCHED_OFFSET,         0x0003ffff,      128,    0x8    }/*BlvlRRDWRRStatus45*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006e980+HAWK_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007a000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlRRDWRRStatus67*/
           ,{DUMMY_NAME_PTR_CNS,           0x00078000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlRRDWRRStatus45*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007c600+HAWK_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*AlvlL0ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x00089800+HAWK_PSI_SCHED_OFFSET,         0x00040000,      256,    0x8    }/*QueueL0ClusterStateHi*/
           ,{DUMMY_NAME_PTR_CNS,           0x00067700+HAWK_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*TMtoTMPortBPState*/
           ,{DUMMY_NAME_PTR_CNS,           0x0006ec00+HAWK_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*TMtoTMBlvlBPState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00076000+HAWK_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*AlvlRRDWRRStatus23*/
           ,{DUMMY_NAME_PTR_CNS,           0x0007d000+HAWK_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*TMtoTMAlvlBPState*/
           ,{DUMMY_NAME_PTR_CNS,           0x00044c00+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio0*/
           ,{DUMMY_NAME_PTR_CNS,           0x00045400+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio1*/
           ,{DUMMY_NAME_PTR_CNS,           0x00045c00+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio2*/
           ,{DUMMY_NAME_PTR_CNS,           0x00046400+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio3*/
           ,{DUMMY_NAME_PTR_CNS,           0x00046c00+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio4*/
           ,{DUMMY_NAME_PTR_CNS,           0x00047400+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio5*/
           ,{DUMMY_NAME_PTR_CNS,           0x00047c00+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio6*/
           ,{DUMMY_NAME_PTR_CNS,           0x00048400+HAWK_PSI_SCHED_OFFSET,         0x00000001,      128,    0x8    }/*PortDefPrio7*/
           ,{DUMMY_NAME_PTR_CNS,           0x00065400+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*PortRRDWRRStatus01*/
           ,{DUMMY_NAME_PTR_CNS,           0x00065c00+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*PortRRDWRRStatus23*/
           ,{DUMMY_NAME_PTR_CNS,           0x00066400+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*PortRRDWRRStatus45*/
           ,{DUMMY_NAME_PTR_CNS,           0x00066c00+HAWK_PSI_SCHED_OFFSET,         0x00003fff,      128,    0x8    }/*PortRRDWRRStatus67*/

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUnitTxqPds function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
static void smemHawkUnitTxqPds
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 32640)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 35776)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 8944)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037000, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037800, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038800, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039000, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039800, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 2048)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041000, 0x00041030)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041040, 0x0004183C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00042000, 0x00042018)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00043000, 0x00043010)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00043020, 0x0004341C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000440F8, 0x00044118)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044120, 0x00044120)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044128, 0x00044128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044130, 0x00044130)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004413C, 0x0004413C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044144, 0x00044144)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004414C, 0x00044158)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044164, 0x00044164)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004416C, 0x0004416C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044174, 0x0004417C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044200, 0x00044204)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045000, 0x0004503C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045100, 0x0004513C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045200, 0x0004523C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045300, 0x0004533C)}
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
            ,{DUMMY_NAME_PTR_CNS,           0x00042004,         0x00000002,      1,    0x4    }/*PDS_Interrupt_Summary_Mask*/
            ,{DUMMY_NAME_PTR_CNS,           0x00043000,         0x0000001f,      1,    0x4    }/*Idle_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004410c,         0x000007ff,      1,    0x4    }/*Free_Next_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00044114,         0x00000fff,      1,    0x4    }/*Free_PID_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004417c,         0x000003ff,      1,    0x4    }/*FIFO_State_Latency_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045100,         0x0000002a,      16,    0x4    }/*Profile_Long_Queue_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045200,         0x00000026,      16,    0x4    }/*Profile_Head_Empty_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045000,         0x00000001,      16,    0x4    }/*Profile_Long_Queue_Enable*/
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}


/**
* @internal smemHawkUnitTxqSdq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq sdq unit
*/
static void smemHawkUnitTxqSdq
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {

         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000056C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x00000768)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000013FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 2048)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004068)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200, 0x00004268)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x00004468)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004600, 0x00004668)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004800, 0x00004868)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x000050D4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005600, 0x000056D4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300, 0x000063D4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007024)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007030, 0x00007030)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007200, 216)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007400, 216)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 2048)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000A000, 1024)}
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
            ,{DUMMY_NAME_PTR_CNS,           0x00004400,         0x00000007,      27,    0x4    }/*Port_Range_High*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004600,         0x00003a98,      27,    0x4    }/*Port_Back_Pressure_Low_Threshold*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004800,         0x00003a98,      27,    0x4    }/*Port_Back_Pressure_High_Threshold*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000007,      27,    0x4    }/*Port_Config*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000500,         0x00000003,      1,    0x4    }/*QCN_Config*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007000,         0x000201ff,      1,    0x4    }/*Sdq_Idle*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005604,         0x00000008,      2,    0x4    }/*Sel_List_Range_Low_1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005608,         0x00000010,      2,    0x4    }/*Sel_List_Range_Low_2*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000560c,         0x00000018,      2,    0x4    }/*Sel_List_Range_Low_3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005610,         0x00000020,      2,    0x4    }/*Sel_List_Range_Low_4*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005614,         0x00000028,      2,    0x4    }/*Sel_List_Range_Low_5*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005618,         0x00000030,      2,    0x4    }/*Sel_List_Range_Low_6*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000561c,         0x00000038,      2,    0x4    }/*Sel_List_Range_Low_7*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005620,         0x00000040,      2,    0x4    }/*Sel_List_Range_Low_8*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005624,         0x00000048,      2,    0x4    }/*Sel_List_Range_Low_9*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005628,         0x00000050,      2,    0x4    }/*Sel_List_Range_Low_10*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000562c,         0x00000058,      2,    0x4    }/*Sel_List_Range_Low_11*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005630,         0x00000060,      2,    0x4    }/*Sel_List_Range_Low_12*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005634,         0x00000068,      2,    0x4    }/*Sel_List_Range_Low_13*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005638,         0x00000070,      2,    0x4    }/*Sel_List_Range_Low_14*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000563c,         0x00000078,      2,    0x4    }/*Sel_List_Range_Low_15*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005640,         0x00000080,      2,    0x4    }/*Sel_List_Range_Low_16*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005644,         0x00000088,      2,    0x4    }/*Sel_List_Range_Low_17*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005648,         0x00000090,      2,    0x4    }/*Sel_List_Range_Low_18*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000564c,         0x00000098,      2,    0x4    }/*Sel_List_Range_Low_19*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005650,         0x000000a0,      2,    0x4    }/*Sel_List_Range_Low_20*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005654,         0x000000a8,      2,    0x4    }/*Sel_List_Range_Low_21*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005658,         0x000000b0,      2,    0x4    }/*Sel_List_Range_Low_22*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000565c,         0x000000b8,      2,    0x4    }/*Sel_List_Range_Low_23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005660,         0x000000c0,      2,    0x4    }/*Sel_List_Range_Low_24*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005664,         0x000000c8,      2,    0x4    }/*Sel_List_Range_Low_25*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005668,         0x000000d0,      2,    0x4    }/*Sel_List_Range_Low_26*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006300,         0x00000007,      2,    0x4    }/*Sel_List_Range_High_0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006304,         0x0000000f,      2,    0x4    }/*Sel_List_Range_High_1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006308,         0x00000017,      2,    0x4    }/*Sel_List_Range_High_2*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000630c,         0x0000001f,      2,    0x4    }/*Sel_List_Range_High_3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006310,         0x00000027,      2,    0x4    }/*Sel_List_Range_High_4*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006314,         0x0000002f,      2,    0x4    }/*Sel_List_Range_High_5*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006318,         0x00000037,      2,    0x4    }/*Sel_List_Range_High_6*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000631c,         0x0000003f,      2,    0x4    }/*Sel_List_Range_High_7*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006320,         0x00000047,      2,    0x4    }/*Sel_List_Range_High_8*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006324,         0x0000004f,      2,    0x4    }/*Sel_List_Range_High_9*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006328,         0x00000057,      2,    0x4    }/*Sel_List_Range_High_10*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000632c,         0x0000005f,      2,    0x4    }/*Sel_List_Range_High_11*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006330,         0x00000067,      2,    0x4    }/*Sel_List_Range_High_12*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006334,         0x0000006f,      2,    0x4    }/*Sel_List_Range_High_13*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006338,         0x00000077,      2,    0x4    }/*Sel_List_Range_High_14*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000633c,         0x0000007f,      2,    0x4    }/*Sel_List_Range_High_15*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006340,         0x00000087,      2,    0x4    }/*Sel_List_Range_High_16*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006344,         0x0000008f,      2,    0x4    }/*Sel_List_Range_High_17*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006348,         0x00000097,      2,    0x4    }/*Sel_List_Range_High_18*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000634c,         0x0000009f,      2,    0x4    }/*Sel_List_Range_High_19*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006350,         0x000000a7,      2,    0x4    }/*Sel_List_Range_High_20*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006354,         0x000000af,      2,    0x4    }/*Sel_List_Range_High_21*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006358,         0x000000b7,      2,    0x4    }/*Sel_List_Range_High_22*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000635c,         0x000000bf,      2,    0x4    }/*Sel_List_Range_High_23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006360,         0x000000c7,      2,    0x4    }/*Sel_List_Range_High_24*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006364,         0x000000cf,      2,    0x4    }/*Sel_List_Range_High_25*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006368,         0x000000d7,      2,    0x4    }/*Sel_List_Range_High_26*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005000,         0x00000001,      2,    0x4    }/*Sel_List_Enable_0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005004,         0x00000001,      2,    0x4    }/*Sel_List_Enable_1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005008,         0x00000001,      2,    0x4    }/*Sel_List_Enable_2*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000500c,         0x00000001,      2,    0x4    }/*Sel_List_Enable_3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005018,         0x00000001,      2,    0x4    }/*Sel_List_Enable_6*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005028,         0x00000001,      2,    0x4    }/*Sel_List_Enable_10*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005038,         0x00000001,      2,    0x4    }/*Sel_List_Enable_14*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005048,         0x00000001,      2,    0x4    }/*Sel_List_Enable_18*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005058,         0x00000001,      2,    0x4    }/*Sel_List_Enable_22*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005068,         0x00000001,      2,    0x4    }/*Sel_List_Enable_26*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002000,         0x60000000,      256,    0x8    }/*queue_cfg*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a000,         0x00000081,      256,    0x4    }/*queue_elig_state*/


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal regAddr_tsu function
* @endinternal
*
* @brief   initialize the register DB - PTP/TSU
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - port number
*
*/
static void regAddr_tsu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,      /* global port in the device */
    IN GT_U32 portIndex,    /* local  port in the unit */
    IN GT_U32 unitIndex     /* the unit index 0..3 */
)
{
    /* aligned to /Cider/EBU-IP/PCA/CTSU IP/CTSU 1.0/CTSU {1.0.6} */
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.tsu[unitIndex];

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_tsu : non-valid unit[%d]",unitIndex);
        return;
    }

    if(portIndex == 0)/* done by first port in unit */
    {
        GT_U32  index;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].PTPInterruptCause    = 0x00000080 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].PTPInterruptMask     = 0x00000084 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].PTPGeneralCtrl       = 0x00000000 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].totalPTPPktsCntr     = 0x00000008 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].NTPPTPOffsetLow      = 0x00000004 + unitOffset;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_0_15_InterruptSummaryCause     = 0x00003800 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_16_31_InterruptSummaryCause    = 0x00003804 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_0_15_InterruptSummaryMask      = 0x00003820 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_16_31_InterruptSummaryMask     = 0x00003824 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].ctsuGlobalInterruptSummaryCause        = 0x00000088 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].ctsuGlobalInterrupSummarytMask         = 0x0000008C + unitOffset;

        for(index=0; index<32; index++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].timestampFrameCntrControl[index] = 0x00000500 + 0x4 * index + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].timestampFrameCntr[index]        = 0x00000100 + 0x4 * index + unitOffset;

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channelInterruptCause[index]               = 0x00003000 + 0x4 * index + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channelInterruptMask[index]                = 0x00003400 + 0x4 * index + unitOffset;
        }
    }

    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].timestampingPrecisionControl = 0x00001400 + portIndex * 0x4 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue0Reg0 = 0x00004000 + portIndex * 0x8 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue0Reg1 = 0x00004004 + portIndex * 0x8 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue1Reg0 = 0x00004800 + portIndex * 0x8 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue1Reg1 = 0x00004804 + portIndex * 0x8 + unitOffset;
    }
}

/**
* @internal regAddr_400GMac function
* @endinternal
*
* @brief   initialize the register DB - 400G MAC
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_400GMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_200G_400G_OFFSET;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_400GMac : non-valid unit[%d]",unitIndex);
        return;
    }

    portFormula = (MAC_STEP_PORT_OFFSET * (portIndex / 4));

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.commandConfig  = 0x00000008 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.frmLength      = 0x00000014 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.macAddr0       = 0x0000000C + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.macAddr1       = 0x00000010 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.status         = 0x00000040 + portFormula + unitOffset;
}

/**
* @internal regAddr_400G_MTIP_EXT function
* @endinternal
*
* @brief   initialize the register DB - 400G MAC - MTIP_EXT
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            sip6_MTI_EXTERNAL_representativePortIndex - port represent global registers addresses per all ports in unit
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_400G_MTIP_EXT
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_EXT_BASE_OFFSET;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_400G_MTIP_EXT : non-valid unit[%d]",unitIndex);
        return;
    }

    if(portIndex == 0)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalChannelControl        = 0x00000008 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalFECControl            = 0x00000004 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalClockEnable           = 0x00000010 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalResetControl          = 0x00000014 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause = 0x0000004C + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryMask  = 0x00000050 + unitOffset;
    }


    /* Set formula per 50/100 ports*/
    portFormula = (0x18 * portIndex);
    /* start of unit MTI_EXT - per channel/port*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus          = 0x00000088 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause  = 0x00000094 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask   = 0x00000098 + portFormula + unitOffset;

    if ((portIndex % 4) == 0)
    {
        /* Set formula per 200/400 ports*/
        portFormula = (0x14 * (portIndex / 4));
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortStatus         = 0x00000060 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause = 0x00000068 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask  = 0x0000006c + portFormula + unitOffset;

        regAddr_400GMac(devObjPtr, portNum, portIndex, unitIndex);
    }
}

/**
* @internal regAddr_USX_MTIP_EXT function
* @endinternal
*
* @brief   initialize the register DB - USX MAC - MTIP_EXT
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            sip6_MTI_EXTERNAL_representativePortIndex - port represent global registers addresses per all ports in unit
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..5
*
*/
static void regAddr_USX_MTIP_EXT
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_EXT_BASE_OFFSET;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_USX_MTIP_EXT : non-valid unit[%d]",unitIndex);
        return;
    }

    if(portIndex == 0)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].USX_GLOBAL.globalChannelControl        = 0x00000008 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].USX_GLOBAL.globalInterruptSummaryCause = 0x00000018 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].USX_GLOBAL.globalInterruptSummaryMask  = 0x0000001c + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].USX_GLOBAL.globalUsxPchControl         = 0x0000016c + unitOffset;
    }

    /* Set formula per USX ports*/
    portFormula = (0x28 * portIndex);
    /* start of unit USX - per channel/port*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portStatus          = 0x00000030 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portInterruptCause  = 0x00000038 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portInterruptMask   = 0x0000003C + portFormula + unitOffset;
    portFormula = (0x10 * portIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portUsxPchSignatureControl   = 0x00000174 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portUsxPchControl   = 0x0000017C + portFormula + unitOffset;
}

/**
* @internal regAddr_100GMac function
* @endinternal
*
* @brief   initialize the register DB - 100G MAC
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_100GMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + ((is_SMEM_UNIT_TYPE_MTI_MAC_USX_E == 0) ? PORT0_100G_OFFSET : USX_PORT0_100G_OFFSET);
    GT_U32  portFormula;

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_100GMac : non-valid unit[%d]",unitIndex);
        return;
    }

    portFormula = (MAC_STEP_PORT_OFFSET * portIndex);

    if(!is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
    {
        devObjPtr->portsArr[portNum].portSupportPreemption = GT_TRUE;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER._802_3_BR.brControl      = 0x000000A8 + portFormula + unitOffset;

        /* WARNING !!! the 'EMAC' is on offset 0x100 from the base ! */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig  = 0x00000008 + EMAC_OFFSET + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.frmLength      = 0x00000014 + EMAC_OFFSET + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr0       = 0x0000000C + EMAC_OFFSET + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr1       = 0x00000010 + EMAC_OFFSET + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.status         = 0x00000040 +               portFormula + unitOffset; /*shared with MTI_PREEMPTION_MAC */

        /* WARNING !!! the 'PMAC' is on offset 0x000 from the base ... meaning on 'legacy addresses' !!! */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI_PREEMPTION_MAC.commandConfig  = 0x00000008 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI_PREEMPTION_MAC.frmLength      = 0x00000014 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI_PREEMPTION_MAC.macAddr0       = 0x0000000C + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI_PREEMPTION_MAC.macAddr1       = 0x00000010 + portFormula + unitOffset;
        /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI_PREEMPTION_MAC.status         = 0x00000040 + portFormula + unitOffset; shared with MTI64_MAC */
    }
    else
    if(GT_OK == devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_50G_E,portNum,&portInfo))
    {
        /* this port already got initialization on 'MTI64_MAC' from the '50G' unit base addr */
    }
    else
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig  = 0x00000008 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.frmLength      = 0x00000014 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr0       = 0x0000000C + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr1       = 0x00000010 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.status         = 0x00000040 + portFormula + unitOffset;
    }

}
/**
* @internal regAddr_MTIP400GMacWrap function
* @endinternal
*
* @brief   initialize the register DB - MTIP 400G MAC
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_MTIP400GMacWrap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  indexTo_macWrap = is_SMEM_UNIT_TYPE_MTI_MAC_USX_E ?
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr  :
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr;

    if (is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
    {
        regAddr_USX_MTIP_EXT(devObjPtr, portNum, sip6_MTI_EXTERNAL_representativePortIndex, portIndex, indexTo_macWrap);
    }
    else
    {
        regAddr_400G_MTIP_EXT(devObjPtr,portNum,sip6_MTI_EXTERNAL_representativePortIndex,portIndex,indexTo_macWrap);
    }
    regAddr_100GMac      (devObjPtr,portNum,is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,portIndex,indexTo_macWrap);
    smemChtGopMtiInitMacMibCounters(devObjPtr,portNum ,portIndex , sip6_MTI_EXTERNAL_representativePortIndex/*channelIndex*/ , 0/*dieIndex*/ , 0 /*globalRaven*/ , 0/*isCpuPort*/);
}

/**
* @internal regAddr_MTIP400GPcs function
* @endinternal
*
* @brief   initialize the register DB - MTIP 400G PCS
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_MTIP400GPcs
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  isPort_200_400;
    GT_U32  indexTo_macPcs = is_SMEM_UNIT_TYPE_MTI_MAC_USX_E ?
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr  :
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macPcs[indexTo_macPcs];

    if(unitOffset == 0)
    {
        skernelFatalError("regAddr_MTIP400GPcs : non-valid unit[%d]",unitIndex);
        return;
    }

    if(is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)/*UNIT_USX_0_PCS_0*/
    {
        portFormula = USX_PCS_PORTS_OFFSET + (USX_PCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.status1  = 0x00000004 + portFormula + unitOffset;

        portFormula = USX_PCS_LPCS_OFFSET  + (USX_LPCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.status1  = 0x00000004 + portFormula + unitOffset;

        if(portIndex == 1) /* this is WA to portIndex == 0 that not reach here */
        {
            /* info of PCS that is per representavive port */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].PCS_common.LPCS_common.gmode = 0x000003e0 + USX_PCS_LPCS_OFFSET + unitOffset;/*0x000063e0*/
        }
        return;
    }

    if ((portIndex % 4) == 0)
    {
        isPort_200_400 = 1;
    }
    else
    {
        isPort_200_400 = 0;
    }

    /* Set formula per 50/100 ports : portIndex = 0..7 */
    portFormula = PCS_PORT0_100G_OFFSET + (PCS_STEP_PORT_OFFSET * portIndex);
    if (portIndex % 2)
    {
        /* Set formula per odd ports */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.status1  = 0x00000004 + portFormula + unitOffset;
    }
    else
    {
        /* Set formula per even ports */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.status1  = 0x00000004 + portFormula + unitOffset;
    }

    portFormula = PCS_LPCS_OFFSET + (LPCS_STEP_PORT_OFFSET * portIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.control1 = 0x00000000 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.status1  = 0x00000004 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.usxgmii_rep = 0x00000058 + portFormula + unitOffset;

    if(isPort_200_400)/* portIndex = 0,4 */
    {
        portFormula = (portIndex == 0) ? PCS_400G_OFFSET : PCS_200G_OFFSET;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.status1  = 0x00000004 + portFormula + unitOffset;
    }

    if(portIndex == 0)
    {
        /* info of PCS that is per representavive port */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].PCS_common.LPCS_common.gmode = 0x000003e0 + PCS_LPCS_OFFSET + unitOffset;/*0x000063e0*/
    }

}

/**
* @internal regAddr_MTIPCpuMacWrap function
* @endinternal
*
* @brief   initialize the register DB - for CPU port of MTI MAC unit
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - port number
*
*/
static void regAddr_MTIPCpuMacWrap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  portFormula;
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex = 0;
    IN GT_U32 unitIndex = 0;
    GT_U32  unitFormula = devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[unitIndex];

    if(unitFormula == 0)
    {
        skernelFatalError("regAddr_MTIPCpuMacWrap : non-valid unit[%d]",unitIndex);
        return;
    }

    portFormula = unitFormula + CPU_MAC_EXT_BASE_OFFSET;

    /* MTIP IP CPU MAC WRAPPER {Current}/<MTIP CPU EXT> MTIP CPU EXT/MTIP CPU EXT Units */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex].MTI_EXT_PORT.portStatus          = 0x00000008  + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex].MTI_EXT_PORT.portInterruptCause  = 0x0000000C + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex].MTI_EXT_PORT.portInterruptMask   = 0x00000010 + portFormula;

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.control               = 0x00000000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalResetControl    = 0x00000014 + portFormula;

#if 0 /* initialized in smemChtGopMtiInitMacMibCounters */
    portFormula = unitFormula + CPU_MAC_MIB_OFFSET;

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].cpu_MTI_STATS.config   = 0x00000008 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].cpu_MTI_STATS.control  = 0x0000000C + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].cpu_MTI_STATS.datHiCdc = 0x0000001C + portFormula;
    for (ii = 0; ii < 56; ii++)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].cpu_MTI_STATS.counterCapture[ii] = 0x00000020 + 0x4 * ii + portFormula;
    }
#endif /*0*/
    portFormula = unitFormula + CPU_MAC_PORT_OFFSET;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuMac.commandConfig = 0x00000008 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuMac.frmLength     = 0x00000014 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuMac.macAddr0      = 0x0000000C + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuMac.macAddr1      = 0x00000010 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuMac.status        = 0x00000040 + portFormula;

    /* do mib counters */
    /* channelIndex --> unitIndex --> devObjPtr->memUnitBaseAddrInfo.cpuMacMib[unitIndex] */
    smemChtGopMtiInitMacMibCounters(devObjPtr,hawkCpuPortsArr[0].globalMacNum ,0/*portIndex*/ , unitIndex/*channelIndex*/ , 0/*dieIndex*/ , 0 /*globalRaven*/ , 1/*isCpuPort*/);
}


/**
* @internal regAddr_MTIPCpuPcs function
* @endinternal
*
* @brief   initialize the register DB - for CPU port of MTI PCS unit
*
* @param[in] devObjPtr - pointer to device object.
*
*/
static void regAddr_MTIPCpuPcs
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 unitIndex = 0;
    GT_U32  portFormula;
    GT_U32  unitFormula = devObjPtr->memUnitBaseAddrInfo.cpuMacPcs[unitIndex];

    if(unitFormula == 0)
    {
        skernelFatalError("regAddr_MTIPCpuPcs : non-valid unit[%d]",unitIndex);
        return;
    }

    portFormula = unitFormula + CPU_PCS_PORT_OFFSET;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuPcs.control1      = 0x00000000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuPcs.status1       = 0x00000004 + portFormula;
}


/**
* @internal smemHawkGopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Hawk.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkGopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  unitIndex,globalPort;
    ENHANCED_PORT_INFO_STC portInfo;

    devObjPtr->portMacOffset = MAC_STEP_PORT_OFFSET;
    devObjPtr->portMacMask   = 0x7F;

    /* MIF support */
    {
        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E,globalPort,&portInfo))
            {
                continue;
            }

            regAddr_mif(devObjPtr,
                globalPort,/* global port in the device */
                portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);  /* the unit index 0..6 */
        }
    }

    /* LMU support */
    {
        GT_U32  numOfLmus = devObjPtr->numOfLmus;

        devObjPtr->tablesInfo.lmuStatTable.commonInfo.multiInstanceInfo.numBaseAddresses = numOfLmus;
        devObjPtr->tablesInfo.lmuStatTable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lmu[0];
        devObjPtr->tablesInfo.lmuCfgTable.commonInfo.multiInstanceInfo.numBaseAddresses = numOfLmus;
        devObjPtr->tablesInfo.lmuCfgTable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lmu[0];

        for(unitIndex = 0 ; unitIndex < numOfLmus ; unitIndex++)
        {
            regAddr_lmu(devObjPtr,unitIndex);
        }
    }

    /* TSU/PTP support */
    {

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E,globalPort,&portInfo))
            {
                continue;
            }

            regAddr_tsu(devObjPtr,
                globalPort,/* global port in the device */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */
        }
    }

    /* 400G MAC,PCS support */
    {
        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_50G_E;
            GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_USX_E;

            is_SMEM_UNIT_TYPE_MTI_MAC_50G_E = (GT_OK == devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_50G_E,globalPort,&portInfo)) ? 1 : 0;
            if(is_SMEM_UNIT_TYPE_MTI_MAC_50G_E)
            {
                regAddr_MTIP400GMacWrap(devObjPtr,
                    globalPort,/* global port in the device */
                    0,/* not usx */
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */

                regAddr_MTIP400GPcs(devObjPtr,
                    globalPort,/* global port in the device */
                    0,/* not usx */
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */
            }

            is_SMEM_UNIT_TYPE_MTI_MAC_USX_E = (GT_OK == devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_USX_E,globalPort,&portInfo)) ? 1 : 0;
            if(is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
            {
                regAddr_MTIP400GMacWrap(devObjPtr,
                    globalPort,/* global port in the device */
                    1,/* is usx */
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */

                regAddr_MTIP400GPcs(devObjPtr,
                    globalPort,/* global port in the device */
                    1,/* is usx */
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */
            }

        }
    }

    globalPort = hawkCpuPortsArr[0].globalMacNum;
    /* CPU MAC,PCS support */
    if(GT_OK == devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E,globalPort,&portInfo))
    {
        regAddr_MTIPCpuMacWrap(devObjPtr);
        regAddr_MTIPCpuPcs(devObjPtr);
    }

}

/**
* @internal smemHawkUnitMac400GWrap function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC 400G unit wrapper
*/
static void smemHawkUnitMac400GWrap
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* Statistic counters - active memory read */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.counterCapture[ii] */
        /* ii = 0..55 : address 0x20-0xfc

          lets brake it down:
                            addr        mask
          0x20 - 0x3c :     0x20      (~0x1F)
          0x40 - 0x7c :     0x40      (~0x3F)
          0x80 - 0xfc :     0x80      (~0x7F)
        */
        {MAC_MIB_OFFSET + 0x00000020, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, SMEM_ACTIVE_MTI_MIB_SUPPORT_PREEMPTION_MAC, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000040, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, SMEM_ACTIVE_MTI_MIB_SUPPORT_PREEMPTION_MAC, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000080, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, SMEM_ACTIVE_MTI_MIB_SUPPORT_PREEMPTION_MAC, NULL , 0},
        /* Statistic counters - active memory write control */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.control */
        {MAC_MIB_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, SMEM_ACTIVE_MTI_MIB_SUPPORT_PREEMPTION_MAC},


        /* MIB - END */

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause */
        {MAC_EXT_BASE_OFFSET + 0x0000004C, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},


        /*0x00000094 + 0x18*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*2, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*3, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*5, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*6, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*7, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},


        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*2, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*3, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*4, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*5, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*6, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*7, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},


        /*0x00000068 + 0x14*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause */
        {MAC_EXT_BASE_OFFSET + 0x00000068 + 0x14*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000068 + 0x14*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask */
        {MAC_EXT_BASE_OFFSET + 0x0000006c + 0x14*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000006c + 0x14*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

        /* Global Amps Lock Status */
        {MAC_EXT_BASE_OFFSET + 0x0000001c , SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0xFFFFFFFF, NULL, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* MIB - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_MIB_OFFSET + 0x00000000, MAC_MIB_OFFSET + 0x00000014)}
             /* memories till start of RX counters per port */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_MIB_OFFSET + 0x0000001C, MAC_MIB_OFFSET + 0x000000FC)}
             /* memories from start of RX counters per port , till TX counters per port */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_MIB_OFFSET + START_MIB_RX_PER_PORT, MAC_MIB_OFFSET + START_MIB_TX_PER_PORT + SIZE_MIB_TX - 0x4)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    /* MIB - END */

    /* MAC 400G - start */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_200G_400G_OFFSET + 0x00000000, MAC_200G_400G_OFFSET + 0x00000014)} , FORMULA_SINGLE_PARAMETER(2 , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_200G_400G_OFFSET + 0x0000001C, MAC_200G_400G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER(2 , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_200G_400G_OFFSET + 0x0000002C, MAC_200G_400G_OFFSET + 0x00000044)} , FORMULA_SINGLE_PARAMETER(2 , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_200G_400G_OFFSET + 0x00000054, MAC_200G_400G_OFFSET + 0x00000074)} , FORMULA_SINGLE_PARAMETER(2 , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_200G_400G_OFFSET + 0x0000007C, MAC_200G_400G_OFFSET + 0x000000A0)} , FORMULA_SINGLE_PARAMETER(2 , MAC_STEP_PORT_OFFSET)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* MAC 400G - end */

    /* MAC 100G - start */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC : ports 0..7 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000000, PORT0_100G_OFFSET + 0x00000014)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x0000001C, PORT0_100G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000030, PORT0_100G_OFFSET + 0x00000048)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000054, PORT0_100G_OFFSET + 0x00000070)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000074, PORT0_100G_OFFSET + 0x00000074)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}/*PATCH till CPSS fix address to 0x174 (from 0x74)!*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x0000007C, PORT0_100G_OFFSET + 0x000000A0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}

            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000A8, PORT0_100G_OFFSET + 0x000000B0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000B8, PORT0_100G_OFFSET + 0x000000B8)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000C0, PORT0_100G_OFFSET + 0x000000C0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000C8, PORT0_100G_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000D0, PORT0_100G_OFFSET + 0x000000D0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000D8, PORT0_100G_OFFSET + 0x000000D8)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x000000E0, PORT0_100G_OFFSET + 0x000000E8)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000108, PORT0_100G_OFFSET + 0x00000114)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x0000011C, PORT0_100G_OFFSET + 0x00000120)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000154, PORT0_100G_OFFSET + 0x00000174)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000184, PORT0_100G_OFFSET + 0x000001A0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* MAC 100G - end */

    /* MAC 400G EXT - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* per MAC */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x00000000, MAC_EXT_BASE_OFFSET + 0x00000054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x0000005c, MAC_EXT_BASE_OFFSET + 0x000001E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x000001F0, MAC_EXT_BASE_OFFSET + 0x000001F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x00000200, MAC_EXT_BASE_OFFSET + 0x00000274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x00000280, MAC_EXT_BASE_OFFSET + 0x0000037C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x00000380, MAC_EXT_BASE_OFFSET + 0x0000039C)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* MAC 400G EXT - end */


   {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* MAC 400G EXT - start */
             {DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000008,         0x5a4c0000,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x0000005c,         0x0405f840,      2,    0x14  }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000084,         0x01017e10,      8,    0x18  }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x000001f0,         0x0000ffff,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000200,         0x20001000,      8,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000240,         0x00000180,      8,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000260,         0x20001000,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000270,         0x00000180,      2,    0x4   }
            /* MAC 400G EXT - end */

            /* MAC 400G - start */
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x00000000,         0x00010102,      1,    0x0  ,2 , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x00000008,         0x00000800,      1,    0x0  ,2 , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x00000014,         0x00000600,      1,    0x0  ,2 , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x0000001c,         0x00000008,      2,    0x4  ,2 , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x00000030,         0x00007fc0,      1,    0x0  ,2 , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           MAC_200G_400G_OFFSET + 0x00000044,         0x5000000c,      1,    0x0  ,2 , MAC_STEP_PORT_OFFSET}
            /* MAC 400G - end */

            /* MAC 100G - start */
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000000,         0x00010200,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000008,         0x00000800,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000014,         0x00000600,      1,    0x0  ,2,0x100,NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x0000001c,         0x00000008,      2,    0x4  ,2,0x100,NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000030,         0x00001440,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000044,         0x0000000c,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000080,         0x00000100,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}

            /* MTI_WRAPPER._802_3_BR.brControl : manual defaults as the Cider not ready yet */
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x000000A8,         0x01780A00,      1,    0x0  ,        NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET}

            /* MAC 100G - end */

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitPcs400G function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC 400G unit wrapper
*/
static void smemHawkUnitPcs400G
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        {PCS_400G_OFFSET + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_200G_OFFSET + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},

        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 4 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 5 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 6 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 7 + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1, 0},
        /* support 4 ports in LPCS  */
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 4 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 5 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 6 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 7 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},

        /* BASER_STATUS1 */
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 0 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 1 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 2 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 3 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 4 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 5 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 6 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},
        {PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET * 7 + 0x00000080, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0x1, NULL, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PCS-400G - start */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000000, PCS_400G_OFFSET + 0x00000024)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000038, PCS_400G_OFFSET + 0x0000003C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000080, PCS_400G_OFFSET + 0x00000084)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x000000A8, PCS_400G_OFFSET + 0x000000B4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x000000C8, PCS_400G_OFFSET + 0x000000C8)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x000000D0, PCS_400G_OFFSET + 0x000000D4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000640, PCS_400G_OFFSET + 0x0000067C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000800, PCS_400G_OFFSET + 0x0000080C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x0000081C, PCS_400G_OFFSET + 0x00000824)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_400G_OFFSET + 0x00000A00, PCS_400G_OFFSET + 0x00000A0C)}}
             /* PCS-400G - end */
             /* PCS-200G - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x00000000, PCS_200G_OFFSET + 0x00000024)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x00000038, PCS_200G_OFFSET + 0x0000003C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x00000080, PCS_200G_OFFSET + 0x00000084)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x000000A8, PCS_200G_OFFSET + 0x000000B4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x000000C8, PCS_200G_OFFSET + 0x000000C8)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_200G_OFFSET + 0x00000100, PCS_200G_OFFSET + 0x0000011C)}}
             /* PCS-200G - end */
             /* PCS-100G - start (ports = 0,2,4,6) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000000, PCS_PORT0_100G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000038, PCS_PORT0_100G_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000080, PCS_PORT0_100G_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x000000C8, PCS_PORT0_100G_OFFSET + 0x000000D4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000320, PCS_PORT0_100G_OFFSET + 0x0000036C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000640, PCS_PORT0_100G_OFFSET + 0x0000068C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000800, PCS_PORT0_100G_OFFSET + 0x0000080C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000820, PCS_PORT0_100G_OFFSET + 0x00000840)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000900, PCS_PORT0_100G_OFFSET + 0x0000099C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
             /* PCS-100G - end */
             /* PCS-50G - start (ports = 1,3,5,7) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000000, PCS_PORT1_50G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000038, PCS_PORT1_50G_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000080, PCS_PORT1_50G_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x000000C8, PCS_PORT1_50G_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x000000D0, PCS_PORT1_50G_OFFSET + 0x000000D0)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000320, PCS_PORT1_50G_OFFSET + 0x0000032C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000640, PCS_PORT1_50G_OFFSET + 0x0000064C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000800, PCS_PORT1_50G_OFFSET + 0x0000080C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000820, PCS_PORT1_50G_OFFSET + 0x00000840)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_50G_OFFSET + 0x00000900, PCS_PORT1_50G_OFFSET + 0x0000091C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
             /* PCS-50G - end */
             /* PCS-RSFEC - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000000, PCS_RS_FEC_OFFSET + 0x00000108)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000110, PCS_RS_FEC_OFFSET + 0x0000012C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000180, PCS_RS_FEC_OFFSET + 0x0000027C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000284, PCS_RS_FEC_OFFSET + 0x00000290)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000300, PCS_RS_FEC_OFFSET + 0x000003FC)}}
             /* PCS-RSFEC - end */
             /* PCS-RSFEC-STATISTIC - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_STATISTICS_OFFSET + 0x00000000, PCS_RS_FEC_STATISTICS_OFFSET + 0x00000014)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_STATISTICS_OFFSET + 0x0000001C, PCS_RS_FEC_STATISTICS_OFFSET + 0x0000001C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_STATISTICS_OFFSET + 0x00000020, PCS_RS_FEC_STATISTICS_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER((RS_FEC_STATISTIC_CAPTURED_COUNTER_INDEX) , (RS_STATISTIC_CONTER_CAPTURE_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_STATISTICS_OFFSET + 0x00000070, PCS_RS_FEC_STATISTICS_OFFSET + 0x000000BC)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT) , (RS_STATISTIC_CONTER_CODEWORDS_OFFSET))}
             /* PCS-RSFEC-STATISTIC - end */
             /* PCS-LPCS - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000000, PCS_LPCS_OFFSET + 0x00000020)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000040, PCS_LPCS_OFFSET + 0x0000005C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000080, PCS_LPCS_OFFSET + 0x000000A0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x000000C0, PCS_LPCS_OFFSET + 0x000000DC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000100, PCS_LPCS_OFFSET + 0x00000120)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000140, PCS_LPCS_OFFSET + 0x0000015C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000180, PCS_LPCS_OFFSET + 0x000001A0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x000001C0, PCS_LPCS_OFFSET + 0x000001DC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000200, PCS_LPCS_OFFSET + 0x00000220)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000240, PCS_LPCS_OFFSET + 0x0000025C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000280, PCS_LPCS_OFFSET + 0x000002A0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x000002C0, PCS_LPCS_OFFSET + 0x000002DC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000300, PCS_LPCS_OFFSET + 0x00000320)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000340, PCS_LPCS_OFFSET + 0x0000035C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x00000380, PCS_LPCS_OFFSET + 0x000003A0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_LPCS_OFFSET + 0x000003C0, PCS_LPCS_OFFSET + 0x000003F4)}}
             /* PCS-LPCS - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*400G*/
             {DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000000,         0x00002068,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000010,         0x00000300,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x0000001c,         0x0000000d,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000020,         0x00008000,      2,    0x94           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000024,         0x00000003,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000804,         0x00004220,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000808,         0x00002000,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x0000080c,         0x00000007,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x0000081c,         0x00000008,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000820,         0x00004a9a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_400G_OFFSET + 0x00000824,         0x00000026,      1,    0x0            }
            /*400G*/

            /*200G*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000000,         0x00002064,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000008,         0x00000001,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000010,         0x00000100,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x0000001c,         0x0000000c,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000020,         0x00008000,      2,    0x94           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_200G_OFFSET + 0x00000024,         0x00000001,      1,    0x0            }
            /*200G*/

            /*100G*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000000,         0x0000204c,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000010,         0x0000003d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000020,         0x000081b1,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000088,         0x0000804f,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000008c,         0x0000cab6,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000090,         0x0000b44d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000094,         0x000003c8,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000098,         0x00008884,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000009c,         0x000085a3,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000a0,         0x000006bb,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000a4,         0x00000349,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000b4,         0x00008000,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000808,         0x00003fff,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000080c,         0x00009999,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000820,         0x000068c1,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000824,         0x00000021,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000828,         0x0000719d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000082c,         0x0000008e,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000830,         0x00004b59,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000834,         0x000000e8,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000838,         0x0000954d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000083c,         0x0000007b,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000840,         0x00000303,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000900,         0x000068c1,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000904,         0x00000021,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000908,         0x0000719d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000090c,         0x0000008e,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000910,         0x00004b59,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000914,         0x000000e8,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000918,         0x0000954d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000091c,         0x0000007b,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000920,         0x000007f5,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000924,         0x00000009,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000928,         0x000014dd,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000092c,         0x000000c2,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000930,         0x00004a9a,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000934,         0x00000026,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000938,         0x0000457b,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000093c,         0x00000066,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000940,         0x000024a0,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000944,         0x00000076,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000948,         0x0000c968,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000094c,         0x000000fb,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000950,         0x00006cfd,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000954,         0x00000099,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000958,         0x000091b9,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000095c,         0x00000055,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000960,         0x0000b95c,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000964,         0x000000b2,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000968,         0x0000f81a,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000096c,         0x000000bd,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000970,         0x0000c783,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000974,         0x000000ca,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000978,         0x00003635,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000097c,         0x000000cd,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000980,         0x000031c4,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000984,         0x0000004c,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000988,         0x0000d6ad,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000098c,         0x000000b7,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000990,         0x0000665f,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000994,         0x0000002a,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000998,         0x0000f0c0,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000099c,         0x000000e5,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            /*100G*/

            /*50G*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000000,         0x0000204c,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000008,         0x00000001,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000010,         0x00000035,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000001c,         0x00000004,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000020,         0x00008191,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000088,         0x0000804f,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000008c,         0x0000cab6,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000090,         0x0000b44d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000094,         0x000003c8,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000098,         0x00008884,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000009c,         0x000085a3,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x000000a0,         0x000006bb,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x000000a4,         0x00000349,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x000000b4,         0x00008000,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000804,         0x00000300,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000808,         0x00003fff,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000080c,         0x00009999,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000820,         0x00007690,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000824,         0x00000047,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000828,         0x0000c4f0,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000082c,         0x000000e6,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000830,         0x000065c5,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000834,         0x0000009b,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000838,         0x000079a2,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000083c,         0x0000003d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000840,         0x00000303,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000900,         0x00007690,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000904,         0x00000047,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000908,         0x0000c4f0,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000090c,         0x000000e6,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000910,         0x000065c5,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000914,         0x0000009b,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x00000918,         0x000079a2,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT1_50G_OFFSET + 0x0000091c,         0x0000003d,      1,    0x0            ,(NUM_PORTS_PER_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            /*50G*/

            /*lpcs*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000000,         0x00001140,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000004,         0x00000009,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000008,         0x00004950,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x0000000c,         0x00004d54,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000010,         0x000001a0,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000018,         0x00000004,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000044,         0x00000001,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000048,         0x000012d0,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x0000004c,         0x00000013,      8,    0x80           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x00000050,         0x00000100,      8,    0x80           }  /* IF_MODE */
            ,{DUMMY_NAME_PTR_CNS,            PCS_LPCS_OFFSET + 0x0000005c,         0x0000000c,      8,    0x80           }
            /*lpcs*/

            /*rsfec*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000004,         0x0000000a,      8,    0x20           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x0000001c,         0x00000033,      8,    0x20           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000100,         0x00002000,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000104,         0x000015b8,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000108,         0x0000023d,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000180,         0x00007101,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000184,         0x000000f3,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000188,         0x0000de5a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x0000018c,         0x0000007e,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000190,         0x0000f33e,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000194,         0x00000056,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000198,         0x00008086,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x0000019c,         0x000000d0,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001a0,         0x0000512a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001a4,         0x000000f2,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001a8,         0x00004f12,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001ac,         0x000000d1,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001b0,         0x00009c42,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001b4,         0x000000a1,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001b8,         0x000076d6,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001bc,         0x0000005b,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001c0,         0x000073e1,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001c4,         0x00000075,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001c8,         0x0000c471,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001cc,         0x0000003c,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001d0,         0x0000eb95,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001d4,         0x000000d8,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001d8,         0x00006622,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001dc,         0x00000038,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001e0,         0x0000f6a2,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001e4,         0x00000095,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001e8,         0x00009731,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001ec,         0x000000c3,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001f0,         0x0000fbca,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001f4,         0x000000a6,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001f8,         0x0000baa6,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x000001fc,         0x00000079,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x0000028c,         0x00000140,      1,    0x0            }
            /*rsfec*/

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUnitMacCpuWrap function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC CPU unit wrapper
*/
static void smemHawkUnitMacCpuWrap
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* MIB - start */

        /* Statistic counters - active memory read */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[1].cpu_MTI_STATS.counterCapture[ii] */
        /* ii = 0..55 : address 0x20-0xfc

          lets brake it down:
                            addr        mask
          0x20 - 0x3c :     0x20      (~0x1F)
          0x40 - 0x7c :     0x40      (~0x3F)
          0x80 - 0xfc :     0x80      (~0x7F)
        */
        {CPU_MAC_MIB_OFFSET + 0x00000020, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {CPU_MAC_MIB_OFFSET + 0x00000040, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {CPU_MAC_MIB_OFFSET + 0x00000080, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        /* Statistic counters - active memory write control */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[1].cpu_MTI_STATS.control */
        {CPU_MAC_MIB_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},

        /* MIB - END */

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[1].MTI_EXT_PORT.portInterruptCause */
        {CPU_MAC_EXT_BASE_OFFSET + 0x0000000c, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[1].MTI_EXT_PORT.portInterruptMask */
        {CPU_MAC_EXT_BASE_OFFSET + 0x00000010, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteCpuPortInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* MIB - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_MIB_OFFSET + 0x00000000, CPU_MAC_MIB_OFFSET + 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_MIB_OFFSET + 0x0000001C, CPU_MAC_MIB_OFFSET + 0x00000A9C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    /* MIB - END */

    /* MAC CPU - start */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_PORT_OFFSET + 0x00000000, CPU_MAC_PORT_OFFSET + 0x00000014)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_PORT_OFFSET + 0x0000001C, CPU_MAC_PORT_OFFSET + 0x00000020)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_PORT_OFFSET + 0x00000030, CPU_MAC_PORT_OFFSET + 0x00000048)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_PORT_OFFSET + 0x00000054, CPU_MAC_PORT_OFFSET + 0x00000074)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_PORT_OFFSET + 0x0000007C, CPU_MAC_PORT_OFFSET + 0x000000A0)}}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* MAC CPU - end */

    /* MAC CPU EXT - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* per MAC */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_MAC_EXT_BASE_OFFSET + 0x00000000, CPU_MAC_EXT_BASE_OFFSET + 0x00000060)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* MAC CPU EXT - end */

   {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* MAC CPU EXT - start */
             {DUMMY_NAME_PTR_CNS,            CPU_MAC_EXT_BASE_OFFSET+0x00000000,         0x083CFE00,      1,    0x0}
            /* MAC CPU EXT - end */

            /* MAC CPU - start */
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000000,         0x00010200,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000008,         0x00000800,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000014,         0x00000600,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x0000001c,         0x00000008,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000030,         0x00001440,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000044,         0x0000000c,      1,    0x0}
            ,{DUMMY_NAME_PTR_CNS,            CPU_MAC_PORT_OFFSET+0x00000080,         0x00000100,      1,    0x0}
            /* MAC CPU - end */

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
   }

}

/**
* @internal smemHawkUnitPcsCpu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCS CPU unit wrapper
*/
static void smemHawkUnitPcsCpu
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* PCS : CPU_PORT : MTIP IP CPU PCS WRAPPER {Current}/<MTIP CPU PCS>MTIP CPU PCS/<MTIP CPU PCS> PCS/PCS Units/PORT_CONTROL1 */
        {CPU_PCS_PORT_OFFSET + 0x00000000, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePcsControl1 , 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)


    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* PCS-CPU - start */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000000, CPU_PCS_PORT_OFFSET+0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000038, CPU_PCS_PORT_OFFSET+0x0000003c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000080, CPU_PCS_PORT_OFFSET+0x000000b4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x000000c8, CPU_PCS_PORT_OFFSET+0x000000c8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x000000d0, CPU_PCS_PORT_OFFSET+0x000000d0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000800, CPU_PCS_PORT_OFFSET+0x0000080c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000820, CPU_PCS_PORT_OFFSET+0x0000082c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000830, CPU_PCS_PORT_OFFSET+0x0000083c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000840, CPU_PCS_PORT_OFFSET+0x00000848)}
            /* PCS-CPU - end */

            /* rsfec-CPU - start */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000000, CPU_PCS_RS_FEC_OFFSET+0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000028, CPU_PCS_RS_FEC_OFFSET+0x0000004c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000200, CPU_PCS_RS_FEC_OFFSET+0x0000020C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000300, CPU_PCS_RS_FEC_OFFSET+0x00000314)}
            /* rsfec-CPU - end */

            /* lpcs-CPU - start */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000000, CPU_PCS_LPCS_OFFSET+0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000040, CPU_PCS_LPCS_OFFSET+0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000080, CPU_PCS_LPCS_OFFSET+0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x000000C0, CPU_PCS_LPCS_OFFSET+0x000000DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000100, CPU_PCS_LPCS_OFFSET+0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000140, CPU_PCS_LPCS_OFFSET+0x0000015C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000180, CPU_PCS_LPCS_OFFSET+0x000001A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x000001C0, CPU_PCS_LPCS_OFFSET+0x000001DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000200, CPU_PCS_LPCS_OFFSET+0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000240, CPU_PCS_LPCS_OFFSET+0x0000025C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000280, CPU_PCS_LPCS_OFFSET+0x000002A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x000002C0, CPU_PCS_LPCS_OFFSET+0x000002DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000300, CPU_PCS_LPCS_OFFSET+0x00000320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000340, CPU_PCS_LPCS_OFFSET+0x0000035C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x00000380, CPU_PCS_LPCS_OFFSET+0x000003A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_LPCS_OFFSET+0x000003C0, CPU_PCS_LPCS_OFFSET+0x000003F4)}
            /* lpcs-CPU - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

   {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* PCS-CPU - start */
             {DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000000,         0x00002040,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000010,         0x00000011,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000014,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000020,         0x00008081,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000088,         0x0000804f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x0000008c,         0x0000cab6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000090,         0x0000b44d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000094,         0x000003c8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000098,         0x00008884,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x0000009c,         0x000085a3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x000000a0,         0x000006bb,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x000000a4,         0x00000349,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x000000b4,         0x00008000,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000808,         0x00004fff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x0000080c,         0x00000007,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000820,         0x000068c1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000824,         0x00000021,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000828,         0x0000c4f0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x0000082c,         0x000000e6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000830,         0x000065c5,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000834,         0x0000009b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000838,         0x000079a2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x0000083c,         0x0000003d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000840,         0x00000303,      1,    0x0     }
            /* PCS-CPU - end */

            /* rsfec-CPU - start */
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_RS_FEC_OFFSET+0x0000020c,       0x00000022,      1,    0x0     }
            /* rsfec-CPU - end */

            /* lpcs-CPU - start */
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000004,         0x00000009,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000008,         0x00004950,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x0000000c,         0x00004d54,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000010,         0x000001a0,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000018,         0x00000004,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000044,         0x00000001,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x00000048,         0x000012d0,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x0000004c,         0x00000013,      8,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_LPCS_OFFSET+0x0000005c,         0x0000000c,      8,    0x80    }
            /* lpcs-CPU - end */

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
   }
}

/**
* @internal smemHawkUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x00001D84, 0xFFFFFFFF  , NULL, 0, smemFalconActiveWriteRxDmaDebugClearAllCounters, 0, NULL},

        /* CP2RX and RX2CP counters*/
        { 0x00002080, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002100, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000118)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000188)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000968)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A68)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B68)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001080, 0x00001244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001258, 0x00001458)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000014F0, 0x00001568)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x00001C08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C80, 0x00001C94)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CA0, 0x00001CA4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D00, 0x00001D00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D80, 0x00001D8C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001E04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E80, 0x00001EAC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F00, 0x00001F0C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F80, 0x00001F90)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000204C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x0000208C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002180, 0x0000219C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x0000220C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002280, 0x00002290)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002310)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002380, 0x0000238C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x03ff03ff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00002c00,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000001a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000800,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x0000ffff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0x00600062,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x03d90007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x0007ffff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x00000002,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x000000ff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x00003fff,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x00000407,     27,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000001,     27,    0x4,      2,    0x200}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a04,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a08,         0x00000002,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a0c,         0x00000003,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a10,         0x00000004,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a14,         0x00000005,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a18,         0x00000006,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a1c,         0x00000007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a20,         0x00000008,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a24,         0x00000009,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a28,         0x0000000a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a2c,         0x0000000b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a30,         0x0000000c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a34,         0x0000000d,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a38,         0x0000000e,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a3c,         0x0000000f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a40,         0x00000010,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a44,         0x00000011,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a48,         0x00000012,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a4c,         0x00000013,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a50,         0x00000014,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a54,         0x00000015,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a58,         0x00000016,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a5c,         0x00000017,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a60,         0x00000018,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a64,         0x00000019,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a68,         0x0000001a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00000003,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001240,         0x00000800,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001244,         0x000086dd,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001258,         0x00008847,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000125c,         0x00008848,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013d0,         0x00018100,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013e0,         0x00008100,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013e4,         0x00008a88,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c00,         0xffffffff,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c08,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d00,         0xffff0000,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d80,         0x7fffffff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d88,         0x00000001,      2,    0x78        }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e80,         0x00005555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e84,         0x00155555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e88,         0x55555555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e8c,         0x01555555,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e94,         0x00000155,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e98,         0x00005555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e9c,         0x00555555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ea0,         0x55555555,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f08,         0x00000033,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f0c,         0x0000004c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f88,         0x0000000b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f8c,         0x0000002a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002008,         0x0000017f,      5,    0x10        }
            ,{DUMMY_NAME_PTR_CNS,            0x00002088,         0x00000006,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108,         0x0000003f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002188,         0x000001ff,      2,    0x10        }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x0000000a,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002288,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000228c,         0x00000007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002308,         0x0000001f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002388,         0x0000000f,      1,    0x0         }


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit sip6_rxDMA[] */

       {/*start of unit configs */
            {/*start of unit rejectConfigs */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.rejectConfigs.rejectPktCommand = 0x00001000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.rejectConfigs.rejectCPUCode = 0x00001004;

            }/*end of unit rejectConfigs */


            {/*start of unit preIngrPrioritizationConfStatus */
                {/*00007d0+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                            0x00001080+n * 0x4;
                    }/* end of loop n */
                }/*00007d0+n * 0x4*/
                {/*0000b5c+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                            0x000012b0+n*0x4;
                    }/* end of loop n */
                }/*0000b5c+n*0x4*/
                {/*0000d3c+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                            0x000013c0+n * 0x4;
                    }/* end of loop n */
                }/*0000d3c+n * 0x4*/
                {/*00007f0+m*0x4 + n*0x20*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 7 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                                0x00001090+m*0x4 + n*0x20;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*00007f0+m*0x4 + n*0x20*/
                {/*0000e74+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.profileTPID[n] =
                            0x000013d0+n*0x4;
                    }/* end of loop n */
                }/*0000e74+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.preingressPrioritizationEnable = 0x00001c08;
                {/*0000f74+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANEtherTypeConf[n] =
                            0x000013e0+n*0x4;
                    }/* end of loop n */
                }/*0000f74+n*0x4*/
                {/*0000880+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                            0x00001110+n * 0x4;
                    }/* end of loop n */
                }/*0000880+n * 0x4*/
                {/*0000ab4+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                            0x00001258+n*0x4;
                    }/* end of loop n */
                }/*0000ab4+n*0x4*/
                {/*00009fc+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                            0x00001220+n * 0x4;
                    }/* end of loop n */
                }/*00009fc+n * 0x4*/
                {/*0000b34+n *0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                            0x00001290+n *0x4;
                    }/* end of loop n */
                }/*0000b34+n *0x4*/
                {/*0000b48+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                            0x000012a0+n*0x4;
                    }/* end of loop n */
                }/*0000b48+n*0x4*/
                {/*0000af4+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                            0x00001270+n*0x4;
                    }/* end of loop n */
                }/*0000af4+n*0x4*/
                {/*0000b14+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                            0x00001280+n*0x4;
                    }/* end of loop n */
                }/*0000b14+n*0x4*/
                {/*0001174+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.latencySensitiveEtherTypeConf[n] =
                            0x000014f0+n*0x4;
                    }/* end of loop n */
                }/*0001174+n*0x4*/
                {/*0000bdc+m*0x4 + n*0x40*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 15 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                                0x000012c0+m*0x4 + n*0x40;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*0000bdc+m*0x4 + n*0x40*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00001244;
                {/*00008a4+m*0x4 + n*0x40*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 15 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                                0x00001120+m*0x4 + n*0x40;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*00008a4+m*0x4 + n*0x40*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00001240;
                {/*0000ad4+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                            0x00001260+n*0x4;
                    }/* end of loop n */
                }/*0000ad4+n*0x4*/
                {/*0000a2c+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                            0x00001230+n*0x4;
                    }/* end of loop n */
                }/*0000a2c+n*0x4*/
                {/*0001074+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {  /* manually fixed from : for(n = 0 ; n <= 63 ; n++) { */
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg[n] =
                            0x00013f0+n*0x4;
                    }/* end of loop n */
                }/*0001074+n*0x4*/

                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds0 = 0x0001c00;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[0].globalRxDMAConfigs.preIngrPrioritizationConfStatus.contextId_PIPPrioThresholds1 = 0x0001c04;
            }/*end of unit preIngrPrioritizationConfStatus */


            {/*start of unit dropThresholds */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.dropThresholds.lowPrioDropThreshold0 = 0x00000080;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.dropThresholds.lowPrioDropThreshold1 = 0x00000084;

            }/*end of unit dropThresholds */


            {/*start of unit dataAccumulator */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.dataAccumulator.PBFillLevelThresholds = 0x00000000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.dataAccumulator.maxTailBC = 0x00000004;

            }/*end of unit dataAccumulator */


            {/*start of unit cutThrough */
                {/*0000038+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.cutThrough.CTGenericConfigs[n] =
                            0x0000010c+n*0x4;
                    }/* end of loop n */
                }/*0000038+n*0x4*/
                {/*0000050+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.cutThrough.channelCTConfig[n] =
                            0x00000120+n*0x4;
                    }/* end of loop n */
                }/*0000050+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.cutThrough.CTGeneralConfig = 0x00000100;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.cutThrough.CTErrorConfig = 0x00000104;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.cutThrough.CTUPEnable = 0x00000108;

            }/*end of unit cutThrough */


            {/*start of unit channelConfig */
                {/*00004a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelToLocalDevSourcePort[n] =
                            0x0000a00+n*0x4;
                    }/* end of loop n */
                }/*00004a8+n*0x4*/
                {/*00003a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.PCHConfig[n] =
                            0x0000900+n*0x4;
                    }/* end of loop n */
                }/*00003a8+n*0x4*/
                {/*00005a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelGeneralConfigs[n] =
                            0x0000b00+n*0x4;
                    }/* end of loop n */
                }/*00005a8+n*0x4*/
                {/*00002a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelCascadePort[n] =
                            0x0000800+n*0x4;
                    }/* end of loop n */
                }/*00002a8+n*0x4*/

            }/*end of unit channelConfig */


        }/*end of unit configs */

        {/*debug*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.dp_rx_enable_debug_counters = 0x00001D84;

            {/*rxIngressDropCounter*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxIngressDropCounter.rx_ingress_drop_count           = 0x00002000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxIngressDropCounter.rx_ingress_drop_count_type_ref  = 0x00002008;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxIngressDropCounter.rx_ingress_drop_count_type_mask = 0x0000200C;
            }/*rxIngressDropCounter*/

            {/*rxToCpCounter*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count           = 0x00002080;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count_type_ref  = 0x00002088;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count_type_mask = 0x0000208C;
            }/*rxToCpCounter*/

            {/*rxCpToRxCounter*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count           = 0x00002100;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count_type_ref  = 0x00002108;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count_type_mask = 0x0000210C;
            }/*rxCpToRxCounter*/
        }/*debug*/
    }/*end of unit sip6_rxDMA[] */
}

/**
* @internal smemHawkUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001168)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001268)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x0000500C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x0000510C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005180, 0x0000519C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x00005200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005280, 0x00005290)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005300, 0x00005304)}

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x000001a1,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00a6589e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00004001,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0x00001000,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x8000005b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x80000000,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x80000001,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x80000002,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x80000003,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x80000004,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x80000005,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x80000006,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x80000007,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x80000008,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000124,         0x80000009,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000128,         0x8000000a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000012c,         0x8000000b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000130,         0x8000000c,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000134,         0x8000000d,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000138,         0x8000000e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000013c,         0x8000000f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x80000010,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x80000011,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000148,         0x80000012,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x80000013,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000150,         0x80000014,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x80000015,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000158,         0x80000016,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000015c,         0x80000017,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000160,         0x80000018,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000164,         0x80000019,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000168,         0x8000001a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000016c,         0x80000000,     65,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x000000b2,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000604,         0x0000005b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608,         0x0000002f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c,         0x00000019,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000610,         0x00000014,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000614,         0x0000000e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000618,         0x0000000b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c,         0x00000007,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000620,         0x00000005,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000624,         0x00000003,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000628,         0x00000001,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x00000079,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0x0000003e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x00000020,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x00000011,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x0000000e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x0000000a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x00000008,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000005,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x00000004,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000724,         0x00000002,      3,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000800,         0x0028d3d2,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000808,         0x002cd412,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000080c,         0x002ce413,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000810,         0x0030e413,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000814,         0x0034f454,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000818,         0x00350495,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000081c,         0x00413558,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000820,         0x005d971e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000824,         0x0061b760,      3,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00000003,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001008,         0x00000008,     24,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001068,         0x00000007,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001104,         0x00000019,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001108,         0x00000032,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000110c,         0x00000039,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0x00000040,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001114,         0x00000047,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001118,         0x0000004e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000111c,         0x00000055,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001120,         0x0000005c,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001124,         0x00000063,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001128,         0x0000006a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000112c,         0x00000071,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001130,         0x00000078,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001134,         0x0000007f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001138,         0x00000086,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000113c,         0x0000008d,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001140,         0x00000094,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001144,         0x0000009b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001148,         0x000000a2,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000114c,         0x000000a9,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001150,         0x000000b0,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001154,         0x000000b7,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001158,         0x000000be,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000115c,         0x000000c5,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001160,         0x000000cc,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001164,         0x000000d3,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001168,         0x000000da,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001204,         0x00000011,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001208,         0x00000022,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000120c,         0x00000027,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001210,         0x0000002c,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001214,         0x00000031,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001218,         0x00000036,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000121c,         0x0000003b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001220,         0x00000040,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001224,         0x00000045,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001228,         0x0000004a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000122c,         0x0000004f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001230,         0x00000054,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001234,         0x00000059,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001238,         0x0000005e,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000123c,         0x00000063,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001240,         0x00000068,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001244,         0x0000006d,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001248,         0x00000072,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000124c,         0x00000077,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001250,         0x0000007c,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001254,         0x00000081,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001258,         0x00000086,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000125c,         0x0000008b,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001260,         0x00000090,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001264,         0x00000095,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001268,         0x0000009a,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0xffff0000,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00005100,         0x0000001f,      1,    0x0  }


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit sip6_txFIFO[] */
        {/*start of unit globalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.globalConfig1 = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.packetsMaximalReadLatencyFromPacketPuffer = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.wordBufferCmn8wr4rdBufferConfig = 0x0000000c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000050;
            {/*0000100+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 92 /* pizza slices number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[p] =
                        0x00000100 + p*0x4;
                }/* end if loop p*/
            } /*0000100+p*0x4*/

        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigs */
            {/*0x00000600 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.tagFIFODepth[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/

        }/*end of unit speedProfileConfigs */

        {/*start of unit channelConfigs */
            {/*0x00001000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.speedProfile[p] =
                        0x00001000 +p*0x4;
                }/* end of loop n */
            }/*0x00001000  + p*0x4*/
            {/*0x00001100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00001100  +p*0x4;
                }/* end of loop n */
            }/*0x00001100   + p*0x4*/
            {/*0x00001200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.tagFIFOBase[p] =
                        0x00001200  +p*0x4;
                }/* end of loop n */
            }/*0x00001200   + p*0x4*/

        }/*end of unit channelConfigs */

        {/*start of unit debug*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].debug.statusBadAddr = 0x00005000;
        }/*end of unit debug*/

     }/*end of unit sip6_txFIFO[] */

}

/**
* @internal smemHawkUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].debug.txd_debug_config*/
    {0x0000700C, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteToTxDmaEnableDebugCounters,0},

    /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TXD_Status.global_status.events_counters.pds_desc_counter */
    {0x00003034, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteToReadOnlyReg,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000092C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002168)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x00002268)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002368)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002468)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003168)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003268)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x00003368)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003468)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003568)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600, 0x00003668)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003700, 0x00003768)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x00004400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004500, 0x00004500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007018, 0x0000701C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007020, 0x00007024)}

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x0000001f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x8000005b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x80000000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x80000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x80000002,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x80000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x80000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x80000005,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x80000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x80000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x80000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000124,         0x80000009,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000128,         0x8000000a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000012c,         0x8000000b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000130,         0x8000000c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000134,         0x8000000d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000138,         0x8000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000013c,         0x8000000f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x80000010,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x80000011,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000148,         0x80000012,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x80000013,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000150,         0x80000014,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x80000015,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000158,         0x80000016,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000015c,         0x80000017,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000160,         0x80000018,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000164,         0x80000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000168,         0x8000001a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000016c,         0x80000000,     65,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00000035,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000604,         0x0000001c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608,         0x0000000f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c,         0x00000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000610,         0x00000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000614,         0x00000005,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c,         0x00000003,      2,    0x4,      2,    0x19e4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000624,         0x00000002,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000628,         0x00000001,      2,    0x4,      2,    0x500  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x00000024,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0x00000013,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x0000000a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x00000005,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000002,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000724,         0x00000001,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000800,         0x59f00bd0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000804,         0x545c063c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000808,         0x51680348,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000080c,         0x501801f8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000810,         0x4fc401a4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000814,         0x4f700150,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000818,         0x4f1c00fc,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000081c,         0x4ec800a8,      5,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000334,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000904,         0x000001a0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000908,         0x000000d7,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000090c,         0x00000071,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000910,         0x0000005d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000914,         0x0000003f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000918,         0x00000034,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000091c,         0x00000020,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000920,         0x00000016,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000924,         0x0000000d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000928,         0x00000007,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a00,         0x0000016b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a04,         0x000000ba,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a08,         0x00000060,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a0c,         0x00000033,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a10,         0x0000002a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a14,         0x0000001e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a18,         0x00000018,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a1c,         0x0000000f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a20,         0x0000000c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a24,         0x00000006,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b00,         0x000000b2,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b04,         0x0000005b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b08,         0x0000002f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b0c,         0x00000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b10,         0x00000014,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b14,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b18,         0x0000000b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b1c,         0x00000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b20,         0x00000005,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b24,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c00,         0x3d590100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c04,         0x1eac0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c08,         0x0f560100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c0c,         0x07ab0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c10,         0x06220100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c14,         0x03d50100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c18,         0x03110100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c1c,         0x01880100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c20,         0x00c40100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c24,         0x00620100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c28,         0x00270100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c2c,         0x00000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d00,         0x13a10100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d04,         0x09d00100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d08,         0x04e80100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d0c,         0x02740100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d10,         0x01f60100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d14,         0x013a0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d18,         0x00fb0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d1c,         0x007d0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d20,         0x003e0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d24,         0x001f0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d28,         0x000c0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d2c,         0x00000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000e00,         0x00000200,     12,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002008,         0x00000008,     23,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002064,         0x00000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002068,         0x00000008,      2,    0x19c    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x00000010,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c,         0x00000013,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002210,         0x00000016,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002214,         0x00000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002218,         0x0000001c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000221c,         0x0000001f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002220,         0x00000022,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002224,         0x00000025,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002228,         0x00000028,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000222c,         0x0000002b,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002230,         0x0000002e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002234,         0x00000031,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002238,         0x00000034,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000223c,         0x00000037,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002240,         0x0000003a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002244,         0x0000003d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002248,         0x00000040,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000224c,         0x00000043,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002250,         0x00000046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002254,         0x00000049,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002258,         0x0000004c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000225c,         0x0000004f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002260,         0x00000052,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002264,         0x00000055,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002268,         0x00000058,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300,         0x18000000,     27,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400,         0x00000007,     27,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003700,         0x00008000,     27,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0x0fa00200,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00005000,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x00000801,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit sip6_txDMA[] */
        {/*start of unit globalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.globalConfigs.txDMAGlobalConfig = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000050;
            {/*0000100+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 33 /* pizza slices number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[p] =
                        0x00000100 + p*0x4;
                }/* end if loop p*/
            } /*0000100+p*0x4*/

        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigurations */
            {/*0x00000600 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqMaxCredits[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/
            {/*0x00000800 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqThresholdBytes[p] =
                        0x00000800 + p*0x4;
                    }/* end of loop p */
            }/*0x00000800 + p*0x4*/
            {/*0x00000900 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxWordCredits[p] =
                        0x00000900 + p*0x4;
                    }/* end of loop p */
            }/*0x00000900 + p*0x4*/
            {/*0x00000A00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxCellsCredits[p] =
                        0x00000A00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000A00 + p*0x4*/
            {/*0x00000B00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxDescCredits[p] =
                        0x00000B00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000B00 + p*0x4*/
            {/*0x00000C00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interPacketRateLimiter[p] =
                        0x00000C00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000C00 + p*0x4*/
             {/*0x00000D00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < HAWK_MAX_PROFILE_CNS; p++) {
                  SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interCellRateLimiter[p] =
                        0x00000D00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000D00 + p*0x4*/

        }/*end of unit speedProfileConfigurations */

        {/*start of unit channelConfigs */
            {/*0x00002000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.speedProfile[p] =
                        0x00002000 +p*0x4;
                }/* end of loop n */
            }/*0x00002000  + p*0x4*/
            {/*0x00002100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.channelReset[p] =
                        0x00002100  +p*0x4;
                }/* end of loop n */
            }/*0x00002100   + p*0x4*/
            {/*0x00002200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00002200  +p*0x4;
                }/* end of loop n */
            }/*0x00002200   + p*0x4*/
            {/*0x00002300  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.interPacketRateLimiterConfig[p] =
                        0x00002300 +p*0x4;
                }/* end of loop n */
            }/*0x00002300  + p*0x4*/
            {/*0x00002400  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.interCellRateLimiterConfig[p] =
                        0x00002400 +p*0x4;
                }/* end of loop n */
            }/*0x00002400  + p*0x4*/
        }/*end of unit channelConfigs */

        {/*start of unit TDSConfiguration */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TDSConfigs.globalConfig = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TDSConfigs.channelReset = 0x00004004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TDSConfigs.cellRateLimiterConfig = 0x00004008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TDSConfigs.cellRateLimiter = 0x0000400C;

        }/*end of unit TDSConfiguration */

        {/* manually added */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].debug.txd_debug_config      = 0x00007004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].debug.enable_debug_Counters = 0x0000700C;

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TXD_Status.global_status.events_counters.pds_desc_counter = 0x00003034;
        }
     }/*end of unit sip6_txDMA[] */
}

/* Start of Packet Buffer */
/**
* @internal smemHawkUnitPacketBuffer_pbCenter function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_pbCenter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCenter */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<PB_CENTER> pb_center*/
    /*IP: \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.1 - Hawk \PB_CENTER {PB_Hawk_20191017} \pb_center*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.1 - Hawk \PB_CENTER_Hawk {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000112C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000125C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001300)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000140C, 0x00001420)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_pbCounter function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_pbCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCounter*/
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<PB_COUNTER> pb_counter*/
    /*IP: \Cider \EBU-IP \Packet Buffer \PB_COUNTER \PB_COUNTER {PB_Hawk_20191017} \pb_counter*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \PB_COUNTER \PB_COUNTER {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000000C, 0x00000198)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_smbWriteArbiter function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_smbWriteArbiter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* smbWriteArbiter */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<SMB_WRITE_ARBITER> smb_write_arbiter*/
    /*IP: \Cider \EBU-IP \Packet Buffer \SMB_WRITE_ARBITER \SMB_WRITE_ARBITER 1.1 Hawk \SMB_WRITE_ARBITER {PB_Hawk_20191017} \smb_write_arbiter*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \SMB_WRITE_ARBITER \SMB_WRITE_ARBITER 1.1 Hawk \SMB_WRITE_ARBITER_Hawk {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000004, 0x0000004C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000164)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_packetWrite function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_packetWrite
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketWrite */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<GPC_PACKET_WRITE> gpc_packet_write/GPC_PACKET_WRITE %k*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_WRITE \GPC_PACKET_WRITE 1.1 - Hawk \GPC_PACKET_WRITE {PB_Hawk_20191017} \gpc_packet_write*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_WRITE \GPC_PACKET_WRITE 1.1 - Hawk \GPC_PACKET_WRITE_Hawk {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000020)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000015C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000208)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000308)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_packetRead function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_packetRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketRead */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<GPC_PACKET_READ> gpc_packet_read*/
    /*IP: Cider \EBU-IP \Packet Buffer \GPC_PACKET_READ \GPC_PACKET_READ 1.1 - Hawk \GPC_PACKET_READ {PB_Hawk_20191017} \gpc_packet_read*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_READ \GPC_PACKET_READ 1.1 - Hawk \GPC_PACKET_READ {PB_Hawk_20191017}*/
    {
        static SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Configuration */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
             /* Packet Count Configuration */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000014)}
            /* pizza arbiter */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000036C)}
            /* Channel Configuration */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000056c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000066C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000076C)}
            /* Speed Profile Configurations */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000092C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000F00, 0x00000F2C)}
            /*debug.*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000106C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000116C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001220)}
            /*debug. Credit Counters. Channel Credits*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001260, 0x00001278)}
            /*debug. Credit Counters*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001284, 0x00001290)}
            /*debug. Latency Statistics*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001294, 0x000012C8)}
            /*Interrupts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x0000138C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x0000148C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001538)}
        };
        static GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_cellRead function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_cellRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcCellRead */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<GPC_CELL_READ> gpc_cell_read*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_CELL_READ \GPC_CELL_READ {PB_Hawk_20191017} \gpc_cell_read*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_CELL_READ \GPC_CELL_READ {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /*Configuration*/
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
            /*Debug*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000028)}
            /*Debug. Latency Statistics*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000002C, 0x00000060)}
            /*Interrupts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A8)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_npmMc function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_npmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* npmMc */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<NPM_MC> npm_mc*/
    /*IP: \Cider \EBU-IP \Packet Buffer \NPM_MC \NPM_MC 1.1 - Hawk \NPM_MC {PB_Hawk_20191217} \npm_mc */
    /*Mask: \Cider \EBU-IP \Packet Buffer \NPM_MC \NPM_MC 1.1 - Hawk \NPM_MC_Hawk {PB_Hawk_20191217}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /*General*/
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
             /*Interrupt*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000158)}
             /*Free Lists*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000200)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x0000021C)}
             /*Memory Units*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
             /*Reference Counters*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000410, 0x0000041C)}
             /* RAM Access. Reference Counters RAM*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001010, 0x00001010)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
             /* RAM Access. Next-Pointer Payload RAM*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001100)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001110, 0x00001110)}
             /* RAM Access. Free List FIFO Buffer RAM*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001200)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001210, 0x00001210)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPacketBuffer_sbmMc function
* @endinternal
 *
*/
static void smemHawkUnitPacketBuffer_sbmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* sbmMc */
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PB/<SMB_MC> smb_mc*/
    /*IP: Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 1.1 Hawk \SMB_MC {PB_Hawk_20191017} \smb_mc*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 1.1 Hawk \SMB_MC_Hawk {PB_Hawk_20191017}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000038)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000006C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000074)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000008C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000AC)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000CC)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000011C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000124)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000134)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000144)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000154)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000168)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/* End of Packet Buffer */

/**
* @internal smemHawkUnitDummyRavenMg function
* @endinternal
*
* @brief   Allocate address type specific memories - dummy for MG in Ravens - for BWC interrupts
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitDummyRavenMg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* 1. dummy for MG in Ravens - for BWC interrupts */
            /* 2. dummy for MPF registers */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000, 1*_1M)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {/*start of MG unit interrupts */
        GT_U32 ii;
        for(ii = 0 ; ii < 4 ; ii++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.globalInterruptCause             = 0x00000030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.globalInterruptMask              = 0x00000034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.mgInternalInterruptCause         = 0x00000038;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.mgInternalInterruptMask          = 0x0000003c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.portsInterruptCause              = 0x00000080;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.portsInterruptMask               = 0x00000084;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.mg1InternalInterruptCause        = 0x0000009c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.mg1InternalInterruptMask         = 0x000000A0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.dfxInterruptCause                = 0x000000AC;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.dfxInterruptMask                 = 0x000000B0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.ports1InterruptsSummaryCause     = 0x00000150;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.ports1InterruptsSummaryMask      = 0x00000154;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.functionalInterruptsSummaryCause = 0x000003F8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.functionalInterruptsSummaryMask  = 0x000003FC;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.functional1InterruptsSummaryCause= 0x000003F0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[ii].globalInterrupt.functional1InterruptsSummaryMask = 0x000003F4;
        }
    }
}

/**
* @internal smemHawkActiveWriteToSdwTxRxTraining function
* @endinternal
*
* @brief   Set Tx/Rx train complete after initiating training
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemHawkActiveWriteToSdwTxRxTraining
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                  *inMemPtr
)
{
    GT_U32  regAddr;

    /* data to be written */
    *memPtr = *inMemPtr;

    /* calculate address */
    regAddr = address + 0x1C0; /* SDW Lane %c Status 1 */

    smemRegFldSet(devObjPtr, regAddr, 2/* tx_train_complete*/ ,1, (*inMemPtr & (1<<1)) ? 1 : 0/*tx_train_enable*/);
    smemRegFldSet(devObjPtr, regAddr, 0/* rx_train_complete*/ ,1, (*inMemPtr & (1<<0)) ? 1 : 0/*rx_train_enable*/);

    return;
}


/**
* @internal smemHawkActiveWriteToSdwExpectedChecksum1 function
* @endinternal
*
* @brief   set the 'actual' checksum according to hint of expected value from the CPU
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemHawkActiveWriteToSdwExpectedChecksum1
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                  *inMemPtr
)
{
    /* data to be written */
    *memPtr = *inMemPtr;

    /* support function : API_C112GX4_ProgCmnXData      of the CPSS */
    /* support function : API_C28GP4X1_DownloadFirmware of the CPSS */
    /* support function : API_C28GP4X1_ProgCmnXData     of the CPSS */


    /* 0x20000 + 0xA3A8 : actualChecksum */
    smemRegSet (devObjPtr, address + 0x4, *memPtr);
    /* 0x20000 + 0xA3AC : checksumPass */
    smemRegSet (devObjPtr, address + 0x8, (1<<1));

    return;
}

/**
* @internal smemHawkActiveWriteToSdwExpectedChecksum2 function
* @endinternal
*
* @brief   set the 'actual' checksum according to hint of expected value from the CPU
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemHawkActiveWriteToSdwExpectedChecksum2
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                  *inMemPtr
)
{

    /* data to be written */
    *memPtr = *inMemPtr;

    /* support function : API_C112GX4_ProgLaneXData of the CPSS */

    /* 0x20000 + 0x22E8 : actualChecksum */
    smemRegSet (devObjPtr, address + 0x4, *memPtr);
    /* 0x20000 + 0x2294 : checksumPass */
    smemRegSet (devObjPtr, (address & (~0xFFFF)) + 0x2294 , (1<<29));

    return;
}

ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemHawkActiveReadFromRxDone);
/**
* @internal smemHawkActiveReadFromRxDone function
* @endinternal
*
* @brief   Read from Rx done register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - pointer to the register's memory in the simulation.
* @param[in] sumBit                   - global summary interrupt bit
*
* @param[out] outMemPtr               - Pointer to the memory to copy register's content.
*/
void smemHawkActiveReadFromRxDone
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32                  address,
    IN         GT_U32                  memSize,
    IN         GT_U32                * memPtr,
    IN         GT_UINTPTR              sumBit,
    OUT        GT_U32 *                outMemPtr
)
{
    *outMemPtr = 0x5;/* Rx and Tx */
}

/**
* @internal smemHawkActiveWriteToClearTriggerBit function
* @endinternal
*
* @brief   clear triggered mask
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemHawkActiveWriteToClearTriggerBit
(
    IN  SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,  /* this is the mask to clear */
    IN  GT_U32                  *inMemPtr
)
{

    /* data to be written */
    *memPtr = *inMemPtr &~ (param);

    return;
}

/**
* @internal smemHawkUnitSerdes0 function
* @endinternal
*
* @brief   Allocate address type specific memories - SERDESes
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
/*static*/ void smemHawkUnitSerdes0
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

    {0x20000 + 0xA3A4 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0x22E4 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum2,0},

    {0x20000 + 0x6068 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToClearTriggerBit,(1<<8)/*mask of bit 8*/},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW>SDW Ip %a/
            <SDW> SDW IP TLU/SDW IP Units*/
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x0000022C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000240, 0x0000024C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x0000028C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002A0, 0x000002AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 65536)}

            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW>SDW Ip %a/
            <COMPHY_112>COMPHY_112G_X4/<COMPHY_112> COMPHY_112G_X4_1_2 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000+0x20000, 0x000000F4+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000FC+0x20000, 0x000001A4+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001E8+0x20000, 0x000001F8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200+0x20000, 0x00000218+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000224+0x20000, 0x0000023C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000244+0x20000, 0x00000254+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000264+0x20000, 0x0000027C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000028C+0x20000, 0x000002AC+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002B4+0x20000, 0x00000300+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000308+0x20000, 0x0000030C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400+0x20000, 0x000007B8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800+0x20000, 0x0000094C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000+0x20000, 0x0000114C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400+0x20000, 0x0000154C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800+0x20000, 0x0000194C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00+0x20000, 0x00001D4C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000+0x20000, 0x00002034+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002044+0x20000, 0x00002098+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100+0x20000, 0x00002138+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002140+0x20000, 0x0000214C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002158+0x20000, 0x00002174+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200+0x20000, 0x00002228+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002230+0x20000, 0x000022E8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300+0x20000, 0x0000232C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400+0x20000, 0x00002468+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002470+0x20000, 0x0000285C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002900+0x20000, 0x00002914+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002920+0x20000, 0x00002940+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002C00+0x20000, 0x00002C68+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002D00+0x20000, 0x00002D7C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002E00+0x20000, 0x00002E08+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002F00+0x20000, 0x00002F0C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000+0x20000, 0x00008098+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008400+0x20000, 0x00008420+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A200+0x20000, 0x0000A218+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A220+0x20000, 0x0000A220+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A224+0x20000, 0x0000A228+0x20000)}/*for FW download*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A22C+0x20000, 0x0000A22C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A234+0x20000, 0x0000A244+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A2EC+0x20000, 0x0000A2EC+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A2F8+0x20000, 0x0000A2F8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A300+0x20000, 0x0000A338+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A340+0x20000, 0x0000A340+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A348+0x20000, 0x0000A34C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A354+0x20000, 0x0000A3B8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A3F8+0x20000, 0x0000A408+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E000+0x20000, 0x0000Effc+0x20000)}/*FW download memory : C112GX4_XDATA_CMN_BASE_ADDR  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000+0x20000, 0x00006ffc+0x20000)}/*FW download memory : C112GX4_XDATA_LANE_BASE_ADDR */

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW>SDW Ip %a/
            <SDW> SDW IP TLU/SDW IP Units*/
             {DUMMY_NAME_PTR_CNS,            0x00000000,           0x00080000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,           0x00000038,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,           0x00030000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,           0x80000400,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,           0x03a00000,      4,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000500,           0xffff0000,      1,    0x4  }

            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW>SDW Ip %a/
            <COMPHY_112>COMPHY_112G_X4/<COMPHY_112> COMPHY_112G_X4_1_2 */
            ,{DUMMY_NAME_PTR_CNS,            0x00000000+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c+0x20000,   0x00000015,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040+0x20000,   0x0000000d,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044+0x20000,   0x00000005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048+0x20000,   0x00000015,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000004c+0x20000,   0x00000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050+0x20000,   0x0000000f,      2,    0x10  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068+0x20000,   0x00000028,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000074+0x20000,   0x0000000f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000078+0x20000,   0x00000003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000007c+0x20000,   0x0000000c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000080+0x20000,   0x00000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000088+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000008c+0x20000,   0x0000000f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000090+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094+0x20000,   0x00000005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000098+0x20000,   0x00000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000a0+0x20000,   0x00000009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000ac+0x20000,   0x0000000f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000b0+0x20000,   0x0000000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000b4+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000b8+0x20000,   0x00000003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000bc+0x20000,   0x0000000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c0+0x20000,   0x0000003f,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c4+0x20000,   0x00000005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000cc+0x20000,   0x00000003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d0+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d4+0x20000,   0x0000003f,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000e0+0x20000,   0x00000007,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000000fc+0x20000,   0x00000002,      2,    0x14  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114+0x20000,   0x0000003c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118+0x20000,   0x00000006,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000128+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000134+0x20000,   0x00000003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000138+0x20000,   0x00000077,      3,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000148+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000150+0x20000,   0x00000055,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000158+0x20000,   0x00000040,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000015c+0x20000,   0x00000007,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000164+0x20000,   0x00000051,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000016c+0x20000,   0x00000020,      5,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000180+0x20000,   0x00000077,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000190+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000198+0x20000,   0x0000000f,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000001a0+0x20000,   0x00000002,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000001e8+0x20000,   0x00000002,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000001f0+0x20000,   0x00000030,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000001f4+0x20000,   0x00000032,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000001f8+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000200+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000208+0x20000,   0x0000002d,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c+0x20000,   0x00000024,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210+0x20000,   0x00000001,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000214+0x20000,   0x0000006a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000228+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000022c+0x20000,   0x0000000c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000230+0x20000,   0x00000076,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000234+0x20000,   0x000000a9,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000238+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000023c+0x20000,   0x00000046,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000244+0x20000,   0x00000009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000248+0x20000,   0x00000004,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000264+0x20000,   0x000000f0,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000270+0x20000,   0x000000f0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000027c+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000002b4+0x20000,   0x00000002,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000002c4+0x20000,   0x00000003,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000002d4+0x20000,   0x00000001,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000002e4+0x20000,   0x000000a0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000002e8+0x20000,   0x0000009c,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000030c+0x20000,   0x00000032,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000408+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000040c+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000410+0x20000,   0x00000004,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000418+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000041c+0x20000,   0x000000e6,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000420+0x20000,   0x000000dc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000428+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000042c+0x20000,   0x000000a0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000434+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000438+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000043c+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000440+0x20000,   0x0000007c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000444+0x20000,   0x000000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000450+0x20000,   0x000000c9,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000454+0x20000,   0x0000003f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000045c+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000464+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000468+0x20000,   0x00000081,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000046c+0x20000,   0x0000002c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000470+0x20000,   0x00000070,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000474+0x20000,   0x000000be,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000488+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000048c+0x20000,   0x00000049,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000490+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000494+0x20000,   0x000000bd,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000004b0+0x20000,   0x000000b2,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000004b4+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000004b8+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000004bc+0x20000,   0x0000007c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000004c0+0x20000,   0x000000fc,      2,    0x64  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000520+0x20000,   0x0000005f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000548+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000054c+0x20000,   0x00000044,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000550+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000554+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000558+0x20000,   0x000000ec,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000055c+0x20000,   0x00000080,      2,    0xc   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000564+0x20000,   0x00000090,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000570+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000574+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000578+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000057c+0x20000,   0x0000000f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000580+0x20000,   0x000000f8,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000584+0x20000,   0x000000ac,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000058c+0x20000,   0x0000006c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000594+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000598+0x20000,   0x00000048,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000059c+0x20000,   0x000000ac,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005a0+0x20000,   0x0000006c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005a8+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005ac+0x20000,   0x00000088,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005b0+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005b4+0x20000,   0x0000007e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005b8+0x20000,   0x000000fe,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005d4+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005d8+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005dc+0x20000,   0x00000021,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005e0+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005e4+0x20000,   0x00000038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005e8+0x20000,   0x00000018,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005ec+0x20000,   0x00000042,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005f0+0x20000,   0x000000ec,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005f4+0x20000,   0x000000dc,      2,    0xc   }
            ,{DUMMY_NAME_PTR_CNS,            0x000005fc+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000610+0x20000,   0x00000024,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000614+0x20000,   0x000000e4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000618+0x20000,   0x00000030,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c+0x20000,   0x00000018,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000620+0x20000,   0x000000dc,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000634+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000638+0x20000,   0x000000a0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000063c+0x20000,   0x00000030,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000640+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000644+0x20000,   0x0000003c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000648+0x20000,   0x00000078,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000064c+0x20000,   0x000000dc,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000660+0x20000,   0x000000c5,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000664+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000668+0x20000,   0x00000024,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000066c+0x20000,   0x000000cc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000670+0x20000,   0x0000001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000674+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000678+0x20000,   0x000000e0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000067c+0x20000,   0x000000bc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000690+0x20000,   0x000000c5,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000694+0x20000,   0x00000040,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000698+0x20000,   0x0000001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000069c+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006a0+0x20000,   0x0000002c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006a4+0x20000,   0x000000e0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006a8+0x20000,   0x000000bc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006bc+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006c0+0x20000,   0x00000060,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006c4+0x20000,   0x0000001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006c8+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006cc+0x20000,   0x0000002c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006d0+0x20000,   0x000000e0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006d4+0x20000,   0x000000bc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006e8+0x20000,   0x000000c5,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006f0+0x20000,   0x0000001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006f4+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006f8+0x20000,   0x0000002c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000006fc+0x20000,   0x000000e0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700+0x20000,   0x000000bc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000714+0x20000,   0x0000008c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000718+0x20000,   0x0000009c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000724+0x20000,   0x0000007e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000728+0x20000,   0x000000fe,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000073c+0x20000,   0x00000081,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000740+0x20000,   0x00000064,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000744+0x20000,   0x00000014,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000748+0x20000,   0x000000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000750+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000758+0x20000,   0x00000081,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000075c+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000760+0x20000,   0x000000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000768+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000778+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000780+0x20000,   0x0000002a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000784+0x20000,   0x00000005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000788+0x20000,   0x000000ff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000794+0x20000,   0x0000004f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000007a0+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000007a4+0x20000,   0x00000012,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000007a8+0x20000,   0x00000014,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000007ac+0x20000,   0x0000003e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000007b0+0x20000,   0x000000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000804+0x20000,   0x000000b2,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000808+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000080c+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000810+0x20000,   0x0000007c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000814+0x20000,   0x000000fc,      2,    0x64  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000874+0x20000,   0x0000005f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008a4+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008a8+0x20000,   0x00000041,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008b0+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008b4+0x20000,   0x0000003c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008b8+0x20000,   0x00000089,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008c0+0x20000,   0x0000007e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008c4+0x20000,   0x000000fe,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008d0+0x20000,   0x000000a8,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008d4+0x20000,   0x0000005f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008d8+0x20000,   0x000000fe,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008e0+0x20000,   0x00000009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008e4+0x20000,   0x00000028,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008e8+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008f8+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000008fc+0x20000,   0x000000c1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900+0x20000,   0x0000002e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000904+0x20000,   0x00000049,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000908+0x20000,   0x00000040,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000090c+0x20000,   0x0000007c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000910+0x20000,   0x000000dc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000918+0x20000,   0x000000bc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000092c+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000930+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000934+0x20000,   0x00000022,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000938+0x20000,   0x00000014,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000093c+0x20000,   0x00000060,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000940+0x20000,   0x0000007c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000944+0x20000,   0x000000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001104+0x20000,   0x00000006,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001504+0x20000,   0x00000006,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001904+0x20000,   0x00000006,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d04+0x20000,   0x00000006,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000+0x20000,   0x01000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002008+0x20000,   0x48000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002014+0x20000,   0x00001000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000201c+0x20000,   0x00100000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002020+0x20000,   0x20000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002024+0x20000,   0x00002000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002034+0x20000,   0x10000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002048+0x20000,   0x0000004f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000204c+0x20000,   0x8000ff00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002050+0x20000,   0x8000003f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000206c+0x20000,   0x00000320,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002070+0x20000,   0x00000018,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002074+0x20000,   0x40900000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002078+0x20000,   0x05cf0000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000207c+0x20000,   0x00046000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002080+0x20000,   0x08000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002088+0x20000,   0x0000a556,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000208c+0x20000,   0x0002fffd,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002090+0x20000,   0x22004000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002094+0x20000,   0x33000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002100+0x20000,   0x00300021,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002104+0x20000,   0x10000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108+0x20000,   0x48000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000211c+0x20000,   0x68000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002120+0x20000,   0xf1800c00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000215c+0x20000,   0x04000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002160+0x20000,   0x2604e800,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002164+0x20000,   0x02020000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002168+0x20000,   0x0000064f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002170+0x20000,   0x00070e66,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208+0x20000,   0x80000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c+0x20000,   0x07ffe000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002278+0x20000,   0x00010001,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002290+0x20000,   0x00050005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002294+0x20000,   0x00000005,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000229c+0x20000,   0x0000000a,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000022dc+0x20000,   0x00000400,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000022e4+0x20000,   0xffffffff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300+0x20000,   0x10104000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002304+0x20000,   0x03000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002314+0x20000,   0xffffffff,      2,    0x18  }
            ,{DUMMY_NAME_PTR_CNS,            0x00002328+0x20000,   0xffff0000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400+0x20000,   0x040014ff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002404+0x20000,   0x00a00c04,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000240c+0x20000,   0x003f00b1,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002410+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002418+0x20000,   0x03f00401,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000241c+0x20000,   0x000404b5,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002424+0x20000,   0x80000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000242c+0x20000,   0x68222291,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002430+0x20000,   0x666a6888,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002434+0x20000,   0x2469a162,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002438+0x20000,   0x24624668,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000243c+0x20000,   0x00244246,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002440+0x20000,   0xe6122483,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002444+0x20000,   0x408820aa,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000244c+0x20000,   0x0f0b0804,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002450+0x20000,   0xffd38d47,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002454+0x20000,   0x00210021,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002458+0x20000,   0x00000483,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000245c+0x20000,   0xb0c08400,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002468+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002478+0x20000,   0x00246268,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000024c4+0x20000,   0x06060000,      4,    0x50  }
            ,{DUMMY_NAME_PTR_CNS,            0x000024c8+0x20000,   0x00000606,      4,    0x50  }
            ,{DUMMY_NAME_PTR_CNS,            0x00002940+0x20000,   0x000000ff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c00+0x20000,   0x00000840,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c0c+0x20000,   0x0c700000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c14+0x20000,   0x27100503,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c18+0x20000,   0x0bb70013,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c20+0x20000,   0x00830000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c24+0x20000,   0x01090842,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c28+0x20000,   0xbf3fa03f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c2c+0x20000,   0x00150016,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c30+0x20000,   0x05260009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c34+0x20000,   0x00000026,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c3c+0x20000,   0x002f2f3f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c40+0x20000,   0x000f0900,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c44+0x20000,   0x00000600,      2,    0xc   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c4c+0x20000,   0x00007700,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c54+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00002c64+0x20000,   0x00030000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000600c+0x20000,   0x00000300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006014+0x20000,   0x37080c00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006018+0x20000,   0x37083708,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000602c+0x20000,   0x03000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006030+0x20000,   0xf0000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006038+0x20000,   0x0c000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006044+0x20000,   0x00000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000604c+0x20000,   0xff01e200,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006058+0x20000,   0x0000c000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006090+0x20000,   0x04000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006094+0x20000,   0x087e8874,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000609c+0x20000,   0x000003e8,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000060a0+0x20000,   0x000000ff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000060fc+0x20000,   0x000000aa,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006300+0x20000,   0x00009300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006304+0x20000,   0x00163209,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006308+0x20000,   0x03ffff12,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006314+0x20000,   0x00000f00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006500+0x20000,   0x00200000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006504+0x20000,   0x00020014,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006508+0x20000,   0x000a0028,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000650c+0x20000,   0x0001000c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006510+0x20000,   0x0007da00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006514+0x20000,   0x09af0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006520+0x20000,   0x00210100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006524+0x20000,   0x00020015,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006528+0x20000,   0x000a0029,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000652c+0x20000,   0x0001000c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006530+0x20000,   0x0007b600,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006534+0x20000,   0x09fb0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006540+0x20000,   0x00270203,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006544+0x20000,   0x00020018,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006548+0x20000,   0x000c0031,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000654c+0x20000,   0x0001000c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006550+0x20000,   0x0007f900,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006554+0x20000,   0x0bcd0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006560+0x20000,   0x00140303,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006564+0x20000,   0x01010019,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006568+0x20000,   0x000d0032,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000656c+0x20000,   0x00010006,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006570+0x20000,   0x0007da00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006574+0x20000,   0x0c1b0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006580+0x20000,   0x003e0403,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006584+0x20000,   0x0103001a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006588+0x20000,   0x000d0034,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000658c+0x20000,   0x2000011f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006590+0x20000,   0x0007c904,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006594+0x20000,   0x0c7b0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065a0+0x20000,   0x00550504,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065a4+0x20000,   0x0104001b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065a8+0x20000,   0x000d0035,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065ac+0x20000,   0x0000020f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065b0+0x20000,   0x0007b900,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065b4+0x20000,   0x0cdb0300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065c0+0x20000,   0x002c0604,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065c4+0x20000,   0x0102001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065c8+0x20000,   0x000e0037,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065cc+0x20000,   0x00010009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065d0+0x20000,   0x0007b600,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065d4+0x20000,   0x0d510200,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065e0+0x20000,   0x002d0704,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065e4+0x20000,   0x0102001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065e8+0x20000,   0x000e0038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065ec+0x20000,   0x00010009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065f0+0x20000,   0x0007e300,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000065f4+0x20000,   0x0d9d0200,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006600+0x20000,   0x002d0804,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006604+0x20000,   0x0102001c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006608+0x20000,   0x000e0038,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000660c+0x20000,   0x20010009,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006610+0x20000,   0x0007da09,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006614+0x20000,   0x0d8f0200,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006624+0x20000,   0x00040100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006628+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000662c+0x20000,   0x03070100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006630+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006634+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006638+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000663c+0x20000,   0x0000150a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006640+0x20000,   0x00020000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006644+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006648+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000664c+0x20000,   0x01640002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006650+0x20000,   0x00010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006654+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000665c+0x20000,   0x03000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006660+0x20000,   0x0101ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006664+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006668+0x20000,   0x00030103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000666c+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006670+0x20000,   0x02060103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006674+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006678+0x20000,   0x04010301,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000667c+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006680+0x20000,   0x00001a4d,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006684+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006688+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000668c+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006690+0x20000,   0x003e0003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006694+0x20000,   0x01010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006698+0x20000,   0x01010201,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000669c+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066a0+0x20000,   0x03000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066a4+0x20000,   0x0101a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066a8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066ac+0x20000,   0x00020101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066b0+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066b4+0x20000,   0x01050101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066b8+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066bc+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066c0+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066c4+0x20000,   0x000015b3,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066c8+0x20000,   0x07070002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066cc+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066d0+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066d4+0x20000,   0x00890101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066d8+0x20000,   0x06010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066dc+0x20000,   0x06060206,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066e0+0x20000,   0x0c0c0c0c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066e4+0x20000,   0x0300010c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066e8+0x20000,   0x0100ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066ec+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066f0+0x20000,   0x01020103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066f4+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066f8+0x20000,   0x01050103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000066fc+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006700+0x20000,   0x04010301,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006704+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006708+0x20000,   0x00001a4d,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000670c+0x20000,   0x07070002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006710+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006714+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006718+0x20000,   0x003e0003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000671c+0x20000,   0x06010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006720+0x20000,   0x06060206,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006724+0x20000,   0x0c0c0c0c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006728+0x20000,   0x0300010c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000672c+0x20000,   0x0100a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006730+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006734+0x20000,   0x01010101,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000673c+0x20000,   0x00040101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006740+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006744+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006748+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000674c+0x20000,   0x000015b3,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006750+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006754+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006758+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000675c+0x20000,   0x00890101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006760+0x20000,   0x0c010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006764+0x20000,   0x0c0c040c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006768+0x20000,   0x08080808,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000676c+0x20000,   0x01000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006770+0x20000,   0x0100ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006774+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006778+0x20000,   0x01010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000677c+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006780+0x20000,   0x00040102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006784+0x20000,   0x05050208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006788+0x20000,   0x04010301,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000678c+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006790+0x20000,   0x000019a5,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006794+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006798+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000679c+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a0+0x20000,   0x00a20101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a4+0x20000,   0x0c010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a8+0x20000,   0x0c0c040c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067ac+0x20000,   0x08080808,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067b0+0x20000,   0x01000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067b4+0x20000,   0x0100a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067b8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067bc+0x20000,   0x01010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067c0+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067c4+0x20000,   0x00040103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067c8+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067cc+0x20000,   0x04010301,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067d0+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067d4+0x20000,   0x00001a4d,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067d8+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067dc+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067e0+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067e4+0x20000,   0x003e0003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067e8+0x20000,   0x0c010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067ec+0x20000,   0x0c0c040c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067f0+0x20000,   0x08080808,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067f4+0x20000,   0x01000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067f8+0x20000,   0x0100a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000067fc+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006800+0x20000,   0x04000104,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006804+0x20000,   0x02010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000680c+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006810+0x20000,   0x04000306,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006814+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006818+0x20000,   0x00001b20,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000681c+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006820+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006824+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006828+0x20000,   0x01560103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000682c+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006830+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006834+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006838+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000683c+0x20000,   0x0100a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006840+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006844+0x20000,   0x04000106,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006848+0x20000,   0x02010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006850+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006854+0x20000,   0x04000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006858+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000685c+0x20000,   0x00001cef,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006860+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006864+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006868+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000686c+0x20000,   0x00890002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006870+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006874+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006878+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000687c+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006880+0x20000,   0x01009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006884+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006888+0x20000,   0x04000107,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000688c+0x20000,   0x02010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006894+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006898+0x20000,   0x04000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000689c+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068a0+0x20000,   0x00001d97,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068a4+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068a8+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068ac+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068b0+0x20000,   0x01460003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068b4+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068b8+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068bc+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068c0+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068c4+0x20000,   0x01009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068c8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068cc+0x20000,   0x04000105,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068d0+0x20000,   0x03010004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068d8+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068dc+0x20000,   0x04000706,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068e0+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068e4+0x20000,   0x00001bf2,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068e8+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068ec+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068f0+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068f4+0x20000,   0x00420003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068f8+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000068fc+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006900+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006904+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006908+0x20000,   0x00009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000690c+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006910+0x20000,   0x04000108,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006914+0x20000,   0x03010004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000691c+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006920+0x20000,   0x04000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006924+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006928+0x20000,   0x00001d76,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000692c+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006930+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006934+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006938+0x20000,   0x01460003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000693c+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006940+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006944+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006948+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000694c+0x20000,   0x00009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006950+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006954+0x20000,   0x00030100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006958+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000695c+0x20000,   0x02060100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006960+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006964+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006968+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000696c+0x20000,   0x0000150a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006970+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006974+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006978+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000697c+0x20000,   0x01640002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006980+0x20000,   0x01010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006984+0x20000,   0x01010201,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006988+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000698c+0x20000,   0x03000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006990+0x20000,   0x0101ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006994+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006998+0x20000,   0x04000101,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000699c+0x20000,   0x02010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069a4+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069a8+0x20000,   0x04000103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069ac+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069b0+0x20000,   0x000015b3,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069b4+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069b8+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069bc+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069c0+0x20000,   0x00890101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069c4+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069c8+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069cc+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069d0+0x20000,   0x01000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069d4+0x20000,   0x0100ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069d8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069dc+0x20000,   0x07000105,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069e0+0x20000,   0x03000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069e4+0x20000,   0x04000105,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069e8+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069ec+0x20000,   0x04000706,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069f0+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069f4+0x20000,   0x00001bf2,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069f8+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x000069fc+0x20000,   0x07070000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a00+0x20000,   0x00000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a04+0x20000,   0x00420003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a08+0x20000,   0x0f010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a0c+0x20000,   0x0f0f0c0f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a10+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a14+0x20000,   0x00000202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a18+0x20000,   0x00009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a20+0x20000,   0x07000108,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a24+0x20000,   0x03000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a28+0x20000,   0x04000108,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a2c+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a30+0x20000,   0x04000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a34+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a38+0x20000,   0x00001d76,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a3c+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a40+0x20000,   0x07070000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a44+0x20000,   0x00000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a48+0x20000,   0x01460003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a4c+0x20000,   0x0f010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a50+0x20000,   0x0f0f0c0f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a54+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a58+0x20000,   0x00000202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a5c+0x20000,   0x00009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a64+0x20000,   0x04000105,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a68+0x20000,   0x02010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a70+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a74+0x20000,   0x04000706,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a78+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a7c+0x20000,   0x00001b04,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a80+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a84+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a88+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a8c+0x20000,   0x01560103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a90+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a94+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a98+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006a9c+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006aa0+0x20000,   0x01009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006aa4+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006aa8+0x20000,   0x04010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006aac+0x20000,   0x01010104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ab0+0x20000,   0x00040104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ab4+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ab8+0x20000,   0x04010301,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006abc+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ac0+0x20000,   0x00001b20,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ac4+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ac8+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006acc+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ad0+0x20000,   0x01560103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ad4+0x20000,   0x0c010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ad8+0x20000,   0x0c0c040c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006adc+0x20000,   0x08080808,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ae0+0x20000,   0x00000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ae4+0x20000,   0x0100a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ae8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006aec+0x20000,   0x04000104,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006af0+0x20000,   0x03010004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006af8+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006afc+0x20000,   0x04000306,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b00+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b04+0x20000,   0x00001b20,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b08+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b0c+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b10+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b14+0x20000,   0x01560103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b18+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b1c+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b20+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b24+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b28+0x20000,   0x0000a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b2c+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b30+0x20000,   0x07000104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b34+0x20000,   0x03000007,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b38+0x20000,   0x04000104,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b3c+0x20000,   0x0505010b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b40+0x20000,   0x04000306,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b44+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b48+0x20000,   0x00001b20,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b4c+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b50+0x20000,   0x07070000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b54+0x20000,   0x00000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b58+0x20000,   0x01560103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b5c+0x20000,   0x0f010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b60+0x20000,   0x0f0f0c0f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b64+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b68+0x20000,   0x00000202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b6c+0x20000,   0x0000a925,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b74+0x20000,   0x00030101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b78+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b7c+0x20000,   0x02060101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b80+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b84+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b88+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b8c+0x20000,   0x000015b3,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b90+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b94+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b98+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006b9c+0x20000,   0x00890101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ba0+0x20000,   0x01010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ba4+0x20000,   0x01010201,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006ba8+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bac+0x20000,   0x03000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bb0+0x20000,   0x0101ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bb4+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bb8+0x20000,   0x01020100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bbc+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bc0+0x20000,   0x01050100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bc4+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bc8+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bcc+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bd0+0x20000,   0x0000150a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bd4+0x20000,   0x07070002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bd8+0x20000,   0x00000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bdc+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006be0+0x20000,   0x01640002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006be4+0x20000,   0x01010103,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006be8+0x20000,   0x01010201,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bec+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bf0+0x20000,   0x03000101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bf4+0x20000,   0x0100ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bf8+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006bfc+0x20000,   0x01010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c00+0x20000,   0x01010101,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c04+0x20000,   0x00040100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c08+0x20000,   0x04040208,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c0c+0x20000,   0x04010100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c10+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c14+0x20000,   0x0000150a,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c18+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c1c+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c20+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c24+0x20000,   0x01640002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c28+0x20000,   0x0c010102,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c2c+0x20000,   0x0c0c040c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c30+0x20000,   0x08080808,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c34+0x20000,   0x01000008,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c38+0x20000,   0x0100ad25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c3c+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c40+0x20000,   0x04000107,      2,    0x8   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c44+0x20000,   0x03010004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c4c+0x20000,   0x0707000e,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c50+0x20000,   0x04000707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c54+0x20000,   0x02050405,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c58+0x20000,   0x00001d97,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c5c+0x20000,   0x07070707,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c60+0x20000,   0x00000100,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c64+0x20000,   0x01010000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c68+0x20000,   0x01460003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c6c+0x20000,   0x0f000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c70+0x20000,   0x0c0c090f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c74+0x20000,   0x02020202,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c78+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c7c+0x20000,   0x00009d25,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00006c80+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008000+0x20000,   0x000000aa,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008004+0x20000,   0x0000006b,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008008+0x20000,   0x0000001f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000800c+0x20000,   0x00000030,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008010+0x20000,   0x0000006c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008014+0x20000,   0x00000099,      8,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008034+0x20000,   0x00000033,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008044+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008048+0x20000,   0x0000004c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000804c+0x20000,   0x00000074,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008054+0x20000,   0x00000003,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008058+0x20000,   0x000000c4,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000805c+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008068+0x20000,   0x00000033,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000806c+0x20000,   0x00000019,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008070+0x20000,   0x000000f8,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008074+0x20000,   0x00000088,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008078+0x20000,   0x00000086,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000807c+0x20000,   0x000000f0,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000808c+0x20000,   0x000000ff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008090+0x20000,   0x000000c0,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008094+0x20000,   0x00000080,      2,    0x374 }
            ,{DUMMY_NAME_PTR_CNS,            0x00008098+0x20000,   0x0000009c,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000840c+0x20000,   0x00000022,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008410+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x00008414+0x20000,   0x0000003f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000841c+0x20000,   0x00000070,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a220+0x20000,   0x00000555,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a22c+0x20000,   0x00000004,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a2f8+0x20000,   0x006c0000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a308+0x20000,   0x0000ff01,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a314+0x20000,   0x0c000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a318+0x20000,   0x00000020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a320+0x20000,   0x00000002,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a328+0x20000,   0x00000800,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a32c+0x20000,   0x0000004f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a330+0x20000,   0x00000080,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a334+0x20000,   0x0080001f,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a340+0x20000,   0x00310a00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a364+0x20000,   0x00010001,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a374+0x20000,   0x80000014,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a398+0x20000,   0xfe000000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3a0+0x20000,   0x00000400,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3a4+0x20000,   0xffffffff,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3f8+0x20000,   0x43004f10,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3fc+0x20000,   0x40100000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e000+0x20000,   0x1f1f1f01,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e004+0x20000,   0x20212021,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e008+0x20000,   0x01011010,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e00c+0x20000,   0x20202020,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e018+0x20000,   0x04000400,      2,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e608+0x20000,   0x008000fc,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e60c+0x20000,   0x03fe8000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e614+0x20000,   0x0e0f0c00,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e620+0x20000,   0x06131015,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e624+0x20000,   0x06050000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e628+0x20000,   0x00000001,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e634+0x20000,   0x20202020,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e644+0x20000,   0x20080000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e650+0x20000,   0x06050000,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e658+0x20000,   0x00000010,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e65c+0x20000,   0x00000190,      1,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e68c+0x20000,   0x000000aa,      1,    0x4   }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitSerdes1 function
* @endinternal
*
* @brief   Allocate address type specific memories - SERDESes
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitSerdes1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {0x00000080, SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwTxRxTraining,0},
    {0x00000220, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0xc ,NULL,0},

    {0x20000 + 0xA358 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0xA224 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0x22F8 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum2,0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW_28G>SDW IP/
            <SDW_28G> SDW IP TLU/SDW IP Units*/
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E0, 0x000000E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000240, 0x00000240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x00000260)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x00000280)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002A0, 0x000002A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}

            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW_28G>SDW IP/
            <COMPHY_28G_1LANE_IP> COMPHY_28G_PIPE4_RPLL_1P2V_1_0/units*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000+0x20000, 0x00000080+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200+0x20000, 0x00000274+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400+0x20000, 0x0000049C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800+0x20000, 0x0000089C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000+0x20000, 0x0000103C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000+0x20000, 0x00002034+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000203C+0x20000, 0x00002058+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002060+0x20000, 0x00002088+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100+0x20000, 0x00002150+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002158+0x20000, 0x00002158+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002160+0x20000, 0x00002174+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200+0x20000, 0x0000220C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002230+0x20000, 0x0000231C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400+0x20000, 0x00002424+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002430+0x20000, 0x000024C4+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000024D0+0x20000, 0x00002520+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002530+0x20000, 0x00002530+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002540+0x20000, 0x00002554+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002560+0x20000, 0x00002580+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000025F0+0x20000, 0x000025F4+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002600+0x20000, 0x00002618+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002620+0x20000, 0x00002624+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002630+0x20000, 0x00002648+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000+0x20000, 0x0000407C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200+0x20000, 0x0000425C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008200+0x20000, 0x00008358+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000+0x20000, 0x0000A000+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A008+0x20000, 0x0000A01C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A024+0x20000, 0x0000A030+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A200+0x20000, 0x0000A244+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A2DC+0x20000, 0x0000A2F8+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A300+0x20000, 0x0000A340+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A348+0x20000, 0x0000A34C+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A354+0x20000, 0x0000A3B4+0x20000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A3F0+0x20000, 0x0000A3FC+0x20000)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E000+0x20000, 0x0000Effc+0x20000)}/*FW download memory : C112GX4_XDATA_CMN_BASE_ADDR  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000+0x20000, 0x00006ffc+0x20000)}/*FW download memory : C112GX4_XDATA_LANE_BASE_ADDR */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000+0x20000, 0x0001fffc+0x20000)}/*FW download memory : C28GP4X1_FW_BASE_ADDR */
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW_28G>SDW IP/
            <SDW_28G> SDW IP TLU/SDW IP Units*/
             {DUMMY_NAME_PTR_CNS,            0x00000000,           0x00080000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,           0x00000038,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,           0x00030000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,           0x80000400,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,           0x03a00000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000500,           0xffff0000,      1,    0x4  }

            /*/Cider/Switching Dies/Hawk/Hawk {Current}/Hawk/Core/EPI/<SDW_28G>SDW IP/
            <COMPHY_28G_1LANE_IP> COMPHY_28G_PIPE4_RPLL_1P2V_1_0/units*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000000+0x20000,   0x0000000f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004+0x20000,   0x0000005c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008+0x20000,   0x00000040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c+0x20000,   0x00000006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000010+0x20000,   0x00000010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c+0x20000,   0x000000e0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020+0x20000,   0x00000083,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000024+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002c+0x20000,   0x0000003c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030+0x20000,   0x00000086,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034+0x20000,   0x000000c6,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000038+0x20000,   0x00000078,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000003c+0x20000,   0x00000022,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044+0x20000,   0x00000008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048+0x20000,   0x000000fc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000054+0x20000,   0x0000001f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000058+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000005c+0x20000,   0x0000000f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000060+0x20000,   0x000000fc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068+0x20000,   0x00000010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000006c+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000070+0x20000,   0x000000a0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000074+0x20000,   0x000000e4,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000078+0x20000,   0x000000f0,      3,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000200+0x20000,   0x0000006c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000204+0x20000,   0x0000000f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000208+0x20000,   0x000000f1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c+0x20000,   0x000000c4,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210+0x20000,   0x0000000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000214+0x20000,   0x00000030,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000218+0x20000,   0x0000001b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000021c+0x20000,   0x00000081,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000220+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000224+0x20000,   0x00000017,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000228+0x20000,   0x00000081,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000022c+0x20000,   0x00000033,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000230+0x20000,   0x00000040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000234+0x20000,   0x0000005c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000238+0x20000,   0x000000ee,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000240+0x20000,   0x00000078,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000248+0x20000,   0x0000007f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000024c+0x20000,   0x00000040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000258+0x20000,   0x000000e0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000025c+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000260+0x20000,   0x00000090,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000264+0x20000,   0x000000b0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000268+0x20000,   0x000000f0,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000+0x20000,   0x00000008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001004+0x20000,   0x00000028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001008+0x20000,   0x000000f3,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000100c+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001010+0x20000,   0x00000070,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001014+0x20000,   0x00000026,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001018+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001020+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001024+0x20000,   0x000000f0,      7,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000+0x20000,   0x29000800,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002004+0x20000,   0x00100009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000201c+0x20000,   0x003c0000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002020+0x20000,   0x20000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002024+0x20000,   0x00000400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002040+0x20000,   0x00000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000204c+0x20000,   0x0000ff00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002060+0x20000,   0x00001000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002064+0x20000,   0x45cf0004,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002068+0x20000,   0x002492c9,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000206c+0x20000,   0x00cc3e82,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002070+0x20000,   0x05340138,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002074+0x20000,   0x00b40000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000207c+0x20000,   0x3001ff00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002080+0x20000,   0x04a80000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002084+0x20000,   0x0009e000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002100+0x20000,   0x00304000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000211c+0x20000,   0xe8000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002120+0x20000,   0x741ffef0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002124+0x20000,   0x33008042,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002134+0x20000,   0x040fea99,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002138+0x20000,   0x2b9a2789,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002144+0x20000,   0x10000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000214c+0x20000,   0x00000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002150+0x20000,   0x001f0000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002160+0x20000,   0x2604e800,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002164+0x20000,   0x02024400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002168+0x20000,   0x0000064f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000216c+0x20000,   0x0b680000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002170+0x20000,   0x00070e66,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208+0x20000,   0x80000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c+0x20000,   0x00fffc00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002278+0x20000,   0x00010001,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002290+0x20000,   0x00050005,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002294+0x20000,   0x00000005,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000229c+0x20000,   0x0000000a,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000022dc+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000022e4+0x20000,   0x10000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000022f8+0x20000,   0xffffffff,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300+0x20000,   0x02492000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002304+0x20000,   0x02003010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400+0x20000,   0x00f014f1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002404+0x20000,   0x00001040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002408+0x20000,   0x00800000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000240c+0x20000,   0x003f1080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002410+0x20000,   0x00000035,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002418+0x20000,   0x00000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000241c+0x20000,   0x24000006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002420+0x20000,   0x68222291,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002424+0x20000,   0x666a6888,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002540+0x20000,   0x66120083,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002544+0x20000,   0x08282155,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000254c+0x20000,   0x0f0b0804,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002550+0x20000,   0xffd38d47,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002554+0x20000,   0x0fff0047,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002610+0x20000,   0x10000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002614+0x20000,   0x07c00000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002630+0x20000,   0x18000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000263c+0x20000,   0x8c000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002640+0x20000,   0x04298bbc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002648+0x20000,   0x04000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004000+0x20000,   0x00010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004008+0x20000,   0x4a810000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000400c+0x20000,   0x1f080601,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004010+0x20000,   0x00040013,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004014+0x20000,   0x68140000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004018+0x20000,   0x00ec0000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000401c+0x20000,   0x0cf80000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004020+0x20000,   0x00000b68,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004024+0x20000,   0x83ed0000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004028+0x20000,   0x02015041,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000402c+0x20000,   0x09e40962,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004030+0x20000,   0x07e70a2d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004034+0x20000,   0x001e09a1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004038+0x20000,   0x020002c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000403c+0x20000,   0x01800240,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004040+0x20000,   0x00050000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004044+0x20000,   0x02450006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004048+0x20000,   0x00070186,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000404c+0x20000,   0x000003c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004050+0x20000,   0x058f4b4f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004054+0x20000,   0x00000100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004058+0x20000,   0x000013ed,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000405c+0x20000,   0x09e40962,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004060+0x20000,   0x07e70a2d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004064+0x20000,   0x001e09a1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004068+0x20000,   0x020002c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000406c+0x20000,   0x01800240,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004070+0x20000,   0x00050000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004074+0x20000,   0x02450006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004078+0x20000,   0x00070186,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000407c+0x20000,   0x000003c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004200+0x20000,   0x00291000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004204+0x20000,   0x40110000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004208+0x20000,   0x00000087,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000420c+0x20000,   0x162820c1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004210+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004214+0x20000,   0x101f0818,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004218+0x20000,   0x02021f1f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000421c+0x20000,   0x0303fcfa,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004220+0x20000,   0x3014011e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004228+0x20000,   0x00010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000422c+0x20000,   0x19300000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004230+0x20000,   0x08180100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004234+0x20000,   0x301f101f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004238+0x20000,   0x00000205,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000423c+0x20000,   0x00000022,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004244+0x20000,   0x00010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004248+0x20000,   0xffffffff,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00004258+0x20000,   0x00000020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000425c+0x20000,   0x00001654,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006008+0x20000,   0x00060000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000600c+0x20000,   0xff0e0300,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006014+0x20000,   0x00000c00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006018+0x20000,   0x00070008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006028+0x20000,   0x030101f3,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000602c+0x20000,   0x280001f3,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006030+0x20000,   0xf2000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006038+0x20000,   0x0c070021,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006044+0x20000,   0x40000008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000604c+0x20000,   0x3f01a200,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006050+0x20000,   0x92800ffd,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006058+0x20000,   0x44013000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000605c+0x20000,   0x00070400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006060+0x20000,   0x04060400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006064+0x20000,   0x3d1e4335,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006068+0x20000,   0x000000fc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000606c+0x20000,   0x1e3d0032,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006070+0x20000,   0x00000003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006074+0x20000,   0x00001e1e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006078+0x20000,   0x1e141414,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006084+0x20000,   0x00000ffc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006088+0x20000,   0x0000fe3c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000608c+0x20000,   0xf4d80040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006090+0x20000,   0x04050000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006098+0x20000,   0x04040000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000609c+0x20000,   0x08020000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000060a0+0x20000,   0x00000053,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000060a4+0x20000,   0x415884c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000060b4+0x20000,   0x0f0f6c9f,      5,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000060fc+0x20000,   0x000000aa,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006304+0x20000,   0x04010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006308+0x20000,   0x00010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000630c+0x20000,   0x0f000f00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006310+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006314+0x20000,   0x01000088,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006318+0x20000,   0x01010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000631c+0x20000,   0x02080307,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006320+0x20000,   0x01000404,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006324+0x20000,   0x05040601,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006328+0x20000,   0x00030203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000632c+0x20000,   0x0a020132,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006330+0x20000,   0x00000015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006334+0x20000,   0x01000000,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000633c+0x20000,   0x01010101,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006344+0x20000,   0x02030101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006348+0x20000,   0x0000a01b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006354+0x20000,   0x03010103,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006358+0x20000,   0x00010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000635c+0x20000,   0x0f000b00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006360+0x20000,   0x01010203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006364+0x20000,   0x01000088,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006368+0x20000,   0x01010300,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000636c+0x20000,   0x010b0206,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006370+0x20000,   0x03000606,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006374+0x20000,   0x05040601,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006378+0x20000,   0x00010203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000637c+0x20000,   0x4d02017d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006380+0x20000,   0x0100001a,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006388+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000638c+0x20000,   0x01010101,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006394+0x20000,   0x03030101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006398+0x20000,   0x0000a021,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063a4+0x20000,   0x02010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063a8+0x20000,   0x00000303,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063ac+0x20000,   0x00010001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063b0+0x20000,   0x01010203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063b4+0x20000,   0x01000088,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063b8+0x20000,   0x01010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063bc+0x20000,   0x02080105,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063c0+0x20000,   0x01000404,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063c4+0x20000,   0x05040601,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063c8+0x20000,   0x01010203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063cc+0x20000,   0xb3020089,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063d0+0x20000,   0x01000015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063d4+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063d8+0x20000,   0x01000100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063dc+0x20000,   0x01010101,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063e4+0x20000,   0x04020001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063e8+0x20000,   0x0000a01b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063f4+0x20000,   0x01010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063f8+0x20000,   0x00010303,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000063fc+0x20000,   0x00010001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006400+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006404+0x20000,   0x01000088,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006408+0x20000,   0x01010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000640c+0x20000,   0x02080004,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006410+0x20000,   0x01000404,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006414+0x20000,   0x05040601,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006418+0x20000,   0x01010403,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000641c+0x20000,   0xb3020089,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006420+0x20000,   0x01000015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006424+0x20000,   0x01010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006428+0x20000,   0x00010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000642c+0x20000,   0x01010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006434+0x20000,   0x07010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006438+0x20000,   0x0000a01b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006444+0x20000,   0x00010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006448+0x20000,   0x00010606,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000644c+0x20000,   0x00010001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006450+0x20000,   0x02000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006454+0x20000,   0x01000088,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006458+0x20000,   0x01010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000645c+0x20000,   0x02080400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006460+0x20000,   0x01000404,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006464+0x20000,   0x05040600,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006468+0x20000,   0x01010403,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000646c+0x20000,   0xb3020089,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006470+0x20000,   0x01000015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006474+0x20000,   0x01010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006478+0x20000,   0x00010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000647c+0x20000,   0x01010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006484+0x20000,   0x0b000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006488+0x20000,   0x0000a01b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006764+0x20000,   0x20202020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006768+0x20000,   0x20200000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000676c+0x20000,   0x00002020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006770+0x20000,   0x20202020,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006774+0x20000,   0x20200000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000677c+0x20000,   0x20202020,      2,    0xc    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006784+0x20000,   0x20202000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000678c+0x20000,   0x00202020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006794+0x20000,   0x16160404,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006798+0x20000,   0x04041616,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000679c+0x20000,   0x16161616,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a0+0x20000,   0x01010000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a4+0x20000,   0x01010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067a8+0x20000,   0x31310101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067ac+0x20000,   0x03033131,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067b0+0x20000,   0x03030303,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067b8+0x20000,   0x08080303,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067bc+0x20000,   0x08080808,     14,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000067f4+0x20000,   0x00020808,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00006800+0x20000,   0x00040004,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008200+0x20000,   0x00000090,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008208+0x20000,   0x00000060,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000820c+0x20000,   0x00000099,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008210+0x20000,   0x00000055,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008214+0x20000,   0x0000006b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008218+0x20000,   0x0000001f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000821c+0x20000,   0x00000028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008220+0x20000,   0x00000084,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008224+0x20000,   0x000000f1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008228+0x20000,   0x00000071,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000822c+0x20000,   0x0000000e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008230+0x20000,   0x00000099,      8,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008250+0x20000,   0x00000058,      8,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008270+0x20000,   0x00000099,      6,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008288+0x20000,   0x000000cc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000828c+0x20000,   0x0000004c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008290+0x20000,   0x00000068,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008294+0x20000,   0x00000070,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008298+0x20000,   0x0000006c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000829c+0x20000,   0x0000006e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082a0+0x20000,   0x00000010,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082a4+0x20000,   0x00000018,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082ac+0x20000,   0x000000f1,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082b0+0x20000,   0x00000068,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082b4+0x20000,   0x000000fb,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082bc+0x20000,   0x00000072,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082c4+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082c8+0x20000,   0x0000001c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082cc+0x20000,   0x00000076,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082d0+0x20000,   0x000000a9,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082d4+0x20000,   0x00000020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082d8+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082dc+0x20000,   0x00000046,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082e0+0x20000,   0x00000092,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082e8+0x20000,   0x00000038,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082ec+0x20000,   0x0000000e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082f0+0x20000,   0x00000008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082f4+0x20000,   0x00000028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082f8+0x20000,   0x000000f2,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000082fc+0x20000,   0x00000080,      2,    0xc    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008304+0x20000,   0x00000020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008310+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008314+0x20000,   0x00000028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008318+0x20000,   0x00000003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000831c+0x20000,   0x000000c4,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008320+0x20000,   0x000000c0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008328+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008330+0x20000,   0x00000040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008334+0x20000,   0x000000df,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008338+0x20000,   0x000000f0,      3,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008344+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00008348+0x20000,   0x00000008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000834c+0x20000,   0x000000f0,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a008+0x20000,   0x40980000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a00c+0x20000,   0x05cf0000,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a010+0x20000,   0x40006400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a01c+0x20000,   0x58000800,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a024+0x20000,   0x20cc3e80,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a02c+0x20000,   0x002049c2,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a030+0x20000,   0x001804d4,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a21c+0x20000,   0x00002a80,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a220+0x20000,   0x00000555,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a224+0x20000,   0xffffffff,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a22c+0x20000,   0x00000004,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a308+0x20000,   0x0000ff00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a314+0x20000,   0x0c040324,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a318+0x20000,   0x00001325,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a31c+0x20000,   0x02000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a320+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a328+0x20000,   0x00000800,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a32c+0x20000,   0x0000004f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a330+0x20000,   0x08007080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a334+0x20000,   0x0080001f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a338+0x20000,   0x00000200,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a340+0x20000,   0x00310a14,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a358+0x20000,   0xffffffff,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a364+0x20000,   0x00000400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a36c+0x20000,   0xff800000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a388+0x20000,   0x80000014,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a398+0x20000,   0x00010001,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3f8+0x20000,   0x43004f10,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000a3fc+0x20000,   0x40100000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e000+0x20000,   0x00000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e004+0x20000,   0x00010014,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e008+0x20000,   0x000a0028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e00c+0x20000,   0x04010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e010+0x20000,   0xda000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e014+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e018+0x20000,   0x000009af,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e020+0x20000,   0x00840100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e024+0x20000,   0x00020015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e028+0x20000,   0x000a0029,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e02c+0x20000,   0x0401000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e030+0x20000,   0xb6000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e034+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e038+0x20000,   0x000009fb,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e040+0x20000,   0x000e0203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e044+0x20000,   0x00010018,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e048+0x20000,   0x000c0031,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e04c+0x20000,   0x06010008,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e050+0x20000,   0xf9000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e054+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e058+0x20000,   0x00000bcd,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e060+0x20000,   0x00500303,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e064+0x20000,   0x01010019,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e068+0x20000,   0x000d0032,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e06c+0x20000,   0x06010006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e070+0x20000,   0xda000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e074+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e078+0x20000,   0x00000c1b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e080+0x20000,   0x00a50403,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e084+0x20000,   0x0102001a,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e088+0x20000,   0x000d0034,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e08c+0x20000,   0x06010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e090+0x20000,   0xc9000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e094+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e098+0x20000,   0x00000c7b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0a0+0x20000,   0x00b00504,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0a4+0x20000,   0x0102001c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0a8+0x20000,   0x000e0037,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0ac+0x20000,   0x06010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0b0+0x20000,   0xb6000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0b4+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0b8+0x20000,   0x00000d51,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0c0+0x20000,   0x00b40604,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0c4+0x20000,   0x0102001c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0c8+0x20000,   0x000e0038,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0cc+0x20000,   0x06010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0d0+0x20000,   0xe3000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0d4+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0d8+0x20000,   0x00000d9d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0e0+0x20000,   0x00aa0703,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0e4+0x20000,   0x0102001b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0e8+0x20000,   0x000d0035,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0ec+0x20000,   0x0601000a,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0f0+0x20000,   0xb9000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0f4+0x20000,   0x00000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e0f8+0x20000,   0x00000cdb,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e100+0x20000,   0x00200000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e104+0x20000,   0x00020014,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e108+0x20000,   0x000a0028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e10c+0x20000,   0x0401000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e110+0x20000,   0xda000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e114+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e118+0x20000,   0x000009af,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e120+0x20000,   0x00210103,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e124+0x20000,   0x00020015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e128+0x20000,   0x000a0029,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e12c+0x20000,   0x0401000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e130+0x20000,   0xb6000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e134+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e138+0x20000,   0x000009fb,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e140+0x20000,   0x00270203,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e144+0x20000,   0x00020018,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e148+0x20000,   0x000c0031,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e14c+0x20000,   0x0601000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e150+0x20000,   0xf9000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e154+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e158+0x20000,   0x00000bcd,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e160+0x20000,   0x00140303,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e164+0x20000,   0x01010019,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e168+0x20000,   0x000d0032,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e16c+0x20000,   0x06010006,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e170+0x20000,   0xda000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e174+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e178+0x20000,   0x00000c1b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e180+0x20000,   0x003e0403,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e184+0x20000,   0x0103001a,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e188+0x20000,   0x000d0034,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e18c+0x20000,   0x0601000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e190+0x20000,   0xc9004201,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e194+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e198+0x20000,   0x00000c7b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1a0+0x20000,   0x002c0504,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1a4+0x20000,   0x0102001c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1a8+0x20000,   0x000e0037,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1ac+0x20000,   0x06010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1b0+0x20000,   0xb6000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1b4+0x20000,   0x02000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1b8+0x20000,   0x00000d51,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1c0+0x20000,   0x002d0604,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1c4+0x20000,   0x0102001c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1c8+0x20000,   0x000e0038,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1cc+0x20000,   0x06010009,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1d0+0x20000,   0xe3000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1d4+0x20000,   0x02000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1d8+0x20000,   0x00000d9d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1e0+0x20000,   0x00000703,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1e4+0x20000,   0x0103001b,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1e8+0x20000,   0x000d0035,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1ec+0x20000,   0x0601000c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1f0+0x20000,   0xb9008101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1f4+0x20000,   0x03000007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e1f8+0x20000,   0x00000cdb,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e200+0x20000,   0x01010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e204+0x20000,   0x00000040,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e208+0x20000,   0x2800fb0c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e20c+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e210+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e218+0x20000,   0x0009af00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e224+0x20000,   0x01010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e228+0x20000,   0x00020042,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e22c+0x20000,   0x2900fd0c,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e230+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e234+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e23c+0x20000,   0x0009fb00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e248+0x20000,   0x01010102,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e24c+0x20000,   0x000e004e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e250+0x20000,   0x3100fa0e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e254+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e258+0x20000,   0x00020002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e260+0x20000,   0x000bcd00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e26c+0x20000,   0x01010103,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e270+0x20000,   0x00500050,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e274+0x20000,   0x3200fb0e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e278+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e27c+0x20000,   0x00020002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e284+0x20000,   0x000c1b00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e290+0x20000,   0x01010100,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e294+0x20000,   0x00100010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e298+0x20000,   0x2800fb0d,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e29c+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2a0+0x20000,   0x00020003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2a8+0x20000,   0x0009af03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2b4+0x20000,   0x02010101,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2b8+0x20000,   0x00210021,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2bc+0x20000,   0x2900fd0e,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2c0+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2c4+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2cc+0x20000,   0x0009fb03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2d8+0x20000,   0x02010102,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2dc+0x20000,   0x00270027,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2e0+0x20000,   0x3100fa0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2e4+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2e8+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2f0+0x20000,   0x000bcd03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e2fc+0x20000,   0x01010103,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e300+0x20000,   0x00140014,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e304+0x20000,   0x3200fb0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e308+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e30c+0x20000,   0x00010003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e314+0x20000,   0x000c1b03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e320+0x20000,   0x01000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e324+0x20000,   0x00280028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e328+0x20000,   0x2800fa0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e32c+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e330+0x20000,   0x00010001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e338+0x20000,   0x0009af00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e344+0x20000,   0x04000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e348+0x20000,   0x00a500a5,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e34c+0x20000,   0x2900fc0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e350+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e354+0x20000,   0x00040003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e35c+0x20000,   0x0009fb00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e368+0x20000,   0x04000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e36c+0x20000,   0x00c300c3,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e370+0x20000,   0x3100f90f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e374+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e378+0x20000,   0x00040003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e380+0x20000,   0x000bcd00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e38c+0x20000,   0x01000003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e390+0x20000,   0x00320032,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e394+0x20000,   0x3200fa0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e398+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e39c+0x20000,   0x00010001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3a4+0x20000,   0x000c1b00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3b0+0x20000,   0x04000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3b4+0x20000,   0x00280028,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3b8+0x20000,   0x2800fa0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3bc+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3c0+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3c8+0x20000,   0x0009af03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3d4+0x20000,   0x04000001,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3d8+0x20000,   0x00290029,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3dc+0x20000,   0x2900fc0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3e0+0x20000,   0x00020500,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3e4+0x20000,   0x39040003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3e8+0x20000,   0x00033903,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3ec+0x20000,   0x0009fb03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3f8+0x20000,   0x04000002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e3fc+0x20000,   0x00310031,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e400+0x20000,   0x3100f90f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e404+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e408+0x20000,   0xa8030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e40c+0x20000,   0x0000a800,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e410+0x20000,   0x000bcd03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e41c+0x20000,   0x04000003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e420+0x20000,   0x00320032,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e424+0x20000,   0x3200fa0f,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e428+0x20000,   0x01010700,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e42c+0x20000,   0x00030003,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e434+0x20000,   0x000c1b03,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e5c0+0x20000,   0x1f1f1f00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e5c4+0x20000,   0x20212021,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e5c8+0x20000,   0x01011010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e5cc+0x20000,   0x20202020,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e5d8+0x20000,   0x04000400,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e604+0x20000,   0x00000080,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e608+0x20000,   0x008040fc,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e60c+0x20000,   0x03fee000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e610+0x20000,   0x000a14a0,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e614+0x20000,   0x0e0f0c00,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e618+0x20000,   0x03050000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e620+0x20000,   0x06131015,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e624+0x20000,   0x06050000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e628+0x20000,   0x14000000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e634+0x20000,   0x20202020,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e644+0x20000,   0x20041000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e648+0x20000,   0x09090000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e654+0x20000,   0xe4120000,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e658+0x20000,   0xc43c0010,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e65c+0x20000,   0x00000190,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e660+0x20000,   0x4b6400fa,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e664+0x20000,   0x7d4b01f4,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e668+0x20000,   0x101900fa,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e66c+0x20000,   0x09090909,      6,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e684+0x20000,   0x800b0007,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e688+0x20000,   0x80060004,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e68c+0x20000,   0x80000000,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e690+0x20000,   0x800b0002,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e6ac+0x20000,   0x12640400,      1,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e6dc+0x20000,   0x0000aa00,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000e6ec+0x20000,   0x000000aa,      1,    0x4    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitTxqQfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq qfc unit - not generated from Cider , but to support 'all' memory space. till cpss be aligned.
*/
static void smemHawkUnitTxqQfc
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
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000070)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000013C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000360)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000048C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000568)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000638)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000814)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000013FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x0000187C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001984)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000235C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003068)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x0000331C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003400)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003568)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600, 0x0000361C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000406C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004108)}

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
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data    = 0x00000808;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address = 0x0000080C;
}
/**
* @internal smemHawkUnitMif function
* @endinternal
*
* @brief   Allocate address type specific memories - for the MIF
*/
static void smemHawkUnitMif
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* 0x00000000 - 0x0x0000007C : up to 32 channels type   8 */
        /* 0x00000080 - 0x0x000000FC : up to 32 channels type  32 */
        /* 0x00000100 - 0x0x0000011C : up to  2 channels type 128 */
        {MIF_GLOBAL_OFFSET + 0x00000000 , 0xffffff83, NULL, 0, smemChtActiveHawkWriteMifChannelMappingReg,   SMEM_CHT_PORT_MTI_MIF_TYPE_8_E /*mif type*/},
        {MIF_GLOBAL_OFFSET + 0x00000080 , 0xffffff83, NULL, 0, smemChtActiveHawkWriteMifChannelMappingReg,  SMEM_CHT_PORT_MTI_MIF_TYPE_32_E /*mif type*/},
        {MIF_GLOBAL_OFFSET + 0x00000100 , 0xfffffff3, NULL, 0, smemChtActiveHawkWriteMifChannelMappingReg, SMEM_CHT_PORT_MTI_MIF_TYPE_128_E /*mif type*/},

        {MIF_TX_OFFSET     + 0x00000000 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveHawkWriteMifTxControlReg ,   SMEM_CHT_PORT_MTI_MIF_TYPE_8_E /*mif type*/},
        {MIF_TX_OFFSET     + 0x00000004 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveHawkWriteMifTxControlReg ,  SMEM_CHT_PORT_MTI_MIF_TYPE_32_E /*mif type*/},
        {MIF_TX_OFFSET     + 0x00000008 , SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveHawkWriteMifTxControlReg , SMEM_CHT_PORT_MTI_MIF_TYPE_128_E /*mif type*/},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* MIF GLOBAL - start */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_GLOBAL_OFFSET + 0x00000000, MIF_GLOBAL_OFFSET + 0x00000060)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_GLOBAL_OFFSET + 0x00000080, MIF_GLOBAL_OFFSET + 0x000000BC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_GLOBAL_OFFSET + 0x00000100, MIF_GLOBAL_OFFSET + 0x00000104)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_GLOBAL_OFFSET + 0x00000110, MIF_GLOBAL_OFFSET + 0x00000128)}}
             /* MIF GLOBAL - end */
             /* MIF RX - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000000, MIF_RX_OFFSET + 0x00000018)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000020, MIF_RX_OFFSET + 0x00000080)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x000000A0, MIF_RX_OFFSET + 0x000000BC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x000000E0, MIF_RX_OFFSET + 0x000000E4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000100, MIF_RX_OFFSET + 0x0000012C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000300, MIF_RX_OFFSET + 0x00000360)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000380, MIF_RX_OFFSET + 0x000003E0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000400, MIF_RX_OFFSET + 0x00000460)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000500, MIF_RX_OFFSET + 0x0000051C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000540, MIF_RX_OFFSET + 0x0000055C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000580, MIF_RX_OFFSET + 0x0000059C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000600, MIF_RX_OFFSET + 0x00000604)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000610, MIF_RX_OFFSET + 0x00000614)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x00000620, MIF_RX_OFFSET + 0x00000624)}}
             /* MIF RX - end */
             /* MIF TX - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000000, MIF_TX_OFFSET + 0x00000014)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000020, MIF_TX_OFFSET + 0x00000080)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000000A0, MIF_TX_OFFSET + 0x000000BC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000000E0, MIF_TX_OFFSET + 0x000000E4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000100, MIF_TX_OFFSET + 0x0000012C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000200, MIF_TX_OFFSET + 0x00000260)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000290, MIF_TX_OFFSET + 0x000002CC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000002D0, MIF_TX_OFFSET + 0x000002D4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000300, MIF_TX_OFFSET + 0x00000360)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000380, MIF_TX_OFFSET + 0x000003E0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000400, MIF_TX_OFFSET + 0x00000460)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000480, MIF_TX_OFFSET + 0x000004E0)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000500, MIF_TX_OFFSET + 0x0000051C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000540, MIF_TX_OFFSET + 0x0000055C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000580, MIF_TX_OFFSET + 0x0000059C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000005C0, MIF_TX_OFFSET + 0x000005DC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000600, MIF_TX_OFFSET + 0x00000604)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000610, MIF_TX_OFFSET + 0x00000614)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000620, MIF_TX_OFFSET + 0x00000624)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000630, MIF_TX_OFFSET + 0x00000634)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000700, MIF_TX_OFFSET + 0x00000700)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000710, MIF_TX_OFFSET + 0x00000770)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000790, MIF_TX_OFFSET + 0x000007CC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000007D0, MIF_TX_OFFSET + 0x000007D4)}}
             /* MIF TX - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,        MIF_GLOBAL_OFFSET + 0x00000128,         0xFFF00000,      1,    0x0            }

            ,{DUMMY_NAME_PTR_CNS,        MIF_RX_OFFSET     + 0x00000020,         0x00000020,      25,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_RX_OFFSET     + 0x000000A0,         0x00000020,      8,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_RX_OFFSET     + 0x000000E0,         0x00000020,      2,    0x4            }

            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000020,         0x00000020,      25,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000000A0,         0x00000020,      8,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000000E0,         0x00000020,      2,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000700,         0x00000005,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000710,         0x00000008,      25,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000790,         0x00000008,      8,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000007D0,         0x00000008,      2,    0x4            }

            ,{NULL,                      0,                                      0x00000000,      0,    0x0            }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUnitAnp function
* @endinternal
*
* @brief   Allocate address type specific memories - for the ANP
*/
static void smemHawkUnitAnp
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{

    if (devObjPtr->deviceFamily == SKERNEL_HARRIER_FAMILY)
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

            {ANP_OFFSET + 0x00000100 + 0x2a4*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*2, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*2, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*3, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*3, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*4, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*5, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*5, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*6, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*6, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

            {ANP_OFFSET + 0x00000100 + 0x2a4*7, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
            {ANP_OFFSET + 0x00000104 + 0x2a4*7, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }


    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* ANP - start */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ANP_OFFSET + 0x00000000, ANP_OFFSET + 0x000000B8)}}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ANP_OFFSET + 0x00000100, ANP_OFFSET + 0x000003a0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, ANP_STEP_PORT_OFFSET)}
             /* ANP - end */

            /* AN - start */
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000000, AN_OFFSET + 0x0000000c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000014, AN_OFFSET + 0x00000018)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000038, AN_OFFSET + 0x0000006c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x000000c0, AN_OFFSET + 0x000000c4)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000800, AN_OFFSET + 0x0000086c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000880, AN_OFFSET + 0x0000088c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT, AN_STEP_PORT_OFFSET)}
            /* AN - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000000,         0x00000910,      1,    0x0            }

            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000004,         0x00000020,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000008,         0x03020100,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x0000000c,         0x07060504,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000010,         0x0000009c,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000014,         0x00000c35,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000018,         0x0002625a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x0000001c,         0x002faf08,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000020,         0x0000ffff,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001BC,         0x0000000e,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001C4,         0x05004000,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001C8,         0x00000020,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001CC,         0x00040000,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001D0,         0x00022810,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001D4,         0x0F00C000,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001D8,         0x10000000,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001DC,         0x00000818,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001E4,         0xF3F10000,      8,    ANP_STEP_PORT_OFFSET }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001E8,         0x00000002,      8,    ANP_STEP_PORT_OFFSET }

            ,{NULL,                      0,                                   0x00000000,      0,    0x0            }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUsxMac function
* @endinternal
*
* @brief   Allocate address type specific memories - for the USX MAC
*/
static void smemHawkUsxMac
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* MIB - start */

        /* Statistic counters - active memory read */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.counterCapture[ii] */
        /* ii = 0..55 : address 0x20-0xfc

          lets brake it down:
                            addr        mask
          0x20 - 0x3c :     0x20      (~0x1F)
          0x40 - 0x7c :     0x40      (~0x3F)
          0x80 - 0xfc :     0x80      (~0x7F)
        */
        {USX_MAC_MIB_OFFSET + 0x00000020, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {USX_MAC_MIB_OFFSET + 0x00000040, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {USX_MAC_MIB_OFFSET + 0x00000080, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        /* Statistic counters - active memory write control */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.control */
        {USX_MAC_MIB_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},


        /* MIB - END */

        /*0x00000094 + 0x18*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*2, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*3, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*5, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*6, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*7, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},


        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*2, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*3, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*4, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*5, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*6, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {USX_MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*7, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* MIB - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_MAC_MIB_OFFSET + 0x00000000, USX_MAC_MIB_OFFSET + 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_MAC_MIB_OFFSET + 0x0000001C, USX_MAC_MIB_OFFSET + 0x00000A9C)}

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    /* MIB - END */

    /* USX MAC - start */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC : ports 0..7 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PORT0_100G_OFFSET + 0x00000000, USX_PORT0_100G_OFFSET + 0x00000014)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PORT0_100G_OFFSET + 0x0000001C, USX_PORT0_100G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PORT0_100G_OFFSET + 0x00000030, USX_PORT0_100G_OFFSET + 0x00000048)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PORT0_100G_OFFSET + 0x00000054, USX_PORT0_100G_OFFSET + 0x00000074)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PORT0_100G_OFFSET + 0x0000007C, USX_PORT0_100G_OFFSET + 0x000000A0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_UNIT , MAC_STEP_PORT_OFFSET)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* USX MAC - end */

    /* USX MAC EXT - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* per MAC */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_MAC_EXT_BASE_OFFSET, USX_MAC_EXT_BASE_OFFSET + 0x00000234)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
    /* USX MAC EXT - end */

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* MAC USX EXT - start */
             {DUMMY_NAME_PTR_CNS,            0x0000002c,         0x002FD10,  8,    0x28  }
            /* MAC USX EXT - end */

            /*USX MAC*/
            ,{DUMMY_NAME_PTR_CNS,            0x00002080,         0x100,      1,    0x0            ,(NUM_PORTS_PER_UNIT) , (MAC_STEP_PORT_OFFSET)}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkUsxPcs function
* @endinternal
*
* @brief   Allocate address type specific memories - for the USX PCS
*/
static void smemHawkUsxPcs
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
             /* PCS start */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000000, USX_PCS_PORTS_OFFSET + 0x00000024)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x100))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000038, USX_PCS_PORTS_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x100))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000080, USX_PCS_PORTS_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x100))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x000000C8, USX_PCS_PORTS_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x100))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x000000F0, USX_PCS_PORTS_OFFSET + 0x000000FC)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x100))}
             /* PCS end */

             /* USXM Units - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_USXM_OFFSET + 0x00000000, USX_PCS_USXM_OFFSET + 0x00000014)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_USXM_OFFSET + 0x00000040, USX_PCS_USXM_OFFSET + 0x0000005C)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
             /* USXM Units - end */

             /* PCS-RSFEC - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x00000000, USX_PCS_RS_FEC_OFFSET + 0x00000014)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x00000028, USX_PCS_RS_FEC_OFFSET + 0x0000004C)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x00000204, USX_PCS_RS_FEC_OFFSET + 0x00000204)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x0000020C, USX_PCS_RS_FEC_OFFSET + 0x0000020C)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x00000228, USX_PCS_RS_FEC_OFFSET + 0x0000022C)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_RS_FEC_OFFSET + 0x00000300, USX_PCS_RS_FEC_OFFSET + 0x00000314)}, FORMULA_SINGLE_PARAMETER((2 ) , (0x3000))}
             /* PCS-RSFEC - end */

             /* PCS-LPCS - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x00000000, USX_PCS_LPCS_OFFSET + 0x00000020)}, FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x80))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x00000040, USX_PCS_LPCS_OFFSET + 0x0000005C)}, FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_UNIT ) , (0x80))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x000003E0, USX_PCS_LPCS_OFFSET + 0x000003EC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x000003F0, USX_PCS_LPCS_OFFSET + 0x000003F4)}}
             /* PCS-LPCS - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}


/**
* @internal smemHawkUnitPcaCtsu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA CTSU unit
*/
static void smemHawkUnitPcaCtsu
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        HAWK_TSU_PER_GROUP_ACTIVE_MEM(0),
        HAWK_TSU_PER_GROUP_ACTIVE_MEM(1),
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        /* aligned to /Cider/EBU-IP/PCA/CTSU IP/CTSU 1.0/CTSU {1.0.9} */
        SMEM_CHUNK_BASIC_STC chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000001FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x000005FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000010FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x000014FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x000018FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x00001CFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x000020FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x000024FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x000030FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x000034FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003800, 0x0000380C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003820, 0x0000382C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x000041FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004800, 0x000049FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x000050FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005400, 0x000054FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005800, 0x000058FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005C00, 0x00005CFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x000060FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006400, 0x000064FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006800, 0x000068FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006C00, 0x00006CFC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPcaBrg function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Bridge
*/
static void smemHawkUnitPcaBrg
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PCA/<BRG> BRG*/
    /*IP: \Cider \EBU-IP \PCA \BRG IP \BRG 1.0 \BRG {1.0.19} \BRG \dp2sdb*/
    /*Mask: \Cider \EBU-IP \PCA \BRG IP \BRG 1.0 \BRG {Hawk1 32-CH} {1.0.19}*/

    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /* Global */
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
        /* Rx Channel */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000017C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000023C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000043C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000004C0, 0x000004FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x00000624)}
        /* Rx Global */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000754)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000780, 0x00000780)}
        /* Tx Channel */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000087C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000097C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000117C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x0000137C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x0000153C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001580, 0x000015BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x0000173C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001780, 0x000017BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001904)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001920, 0x00001924)}
        /* Tx Global BRB */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A00, 0x00001A68)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A80, 0x00001A80)}
        /* PGC */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000207C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x000022F8)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x0000247C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002600, 0x0000267C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002800, 0x00002B7C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002F00, 0x00002F7C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003B00, 0x00003B7C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003D00, 0x00003D7C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003F00, 0x00003F04)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003FFC, 0x00003FFC)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}

/**
* @internal smemHawkUnitPcaArbiter function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Arbiter
*/
static void smemHawkUnitPcaArbiter
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PCA/<PIZ_ARB> PizArb IP TLU*/
    /*IP: \Cider \EBU-IP \PCA \PizArb \PizArb 1.0 \PizArb {1.5.3} \PizArb IP TLU \pzarb*/
    /*Mask: \Cider \EBU-IP \PCA \PizArb \PizArb 1.0 \PizArb hawk1 {1.5.3}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000007C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000006FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B10)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B14, 0x00000B90)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Pizza arbiter slot configuration */
             {DUMMY_NAME_PTR_CNS,            0x00000300,         0x0000011F,      256,    0x4      }
            /* Pizza arbiter control */
             ,{DUMMY_NAME_PTR_CNS,            0x00000B0C,         0x0000005B,      1,    0x4      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitPcaSffDefaultLLs function
* @endinternal
*
* @brief   default LLs registers values for PCA Sff
*/
static GT_U32 smemHawkUnitPcaSffDefaultLLs
(
    IN struct SMEM_REGISTER_DEFAULT_VALUE_STRUCT *defRegsPtr,
    IN GT_U32 repCount
)
{
    GT_UNUSED_PARAM (defRegsPtr);
    return repCount;
}

/**
* @internal smemHawkUnitPcaSff function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Sff
*/
static void smemHawkUnitPcaSff
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PCA/<SFF> SFF IP TLU */
    /*IP: \Cider \EBU-IP \PCA \SFF IP \SFF 1.0 \SFF IP {rtlf-2.0} \SFF IP TLU */
    /*Mask: \Cider \EBU-IP \PCA \SFF IP \SFF 1.0 \SFF IP Hawk-32 {rtlf-2.0}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /*Channel - Channel config*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x000006FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000008FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000C1C)} /*136 entries */
        /*Chan Interrupt*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000023C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x000002BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000043C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x000004BC)}
        /*Chan Statistics*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x0000187C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A00, 0x00001A7C)}
        /*Chan Status*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000127C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x0000147C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001600, 0x0000167C)}
        /*Glob Config*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
        /*Glob Interrupts*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000014)}
        /*Glob Status*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000018, 0x0000001C)}
        /*SFF Interrupt Summary*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000034)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
        /*Selected CHID Statistics*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x000000AC)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*Global Configurations */
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00008013,      1,    0x4      }
            /* Channeel Control Enable */
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00080810,      64,    0x4      }
            /* Channeel Control Configuration */
            ,{DUMMY_NAME_PTR_CNS,            0x00000800,         0x0000C000,      64,    0x4      }
            /* LLs */
            ,{DUMMY_NAME_PTR_CNS,            0x00000A00,         0x00000000,      136,   0x4, 0,0, 0,0, &smemHawkUnitPcaSffDefaultLLs}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitPcaMacsecExt function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC Extension
*/
static void smemHawkUnitPcaMacsecExt
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*/Cider/Switching Dies/Hawk/Hawk {hawk1_RTLF_NO_GOP_191115}/Hawk/Core/PCA/<MACSEC_EXT> MACSEC EXT IP TLU*/
    /*IP: \Cider \EBU-IP \PCA \MACSEC EXT IP \MACSEC EXT 1.0 \MACSEC EXT IP {1.1.13} \MACSEC EXT IP TLU */
    /*Mask: \Cider \EBU-IP \PCA \MACSEC EXT IP \MACSEC EXT 1.0 \MACSEC EXT IP hawk {1.1.13}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /*SDB Debug Monitor Pre MacSec*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000001C)}
        /*SDB Debug Monitor Post MacSec*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000011C)}
        /*MacSec Error Handler*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000208)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000020C, 0x00000288)}
        /*MacSec General*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000344)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}


/**
* @internal smemHawkUnitPcaMacsecEip163Alloc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-163 device (MACSec Classifier)
*/
static void smemHawkUnitPcaMacsecEip163Alloc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_163h_e_c32_512_mrvl/units/EIP_163h_e_c32_512_mrvl*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* TCAM_KEY_MASK_page0: Input TCAM entries 0:511 registers */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00007FFC)}
            /* TCAM_ENABLE_CTRL0: TCAM match enable/status control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000, 0x000087FC)}
            /* TCAM_POLICY_page0: TCAM policy values 0 till 511 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008800, 0x00008FFC)}
            /* VPORT_POLICY_page0: vPort policy (Egress only) 0:255 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00009000, 0x000093FC)}
            /* Channel_Settings_page0: Per-channel classifier settings registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000, 0x0000A7FC)}
            /* Control_packet: Global control packet detector settings registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A800, 0x0000A89C)}
            /* Classifier_parser_control_regs:  Global Header Parser control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A900, 0x0000A960)}
            /* Classifier_debug: Classifier Debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AC00, 0x0000AD20)}
            /* TCAM_Counters_0_511: TCAM Hit counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C000, 0x0000CFFC)}
            /* TCAM_block_stat_sum0: TCAM Statistics control 0:15 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000D03C)}
            /* TCAM_cntr_debug_regs: Counters module debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D800, 0x0000D80C)}
            /* TCAM_count_control: Counters control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D810, 0x0000D810)}
            /* TCAM_cntr_incr_ena: Counters increment enables registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D814, 0x0000D814)}
            /* TCAM_count_pack_timing: Host access scheduling registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D81C, 0x0000D81C)}
            /* TCAM_summary_thresholds: Threshold for the frame counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D820, 0x0000D824)}
            /* TCAM_count_posted_ctrl: Posted access control/status registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D838, 0x0000D838)}
            /* TCAM_options_version: Configuration options & Version information registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D874, 0x0000D87C)}
            /* TCAM_posted_operation_counters: Posted transfer counter data lo & hi registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000DC00, 0x0000DC04)}
            /* CHAN_Counters_0_31: Channel Statistics counters 0:31 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E000, 0x0000E7FC)}
            /* CHAN_block_stat_sum0: Per channel statistics summary registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E800, 0x0000E800)}
            /* CHAN_cntr_debug_regs: Counters module debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC00, 0x0000EC0C)}
            /* CHAN_count_control: Counters control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC10, 0x0000EC10)}
            /* CHAN_cntr_incr_ena: Counters increment enables registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC14, 0x0000EC14)}
            /* CHAN_count_pack_timing: Host access scheduling registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC1C, 0x0000EC1C)}
            /* CHAN_summary_thresholds: Threshold for the frame counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC20, 0x0000EC24)}
            /* CHAN_count_posted_ctrl: Posted access control/status registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC38, 0x0000EC38)}
            /* CHAN_options_version: Configuration options & Version information registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC74, 0x0000EC7C)}
            /* CHAN_posted_operation_counters: Posted transfer counter data lo & hi for counters 0...7 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EE00, 0x0000EE3C)}
            /* MTTCAM_Counters_0_15: MTTCAM HIT LO & HI registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F000, 0x0000F07C)}
            /* MTTCAM_block_stat_sum0: MTTCAM_COUNT_SUM_P registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F100, 0x0000F100)}
            /* MTTCAM_cntr_debug_regs: MTTCAM_COUNT_DEBUG1 & MTTCAM_COUNT_DEBUG4 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F200, 0x0000F20C)}
            /* MTTCAM_count_control: MTTCAM_COUNT_CONTROL register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F210, 0x0000F210)}
            /* MTTCAM_cntr_incr_ena: MTTCAM_COUNT_INCEN1 register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F214, 0x0000F214)}
            /* MTTCAM_count_pack_timing: MTTCAM_COUNT_PACK_TIMING register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F21C, 0x0000F21C)}
            /* MTTCAM_summary_thresholds: MTTCAM_COUNT_FRAME_THR_1 & 2 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F220, 0x0000F224)}
            /* MTTCAM_count_posted_ctrl: MTTCAM_COUNT_POSTED_CTRL register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F238, 0x0000F238)}
            /* MTTCAM_options_version: MTTCAM_COUNT_OPTIONS & MTTCAM_COUNT_VERSION */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F274, 0x0000F27C)}
            /* MTTCAM_posted_operation_counters: MTTCAM_COUNT_POSTED_RES_LO & HI  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F300, 0x0000F304)}
            /* Channel_AIC_page0: Channel interrupt controllers 0:31 channels */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F400, 0x0000FBFC)}
            /* AIC_Regs: Global interrupt controller registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FC00, 0x0000FC3C)}
            /* SP_AIC_Regs: Spare interrupt controller registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FC40, 0x0000FC7C)}
            /* Input_adapter: Input adapter registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FC80, 0x0000FC94)}
            /* Channel_control: Channel control 0:31 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FE00, 0x0000FE7C)}
            /* Spare_config_reg:  SPARE_CFG register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FEFC, 0x0000FEFC)}
            /* ECC_status: ECC Status registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FF00, 0x0000FF7C)}
            /* System_control: System control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FFE4, 0x0000FFFC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPcaMacsecEip163Egr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Egress MACSEC EIP-163 device (MACSec Classifier)
*/
static void smemHawkUnitPcaMacsecEip163Egr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000D810, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_163h_e_c32_512_mrvl/units/EIP_163h_e_c32_512_mrvl*/
    smemHawkUnitPcaMacsecEip163Alloc(devObjPtr,unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*                             LSBits of 'Addr',     val,                     */
             {DUMMY_NAME_PTR_CNS,            0x0000d874,         0x01400200,      1,    0x0}   /* TCAM_COUNT_OPTIONS2 */
            ,{DUMMY_NAME_PTR_CNS,            0x0000d878,         0x40000001,      1,    0x0}   /* TCAM_COUNT_OPTIONS  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000d87c,         0x022226d9,      1,    0x0}   /* TCAM_COUNT_VERSION  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x20978010,      1,    0x0}   /* EIP163_CONFIG2      */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0xa0040000,      1,    0x0}   /* EIP163_CONFIG       */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02425ca3,      1,    0x0}   /* EIP163_VERSION      */
            ,{NULL,                          0,                  0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/*
* @internal smemHawkUnitPcaMacsecEip163Ingr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Ingress MACSEC EIP-163 device (MACSec Classifier)
*/
static void smemHawkUnitPcaMacsecEip163Ingr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000D810, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_163h_i_c32_512_mrvl/units/EIP_163h_i_c32_512_mrvl*/
    smemHawkUnitPcaMacsecEip163Alloc(devObjPtr,unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*                             LSBits of 'Addr',     val,                     */
             {DUMMY_NAME_PTR_CNS,            0x0000d874,         0x01400200,      1,    0x0}   /* TCAM_COUNT_OPTIONS2 */
            ,{DUMMY_NAME_PTR_CNS,            0x0000d878,         0x40000001,      1,    0x0}   /* TCAM_COUNT_OPTIONS  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000d87c,         0x022226d9,      1,    0x0}   /* TCAM_COUNT_VERSION  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x20978010,      1,    0x0}   /* EIP163_CONFIG2      */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x60040000,      1,    0x0}   /* EIP163_CONFIG       */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02425ca3,      1,    0x0}   /* EIP163_VERSION      */
            ,{NULL,                          0,                  0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitPcaMacsecEip164Alloc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-164 device (MACSec Transform)
*/
static void smemHawkUnitPcaMacsecEip164Alloc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{

    {
        /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_164h_e_c32_512_mrvl/units/EIP_164h_e_c32_512_mrvl*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* transform_records_page0: Transform records area (SA) 0:127 registers */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00003FFC)}
            /* RXSC_CAM_KEY_page0: RxSC CAM (ingress core only) 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x000043F8)}
            /* RXSC_CAM_ENABLE: RXSC_ENTRY_ENABLE1..8 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x000047FC)}
            /* SC_SA_map_page0: SC_SA_map_page0 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004800, 0x000049F8)}
            /* Flow_control_page0: Flow control (vPort/SecY/SC policy) 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004A00, 0x00004BFC)}
            /* SAM_NM_PARAMS_page0: Classifier control/status 0:31 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x0000507C)}
            /* Classifier_control_regs: In-flight counter & Crypt-authenticate control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005104, 0x0000510C)}
            /* SA_exp_summary_page0: Expired SA summary 0:15 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x0000523C)}
            /* Classifier_debug: Classifier debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005400, 0x0000549C)}
            /* EIP66_Channel_Registers_page0: EIP-66 Channels control/status 0:31 registers  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005800, 0x00005BF8)}
            /* EIP66_AIC_Regs: EIP-66 Interrupt Controller registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005C00, 0x00005C3C)}
            /* EIP66_Global_Registers: NextPN update value LSB & MSB registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005F80, 0x00005FFC)}
            /* SECY_Counters_0_63: SecY statistics counters 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00007FC4)}
            /* SA_Counters_0_127:  SA statistics counters 0:127 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000, 0x0000BFCC)}
            /* IFC0_Counters_0_63: Interface statistics group 0 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000C000, 0x0000CFE4)}
            /* IFC1_Counters_0_63: Interface statistics group 1 0:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000D000, 0x0000DFE4)}
            /* SA_cntr_debug_regs: Statistics debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E000, 0x0000E00C)}
            /* SA_count_control: Statistics counters control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E010, 0x0000E010)}
            /* SA_cntr_incr_ena: Increment enable control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E014, 0x0000E014)}
            /* SA_count_pack_timing: Statistics pack timing control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E01C, 0x0000E01C)}
            /* SA_summary_thresholds: Threshold for frame & octet counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E020, 0x0000E034)}
            /* SA_count_posted_ctrl: Posted operations control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E038, 0x0000E038)}
            /* SA_options_version: Statistics configuration options & version registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E074, 0x0000E07C)}
            /* SA_block_stat_sum0: Per SA statistics summary 0:15 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E100, 0x0000E13C)}
            /* SECY_cntr_debug_regs: Statistics debug registers  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E400, 0x0000E40C)}
            /* SECY_count_control: Statistics counters control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E410, 0x0000E410)}
            /* SECY_cntr_incr_ena: Increment enable control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E414, 0x0000E414)}
            /* SECY_count_pack_timing: Statistics pack timing control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E41C, 0x0000E41C)}
            /* SECY_summary_thresholds: Threshold for frame counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E420, 0x0000E424)}
            /* SECY_count_posted_ctrl: Posted operations control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E438, 0x0000E438)}
            /* SECY_options_version: Statistics configuration options & version registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E474, 0x0000E47C)}
            /* SECY_block_stat_sum0: Per SecY statistics summary 0:7 register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E480, 0x0000E49C)}
            /* IFC0_cntr_debug_regs: Statistics debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E600, 0x0000E60C)}
            /* IFC0_count_control: Statistics counters control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E610, 0x0000E610)}
            /* IFC0_cntr_incr_ena: Increment enable control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E614, 0x0000E614)}
            /* IFC0_count_pack_timing: Statistics pack timing control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E61C, 0x0000E61C)}
            /* IFC0_summary_thresholds: Threshold for frame & octet counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E620, 0x0000E634)}
            /* IFC0_count_posted_ctrl: Posted operations control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E638, 0x0000E638)}
            /* IFC0_options_version: Statistics configuration options & version registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E674, 0x0000E67C)}
            /* IFC0_block_stat_sum0: Per IFC0 statistics summary 0:7 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E680, 0x0000E69C)}
            /* IFC1_cntr_debug_regs: Statistics debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E800, 0x0000E80C)}
            /* IFC1_count_control: Statistics counters control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E810, 0x0000E810)}
            /* IFC1_cntr_incr_ena: Increment enable control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E814, 0x0000E814)}
            /* IFC1_count_pack_timing: Statistics pack timing control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E81C, 0x0000E81C)}
            /* IFC1_summary_thresholds: Threshold for frame & octet counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E820, 0x0000E834)}
            /* IFC1_count_posted_ctrl: Posted operations control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E838, 0x0000E838)}
            /* IFC1_options_version: Statistics configuration options & version registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E874, 0x0000E87C)}
            /* IFC1_block_stat_sum0: Per IFC1 statistics summary registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E880, 0x0000E89C)}
            /* RXCAM_cntr_debug_regs: Statistics debug registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA00, 0x0000EA0C)}
            /* RXCAM_count_control: Statistics counters control */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA10, 0x0000EA10)}
            /* RXCAM_cntr_incr_ena: Increment enable control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA14, 0x0000EA14)}
            /* RXCAM_count_pack_timing: Statistics pack timing control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA1C, 0x0000EA1C)}
            /* RXCAM_summary_thresholds: Threshold for frame counters registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA20, 0x0000EA24)}
            /* RXCAM_count_posted_ctrl: Posted operations control register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA38, 0x0000EA38)}
            /* RXCAM_options_version: Statistics configuration options & version registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA74, 0x0000EA7C)}
            /* RXCAM_block_stat_sum0: Per RXCAM statistics summary 0:7 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EA80, 0x0000EA9C)}
            /* RXCAM_Counters_0_63: RxCAMHit counters (0..63) registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000EC00, 0x0000EDFC)}
            /* SA_pn_thr_summary_page0: SA packet number threshold summary 0:15 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F000, 0x0000F03C)}
            /* Post_processor: Post-processor registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F100, 0x0000F150)}
            /* SA_posted_operation_counters: Result area for posted operations 0:9 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F200, 0x0000F24C)}
            /* SECY_posted_operation_counters: Result area for posted operations 0:8 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F280, 0x0000F2C4)}
            /* IFC0_posted_operation_counters: Result area for posted operations 0:3 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F300, 0x0000F31C)}
            /* IFC1_posted_operation_counters: Result area for posted operations 0:3 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F340, 0x0000F35C)}
            /* SP_AIC_Regs: Spare Interrupt Controller registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F380, 0x0000F3BC)}
            /* Channel_AIC_page0: Channel interrupt controllers 0:31 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000F400, 0x0000FBFC)}
            /* AIC_Regs: Global Interrupt Controller registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FC00, 0x0000FC3C)}
            /* Input_adapter: Input adapter registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FC40, 0x0000FC58)}
            /* Posted_access: Posted access for Control Structures registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FD00, 0x0000FDFC)}
            /* Channel_control: Channel controls 0:31 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FE00, 0x0000FE7C)}
            /* RXCAM_posted_operation_counters: Result area for posted operation registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FEF0, 0x0000FEF4)}
            /* Spare_config_reg: Spare configuration register */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FEFC, 0x0000FEFC)}
            /* ECC_status: ECC status registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FF00, 0x0000FF7C)}
            /* System_control: System control registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000FFD0, 0x0000FFFC)}
            /* transform_records_page1: 0:127 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x00013FFC)}
            /* RXSC_CAM_KEY_page1: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x000143F8)}
            /* SC_SA_map_page1: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014800, 0x000149FC)}
            /* Flow_control_page1: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014A00, 0x00014BFC)}
            /* SECY_Counters_64_127: 0:63  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00016000, 0x00017FC4)}
            /* SA_Counters_128_255: 0:127 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00018000, 0x0001BFCC)}
            /* IFC0_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001C000, 0x0001CFE4)}
            /* IFC1_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001D000, 0x0001DFE4)}
            /* RXCAM_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001EC00, 0x0001EDFC)}
            /* transform_records_page2: 0:127 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020000, 0x00023FFC)}
            /* RXSC_CAM_KEY_page2: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x000243F8)}
            /* SC_SA_map_page2: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024800, 0x000249FC)}
            /* Flow_control_page2: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024A00, 0x00024BFC)}
            /* SECY_Counters_64_127: 0:63  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00026000, 0x00027FC4)}
            /* SA_Counters_128_255: 0:127 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00028000, 0x0002BFCC)}
            /* IFC0_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002C000, 0x0002CFE4)}
            /* IFC1_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002D000, 0x0002DFE4)}
            /* RXCAM_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002EC00, 0x0002EDFC)}
            /* transform_records_page3: 0:127 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00030000, 0x00033FFC)}
            /* RXSC_CAM_KEY_page3: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034000, 0x000343F8)}
            /* SC_SA_map_page3: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034800, 0x000349FC)}
            /* Flow_control_page3: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034A00, 0x00034BFC)}
            /* SECY_Counters_64_127: 0:63  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00036000, 0x00037FC4)}
            /* SA_Counters_128_255: 0:127 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00038000, 0x0003BFCC)}
            /* IFC0_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003C000, 0x0003CFE4)}
            /* IFC1_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003D000, 0x0003DFE4)}
            /* RXCAM_Counters_64_127: 0:63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003EC00, 0x0003EDFC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemHawkUnitPcaMacsecEip164Egr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Egress MACSEC EIP-164 device (MACSec Transform)
*/
static void smemHawkUnitPcaMacsecEip164Egr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000E010, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E410, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E610, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E810, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000EA10, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_164h_e_c32_512_mrvl/units/EIP_164h_e_c32_512_mrvl*/
    smemHawkUnitPcaMacsecEip164Alloc(devObjPtr,unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*                             LSBits of 'Addr',     val,                      */
             {DUMMY_NAME_PTR_CNS,            0x00005ff8,         0xa0000008,      1,    0x0}   /* EIP66_CONTROL  */
            ,{DUMMY_NAME_PTR_CNS,            0x00005ffc,         0x0141bd42,      1,    0x0}   /* EIP66_VERSION  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x20970100,      1,    0x0}   /* EIP164_CONFIG2 */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x82000100,      1,    0x0}   /* EIP164_CONFIG  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02445ba4,      1,    0x0}   /* EIP164_VERSION */
            ,{NULL,                          0,                  0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemHawkUnitPcaMacsecEip164Ingr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Ingress MACSEC EIP-164 device (MACSec Transform)
*/
static void smemHawkUnitPcaMacsecEip164Ingr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000E010, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E410, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E610, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000E810, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x0000EA10, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecCountersControl, 0},
        {0x00005F90, SMEM_FULL_MASK_CNS  , NULL, 0, smemHawkActiveWriteMacSecNextPnCtrl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    /* aligned to /Cider/EBU-IP/PCA IP/PCA 1.0/PCA IP/<eip_163_164_32ch>/eip_163_164_32ch/<eip_163_164_32ch>/EIP_164h_e_c32_512_mrvl/units/EIP_164h_e_c32_512_mrvl*/
    smemHawkUnitPcaMacsecEip164Alloc(devObjPtr,unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*                             LSBits of 'Addr',     val,                      */
             {DUMMY_NAME_PTR_CNS,            0x00005ff8,         0xa0000008,      1,    0x0}   /* EIP66_CONTROL  */
            ,{DUMMY_NAME_PTR_CNS,            0x00005ffc,         0x0141bd42,      1,    0x0}   /* EIP66_VERSION  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x20970100,      1,    0x0}   /* EIP164_CONFIG2 */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x42000100,      1,    0x0}   /* EIP164_CONFIG  */
            ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02445ba4,      1,    0x0}   /* EIP164_VERSION */
            ,{NULL,                          0,                  0x00000000,      0,    0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemHawkUnitLed function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LED
*/
static void smemHawkUnitLed
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000A8)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000160)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000220)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}

/**
* @internal smemHawkUnitDfxServer function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the DFX unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitDfxServer
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
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8000, 0x000F8018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8020, 0x000F8038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8040, 0x000F804C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8060, 0x000F806C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8078, 0x000F80AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80B0, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80C0, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F80D0, 0x000F80DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F8D20, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8100, 0x000F8114)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8120, 0x000F813C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8200, 0x000F8230)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8240, 0x000F8248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8250, 0x000F82AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F82D0, 0x000F82D8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F82E0, 0x000F8314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8340, 0x000F836C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F83C0, 0x000F840C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8450, 0x000F849C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8C80, 0x000F8C94)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8D00, 0x000F8D1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F8F00, 64)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F9000, 1024)}

            /* XSB_XBAR_IP units*/
            FALCON_XSB_CLUSTER_MEMORY_MAC(XSB_ADRESS_SPACE)
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    if (devObjPtr->deviceFamily == SKERNEL_HARRIER_FAMILY)
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8D00, 0x000F8D2C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8DD0, 0x000F8DD8)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* DFX_RAMs */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000003C)}, FORMULA_TWO_PARAMETERS(128, 0x40, 28, 0x8000)}
            /* DFX_CLIENTS */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000201C)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002040, 0x0000204C)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002070, 0x00002074)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x00002080)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002088, 0x0000208c)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020b0, 0x000020b0)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020e0, 0x000020ec)}, FORMULA_SINGLE_PARAMETER(31 , 0x8000)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
             {DUMMY_NAME_PTR_CNS,            0x000f8004,         0x02040053,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8008,         0x2ff92830,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f800c,         0x0070201e,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8010,         0x0000000f,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8014,         0xfffffff3,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8018,         0x04000000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8020,         0x00000187,      4,    0x4,      3,    0x20 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8030,         0x00000387,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8034,         0x00000187,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f807c,         0x8011e214,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8080,         0x30a88019,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8084,         0x33981000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8088,         0x08c01a09,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8090,         0x00000187,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f809c,         0xff8000ff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f80a4,         0x00000208,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000F80D0,         0xF0F01032,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000F80D4,         0x00000781,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000F80D8,         0x0184E140,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000F80DC,         0x00030000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8120,         0x0d08007f,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8124,         0x0800806c,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8128,         0xa2008179,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f812c,         0xbde1ffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8130,         0x0056358c,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8200,         0x00000bff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8244,         0x00000357,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8248,         0x0000ffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8250,         0x76543210,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8254,         0x25022a00,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8258,         0x0000105c,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f825c,         0x82600000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8260,         0xf8000000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8264,         0x105003ff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8268,         0x05c40000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f826c,         0x0f000001,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8270,         0x020b9080,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8278,         0x1551041b,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f827c,         0x2aa20837,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8280,         0x0c090346,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8284,         0x1e14f098,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f828c,         0x0500046c,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8290,         0x00000009,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8294,         0xffffffc2,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8298,         0x19010000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f829c,         0x0000000e,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82d0,         0xffffffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f82d4,         0x00000bff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8314,         0x00428800,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8340,         0x00001bfc,     12,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000f8498,         0x00400000,      1,    0x0 }

         ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemHawkSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHawkSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{


    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_PCA_LMU_0)     ,smemHawkUnitLmu              }
        ,{STR(UNIT_RX_DMA)        ,smemHawkUnitRxDma            }
        ,{STR(UNIT_TX_FIFO)       ,smemHawkUnitTxFifo           }
        ,{STR(UNIT_TX_DMA)        ,smemHawkUnitTxDma            }
        ,{STR(UNIT_PB_CENTER_BLK)              ,smemHawkUnitPacketBuffer_pbCenter        }
        ,{STR(UNIT_PB_COUNTER_BLK)             ,smemHawkUnitPacketBuffer_pbCounter       }
        ,{STR(UNIT_PB_WRITE_ARBITER_0)         ,smemHawkUnitPacketBuffer_smbWriteArbiter }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,smemHawkUnitPacketBuffer_packetWrite     }
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_0)     ,smemHawkUnitPacketBuffer_cellRead        }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_0)   ,smemHawkUnitPacketBuffer_packetRead      }
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_0)     ,smemHawkUnitPacketBuffer_npmMc           }
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0)       ,smemHawkUnitPacketBuffer_sbmMc           }
        ,{STR(UNIT_TXQ_PDX)       ,smemHawkUnitTxqPdx           }
        ,{STR(UNIT_TXQ_PFCC)      ,smemHawkUnitTxqPfcc          }
        ,{STR(UNIT_TXQ_PSI)       ,smemHawkUnitTxqPsi           }
        ,{STR(UNIT_TXQ_SDQ0)      ,smemHawkUnitTxqSdq           }
        ,{STR(UNIT_TXQ_PDS0)      ,smemHawkUnitTxqPds           }
        ,{STR(UNIT_MAC_400G_0)    ,smemHawkUnitMac400GWrap      }
        ,{STR(UNIT_PCS_400G_0)    ,smemHawkUnitPcs400G          }
        ,{STR(UNIT_MAC_CPU)       ,smemHawkUnitMacCpuWrap       }
        ,{STR(UNIT_PCS_CPU)       ,smemHawkUnitPcsCpu           }
        ,{STR(UNIT_DUMMY_RAVEN_MG),smemHawkUnitDummyRavenMg     }
        ,{STR(UNIT_SERDES_0_0)    ,smemHawkUnitSerdes0          }
        ,{STR(UNIT_SERDES_1)      ,smemHawkUnitSerdes1          }
        ,{STR(UNIT_TXQ_QFC0)      ,smemHawkUnitTxqQfc           }
        ,{STR(UNIT_PCA_CTSU_0)    ,smemHawkUnitPcaCtsu          }
        ,{STR(UNIT_PCA_BRG_0)     ,smemHawkUnitPcaBrg           }
        ,{STR(UNIT_PCA_PZ_ARBITER_I_0)  ,smemHawkUnitPcaArbiter }
        ,{STR(UNIT_PCA_SFF_0)     ,smemHawkUnitPcaSff           }
        ,{STR(UNIT_PCA_MACSEC_EXT_I_163_0)     ,smemHawkUnitPcaMacsecExt}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_E_0)     ,smemHawkUnitPcaMacsecEip163Egr}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_I_0)     ,smemHawkUnitPcaMacsecEip163Ingr}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_E_0)     ,smemHawkUnitPcaMacsecEip164Egr}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_I_0)     ,smemHawkUnitPcaMacsecEip164Ingr}
        ,{STR(UNIT_LED_0)         ,smemHawkUnitLed}
        ,{STR(UNIT_MIF_0)         ,smemHawkUnitMif              }
        ,{STR(UNIT_ANP_0)         ,smemHawkUnitAnp              }
        ,{STR(UNIT_USX_0_MAC_0)   ,smemHawkUsxMac               }
        ,{STR(UNIT_USX_0_PCS_0)   ,smemHawkUsxPcs               }
        ,{STR(UNIT_DFX_SERVER)    ,smemHawkUnitDfxServer}

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units(devObjPtr);
}

/**
* @internal smemHawkUnitBar0 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM BAR0 unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemHawkUnitBar0
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
            /* get value from Hawk HW , from command in linux :
               hexdump /sys/bus/pci/devices/0000\:01\:00.0/config
            0000000 11ab 9400 0006 0010 0000 0200 0008 0000
            0000010 0004 c040 0000 0000 0004 c000 0000 0000
            0000020 0000 0000 0000 0000 0000 0000 11ab 11ab
            0000030 0000 0000 0040 0000 0000 0000 01ff 0000
            0000040 5001 0603 0008 0000 0000 0000 0000 0000
            0000050 7005 0080 0000 0000 0000 0000 0000 0000
            0000060 0000 0000 0000 0000 0000 0000 0000 0000
            0000070 0010 0002 8fc0 0000 2010 0000 3813 0047
            0000080 0040 1013 0000 0000 0000 0000 0000 0000
            0000090 0000 0000 001f 0001 0000 0000 000e 0000
            00000a0 0003 001e 0000 0000 0000 0000 0000 0000
            00000b0 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000100 0001 1482 0000 0000 0000 0040 2030 0046
            0000110 0000 0000 e000 0000 00a0 0000 0000 0000
            0000120 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000140 0000 0000 0000 0000 0019 1581 0000 0000
            0000150 0000 0000 7f00 0000 001e 1681 0a1b 0028
            0000160 0000 0000 0028 0000 000b 0001 0002 1004
            0000170 0000 0000 0000 0000 0100 0000 07d0 0000
            0000180 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000210 0000 0000 0000 0000 0000 0406 0000 0040
            0000220 0001 000f 7178 0317 0100 0190 0000 0000
            0000230 0000 0000 0000 0000 0000 0000 0000 0000
            0000240 0000 0000 0000 0000 0002 0000 0b40 1b1c
            0000250 0784 2230 0000 0000 0000 0000 0000 0000
            0000260 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000700 003b 00b1 ffff ffff 0004 0080 c000 1b60
            0000710 0120 0001 0000 0000 0000 0001 0140 0008
            0000720 0008 0000 0001 0000 a591 01d6 0010 0800
            0000730 e0c0 0004 e041 0004 ffff 000f 0000 0000
            0000740 000f 0000 0000 0000 8038 4521 8005 0521
            0000750 0000 0540 0000 0000 0000 0000 0000 0000
            0000760 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000800 0000 0000 0000 0000 0000 0000 0178 0000
            0000810 ffff ffff 0000 0000 0000 0000 007e 0000
            0000820 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000880 0000 0000 0000 0000 0000 0000 0001 0000
            0000890 2400 0000 0000 0000 0000 0000 0000 0000
            00008a0 0000 0000 0000 0000 0060 0402 0000 0000
            00008b0 0000 0000 0000 0000 0001 0000 0001 0000
            00008c0 0000 0000 0044 0000 0000 0000 0001 ff00
            00008d0 9c00 0000 0132 0000 0000 0000 0000 0000
            00008e0 0000 0000 0000 0000 0000 0000 0000 0000
            00008f0 0000 0000 0000 0000 302a 3439 2a2a 6761
            0000900 ffff ffff 0000 0000 0000 0000 0000 0000
            0000910 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0000b40 0004 0000 00d2 0000 0000 0000 0000 0000
            0000b50 0000 0000 0000 0000 0000 0000 0000 0000
            *
            0001000
            */
              {DUMMY_NAME_PTR_CNS,            0x00000000        ,    0x11ab    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000004        ,0x00100006    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000008        ,0x02000000    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000000c        ,0x00000008    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000010        ,0x00000004    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000018        ,0x00000004    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000002c        ,0x11ab11ab    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000034        ,0x00000040    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000003c        ,0x000001ff    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000040        ,0x06035001    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000044        ,0x00000008    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000050        ,0x00807005    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000070        ,0x00020010    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000074        ,0x00008fc0    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000078        ,0x00002010    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000007c        ,0x00473813    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000080        ,0x10130040    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000094        ,0x0001001f    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000009c        ,0x0000000e    }
             ,{DUMMY_NAME_PTR_CNS,            0x000000a0        ,0x001e0003    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000100        ,0x14820001    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000108        ,0x00400000    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000010c        ,0x00462030    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000114        ,0x0000e000    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000118        ,0x000000a0    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000148        ,0x15810019    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000154        ,0x00007f00    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000158        ,0x1681001e    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000015c        ,0x00280a1b    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000164        ,0x00000028    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000168        ,0x0001000b    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000016c        ,0x10040002    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000178        ,0x00000100    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000017c        ,0x000007d0    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000218        ,0x04060000    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000021c        ,0x00400000    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000220        ,0x000f0001    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000224        ,0x03177178    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000228        ,0x01900100    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000248        ,0x00000002    }
             ,{DUMMY_NAME_PTR_CNS,            0x0000024c        ,0x1b1c0b40    }
             ,{DUMMY_NAME_PTR_CNS,            0x00000250        ,0x22300784    }

             /* next are from Hawk Cider */
             ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x0c23040b,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0xffffffff,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x00800004,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x1b60c000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x00010120,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x0000c000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00080140,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x00000008,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000724,         0x00000001,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000740,         0x0000000f,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000748,         0x45218038,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000074c,         0x05218005,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000750,         0x05400000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000080c,         0x00020178,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000081c,         0x00000040,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000088c,         0x00000001,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000890,         0x00002400,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008a8,         0x04000060,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008b8,         0x00000001,      2,    0x4  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008c4,         0x00000044,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008cc,         0xff000001,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008d0,         0x00009c00,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008d4,         0x00000132,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008f8,         0x3439302a,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x000008fc,         0x67612a2a,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000b40,         0x00000004,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000b44,         0x000000d2,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00004010,         0xffffffff,      2,    0x8  }
             ,{DUMMY_NAME_PTR_CNS,            0x00004014,         0xfffffffe,      2,    0x8  }
             ,{DUMMY_NAME_PTR_CNS,            0x00004030,         0x0001ffff,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000407c,         0x00473c13,      1,    0x0  }
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
* @internal smemHawkPexAndBar0DeviceUnitAlloc function
* @endinternal
*
* @brief   allocate 'PEX config space' and 'BAR0' -- if not allocated already
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void smemHawkPexAndBar0DeviceUnitAlloc
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
        smemHawkUnitBar0(devObjPtr,currUnitChunkPtr);
    }

    /*allocate 'PEX config space' and 'BAR0' -- if not allocated already*/
    smemFalconPexAndBar0DeviceUnitAlloc(devObjPtr);
}

/**
* @internal smemHawkSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemHawkSpecificDeviceUnitAlloc
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
        SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &hawk_units[0];

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

    /*allocate 'PEX config space' and 'BAR0' -- if not allocated already*/
    smemHawkPexAndBar0DeviceUnitAlloc(devObjPtr);

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    smemHawkSpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemHawkSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

/**
* @internal smemHawkConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Hawk : Convert (dev,address) to new (dev,address).
*         needed for multi-MG device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void smemHawkConvertDevAndAddrToNewDevAndAddr
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

    pipeOffset = internalHawkUnitPipeOffsetGet(devObjPtr, address,
        &pipeIdOfAddress,&currentPipeId,GT_FALSE);

    newAddress = address + pipeOffset;

    *newAddressPtr = newAddress;

    return;
}

/**
* @internal smemHawkPrepareMultiMgRecognition function
* @endinternal
*
* @brief   prepare multi MG units recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void smemHawkPrepareMultiMgRecognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    /* NOTE: the function smemHawkConvertDevAndAddrToNewDevAndAddr will help
       any memory access to 'MG0' address to access proper MG unit */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        smemHawkConvertDevAndAddrToNewDevAndAddr;

}

/**
* @internal smemHawkActiveWriteExactMatchMsg function
* @endinternal
*
* @brief   Write to the Message from CPU Registers - activate
*          update Exact Match message.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* logic based on : smemLion3ActiveWriteFdbMsg
*/
static void smemHawkActiveWriteExactMatchMsg(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32            * regPtr;         /* register's entry pointer */
    GT_U32              line;           /* line number */
    GT_U32              rdWr;           /* Read or write operation */
    GT_U32              RdWrTrig;       /* When set to 1, an FDB read or write access is performed.
                                           This bit is cleared by the device when the access action is completed*/
    GT_U32              numOfBanks;     /* hold number of Exact Match banks*/

    DECLARE_FUNC_NAME(smemHawkActiveWriteExactMatchMsg);

    /* data to be written */
    *memPtr = *inMemPtr;

    RdWrTrig = SMEM_U32_GET_FIELD(*inMemPtr, 0, 1);
    if(RdWrTrig == 0)
    {
        return;
    }

    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);
    line = SMEM_U32_GET_FIELD(*inMemPtr, 2, 21);

    __LOG(("smemFalconActiveWriteExactMatchMsg : Exact Match index befor change [0x%08x]   \n", line ));
    /*change back the index format from GM pattern (4 msb bits for bank number )
    to  WM pattern,(number of bits depends on number of banks )*/
    numOfBanks = devObjPtr->emNumOfBanks ;
    line = (((line>>4) * numOfBanks /*shift 2,3,4 bits left */ ) | ( line & (numOfBanks-1)/*num of bits to take */)) ;

    __LOG(("smemFalconActiveWriteExactMatchMsg : Exact Match index after change [0x%08x]   \n", line ));

    /* Get EM entry */
    if (rdWr)/* write the data from the msg registers to the Exact Match table  */
    {
        /* copy from registers of indirect to the EM spcific index */
        regPtr = smemMemGet(devObjPtr, address + 0x4);
        smemGenericHiddenMemSet(devObjPtr, SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E, line , regPtr, 4);
    }
    else/* read the data from the Exact Match table into the msg registers */
    {
        /* copy from the EM spcific index to the registers of indirect */
        regPtr = SMEM_SIP6_HIDDEN_EXACT_MATCH_PTR(devObjPtr, line);
        /* Mac Table Access Data */
        smemMemSet(devObjPtr, address + 0x4, regPtr, 4);
    }

    /* clear <RdWrTrig> */
    SMEM_U32_SET_FIELD(*memPtr, 0, 1, 0);

    return;

}
/**
* @internal smemHawkActiveWriteExactMatchGlobalCfg1Reg function
* @endinternal
*
* @brief   The function updates Exact Match table size according
*          to EM_Global_Configuration 1
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemHawkActiveWriteExactMatchGlobalCfg1Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
        GT_U32 emSize;
        GT_U32 emNumEntries;
        GT_U32 banksNumber;
        GT_U32 emNumOfBanks;

    /* data to be written */
    *memPtr = *inMemPtr;

    /* emSize */
    emSize = SMEM_U32_GET_FIELD(*inMemPtr, 0, 3);

    switch(emSize)
    {
        case 0:
            emNumEntries = SMEM_MAC_TABLE_SIZE_4KB;
            break;
        case 1:
            emNumEntries = SMEM_MAC_TABLE_SIZE_8KB;
            break;
        case 2:
            emNumEntries = SMEM_MAC_TABLE_SIZE_16KB;
            break;
        case 3:
            emNumEntries = SMEM_MAC_TABLE_SIZE_32KB;
            break;
        case 4:
            emNumEntries = SMEM_MAC_TABLE_SIZE_64KB;
            break;
        case 5:
            emNumEntries = SMEM_MAC_TABLE_SIZE_128KB;
            break;
        case 6:
            emNumEntries = SMEM_MAC_TABLE_SIZE_256KB;
            break;
        case 7:
            emNumEntries = SMEM_MAC_TABLE_SIZE_512KB;
            break;
        default:/*on 3 bits value .. can not get here*/
            emNumEntries = devObjPtr->emNumEntries;/*unchanged*/
            break;
    }

    devObjPtr->emNumEntries = emNumEntries;

    if(devObjPtr->emMaxNumEntries < devObjPtr->emNumEntries)
    {
        skernelFatalError("smemFalconActiveWriteExactMatchGlobalCfg1Reg: EM num entries in HW [0x%4.4x] , but set to use[0x%4.4x] \n",
            devObjPtr->emMaxNumEntries , devObjPtr->emNumEntries);
    }

    /* banksNumber */
    banksNumber = SMEM_U32_GET_FIELD(*inMemPtr, 3, 2);

    switch(banksNumber)
    {
        case 0:
            emNumOfBanks = 4;
            break;
        case 1:
            emNumOfBanks = 8;
            break;
        case 2:
            emNumOfBanks = 16;
            break;
        default:/*on 3 bits value .. can not get here*/
            emNumOfBanks =  devObjPtr->emNumOfBanks;/*unchanged*/
            break;
    }

    devObjPtr->emNumOfBanks = emNumOfBanks;

        return;
}

/**
* @internal smemHawkActiveWriteEMFlowIdAllocationConfiguration1 function
* @endinternal
*
* @brief  automatically clear "Recycle Flow ID En" field
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter - global interrupt bit number.
*
* @param[out] outMemPtr               - Pointer to the memory to copy register's content.
*
*
*/
static void smemHawkActiveWriteEMFlowIdAllocationConfiguration1
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
     /* data to be written */
    *memPtr = *outMemPtr;

    /* bit 0 is set by the CPU and causes the <First Flow ID> to be marked as free */
    if(*memPtr&1)
    {
        sip6_10_ExactMatchRecycleFlowId(devObjPtr);
        /* bit 0 should be automatically cleared. */
        SMEM_U32_SET_FIELD(*memPtr, 0, 1 , 0);
    }
}

typedef enum{
     SNET_CHT_mif_Tx_channel_id_number_E  /* sip6.10 mif tx channel id number . used to search  global mac number that hold the to local txDma number */
    ,SNET_CHT_mif_is_Tx_channel_enable_E  /* sip6.10 mif is tx channel enabled. used to check for tx if the MIF enabled. if not packet dropped.  */
}SNET_CHT_MIF_FIELDS_ENT;

static void snetHawkMifConfigTxFieldGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SNET_CHT_MIF_FIELDS_ENT          fieldType,
    IN GT_U32                           mifUnitId,
    IN GT_U32                           local_mif_channel_number,
    IN GT_U32                           mifType,
    OUT GT_U32                         *dpIndexPtr,
    OUT GT_U32                         *local_mac_numberPtr,
    OUT GT_U32                         *isPreemptiveChannelPtr,
    OUT GT_U32                         *valuePtr
)
{
    GT_U32  dpIndex,globalMacPort,local_mac_number,isPreemptiveChannel,txEnabled,local_dma_number;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32 pipeId;
    GT_U32 regAddr;
    GT_BIT isCpuPort;
    GT_U32 cpuPortIndex;
    GT_U32 sip6_MTI_EXTERNAL_representativePort;

    if(fieldType != SNET_CHT_mif_Tx_channel_id_number_E &&
       fieldType != SNET_CHT_mif_is_Tx_channel_enable_E)
    {
        skernelFatalError("snetHawkMifConfigTxFieldGet : fieldType[%d] not supported \n",
            fieldType);
    }

    /* convert mif unit id and local mif channel to dpIndex and local dma port num */
    portInfo.simplePortInfo.indexInUnit = local_mif_channel_number; /* !!! used as INPUT !!! */
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E,
        mifUnitId,/* !!! not using 'mac number' but rather mifUnitId !!! */
        &portInfo);

    dpIndex          = portInfo.simplePortInfo.unitIndex;
    local_mac_number = portInfo.simplePortInfo.indexInUnit;
    isPreemptiveChannel = portInfo.sip6_MTI_bmpPorts[0] & 0x1;/* bit 0 information ! */

    pipeId = smemGetCurrentPipeId(devObjPtr);
    /* convert dpIndex and local mac port to global mac number */
    smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
        devObjPtr,
        pipeId, /* current pipe */
        dpIndex, /* DP unit local to the current pipe */
        local_mac_number,
        GT_FALSE,/* CPU port not muxed ... not relevant */
        &globalMacPort);

    isCpuPort = devObjPtr->portsArr[globalMacPort].state == SKERNEL_PORT_STATE_MTI_CPU_E;
    if(isCpuPort)
    {
        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,globalMacPort,&portInfo);
        cpuPortIndex = portInfo.simplePortInfo.unitIndex;
    }
    else
    {
        cpuPortIndex = 0;/* dont care */
    }

    /* get the local channel index in this unit */
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E  ,globalMacPort,&portInfo);
    /* MIF hold dedicated logic for representative port (apply to 'CPU port' too!) */
    sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;

    *dpIndexPtr = dpIndex;
    *local_mac_numberPtr = local_mac_number;
    *isPreemptiveChannelPtr = isPreemptiveChannel;

    if(fieldType == SNET_CHT_mif_Tx_channel_id_number_E)
    {
        if(isPreemptiveChannel)
        {
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).preemption.mif_channel_mapping_register[mifType];
            }
            else
            {
                regAddr = MTI_PORT_MIF(devObjPtr,globalMacPort).preemption.mif_channel_mapping_register[mifType];
            }
        }
        else
        {
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).mif_channel_mapping_register[mifType];
            }
            else
            {
                regAddr = MTI_PORT_MIF(devObjPtr,globalMacPort).mif_channel_mapping_register[mifType];
            }
        }

        smemRegFldGet(devObjPtr, regAddr ,0 ,6, &local_dma_number);

        *valuePtr = local_dma_number;
    }
    else /* SNET_CHT_mif_is_Tx_channel_enable_E */
    {
        regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_tx_control_register[mifType];
        /* NOTE : for preemption channel the 'local_mif_channel_number' is the 'preemption index' */
        smemRegFldGet(devObjPtr, regAddr ,local_mif_channel_number ,1, &txEnabled);

        *valuePtr = txEnabled;
    }

    return;
}





/**
* @internal smemChtActiveHawkWriteMifChannelMappingReg function
* @endinternal
*
* @brief   The function save the tx mapping to mac for better runtime performance
*          when mapping used by the traffic.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveHawkWriteMifChannelMappingReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR   param,
    IN GT_U32 * inMemPtr
)
{
    SMEM_CHT_PORT_MTI_MIF_TYPE_ENT mifType = param;
    GT_U32   dpIndex;
    GT_U32   ii,mifUnitId,baseAddr,mifUnitIdMax=SIM_MAX_MIF_UNITS;
    GT_U32   local_mif_channel_number;/* local mif channel number */
    GT_U32   local_mac_number;   /*local mac port */
    GT_U32   txDmaGlobalPortNum; /* global dma port   */
    GT_U32   old_local_dma_number; /* old DMA number before the change */
    GT_U32   new_local_dma_number; /* new DMA number after  the change */
    GT_U32   globalMacPort;      /* global MAC number */
    GT_U32   pipeId;
    SKERNEL_PORT_MIF_INFO_STC   *portMifInfoPtr;
    static  GT_U32  mifTypeAddrOffset[SMEM_CHT_PORT_MTI_MIF_TYPE___last___E] = {0x00000000,0x00000080,0x00000100};
    static  GT_U32  mifTypeFactor[SMEM_CHT_PORT_MTI_MIF_TYPE___last___E] = {1,1,4};
    GT_U32  emptyIndex = SMAIN_NOT_VALID_CNS;
    GT_BIT  isPreemptiveChannel,txEnabled;

    /* old value */
    old_local_dma_number = (*memPtr)   & 0x3f;/* 6 bits */
    /* new value */
    new_local_dma_number = (*inMemPtr) & 0x3f;/* 6 bits */

    /* data to be written */
    *memPtr = *inMemPtr;

    if(old_local_dma_number == new_local_dma_number)
    {
        /* not changed */
        return;
    }
    for(mifUnitId = 0 ; mifUnitId < mifUnitIdMax ; mifUnitId++)
    {
        baseAddr = devObjPtr->memUnitBaseAddrInfo.mif[mifUnitId];
        if(address >= baseAddr && address < baseAddr + (16*_1K))
        {
            /* found unit */
            break;
        }
    }
    if (mifUnitId == mifUnitIdMax)
    {
        /* not found ?! */
        skernelFatalError("smemChtActiveHawkWriteMifChannelMappingReg : mifUnitId was not enough for address [0x%8.8x] \n",
            address);

        return ;
    }

    local_mif_channel_number = ((address & 0x3FFF)/*16K*/ - mifTypeAddrOffset[mifType]) / 4;
    if(mifTypeFactor[mifType] > 1)
    {
        local_mif_channel_number *= mifTypeFactor[mifType];
    }

    snetHawkMifConfigTxFieldGet(devObjPtr,SNET_CHT_mif_is_Tx_channel_enable_E,
        mifUnitId,
        local_mif_channel_number,
        mifType,
        &dpIndex,
        &local_mac_number,
        &isPreemptiveChannel,
        &txEnabled);

    if(new_local_dma_number >= devObjPtr->multiDataPath.info[dpIndex].dataPathNumOfPorts)
    {
        /*FE-3290083 - Incorrect default values for assigned MIF channel ID*/
        return;
    }

    pipeId = smemGetCurrentPipeId(devObjPtr);
    /* convert dpIndex and local mac port to global mac number */
    smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
        devObjPtr,
        pipeId, /* current pipe */
        dpIndex, /* DP unit local to the current pipe */
        local_mac_number,
        GT_FALSE,/* CPU port not muxed ... not relevant */
        &globalMacPort);


    /* convert dpIndex and local dma channel to global dma channel */
    smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
        devObjPtr,
        pipeId, /* current pipe */
        dpIndex, /* DP unit local to the current pipe */
        new_local_dma_number,
        GT_FALSE,/* CPU port not muxed ... not relevant */
        &txDmaGlobalPortNum);

    portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];

    for(ii = 0 ; ii < SKERNEL_PORT_MIF_INFO_MAX_CNS; ii++,portMifInfoPtr++)
    {
        if(!portMifInfoPtr->txEnabled)
        {
            /* the entry not used */
            if(emptyIndex == SMAIN_NOT_VALID_CNS)
            {
                /* empty - can be used */
                emptyIndex = ii;
            }

            continue;
        }

        if(portMifInfoPtr->txMacNum == globalMacPort)
        {
            /* used already by current mac */
            return;
        }
    }

    if(ii == SKERNEL_PORT_MIF_INFO_MAX_CNS &&
       emptyIndex == SMAIN_NOT_VALID_CNS)
    {
        skernelFatalError("smemChtActiveHawkWriteMifChannelMappingReg : SKERNEL_PORT_MIF_INFO_MAX_CNS [%d] is not enough ?! \n",
            SKERNEL_PORT_MIF_INFO_MAX_CNS);
        return;
    }

    /* the MAC not exists in the array. need to use the empty index */
    portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[emptyIndex];
    portMifInfoPtr->txEnabled = txEnabled;

    if(portMifInfoPtr->txEnabled)
    {
        portMifInfoPtr->txMacNum  = globalMacPort;
        portMifInfoPtr->mifType   = mifType;
        portMifInfoPtr->egress_isPreemptiveChannel = isPreemptiveChannel;
    }

    return;
}

/**
* @internal smemChtActiveHawkWriteMifTxControlReg function
* @endinternal
*
* @brief   The function save the tx enable of a channel for better runtime performance
*          when mapping used by the traffic.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveHawkWriteMifTxControlReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR   param,
    IN GT_U32 * inMemPtr
)
{
    SMEM_CHT_PORT_MTI_MIF_TYPE_ENT mifType = param;
    GT_U32   dpIndex;
    GT_U32   ii,kk,mifUnitId,baseAddr,mifUnitIdMax=SIM_MAX_MIF_UNITS;
    GT_U32   local_mif_channel_number;/* local mif channel number */
    GT_U32   local_mac_number;   /*local mac port */
    GT_U32   globalMacPort;      /* global MAC number */
    GT_U32   local_dma_number;   /*local dma port */
    GT_U32   txDmaGlobalPortNum; /* global dma port   */
    GT_U32   pipeId;
    SKERNEL_PORT_MIF_INFO_STC   *portMifInfoPtr;
    static  GT_U32  mifTypeFactor[SMEM_CHT_PORT_MTI_MIF_TYPE___last___E] = {1,1,4};
    GT_U32  oldValue = *memPtr;
    GT_U32  newValue = *inMemPtr;
    GT_U32  removedChannels = oldValue & ~newValue;
    GT_U32  addedChannels   = newValue & ~oldValue;
    GT_BIT  isPreemptiveChannel;

    /* data to be written */
    *memPtr = *inMemPtr;

    if(removedChannels == 0 && addedChannels == 0)
    {
        /* not modified */
        return;
    }

    for(mifUnitId = 0 ; mifUnitId < mifUnitIdMax ; mifUnitId++)
    {
        baseAddr = devObjPtr->memUnitBaseAddrInfo.mif[mifUnitId];
        if(address >= baseAddr && address < baseAddr + (16*_1K))
        {
            /* found unit */
            break;
        }
    }
    if (mifUnitId == mifUnitIdMax)
    {
        /* not found ?! */
        skernelFatalError("smemChtActiveHawkWriteMifTxControlReg : mifUnitId was not enough for address [0x%8.8x] \n",
            address);

        return ;
    }

    pipeId = smemGetCurrentPipeId(devObjPtr);

    for(kk = 0 ; kk < 32; kk++)
    {
        if(!((1<<kk) & removedChannels) &&
           !((1<<kk) & addedChannels))
        {
            /* not modified */
            continue;
        }

        local_mif_channel_number = kk;
        if(mifTypeFactor[mifType] > 1)
        {
            local_mif_channel_number *= mifTypeFactor[mifType];
        }

        snetHawkMifConfigTxFieldGet(devObjPtr,SNET_CHT_mif_Tx_channel_id_number_E,
            mifUnitId,
            local_mif_channel_number,
            mifType,
            &dpIndex,
            &local_mac_number,
            &isPreemptiveChannel,
            &local_dma_number);

        if(local_dma_number >= devObjPtr->multiDataPath.info[dpIndex].dataPathNumOfPorts)
        {
            /*FE-3290083 - Incorrect default values for assigned MIF channel ID*/
            return;
        }
        /* convert dpIndex and local mac port to global mac number */
        smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
            devObjPtr,
            pipeId, /* current pipe */
            dpIndex, /* DP unit local to the current pipe */
            local_mac_number,
            GT_FALSE,/* CPU port not muxed ... not relevant */
            &globalMacPort);

        /* convert dpIndex and local dma channel to global dma channel */
        smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
            devObjPtr,
            pipeId, /* current pipe */
            dpIndex, /* DP unit local to the current pipe */
            local_dma_number,
            GT_FALSE,/* CPU port not muxed ... not relevant */
            &txDmaGlobalPortNum);


        portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];

        for(ii = 0 ; ii < SKERNEL_PORT_MIF_INFO_MAX_CNS; ii++,portMifInfoPtr++)
        {
            if(portMifInfoPtr->txMacNum != globalMacPort)
            {
                /* not current mac */
                continue;
            }

            portMifInfoPtr->txEnabled = (newValue >> kk) & 0x1;
            portMifInfoPtr->egress_isPreemptiveChannel = isPreemptiveChannel;
            break;
        }

        if(ii == SKERNEL_PORT_MIF_INFO_MAX_CNS &&
           ((newValue >> kk) & 0x1))
        {
            /* not found as existing mac , and need to be 'enabled' */
            /* so we need to find empty place for it and add it     */

            portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];
            for(ii = 0 ; ii < SKERNEL_PORT_MIF_INFO_MAX_CNS; ii++,portMifInfoPtr++)
            {
                if(portMifInfoPtr->txEnabled)
                {
                    continue;
                }

                portMifInfoPtr->txEnabled = 1;
                portMifInfoPtr->txMacNum  = globalMacPort;
                portMifInfoPtr->mifType   = mifType;
                portMifInfoPtr->egress_isPreemptiveChannel = isPreemptiveChannel;
                break;
            }

            if(ii == SKERNEL_PORT_MIF_INFO_MAX_CNS)
            {
                if(sinit_global_usePexLogic)
                {
                    /* do not fatal error */
                    printf("smemChtActiveHawkWriteMifTxControlReg : ERROR <Type 32 Tx channel id number> was not configured for "
                    "globalMacPort[%d] prior to enabling the channel \n",
                        globalMacPort);
                }
                else
                {
                    skernelFatalError("smemChtActiveHawkWriteMifTxControlReg : SKERNEL_PORT_MIF_INFO_MAX_CNS [%d] is not enough ?! \n",
                        SKERNEL_PORT_MIF_INFO_MAX_CNS);
                }
                return;
            }
        }
    }

    return;
}


/**
* @internal smemHawkActiveWriteMacSecCountersControl function
* @endinternal
*
* @brief   MACSec counters control
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemHawkActiveWriteMacSecCountersControl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* Get current register content from simulation's memory */
    *memPtr = *inMemPtr;

    /* Check if bit#0 was set to '1', in this case the expected behaviour is to automatically falls back to 0b when reset is done */
    if ( (*inMemPtr) & 1 )
    {
        /* Clear bit#0 to '0' */
        SMEM_U32_SET_FIELD(*memPtr, 0, 1, 0);

        /* Write new value into register address */
        smemRegSet(devObjPtr, address , *memPtr);
    }
}

/**
* @internal smemHawkActiveWriteMacSecNextPnCtrl function
* @endinternal
*
* @brief   MACSec EIP66 next packet number control
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemHawkActiveWriteMacSecNextPnCtrl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* Get current register content from simulation's memory */
    *memPtr = *inMemPtr;

    /* Check if bit#0 (enable_update) was set to '1', in this case the expected behaviour
       is to automatically flips to zero when the action is completed */
    if ( (*inMemPtr) & 1 )
    {
        /* Clear bit#0 to '0' */
        SMEM_U32_SET_FIELD(*memPtr, 0, 1, 0);

        /* Write new value into register address */
        smemRegSet(devObjPtr, address , *memPtr);
    }
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
* @internal smemHawkInitRegDbDpUnits function
* @endinternal
*
* @brief   Init RegDb for DP units
*/
static void smemHawkInitRegDbDpUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    /* set register addresses for SDQ[0..3] */
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ1 , SIP6_TXQ_SDQ ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ2 , SIP6_TXQ_SDQ ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ3 , SIP6_TXQ_SDQ ,3);/*DP[3]*/

    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC1 , SIP6_TXQ_QFC ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC2 , SIP6_TXQ_QFC ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC3 , SIP6_TXQ_QFC ,3);/*DP[3]*/

    /* set register addresses for sip6_rxDMA[0..3])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA   , sip6_rxDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_1 , sip6_rxDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_2 , sip6_rxDMA ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_3 , sip6_rxDMA ,3);/*DP[3]*/

    /* set register addresses for sip6_txDMA[0..3])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA   , sip6_txDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_1 , sip6_txDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_2 , sip6_txDMA ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_3 , sip6_txDMA ,3);/*DP[3]*/

    /* set register addresses for sip6_txFIFO[0..3])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_1 , sip6_txFIFO ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_2 , sip6_txFIFO ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_3 , sip6_txFIFO ,3);/*DP[3]*/

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
    smemHawkInterruptTreeInit(devObjPtr);
}

/**
* @internal internal_smemHawkInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
static void internal_smemHawkInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isHawk = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP6_GET(devObjPtr)    = 1;
    SMEM_CHT_IS_SIP6_10_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isHawk = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = hawk_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = hawk_duplicatedUnits;
    }

    if (isHawk == GT_TRUE)
    {
        devObjPtr->devMemGopRegDbInitFuncPtr = smemHawkGopRegDbInit;
        devObjPtr->devIsOwnerMemFunPtr = NULL;
        devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;
        devObjPtr->devMemPortInfoGetPtr =  smemHawkPortInfoGet;

        devObjPtr->devMemGetMgUnitIndexFromAddressPtr = smemHawkGetMgUnitIndexFromAddress;
        devObjPtr->devMemGopPortByAddrGetPtr = smemHawkGopPortByAddrGet;
        devObjPtr->devMemMibPortByAddrGetPtr = smemHawkMibPortByAddrGet;

        devObjPtr->numOfPipesPerTile = 0;/* no pipes */
        devObjPtr->numOfTiles        = 0;/* no tiles */
        devObjPtr->tileOffset        = 0;
        devObjPtr->mirroredTilesBmp  = 0;

        devObjPtr->numOfMgUnits = HAWK_NUM_MG_UNITS;/* 4 MG units */

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = 4;
        /* !!! there is NO TXQ-dq in Hawk (like in Flacon) !!! */
        devObjPtr->multiDataPath.numTxqDq           = 0;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 0;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = FIRST_CPU_SDMA;

        /* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = 0;
        devObjPtr->numOfPortsPerPipe = SMAIN_NOT_VALID_CNS;/* no 'pipes' */

        devObjPtr->txqNumPorts = 1024;/* the limit on the 10 bits in SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM */

        devObjPtr->multiDataPath.maxIa = 1;/* single IA */

        {/* support the multi DP units */
            GT_U32  index;

            for(index = 0 ; index < devObjPtr->multiDataPath.maxDp ; index++)/* ALL 4 DPs duplicated although not evenly used! */
            {
                devObjPtr->multiDataPath.info[index].dataPathFirstPort  = NUM_PORTS_PER_DP_UNIT*index;/* without the 'CPU port' */
                devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = NUM_PORTS_PER_DP_UNIT;      /* without the 'CPU port' */
                devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = NUM_PORTS_PER_DP_UNIT;      /* the 'CPU port' number  in the DP */
            }
        }

        /* MIF support */
        {
            GT_U32  numOfMifs = 0;

            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_0_BASE_ADDR;/*400G_0*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_1_BASE_ADDR;/*400G_1*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_2_BASE_ADDR;/*400G_2*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_3_BASE_ADDR;/*400G_3*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_4_BASE_ADDR;/*USX0*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_5_BASE_ADDR;/*USX1*/
            devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_6_BASE_ADDR;/*CPU*/
        }

        /* LMU support */
        devObjPtr->memUnitBaseAddrInfo.lmu[0] = LMU_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.lmu[1] = LMU_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.lmu[2] = LMU_2_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.lmu[3] = LMU_3_BASE_ADDR;

        devObjPtr->numOfLmus = 4;

        devObjPtr->memUnitBaseAddrInfo.tsu[0] = TSU_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.tsu[1] = TSU_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.tsu[2] = TSU_2_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.tsu[3] = TSU_3_BASE_ADDR;

        {
            /* MAC MIB support : needed by smemChtGopMtiInitMacMibCounters(...) */
            GT_U32  unitIndex;

            /* support for the 400G MAC */
            unitIndex = sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr;
            devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MAC_MTIP_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MAC_MTIP_MAC_0_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MAC_MTIP_MAC_PCS_0_BASE_ADDR;

            unitIndex++;
            devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MAC_MTIP_MAC_1_BASE_ADDR + MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MAC_MTIP_MAC_1_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MAC_MTIP_MAC_PCS_1_BASE_ADDR;

            unitIndex++;
            devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MAC_MTIP_MAC_2_BASE_ADDR + MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MAC_MTIP_MAC_2_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MAC_MTIP_MAC_PCS_2_BASE_ADDR;

            unitIndex++;
            devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MAC_MTIP_MAC_3_BASE_ADDR + MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MAC_MTIP_MAC_3_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MAC_MTIP_MAC_PCS_3_BASE_ADDR;

            /* support for the USX MAC */
            unitIndex = sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_0_MAC_0_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_0_MAC_0_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_0_PCS_0_BASE_ADDR;

            unitIndex ++;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_1_MAC_0_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_1_MAC_0_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_1_PCS_0_BASE_ADDR;

            unitIndex ++;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_2_MAC_0_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_2_MAC_0_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_2_PCS_0_BASE_ADDR;

            unitIndex ++;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_0_MAC_1_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_0_MAC_1_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_0_PCS_1_BASE_ADDR;

            unitIndex ++;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_1_MAC_1_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_1_MAC_1_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_1_PCS_1_BASE_ADDR;

            unitIndex ++;
            devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex] = MTIP_USX_2_MAC_1_BASE_ADDR + USX_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_2_MAC_1_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_2_PCS_1_BASE_ADDR;
        }

        {
            /* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
            devObjPtr->memUnitBaseAddrInfo.cpuMacMib [0] = CPU_MAC_MTIP_MAC_BASE_ADDR + CPU_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[0] = CPU_MAC_MTIP_MAC_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.cpuMacPcs [0] = CPU_MAC_MTIP_MAC_PCS_BASE_ADDR;
        }

        /* ANP support */
        {
            GT_U32  numOfAnps = 0;

            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_0_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_1_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_2_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_3_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_4_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_5_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_6_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_7_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_8_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_9_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_10_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_11_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_12_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_13_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_14_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_15_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_16_BASE_ADDR;
        }

        devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);

        devObjPtr->dma_specialPortMappingArr = hawk_DMA_specialPortMappingArr;
        devObjPtr->gop_specialPortMappingArr = hawk_GOP_specialPortMappingArr;
        devObjPtr->ravens_specialPortMappingArr = NULL;
        devObjPtr->cpuPortSdma_specialPortMappingArr = hawk_cpuPortSdma_specialPortMappingArr;


        devObjPtr->tcam_numBanksForHitNumGranularity = 2; /* like BC2 */
        devObjPtr->tcamNumOfFloors   = 16; /*support 16 floors : each 3K of 10B = total 48K of 10B = 24K@20B */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->support_remotePhysicalPortsTableMode = 1;

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);/*1024 in Falcon , 512 in BC3, 128 in Aldrin2*/
        /*NOTE: support 8K eports like Falcon.
        */

        {
            GT_U32  index;
            /* bind CNC uploads of CNC[0,1] to MG[0] and CNC[2,3] to MG[1]*/
            for(index = 0 ; index < 4 ; index++)
            {
                devObjPtr->cncUnitInfo[index].mgUnit = (index < 2) ? 0 : 1;
            }
        }
        devObjPtr->cncNumOfUnits = 4;/* this is not 'sip' feature , but device specific */
        devObjPtr->policerSupport.iplrTableSize =  4*_1K;
        devObjPtr->policerSupport.numOfIpfix    = 64*_1K;
        devObjPtr->limitedResources.nextHop     = 16*_1K;
        devObjPtr->limitedResources.numOfArps   = 256*_1K;
        devObjPtr->limitedResources.ipvxEcmp    = 6*_1K;          /* each line 2 entries */

        devObjPtr->numofTcamClients = HAWK_TCAM_NUM_OF_GROUPS_CNS;

    }

    /* CNC0,1 will be set later by code of 'legacy devide' */
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CNC_2))
    {
        devObjPtr->memUnitBaseAddrInfo.CNC[2] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_CNC_2);
    }
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CNC_3))
    {
        devObjPtr->memUnitBaseAddrInfo.CNC[3] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_CNC_3);
    }

    SET_IF_ZERO_MAC(devObjPtr->tcamNumOfFloors,16);
    SET_IF_ZERO_MAC(devObjPtr->numofIPclProfileId,256);
    SET_IF_ZERO_MAC(devObjPtr->numofEPclProfileId,256);
    SET_IF_ZERO_MAC(devObjPtr->ipvxEcmpIndirectMaxNumEntries , 4*1024);/* 6K in falcon */
    SET_IF_ZERO_MAC(devObjPtr->defaultEPortNumEntries,SIP6_10_NUM_DEFAULT_E_PORTS_CNS);/*1K in Falcon*/
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort , 7);/*10 bits in falcon*/

    SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries,10*1024);/*14K in falcon */
    SET_IF_ZERO_MAC(devObjPtr->emMaxNumEntries, SMEM_MAC_TABLE_SIZE_256KB);/*128K in Falcon*/
    SET_IF_ZERO_MAC(devObjPtr->emAutoLearnNumEntries, 64*_1K);/* new table in Hawk */

#if 0 /* like Falcon */
    {
        SET_IF_ZERO_MAC(devObjPtr->fdbMaxNumEntries , SMEM_MAC_TABLE_SIZE_256KB);/*128K in Aldrin2*/

        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfLpmRams , 30);
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfEntriesBetweenRams , 32*1024);

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_SIP6_CNC_CLIENTS_BMP_ALL_CNS);
    }
#endif

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemHawkSpecificDeviceUnitAlloc;
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

    /* new erratum */
    devObjPtr->errata.eqAppSpecCpuCodeBothMode = 1;

    smemFalconInit(devObjPtr);

    SET_IF_ZERO_MAC(devObjPtr->tablesInfo.mcast.paramInfo[0].outOfRangeIndex,12*_1K);

    if (isHawk == GT_TRUE)
    {

        /* Hawk - bind special 'find memory' functions */
        smemHawkBindFindMemoryFunc(devObjPtr);

        /* Init RegDb for DP units */
        smemHawkInitRegDbDpUnits(devObjPtr);

        /* prepare multi MG units recognition */
        smemHawkPrepareMultiMgRecognition(devObjPtr);

        /* check that no unit exceed the size of it's unit */
        smemGenericUnitSizeCheck(devObjPtr,hawk_units);

        /* init the FIRMAWARE */
        simulationAc5pFirmwareInit(devObjPtr);
    }

}

/* update a register */
static void updateRegisterInAllPipes
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       regAddress,
    IN GT_U32                       startBit,
    IN GT_U32                       numBits,
    IN GT_U32                       value
)
{
    smemRegFldSet(devObjPtr,regAddress,startBit,numBits,value);
}

/**
* @internal smemHawkEgfLinkUpOnSlanBind function
* @endinternal
*
* @brief   debug - allow to set EGF link filter according to to Bind/Unbind SLAN to port.
*
* @param[in] devObjPtr                (pointer to) the device object
* @param[in] portNumber               - port number
*                                      if set with flag 0x80000000 , meaning that need to use map to RxDma virtualPortNum
* @param[in] bindRx                   - bind to Rx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] bindTx                   - bind to Tx direction ? GT_TRUE - yes , GT_FALSE - no
*                                       None
*/
void smemHawkEgfLinkUpOnSlanBind
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx
)
{
    GT_U32  regAddress,startBit,value;
    /* EGF link down filter */
    GT_U32  virtualPortNum;
    GT_U32  specialMode = 0;

    if(portNumber & 0x80000000)
    {
        specialMode = 1;
        /* running after RxDma mapping was done (by the supper image) */
        portNumber &= ~0x80000000;/* remove the flag */

        /* get the physical port number */
        smemRegFldGet(devObjPtr,
            SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,portNumber),
            0,
            SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 10 :
            SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 9 : 8,
            &virtualPortNum);
    }
    else
    {
        /* running before RxDma mapping was done */
        virtualPortNum =  portNumber;

        if(portNumber >= 59 ) /* convert MAC to skip the 'physical ports hole' 59..63 */
        {
            virtualPortNum += 5;
        }
    }

    if(virtualPortNum >= 128)
    {
        skernelFatalError("egfLinkUpOnSlanBind : unknown portNumber [%d] \n",
            portNumber);
    }

    /* set the SLAN ports in link up */
    regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(devObjPtr,virtualPortNum/32);
    startBit = virtualPortNum % 32;
    if(specialMode == 0)
    {
        value = bindTx == GT_TRUE ? 1 : 0;
    }
    else
    {
        value = bindTx == GT_TRUE ? 1 : 0;
        if(value)/* if egf link up , then check if link is up too */
        {
            /* check the link status of the port , that should already by set with configuration */
            value = snetChtPortMacFieldGet(devObjPtr, portNumber,
                SNET_CHT_PORT_MAC_FIELDS_LinkState_E);

            if(value == 0)
            {
                simForcePrintf("WM : EGF link of physical port[%d] (macNum[%d]) FORCED to down because hold no link on the MAC \n",
                    virtualPortNum , portNumber);
            }
        }
    }


    simForcePrintf("WM : EGF link of physical port[%d] (macNum[%d]) set to [%s] \n",
        virtualPortNum , portNumber ,
        value ? "Link Up":"Link Down");

    /* need to update the register in the 2 pipes */
    updateRegisterInAllPipes(devObjPtr,regAddress,startBit,1,value);
}

void snetHawkMifMacEnableTraffic
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  macPort ,
    IN GT_U32                  local_dma_port_number
)
{
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32  regAddr,mifType;
    GT_U32  isCpuPort;
    GT_U32  cpuPortIndex;
    GT_U32  sip6_MTI_EXTERNAL_representativePort;
    GT_U32  local_mif_Rx_channel_id_number;
    GT_U32  isPreemptiveChannel = 0;
    GT_U32  regValue;

    isCpuPort = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E;
    if(isCpuPort)
    {
        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,macPort,&portInfo);
        cpuPortIndex = portInfo.simplePortInfo.unitIndex;
    }
    else
    {
        cpuPortIndex = 0;/* dont care */
    }

    /* get the local channel index in this unit */
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E  ,macPort,&portInfo);
    local_mif_Rx_channel_id_number =  portInfo.simplePortInfo.indexInUnit;

    /* MIF hold dedicated logic for representative port (apply to 'CPU port' too!) */
    sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;

    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_TYPE_E  ,macPort,&portInfo);
    mifType  = portInfo.simplePortInfo.unitIndex;

    if(isPreemptiveChannel)
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).preemption.mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).preemption.mif_channel_mapping_register[mifType];
        }
    }
    else
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).mif_channel_mapping_register[mifType];
        }
    }

    /*update the register via SCIB for active memory */
    scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
    SMEM_U32_SET_FIELD(regValue,18,1,1); /*clock enable*/
    SMEM_U32_SET_FIELD(regValue, 0,6,local_dma_port_number); /*Tx*/
    SMEM_U32_SET_FIELD(regValue, 8,6,local_dma_port_number); /*Rx*/
    scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);

    if(isCpuPort)
    {
        regAddr = MTI_CPU_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_rx_control_register[mifType];
    }
    else
    {
        regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_rx_control_register[mifType];
    }

    /*update the register via SCIB for active memory */
    scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
    SMEM_U32_SET_FIELD(regValue,local_mif_Rx_channel_id_number,1,1); /*Rx enable*/
    scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);

    regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_tx_control_register[mifType];
    /* NOTE : for preemption channel the 'local_mif_channel_number' is the 'preemption index' */
    scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
    SMEM_U32_SET_FIELD(regValue,local_mif_Rx_channel_id_number,1,1); /*Tx enable*/
    scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);



    return;
}

/* global flag to allow control the option to make 'micro-init' for WM without enabling the Rx,Tx of the ports */
static GT_U32   smemHawkInit_debug_enable_rx_tx_all_ports = 1;
/* debug function to allow control the option to make 'micro-init' for WM without enabling the Rx,Tx of the ports */
GT_STATUS smemHawkInit_debug_enable_rx_tx_all_ports_set(IN GT_U32    enable)
{
    smemHawkInit_debug_enable_rx_tx_all_ports = enable ? 1 : 0;
    return GT_OK;
}

GT_U32 smemHawkInit_debug_enable_rx_tx_all_ports_get(void)
{
    return smemHawkInit_debug_enable_rx_tx_all_ports;
}

static GT_U32   smemHawkInit_debug_mru = 0x5F2;/*1522*/
GT_STATUS smemHawkInit_debug_mru_set(IN GT_U32 mru)
{
    smemHawkInit_debug_mru = mru;
    return GT_OK;
}

/**
* @internal smemHawkInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{

    GT_BOOL isHarrier = (devObjPtr->deviceFamily == SKERNEL_HARRIER_FAMILY)?GT_TRUE:GT_FALSE;

    GT_U32 harrierEgfQagConfig[128];
    GT_U32 goqIndex =0 ;
    /*temporary configuration that should be removed once HWS relevant code is in MI*/

    GT_U32 additionalConfig[][2]=
        {
            /*<FDB> MT/Units/FDB_IP_Units/FDB Global Configuration/FDB Global Configuration 1*/
            {0x04000004 ,0x37011b2c},/*fid16BitHashEn value should be  0*/
            /*<FDB> MT/Units/FDB_IP_Units/FDB Global Configuration/FDB Global Configuration*/
            {0x04000000 ,0x107bfc48},/*vlanMode value should be 1 */

            {0xfffffffe ,0xfffffffe}/*last*/
        };

    /*Match CPSS init*/

    if(isHarrier==GT_TRUE)
    {
        GT_U32  ii;

        memset(harrierEgfQagConfig,0xFE,128*sizeof(GT_U32));
        for(ii=0;ii<=38;ii+=2,goqIndex+=2)
        {
            if(ii==16||ii==32)
            {
              goqIndex+=2;
            }

            harrierEgfQagConfig[ii]=goqIndex;
        }
         /*CPU  SDMA ports*/
         harrierEgfQagConfig[60] = 34;
         harrierEgfQagConfig[63] = 16;

        /*Allign to  CPSS init*/
        for(ii=0;additionalConfig[ii][0]!=0xfffffffe;ii++)
        {
            smemRegSet(devObjPtr ,
                additionalConfig[ii][0], additionalConfig[ii][1]);
        }
    }

    if(smemHawkInit_debug_enable_rx_tx_all_ports)
    {
        /* EGF link down filter */
        /* set the SLAN ports in link up */
        devObjPtr->devSlanBindPostFuncPtr = smemHawkEgfLinkUpOnSlanBind;
    }

    {
        GT_U32  ii;
        GT_U32  regAddress;
        GT_U32  value;
        GT_U32  numPortsPerTile = devObjPtr->portsNumber;

        /* port per register */
        for(ii = 0 ; ii < numPortsPerTile ; ii++)
        {
            regAddress = SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,ii);

            value = ii;

            /*perform RXDMA mapping from local port to 'virual' port on the field of:
              localDevSrcPort */
            smemRegFldSet(devObjPtr,regAddress,
                0, 9, value);
        }
    }

    {
        GT_U32  ii;
        GT_U32  regAddress = SMEM_BOBCAT2_BMA_PORT_MAPPING_TBL_MEM(devObjPtr,0);
        GT_U32  value;

        /* /Cider/EBU/Bobcat3/Bobcat3 {Current}/Switching Core/BMA_IP/Tables/<BMA_IP> BMA_IP Features/Port Mapping/Port Mapping entry */
        /* set mapping 1:1 */

        /* port per register */
        for(ii = 0 ; ii < 128 ; ii++ , regAddress+=4)
        {
            value = ii % 32 ;/* local number*/
            updateRegisterInAllPipes(devObjPtr,regAddress,0,32,value);
        }
    }

    {
        GT_U32  ii;

        for(ii = 0 ; ii < 128 ; ii++)
        {
            if(!IS_CHT_VALID_PORT(devObjPtr,ii))
            {
                continue;
            }

            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E,smemHawkInit_debug_enable_rx_tx_all_ports);
            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E,smemHawkInit_debug_enable_rx_tx_all_ports);
            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_mru_E,smemHawkInit_debug_mru);
        }

        /* set MRU for the vlan prifile 0 that default vlan uses */
        updateRegisterInAllPipes(devObjPtr,
            SMEM_CHT2_MRU_PROFILE_REG(devObjPtr, 0/*mruEntryIndex*/),
            0,14,smemHawkInit_debug_mru);
    }

    {
        GT_U32  ii;
        GT_U32  value;
        GT_U32  regAddress = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,0);
        if(isHarrier==GT_TRUE)
        {
             /*Allign to  CPSS init*/
            for(ii = 0 ; ii <128 ; ii++)
            {
                /* EGF_QAG -  Target Port Mapper table */
                regAddress = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,ii);

                value = harrierEgfQagConfig[ii];

                if(value!=0xFEFEFEFE)
                {
                    /* set only 10 bits , to not harm other defaults */
                    updateRegisterInAllPipes(devObjPtr,regAddress,0,10,value);
                }
            }
        }
        else
        {
            for(ii = 0 ; ii < 128 ; ii++)
            {
                /* EGF_QAG -  Target Port Mapper table */
                regAddress = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,ii);
                /* currently 1:1 */
                value = ii;

                /* set only 10 bits , to not harm other defaults */
                updateRegisterInAllPipes(devObjPtr,regAddress,0,10,value);
            }
        }
    }

    {
        GT_U32  value = 0x1;/* 128 ports mode */
        GT_U32  regAddress;

        regAddress = SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr);
        /*EGF_QAG_TableMode*/
        updateRegisterInAllPipes(devObjPtr,regAddress,15, 3, value);

        regAddress = SMEM_CHT_BRDG_GLB_CONF0_REG(devObjPtr);
        /*l2i_TableMode*/
        updateRegisterInAllPipes(devObjPtr,regAddress,28, 3, value);


        regAddress = SMEM_LION_TXQ_SHT_GLOBAL_CONF_REG(devObjPtr);
        /*EGF_SHT_TableMode*/
        updateRegisterInAllPipes(devObjPtr,regAddress,8, 3, value);
    }

    /* allow vidx 0x7ff to hold all ports (only second half is of the '0xfff')*/
    {
        GT_U32  regAddress;
        GT_U32  vidx = 0xfff;
        GT_U32  value = 0xFFFFFFFF;

        regAddress = SMEM_CHT_MCST_TBL_MEM(devObjPtr, vidx);

        updateRegisterInAllPipes(devObjPtr,regAddress + 0x0,0, 32, value);
        updateRegisterInAllPipes(devObjPtr,regAddress + 0x4,0, 32, value);
        updateRegisterInAllPipes(devObjPtr,regAddress + 0x8,0, 32, value);
        updateRegisterInAllPipes(devObjPtr,regAddress + 0xc,0, 32, value);
    }

    {
        GT_U32  ii,jj;
        GT_U32  value;

        for(ii = 0 ; ii < devObjPtr->multiDataPath.maxDp; ii++)
        {
            /*enable ports at the SDQ */
            value = 0x1;

           for(jj=0;jj<devObjPtr->multiDataPath.info[ii].dataPathNumOfPorts;jj++)
           {
                smemRegSet(devObjPtr ,
                SMEM_SIP6_TXQ_SDQ_GLOBAL_PORT_ENABLE_REG(devObjPtr,ii,jj) , value);
           }
        }
    }
#define HELP_CPSS_IN_MISSING_CONFIG
#ifdef HELP_CPSS_IN_MISSING_CONFIG
    /* MAP the TXQ-SDQ queue index to local DMA port number (by using ranges of 8 queues per port)*/
    {
        GT_U32  ii,jj;
        GT_U32  value;

        for(ii = 0 ; ii < devObjPtr->multiDataPath.maxDp; ii++)
        {
            value = 0;
            for(jj = 0 ; jj < devObjPtr->multiDataPath.info[ii].dataPathNumOfPorts; jj++)
            {
                smemRegSet(devObjPtr ,
                    SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_LOW_REG(devObjPtr,ii,jj) , value);

                    value += 7;
                smemRegSet(devObjPtr ,
                    SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_HIGH_REG(devObjPtr,ii,jj) , value);
                value ++;
            }
        }
    }


    {
        GT_U32  ii;
        GT_U32  value;
        GT_U32  queue_pds_index,queue_base;
        GT_U32  numPorts = devObjPtr->portsNumber > 64 ? devObjPtr->portsNumber : 128;
        GT_U32 portsPerDp=devObjPtr->multiDataPath.info[0].dataPathNumOfPorts;

        if(isHarrier==GT_TRUE)
        {
            portsPerDp =17;/*also cpu port is mapped*/
            numPorts = 44;/*number of taken entries*/
        }

        /*for(tileId = 0 ; tileId < devObjPtr->numOfTiles;tileId++)*/
        {
            for(ii = 0 ; ii < numPorts; ii++)
            {
                queue_base = 0;
                queue_pds_index = 0;

                if(ii < 59)/* physical ports mapped 1:1 with global MAC/DMA*/
                {
                    queue_base = 8 * (ii %portsPerDp);
                    queue_pds_index = ii /portsPerDp;

                    if(isHarrier == GT_TRUE)
                    {
                        if((ii==17)||(ii==35))
                        {
                            continue;
                        }
                        else if(ii>17&&ii<35)
                        {
                            queue_base = 8 * ((ii-1) %portsPerDp);
                            queue_pds_index = (ii-1) /portsPerDp;
                        }
                        else  if(ii>35)
                        {
                            queue_base = 8 * ((ii-2) %portsPerDp);
                            queue_pds_index = (ii-2) /portsPerDp;
                        }

                    }
                }
                else if (ii < 63 )
                {
                    /*keep 0*/
                }
                else if (ii == 63) /* SDMA CPU port */
                {
                    queue_base = portsPerDp* 8;
                    queue_pds_index = 0;/*DP[0]*/
                }
                else if (ii < 104)
                {
                    queue_base = (8*4)/*skip60..63*/ + 8 * (ii % portsPerDp);
                    queue_pds_index = ii /portsPerDp;
                }
                else if (ii == 104) /* network CPU port */
                {
                    queue_base = portsPerDp * 8;
                    queue_pds_index = 1;/*DP[1]*/
                }
                else
                {
                    /*keep 0*/
                }

                value = 0;

                SMEM_U32_SET_FIELD(value,0,8,queue_base);
                SMEM_U32_SET_FIELD(value,8,5,0/*dp_core_local_trg_port -- not care for simulation ... so 0 */);
                SMEM_U32_SET_FIELD(value,13,3,queue_pds_index);
                SMEM_U32_SET_FIELD(value,16,2,0/*queue_pdx_index -- tileId*/);



                smemRegSet(devObjPtr ,SMEM_SIP6_TXQ_PDX_QUEUE_GROUP_MAP_TBL_MEM(devObjPtr,
                    ii,0),value);
            }
        }
    }
#endif /*HELP_CPSS_IN_MISSING_CONFIG*/

    /* set <Flood eVIDX> */
    smemRegSet(devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTGlobalConfigs,
        0x1FFF007);

    /* In micro-init profiles the 'create' of port will set it's needed values */
    if(smemHawkInit_debug_enable_rx_tx_all_ports)
    {
        GT_U32  ii,localDmaPort;
        /* add here logic for Harrier that differ/override from Hawk */
        for(ii = 0 ; ii < devObjPtr->portsNumber;ii++)
        {
            if(!IS_CHT_VALID_PORT(devObjPtr,ii))
            {
                continue;
            }

            if(devObjPtr->portsArr[ii].state != SKERNEL_PORT_STATE_MTI_50_E  &&
               devObjPtr->portsArr[ii].state != SKERNEL_PORT_STATE_MTI_100_E &&
               devObjPtr->portsArr[ii].state != SKERNEL_PORT_STATE_MTI_200_E &&
               devObjPtr->portsArr[ii].state != SKERNEL_PORT_STATE_MTI_400_E)
            {
                continue;
            }


            localDmaPort = ii % devObjPtr->multiDataPath.info[0].dataPathNumOfPorts;
            snetHawkMifMacEnableTraffic(devObjPtr,ii,localDmaPort);
        }
    }

    /* <DeviceEn> -- enable only after the mapping of DP units are done */
    smemDfxRegFldSet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, 1);
}

/**
* @internal smemHawkInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemHawkInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    internal_smemHawkInit(devObjPtr);
}
/**
* @internal smemHawkInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    static int my_dummy = 0;

    smemFalconInit2(devObjPtr);

    {
        /* fix single register default in EFT that unit is not aligned yet ! */
        smemRegSet(devObjPtr ,
            SMEM_LION3_EGF_EFT_PORT_ISOLATION_LOOKUP_1_REG(devObjPtr),
            0x0000009c);
    }


    if(my_dummy)
    {
        smemHawkInit_debug_allowTraffic(devObjPtr);
    }
}


