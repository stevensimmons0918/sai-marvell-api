/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemFalcon.c
*
* DESCRIPTION:
*       Falcon memory mapping implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemFalcon.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <common/Utils/Math/sMath.h>

#define SIP6_NUM_DEFAULT_E_PORTS_CNS   (_1K)/* was 512 in sip5 */

extern void smemBobcat3SpecificDeviceMemInitPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemFalconActiveWriteExactMatchMsg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
static void smemFalconActiveWriteExactMatchGlobalCfg1Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
static void smemFalconActiveWriteExactMatchGlobalCfg2Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
static void smemFalconActiveWriteAacTableMemory
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);
static void smemFalconActiveWriteFdbGlobalCfg2Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);

extern GT_VOID microInitCm3Trigger
(
    GT_VOID
);

extern GT_STATUS microInitCm3Unq_pollingPerProjectUserFuntion
(
    GT_VOID
);



#define FALCON_PORT_STATSTIC_COUNTER_REG_ADDR_GET(dev, port, counter,isPmacMib) \
    ((!isPmacMib)?                                                                           \
     (*(&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[port].MTI_STATS.RX_etherStatsOctets+counter)) : \
     (*(&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[port].MTI_PREEMPTION_STATS.RX_etherStatsOctets+counter)))


#define FALCON_NUM_GOP_PORTS_GIG   (32+1) /*per pipe*/
#define FALCON_NUM_GOP_PORTS_XLG   (32+1) /*per pipe*/
#define FALCON_NUM_GOP_PORTS_CG_100G   8  /* per pipe , steps of 4 ports */

/* indication of init value in runtime ... not during compilation */
#define INIT_INI_RUNTIME    0

#define PER_TILE_INDICATION_CNS     DUMMY_PARAM_NAME_PTR_CNS(1)
#define PER_2_TILES_INDICATION_CNS  DUMMY_PARAM_NAME_PTR_CNS(2)

#define PIPE_MEM_SIZE         0x08000000
#define PIPE_0_START_ADDR     0x08000000/*start of pipe 0*/
#define PIPE_0_END_ADDR       (PIPE_0_START_ADDR + PIPE_MEM_SIZE - 4)/* end of pipe 0 */
#define PIPE_1_START_ADDR     0x10000000/*start of pipe 1*/
#define PIPE_1_END_ADDR       (PIPE_1_START_ADDR + PIPE_MEM_SIZE - 4)/* end of pipe 0 */

#define PIPE_1_FROM_PIPE_0_OFFSET   (PIPE_1_START_ADDR - PIPE_0_START_ADDR)

#define RAVEN_PIPE_OFFSET     0x02000000
#define RAVEN_MEM_SIZE        0x00800000
#define RAVEN_0_START_ADDR    0x00000000
#define RAVEN_1_START_ADDR    0x01000000
#define RAVEN_2_START_ADDR    0x02000000
#define RAVEN_3_START_ADDR    0x03000000
#define RAVEN_3_END_ADDR      RAVEN_3_START_ADDR + (RAVEN_MEM_SIZE - 4)

#define GOP_CHANNEL_OFFSET 0x80000

/* offset between tile 0 to tile 1 to tile 2 to tile 3 */
#define TILE_OFFSET          0x20000000

#define FALCON_NUM_MG_PER_TILE  4
/* the size in bytes of the MG unit */
#define MG_SIZE             _1M
/* base address of the CNM unit that is single for 2 tiles */
#define CNM_OFFSET_CNS       0x1C000000
/* base address of the MG 0_0 unit . MG_0_0 to MG_0_3 serve tile 0 */
#define MG_0_0_OFFSET_CNS    0x1D000000
/* base address of the MG 0_1 unit . MG_0_1 is part of tile 0 */
#define MG_0_1_OFFSET_CNS    (MG_0_0_OFFSET_CNS + 1*MG_SIZE)
/* base address of the MG 0_1 unit . MG_1_0 to MG_1_3 serve tile 1 */
#define MG_1_0_OFFSET_CNS    (MG_0_0_OFFSET_CNS + FALCON_NUM_MG_PER_TILE*MG_SIZE)

#define TILE_0      0
#define TILE_1      1
#define TILE_2      2
#define TILE_3      3
/* define for tile 1,3 (that are mirrored to tile 0. tile 2 is like tile 0) */
#define TILE_1_BIT  (1 << TILE_1)
#define TILE_3_BIT  (1 << TILE_3)

/* Tile 1 : pipe 2,3 : are mirrored to addresses in Pipe 0 */
#define UNIT_IN_PIPES_2_3_INSTANCE(baseAddr,unitName,size) \
     {baseAddr + TILE_1*TILE_OFFSET + 0                         , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),1,1) , size , STR(unitName)},\
     {baseAddr + TILE_1*TILE_OFFSET + PIPE_1_FROM_PIPE_0_OFFSET , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),0,1) , size , STR(unitName)}

/* Tile 2 : pipe 4,5 : like in Pipe 0 */
#define UNIT_IN_PIPES_4_5_INSTANCE(baseAddr,unitName,size) \
     {baseAddr + TILE_2*TILE_OFFSET + 0                         , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),0,2) , size , STR(unitName)},\
     {baseAddr + TILE_2*TILE_OFFSET + PIPE_1_FROM_PIPE_0_OFFSET , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),1,2) , size , STR(unitName)}

/* Tile 3 : pipe 6,7 : are mirrored to addresses in Pipe 0 */
#define UNIT_IN_PIPES_6_7_INSTANCE(baseAddr,unitName,size) \
     {baseAddr + TILE_3*TILE_OFFSET + 0                         , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),1,3) , size , STR(unitName)},\
     {baseAddr + TILE_3*TILE_OFFSET + PIPE_1_FROM_PIPE_0_OFFSET , ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(STR(unitName),0,3) , size , STR(unitName)}


/* unit per pipe (exists in pipe 0 and in pipe 1) */
#define PER_PIPE_UNIT_INFO_MAC(baseAddr,unitName,size)  \
    /* add the unit to the pipe 0 addresses */          \
     {baseAddr /*Cider address of unit in pipe 0 */ , STR(unitName) , size}, \
    /* add the unit to the pipe 1 addresses */          \
     {baseAddr + PIPE_1_FROM_PIPE_0_OFFSET          , ADD_INSTANCE_OF_UNIT_TO_STR(STR(unitName),1) , size , STR(unitName)}, \
     /* add the units in pipe 2,3*/                     \
     UNIT_IN_PIPES_2_3_INSTANCE(baseAddr,unitName,size),\
     /* add the units in pipe 4,5*/                     \
     UNIT_IN_PIPES_4_5_INSTANCE(baseAddr,unitName,size),\
     /* add the units in pipe 6,7*/                     \
     UNIT_IN_PIPES_6_7_INSTANCE(baseAddr,unitName,size)

/* unit per tile (shared to pipe 0 and pipe 1) */
#define PER_TILE_UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr + TILE_0*TILE_OFFSET , STR(unitName)                                , size , PER_TILE_INDICATION_CNS}, \
     {baseAddr + TILE_1*TILE_OFFSET , ADD_INSTANCE_OF_TILE_TO_STR(STR(unitName),1) , size , STR(unitName)}, \
     {baseAddr + TILE_2*TILE_OFFSET , ADD_INSTANCE_OF_TILE_TO_STR(STR(unitName),2) , size , STR(unitName)}, \
     {baseAddr + TILE_3*TILE_OFFSET , ADD_INSTANCE_OF_TILE_TO_STR(STR(unitName),3) , size , STR(unitName)}

/* unit per 2 tiles (shared to pipes 0,1,2,3) */
#define PER_2_TILES_UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr + TILE_0*TILE_OFFSET , STR(unitName)                                , size , PER_2_TILES_INDICATION_CNS}, \
     {baseAddr + TILE_2*TILE_OFFSET , ADD_INSTANCE_OF_TILE_TO_STR(STR(unitName),2) , size , STR(unitName)}

/* single unit in the device */
#define SINGLE_UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName) , size , 0}

/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/Projects/EAGLE/
Shared%20Documents/Design/AXI/Address%20Space/Eagle_Address_Space_falcon_171111.xlsm
*/
/* the units of falcon */
static SMEM_GEN_UNIT_INFO_STC falcon_units[] =
{
    /****************************************************************/
    /* NOTE: the table is sorted according to the base address ...  */
    /****************************************************************/

     PER_PIPE_UNIT_INFO_MAC(0x08000000,UNIT_LPM               ,16  * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x09000000,UNIT_IPVX              ,8   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x09800000,UNIT_L2I               ,8   * _1M)

    ,PER_PIPE_UNIT_INFO_MAC(0x0A000000,UNIT_CNC               ,256 *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A040000,UNIT_CNC_1             ,256 *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A080000,UNIT_IPCL              ,512 *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A100000,UNIT_EPLR              ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A200000,UNIT_EOAM              ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A300000,UNIT_EPCL              ,512 *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A380000,UNIT_PREQ              ,512 *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A400000,UNIT_ERMRK             ,4   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A800000,UNIT_TAI_SLAVE_PIPE0_TAI0       ,     64  * _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0A810000,UNIT_TAI_SLAVE_PIPE0_TAI1       ,     64  * _1K)


    ,PER_PIPE_UNIT_INFO_MAC(0x0AB00000,UNIT_IOAM              ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0AC00000,UNIT_MLL               ,2   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0AE00000,UNIT_IPLR              ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0AF00000,UNIT_IPLR1             ,1   * _1M)

    ,PER_PIPE_UNIT_INFO_MAC(0x0B000000,UNIT_EQ                ,16  * _1M)

    ,PER_PIPE_UNIT_INFO_MAC(0x0C900000,UNIT_EGF_QAG           ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0CA00000,UNIT_EGF_SHT           ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0CB00000,UNIT_EGF_EFT           ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0CC00000,UNIT_TTI               ,4   * _1M)

    ,PER_PIPE_UNIT_INFO_MAC(0x0D3F0000,UNIT_EREP              ,64  *     _1K)/* moved from 0x0C800000 */
    ,PER_PIPE_UNIT_INFO_MAC(0x0D400000,UNIT_BMA               ,1   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D500000,UNIT_HBU               ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D510000,UNIT_IA                ,64  *     _1K)

    ,PER_PIPE_UNIT_INFO_MAC(0x0D520000,UNIT_RX_DMA            ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D530000,UNIT_RX_DMA_1          ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D540000,UNIT_RX_DMA_2          ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D550000,UNIT_RX_DMA_3          ,64  *     _1K)

    ,PER_PIPE_UNIT_INFO_MAC(0x0D560000,UNIT_TX_DMA            ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D570000,UNIT_TX_FIFO           ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D580000,UNIT_TX_DMA_1          ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D590000,UNIT_TX_FIFO_1         ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D5a0000,UNIT_TX_DMA_2          ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D5b0000,UNIT_TX_FIFO_2         ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D5c0000,UNIT_TX_DMA_3          ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D5d0000,UNIT_TX_FIFO_3         ,64  *     _1K)


    ,PER_PIPE_UNIT_INFO_MAC(0x0D600000,UNIT_TXQ_SDQ0  ,128  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D620000,UNIT_TXQ_SDQ1  ,128  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D640000,UNIT_TXQ_PDS0  ,256  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D680000,UNIT_TXQ_PDS1  ,256  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D6c0000,UNIT_TXQ_QFC0  ,64   *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D6d0000,UNIT_TXQ_QFC1  ,64   *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D6E0000,UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0   ,     64  * _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D700000,UNIT_TXQ_SDQ2  ,128  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D720000,UNIT_TXQ_SDQ3  ,128  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D740000,UNIT_TXQ_PDS2  ,256  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D780000,UNIT_TXQ_PDS3  ,256  *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D7c0000,UNIT_TXQ_QFC2  ,64   *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D7d0000,UNIT_TXQ_QFC3  ,64   *    _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D7E0000,UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1   ,     64  * _1K)

    ,PER_PIPE_UNIT_INFO_MAC(0x0D7F0000,UNIT_EAGLE_D2D   ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D800000,UNIT_EAGLE_D2D_1 ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D810000,UNIT_EAGLE_D2D_2 ,64  *     _1K)
    ,PER_PIPE_UNIT_INFO_MAC(0x0D820000,UNIT_EAGLE_D2D_3 ,64  *     _1K)

    ,PER_PIPE_UNIT_INFO_MAC(0x0E000000,UNIT_HA                ,4   * _1M)
    ,PER_PIPE_UNIT_INFO_MAC(0x0C000000,UNIT_PHA               ,8   * _1M)

    /* units per tile */
    ,PER_TILE_UNIT_INFO_MAC(0x18000000,UNIT_TCAM              ,8    * _1M)
    ,PER_TILE_UNIT_INFO_MAC(0x18800000,UNIT_TXQ_PDX_PAC_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x18810000,UNIT_TXQ_PDX_PAC_1     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x18840000,UNIT_TXQ_PDX           ,256 *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x18880000,UNIT_TXQ_PFCC           ,64    * _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x18890000,UNIT_SHM               ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x188A0000,UNIT_EM                ,64  *     _1K)/*EM unit*/
    ,PER_TILE_UNIT_INFO_MAC(0x188B0000,UNIT_FDB               ,64  *     _1K)/*MT unit*/
    ,PER_TILE_UNIT_INFO_MAC(0x18900000,UNIT_TXQ_PSI           ,1    * _1M)

    /* packet buffer subunits */
    ,PER_TILE_UNIT_INFO_MAC(0x19000000, UNIT_PB_CENTER_BLK              ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19030000, UNIT_PB_WRITE_ARBITER           ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19040000, UNIT_PB_GPC_GRP_PACKET_WRITE_0  ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19060000, UNIT_PB_GPC_GRP_CELL_READ_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19070000, UNIT_PB_GPC_GRP_CELL_READ_1     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19080000, UNIT_PB_GPC_GRP_PACKET_READ_0   ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19090000, UNIT_PB_GPC_GRP_PACKET_READ_1   ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x190A0000, UNIT_PB_GPC_GRP_PACKET_WRITE_1  ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x190C0000, UNIT_PB_GPC_GRP_CELL_READ_2     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x190D0000, UNIT_PB_GPC_GRP_CELL_READ_3     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x190E0000, UNIT_PB_GPC_GRP_PACKET_READ_2   ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x190F0000, UNIT_PB_GPC_GRP_PACKET_READ_3   ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19100000, UNIT_PB_NEXT_POINTER_MEMO_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19110000, UNIT_PB_NEXT_POINTER_MEMO_1     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19120000, UNIT_PB_NEXT_POINTER_MEMO_2     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19130000, UNIT_PB_SHARED_MEMO_BUF_0_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19140000, UNIT_PB_SHARED_MEMO_BUF_0_1     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19150000, UNIT_PB_SHARED_MEMO_BUF_1_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19160000, UNIT_PB_SHARED_MEMO_BUF_1_1     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19170000, UNIT_PB_SHARED_MEMO_BUF_2_0     ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x19180000, UNIT_PB_SHARED_MEMO_BUF_2_1     ,64  *     _1K)

    /* DFX */
    ,PER_TILE_UNIT_INFO_MAC(0x1BE00000, UNIT_DFX_SERVER       ,1  * _1M)

    /* TAI 0,1 are units per tile */
    ,PER_TILE_UNIT_INFO_MAC(0x1BF00000,UNIT_TAI               ,64  *     _1K)
    ,PER_TILE_UNIT_INFO_MAC(0x1BF80000,UNIT_TAI_1             ,64  *     _1K)

    /*NOTE: MG0 is part of the CnM : 0x1D000000 size 1M */
    ,PER_2_TILES_UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG    /*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_0_1/*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 2*MG_SIZE ,UNIT_MG_0_2/*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 3*MG_SIZE ,UNIT_MG_0_3/*CNM*/, MG_SIZE)

    ,PER_2_TILES_UNIT_INFO_MAC(MG_1_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG_1_0/*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_1_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_1_1/*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_1_0_OFFSET_CNS + 2*MG_SIZE ,UNIT_MG_1_2/*CNM*/, MG_SIZE)
    ,PER_2_TILES_UNIT_INFO_MAC(MG_1_0_OFFSET_CNS + 3*MG_SIZE ,UNIT_MG_1_3/*CNM*/, MG_SIZE)

    /* SRAM  is in the CnM section */
    ,PER_2_TILES_UNIT_INFO_MAC(0x1C000000,UNIT_CNM_SRAM   ,64   *     _1K)/*in CnM */

    /* RFU is in the CnM section */
    ,PER_2_TILES_UNIT_INFO_MAC(0x1C100000,UNIT_CNM_RFU  ,64   *     _1K) /*in CnM */

    /* SMI is in the CnM section */
    ,PER_2_TILES_UNIT_INFO_MAC(0x1C110000,UNIT_GOP_SMI_0  ,64   *     _1K) /*in CnM */
    ,PER_2_TILES_UNIT_INFO_MAC(0x1C120000,UNIT_GOP_SMI_1  ,64   *     _1K) /*in CnM */

     /* AAC is in the CnM section */
    ,PER_2_TILES_UNIT_INFO_MAC(0x1C130000,UNIT_CNM_AAC  ,64   *     _1K) /*in CnM */

     /* PEX MAC is in the CnM section */
    ,SINGLE_UNIT_INFO_MAC(0x1C160000,UNIT_CNM_PEX_MAC   ,64   *     _1K)/*in CnM */

     /* PEX COMPHY is in the CnM section */
    ,SINGLE_UNIT_INFO_MAC(0x1C140000,UNIT_CNM_PEX_COMPHY   ,64   *     _1K)/*in CnM */



/* in the memory space of Raven 0
    ,PER_TILE_UNIT_INFO_MAC(0x002e0000,UNIT_EAGLE_D2D_CP_0,64  * _1K)
*/

    /* The Ravens */
    ,PER_TILE_UNIT_INFO_MAC(0x00000000,UNIT_BASE_RAVEN_0      ,8    * _1M)
    ,PER_TILE_UNIT_INFO_MAC(0x01000000,UNIT_BASE_RAVEN_1      ,8    * _1M)
    ,PER_TILE_UNIT_INFO_MAC(0x02000000,UNIT_BASE_RAVEN_2      ,8    * _1M)
    ,PER_TILE_UNIT_INFO_MAC(0x03000000,UNIT_BASE_RAVEN_3      ,8    * _1M)

    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

static SMEM_GEN_UNIT_INFO_STC SORTED___falcon_units[sizeof(falcon_units)/sizeof(falcon_units[0])] =
{
    /* sorted and build during smemFalconInit(...) from falcon_units[] */
    {SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};


/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from falcon_units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC falcon_duplicatedUnits[1500] =
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

    {STR(UNIT_EAGLE_D2D)  ,3}, /* 3 duplication of this unit */
        {STR(UNIT_EAGLE_D2D_1)},
        {STR(UNIT_EAGLE_D2D_2)},
        {STR(UNIT_EAGLE_D2D_3)},

   {STR(UNIT_MG)        ,7}, /* 7 duplication of this unit (CnM : per 2 tiles) */
        {STR(UNIT_MG_0_1)},
        {STR(UNIT_MG_0_2)},
        {STR(UNIT_MG_0_3)},
        {STR(UNIT_MG_1_0)},
        {STR(UNIT_MG_1_1)},
        {STR(UNIT_MG_1_2)},
        {STR(UNIT_MG_1_3)},

    {STR(UNIT_TAI)  ,1},  /* 1 duplication of this unit (per tile : not per pipe !!!) */
        {STR(UNIT_TAI_1)},

    {STR(UNIT_TAI)  ,4},  /* 4 more per pipe */
        {STR(UNIT_TAI_SLAVE_PIPE0_TAI0)},
        {STR(UNIT_TAI_SLAVE_PIPE0_TAI1)},
        {STR(UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0)},
        {STR(UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1)},

    /* start duplication on units of instance 0 , to instance 1 !!! */
    {STR(UNIT_BASE_RAVEN_0)  ,3},  /* 1 duplication of this unit (per tile : not per pipe !!!) */
        {STR(UNIT_BASE_RAVEN_1)},
        {STR(UNIT_BASE_RAVEN_2)},
        {STR(UNIT_BASE_RAVEN_3)},

    /* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
        it is built from falcon_units[].orig_nameStr*/


    {NULL,0} /* must be last */
};
/* the DP that hold MG0_0*/
#define CNM0_DP_FOR_MG_0_0   4
/* the DP that hold MG0_1*/
#define CNM0_DP_FOR_MG_0_1   5
/* the DP that hold MG0_2*/
#define CNM0_DP_FOR_MG_0_2   6
/* the DP that hold MG0_3*/
#define CNM0_DP_FOR_MG_0_3   7

#define CNM0_DP_FOR_MG_1_0   11     /* local DP[4] in mirrored tile 1 */
#define CNM0_DP_FOR_MG_1_1   10     /* local DP[5] in mirrored tile 1 */
#define CNM0_DP_FOR_MG_1_2    9     /* local DP[6] in mirrored tile 1 */
#define CNM0_DP_FOR_MG_1_3    8     /* local DP[7] in mirrored tile 1 */

#define CNM1_DP_FOR_MG_0_0   (16 + CNM0_DP_FOR_MG_0_0)
#define CNM1_DP_FOR_MG_0_1   (16 + CNM0_DP_FOR_MG_0_1)
#define CNM1_DP_FOR_MG_0_2   (16 + CNM0_DP_FOR_MG_0_2)
#define CNM1_DP_FOR_MG_0_3   (16 + CNM0_DP_FOR_MG_0_3)

#define CNM1_DP_FOR_MG_1_0   (16 + CNM0_DP_FOR_MG_1_0)
#define CNM1_DP_FOR_MG_1_1   (16 + CNM0_DP_FOR_MG_1_1)
#define CNM1_DP_FOR_MG_1_2   (16 + CNM0_DP_FOR_MG_1_2)
#define CNM1_DP_FOR_MG_1_3   (16 + CNM0_DP_FOR_MG_1_3)



#define FALCON_3_2_PRIMARY_CPU_SDMA    64+4 /*after 64 50G MACs + 4 'cpu ports' 10G macs */
/* DMA : special ports mapping {global,local,DP}    */
static SPECIAL_PORT_MAPPING_CNS falcon_3_2_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_3_2_PRIMARY_CPU_SDMA+0/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_0/*DP[4]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_1/*DP[5]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_2/*DP[6]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_3/*DP[7]*/}

    ,{FALCON_3_2_PRIMARY_CPU_SDMA-4/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,0/*DP[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,2/*DP[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,4/*DP[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,6/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* GOP : special ports mapping {global,local,pipe}*/
static SPECIAL_PORT_MAPPING_CNS falcon_3_2_GOP_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_3_2_PRIMARY_CPU_SDMA-4/*global GOP port*/,32/*local GOP port*/,0/*pipe*/} /*DP[0] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-3/*global GOP port*/,33/*local GOP port*/,0/*pipe*/} /*DP[2] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-2/*global GOP port*/,32/*local GOP port*/,1/*pipe*/} /*DP[4] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-1/*global GOP port*/,33/*local GOP port*/,1/*pipe*/} /*DP[6] port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

#define FALCON_6_4_PRIMARY_CPU_SDMA    (128+8)
/* DMA : special ports mapping {global,local,DP}*/
static SPECIAL_PORT_MAPPING_CNS falcon_6_4_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_6_4_PRIMARY_CPU_SDMA+0/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_0/*DP[ 4]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_1/*DP[ 5]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_2/*DP[ 6]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_0_3/*DP[ 7]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+4/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_1_3/*DP[ 8]*/}/* note : using MG 7 */
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+5/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_1_2/*DP[ 9]*/}/* note : using MG 6 */
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+6/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_1_1/*DP[10]*/}/* note : using MG 5 */
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+7/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM0_DP_FOR_MG_1_0/*DP[11]*/}/* note : using MG 4 */

    ,{FALCON_6_4_PRIMARY_CPU_SDMA-8/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/, 0/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-7/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/, 2/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-6/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/, 4/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-5/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/, 6/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-4/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/, 8/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,10/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,12/*DP[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,14/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* GOP : special ports mapping {global,local,pipe}*/
static SPECIAL_PORT_MAPPING_CNS falcon_6_4_GOP_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_6_4_PRIMARY_CPU_SDMA-8/*global GOP port*/,32/*local GOP port*/,0/*pipe*/} /*DP[0] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-7/*global GOP port*/,33/*local GOP port*/,0/*pipe*/} /*DP[2] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-6/*global GOP port*/,32/*local GOP port*/,1/*pipe*/} /*DP[4] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-5/*global GOP port*/,33/*local GOP port*/,1/*pipe*/} /*DP[6] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-4/*global GOP port*/,32/*local GOP port*/,2/*pipe*/} /*DP[8] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-3/*global GOP port*/,33/*local GOP port*/,2/*pipe*/} /*DP[10]port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-2/*global GOP port*/,32/*local GOP port*/,3/*pipe*/} /*DP[12]port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-1/*global GOP port*/,33/*local GOP port*/,3/*pipe*/} /*DP[14]port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

#define FALCON_12_8_PRIMARY_CPU_SDMA    (256+16)
/* DMA : special ports mapping {global,local,DP}*/
static SPECIAL_PORT_MAPPING_CNS falcon_12_8_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_12_8_PRIMARY_CPU_SDMA+0/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_0_0/*DP[ 4]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_0_1/*DP[ 5]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_0_2/*DP[ 6]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_0_3/*DP[ 7]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+4/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_1_3/*DP[ 8]*/}/* note : using MG 7 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+5/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_1_2/*DP[ 9]*/}/* note : using MG 6 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+6/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_1_1/*DP[10]*/}/* note : using MG 5 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+7/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM0_DP_FOR_MG_1_0/*DP[11]*/}/* note : using MG 4 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+8/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM1_DP_FOR_MG_0_0/*DP[20]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+9/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,CNM1_DP_FOR_MG_0_1/*DP[21]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+10/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_0_2/*DP[22]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+11/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_0_3/*DP[23]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+12/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_1_3/*DP[24]*/}/* note : using MG 15 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+13/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_1_2/*DP[25]*/}/* note : using MG 16 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+14/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_1_1/*DP[26]*/}/* note : using MG 17 */
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+15/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/,CNM1_DP_FOR_MG_1_0/*DP[27]*/}/* note : using MG 18 */


    ,{FALCON_12_8_PRIMARY_CPU_SDMA-16/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ , 0/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-15/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ , 2/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-14/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ , 4/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-13/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ , 6/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-12/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ , 8/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-11/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,10/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-10/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,12/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 9/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,14/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 8/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,16/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 7/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,18/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 6/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,20/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 5/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,22/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 4/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,24/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 3/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,26/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 2/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,28/*DP[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 1/*global DMA port*/,FALCON_PORTS_PER_DP/*local DMA port*/ ,30/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* GOP : special ports mapping {global,local,pipe}*/
static SPECIAL_PORT_MAPPING_CNS falcon_12_8_GOP_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_12_8_PRIMARY_CPU_SDMA-16/*global GOP port*/,32/*local GOP port*/,0/*pipe*/} /*DP[0] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-15/*global GOP port*/,33/*local GOP port*/,0/*pipe*/} /*DP[2] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-14/*global GOP port*/,32/*local GOP port*/,1/*pipe*/} /*DP[4] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-13/*global GOP port*/,33/*local GOP port*/,1/*pipe*/} /*DP[6] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-12/*global GOP port*/,32/*local GOP port*/,2/*pipe*/} /*DP[8] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-11/*global GOP port*/,33/*local GOP port*/,2/*pipe*/} /*DP[10] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-10/*global GOP port*/,32/*local GOP port*/,3/*pipe*/} /*DP[12] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 9/*global GOP port*/,33/*local GOP port*/,3/*pipe*/} /*DP[14] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 8/*global GOP port*/,32/*local GOP port*/,4/*pipe*/} /*DP[16] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 7/*global GOP port*/,33/*local GOP port*/,4/*pipe*/} /*DP[18] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 6/*global GOP port*/,32/*local GOP port*/,5/*pipe*/} /*DP[20] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 5/*global GOP port*/,33/*local GOP port*/,5/*pipe*/} /*DP[22] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 4/*global GOP port*/,32/*local GOP port*/,6/*pipe*/} /*DP[24] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 3/*global GOP port*/,33/*local GOP port*/,6/*pipe*/} /*DP[26] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 2/*global GOP port*/,32/*local GOP port*/,7/*pipe*/} /*DP[28] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 1/*global GOP port*/,33/*local GOP port*/,7/*pipe*/} /*DP[30] port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

#define RAVEN_NUM_OF_MACS  17
#define RAVEN_CPU_PORT_NUM 16
/* Falcon 3.2 : Ravens 1 hold the CPU port */
static SPECIAL_PORT_MAPPING_ENHANCED_CNS falcon_3_2_Ravens_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT -- local index in channel instance -- channel instance number */
     {FALCON_3_2_PRIMARY_CPU_SDMA-4/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,0/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)}/*DP[2] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-3/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,1/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)}/*DP[4] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-2/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,2/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)}/*DP[2] port 9*/
    ,{FALCON_3_2_PRIMARY_CPU_SDMA-1/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,3/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)}/*DP[4] port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};
/* Falcon 6.4 : Ravens 3,4 hold the CPU ports */
static SPECIAL_PORT_MAPPING_ENHANCED_CNS falcon_6_4_Ravens_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT -- local index in channel instance -- channel instance number */
     {FALCON_6_4_PRIMARY_CPU_SDMA-8/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,0/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[0] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-7/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,1/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[2] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-6/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,2/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[4] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-5/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,3/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[6] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-4/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,4/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[8] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-3/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,5/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[10] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-2/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,6/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[12] port 9*/
    ,{FALCON_6_4_PRIMARY_CPU_SDMA-1/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,7/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[14] port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};
/* Falcon 12.8 : Ravens 1,2 hold the CPU ports */
static SPECIAL_PORT_MAPPING_ENHANCED_CNS falcon_12_8_Ravens_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT -- local index in channel instance -- channel instance number */
     {FALCON_12_8_PRIMARY_CPU_SDMA-16/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,0/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[0] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-15/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,1/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[2] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-14/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,2/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[4] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-13/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,3/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[6] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-12/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,4/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[8] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-11/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,5/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[10] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA-10/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,6/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[12] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 9/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,7/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[14] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 8/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,8/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[16] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 7/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,9/*Raven*/ ,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[18] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 6/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,10/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[20] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 5/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,11/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[22] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 4/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,12/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[24] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 3/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,13/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[26] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 2/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,14/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[28] port 9*/
    ,{FALCON_12_8_PRIMARY_CPU_SDMA- 1/*global MAC port*/,RAVEN_CPU_PORT_NUM/*local MAC port*/,15/*Raven*/,PORT_MAPPING_ENHANCED_TWO_PARAMS_MAC(RAVEN_CPU_PORT_NUM/2,0)} /*DP[30] port 9*/

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};


/* multi-CPU SDMA MG : special ports mapping {global,NA,mgUnitIndex}    */
static SPECIAL_PORT_MAPPING_CNS falcon_3_2_cpuPortSdma_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --NA;OUT       --mgUnitIndex;OUT */
     {FALCON_3_2_PRIMARY_CPU_SDMA+0/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,0/*mgUnitIndex[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+1/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,1/*mgUnitIndex[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+2/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,2/*mgUnitIndex[]*/}
    ,{FALCON_3_2_PRIMARY_CPU_SDMA+3/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,3/*mgUnitIndex[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* multi-CPU SDMA MG : special ports mapping {global,NA,mgUnitIndex}    */
static SPECIAL_PORT_MAPPING_CNS falcon_6_4_cpuPortSdma_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_6_4_PRIMARY_CPU_SDMA+0/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,0/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+1/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,1/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+2/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,2/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+3/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,3/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+4/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,7/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+5/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,6/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+6/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,5/*mgUnitIndex[]*/}
    ,{FALCON_6_4_PRIMARY_CPU_SDMA+7/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,4/*mgUnitIndex[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* multi-CPU SDMA MG : special ports mapping {global,NA,mgUnitIndex}    */
static SPECIAL_PORT_MAPPING_CNS falcon_12_8_cpuPortSdma_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {FALCON_12_8_PRIMARY_CPU_SDMA+0/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,0/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+1/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,1/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+2/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,2/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+3/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,3/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+4/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,7/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+5/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,6/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+6/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,5/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+7/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,4/*mgUnitIndex[]*/}

    ,{FALCON_12_8_PRIMARY_CPU_SDMA+8/*global DMA port*/ ,SMAIN_NOT_VALID_CNS/*NA*/, 8/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+9/*global DMA port*/ ,SMAIN_NOT_VALID_CNS/*NA*/, 9/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+10/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,10/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+11/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,11/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+12/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,15/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+13/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,14/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+14/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,13/*mgUnitIndex[]*/}
    ,{FALCON_12_8_PRIMARY_CPU_SDMA+15/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,12/*mgUnitIndex[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};


typedef enum{
     SHERED_MEM_TYPE_NOT_VALID         = 0
    ,SHERED_MEM_TYPE_NO_SHARE_E              /* the memory is not shared pipe 1 hold it's memories regardless to pipe 0 */
    ,SHERED_MEM_TYPE_PER_TILE_PIPE_0_ONLY_E /*the memory as per tile and exists only in LOCAL pipe 0 of tile (global pipes : 0,3,4,7) */
}SHERED_MEM_TYPE_ENT;

typedef enum {
    AAC_ENGINE_DATA_CHANNEL_0_E,
    AAC_ENGINE_DATA_CHANNEL_1_E,
    AAC_ENGINE_DATA_CHANNEL_2_E,
    AAC_ENGINE_DATA_CHANNEL_3_E,
    AAC_ENGINE_DATA_CHANNEL_4_E,
    AAC_ENGINE_DATA_CHANNEL_5_E,
    AAC_ENGINE_DATA_CHANNEL_6_E,
    AAC_ENGINE_DATA_CHANNEL_7_E,
    AAC_ENGINE_ADDRESS_CHANNEL_0_E,
    AAC_ENGINE_ADDRESS_CHANNEL_1_E,
    AAC_ENGINE_ADDRESS_CHANNEL_2_E,
    AAC_ENGINE_ADDRESS_CHANNEL_3_E,
    AAC_ENGINE_ADDRESS_CHANNEL_4_E,
    AAC_ENGINE_ADDRESS_CHANNEL_5_E,
    AAC_ENGINE_ADDRESS_CHANNEL_6_E,
    AAC_ENGINE_ADDRESS_CHANNEL_7_E
}AAC_ENGINE_REGISTER_TYPE_ENT;

typedef struct{
    GT_BIT        tableOffsetValid;  /* use MACRO SMEM_BIND_TABLE_MAC */
    GT_U32        tableOffsetInBytes;/* use MACRO SMEM_BIND_TABLE_MAC */
    GT_BOOL       forbidDirectWriteToChunk;/* use MACRO SMEM_BIND_TABLE_MAC */
    GT_U32        startAddr;/* init in run time : absolute start address of table (with 8 MSBits) */
    GT_U32        lastAddr; /* init in run time : absolute last  address of table (with 8 MSBits) */
    SHERED_MEM_TYPE_ENT table_unitType_deviceAccess;/* for skernel access   : indication that the table is pipe0/1/shared*/
    SHERED_MEM_TYPE_ENT table_unitType_cpu_access;  /* for SCIB(CPU) access : indication that the table is pipe0/1/shared*/
}SPECIAL_INSTANCE_TABLES_ARR_STC;

/* define memory that access the same one when CPU/PP use pipe 0 or pipe 1 */
#define TABLE_SHARED_2_PIPES_MAC(tableName) \
    /*tableOffsetValid,tableOffsetInBytes*/  /*startAddr*/ /*lastAddr*/    /*table_unitType_deviceAccess*/          /*table_unitType_cpu_access*/ \
    /*forbidDirectWriteToChunk*/                                                                                                                  \
    SMEM_BIND_TABLE_MAC(tableName),                 0           ,0          ,SHERED_MEM_TYPE_PER_TILE_PIPE_0_ONLY_E ,SHERED_MEM_TYPE_PER_TILE_PIPE_0_ONLY_E

/* address belongs to 'Special tables' that number of instances
   different than the number on instances of the UNIT ! */
static SPECIAL_INSTANCE_TABLES_ARR_STC specialSingleInstanceTablesArr[] =
{ /*tableOffsetValid,tableOffsetInBytes*/                           /*startAddr*/ /*lastAddr*/    /*table_unitType_deviceAccess*/  /*table_unitType_cpu_access*/
    /* HA : shared tables between 2 pipes */
     {TABLE_SHARED_2_PIPES_MAC(arp)}
    /* LPM : shared tables between 2 pipes */
    ,{TABLE_SHARED_2_PIPES_MAC(lpmMemory)}
    ,{TABLE_SHARED_2_PIPES_MAC(lpmAgingMemory)}
    /* must be last  */
    ,{0,0                                           ,0           ,0          ,0                             ,0}
};

typedef enum {
     MTI_ACTIVE_MEM_REG_PORT_MTI_64_STATUS_E                /* formula for 50/100G MTI status registers */
    ,MTI_ACTIVE_MEM_REG_PORT_MTI_400_STATUS_E               /* formula for 200/400G MTI status registers */
    ,MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_MASK_E        /* formula for 50/100G MTI interrupt mask registers */
    ,MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_MASK_E       /* formula for 200/400G MTI interrupt mask registers */
    ,MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_CAUSE_E       /* formula for 50/100G MTI interrupt cause registers */
    ,MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_CAUSE_E      /* formula for 200/400G MTI interrupt cause registers */
    ,MTI_ACTIVE_MEM_REG_MAC_CDMMAND_CFG_MTI_64_E            /* formula for 50/100G MTI command config registers */
    ,MTI_ACTIVE_MEM_REG_MAC_COMMAND_CFG_MTI_400_E           /* formula for 200/400G MTI command config registers */
    ,MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E                      /* formula for MTI PCS control1 registers */
    ,MTI_ACTIVE_MEM_REG_STAT_CONTROL_E                      /* formula for MTI statistic control */
    ,MTI_ACTIVE_MEM_REG_STAT_CAPTURE_E                      /* formula for MTI statistic capture memory */
    /* FEC interrupt registers */
    ,MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_MASK_E
    ,MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_MASK_E
    ,MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_CAUSE_E
    ,MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_CAUSE_E

}MTI_ACTIVE_MEM_REG_ENT;

enum{
     MTI_PCS_50_E  = 50/*0x00465000*/
    ,MTI_PCS_100_E = 100/*0x00464000*/
    ,MTI_PCS_400_E = 400/*0x00460000*/
    ,MTI_PCS_CPU_E = 1/*0x00519000*/
};


/* Statistic Counter: Counter Base Address + Word Number + Name */
#define PORT_MTI_STATISTIC_INFO_ENTRY(counter) ((GT_U32) ( (GT_UINTPTR) (&((SMEM_SIP5_PP_REGS_ADDR_STC *)0)->sip6_MTI[0].MTI_STATS.counter) )), 1, STR(counter)
#define PORT_MTI_STATISTIC_INFO_DUMMY          0, 1, "dummy"

static MIB_COUNTER_INFO_STC portMtiStatisticCountersInfoArr[SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E] =
{
    /* RX counters */
     {PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsOctets)                }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_OctetsReceivedOK)                }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aAlignmentErrors)                }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aPauseMacCtrlFramesReceived)     }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aFrameTooLong)                   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aInRangeLengthError)             }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aFramesReceivedOK)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aFrameCheckSequenceErrors)       }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_VLANReceivedOK)                  }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_ifInErrors)                      }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_ifInUcastPkts)                   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_ifInMulticastPkts)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_ifInBroadcastPkts)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsDropEvents)            }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts)                  }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsUndersizePkts)         }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts64Octets)          }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts65to127Octets)     }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts128to255Octets)    }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts256to511Octets)    }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts512to1023Octets)   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts1024to1518Octets)  }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsPkts1519toMaxOctets)   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsOversizePkts)          }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsJabbers)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_etherStatsFragments)             }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aCBFCPAUSEFramesReceived)        }
    /* dummy 15 entries ... for 'alignment' */
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(RX_aCBFCPAUSEFramesReceived)         }
    /* TX counters */
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsOctets)                 }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_OctetsTransmittedOK)              }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_aPauseMacCtrlFramesTransmitted)   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_aFramesTransmittedOK)             }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_VLANTransmittedOK)                }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_ifOutErrors)                      }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_ifOutUcastPkts)                   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_ifOutMulticastPkts)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_ifOutBroadcastPkts)               }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts64Octets)           }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts65to127Octets)      }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts128to255Octets)     }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts256to511Octets)     }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts512to1023Octets)    }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts1024to1518Octets)   }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts1519toMaxOctets)    }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_aCBFCPAUSEFramesTransmitted)      }
    /* dummy 15 entries ... for 'alignment' */
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_DUMMY}
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_aMACControlFramesTransmitted)     }
    ,{PORT_MTI_STATISTIC_INFO_ENTRY(TX_etherStatsPkts)                   }
};

/**
* @internal smemFalconGetMgUnitIndexFromAddress function
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
static GT_U32  smemFalconGetMgUnitIndexFromAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  regAddress
)
{
    GT_U32  tmpRegAddr = regAddress % TILE_OFFSET;
    GT_U32  mgIndex;
    GT_U32  dual_tileId;/* 0 or 1 */

    if(tmpRegAddr >= MG_0_0_OFFSET_CNS &&
       tmpRegAddr < (MG_1_0_OFFSET_CNS + (MG_SIZE*FALCON_NUM_MG_PER_TILE)))
    {
        dual_tileId  = regAddress/(2*TILE_OFFSET);/*per 2 tiles*/
        mgIndex  = (tmpRegAddr - MG_0_0_OFFSET_CNS)/MG_SIZE;
        mgIndex += dual_tileId * (2 * FALCON_NUM_MG_PER_TILE);

        return mgIndex;
    }

    return SMAIN_NOT_VALID_CNS;
}


/**
* @internal internalFalconUnitPipeOffsetGet function
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
static GT_U32 internalFalconUnitPipeOffsetGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   regAddress,
    OUT GT_U32                  *pipeIdOfAddressIndexPtr,
    INOUT GT_U32                *currentPipeIdPtr,
    IN GT_BOOL                  needPipeOffsetFromPipe0
)
{
    GT_U32  tmpTileId;
    GT_U32  tileBaseAddr = 0;
    GT_U32  pipeOffset;
    GT_U32  onlyCheckAddr = (pipeIdOfAddressIndexPtr && ((*pipeIdOfAddressIndexPtr) == SMAIN_NOT_VALID_CNS)) ? 1 : 0;
    GT_U32  currentPipeId;
    GT_U32  currentTileId = 0;
    GT_U32  localPipeId = 0;/* local pipeId in tile */
    GT_U32  finalOffset = 0;/* final offset that need to return to the caller */
    GT_U32  globalPipeId;/* */
    GT_U32  isPerTile = 0;
    GT_U32  isPer2Tiles = 0;
    GT_U32  addrInTile0 = 0;

    if(pipeIdOfAddressIndexPtr)
    {
        *pipeIdOfAddressIndexPtr = 0;
    }

    if (regAddress == SMAIN_NOT_VALID_CNS)
    {
        /* support NOT valid address if expected */
        return 0;
    }

    currentPipeId = smemGetCurrentPipeId(devObjPtr);
    if(currentPipeIdPtr)
    {
        *currentPipeIdPtr = currentPipeId;
    }

    if(regAddress < TILE_OFFSET)
    {
        addrInTile0 = 1;

        /**********************************************************************/
        /* the address maybe in 'tile 0' but we need to convert to other tile */
        /**********************************************************************/
        currentTileId = currentPipeId / devObjPtr->numOfPipesPerTile;

        if(regAddress >= CNM_OFFSET_CNS)
        {
            /* MnG unit per 2 tile */
            tileBaseAddr += (2 * TILE_OFFSET) * (currentTileId / 2);

            if(regAddress >= MG_0_0_OFFSET_CNS &&
               regAddress <  MG_0_1_OFFSET_CNS)
            {
                GT_U32 mgTileId;
                GT_U32 mgIndexInTile;
                GT_U32 mgUnitIndex;

                mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);
                mgTileId      = mgUnitIndex / FALCON_NUM_MG_PER_TILE;
                mgIndexInTile = mgUnitIndex % FALCON_NUM_MG_PER_TILE;

#if 0 /*NOTE: the MnG unit not subject to mirroring because it is single in the 2 units */
                if((1<<mgTileId) & devObjPtr->mirroredTilesBmp)
                {
                    /* treat the mirroring MG units */
                    mgIndexInTile = (FALCON_NUM_MG_PER_TILE - 1) - mgIndexInTile;
                }
#endif

                /* jump from MG_0_0 to needed MG_0_1/MG_0_2/MG_0_3 */
                tileBaseAddr = mgIndexInTile * MG_SIZE;

                /* the caller need to be moved into next 4 MG units */
                if(mgTileId & 1)
                {
                    /* jump from MG_0_x to needed MG_1_x */
                    /* address of 'tile 0' but we need address of 'tile 1' */
                    tileBaseAddr += (MG_1_0_OFFSET_CNS - MG_0_0_OFFSET_CNS);
                }

                if(mgTileId & 2)
                {
                    /* jump to second 'dual tile' */
                    tileBaseAddr += (2 * TILE_OFFSET);
                }

                if(pipeIdOfAddressIndexPtr)
                {
                    *pipeIdOfAddressIndexPtr = 0; /* make sure that caller update the address with the needed offset */
                }
                if(currentPipeIdPtr)
                {
                    *currentPipeIdPtr = 1;/* make sure that caller update the address with the needed offset */
                }

                return tileBaseAddr;
            }
            else
            if (regAddress >= MG_0_1_OFFSET_CNS &&
                regAddress < (MG_1_0_OFFSET_CNS + (MG_SIZE*FALCON_NUM_MG_PER_TILE)))
            {
                /* MG_0_1 to MG_0_3 serve tile 0 -- accessed with MG 'aware' address  */
                /* MG_1_0 to MG_1_3 serve tile 1  */
                /* explicit call to needed offset ... nothing more to do */
                if(pipeIdOfAddressIndexPtr)
                {
                    *pipeIdOfAddressIndexPtr = 0; /* make sure that caller update the address with the needed offset */
                }
                if(currentPipeIdPtr)
                {
                    *currentPipeIdPtr = 1;/* make sure that caller update the address with the needed offset */
                }

                return 0;
            }
        }
        else
        {
            /* unit per tile */
            tileBaseAddr += TILE_OFFSET* currentTileId;
        }
    }

    tmpTileId = regAddress / TILE_OFFSET;
    currentTileId += tmpTileId;
    tileBaseAddr += TILE_OFFSET* tmpTileId;
    regAddress %= TILE_OFFSET;

    if(regAddress >= PIPE_0_START_ADDR && regAddress <= PIPE_0_END_ADDR)
    {
        /* the address of pipe 0 hold 'clone' in pipe 1 */
        pipeOffset  = PIPE_1_FROM_PIPE_0_OFFSET ;
    }
    else
    if(regAddress >= PIPE_1_START_ADDR && regAddress <= PIPE_1_END_ADDR)
    {
        localPipeId = 1;
        /* the address already in pipe 1 ... */
        pipeOffset  = PIPE_1_FROM_PIPE_0_OFFSET ;
    }
    else
    /* Raven memory ranges */
    if (regAddress <= RAVEN_3_END_ADDR)
    {
        pipeOffset = RAVEN_PIPE_OFFSET;
        if (regAddress >= RAVEN_2_START_ADDR)
        {
            localPipeId = 1;
        }
    }
    else
    /*if(regAddress < TILE_OFFSET)*/
    {
        pipeOffset  = 0;

        if(regAddress >= CNM_OFFSET_CNS)
        {
            /* unit per 2 tile */
            isPer2Tiles = 1;
        }
        else
        {
            /* unit per tile : shared memories that are not per pipe */
            isPerTile =  1;
        }
    }

    if(currentTileId < devObjPtr->numOfTiles)
    {
        /* the address was found in the memory ranges */
        if(pipeIdOfAddressIndexPtr)
        {
            if(!addrInTile0)
            {
                if(isPerTile || isPer2Tiles)
                {
                    if(isPer2Tiles &&
                       regAddress >= MG_1_0_OFFSET_CNS &&
                       regAddress <  MG_1_0_OFFSET_CNS + (3*MG_SIZE))
                    {
                        currentTileId++;
                    }

                    /* convert the local pipeId in the tile to global pipeId */
                    /* we ignore 'local pipeId' .. considered as 0 in the calculation but we can't call
                    smemConvertTileAndLocalPipeIdInTileToGlobalPipeId(...) because
                    local PipeId = 0 in Tile 1 considered as global pipe 3 in the device
                    */
                    globalPipeId = (currentTileId * devObjPtr->numOfPipesPerTile);
                }
                else /* per pipe */
                {
                    /* convert the local pipeId in the tile to global pipeId */
                    globalPipeId = smemConvertTileAndLocalPipeIdInTileToGlobalPipeId(
                        devObjPtr,currentTileId/*tileId*/,localPipeId);
                }

                (*pipeIdOfAddressIndexPtr) = globalPipeId;
            }
            else /* address in tile 0 */
            {
                (*pipeIdOfAddressIndexPtr) = localPipeId;
            }
        }

        if(needPipeOffsetFromPipe0 == GT_TRUE)
        {
            /*if((1<<currentTileId) & devObjPtr->mirroredTilesBmp)*/
            if(currentTileId >= 1)
            {
                if(localPipeId == 0)
                {
                    pipeOffset = 0;
                }
            }
        }
        else
        if((1<<currentTileId) & devObjPtr->mirroredTilesBmp)
        {
            /* mirrored Tile */

            if(localPipeId == 0 && (currentPipeId & 1))
            {
                /* the address NOT hold the pipe offset , and target pipe not need it */
                pipeOffset = 0;
            }
            else
            if(localPipeId == 1 && (0 == (currentPipeId & 1)))
            {
                /* the address already hold the pipe offset , and also needed at the target */
                pipeOffset = 0;
            }
        }
        else
        if(currentTileId >= 2)/* non mirrored tile */
        {
            if(localPipeId == 0 && (0 == (currentPipeId & 1)))
            {
                /* the address NOT hold the pipe offset , and target pipe not need it */
                pipeOffset = 0;
            }
            else
            if(localPipeId == 1 && (1 == (currentPipeId & 1)))
            {
                /* the address already hold the pipe offset , and also needed at the target */
                pipeOffset = 0;
            }
        }
        else
        {
            if(localPipeId == 1)
            {
                /* the address already hold the pipe offset */
                pipeOffset = 0;
            }
        }


        finalOffset = pipeOffset + tileBaseAddr;

        return finalOffset;
    }

    if(onlyCheckAddr)
    {
        /* indication that the caller only check that the unit exists. */
        /* and the check 'failed' without fatal error */
        *pipeIdOfAddressIndexPtr = SMAIN_NOT_VALID_CNS;
    }
    else
    {
        skernelFatalError("internalFalconUnitPipeOffsetGet : address[0x%8.8x]not found \n",
            regAddress + tileBaseAddr);
    }

    return 0;
}
/**
* @internal smemFalconUnitPipeOffsetGet function
* @endinternal
*
* @brief   Get pipe offset for unit register address in data base.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] regAddress               - unit register lookup addres
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe index that hold the 'regAddress'
*                                      ignored if NULL
*
* @retval Pipe offset !== 0        - pipe1 offset
* @retval == 0                     - pipe1 offset not found (should not happen)
*/
static GT_U32 smemFalconUnitPipeOffsetGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   regAddress,
    IN GT_U32                   *pipeIndexPtr
)
{
    return internalFalconUnitPipeOffsetGet(devObjPtr, regAddress,
        pipeIndexPtr,NULL/*currentPipeIdPtr*/,GT_TRUE);
}


static void addDuplicatedUnitIfNotExists
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_CHAR*         origUnitNamePtr,
    IN GT_CHAR*         dupUnitNamePtr
)
{
    GT_U32  ii,jj;
    SMEM_UNIT_DUPLICATION_INFO_STC   *unitInfoPtr_1 = &falcon_duplicatedUnits[0];
    SMEM_UNIT_DUPLICATION_INFO_STC   *unitInfoPtr_2;
    GT_U32  found_1,found_2;


    for(ii = 0 ; unitInfoPtr_1->unitNameStr != NULL; ii++)
    {
        if(0 == strcmp(origUnitNamePtr,unitInfoPtr_1->unitNameStr))
        {
            found_1 = 1;
        }
        else
        {
            found_1 = 0;
        }

        found_2 = 0;

        unitInfoPtr_2 = unitInfoPtr_1+1;
        for(jj = 0 ; jj < unitInfoPtr_1->numOfUnits ; jj++,unitInfoPtr_2++)
        {
            if(found_1 == 0)
            {
                if(0 == strcmp(origUnitNamePtr,unitInfoPtr_2->unitNameStr))
                {
                    found_1 = 1;
                }
            }

            if(found_2 == 0)
            {
                if(0 == strcmp(dupUnitNamePtr,unitInfoPtr_2->unitNameStr))
                {
                    found_2 = 1;
                }
            }

            if(found_1 && found_2)
            {
                return;
            }
        }

        unitInfoPtr_1 = unitInfoPtr_2;
        ii += jj;
    }

    /* we need to add 2 entries to the DB (and 'NULL' entry) */
    if((ii+2) >= (sizeof(falcon_duplicatedUnits)/sizeof(falcon_duplicatedUnits[0])))
    {
        skernelFatalError("addDuplicatedUnitIfNotExists : array falcon_duplicatedUnits[] not large enough need at least ([%d] cells)\n",
            (ii+2+1));
        return;
    }

    falcon_duplicatedUnits[ii].unitNameStr = origUnitNamePtr;
    falcon_duplicatedUnits[ii].numOfUnits = 1;

    falcon_duplicatedUnits[ii+1].unitNameStr = dupUnitNamePtr;
    falcon_duplicatedUnits[ii+1].numOfUnits = 0;

    falcon_duplicatedUnits[ii+2].unitNameStr = NULL;
    falcon_duplicatedUnits[ii+2].numOfUnits = 0;

    return ;
}

/* there are no more than 70 units , in pipe 0  units */
/* there are no more than 50 units , in central units */
#define FALCON_NUM_UNITS_PIPE     70
#define FALCON_FACTOR_ON_PIPE     8
#define FALCON_NUM_UNITS_CENTRAL  50
#define FALCON_FACTOR_ON_CENTRAL  4
#define FALCON_NUM_UNITS (FALCON_NUM_UNITS_PIPE*FALCON_FACTOR_ON_PIPE + FALCON_NUM_UNITS_CENTRAL*FALCON_FACTOR_ON_CENTRAL)
/* use 8 times the 'FALCON_NUM_UNITS_PIPE_0_E' because:
   8 times to support duplicated units from pipe 0
*/
static SMEM_UNIT_NAME_AND_INDEX_STC falconUnitNameAndIndexArr[FALCON_NUM_UNITS]=
{
    /* filled in runtime from falcon_units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};
/* the addresses of the units that the bobk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   falconUsedUnitsAddressesArray[FALCON_NUM_UNITS]=
{
    {0,0}    /* filled in runtime from falcon_units[] */
};

/**
* @internal sim_sip6_units_cellCompare function
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
int  sim_sip6_units_cellCompare
(
    const GT_VOID * cell1Ptr,
    const GT_VOID * cell2Ptr
)
{
    SMEM_GEN_UNIT_INFO_STC * cellFirstPtr = (SMEM_GEN_UNIT_INFO_STC *)cell1Ptr;
    SMEM_GEN_UNIT_INFO_STC * cellNextPtr  = (SMEM_GEN_UNIT_INFO_STC *)cell2Ptr;

    return (cellFirstPtr->base_addr  > cellNextPtr->base_addr) ? 1 :
           (cellFirstPtr->base_addr == cellNextPtr->base_addr) ? 0 : -1;
}

/* build once the sorted memory for the falcon .. for better memory search performance

    that use by :

    devObjPtr->devMemUnitNameAndIndexPtr      = falconUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = falconUsedUnitsAddressesArray;


*/
static void build_SORTED___falcon_units(void)
{
    GT_U32  numValidElem,ii;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &falcon_units[0];

    if(SORTED___falcon_units[0].base_addr != SMAIN_NOT_VALID_CNS)
    {
        /* already initialized */
        return;
    }

    memcpy(SORTED___falcon_units,falcon_units,sizeof(falcon_units));

    numValidElem = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        numValidElem++;
    }

    qsort(SORTED___falcon_units, numValidElem, sizeof(SMEM_GEN_UNIT_INFO_STC),
          sim_sip6_units_cellCompare);

}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        falcon_duplicatedUnits - the array of duplicated units
        falconUsedUnitsAddressesArray - the addresses of the units that the Falcon uses
        falconUnitNameAndIndexArr - name of unit and index in falconUsedUnitsAddressesArray */
    GT_U32  ii;
    GT_U32  index;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &SORTED___falcon_units[0];

    /* build once the sorted memory for the falcon .. for better memory search performance */
    build_SORTED___falcon_units();

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        if(unitInfoPtr->orig_nameStr == PER_2_TILES_INDICATION_CNS)
        {
            /* the instance in Tile 0 (the next instance come next)*/
        }
        else
        if(unitInfoPtr->orig_nameStr == PER_TILE_INDICATION_CNS)
        {
            /* the instance in Tile 0 (the next instances come next)*/
        }
        else
        if(unitInfoPtr->orig_nameStr)
        {
            addDuplicatedUnitIfNotExists(devObjPtr, unitInfoPtr->orig_nameStr, unitInfoPtr->nameStr);
        }

        if(index >= (sizeof(falconUsedUnitsAddressesArray) / sizeof(falconUsedUnitsAddressesArray[0])))
        {
            skernelFatalError("buildDevUnitAddr : over flow of units (1) \n");
        }

        if(index >= (sizeof(falconUnitNameAndIndexArr) / sizeof(falconUnitNameAndIndexArr[0])))
        {
            skernelFatalError("buildDevUnitAddr : over flow of units (2) \n");
        }

        falconUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
        falconUsedUnitsAddressesArray[index].unitSizeInBytes = unitInfoPtr->size;
        falconUnitNameAndIndexArr[index].unitNameIndex = index;
        falconUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        index++;
    }

    if(index >= (sizeof(falconUnitNameAndIndexArr) / sizeof(falconUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    falconUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    falconUnitNameAndIndexArr[index].unitNameStr = NULL;
    falconUsedUnitsAddressesArray[index].unitBaseAddr = SMAIN_NOT_VALID_CNS;
    falconUsedUnitsAddressesArray[index].unitSizeInBytes = 0;

    devObjPtr->devMemUnitNameAndIndexPtr = falconUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = falconUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = smemFalconUnitPipeOffsetGet;/*SMEM_UNIT_PIPE_OFFSET_GET_MAC*/
    devObjPtr->support_memoryRanges = 1;

#if 0 /* check that the array is ascending ! (falconUsedUnitsAddressesArray) */
    for(ii = 0 ; ii < (index+1) ; ii++)
    {
        printf("unitBaseAddr = [0x%8.8x] \n",
            falconUsedUnitsAddressesArray[ii].unitBaseAddr);

        if(ii &&
            (falconUsedUnitsAddressesArray[ii].unitBaseAddr <=
             falconUsedUnitsAddressesArray[ii-1].unitBaseAddr))
        {
            printf("Error: at index[%d] prev index higher \n",ii);
            break;
        }
    }
#endif /*0*/
}
/**
* @internal smemFalconUnitTxqPdxPac function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pdx pac unit
*/
static void smemFalconUnitTxqPdxPac
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000008C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000010C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x0000012C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x0000015C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x0000105C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001180)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001380)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001580)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x00001780)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F04, 0x00001F04)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F10, 0x00001F18)}
    };

         GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
     smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00001050,         0x80000009,      4,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x00001104,         0x00000001,      4,    0x200   }
            ,{DUMMY_NAME_PTR_CNS,           0x00001108,         0x00000002,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x0000110c,         0x00000003,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001110,         0x00000004,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001114,         0x00000005,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001118,         0x00000006,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x0000111c,         0x00000007,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001120,         0x00000008,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001124,         0x00000009,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001128,         0x0000000a,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x0000112c,         0x0000000b,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001130,         0x0000000c,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001134,         0x0000000d,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001138,         0x0000000e,      4,    0x200    }
            ,{DUMMY_NAME_PTR_CNS,           0x00001f04,         0x0000ffff,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }




}


/**
* @internal smemFalconUnitTxqPdx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pdx unit
*/
static void smemFalconUnitTxqPdx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(txqPdxQueueGroupMap)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 4096)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 4096)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000, 4096)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000, 0x00010000)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010008, 0x0001000C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00018000, 0x00018018)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00018030, 0x00018030)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00018040, 0x0001807C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001F004, 0x0001F004)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001F010, 0x0001F018)}
    };




    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00018004,         0x0000ffff,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,           0x00018008,         0x000000c0,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,           0x0001f004,         0x0000ffff,      1,    0x0  }
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconUnitTxqPfcc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pfcc unit
*/
static void smemFalconUnitTxqPfcc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{       /*sync to /Cider/EBU-IP/TXQ_IP/SIP7.0 (Falcon)/TXQ_PR_IP/TXQ_PR_IP {7.1.2}/TXQ_PFCC/PFCC*/

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000108)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000031C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000444)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 1256)}

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

           {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000272,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x00000008,         0x00924924,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x0000000c,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000400,         0x000001ff,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemFalconUnitTxqPsi function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq psi unit
*/
static void smemFalconUnitTxqPsi
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000128)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000154)}



         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000208, 0x0000020C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000214)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000218, 0x0000021C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000224)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000228, 0x0000022C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000230, 0x00000234)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000238, 0x0000023C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000240, 0x00000244)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000248, 0x0000024C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000254)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A08, 0x00000A0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A10, 0x00000A14)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A18, 0x00000A1C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000C00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003E00, 1016)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005600, 0x00005604)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005608, 0x0000560C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005800, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00009A00, 0x00009A04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00009A08, 0x00009A0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009C00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000AE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000BE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000C600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000CE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000DE00, 0x0000DE04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000DE08, 0x0000DE0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000E000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000F200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00011200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00013200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00019200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001B200, 0x0001B204)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001B208, 0x0001B20C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001B400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001C200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00024200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0002C200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00034200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0003C200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044A00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00045600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00045E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00046600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00046E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00047600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00047E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00049600, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00049A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004A200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004AA00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004AE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004C200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004E200, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0004F200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00054200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0005C200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064200, 0x00064204)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064208, 0x0006420C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064210, 0x00064214)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00064600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00064E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00065600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00065E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00066600, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00066E00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067600, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067620, 0x00067624)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067628, 0x0006762C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067630, 0x00067634)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067700, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067800, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067900, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067920, 0x00067924)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00067A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00068200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00068A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00069200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00069A00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006A200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AA00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AA80, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AB80, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AC80, 0x0006AC84)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AC88, 0x0006AC8C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006AC90, 0x0006AC94)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ACA0, 0x0006ACA4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006ACB0, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006AE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006B600, 0x0006B604)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006BA00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006C200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006CA00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006D200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006DA00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006E200, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EA00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EA80, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EB80, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EC80, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006ECA0, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ECC0, 0x0006ECC4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ECC8, 0x0006ECCC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ECD0, 0x0006ECD4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ECE0, 0x0006ECE4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0006ECF0, 0x0006ECF4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0006EE00, 1024)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00072200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00074200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00076200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00078200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007A200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C200, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007C800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CC00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CD00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CE00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007CE40, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CE80, 0x0007CE84)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CE88, 0x0007CE8C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CE90, 0x0007CE94)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CEA0, 0x0007CEA4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007CEB0, 0x0007CEB4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0007D200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00088200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00088A00, 2048)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00089A00, 2048)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AA00, 256)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AC00, 256)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AE00, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008AE80, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF00, 0x0008AF04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF08, 0x0008AF0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF20, 0x0008AF24)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF28, 0x0008AF2C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF30, 0x0008AF34)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008AF38, 0x0008AF3C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0008C200, 16384)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00094200, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094240, 0x00094244)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094248, 0x0009424C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094250, 0x00094254)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094258, 0x0009425C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094260, 0x00094264)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094268, 0x0009426C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094270, 0x00094274)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094278, 0x0009427C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094280, 0x00094284)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094288, 0x0009428C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094290, 0x00094294)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094298, 0x0009429C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942A0, 0x000942A4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942A8, 0x000942AC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942B0, 0x000942B4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942B8, 0x000942BC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942C0, 0x000942C4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942C8, 0x000942CC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942D0, 0x000942D4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942D8, 0x000942DC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942E0, 0x000942E4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942E8, 0x000942EC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942F0, 0x000942F4)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000942F8, 0x000942FC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094300, 0x00094304)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094308, 0x0009430C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094310, 0x00094314)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094318, 0x0009431C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094320, 0x00094324)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094328, 0x0009432C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094330, 0x00094334)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094338, 0x0009433C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094340, 0x00094344)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094348, 0x0009434C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094350, 0x00094354)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094358, 0x0009435C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094360, 0x00094364)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094368, 0x0009436C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00094370, 0x00094374)}

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

   {
            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
            {
                 {DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x0   }
                ,{DUMMY_NAME_PTR_CNS,           0x00000124,         0x00000002,      2,    0x4   }
                ,{DUMMY_NAME_PTR_CNS,           0x00000130,         0x00001000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000144,         0x00000fff,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000220,         0x0FFFF117,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000228,         0x00001033,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000238,         0x04081020,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0000023C,         0x00000004,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000240,         0x00000007,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000244,         0x00000008,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000200,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000A00,         0x00500000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000A08,         0x066F0002,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000A0C,         0x0000016E,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000208,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00000A18,         0x00001001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00005600,         0x071C0000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00005608,         0x066F0002,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0000560C,         0x0000016E,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00009A00,         0x0E390000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0000DE00,         0x0E390000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0001B200,         0x0E390000,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00009A08,         0x066F0002,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00009A0C,         0x0000016E,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0000DE08,         0x066F0002,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0000DE0C,         0x0000016E,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0001B208,         0x066F0002,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0001B20C,         0x0000016E,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00064210,         0xFFFFFFFF,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00064214,         0x00FFFFFF,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00067620,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0006AC80,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0006ECC0,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0007CE80,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0008AF08,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x00067624,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0006AC84,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0006ECC4,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0007CE84,         0x00000001,      1,    0x0    }
                ,{DUMMY_NAME_PTR_CNS,           0x0008AF0C,         0x00000001,      1,    0x0    }

                ,{NULL,            0,         0x00000000,      0,    0x0      }
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
            unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconUnitTxqPds function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
static void smemFalconUnitTxqPds
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /*memory space from /Cider/EBU-IP/TXQ_IP/SIP7.0 (Falcon)/TXQ_SCN_IP/TXQ_SCN_IP {7.0.11}/TXQ_PDS/PDS*/

        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 24576)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 42688)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 10672)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027000, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027800, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00028000, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00028800, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00029000, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00029800, 1600)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 3200)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00031000, 0x00031030)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00031040, 0x0003167C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00032000, 0x00032018)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00033000, 0x00033010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00033020, 0x0003365C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000340F8, 0x00034118)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034120, 0x00034120)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034128, 0x00034128)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034130, 0x00034130)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003413C, 0x0003413C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034144, 0x00034144)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003414C, 0x00034158)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034164, 0x00034164)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0003416C, 0x0003416C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034174, 0x0003417C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00034200, 0x00034204)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00035000, 0x0003503C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00035100, 0x0003513C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00035200, 0x0003523C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00035300, 0x0003533C)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00031000,         0x0000ffff,     1,   0x0     }
            ,{DUMMY_NAME_PTR_CNS,           0x00031004,         0x00000040,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x00031008,         0x00000ba4,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x0003100c,         0x00000be4,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031010,         0x00000190,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031014,         0x0000000f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031018,         0x00000014,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031020,         0x0000000c,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031028,         0x00000022,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00031030,         0x000002fb,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00032004,         0x00000002,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00033000,         0x0000001f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x0003410c,         0x000007ff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00034114,         0x00000fff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x0003417c,         0x000003ff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00035000,         0x00000001,      16,   0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00035100,         0x0000002a,      16,   0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00035200,         0x00000026,     16,   0x4    }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}


/**
* @internal smemFalconUnitTxqSdq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq sdq unit
*/
static void smemFalconUnitTxqSdq
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {

      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000524)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000550, 0x00000570)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000163C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 3200)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004020)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004120)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200, 0x00004220)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004300, 0x00004320)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x00004420)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005044)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005500, 0x00005544)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006100, 72)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006300, 72)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00007000, 3200)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000, 1600)}

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PortConfig*/
        {
            GT_U32  pt;
            for(pt = 0; pt <= 8; pt++)
            {
                /*0x00003000 + pt*0x4: where pt (0-8) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortConfig[pt] =
                    0x00004000+ pt*0x4;

                /*0x00003000 + pt*0x4: where pt (0-8) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeLow[pt] =
                    0x00004100+ pt*0x4;
                /*0x00003100  + pt*0x4: where pt (0-8) represents PORT*/

                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeHigh[pt] =
                    0x00004200 + pt*0x4;
            }
        }
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x00000006,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000500,         0x00000003,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000007,      9,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00006000,         0x000001ff,      1,    0x0    }
            /*manual mask*/

               /*Sel_List_Range_0_High*/
             ,{DUMMY_NAME_PTR_CNS,0x00005500,        7,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005504,        15,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005508,        23,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000550C,        31,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005510,        39,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005514,        47,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005518,        55,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000551C,        63,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005520,        391,      1,    0x0    }

             /*Sel_List_Range_0_Low*/
             ,{DUMMY_NAME_PTR_CNS,0x00005000,        0,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005004,        8,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005008,        23,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000500C,        16,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005010,        24,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005014,        32,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005018,        40,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000501C,        48,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00005020,        384,      1,    0x0    }

             /*Port_Range_Low*/
             ,{DUMMY_NAME_PTR_CNS,0x00004100,        0,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004104,        8,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004108,        23,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000410C,        16,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004110,        24,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004114,        32,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004118,        40,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000411C,        48,      1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004120,        384,      1,    0x0    }


                /*Port_Range_High*/
             ,{DUMMY_NAME_PTR_CNS,0x00004200,        7,        1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004204,        15,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004208,        23,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000420C,        31,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004210,        39,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004214,        47,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004218,        55,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x0000421C,        63,       1,    0x0    }
             ,{DUMMY_NAME_PTR_CNS,0x00004220,        391,       1,    0x0    }

              /*Port Back Pressure Low Threshold */
              ,{DUMMY_NAME_PTR_CNS,0x00004300,        15000,      8,      0x4  }
              ,{DUMMY_NAME_PTR_CNS,0x00004320,        14200,      1,    0x0    }

              /*Port Back Pressure Low Threshold */
              ,{DUMMY_NAME_PTR_CNS,0x00004400,        15000,      8,    0x4  }
              ,{DUMMY_NAME_PTR_CNS,0x00004420,        14200,      1,    0x0  }
               /*Queue Params Table*/
              ,{DUMMY_NAME_PTR_CNS,           0x00002000,         0x124a4,      400,    0x8    }
              ,{DUMMY_NAME_PTR_CNS,           0x00002004,         0x60000000,   400,    0x8    }

              /*Queue config*/
              ,{DUMMY_NAME_PTR_CNS,           0x00001000,         0x80,      400,    0x4    }



             ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemFalconUnitTxqQfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq qfc unit
*/
static void smemFalconUnitTxqQfc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address */
        {0x0000070C , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteTxqQfcCounterTable1IndirectReadAddress, 20/*bit 20*/},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /*memory space from /Cider/EBU-IP/TXQ_IP/SIP7.0 (Falcon)/TXQ_SCN_IP/TXQ_SCN_IP {7.1.3}/TXQ_QFC/QFC*/

            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000021C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000508)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000630)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000071C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000163C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x0000177C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x0000187C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000211C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x00002220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x0000231C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002500, 0x00002540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003108)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x80000000,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           0x0000000c,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000010,         0x00000002,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000014,         0x00000003,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000018,         0x00000004,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x0000001c,         0x00000005,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000020,         0x00000006,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000024,         0x00000007,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000028,         0x00000008,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000608,         0x0000007f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000610,         0x00000078,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000700,         0x00100000,      1,    0x0    }/*manual entry in order to get 0 */
            ,{DUMMY_NAME_PTR_CNS,           0x00003004,         0x000fffff,      9,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00003108,         0x000fffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000000,      0,    0x0    }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data    = 0x00000708;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address = 0x0000070C;
}


/**
* @internal smemFalconUnitIpvx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the ipvx unit
*/
static void smemFalconUnitIpvx
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
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000F20, 0x00000F24)}
        /* Router QoS Profile Offsets Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ipvxQoSProfileOffsets)}
        /* Router Next Hop Table Age Bits */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000, 3072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(routeNextHopAgeBits)}
        /* Router Acces Matrix Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 3072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(ipvxAccessMatrix)}
        /* Router EVlan Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00100000, 131072),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(66, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEVlan)}
        /* Router EPort Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(112, 16),SMEM_BIND_TABLE_MAC(ipvxIngressEPort)}
        /* Router ECMP table : 2 entries per line */
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00250000, 49152),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(58,  8),SMEM_BIND_TABLE_MAC(ipvxEcmp)}
        /* Router Next Hop Table */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 393216),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(106, 16),SMEM_BIND_TABLE_MAC(ipvxNextHop)}

        /* Router ECMP pointer Table : 4 'pointers' per line */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 49152 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(60, 8),SMEM_BIND_TABLE_MAC(ipvxEcmpPointer)}

    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000047,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x017705dc,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000020,         0x0380001c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000024,         0x00000fff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000100,         0x1b79b01b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000104,         0x001b665b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000200,         0x1b79b01b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000204,         0x0000661B,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000250,         0xffc0fe80,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000254,         0xfe00fc00,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000264,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000268,         0x00000003,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000360,         0x00006140,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000364,         0xffffffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000380,         0x001b9360,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000968,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000978,         0x99abadad,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e00,         0x88878685,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e04,         0x8c8b8a89,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e08,         0x9f8f8e8d,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e0c,         0xa3a2a1a0,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e10,         0xa7a6a5a4,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e14,         0xabaaa9a8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e18,         0xafaeadac,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e1c,         0x90b6b1b0,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e20,         0x91b5b4b3,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e24,         0xcbcac9c8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000e28,         0x9392cdcc,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000f00,         0xffffffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000f04,         0xffffffff,      4,    0x4    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitEgfSht function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-SHT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEgfSht
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* L2 Port Isolation Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00040000 ,34816 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(l2PortIsolation)}
            /* Egress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00060000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(egressStp)}
            /* Non Trunk Members 2 Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00090000 , 32768  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(nonTrunkMembers2)}
            /* Source ID Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00068000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(sst)}
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            /* NOTE: according to Design team the RAM was removed , although CIDER keep showing this table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egfShtEportEVlanFilter)}
#endif /*0*/
            /* Multicast Groups Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00070000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(mcast)}
             /* Device Map Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A0000 ,  16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(deviceMapTable)}
             /* Vid Mapper Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00098000 ,  32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtVidMapper)}
             /* Designated Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A4000 ,   16384 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(1024, 128),SMEM_BIND_TABLE_MAC(designatedPorts)}
            /* Egress EPort table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00078000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(egfShtEgressEPort)}
            /* Non Trunk Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A8000 , 32768  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(1024, 128),SMEM_BIND_TABLE_MAC(nonTrunkMembers)}
            /* Egress vlan table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(egressVlan)}
            /* EVlan Attribute table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00080000 , 32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanAttribute)}
            /* EVlan Spanning table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00088000 , 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanSpanning)}
            /* L3 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00050000 ,34816), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(l3PortIsolation)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0000, 0x000B0010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0020, 0x000B0020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0030, 0x000B0030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000B0040, 0x000B1140)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x000B0000,         0x00004207 ,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0004,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0008,         0x0000000f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0030,         0xffff0000                      }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0040,         0xffffffff,         32,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0240,         0xffffffff,         32,    0x4   }
            ,{DUMMY_NAME_PTR_CNS,           0x000B0340,         0xffffffff,         32,    0x4   }
             /* Device Map Table */
            /* Default value of 'UplinkIsTrunk - 0x1 = Trunk;' like in legacy devices */
            ,{DUMMY_NAME_PTR_CNS,           0x000A0000,         0x00000002,   16384/4,    0x4   }
             /* Designated Port Table */
            ,{DUMMY_NAME_PTR_CNS,           0x000A4000,         0xffffffff,   16384/4,    0x4   }
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020000,         0xffffffff,  131072/4,    0x4   }
#endif
            /* Source ID Members Table */
            ,{DUMMY_NAME_PTR_CNS,           0x00068000,         0xffffffff,   32768/4,    0x4   }

#if 0
            /* since entry 0xfff is out of range ... the HW will not initialize the entry ! */
            /* Multicast Groups Table */
            /* vidx 0xfff - 128 members */
            ,{DUMMY_NAME_PTR_CNS, 0x00070000 + (0xfff * 0x10),  0xffffffff,         4,    0x4   }
      /* since entry 1 depend on 'remote physical port mode' ... the HW will not initialize the entry ! */
            /* Egress vlan table */
            /* vlan 1 members (first 128 port )*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000000 + (1 * 0x10),      0xffffffff,         4,    0x4   }
            /* vlan 1 members (next 128 ports) , because HW default is 256 ports mode */
            ,{DUMMY_NAME_PTR_CNS, 0x00000000 + (((4*_1K)+1) * 0x10),0xffffffff,         4,    0x4    }
#endif /*0*/

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit EGF_sht */
        {/*start of unit global */
            {/*start of unit miscellaneous */
/*              SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.miscellaneous.badAddrLatchReg = 0x000b1140;*/

            }/*end of unit miscellaneous */


            {/*00b07c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 511 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportAssociatedVid1[n] =
                        0x00b07c0+n*0x4;
                }/* end of loop n */
            }/*00b07c0+n*0x4*/
            {/*00b0040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.UCSrcIDFilterEn[n] =
                        0x00b0040+0x4*n;
                }/* end of loop n */
            }/*00b0040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MESHIDConfigs = 0x000b0004;
            {/*00b10c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MCLocalEn[n] =
                        0x00b10c0+0x4*n;
                }/* end of loop n */
            }/*00b10c0+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsMask = 0x000b0020;
            {/*00b1040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        0x00b1040+0x4*n;
                }/* end of loop n */
            }/*00b1040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTGlobalConfigs = 0x000b0000;
            {/*00b02c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpStateMode[n] =
                        0x00b02c0+0x4*n;
                }/* end of loop n */
            }/*00b02c0+0x4*n*/
            {/*00b0340+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpState[n] =
                        0x00b0340+0x4*n;
                }/* end of loop n */
            }/*00b0340+0x4*n*/
            {/*00b01c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        0x00b01c0+0x4*n;
                }/* end of loop n */
            }/*00b01c0+0x4*n*/
            {/*00b03c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportPortIsolationMode[n] =
                        0x00b03c0+0x4*n;
                }/* end of loop n */
            }/*00b03c0+0x4*n*/
            {/*00b0540+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        0x00b0540+0x4*n;
                }/* end of loop n */
            }/*00b0540+0x4*n*/
            {/*00b05c0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ePortMeshID[n] =
                        0x00b05c0+n * 0x4;
                }/* end of loop n */
            }/*00b05c0+n * 0x4*/
            {/*00b04c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        0x00b04c0+0x4*n;
                }/* end of loop n */
            }/*00b04c0+0x4*n*/
            {/*00b00c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        0x00b00c0+0x4*n;
                }/* end of loop n */
            }/*00b00c0+0x4*n*/
            {/*00b0240+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        0x00b0240+0x4*n;
                }/* end of loop n */
            }/*00b0240+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportVlanEgrFiltering = 0x000b000c;
            {/*00b0fc0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportDropOnEportVid1Mismatch[n] =
                        0x00b0fc0+n * 0x4;
                }/* end of loop n */
            }/*00b0fc0+n * 0x4*/
            {/*00b0140+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportBridgedForwardRestricted[n] =
                        0x00b0140+0x4*n;
                }/* end of loop n */
            }/*00b0140+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsCause = 0x000b0010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTEgrFiltersEnable = 0x000b0008;

        }/*end of unit global */


    }/*end of unit EGF_sht */


}
/**
* @internal smemFalconUnitEgfQag function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-QAG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEgfQag
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(egfQagTcDpMapper)}
            /* VOQ_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00094000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4)}
            /* Port_Target_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00096000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(egfQagPortTargetAttribute)}
            /* Port_Enq_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00097000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(15, 4),SMEM_BIND_TABLE_MAC(egfQagTargetPortMapper)}
            /* Port_Source_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00098000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(9, 4) ,SMEM_BIND_TABLE_MAC(egfQagPortSourceAttribute)}
            /* EVIDX_Activity_Status_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00099000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}
            /* Cpu_Code_To_Loopback_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0009A000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4), SMEM_BIND_TABLE_MAC(egfQagCpuCodeToLbMapper)}

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
* @internal smemFalconUnitEgfEft function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-EFT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEgfEft
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* ROC (read only clear) counters */
        { 0x00006410, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00006414, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00006418, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00006420, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00006424, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x0000642c, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Secondary Target Port Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4) , SMEM_BIND_TABLE_MAC(secondTargetPort)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030, 0x00001030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010A0, 0x000010A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010B0, 0x000010B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001110, 0x00001110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001130, 0x00001130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001140, 0x00001140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x000013CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002108, 0x00002108)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021D0, 0x000021D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x000022FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x0000237C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000301C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003080, 0x0000337C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004010, 0x0000401C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006080, 0x000063FC)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006400, 0x00006424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006440, 0x00006440)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);


        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00001000,         0x000007e0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x08e00800 ,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001030,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001130,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001150,         0x0000FC3F,    128,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002100,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108,         0x00000808,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000021d0,         0x00000088,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002204,         0xffffffff,      16,   0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000010,      8,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00006000,         0x00000801,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00006004,         0x000FFF39,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }


    {/*start of unit EGF_eft */
        {/*start of unit mcFIFO */
            {/*start of unit mcFIFOConfigs */
                {/*0002200+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 63 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[n] =
                            0x0002200+4*n;
                    }/* end of loop n */
                }/*0002200+4*n*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOGlobalConfigs = 0x00002100;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOArbiterWeights0 = 0x00002108;

            }/*end of unit mcFIFOConfigs */


        }/*end of unit mcFIFO */


        {/*start of unit miscellaneous */
            /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.miscellaneous.badAddrLatchReg = 0x00005000;*/

        }/*end of unit miscellaneous */


        {/*start of unit global */
            {/*0001350+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.bypassEGFTargetEPortAttributes[n] =
                        0x0001350+n*0x4;
                }/* end of loop n */
            }/*0001350+n*0x4*/
            {/*0001150+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.cpuPortMapOfReg[n] =
                        0x0001150+4*n;
                }/* end of loop n */
            }/*0001150+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.EFTInterruptsMask = 0x000010b0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.portIsolationLookup0 = 0x00001020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.portIsolationLookup1 = 0x00001030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.EFTInterruptsCause = 0x000010a0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.cpuPortDist = 0x00001000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.ECCConf = 0x00001130;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.ECCStatus = 0x00001140;

        }/*end of unit global */


        {/*start of unit egrFilterConfigs */
            {/*0006100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedMCFilterEn[n] =
                        0x0006100+0x4*n;
                }/* end of loop n */
            }/*0006100+0x4*n*/
            {/*0006180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedBCFilterEn[n] =
                        0x0006180+0x4*n;
                }/* end of loop n */
            }/*0006180+0x4*n*/
            {/*0006080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unknownUCFilterEn[n] =
                        0x0006080+0x4*n;
                }/* end of loop n */
            }/*0006080+0x4*n*/
            {/*0006380+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        0x0006380+4*n;
                }/* end of loop n */
            }/*0006380+4*n*/
            {/*0006200+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[n] =
                        0x0006200+0x4*n;
                }/* end of loop n */
            }/*0006200+0x4*n*/
            {/*0006300+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.enFCTriggerByCNFrameOnPort[n] =
                        0x0006300+0x4*n;
                }/* end of loop n */
            }/*0006300+0x4*n*/
            {/*0006280+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.enCNFrameTxOnPort[n] =
                        0x0006280+0x4*n;
                }/* end of loop n */
            }/*0006280+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersGlobal = 0x00006000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersEnable = 0x00006004;

        }/*end of unit egrFilterConfigs */


        {/*start of unit deviceMapConfigs */
            {/*0003080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localSrcPortMapOwnDevEn[n] =
                        0x0003080+0x4*n;
                }/* end of loop n */
            }/*0003080+0x4*n*/
            {/*0003100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localTrgPortMapOwnDevEn[n] =
                        0x0003100+0x4*n;
                }/* end of loop n */
            }/*0003100+0x4*n*/
            {/*0003180+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.portAddrConstructMode[n] =
                        0x0003180+4*n;
                }/* end of loop n */
            }/*0003180+4*n*/
            {/*0003000+4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.devMapTableAddrConstructProfile[p] =
                        0x0003000+4*p;
                }/* end of loop p */
            }/*0003000+4*p*/

        }/*end of unit deviceMapConfigs */


        {/*start of unit cntrs */
            {/*0004010+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.TXQ0MCFIFODropCntr[n] =
                        0x0004010+4*n;
                }/* end of loop n */
            }/*0004010+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.descEccCriticalDropCounter = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.descEccNonCriticalDropCounter = 0x00004004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00004008;

        }/*end of unit cntrs */


        {/*start of unit egress MIB counters */
            GT_U32    n;
            for(n = 0 ; (n <= 1); n++)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                    egrMIBCntrs.egrMIBCntrsPortSetConfig[n]       = 0x00006408 + (n * 4) ;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                    egrMIBCntrs.egrMIBCntrsSetConfig[n]           = 0x00006400 + (n * 4);
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                    egrMIBCntrs.egrMIBMcFIFODroppedPktsCntr[n]    = 0x00006418 + (n * 4);
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                    egrMIBCntrs.egrMIBBridgeEgrFilteredPktCntr[n] = 0x00006410 + (n * 4);
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                    egrMIBCntrs.egrMIBMcFilteredPktCntr[n]        = 0x00006420 + (n * 4);
            }
        }/*end of unit egress MIB counters */

        {/* start of unit CNC eVlan Configurations */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
                cncEvlanConfigs.vlanPassDropConfigs     = 0x00006440;
        }/*end of unit CNC eVlan Configurations */
    }/*end of unit EGF_eft */


}


/**
* @internal smemFalconUnitPolicerUnify function
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
static void smemFalconUnitPolicerUnify
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000070)}
            /*registers -- not table/memory !! -- Policer Table Access Data<%n> */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000074 ,8*4),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTblAccessData)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000224)}

            /*Policer Timer Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000300, 36), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTimer)}
            /*Policer Management Counters Memory --> NOTE: those addresses are NOT accessible by CPU !!! */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (POLICER_MANAGEMENT_COUNTER_ADDR_CNS/*0x00000500*/, 192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74,16),SMEM_BIND_TABLE_MAC(policerManagementCounters)}
            /*IPFIX wrap around alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixWaAlert)}
            /*IPFIX aging alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixAgingAlert)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002010, 0x00002014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002020, 0x00002058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x000024FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003510, 0x00003514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003520, 0x00003524)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003550, 0x00003554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003560, 0x00003564)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003604, 0x0000360C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003704, 0x0000370C)}

            /*registers (not memory) : Port%p and Packet Type Translation Table*/
            /* port attributes (replace -- Port%p and Packet Type Translation Table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000 , 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13,4),SMEM_BIND_TABLE_MAC(policerMeterPointer)}
            /* replace : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInitialDP*/
            /*qos Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 1024) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20,4),SMEM_BIND_TABLE_MAC(policerQosAttributes)}
            /*e Attributes Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerEPortEVlanTrigger)}
            /*Ingress Policer Re-Marking Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 8192 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(51,8),SMEM_BIND_TABLE_MAC(policerReMarking)}
            /*Metering Conformance Level Sign Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 16384 ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(2,4),SMEM_BIND_TABLE_MAC(policerConformanceLevelSign)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemBobkPolicerTablesSupport(devObjPtr,numOfChunks,chunksMem,plrUnit);

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
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C0000, 131072 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(229,32),SMEM_BIND_TABLE_MAC(policerCounters)}
                /*Metering Configuration Memory*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768  ) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(55,8),SMEM_BIND_TABLE_MAC(policerConfig)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

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
            ,{DUMMY_NAME_PTR_CNS, 0x00060000, 0x00000030, 0x1000, 0x8}
            ,{NULL,               0,          0x00000000, 0,     0x0}
        };


        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list =
            {myUnit_registersDefaultValueArr,NULL};

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

static void smemFalconUnitIplr0
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

    smemFalconUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E);
}

static void smemFalconUnitIplr1
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

    smemFalconUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E);
}

static void smemFalconUnitEplr
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

    smemFalconUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_EPLR_E);
}

/**
* @internal smemFalconUnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitHa
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000009C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000a0, 0x000000f0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C0, 0x000003C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003D0, 0x000003D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000410)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000420, 0x00000428)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000430, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000550, 0x00000560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x0000071C)}
            /*HA Physical Port Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(27, 4) , SMEM_BIND_TABLE_MAC(haEgressPhyPort1)}
            /*HA Physical Port Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(82, 16) , SMEM_BIND_TABLE_MAC(haEgressPhyPort2)}
            /*HA QoS Profile to EXP Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000 , 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(3, 4) , SMEM_BIND_TABLE_MAC(haQosProfileToExp)}
            /*HA Global MAC SA Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8) , SMEM_BIND_TABLE_MAC(haGlobalMacSa)}
            /*EVLAN Table (was 'vlan translation' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36, 8) , SMEM_BIND_TABLE_MAC(egressVlanTranslation)}
            /*VLAN MAC SA Table (was 'VLAN/Port MAC SA Table' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4) , SMEM_BIND_TABLE_MAC(vlanPortMacSa)}
            /*PTP Domain table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 40960) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(35, 8) , SMEM_BIND_TABLE_MAC(haPtpDomain)}
            /*Generic TS Profile table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(537,128), SMEM_BIND_TABLE_MAC(tunnelStartGenericIpProfile) }
            /*EPCL User Defined Bytes Configuration Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(600,128) , SMEM_BIND_TABLE_MAC(haEpclUserDefinedBytesConfig)}
            /*HA Egress ePort Attribute Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(96,16) , SMEM_BIND_TABLE_MAC(haEgressEPortAttr1)}
            /*HA Egress ePort Attribute Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4) , SMEM_BIND_TABLE_MAC(haEgressEPortAttr2)}
            /*Router ARP DA and Tunnel Start Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 1572864 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384,64) , SMEM_BIND_TABLE_MAC(arp)/*tunnelStart*/}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS, 0x00000000, 0x00101010, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000004, 0x03FDD003, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000008, 0x0000000C, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000068, 0x00000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000006C, 0x00000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000070, 0x000C0000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000080, 0xFF000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000084, 0x00000001, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000008c, 0xFF020000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000090, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000094, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000098, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000009c, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x000000A4, 0x00000003, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x000003D0, 0x00000008, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000408, 0x81000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000040c, 0x81000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000410, 0x00010000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000424, 0x00110000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000500, 0x00008100, 8,     0x4}
           ,{DUMMY_NAME_PTR_CNS, 0x00000550, 0x88488847, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000554, 0x00008100, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000558, 0x000022F3, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000055C, 0x00006558, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000710, 0x0000FFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000714, 0x0000003F, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000071C, 0x00000803, 1,     0x0}
           ,{NULL,               0,          0x00000000, 0,     0x0}
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list =
                {myUnit_registersDefaultValueArr,NULL};

        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

#define FALCON_PPG_OFFSET 0x80000
#define FALCON_PPN_OFFSET  0x4000
#define FALCON_PPN_NUM     3
#define FALCON_PPG_NUM     9

/*ppn%n 0x00000000 + 0x80000 * m + 0x4000 * p: where p (0-8) represents PPN, where m (0-2) represents PPG*/
#define FALCON_PPN_REPLICATIONS FALCON_PPN_NUM,FALCON_PPG_OFFSET , FALCON_PPG_NUM,FALCON_PPN_OFFSET

#define FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS FORMULA_TWO_PARAMETERS(FALCON_PPN_NUM,FALCON_PPG_OFFSET , FALCON_PPG_NUM,FALCON_PPN_OFFSET)

/*ppg%g 0x00000000 + 0x80000 * g : where g (0-2) represents PPG*/
#define FALCON_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER FORMULA_SINGLE_PARAMETER(FALCON_PPN_NUM,FALCON_PPG_OFFSET)

#define FALCON_PPG_REPLICATION_ADDR(ppg)   \
    (FALCON_PPG_OFFSET * (ppg))

#define FALCON_PPN_REPLICATION_ADDR(ppg,ppn)   \
    (FALCON_PPG_REPLICATION_ADDR(ppg) + (FALCON_PPN_OFFSET * (ppn)))

#define FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,ppn) \
    /*PPN Internal Error Cause Register*/         \
    {0x00003040 + FALCON_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003044 + FALCON_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define FALCON_PPG_REPLICATIONS_ACTIVE_MEM(ppg) \
    /*PPG Interrupt Cause Register*/     \
    {0x0007FF00 + FALCON_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF04 + FALCON_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
    /*PPG internal error cause Register*/                                                                                              \
    {0x0007FF10 + FALCON_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF14 + FALCON_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
                                        \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,0), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,1), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,2), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,3), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,4), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,5), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,6), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,8), \
    FALCON_PPN_REPLICATIONS_ACTIVE_MEM(ppg,7)

/**
* @internal smemFalconUnitPha function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PHA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitPha
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

    /* 3 PPG (with 9 PPNs each) */
    FALCON_PPG_REPLICATIONS_ACTIVE_MEM(0),/*include all it's PPN*/
    FALCON_PPG_REPLICATIONS_ACTIVE_MEM(1),/*include all it's PPN*/
    FALCON_PPG_REPLICATIONS_ACTIVE_MEM(2),/*include all it's PPN*/

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
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16)}, FALCON_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}  /*IMEM*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF00, 0x0007FF04)}, FALCON_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF10, 0x0007FF14)}, FALCON_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF70, 0x0007FF70)}, FALCON_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}

            /* per PPN */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2048)}      , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*scratchpad*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 20)}        , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet configuration*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002020, 4)}         , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet special registers*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002100, 64)}        , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet descriptor*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002200, 160)}       , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet header*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003008)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003010, 0x00003010)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003020, 0x00003024)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x00003044)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003050, 0x00003058)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003060, 0x00003068)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003070, 0x00003070)} , FALCON_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}

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
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      FALCON_PPN_REPLICATIONS   }

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

    /*m * 0x80000: where m (0-2) represents PPG*/
    for(ppg = 0 ; ppg < 3; ppg++) /* per PPG */
    {
        ppgOffset = ppg * 0x80000;

        {/*start of unit ppg_regs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInterruptSumCause = 0x0007ff00 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInterruptSumMask = 0x0007ff04 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInternalErrorCause = 0x0007ff10 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGInternalErrorMask = 0x0007ff14 + ppgOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PHA.PPG[ppg].ppg_regs.PPGBadAddrLatch = 0x0007ff70 + ppgOffset;

        }/*end of unit ppg_regs */

        /* 0x4000 * p: where p (0-8) represents PPN*/
        for(ppn = 0 ; ppn < 9; ppn++) /* per PPN (PER PPG) */
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
* @internal smemFalconUnitErmrk function
* @endinternal
*
* @brief   Allocate address type specific memories for ermrk (egress remark):
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitErmrk
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* ERMRK Interrupt Cause */
        {0x00000004, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 11 /*parent summary bit*/, smemChtActiveWriteIntrCauseReg, 0},
        /* ERMRK Interrupt Mask */
        {0x00000008, SMEM_FULL_MASK_CNS,
            NULL, 0, smemLion3ActiveWriteHaInterruptsMaskReg, 0},

        /* ROC (read only clear) counters */
        /*Tables Error Drop Counter*/
        { 0x0000002C, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /*ERMRK Config Stage Desc Counter*/
        { 0x00000100, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        /*ERMRK Modify Stage Desc Counter*/
        { 0x00000104, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        /* ERMRK PTP Configuration */
        {0x10, 0xFFFFFFFF, NULL, 0, smemLion3ActiveWriteErmrkPtpConfReg, 0},

        /* ERMRK Timestamp Queue Clear Entry ID */
        {0xE4, 0xFFFFFFFF, NULL, 0, smemLion3ActiveWriteErmrkTsQueueEntryIdClearReg, 0},

        /* ERMRK Timestamp Queue Entry Word1/Word2 */
        {0x120, 0xFFFFFFE0, smemLion3ActiveReadErmrkTsQueueEntryWord1Word2Reg, 0, NULL, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000038, 0x00000038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x0000009C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E0, 0x000000E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000013C)}
            /* tables */
            /*ERMRK LM Table  Memory  512 Byte, 128 entries (8 bits)aligned to 4 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 512),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4), SMEM_BIND_TABLE_MAC(oamLmOffsetTable)}
            /*QOS_DSCP_MAP and QOS_TC_DP_MAP 384 entries, (24 bits)aligned to 4 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000, 1536),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24, 4), SMEM_BIND_TABLE_MAC(ERMRKQosMapTable)}
            /*ERMRK Timestamp Configuration Table Memory 128 entries (34 bits)aligned to 8 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 1024),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(34, 8), SMEM_BIND_TABLE_MAC(oamTimeStampOffsetTable)}
            /*ERMRK PTP Local Action Table 16384  entries (30 bits)aligned to 4 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000,  65536),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(30, 4), SMEM_BIND_TABLE_MAC(ptpLocalActionTable)}
            /*ERMRK Target Port Table Memory 1024 entries (321 bits)aligned to 64 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 65536),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(321, 64), SMEM_BIND_TABLE_MAC(ptpTargetPortTable)}
            /*ERMRK PTP Source Port Table Memory 1024 entries (321 bits)aligned to 64 bytes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 65536),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(321, 64), SMEM_BIND_TABLE_MAC(ptpSourcePortTable)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* ERMRK Cumulative Scale Rate TAI<<%i>> reg0/1 */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x0000015C)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }
}

/**
* @internal smemFalconUnitErep function
* @endinternal
*
* @brief   Allocate address type specific memories for EREP.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitErep
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* EREP Interrupt Cause */
        {0x00003000, SMEM_FULL_MASK_CNS,
            smemChtActiveReadIntrCauseReg, 31 /*unknown parent summary bit*/, smemChtActiveWriteIntrCauseReg, 0},
        /* ERMRK Interrupt Mask */
        {0x00003004, SMEM_FULL_MASK_CNS,
            NULL, 0, smemLion3ActiveWriteHaInterruptsMaskReg, 0},

        /* ROC (read only clear) counters */
        { 0x00002000, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002004, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002008, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x0000200C, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002010, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002014, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002018, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x0000201C, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002020, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002024, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002028, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x0000202C, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002030, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
        { 0x00002034, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000FFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x0000103C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002044, 0x00002060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003104)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Egress Mirroring Analyzer Ratio */
            { DUMMY_NAME_PTR_CNS,   0x00001024,     0x00000001,     7,      0x4 }

            ,{NULL,                 0,              0x00000000,     0,      0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* PRP_Global_Configuration */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x00001050)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);

        {
            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
            {
                { DUMMY_NAME_PTR_CNS,   0x00001050,     0x000088FB ,     1,      0x0 }

                ,{NULL,            0,         0x00000000,      0,    0x0      }
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

            /* set the next !! to point to the additional defaults */
            unitPtr->unitDefaultRegistersPtr->nextPtr = &list;
        }
    }

    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.configurations.dropCodeConfigs                        = 0x0000100C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.configurations.erepGlobalConfigs                      = 0x00001014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.configurations.dropCodeStatus                         = 0x00001018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.qcnOutgoingCounter                           = 0x00002000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.sniffOutgoingCounter                         = 0x00002004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.trapOutgoingCounter                          = 0x00002008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.mirrorOutgoingCounter                        = 0x0000200C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.qcnFifoFullDroppedPacketsCounter             = 0x00002010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.sniffFifoFullDroppedPacketsCounter           = 0x00002014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.trapFifoFullDroppedPacketsCounter            = 0x00002018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.mirrorFifoFullDroppedPacketsCounter          = 0x0000201C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.hbuPacketsOutgoingForwardCounter             = 0x00002034;
        {/*0x00001020+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 6 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.configurations.egressStatisticalMirroringToAnalyzerRatio[n] =
                                                                                        0x00001020+((n+1)*0x4);
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.counters.egressAnalyzerCounter[n] =
                                                                                        0x00002044+((n+1)*0x4);
            }/* end of loop n */
        }/*00001020+n*0x4*/
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EREP.configurations.PRP_Global_Configuration = 0x00001050;
        }
    }


}

/**
* @internal smemFalconUnitFdbTable function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the FDB TABLE unit
*
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] instanceId           - the instance Id of the FDB memory
*/
static void smemFalconUnitFdbTable
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  instanceId
)
{
#define  fdbEntryNumBytes_align 16 /* entry is 115 bits */

    {
        SMEM_GENERIC_HIDDEN_MEM_STC *infoPtr;
        infoPtr = SMEM_HIDDEN_MEM_INFO_GET(devObjPtr,SMEM_GENERIC_HIDDEN_MEM_FDB_E,instanceId);

        if(infoPtr->startOfMemPtr == NULL)
        {
            infoPtr->startOfMemPtr = smemDeviceObjMemoryAlloc__internal(devObjPtr,devObjPtr->fdbMaxNumEntries,fdbEntryNumBytes_align,__FILE__,__LINE__);
            if(infoPtr->startOfMemPtr == NULL)
            {
                skernelFatalError("smemFalconUnitFdbTable: allocation failed \n" );
            }

            infoPtr->alignmentOfEntryInWords = fdbEntryNumBytes_align / 4;
            infoPtr->numOfEntries = devObjPtr->fdbMaxNumEntries;
            infoPtr->tableType = SMEM_GENERIC_HIDDEN_MEM_FDB_E;
        }
    }

}

/**
* @internal smemFalconUnitExactMatchTable function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EXACT MATCH TABLE unit
*
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] instanceId           - the instance Id of the EM memory
*/
static void smemFalconUnitExactMatchTable
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  instanceId
)
{
#define  exactMatchEntryNumBytes_align  16 /* entry is 115 bits */

    {
        SMEM_GENERIC_HIDDEN_MEM_STC *infoPtr;
        infoPtr = SMEM_HIDDEN_MEM_INFO_GET(devObjPtr,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E,instanceId);

        if(infoPtr->startOfMemPtr == NULL)
        {
            infoPtr->startOfMemPtr = smemDeviceObjMemoryAlloc__internal(devObjPtr,devObjPtr->emMaxNumEntries,
                                                                        exactMatchEntryNumBytes_align,__FILE__,__LINE__);
            if(infoPtr->startOfMemPtr == NULL)
            {
                skernelFatalError("smemFalconUnitExactMatchTable: allocation failed \n" );
            }

            infoPtr->alignmentOfEntryInWords = exactMatchEntryNumBytes_align / 4;
            infoPtr->numOfEntries = devObjPtr->emMaxNumEntries;
            infoPtr->tableType = SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E;
        }
    }

}

/**
* @internal smemFalconUnitAacMem function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the AAC unit
*
* @param[in] devObjPtr                - pointer to device memory.
*/
static void smemFalconUnitAacMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    #define  aacEntryNumBytes_align                 4   /* entry is one word bytes */
    #define  AAC_MAX_NUM_OF_TRANSACTION_WORDS       32  /* Up to 32 Outstanding accesses */
    {
        SMEM_GENERIC_HIDDEN_MEM_STC *infoPtr;
        SMEM_GENERIC_HIDDEN_MEM_ENT memType;

        for (memType = SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL0_E; memType <= SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL7_E; memType++)
        {
            infoPtr = SMEM_HIDDEN_MEM_INFO_GET(devObjPtr, memType, 0);

            if(infoPtr->startOfMemPtr == NULL)
            {
                /* Allocate AAC memory */
                infoPtr->startOfMemPtr = smemDeviceObjMemoryAlloc__internal(devObjPtr, AAC_MAX_NUM_OF_TRANSACTION_WORDS, aacEntryNumBytes_align, __FILE__, __LINE__);
                if(infoPtr->startOfMemPtr == NULL)
                {
                    skernelFatalError("smemFalconUnitAacMem: allocation failed \n" );
                }

                infoPtr->alignmentOfEntryInWords = aacEntryNumBytes_align / 4;
                infoPtr->numOfEntries = AAC_MAX_NUM_OF_TRANSACTION_WORDS;
                infoPtr->tableType = memType;
                infoPtr->currentWordIndex = 0;
            }
        }
    }
}

/**
* @internal smemFalconUnitIa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Ingress Aggregator
*         unit
*/
static void smemFalconUnitIa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    GT_U32  unitIndex = 0;
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000404)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000504)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,         0x00000000,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000004,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000008,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x0000000c,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000100,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000104,         0xffffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000300,         0xFFFF0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,         0x00000404,         0xAAAAAAAA,      1,    0x0      }

            ,{NULL,                                0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    /* init the addresses of the registers DB */
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPPrioThresholds0 = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPPrioThresholds1 = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPLsThreshold     = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPEnable          = 0x0000000C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPPrioThresholdsRandomization = 0x000000010;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].pfcTriggering.PIPPfcCounterThresholds         = 0x00000100;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].pfcTriggering.PIPPfcGlobalFillLevelThresholds = 0x00000104;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].pfcTriggering.GlobalTcPfcTriggerVectorsConfig = 0x00000108;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].pfcTriggering.PIPTcPfcTriggerVectorsConfig[0] = 0x0000010c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].pfcTriggering.PIPTcPfcTriggerVectorsConfig[1] = 0x00000110;
    }

}
/**
* @internal smemFalconUnitDfxServer function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the DFX unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitDfxServer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    BUILD_STRING_FOR_UNIT_NAME(DFX_EXT_SERVER);

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG() */
    {0x000f800c, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteDfxServerResetControlReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /* DDR 0 */
             BC2_XSB_DDR_CONTROLER_DDR_MEMORY_MAC(0x00000)
             /* DDR 1 */
            ,BC2_XSB_DDR_CONTROLER_DDR_MEMORY_MAC(0x28000)
             /* DDR 2 */
            ,BC2_XSB_DDR_CONTROLER_DDR_MEMORY_MAC(0x48000)
             /* DDR 3 */
            ,BC2_XSB_DDR_CONTROLER_DDR_MEMORY_MAC(0x70000)
             /* DDR 4 */
            ,BC2_XSB_DDR_CONTROLER_DDR_MEMORY_MAC(0xB8000)

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8000, 0x000F8018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8020, 0x000F8038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8040, 0x000F804C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8060, 0x000F806C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8078, 0x000F80AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80B0, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F80C0, 4)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8D00, 0x000F8D28)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F8F00, 64)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000F9000, 1024)}

            /* XSB_XBAR_IP units*/
            FALCON_XSB_CLUSTER_MEMORY_MAC(XSB_ADRESS_SPACE)
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
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

    /* smemLion3UnitDfx(devObjPtr, unitPtr, 0); */

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

            /* DDR 0 */
            ,REG_DEFAULT___XSB_XBAR_DUNIT__INSTANCE_MAC(0x00000)
            /* DDR 1 */
            ,REG_DEFAULT___XSB_XBAR_DUNIT__INSTANCE_MAC(0x28000)
            /* DDR 2 */
            ,REG_DEFAULT___XSB_XBAR_DUNIT__INSTANCE_MAC(0x48000)
            /* DDR 3 */
            ,REG_DEFAULT___XSB_XBAR_DUNIT__INSTANCE_MAC(0x70000)
            /* DDR 4 */
            ,REG_DEFAULT___XSB_XBAR_DUNIT__INSTANCE_MAC(0xB8000)


         ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

extern GT_U32 simulationSoftResetDone;

/**
* @internal smemFalconActiveWriteCnmRfuSystemSoftResetReg function
* @endinternal
*
* @brief   CNM RFU System Soft Reset Register active write.
*/
void smemFalconActiveWriteCnmRfuSystemSoftResetReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemFalconActiveWriteCnmRfuSystemSoftResetReg);

    GT_U32 dfx0ServerResetControl;      /* dfx0 Server Reset Control register data */
    GT_U32 regAddr;                     /* register address */
    GT_U32 dfx0ResetEdge;               /* DFX0 Reset Edge*/
    GT_U32 unitBase;                    /* base address of unit */

    *memPtr = *inMemPtr;/* save value that written by the CPU*/

    /* EAGLE tile0 serverResetControl register address */
    unitBase = smemUnitBaseAddrByNameGet(
        devObjPtr, "UNIT_DFX_SERVER"/*unitNameStr*/, 0/*allowNonExistUnit*/);
    /* this unit size is 1 M bytes */
    regAddr = (unitBase | 0x000F800C);

    smemRegGet(devObjPtr, regAddr, &dfx0ServerResetControl);
    dfx0ResetEdge = ((dfx0ServerResetControl >> 12) & 1);

    /*<CNM RFU System Soft Reset Trigger>*/
    if (SMEM_U32_GET_FIELD((*memPtr) ,0 ,1) != dfx0ResetEdge)
    {
        /* trigger bit not set */
        return;
    }

    /* self clear */
    SMEM_U32_SET_FIELD((*memPtr) ,0 ,1, ((~ dfx0ResetEdge) & 1));

    if(devObjPtr->numOfTiles)
    {
        /* Falcon TBD: Currently only first tile pair is triggering reset for the whole device */
        if ((address >> 29) > 0)
        {
            __LOG(("Trigger for reset on address [0x%08x] (tile:[%d]) is IGNORED.\n",
                address, (address >> 29)));
            return;
        }

        __LOG(("Trigger for reset on address [0x%08x] (tile:[%d]) affects the whole device.\n",
            address, (address >> 29)));
    }

    smemDevSoftResetCommonStart(devObjPtr,*memPtr);
}

/**
* @internal smemFalconUnitCnmRfu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM RFU unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitCnmRfu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Active access bind */
    /*BUILD_STRING_FOR_UNIT_NAME(CNM_RFU);*/

    if (devObjPtr->deviceFamily == SKERNEL_PHOENIX_FAMILY)
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* /Phoenix/<CNM_IP>CNM_IP/<CNM_IP> <CnM_RFU> CnM_RFU/Units/CNM_Misc/System Soft Reset */
        {0x00000014, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteCnmRfuSystemSoftResetReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    else
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* /Eagle/CnM/<SW_CNM>CNM_IP/<SW_CNM> CNM_RFU/Units/CNM_Misc/System Soft Reset */
        {0x00000044, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteCnmRfuSystemSoftResetReg, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    /* Address space */
    if (devObjPtr->deviceFamily == SKERNEL_HAWK_FAMILY)
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000018)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000020)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000004C, 0x0000005C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000114)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001514)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001520, 0x00001524)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001530, 0x00001534)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001540, 0x00001544)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }
    else
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000004C, 0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000010C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001520, 0x00001524)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001530, 0x00001534)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001540, 0x00001544)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    /* default values */
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
              {DUMMY_NAME_PTR_CNS,            0x00000058,         0x00008000,      1,    0x0   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x11111111,      2,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x00000011,      1,    0x0   }
             ,{DUMMY_NAME_PTR_CNS,            0x00001510,         0x76543210,      4,    0x10  }
             ,{DUMMY_NAME_PTR_CNS,            0x00001514,         0xfedcba98,      4,    0x10  }

             ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitCnmPexComphyUnit function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM COMPHY unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemFalconUnitCnmPexComphyUnit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Address space */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A22c, 0x0000A230)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A314, 0x0000A318)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002294, 0x00002298)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    /* default values */
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
              {DUMMY_NAME_PTR_CNS,            0x0000A22C,         0x00000004,      1,    0x0   }
             ,{DUMMY_NAME_PTR_CNS,            0x0000A314,         0x0C040324,      1,    0x0   }
             ,{DUMMY_NAME_PTR_CNS,            0x00002294,         0x00000005,      1,    0x0   }

             ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitCnmAac function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM ACC unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitCnmAac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Active access bind */
    /*BUILD_STRING_FOR_UNIT_NAME(CNM_RFU);*/

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)

    /* indirect Access Data Channel 0 Register */
    {0x00000050, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 1 Register */
    {0x00000054, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 2 Register */
    {0x00000058, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 3 Register */
    {0x0000005c, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 4 Register */
    {0x00000060, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 5 Register */
    {0x00000064, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 6 Register */
    {0x00000068, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Data Channel 7 Register */
    {0x0000006c, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},

    /* indirect Access Address Pipe 0 Register */
    {0x00000080, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 1 Register */
    {0x00000084, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 2 Register */
    {0x00000088, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 3 Register */
    {0x0000008c, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 4 Register */
    {0x00000090, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 5 Register */
    {0x00000094, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 6 Register */
    {0x00000098, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},
    /* indirect Access Address Pipe 7 Register */
    {0x0000009c, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteAacTableMemory, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr, unitPtr)

    /* Address space */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000003c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000006c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x0000009c)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000b0, 0x000000cc)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000e0, 0x000000fc)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000012c)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {/*start of unit cnm_regs */
        {/*start of unit AAC */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACControl                             = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEnginesStatus                       = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACInvalidtransactionaddress           = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACErrorStatus                         = 0x0000000c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACInterruptsCause                     = 0x00000010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACInterruptsMask                      = 0x00000014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACBadAddrLatchReg                     = 0x00000018;
            {
                GT_U32    a;
                for(a = 0 ; a <= 7 ; a++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineControl[a]            = 0x00000020 + 0x4 * a;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineData[a]               = 0x00000050 + 0x4 * a;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineAddress[a]            = 0x00000080 + 0x4 * a;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCInternalOffset[a]   = 0x000000b0 + 0x4 * a;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCExternalOffset[a]   = 0x000000e0 + 0x4 * a;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCControl[a]          = 0x00000110 + 0x4 * a;
                }
            }
        } /*end of unit AAC */
    }/* end of unit cnm_regs */

}

/**
* @internal smemFalconUnitEq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Eq unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEq
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000058, 0x0000007C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000088, 0x0000008C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000011C)}
#if 0 /* Ingress STC interrupts removed */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000060C)}
#endif
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005018, 0x00005058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000703C),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(tcpUdpDstPortRangeCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007800, 0x0000783C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007C00, 0x00007C10),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ipProtCpuCode)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000, 0x0000A008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A010, 0x0000A010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A020, 0x0000A034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A100, 0x0000A17C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF00, 0x0000AF08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF10, 0x0000AF10)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000AF30, 0x0000AF30)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B000, 0x0000B000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B008, 0x0000B024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B04C, 0x0000B064)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B200, 0x0000B218)}
            /*Mirroring to Analyzer Port Configurations/Port Ingress Mirror Index Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x0000B400, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqPhysicalPortIngressMirrorIndexTable)}
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

            /*Statistical Rate Limits Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(statisticalRateLimit)}
            /*QoSProfile to QoS Table/QoSProfile to QoS Table              */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00110000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(qosProfile)}
            /*CPU Code Table/CPU Code Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00120000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(cpuCode)}
            /*TO CPU Packet Rate Limiters/TO CPU Rate Limiter Configuration*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00130000, 1024)}
            /*L2 ECMP/Trunk LTT                                            */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(28, 4),SMEM_BIND_TABLE_MAC(eqTrunkLtt)}
            /*ePort/EQ Ingress ePort Table                                 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00700000, 8192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(eqIngressEPort)}
            /*L2 ECMP/ePort LTT                                            */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00800000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(28, 4),SMEM_BIND_TABLE_MAC(eqL2EcmpLtt)}
            /*L2 ECMP/L2ECMP Table                                         */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00900000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(23, 4),SMEM_BIND_TABLE_MAC(eqL2Ecmp)}
            /*ePort/Tx Protection Switching Table                          */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00A00000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(txProtectionSwitchingTable)}
            /*ePort/ePort to LOC Mapping Table                             */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00A80000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(ePortToLocMappingTable)}
            /*ePort/Protection LOC RX Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00B00000, 128),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamProtectionLocStatusTable)}
            /*ePort/Protection LOC TX Table                                */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00B10000, 128),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(oamTxProtectionLocStatusTable)}
            /*ePort/E2PHY Table                                            */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00c00000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24, 4),SMEM_BIND_TABLE_MAC(eqE2Phy)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
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
             {DUMMY_NAME_PTR_CNS,         0x0000001c,         0x00000498,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00000074,         0x0000003f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005000,         0x00000004,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005020,         0x00000020,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005024,         0xffffffff,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005028,         0x000001e0,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000502c,         0x0001ffff,      4,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000503c,         0x00002190,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005044,         0x0007ffff,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005050,         0x00000002,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00005058,         0x0000003f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a000,         0x3ef084e2,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a010,         0x00000160,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a020,         0x000007e0,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000a100,         0xffffffff,     32,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af00,         0x1084211f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af04,         0x00004201,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af08,         0x00000011,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af10,         0x0000421f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000af30,         0x0000ffff,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b000,         0x00004e01,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000b00c,         0x00000001,      7,    0x4,      2,    0x40}
            ,{DUMMY_NAME_PTR_CNS,         0x0000b200,         0x02000000,      7,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d000,         0x00000020,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d004,         0xffffffff,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x0000d010,         0x000001e0,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x00020010,         0x0000003f,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,         0x00080000,         0x0000ffff,    255,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x00100000,         0xffffffff,    256,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,         0x00130000,         0x0fffffff,    256,    0x4  }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitTti function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TTI unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitTti
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000001BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001E8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000218)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000340)}

            /*DSCP To DSCP Map - one entry per translation profile
              Each entry holds 64 DSCP values, one per original DSCP value. */
#define  lion3_dscpToDscpMap_size  64
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384 , lion3_dscpToDscpMap_size),SMEM_BIND_TABLE_MAC(dscpToDscpMap)}
            /*EXP To QosProfile Map - Each entry represents Qos translation profile
              Each entry holds QoS profile for each of 8 EXP values*/
#define  lion3_expToQoSProfile_size  16
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000700, 192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80, lion3_expToQoSProfile_size),SMEM_BIND_TABLE_MAC(expToQoSProfile)}
            /*DSCP To QosProfile Map - holds 12 profiles, each defining a 10bit QoSProfile per each of the 64 DSCP values : 0x400 + 0x40*p: where p (0-11) represents profile*/
#define  lion3_dscpToQoSProfile_size  128
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 1536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(640 , lion3_dscpToQoSProfile_size),SMEM_BIND_TABLE_MAC(dscpToQoSProfile)}
            /*UP_CFI To QosProfile Map - Each entry holds a QoS profile per each value of {CFI,UP[2:0]} */
#define  lion3_upToQoSProfile_size  32
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000E00, 384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, lion3_upToQoSProfile_size),SMEM_BIND_TABLE_MAC(upToQoSProfile)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013B0, 0x000013DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000013F8, 0x00001424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001430, 0x000014CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000300C, 0x0000300C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x0000307C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003090, 0x000030AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x000065FC)}

            /* Default Port Protocol eVID and QoS Configuration Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(portProtocolVidQoSConf)}
            /* PCL User Defined Bytes Configuration Memory -- 70 udb's in each entry */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(840, 128),SMEM_BIND_TABLE_MAC(ipclUserDefinedBytesConf)}
            /* TTI User Defined Bytes Configuration Memory -- TTI keys based on UDB's : 8 entries support 8 keys*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 1280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(360, 64),SMEM_BIND_TABLE_MAC(ttiUserDefinedBytesConf)}
            /* VLAN Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13, 4),SMEM_BIND_TABLE_MAC(ingressVlanTranslation)}
            /* Port to Queue Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(ttiPort2QueueTranslation)}
            /* (new table in sip6) vrf_id eVlan Mapping Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ttiVrfidEvlanMapping)}
             /*Physical Port Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(58, 8),SMEM_BIND_TABLE_MAC(ttiPhysicalPortAttribute)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110000, 0x00110480)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110500, 0x00110500)}
             /*Physical Port Attributes 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(251, 32),SMEM_BIND_TABLE_MAC(ttiPhysicalPort2Attribute)}
             /* QCN to Pause Timer Map*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(ttiQcnToPauseTimerMap)}
            /*Default ePort Attributes (pre-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00210000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(126, 16),SMEM_BIND_TABLE_MAC(ttiPreTtiLookupIngressEPort)}
            /*ePort Attributes (post-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00240000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(ttiPostTtiLookupIngressEPort)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

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
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x30002503,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000020,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x65586558,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x0000000e,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000024,         0x1001ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000028,         0x000088e7,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x88488847,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x65586558,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000038,         0x00003232,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000003c,         0x0000000d,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0xff000000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000004c,         0xff020000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0xff000000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000054,         0xffffffff,      3,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000060,         0x00001800,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068,         0x1b6db81b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000006c,         0x00000007,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x00008906,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x000fff00,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000088,         0x3fffffff,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x000000f0,         0xffffffff,      4,    0x4,      2,    0x10}
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x0000004b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x00001320,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x0000001b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x20a6c01b,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x24924924,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x00ffffff,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x0000311f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0fffffff,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000188,         0x0a00003c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000018c,         0x00000030,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x000001e4,         0x00000002,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x000001e8,         0x00000104,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000200,         0x030022f3,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000204,         0x00400000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x12492492,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00000092,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0x0180c200,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000300,         0x81008100,      4,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000310,         0xffffffff,      2,    0x8    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000340,         0x0000000f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x000013bc,         0x0000003f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x000013f8,         0x0000ffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x000013fc,         0x000001da,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001500,         0x00602492,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0x00000fff,    128,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00001608,         0xffff1fff,    128,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000160c,         0xffffffff,    128,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x88f788f7,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x013f013f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000300c,         0x00000570,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00003094,         0x88b588b5,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00110004,         0x00008000,    288,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00110500,         0x88a8893f,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
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
* @internal smemFalconUnitIpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitIpcl
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
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000044, 0x00000048)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000005C, 0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000070, 0x00000078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000080, 0x000000FC)} /* SIP-6: Added 16 entries (64 B) */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000100, 4)}          /* SIP-6: Added */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000200, 4*128)}      /* SIP-6: Added */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000005C0, 0x000005FC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(pearsonHash)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000730, 0x0000073C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000744, 0x0000077C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000800, 0x00000850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000C00, 512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74, 16),SMEM_BIND_TABLE_MAC(crcHashMask)}
            /* SIP-6: Added: */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 0x20*16),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, 32),SMEM_BIND_TABLE_MAC(crcHashMode)}
            /* next are set below as formula of 2 tables
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 20480)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 20480)}*/

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00042000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(506, 64),SMEM_BIND_TABLE_MAC(ipcl1UdbSelect)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(506, 64),SMEM_BIND_TABLE_MAC(ipcl2UdbSelect)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IPCL1,IPCL2 Configuration Table */   /* SIP-6: Removed IPCL0 */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000, 20480), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(30, 4),SMEM_BIND_TABLE_MAC(pclConfig)}, FORMULA_SINGLE_PARAMETER(2 , 0x00010000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;
        GT_U32  numMoreEntries = 1024;  /* Falcon */

        /* number of entries : 4K + numMoreEntries . keep alignment and use for memory size */
        chunksMem[0].memChunkBasic.numOfRegisters = ((4*1024) + numMoreEntries) * (chunksMem[0].memChunkBasic.enrtyNumBytesAlignement / 4);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00010001,      1,    0x0      }  /* SIP-6: Default value change */
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x02801000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000028,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x00000fff,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x3fffffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000730,         0x00ff0080,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000738,         0x00080008,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };



        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconUnitEpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEpcl
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000110)}  /* New in SIP-6 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x0000013C)}  /* New in SIP-6 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000160)}  /* New in SIP-6 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000164, 0x00000168)}  /* New in SIP-6 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000254)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000003FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 6144) ,SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(306, 64),SMEM_BIND_TABLE_MAC(epclUdbSelect)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 4*5120),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(30, 4),SMEM_BIND_TABLE_MAC(epclConfigTable)}
            /* New tables in SIP-6: */
            ,{
                SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00016000, 512),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4),
                SMEM_BIND_TABLE_MAC(epclExactMatchProfileIdMapping)
             }
            ,{
                SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00014000, 8192),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),
                SMEM_BIND_TABLE_MAC(epclPortLatencyMonitoring)
             }
            ,{
                SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00013000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),
                SMEM_BIND_TABLE_MAC(epclSourcePhysicalPortMapping)
             }
            ,{
                SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),
                SMEM_BIND_TABLE_MAC(epclTargetPhysicalPortMapping)
             }
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000115,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00FF0080,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000808,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x00000042,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000074,         0x76543210,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000A0,         0x0000002D,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000C0,         0x0000ffff,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0xFFFFFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0xFFFFFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0xFFFFFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010C,         0xFFFFFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0xFFFFFFFF,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000220,         0x00000008,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };



        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitL2i function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the L2i unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitL2i
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000048)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x0000014C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000164)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000170, 0x00000170)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x0000019C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000071C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000087C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000093C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B7C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D1C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000112C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000120C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001220, 0x00001224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001240, 0x000012DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x0000140C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001508)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001510, 0x00001514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001520, 0x00001528)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001530, 0x00001544)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001550, 0x00001564)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001600, 0x0000167C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002010, 0x00002010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002020, 0x00002020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002120, 0x00002120)}

            /* Source Trunk Attribute Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00202000 ,4096 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressTrunk)}
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
            /* IEEE Reserved Multicast Configuration register */ /* register and not table SMEM_BIND_TABLE_MAC(ieeeRsrvMcConfTable) */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000200 ,0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ieeeRsrvMcConfTable)}, FORMULA_TWO_PARAMETERS(16 , 0x4 , 8 ,0x80)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000600 ,0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(ieeeRsrvMcCpuIndex)}, FORMULA_SINGLE_PARAMETER(16 , 0x4)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    /* to support flex params :
       add tables that depend on number of vlans and number of eport and number
       of physical ports */
    {
        GT_U32  numPhyPorts = devObjPtr->limitedResources.phyPort;
        GT_U32  numEPorts = devObjPtr->limitedResources.ePort;
        GT_U32  numEVlans = devObjPtr->limitedResources.eVid;
        GT_U32  numStg = devObjPtr->limitedResources.stgId;



        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Ingress Bridge physical Port Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000 ,INIT_INI_RUNTIME ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortEntry)}
                /*Ingress Bridge physical Port Rate Limit Counters Table*/
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00201000 ,INIT_INI_RUNTIME ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(22, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortRateLimitCountersEntry)}
            };

            /* number of entries : numPhyPorts . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = numPhyPorts * (chunksMem[0].enrtyNumBytesAlignement / 4);
            /* number of entries : numPhyPorts . keep alignment and use for memory size*/
            chunksMem[1].numOfRegisters = numPhyPorts * (chunksMem[1].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* ingress EPort learn prio Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00210000 ,INIT_INI_RUNTIME ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressEPortLearnPrio)}
            };

            /* number of entries : numEPorts/8 . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numEPorts / 8) * (chunksMem[0].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Spanning Tree State Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00240000 , INIT_INI_RUNTIME ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(256, 32),SMEM_BIND_TABLE_MAC(stp)}
            };
            GT_U32  numLines = numStg;

            /* each entry : 2 bits per physical port */
            chunksMem[0].enrtySizeBits = 2 * 128;


            /* alignment is according to entry size */
            chunksMem[0].enrtyNumBytesAlignement = ((chunksMem[0].enrtySizeBits + 31)/32)*4;
            /* number of entries : numStg . use calculated alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numLines) * (chunksMem[0].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }
        else
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Spanning Tree State Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00240000 , INIT_INI_RUNTIME ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(2048, 256),SMEM_BIND_TABLE_MAC(stp)}
            };
            GT_U32  numLines =
                (devObjPtr->support_remotePhysicalPortsTableMode) ?
                    numStg / 8 :  /* there are 8 'entries' per line */
                    numStg;

            /* each entry : 2 bits per physical port */
            chunksMem[0].enrtySizeBits = 2 * _1K; /* 2K bits per entry regardless to number of physical ports ! */


            /* alignment is according to entry size */
            chunksMem[0].enrtyNumBytesAlignement = ((chunksMem[0].enrtySizeBits + 31)/32)*4;
            /* number of entries : numStg . use calculated alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numLines) * (chunksMem[0].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Port Membership Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340000 , INIT_INI_RUNTIME), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(1024 , 128),SMEM_BIND_TABLE_MAC(bridgeIngressPortMembership)}
            };

            GT_U32  numLines = 1024; /* table support 8K eVLANs for 128 port mode regardless of number supported eVLANs */
            /* each entry : 1 bit per physical port */
            chunksMem[0].enrtySizeBits = _1K; /* 1K bits per entry regardless to number of physical ports ! */
            /* alignment is according to entry size */
            chunksMem[0].enrtyNumBytesAlignement = ((chunksMem[0].enrtySizeBits + 31)/32)*4;
            /* number of entries : numEVlans . use calculated alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numLines ) * (chunksMem[0].enrtyNumBytesAlignement / 4) ;

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /* Ingress Vlan Table */
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00440000 , INIT_INI_RUNTIME), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(102 , 16),SMEM_BIND_TABLE_MAC(vlan)}
            };

            /* number of entries : numEVlans . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numEVlans ) * (chunksMem[0].enrtyNumBytesAlignement / 4) ;

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*Bridge Ingress ePort Table*/
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00500000 ,INIT_INI_RUNTIME), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80 ,16),SMEM_BIND_TABLE_MAC(bridgeIngressEPort)}
                /* Ingress Span State Group Index Table */
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00218000 , INIT_INI_RUNTIME), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12 , 4),SMEM_BIND_TABLE_MAC(ingressSpanStateGroupIndex)}
            };
            /* number of entries : numEPorts . keep alignment and use for memory size */
            chunksMem[0].numOfRegisters = (numEPorts ) * (chunksMem[0].enrtyNumBytesAlignement / 4) ;
            /* number of entries : numEVlans . keep alignment and use for memory size */
            chunksMem[1].numOfRegisters = (numEVlans ) * (chunksMem[1].enrtyNumBytesAlignement / 4) ;
            /* number of bits : stgId */
            chunksMem[1].enrtySizeBits = devObjPtr->flexFieldNumBitsSupport.stgId;
            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }

    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x22806004,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00002240,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00000fff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x22023924,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x001f803f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x52103210,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000130,         0x00001ffe,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x05f205f2,      4,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000160,         0x0001ffff,      2,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001200,         0x04d85f41,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001204,         0x18027027,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001208,         0x02002019,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000120c,         0x00ffffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x000012c0,         0xffffffff,      8,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0xffffffff,     32,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x0000007f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002010,         0x0000ffff,      1,    0x0     }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            myUnit_registersDefaultValueArr[0].value = 0x12806004;/* address 0x00000000 */
            myUnit_registersDefaultValueArr[2].value = 0x0000ffff;/* address 0x00000008 */
        }
        else
        {
            myUnit_registersDefaultValueArr[0].value = 0x22806004;/* address 0x00000000 */
            myUnit_registersDefaultValueArr[2].value = 0x00000fff;/* address 0x00000008 */
        }


    }
}


/**
* @internal smemFalconUnitShm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Falcon SHM unit
*
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemFalconUnitShm
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
             {DUMMY_NAME_PTR_CNS,            0x000000a0,         0x00011111,      1,    0x0  }
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
* @internal smemFalconUnitFdb function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Falcon FDB unit
*         (without the actual FDB table)
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitFdb
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*FDB*/
    /* Message to CPU register  */
    {0x00000090, SMEM_FULL_MASK_CNS, smemChtActiveReadMsg2Cpu, 0 , NULL,0},
    /* Mac Table Access Control Register */
    {0x00000130, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteFdbMsg,0},

    /* Message From CPU Management */
    {0x000000c0, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteNewFdbMsgFromCpu, 0},

    /* MAC Table Action general Register */
    {0x00000020, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteFdbActionTrigger, 0},
    /* FDB Global Configuration 1 register */
    {0x00000004, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteFDBGlobalCfg1Reg, 0},
    /* FDB Global Configuration 2 register */
    {0x00000008, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteFdbGlobalCfg2Reg, 0},

    {0x000001b0, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    /* MAC Table Interrupt Mask Register */
    {0x000001b4, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteMacInterruptsMaskReg, 0},

    /* FDB Counters Update Control */
    {0x00000340, SMEM_FULL_MASK_CNS, NULL, 0, smemLion3ActiveWriteFDBCountersUpdateControlReg,0},
    /* FDB Counters Update Control */
    {0x00000280, SMEM_FULL_MASK_CNS, NULL, 0, smemLion3ActiveWriteFDBCountersControlReg,0},

    /* active memory for registers on sip 6.30 */
    /* register <HSR PRP Timer> */
    {0x00000508, SMEM_FULL_MASK_CNS, smemSip6_30ActiveReadFdbHsrPrpTimerReg , 0, smemSip6_30ActiveWriteFdbHsrPrpTimerReg,0},
    {0x00000500, SMEM_FULL_MASK_CNS, NULL , 0, smemSip6_30ActiveWriteFdbHsrPrpGlobalConfigReg ,0},

    /* ROC counters :
       0x510 , 0x514 , 0x518 : each 10 times insteps of 0xC
       so range is :
       0x510 .. 0x584
    */
    /* 0x510 .. 0x51C */
    {0x00000510, SMEM_FULL_MASK_CNS-0x0F, smemChtActiveReadCntrs , 0, NULL ,0},
    /* 0x520 .. 0x53C */
    {0x00000520, SMEM_FULL_MASK_CNS-0x1F, smemChtActiveReadCntrs , 0, NULL ,0},
    /* 0x540 .. 0x57C */
    {0x00000540, SMEM_FULL_MASK_CNS-0x3F, smemChtActiveReadCntrs , 0, NULL ,0},
    /* 0x580 .. 0x584 */
    {0x00000580, SMEM_FULL_MASK_CNS-0x07, smemChtActiveReadCntrs , 0, NULL ,0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000074)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x00000094)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000D0, 0x000000E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x00000120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000150)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000170, 0x00000174)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001B0, 0x000001B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000280)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000324)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000388)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000410, 0x00000430)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x0000048C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000004A0, 0x000004A0)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x107bfc00,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x0701233c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0xffffffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x000023c3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x000003c1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000074,         0x0036ee80,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0xffffffff,      2,    0x80    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094,         0x0000002a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x0000ffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x00001fff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0xffffffff,      2,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0000ffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000174,         0x007fffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000244,         0x00000001,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000248,         0x00000002,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000024c,         0x00000003,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000250,         0x00000004,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000254,         0x00000005,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000258,         0x00000006,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000025c,         0x00000007,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000260,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000264,         0x00000009,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000268,         0x0000000a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000026c,         0x0000000b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000270,         0x0000000c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000274,         0x0000000d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000278,         0x0000000e,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000027c,         0x0000000f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000380,         0x00000002,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000384,         0x00010162,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000388,         0x00000182,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000400,         0x00000002,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000410,         0x00000007,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000480,         0x000000ec,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000484,         0x00088420,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000488,         0x002bfdfe,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000048c,         0x00000014,      1,    0x0     }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        {
            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                 {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000508)}
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000510, 0x00000584)}
                ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000590, 0x00000594)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }

        {
            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
            {
                 {DUMMY_NAME_PTR_CNS,            0x00000500,         0x00000064,      1,    0x0     }
                ,{DUMMY_NAME_PTR_CNS,            0x00000504,         0x00000019,      1,    0x0     }

                ,{NULL,            0,         0x00000000,      0,    0x0      }
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

            /* set the next !! to point to the additional defaults */
            unitPtr->unitDefaultRegistersPtr->nextPtr = &list;
        }
    }
}


/**
* @internal smemFalconUnitEm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EM
*          unit
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitEm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*EM*/
    /* indirect Access Control Register */
    {0x00000130, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteExactMatchMsg,0},

    /* EM Global Configuration 1 register */
    {0x00000004, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteExactMatchGlobalCfg1Reg, 0},

    /* EM Global Configuration 2 register */
    {0x00000008, SMEM_FULL_MASK_CNS, NULL, 0 , smemFalconActiveWriteExactMatchGlobalCfg2Reg, 0},

    {0x000001b0, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 17, smemChtActiveWriteIntrCauseReg, 0},
    /* MAC Table Interrupt Mask Register */
    {0x000001b4, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteMacInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000000, 0x00000014) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000020, 0x0000003C) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000060, 0x00000060) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000070, 0x00000074) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000090, 0x00000094) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x000000C0, 0x000000C0) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x000000D0, 0x000000E4) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000110, 0x00000120) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000130, 0x00000144) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000150, 0x00000150) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000170, 0x00000174) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x000001B0, 0x000001B4) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000200, 0x00000280) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000300, 0x00000324) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000340, 0x00000340) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000380, 0x00000388) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000400, 0x00000400) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000410, 0x00000430) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x00000480, 0x0000048C) }
            , { SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC(0x000004A0, 0x000004A0) }

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000004,         0x07012b3c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000008,         0x00000008,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x0000000C,         0xffffffff,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000150,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000380,         0x00000002,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,           0x00000384,         0x00010162,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit Exact Match */

        {/*start of emGlobalConfiguration*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emGlobalConfiguration.emGlobalConfiguration1 = 0x0004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emGlobalConfiguration.emGlobalConfiguration2 = 0x0008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emGlobalConfiguration.emCRCHashInitConfiguration = 0x000c;
        }/*end of emGlobalConfiguration*/

        {/*start of emIndirectAccess*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emIndirectAccess.emIndirectAccessControl = 0x0130;
            {/* 0x134 + n*4 */
                GT_U32    n;
                for (n = 0 ; n <= 4 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emIndirectAccess.emIndirectAccessData[n] =
                        0x134+4*n;
                }/* end of loop n */
            }/* 0x134 + n*4 */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emIndirectAccess.emIndirectAccessAccess = 0x0150;
        }/*end of emIndirectAccess*/

        {/*start of emInterrupt*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emInterrupt.emInterruptCauseReg = 0x01b0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emInterrupt.emInterruptMaskReg = 0x01b4;
        }/*end of emInterrupt*/

        {/*start of emHashResults*/
            {/* 0x300 + n*4 */
                GT_U32    n;
                for (n = 0 ; n <= 7 ; n++) {
                     SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emHashResults.emMultiHashCRCResultReg[n] =
                        0x300+n*4 ;
                }/* end of loop n */
            }/* 0x300 + n*4 */
        }/*end of emHashResults*/


        {/*start of emSchedulerConfiguration*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emSchedulerConfiguration.emSchedulerSWRRControl = 0x0380;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EXACT_MATCH.emSchedulerConfiguration.emSchedulerSWRRConfig = 0x0384;
        }/*end of emSchedulerConfiguration*/
    }
}

/**
* @internal smemFalconUnitPreq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PREQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitPreq
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

            /*Fake adress - Need to remove this after CPSS fix interrupt code*/
              ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000610)}
            /* Egress MIB counters*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000720, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x00000734)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000750, 0x00000754)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000760, 0x00000764)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000850)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000860, 0x00000864)}

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
* @internal smemFalconUnitMll function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MLL unit
*/
static void smemFalconUnitMll
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
/*MLL*/
    MLL_ACTIVE_MEM_MAC,

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x0000006C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000001FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000204, 0x00000208)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000308, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000404)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000814)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000900)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000980, 0x00000984)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A80, 0x00000A84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B04, 0x00000B04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C80, 0x00000C84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D80, 0x00000D84)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E20, 0x00000E28)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* support 16K lines (like Aldrin2) */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 65536) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4)   ,SMEM_BIND_TABLE_MAC(l2MllLtt)}
            /* support 16K lines according to IAS (like Aldrin2) */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 524288), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(162, 32) , SMEM_BIND_TABLE_MAC(mll)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        GT_U32  numMllPairs = devObjPtr->limitedResources.mllPairs;
        GT_U32  numL2MllLtt = devObjPtr->limitedResources.l2LttMll;
        /* support Falcon , Hawk , Phoenix */
        chunksMem[0].numOfRegisters = numL2MllLtt * (chunksMem[0].enrtyNumBytesAlignement / 4);
        chunksMem[1].numOfRegisters = numMllPairs * (chunksMem[1].enrtyNumBytesAlignement / 4);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x0000000c,         0x0/*set below*/,1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000000,         0x00200700,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00008888,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00000003,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0x0000073c,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x0000ffff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000054,         0x00000001,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000060,         0x00000fff,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068,         0x0001ffff,      2,    0x4,      2,    0x2a8}
            ,{DUMMY_NAME_PTR_CNS,            0x00000204,         0x08040201,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0x20000000,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000214,         0x00016300,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000218,         0x00000101,      1,    0x0 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000021c,         0x00000600,      2,    0x4 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            /* address 0x0000000c : SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->MLL.MLLGlobalCtrl.MLLLookupTriggerConfig */
            myUnit_registersDefaultValueArr[0].value = 0x00008fff;
        }
        else
        {
            /* address 0x0000000c : SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->MLL.MLLGlobalCtrl.MLLLookupTriggerConfig*/
            myUnit_registersDefaultValueArr[0].value = 0x00002fff;
        }

        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconUnitTai function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TAI unit
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitTai
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* chunks with formulas */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000200)}
            /* TAI SER RX Registers */
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000304, 0x00000344)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Manchester Decoding window control initial values */
             {DUMMY_NAME_PTR_CNS,  0x0000033C,         0x0101000A,      1,       0x0}
            ,{NULL,                0,                  0x00000000,      0,       0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemFalconUnitRavenMgInterruptAddrSet function
* @endinternal
*
* @brief   set register DB for raven MG interrupts per base address.
*
*/
static void smemFalconUnitRavenMgInterruptAddrSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    {/*start of MG unit interrupts */
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.globalInterruptCause             = 0x00300030;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.globalInterruptMask              = 0x00300034;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.mgInternalInterruptCause         = 0x00300038;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.mgInternalInterruptMask          = 0x0030003c;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.portsInterruptCause              = 0x00300080;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.portsInterruptMask               = 0x00300084;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.mg1InternalInterruptCause        = 0x0030009c;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.mg1InternalInterruptMask         = 0x003000A0;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.dfxInterruptCause                = 0x003000AC;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.dfxInterruptMask                 = 0x003000B0;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.ports1InterruptsSummaryCause     = 0x00300150;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.ports1InterruptsSummaryMask      = 0x00300154;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.functionalInterruptsSummaryCause = 0x003003F8;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.functionalInterruptsSummaryMask  = 0x003003FC;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.functional1InterruptsSummaryCause= 0x003003F0;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN[0].globalInterrupt.functional1InterruptsSummaryMask = 0x003003F4;
    }
}

/**
* @internal smemFalconUnitRavenD2dDefaultsBaseAddr function
* @endinternal
*
* @brief   define defualt register for D2D in raven per base
*          address. there are d2d units in one raven
* @param[in] base address                 - base address of the
*       d2d unit in the raven
*
* @param[in,out] unitPtr                  - pointer to the unit
*       chunk
*
* @param[out]                            - pointer to the
*       defaults reisters list
*/
static void smemFalconUnitRavenD2dDAddrSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN  GT_U32                    baseAddr,
    IN  GT_U32                    index
)
{
   /*start of unit raven_d2d_pair[] */
    {/*start of unit mac rx */
       {/*0x00004604+ baseAddr +p*0x4*/
            GT_U32    p;
            for (p = 0 ; p < 40 /* mac rx registers number*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macRx.rxTdmSchdule[p] =
                    0x00004604 + baseAddr + p*0x4;
            }/* end if loop p*/
       } /*0x00004604+ baseAddr+p*0x4*/

       {/*0x000047E4+ baseAddr+p*0x4*/
            GT_U32    p;
            for (p = 0 ; p < 3 /* mac rx select bandwidth registers number*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macRx.rxFifoBandwidthSelect[p] =
                    0x000047E4  + baseAddr + p*0x4;
            }/* end if loop p*/
       } /*0x000047E4+ baseAddr+p*0x4*/

       {/*0x00004804+ baseAddr+p*0x4*/
            GT_U32    p;
            for (p = 0 ; p < 18 /* mac rx channels number*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macRx.rxChannel[p] =
                    0x00004804  + baseAddr + p*0x4;
            }/* end if loop p*/
       } /*0x00004804+ baseAddr+p*0x4*/

       {/*0x00004E04+p*0x4*/
            GT_U32    p;
            for (p = 0 ; p < 18 /* mac rx channels number*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macRx.rxChannel2[p] =
                    0x00004E04  + baseAddr + p*0x4;
            }/* end if loop p*/
       } /*0x00004E04+ baseAddr+p*0x4*/

       {/*0x00004904+ baseAddr+p*0x4*/
            GT_U32    p;
            for (p = 0 ; p < 34 /* number of rx fifos segments*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macRx.rxFifoSegPtr[p] =
                    0x00004904  + baseAddr + p*0x4;
            }/* end if loop p*/
        } /*0x00004904+ baseAddr+p*0x4*/

    }/*end of unit  mac rx */

    {/*start of unit macTx */
        {/*0x00004004 + baseAddr + p*0x4*/
            GT_U32    p;
            for(p = 0 ; p < 18; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macTx.txChannel[p] =
                    0x00004004  + baseAddr+ p*0x4;
                }/* end of loop p */
        }/*0x00004004 + baseAddr+ p*0x4*/

        {/*0x00004104 + baseAddr + p*0x4*/
            GT_U32    p;
            for(p = 0 ; p < 32 /* number of tx fifos segments*/; p++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].macTx.txFifoSegPtr[p] =
                    0x00004104  + baseAddr + p*0x4;
                }/* end of loop p */
        }/*0x00004104 + baseAddr+ p*0x4*/

    }/*end of unit  mac tx */

    {/*start of unit pcs */
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pcs.globalControl = 0x00007000  + baseAddr;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pcs.transmitCalCtrl = 0x00007004  + baseAddr;
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pcs.receiveCalCtrl = 0x00007008  + baseAddr;

         {/*0x00007200 + baseAddr+ p*0x4*/
             GT_U32    p;
             for(p = 0 ; p < 32 /* number of pcs calendar slices*/; p++) {
                 SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pcs.transmitCalSlotConfig[p] =
                     0x00007200  + baseAddr + p*0x4;
                 }/* end of loop p */
         }/*0x00007200+ baseAddr + p*0x4*/

         {/*0x00007400 + baseAddr+ p*0x4*/
             GT_U32    p;
             for(p = 0 ; p < 32 /* number of pcs calendar slices*/; p++) {
                 SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pcs.receiveCalSlotConfig[p] =
                     0x00007400  + baseAddr + p*0x4;
                 }/* end of loop p */
         }/*0x00007400 + baseAddr+ p*0x4*/
    }/*end of unit  pcs */

    {/*start of unit pma */
         SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair[0].d2d[index].pma.pmaOperationControl = 0x00008008  + baseAddr;
    }/*end of unit  pma */



/*end of unit eagle_d2d[] */
}

/**
* @internal smemFalconUnitRavenD2dDefaultsBaseAddr function
* @endinternal
*
* @brief   define defualt register for D2D in raven per base
*          address. there are d2d units in one raven
* @param[in] base address                 - base address of the
*       d2d unit in the raven
*
* @param[in,out] unitPtr                  - pointer to the unit
*       chunk
*
* @param[out]                            - pointer to the
*       defaults reisters list
*/
static void smemFalconUnitRavenD2dDefaultsBaseAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN  GT_U32                    baseAddr,
    OUT SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC *listPtr
)
{
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
           {DUMMY_NAME_PTR_CNS,            0x00004000,         0x00020008,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0xe000000c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0xe0000c0c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0xe000180c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004010,         0xe000240c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004014,         0xe000300c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004018,         0xe0003c0c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000401c,         0xe000480c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004020,         0xe000540c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004024,         0x6000600c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004028,         0x60006c0c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000402c,         0x6000780c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004030,         0x6000840c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004034,         0x6000900c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004038,         0x60009c0c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000403c,         0x6000a80c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004040,         0x6000b40c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004044,         0xe0006004,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004048,         0xe0006c04,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004104,         0x00000001,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004108,         0x00000002,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004110,         0x00000004,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004114,         0x00000005,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004118,         0x00000003,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000411c,         0x00000007,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004120,         0x00000008,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004124,         0x00000006,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004128,         0x0000000a,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000412c,         0x0000000b,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004130,         0x00000009,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004134,         0x0000000d,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004138,         0x0000000e,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000413c,         0x0000000c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004140,         0x00000010,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004144,         0x00000011,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004148,         0x0000000f,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000414c,         0x00000013,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004150,         0x00000014,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004154,         0x00000012,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004158,         0x00000016,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000415c,         0x00000017,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004160,         0x00000015,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004164,         0x00000018,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004168,         0x00000019,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000416c,         0x00000017,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004170,         0x0000001b,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004174,         0x0000001c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004178,         0x0000001a,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000417c,         0x0000001e,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004180,         0x0000001f,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004604,         0x86848280,      2,    0x8}
          ,{DUMMY_NAME_PTR_CNS,            0x00004608,         0x87858381,     10,    0x8}
          ,{DUMMY_NAME_PTR_CNS,            0x00004614,         0x86848290,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000461c,         0x86848280,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004624,         0x86849080,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000462c,         0x86848280,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004634,         0x86908280,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000463c,         0x86848280,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004644,         0x90848280,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000464c,         0x86848280,     10,    0x8}
          ,{DUMMY_NAME_PTR_CNS,            0x00004658,         0x87858390,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004660,         0x87858381,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004668,         0x87859081,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004670,         0x87858381,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004678,         0x87908381,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004680,         0x87858381,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004688,         0x90858381,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004690,         0x87858381,      3,    0x8}
          ,{DUMMY_NAME_PTR_CNS,            0x0000469c,         0x86848290,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004804,         0xd0380000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004808,         0xd0381000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000480c,         0xd0382000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004810,         0xd0383000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004814,         0xd0384000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004818,         0xd0385000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000481c,         0xd0386000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004820,         0xd0387000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004824,         0x501c0000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004828,         0x501c1000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000482c,         0x501c2000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004830,         0x501c3000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004834,         0x501c4000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004838,         0x501c5000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000483c,         0x501c6000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004840,         0x501c7000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004844,         0xd01c8000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004848,         0xd01c9000,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004904,         0x00000001,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004908,         0x00000002,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000490c,         0x00000003,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004914,         0x00000005,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004918,         0x00000006,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000491c,         0x00000007,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004920,         0x00000004,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004924,         0x00000009,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004928,         0x0000000a,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000492c,         0x0000000b,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004930,         0x00000008,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004934,         0x0000000d,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004938,         0x0000000e,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000493c,         0x0000000f,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004940,         0x0000000c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004944,         0x00000011,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004948,         0x00000012,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000494c,         0x00000013,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004950,         0x00000010,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004954,         0x00000015,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004958,         0x00000016,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000495c,         0x00000017,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004960,         0x00000014,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004964,         0x00000019,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004968,         0x0000001a,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000496c,         0x0000001b,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004970,         0x00000018,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004974,         0x0000001d,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004978,         0x0000001e,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x0000497c,         0x0000001f,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004980,         0x0000001c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004984,         0x00000021,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004988,         0x00000020,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00004e04,         0x00401406,      8,    0x4}
          ,{DUMMY_NAME_PTR_CNS,            0x00004e24,         0x00200c0a,     10,    0x4}
          ,{DUMMY_NAME_PTR_CNS,            0x00007000,         0x000d003c,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x01001654,      2,    0x4}
          ,{DUMMY_NAME_PTR_CNS,            0x0000700c,         0x1eb4ccb0,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007078,         0x0000ffff,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007200,         0x00182100,      2,    0x8,      2,    0x44  }
          ,{DUMMY_NAME_PTR_CNS,            0x00007204,         0x001c3141,      2,    0x8,      2,    0x44  }
          ,{DUMMY_NAME_PTR_CNS,            0x00007210,         0x00084010,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007214,         0x000c5046,      2,    0x8,      2,    0x44  }
          ,{DUMMY_NAME_PTR_CNS,            0x00007218,         0x00084007,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007220,         0x00100407,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007224,         0x00141182,      2,    0x8,      2,    0x44  }
          ,{DUMMY_NAME_PTR_CNS,            0x00007228,         0x001001c3,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007230,         0x000101c3,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007234,         0x00046084,      2,    0x8,      2,    0x200 }
          ,{DUMMY_NAME_PTR_CNS,            0x00007238,         0x000070c5,      2,    0x44,      2,    0x200}
          ,{DUMMY_NAME_PTR_CNS,            0x00007240,         0x004070c5,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007278,         0x00046084,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007400,         0x00182100,      2,    0x8,      2,    0x44}
          ,{DUMMY_NAME_PTR_CNS,            0x00007404,         0x001c3141,      2,    0x8,      2,    0x44}
          ,{DUMMY_NAME_PTR_CNS,            0x00007414,         0x000c5046,      2,    0x8,      2,    0x44}
          ,{DUMMY_NAME_PTR_CNS,            0x00007424,         0x00141182,      2,    0x8,      2,    0x44}
          ,{DUMMY_NAME_PTR_CNS,            0x00007440,         0x004070c5,      1,    0x0}
          ,{DUMMY_NAME_PTR_CNS,            0x00007478,         0x00046084,      1,    0x0}

          ,{NULL,            0x00000000,         0x00000000,      0,    0x0}

       };

       GT_U32  numOfDefualtsReg = (sizeof(myUnit_registersDefaultValueArr)/sizeof(SMEM_REGISTER_DEFAULT_VALUE_STC));
       GT_U32  ii;
       SMEM_REGISTER_DEFAULT_VALUE_STC *tempRegistersDefaultValueArr = (SMEM_REGISTER_DEFAULT_VALUE_STC*)smemDeviceObjMemoryAlloc(devObjPtr,numOfDefualtsReg,sizeof(SMEM_REGISTER_DEFAULT_VALUE_STC));
       if(tempRegistersDefaultValueArr == NULL)
       {
           skernelFatalError(" allocation of memory failed");
           return;
       }

       for (ii = 0; ii < numOfDefualtsReg; ii++)
       {
            tempRegistersDefaultValueArr[ii].unitNameStrPtr =  myUnit_registersDefaultValueArr[ii].unitNameStrPtr;
            tempRegistersDefaultValueArr[ii].address = (myUnit_registersDefaultValueArr[ii].address + baseAddr);
            tempRegistersDefaultValueArr[ii].value =  myUnit_registersDefaultValueArr[ii].value;
            tempRegistersDefaultValueArr[ii].numOfRepetitions =  myUnit_registersDefaultValueArr[ii].numOfRepetitions;
            tempRegistersDefaultValueArr[ii].stepSizeBeteenRepetitions =  myUnit_registersDefaultValueArr[ii].stepSizeBeteenRepetitions;
            tempRegistersDefaultValueArr[ii].numOfRepetitions_2 =  myUnit_registersDefaultValueArr[ii].numOfRepetitions_2;
            tempRegistersDefaultValueArr[ii].stepSizeBeteenRepetitions_2 =  myUnit_registersDefaultValueArr[ii].stepSizeBeteenRepetitions_2;
       }

       listPtr->currDefaultValuesArrPtr = tempRegistersDefaultValueArr;
       listPtr->nextPtr = NULL;
    }
}

/**
* @internal smemFalconActiveReadLatencyProfileStatTable function
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
void smemFalconActiveReadLatencyProfileStatTable
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
    GT_U32  ravenNum;
    GT_U32  lmuNum;
    GT_U32  index;

    /* Check raven address space */
    ravenNum = (address >> 24) & 0x3;

    /* Calculate LMU and index */
    lmuNum = ((address & 0x000F0000) == 0x00030000) ? 0 : 1;
    index = (address >> 5) & 0x1FF;

    regAddr = SMEM_SIP6_LATENCY_MONITORING_STAT_TBL_MEM(devObjPtr, ravenNum, lmuNum, index);

    memPtr = smemMemGet(devObjPtr, regAddr);

    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 0),
        0, 32, snetFieldValueGet(memPtr, 0, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 1),
        0, 32, snetFieldValueGet(memPtr, 32, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 2),
        0, 32, snetFieldValueGet(memPtr, 64, 32));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 3),
        0, 32, snetFieldValueGet(memPtr, 96, 30));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 4),
        0, 32, snetFieldValueGet(memPtr, 126, 30));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_LMU_PROFILE_STATISTICS_READ_DATA_REG(devObjPtr, ravenNum, lmuNum, 5),
        0, 32, snetFieldValueGet(memPtr, 156, 30));
}

/**
* @internal smemFalconActiveWrite1IntrCauseReg function
* @endinternal
*
* @brief   Write interrupts RW1C cause registers by read/write mask.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] writeMask                - 32 bits mask of writable bits.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemFalconActiveWrite1IntrCauseReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR writeMask,
    IN         GT_U32 * inMemPtr
)
{
    writeMask &= (*memPtr);
    *memPtr = 0;
    smemChtActiveWriteIntrCauseReg(devObjPtr, address, memSize, memPtr,
        writeMask, inMemPtr);
}

/**
* @internal smemFalconActiveWriteLatencyOverThreshIntrCauseReg function
* @endinternal
*
* @brief   Write interrupts RW1C lstency over threshold interrupt cause
*          registers by read/write mask.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] writeMask                - 32 bits mask of writable bits.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemFalconActiveWriteLatencyOverThreshIntrCauseReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR writeMask,
    IN         GT_U32 * inMemPtr
)
{
    if((*memPtr) & writeMask & 0x1)
    {
        return;
    }

    smemFalconActiveWrite1IntrCauseReg(devObjPtr, address, memSize, memPtr,
        writeMask, inMemPtr);
}

/*******************************************************************************
*  smemFalconExtMtiActiveMemAddressFind
*
* DESCRIPTION:
*      Definition of the extended active memory GET function.
* INPUTS:
*       deviceObjPtr    - device object PTR.
*       address         - address to find in register's DB
*       param           - specific parameter
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE     - the address found in register DB
*       GT_FALSE    - the address not found in register DB
*
* COMMENTS:
*
*******************************************************************************/
static GT_BOOL smemFalconExtMtiActiveMemAddressFind
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 address,
    IN  GT_UINTPTR param
)
{
    SMEM_ACTIVE_MEM_ENTRY_STC  * activeMemoryPtr = (SMEM_ACTIVE_MEM_ENTRY_STC *)param;
    MTI_ACTIVE_MEM_REG_ENT activeMemType;
    GT_U32 mask;
    SMEM_EXT_ACTIVE_MEM_ENTRY_STC extMemAddressInfo = {0};
    SMEM_EXT_ACTIVE_MEM_ENTRY_STC *extMemPtr = &extMemAddressInfo;
    GT_U32 curAddr;
    GT_U32 i, j;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        /* Hawk need to use smemGopPortByAddrGet(...) */
        skernelFatalError("smemFalconExtMtiActiveMemAddressFind : function only for Falcon with Ravens (not for Hawk) \n");
        return 0;
    }

    activeMemType = (activeMemoryPtr->writeFun) ?
        (MTI_ACTIVE_MEM_REG_ENT)activeMemoryPtr->writeFunParam :
        (MTI_ACTIVE_MEM_REG_ENT)activeMemoryPtr->readFunParam;

    mask = activeMemoryPtr->mask;

    /* Size of address offset in words */
    extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0]) / 4);
    /* Number of addresses */
    extMemPtr->addressNum = devObjPtr->portsNumber;
    /* At least one register */
    extMemPtr->addressNum1 = 1;

    switch (activeMemType)
    {
        case MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E:
            if (mask == MTI_PCS_400_E/*0x00460000*/)
            {
                extMemPtr->addressPtr =
                    &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[0].PCS_400G_200G.control1;
                extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[0]) / 4);

                /* need to support 0x00460000..0x00461000 (steps of 0x1000) (2 ports) */
                mask = 0x00FFFFFF - 0x1000;
                mask -= GOP_CHANNEL_OFFSET;/* support '2 channels' */
            }
            else
            if (mask == MTI_PCS_50_E/*0x00465000*/)
            {
                extMemPtr->addressPtr =
                    &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[1].MTI_WRAPPER.PCS_10254050.control1;
                extMemPtr->addressNum -= 1;/* start from port 1 because port 0 info is 0xFFFFFFFF */
                extMemPtr->addressOffset *= 2;/* jump 2 ports */
                extMemPtr->addressNum    /= 2;/* cut by half the number of iterations */

                /* need to support 0x00465000..0x0046b000 (steps of 0x2000) (4 ports) */
                mask = 0x00FFFFFF - (0x7 * 0x2000);
                mask -= GOP_CHANNEL_OFFSET;/* support '2 channels' */
            }
            else
            if (mask == MTI_PCS_100_E/*0x00464000*/)
            {
                extMemPtr->addressPtr =
                    &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_WRAPPER.PCS_10TO100.control1;
                extMemPtr->addressOffset *= 2;/* jump 2 ports */
                extMemPtr->addressNum    /= 2;/* cut by half the number of iterations */
                /* need to support 0x00464000..0x0046b000 (steps of 0x2000) (4 ports) */
                mask = 0x00FFFFFF - (0x7 * 0x2000);
                mask -= GOP_CHANNEL_OFFSET;/* support '2 channels' */
            }
            else
            if (mask == MTI_PCS_CPU_E/*0x00519000*/)
            {
                extMemPtr->addressPtr =
                    &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[0].MTI_cpuPcs.control1;
                extMemPtr->addressNum = SIM_MAX_RAVEN_CNS;
                extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[0]) / 4);
                mask = 0x00FFFFFF;/* exact match */
            }
            else
            {
                return GT_FALSE;
            }

            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_MASK_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.portInterruptMask;
            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_MASK_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.segPortInterruptMask;
            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_CAUSE_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.portInterruptCause;
            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_CAUSE_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.segPortInterruptCause;
            break;
        case MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_MASK_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecCeInterruptMask;
            extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0]) / 4);
            extMemPtr->addressNum = 2 * SIM_MAX_RAVEN_CNS;
            break;
        case MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_MASK_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecNceInterruptMask;
            extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0]) / 4);
            extMemPtr->addressNum = 2 * SIM_MAX_RAVEN_CNS;
            break;
        case MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_CAUSE_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecCeInterruptCause;
            extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0]) / 4);
            extMemPtr->addressNum = 2 * SIM_MAX_RAVEN_CNS;
            break;
        case MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_CAUSE_E:
            extMemPtr->addressPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecNceInterruptCause;
            extMemPtr->addressOffset = (sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0]) / 4);
            extMemPtr->addressNum = 2 * SIM_MAX_RAVEN_CNS;
            break;
        default:
            skernelFatalError("smemFalconExtMtiActiveMemAddressFind : unknown MTI type[%d]\n", activeMemType);
    }

    if ((address & mask) != (extMemPtr->addressPtr[0] & mask))
    {
        /* Address not matches current DB range. Skip to next entry */
        return GT_FALSE;
    }

    for (i = 0; i < extMemPtr->addressNum; i++)
    {
        for (j = 0; j < extMemPtr->addressNum1; j++)
        {
            /* Get current address from register DB */
            curAddr = *(extMemPtr->addressPtr + (i * extMemPtr->addressOffset) + (j * extMemPtr->addressOffset1));
            if (address == curAddr)
            {
                /* Match found */
                return GT_TRUE;
            }
        }
    }

    return GT_FALSE;
}

/**
* @internal smemFalconUnitRaven function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TAI unit
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitRaven
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1
           SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1 */
        {0, MTI_PCS_50_E , NULL, 0, smemFalconActiveWritePcsControl1, MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E, smemFalconExtMtiActiveMemAddressFind},
        {0, MTI_PCS_100_E, NULL, 0, smemFalconActiveWritePcsControl1, MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E, smemFalconExtMtiActiveMemAddressFind},
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.control1 */
        {0, MTI_PCS_400_E, NULL, 0, smemFalconActiveWritePcsControl1, MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E, smemFalconExtMtiActiveMemAddressFind},
        /* PCS : CPU_PORT : <GOP TAP 1>GOP TAP 1/<MTIP IP CPU WRAPPER>MTIP IP CPU WRAPPER/<MTIP IP CPU>MTIP_IP_CPU/<MTIP IP CPU> PCS/PCS Units/CONTROL1*/
        {0, MTI_PCS_CPU_E, NULL, 0, smemFalconActiveWritePcsControl1, MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E, smemFalconExtMtiActiveMemAddressFind},

        /*MPFS : <GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MPF>MPF/<MPF> <MPFS> MPFS/MPFS Units %a %b/SAU Status*/
        /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MPFS.SAUStatus*/
        /*0x00406000 + 0x80000*a + 0x4000*b: where a (0-1) represents mpf num, where b (0-7) represents mpfs num*/
        {0x00406104, SMEM_FULL_MASK_CNS-GOP_CHANNEL_OFFSET-0x3C000, smemChtActiveReadCntrs, 0, NULL, 0},
        /*SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_high_speed[portNum].MPFS.SAUStatus*/
        /*0x00406000 + 0x80000*a + 0x4000*b: where a (0-1) represents mpf num, where b (8-9) represents mpfs num*/
        {0x00406104+(8*0x4000), SMEM_FULL_MASK_CNS-GOP_CHANNEL_OFFSET-0x4000, smemChtActiveReadCntrs, 0, NULL, 0},
        /*SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MPFS_CPU.SAUStatus*/
        {0x00516104, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL, 0},

        /* Statistic counters - active memory write control */
        {0x0045000c, SMEM_FULL_MASK_CNS-GOP_CHANNEL_OFFSET, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},
        /* CPU port : Statistic counters - active memory write control */
        {0x0051a00c, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},

        /* Statistic counters - active memory read */
        /* 0x00450020 .. FC : 56 register */
        /* 0x20 .. 0x3c */
        {0x00450000 + 0x20, 0xFFFFFFE0-GOP_CHANNEL_OFFSET, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},
        /* 0x40 .. 0x7c */
        {0x00450000 + 0x40, 0xFFFFFFC0-GOP_CHANNEL_OFFSET, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},
        /* 0x80 .. 0xfc */
        {0x00450000 + 0x80, 0xFFFFFF80-GOP_CHANNEL_OFFSET, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},

        /* CPU port : Statistic counters - active memory read */
        /* 0x0051a020 .. FC : 56 register */
        /* 0x0051a020 .. FC */
        /* 0x20 .. 0x3c */
        {0x0051a000 + 0x20, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},
        /* 0x40 .. 0x7c */
        {0x0051a000 + 0x40, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},
        /* 0x80 .. 0xfc */
        {0x0051a000 + 0x80, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, 0, NULL, 0},


        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_GLOBAL[pipeChannelIndex].globalInterruptSummaryCause */
        {0x00478034, 0xFFF7FFFF, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
        {0, 0x00478000, smemChtActiveReadIntrCauseReg, MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_CAUSE_E,
            smemChtActiveWriteIntrCauseReg, MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_CAUSE_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause */
        {0, 0x00478000, smemChtActiveReadIntrCauseReg, MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_CAUSE_E,
            smemChtActiveWriteIntrCauseReg, MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_CAUSE_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
        {0, 0x00478000, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_MASK_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask */
        {0, 0x00478000, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_MASK_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecCeInterruptCause; */
        {0, 0x00478000, smemChtActiveReadIntrCauseReg, MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_CAUSE_E,
            smemChtActiveWriteIntrCauseReg, MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_CAUSE_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecNceInterruptCause; */
        {0, 0x00478000, smemChtActiveReadIntrCauseReg, MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_CAUSE_E,
            smemChtActiveWriteIntrCauseReg, MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_CAUSE_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecCeInterruptMask; */
        {0, 0x00478000, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, MTI_ACTIVE_MEM_REG_FEC_CE_INTERRUPT_MASK_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_GLOBAL.globalFecNceInterruptMask; */
        {0, 0x00478000, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, MTI_ACTIVE_MEM_REG_FEC_NCE_INTERRUPT_MASK_E, smemFalconExtMtiActiveMemAddressFind},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[0].MTI_EXT_PORT.portInterruptCause */
        {0x0051C008, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[0].MTI_EXT_PORT.portInterruptMask */
        {0x0051C00C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteCpuPortInterruptsMaskReg, 0},

        /* Read only latency statistics table */
        {0x00430000, 0xFFFFC000, smemFalconActiveReadLatencyProfileStatTable, 0, NULL, 0},
        {0x004B0000, 0xFFFFC000, smemFalconActiveReadLatencyProfileStatTable, 0, NULL, 0},

        /* LMU Profile Statistics Read data 1-6 */
        {0x00438030, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B8030, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00438034, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B8034, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00438038, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B8038, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x0043803C, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B803C, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00438040, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B8040, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x00438044, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
        {0x004B8044, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},

        /* LMU lmu_latency_over_threshold_n_cause lmu/latency_over_threshold_n_mask */
        {0x00438100, 0x00FFFF80, smemChtActiveReadIntrCauseReg, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x004B8100, 0x00FFFF80, smemChtActiveReadIntrCauseReg, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00438180, 0x00FFFF80, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},
        {0x004B8180, 0x00FFFF80, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_latency_fifo_full_n_cause/lmu_latency_fifo_full_n_mask */
        {0x00438200, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x004B8200, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00438210, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},
        {0x004B8210, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_global_interrupt_cause/lmu_global_interrupt_mask */
        {0x00438220, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x004B8220, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteLatencyOverThreshIntrCauseReg, 0},
        {0x00438224, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},
        {0x004B8224, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* LMU lmu_cause_summary/lmu_mask_summary*/
        {0x00438230, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg,                    0},
        {0x004B8230, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg,                    0},
        {0x00438234, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},
        {0x004B8234, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg,         0},

        /* XSMI Management Register */
        {0x00330000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},

        /* D2D_cp - raven side : those 2 belong to the 'raven' interrupts tree - and are per Raven */
        {0x002F0050, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {0x002F0054, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
        /* D2D_cp - eagle side : those 2 belong to the 'eagle' interrupts tree - and are per Tile (only on 'raven 0')*/
        {0x002E0050, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {0x002E0054, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* chunks without formulas */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers address space */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100000, 0x001FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00200000, 0x002FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00300000, 0x003FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00400000, 0x0042FFFF)}
           ,{
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00430000, 16384),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(186, 32),
               SMEM_BIND_TABLE_MAC(lmuStatTable)
            }
           ,{
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00434000, 8192),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(90, 16),
               SMEM_BIND_TABLE_MAC(lmuCfgTable)
            }
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00435FFF, 0x004AFFFF)}
           ,{
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x004B0000, 16384),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(186, 32)
            }
           ,{
               SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x004B4000, 8192),
               SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(90, 16)
            }
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x004B5FFF, 0x004FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00500000, 0x005FFFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00600000, 0x0063FFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00640400, 0x006447FF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00644800, 0x00644BFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00644C00, 0x00644FFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00645000, 0x006453FF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00680000, 0x0068FFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00690000, 0x0069FFFF)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00700000, 0x007FFFFF)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00300050,         0x000011AB,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00438020,         0x00000005,      2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00430010,         0xF0000000,    512,    0x00020, 2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00430014,         0x03FFFFFF,    512,    0x00020, 2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00434000,         0xFFFFFFFF,    512,    0x00010, 2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00434004,         0x0FFFFFFF,    512,    0x00010, 2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00478010,         0x0003ffff,      2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00478014,         0xffff0000,      2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00478018,         0x00000003,      2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00478058,         0xffff0000,      2,    0x80000 }
            ,{DUMMY_NAME_PTR_CNS,            0x00444000,         0x00010200,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00444008,         0x00000800,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00444014,         0x00000600,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0044401c,         0x00000008,      2,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00444030,         0x00001440,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00444044,         0x0000000c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464000,         0x0000204c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464010,         0x00000015,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464014,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464020,         0x000081b1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464050,         0x00000741,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464088,         0x0000804f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046408c,         0x0000cab6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464090,         0x0000b44d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464094,         0x000003c8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464098,         0x00008884,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046409c,         0x000085a3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004640a0,         0x000006bb,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004640a4,         0x00000349,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464804,         0x00000220,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464808,         0x00003fff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046480c,         0x00009999,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464840,         0x00000003,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464900,         0x000068c1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464904,         0x00000021,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464908,         0x0000719d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046490c,         0x0000008e,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464910,         0x00004b59,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464914,         0x000000e8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464918,         0x0000954d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046491c,         0x0000007b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464920,         0x000007f5,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464924,         0x00000009,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464928,         0x000014dd,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046492c,         0x000000c2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464930,         0x00004a9a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464934,         0x00000026,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464938,         0x0000457b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046493c,         0x00000066,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464940,         0x000024a0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464944,         0x00000076,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464948,         0x0000c968,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046494c,         0x000000fb,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464950,         0x00006cfd,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464954,         0x00000099,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464958,         0x000091b9,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046495c,         0x00000055,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464960,         0x0000b95c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464964,         0x000000b2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464968,         0x0000f81a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046496c,         0x000000bd,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464970,         0x0000c783,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464974,         0x000000ca,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464978,         0x00003635,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046497c,         0x000000cd,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464980,         0x000031c4,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464984,         0x0000004c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464988,         0x0000d6ad,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046498c,         0x000000b7,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464990,         0x0000665f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464994,         0x0000002a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00464998,         0x0000f0c0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046499c,         0x000000e5,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465000,         0x0000204c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465010,         0x00000015,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465014,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046501c,         0x00000004,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465020,         0x00008091,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465050,         0x00000741,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465088,         0x0000804f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046508c,         0x0000cab6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465090,         0x0000b44d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465094,         0x000003c8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465098,         0x00008884,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046509c,         0x000085a3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004650a0,         0x000006bb,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004650a4,         0x00000349,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465804,         0x00000220,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465808,         0x00003fff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046580c,         0x00000007,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465810,         0x00000d80,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465820,         0x00007690,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465824,         0x00000047,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465828,         0x0000c4f0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046582c,         0x000000e6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465830,         0x000065c5,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465834,         0x0000009b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465838,         0x000079a2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046583c,         0x0000003d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00465840,         0x00000003,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00461000,         0x00002064,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00461008,         0x00000001,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00461010,         0x00000100,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00461014,         0x00000008,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0046101c,         0x0000000c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00461020,         0x00008000,      2,    0x94    }
            ,{DUMMY_NAME_PTR_CNS,            0x00461024,         0x00000001,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470004,         0x0000000a,      8,    0x20    }
            ,{DUMMY_NAME_PTR_CNS,            0x0047001c,         0x00000033,      8,    0x20    }
            ,{DUMMY_NAME_PTR_CNS,            0x00470100,         0x00002000,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470104,         0x000015b8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470108,         0x0000023d,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470180,         0x00007101,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470184,         0x000000f3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470188,         0x0000de5a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0047018c,         0x0000007e,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470190,         0x0000f33e,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470194,         0x00000056,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00470198,         0x00008086,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0047019c,         0x000000d0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701a0,         0x0000512a,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701a4,         0x000000f2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701a8,         0x00004f12,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701ac,         0x000000d1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701b0,         0x00009c42,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701b4,         0x000000a1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701b8,         0x000076d6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701bc,         0x0000005b,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701c0,         0x000073e1,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701c4,         0x00000075,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701c8,         0x0000c471,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701cc,         0x0000003c,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701d0,         0x0000eb95,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701d4,         0x000000d8,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701d8,         0x00006622,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701dc,         0x00000038,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701e0,         0x0000f6a2,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701e4,         0x00000095,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701e8,         0x00009731,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701ec,         0x000000c3,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701f0,         0x0000fbca,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701f4,         0x000000a6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701f8,         0x0000baa6,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x004701fc,         0x00000079,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0047028c,         0x00000140,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00510000,         0x6610E017,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00510004,         0x00100908,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00510008,         0x00000F04,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00510214,         0x00005008,      1,    0x0     }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};

        {
            /* D2d support*/
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list0;
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list1;
            smemFalconUnitRavenD2dDefaultsBaseAddr(devObjPtr,unitPtr,0x00680000,&list0);
            smemFalconUnitRavenD2dDefaultsBaseAddr(devObjPtr,unitPtr,0x00690000,&list1);
            list0.nextPtr = &list1;

            list.nextPtr = &list0;
       }


        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {
        /* unit d2d 0 Addresses */
        smemFalconUnitRavenD2dDAddrSet(devObjPtr,unitPtr,0x00680000,0);

        /* unit d2d 1 Addresses */
        smemFalconUnitRavenD2dDAddrSet(devObjPtr,unitPtr,0x00690000,1);

        /* MG interrupts */
        smemFalconUnitRavenMgInterruptAddrSet(devObjPtr);
    }
}

/**
* @internal smemFalconUnitLpm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the
*          falcon LPM unit
*/
static void smemFalconUnitLpm
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    if(unitPtr->chunkType == SMEM_UNIT_CHUNK_TYPE_9_MSB_E)
    {
        unitPtr->numOfUnits = 2;/* supports 2 '23 bits' address units */
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00f10000, 16384),
                     SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4),SMEM_BIND_TABLE_MAC(lpmIpv4VrfId)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00F20000, 16384),
                     SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4),SMEM_BIND_TABLE_MAC(lpmIpv6VrfId)},
           {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00F30000, 16384),
                     SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4),SMEM_BIND_TABLE_MAC(lpmFcoeVrfId)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00F80000, 251680),
                     SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(lpmAgingMemory)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00000, 0x00F00008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00010, 0x00F00010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00020, 0x00F00020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F000F0, 0x00F000F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00120, 0x00F00120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00130, 0x00F00130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00200, 0x00F00240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00250, 0x00F0026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00300, 0x00F00374)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00400, 0x00F00474)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00500, 0x00F00540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00600, 0x00F00600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00F00608, 0x00F00608)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }


    /* to support bobk :
       add tables that depend on number of vlans and number of eport and number
       of physical ports */
    {
        GT_U32  perRamNumEntries = devObjPtr->lpmRam.perRamNumEntries;
        GT_U32  index;

        {

            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                /* support 30 chunks of 14K each in steps of 32K */
                {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000, ((14*1024)*16)),
                     SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(115, 16),SMEM_BIND_TABLE_MAC(lpmMemory)},
                     FORMULA_SINGLE_PARAMETER(30, (32*1024)*16)},
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            index = 0;
            chunksMem[index].memChunkBasic.numOfRegisters = perRamNumEntries * (chunksMem[index].memChunkBasic.enrtyNumBytesAlignement / 4);
            index++;

            smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,         0x00f00000,         0x00000001,      2,    0x4                  }
            ,{DUMMY_NAME_PTR_CNS,         0x00f000f0,         0x0000ffff,      1,    0x0                  }
            ,{DUMMY_NAME_PTR_CNS,         0x00f00250,         0x0007ffff,      4,    0x4,      2,    0x10 }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemFalconSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_IPVX)        ,smemFalconUnitIpvx}
        ,{STR(UNIT_EGF_SHT)     ,smemFalconUnitEgfSht}
        ,{STR(UNIT_EGF_QAG)     ,smemFalconUnitEgfQag}
        ,{STR(UNIT_EGF_EFT)     ,smemFalconUnitEgfEft}
        ,{STR(UNIT_IPLR)        ,smemFalconUnitIplr0}
        ,{STR(UNIT_IPLR1)       ,smemFalconUnitIplr1}
        ,{STR(UNIT_EPLR)        ,smemFalconUnitEplr}
        ,{STR(UNIT_HA)          ,smemFalconUnitHa}
        ,{STR(UNIT_ERMRK)       ,smemFalconUnitErmrk}
        ,{STR(UNIT_EQ)          ,smemFalconUnitEq}
        ,{STR(UNIT_L2I)         ,smemFalconUnitL2i}
        ,{STR(UNIT_SHM)         ,smemFalconUnitShm}
        ,{STR(UNIT_EM)          ,smemFalconUnitEm}
        ,{STR(UNIT_FDB)         ,smemFalconUnitFdb}
        ,{STR(UNIT_TTI)         ,smemFalconUnitTti}
        ,{STR(UNIT_IPCL)        ,smemFalconUnitIpcl}
        ,{STR(UNIT_EPCL)        ,smemFalconUnitEpcl}
        ,{STR(UNIT_EREP)        ,smemFalconUnitErep}
        ,{STR(UNIT_PREQ)        ,smemFalconUnitPreq}
        ,{STR(UNIT_MLL)         ,smemFalconUnitMll}
        ,{STR(UNIT_TAI)         ,smemFalconUnitTai}
        ,{STR(UNIT_LPM)         ,smemFalconUnitLpm}
        ,{STR(UNIT_PHA)         ,smemFalconUnitPha}
        /* must be last */
        ,{NULL,NULL}
    };
    GT_U32  numOfTiles,tileId;

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    smemAldrin2SpecificDeviceUnitAlloc_SIP_units(devObjPtr);

    numOfTiles = devObjPtr->numOfTiles ? devObjPtr->numOfTiles : 1;

    for(tileId = 0 ; tileId < numOfTiles ; tileId++)
    {
        /* allocate the FDB hidden memory */
        smemFalconUnitFdbTable(devObjPtr,tileId);
        /* allocate the Exact Match hidden memory */
        smemFalconUnitExactMatchTable(devObjPtr,tileId);
    }
    /* allocate the AAC hidden memory */
    smemFalconUnitAacMem(devObjPtr);

}

/**
* @internal smemFalconUnitBma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitBma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 5376)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059000, 0x0005900C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059014, 0x00059018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059020, 0x00059054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00059060, 0x00059074)}
            /*Virtual => Physical source port mapping*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0005A000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(7 , 4),SMEM_BIND_TABLE_MAC(bmaPortMapping)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

             {DUMMY_NAME_PTR_CNS,            0x00059008,         0x0060A000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00059034,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00059040,         0x00000100,      4,    0x4      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemFalconUnitHbu function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitHbu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000050)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

#define FALCON_MAX_PROFILE_CNS  8

/**
* @internal smemFalconUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001220)}
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
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x0079a591,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00004001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00001000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x80000020,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x80000000,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x80000004,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x80000002,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x80000006,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x80000001,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x80000005,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x80000003,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x80000007,      4,    0x20   }
            ,{DUMMY_NAME_PTR_CNS,            0x00000180,         0x80000008,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00000120,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000604,         0x00000090,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608,         0x00000048,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c,         0x00000024,      4,    0x4,      2,    0x14}
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c,         0x00000009,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x000000b8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0x0000005c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x0000002e,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x00000017,      4,    0x4,      2,    0x14}
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000006,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00000003,      8,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x00000007,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001104,         0x00000024,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001108,         0x00000048,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000110c,         0x0000006c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0x00000090,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001114,         0x000000b4,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001118,         0x000000d8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000111c,         0x000000fc,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001120,         0x00000120,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001204,         0x00000017,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001208,         0x0000002e,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000120c,         0x00000045,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001210,         0x0000005c,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001214,         0x00000073,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001218,         0x0000008a,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0000121c,         0x000000a1,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001220,         0x000000b8,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0xffff0000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00005100,         0x0000001f,      1,    0x0    }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit sip6_txFIFO[] */
        {/*start of unit globalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.globalConfig1 = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.packetsMaximalReadLatencyFromPacketPuffer = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.wordBufferCmn8wr4rdBufferConfig = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000050;
            {/*0000100+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 33 /* pizza slices number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[p] =
                        0x00000100 + p*0x4;
                }/* end if loop p*/
            } /*0000100+p*0x4*/

        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigs */
            {/*0x00000600 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.tagFIFODepth[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/

        }/*end of unit speedProfileConfigs */

        {/*start of unit channelConfigs */
            {/*0x00001000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.speedProfile[p] =
                        0x00001000 +p*0x4;
                }/* end of loop n */
            }/*0x00001000  + p*0x4*/
            {/*0x00001100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00001100  +p*0x4;
                }/* end of loop n */
            }/*0x00001100   + p*0x4*/
            {/*0x00001200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
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
* @internal smemFalconActiveWriteRxDmaDebugClearAllCounters function
* @endinternal
*
* @brief   write of RxDma Debug Counter subunit disabled all counters cleared
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemFalconActiveWriteRxDmaDebugClearAllCounters (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  i;
    GT_U32 counterBase = (address & 0xFFFF0000);
    static const GT_U32 counterOffsetArr[] =
    {
         0x1F00 /*RX Ingress Stress and Errors Counters*/
        ,0x1F80 /*RX Accum MEM Counters*/
        ,0x2000 /*RX Ingress Drop Counters*/
        ,0x2080 /*RX to CP Counters*/
        ,0x2100 /*CP to RX Counters*/
        ,0x2180 /*RX Egress Drop Counters*/
        ,0x2200 /*RX to PB Counters*/
        ,0x2280 /*PB to RX Counters*/
        ,0x2300 /*RX to PDX Counters*/
        ,0x2380 /*PDX to RX Counters*/
    };
    static const GT_U32 counterOffsetArrSise =
        (sizeof(counterOffsetArr) / sizeof(counterOffsetArr[0]));

    /* data to be written */
    for (i = 0; (i < memSize); i++)
    {
        memPtr[i] = inMemPtr[i];
    }

    /* counters cleared on on disable, on enable nothing to do */
    if (inMemPtr[0] & 1) return;

    /* clear counters */
    for (i = 0; (i < counterOffsetArrSise); i++)
    {
        smemRegSet(devObjPtr, (counterBase + counterOffsetArr[i]), 0);
    }
}

/**
* @internal smemFalconUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writting 0 to enable register*/
        {0x00001D84, 0xFFFFFFFF  , NULL, 0, smemFalconActiveWriteRxDmaDebugClearAllCounters, 0, NULL},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000118)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000920)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001080, 0x00001244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001258, 0x00001410)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000014F0, 0x000014FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x0000156C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C00, 0x00001C08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C80, 0x00001C94)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D00, 0x00001D00)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D80, 0x00001D88)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001E04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E80, 0x00001EA0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F00, 0x00001F08)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001F80, 0x00001F88)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x00002088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002108)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002180, 0x00002188)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x00002208)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002280, 0x0000228C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002308)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002380, 0x00002388)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {

             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x03ff03ff      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x0000ffff      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0x003e003e      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x03d90007      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x0003ffff      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x00000002      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x000000ff      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x00003fff,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x00000407,      9,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000001,      9,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a04,         0x00000001      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a08,         0x00000002      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a0c,         0x00000003,      2,    0x5f4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a10,         0x00000004      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a14,         0x00000005      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a18,         0x00000006      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a1c,         0x00000007,      2,    0x186c}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a20,         0x00000008      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b00,         0x00000001,      9,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001240,         0x00000800      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001244,         0x000086dd      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001258,         0x00008847      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000125c,         0x00008848      }
            ,{DUMMY_NAME_PTR_CNS,            0x000013d0,         0x00018100,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000013e0,         0x00008100      }
            ,{DUMMY_NAME_PTR_CNS,            0x000013e4,         0x00008a88      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c00,         0xffffffff,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001c08,         0x00000001,      2,    0x1f8}
            ,{DUMMY_NAME_PTR_CNS,            0x00001d00,         0xffff0000      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d80,         0x7fffffff      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e80,         0x00000555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e84,         0x00155555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e88,         0x55555555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e8c,         0x01555555,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00001e94,         0x00000155      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e98,         0x00005555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e9c,         0x00555555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ea0,         0x00015555      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f04,         0x00000033      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f08,         0x0000004c      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f84,         0x0000000b      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f88,         0x0000000a,      2,    0x27c}
            ,{DUMMY_NAME_PTR_CNS,            0x00002004,         0x0000017f      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002104,         0x0000001f,      2,    0x80}
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x0000000a      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002284,         0x00000001      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002304,         0x0000001f      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002384,         0x00000007      }

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
                    for(n = 0 ; n <= 8 ; n++) {  /* manually fixed from : for(n = 0 ; n <= 63 ; n++) { */
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
                    for(n = 0 ; n <= 8 ; n++) {
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
                    for(n = 0 ; n <= 8 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelToLocalDevSourcePort[n] =
                            0x0000a00+n*0x4;
                    }/* end of loop n */
                }/*00004a8+n*0x4*/
                {/*00003a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 8 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.PCHConfig[n] =
                            0x0000900+n*0x4;
                    }/* end of loop n */
                }/*00003a8+n*0x4*/
                {/*00005a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 8 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelGeneralConfigs[n] =
                            0x0000b00+n*0x4;
                    }/* end of loop n */
                }/*00005a8+n*0x4*/
                {/*00002a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 8 ; n++) {
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
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxIngressDropCounter.rx_ingress_drop_count_type_ref  = 0x00002004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxIngressDropCounter.rx_ingress_drop_count_type_mask = 0x00002008;
            }/*rxIngressDropCounter*/

            {/*rxToCpCounter*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count           = 0x00002080;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count_type_ref  = 0x00002084;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxToCpCounter.rx_to_cp_count_type_mask = 0x00002088;
            }/*rxToCpCounter*/

            {/*rxCpToRxCounter*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count           = 0x00002100;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count_type_ref  = 0x00002104;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].debug.rxCpToRxCounter.cp_to_rx_count_type_mask = 0x00002108;
            }/*rxCpToRxCounter*/
        }/*debug*/
    }/*end of unit sip6_rxDMA[] */
}

/**
* @internal smemFalconUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitTxDma
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
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x00000050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000190)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000092C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x00002220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x00002320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002500, 0x00002520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003120)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x00003320)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600, 0x00003620)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003700, 0x00003720)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x0000400C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007014)}
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
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0x80000020,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x80000000,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x80000004,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x80000002,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x80000006,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x80000001,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000114,         0x80000005,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x80000003,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x80000007,      4,    0x20     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000180,         0x80000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000184,         0x80000000,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00000038,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000604,         0x0000001c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c,         0x00000007,      4,    0x4,      2,    0x14}
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x00000024,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0x00000013,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x0000000a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x00000005,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x00000004,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000002,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000800,         0x59f00bd0,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000804,         0x545c063c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000808,         0x51680348,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000080c,         0x501801f8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000810,         0x4fc401a4,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000814,         0x4f700150,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000818,         0x4f1c00fc,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000081c,         0x4ec800a8,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000456,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000904,         0x0000022e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000908,         0x0000011c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000090c,         0x00000093,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000910,         0x00000076,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000914,         0x0000004e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000918,         0x0000003f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000091c,         0x00000025,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a00,         0x00000228,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a04,         0x00000114,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a08,         0x0000008a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000a0c,         0x00000045,      4,    0x4,      2,    0x14}
            ,{DUMMY_NAME_PTR_CNS,            0x00000a1c,         0x0000000d,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b00,         0x00000120,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b04,         0x00000090,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b08,         0x00000048,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b0c,         0x00000024,      4,    0x4,      2,    0x14}
            ,{DUMMY_NAME_PTR_CNS,            0x00000b1c,         0x00000009,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c00,         0x3e800100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c04,         0x1f400100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c08,         0x0fa00100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c0c,         0x07d00100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c10,         0x06400100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c14,         0x03e80100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c18,         0x03200100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c1c,         0x01900100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c20,         0x00000100,      4,    0x4,      2,    0x100}
            ,{DUMMY_NAME_PTR_CNS,            0x00000d00,         0x14000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d04,         0x0a000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d08,         0x05000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d0c,         0x02800100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d10,         0x02000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d14,         0x01400100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d18,         0x01000100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d1c,         0x00800100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x00000003,      8,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002020,         0x00000007,      2,    0x1e4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x0000000e,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c,         0x00000015,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002210,         0x0000001c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002214,         0x00000023,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002218,         0x0000002a,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000221c,         0x00000031,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002220,         0x00000038,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300,         0x18000001,      9,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400,         0x00000001,      9,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002500,         0x04000001,      9,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003700,         0x00004000,      9,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0x0fa00100,      1,    0x0      }
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
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqMaxCredits[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/
            {/*0x00000800 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqThresholdBytes[p] =
                        0x00000800 + p*0x4;
                    }/* end of loop p */
            }/*0x00000800 + p*0x4*/
            {/*0x00000900 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxWordCredits[p] =
                        0x00000900 + p*0x4;
                    }/* end of loop p */
            }/*0x00000900 + p*0x4*/
            {/*0x00000A00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxCellsCredits[p] =
                        0x00000A00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000A00 + p*0x4*/
            {/*0x00000B00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxDescCredits[p] =
                        0x00000B00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000B00 + p*0x4*/
            {/*0x00000C00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interPacketRateLimiter[p] =
                        0x00000C00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000C00 + p*0x4*/
             {/*0x00000D00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < FALCON_MAX_PROFILE_CNS; p++) {
                  SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interCellRateLimiter[p] =
                        0x00000D00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000D00 + p*0x4*/

        }/*end of unit speedProfileConfigurations */

        {/*start of unit channelConfigs */
            {/*0x00002000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.speedProfile[p] =
                        0x00002000 +p*0x4;
                }/* end of loop n */
            }/*0x00002000  + p*0x4*/
            {/*0x00002100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.channelReset[p] =
                        0x00002100  +p*0x4;
                }/* end of loop n */
            }/*0x00002100   + p*0x4*/
            {/*0x00002200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00002200  +p*0x4;
                }/* end of loop n */
            }/*0x00002200   + p*0x4*/
            {/*0x00002300  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.interPacketRateLimiterConfig[p] =
                        0x00002300 +p*0x4;
                }/* end of loop n */
            }/*0x00002300  + p*0x4*/
            {/*0x00002400  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 8 ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
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

/**
* @internal smemFalconUnitEagleD2d function
* @endinternal
*
* @brief   Allocate address type specific memories
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr              - pointer to the unit
*       chunk
*/
static void smemFalconUnitEagleD2d
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004048)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004104, 0x00004180)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004204, 0x00004288)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x00004488)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000045FC, 0x000046A0)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000047E4, 0x000047EC)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004804, 0x00004848)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004904, 0x00004988)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004A04, 0x00004A88)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004C04, 0x00004C88)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004E04, 0x00004E48)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004F04, 0x00004F24)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004FFC, 0x00004FFC)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006038, 0x0000603C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000607C, 0x0000607C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000702C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007040, 0x00007084)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x0000710C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007200, 0x0000727C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007400, 0x0000747C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007600, 0x0000767C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008004, 0x00008008)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008010, 0x00008068)}

           /* manually added (not from report)*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002344)}/*rxc*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003c00, 0x00003f38)}/*rxbr-rxdata*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003f40, 0x00003ffc)}/*rxbr-rxdatacon*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000004, 0x0000027c)}/*txc*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001c40, 0x00001d04)}/*txbr*/

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
           /* manually added (not from report)*/
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x000027fc)}, FORMULA_SINGLE_PARAMETER(4 , 0x0400)}/*rx0..3-rxdatacon*/

           ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x00000504)}, FORMULA_SINGLE_PARAMETER(4 , 0x0400)}/*tx0..3*/
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
                {DUMMY_NAME_PTR_CNS,            0x00004000,         0x00030007,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0xe000000c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0xe000080c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0xe000100c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004010,         0xe000180c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004014,         0xe000200c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004018,         0xe000280c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000401c,         0xe000300c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004020,         0xe000380c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004024,         0xe000400c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004028,         0xe000480c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000402c,         0xe000500c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004030,         0xe000580c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004034,         0xe000600c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004038,         0xe000680c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000403c,         0xe000700c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004040,         0xe000780c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004044,         0xe0008006,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004048,         0xe0008806,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004104,         0x00000001,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000410c,         0x00000003,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004110,         0x00000002,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004114,         0x00000005,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004118,         0x00000004,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000411c,         0x00000007,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004120,         0x00000006,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004124,         0x00000009,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004128,         0x00000008,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000412c,         0x0000000b,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004130,         0x0000000a,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004134,         0x0000000d,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004138,         0x0000000c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000413c,         0x0000000f,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004140,         0x0000000e,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004144,         0x00000010,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004148,         0x0000000f,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000414c,         0x00000012,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004150,         0x00000011,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004154,         0x00000014,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004158,         0x00000013,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000415c,         0x00000016,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004160,         0x00000015,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004164,         0x00000018,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004168,         0x00000017,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000416c,         0x0000001a,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004170,         0x00000019,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004174,         0x0000001c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004178,         0x0000001b,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000417c,         0x0000001e,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004180,         0x0000001d,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004604,         0x86848280,      2,    0x8}
               ,{DUMMY_NAME_PTR_CNS,            0x00004608,         0x87858381,     10,    0x8}
               ,{DUMMY_NAME_PTR_CNS,            0x00004614,         0x86848288,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000461c,         0x86848280,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004624,         0x86848880,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000462c,         0x86848280,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004634,         0x86888280,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000463c,         0x86848280,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004644,         0x88848280,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000464c,         0x86848280,     10,    0x8}
               ,{DUMMY_NAME_PTR_CNS,            0x00004658,         0x87858388,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004660,         0x87858381,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004668,         0x87858881,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004670,         0x87858381,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004678,         0x87888381,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004680,         0x87858381,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004688,         0x88858381,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004690,         0x87858381,      3,    0x8}
               ,{DUMMY_NAME_PTR_CNS,            0x0000469c,         0x86848288,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004804,         0xf0380000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004808,         0xf0381000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000480c,         0xf0382000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004810,         0xf0383000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004814,         0xf0384000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004818,         0xf0385000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000481c,         0xf0386000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004820,         0xf0387000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004824,         0xf01c0000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004828,         0xf01c1000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000482c,         0xf01c2000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004830,         0xf01c3000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004834,         0xf01c4000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004838,         0xf01c5000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000483c,         0xf01c6000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004840,         0xf01c7000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004844,         0xf01c8000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004848,         0xf01c9000,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004904,         0x00000001,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004908,         0x00000002,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000490c,         0x00000003,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004914,         0x00000005,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004918,         0x00000006,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000491c,         0x00000007,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004920,         0x00000004,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004924,         0x00000009,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004928,         0x0000000a,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000492c,         0x0000000b,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004930,         0x00000008,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004934,         0x0000000d,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004938,         0x0000000e,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000493c,         0x0000000f,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004940,         0x0000000c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004944,         0x00000011,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004948,         0x00000012,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000494c,         0x00000013,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004950,         0x00000010,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004954,         0x00000015,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004958,         0x00000016,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000495c,         0x00000017,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004960,         0x00000014,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004964,         0x00000019,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004968,         0x0000001a,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000496c,         0x0000001b,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004970,         0x00000018,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004974,         0x0000001d,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004978,         0x0000001e,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000497c,         0x0000001f,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004980,         0x0000001c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004984,         0x00000021,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004988,         0x00000020,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00004e04,         0x00401406,      8,    0x4}
               ,{DUMMY_NAME_PTR_CNS,            0x00004e24,         0x00200c0a,     10,    0x4}
               ,{DUMMY_NAME_PTR_CNS,            0x00007000,         0x000d003c,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x01001654,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007008,         0x01001654,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x0000700c,         0x1eb18602,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007078,         0x0000ffff,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007200,         0x00182100,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007204,         0x001c3141,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007210,         0x00084008,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007214,         0x000c5046,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007218,         0x00084007,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007220,         0x00100207,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007224,         0x00141182,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007228,         0x001001c3,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007230,         0x000081c3,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007234,         0x00046084,      2,    0x8,      2,    0x200 }
               ,{DUMMY_NAME_PTR_CNS,            0x00007238,         0x000070c5,      2,    0x44,      2,    0x200}
               ,{DUMMY_NAME_PTR_CNS,            0x00007240,         0x002070c5,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007278,         0x00046084,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007400,         0x00182100,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007404,         0x001c3141,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007414,         0x000c5046,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007424,         0x00141182,      2,    0x8,      2,    0x44  }
               ,{DUMMY_NAME_PTR_CNS,            0x00007440,         0x002070c5,      1,    0x0}
               ,{DUMMY_NAME_PTR_CNS,            0x00007478,         0x00046084,      1,    0x0}

               ,{NULL,            0x00000000,         0x00000000,      0,    0x0}

            };

            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
            unitPtr->unitDefaultRegistersPtr = &list;
    }
    {/*start of unit eagle_d2d[] */
        {/*start of unit mac rx */

           {/*0x00004604+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 40 /* mac rx registers number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macRx.rxTdmSchdule[p] =
                        0x00004604 + p*0x4;
                }/* end if loop p*/
           } /*0x00004604+p*0x4*/

           {/*0x000047E4+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 3 /* mac rx select bandwidth registers number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macRx.rxFifoBandwidthSelect[p] =
                        0x000047E4 + p*0x4;
                }/* end if loop p*/
           } /*0x000047E4+p*0x4*/

           {/*0x00004804+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 18 /* mac rx channels number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macRx.rxChannel[p] =
                        0x00004804  + p*0x4;
                }/* end if loop p*/
           } /*0x00004804+p*0x4*/

           {/*0x00004E04+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 18 /* mac rx channels number*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macRx.rxChannel2[p] =
                        0x00004E04  + p*0x4;
                }/* end if loop p*/
           } /*0x00004E04+p*0x4*/

           {/*0x00004904+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 34 /* number of rx fifos segments*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macRx.rxFifoSegPtr[p] =
                        0x00004904  + p*0x4;
                }/* end if loop p*/
            } /*0x00004904+p*0x4*/

        }/*end of unit  mac rx */

        {/*start of unit macTx */
            {/*0x00004004 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < 18; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macTx.txChannel[p] =
                        0x00004004 + p*0x4;
                    }/* end of loop p */
            }/*0x00004004 + p*0x4*/

            {/*0x00004104 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < 32 /* number of tx fifos segments*/; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.macTx.txFifoSegPtr[p] =
                        0x00004104 + p*0x4;
                    }/* end of loop p */
            }/*0x00004104 + p*0x4*/

        }/*end of unit  mac tx */

        {/*start of unit pcs */
             SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.pcs.globalControl = 0x00007000;
             SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.pcs.transmitCalCtrl = 0x00007004;
             SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.pcs.receiveCalCtrl = 0x00007008;

             {/*0x00007200 + p*0x4*/
                 GT_U32    p;
                 for(p = 0 ; p < 32 /* number of pcs calendar slices*/; p++) {
                     SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.pcs.transmitCalSlotConfig[p] =
                         0x00007200 + p*0x4;
                     }/* end of loop p */
             }/*0x00007200 + p*0x4*/

             {/*0x00007400 + p*0x4*/
                 GT_U32    p;
                 for(p = 0 ; p < 32 /* number of pcs calendar slices*/; p++) {
                     SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d[0].d2d.pcs.receiveCalSlotConfig[p] =
                         0x00007400 + p*0x4;
                     }/* end of loop p */
             }/*0x00007400 + p*0x4*/

        }/*end of unit  pcs */
    } /*end of unit eagle_d2d[] */
}

/**
* @internal smemFalconUnitPacketBuffer_pbCenter function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_pbCenter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCenter */
    /* pbCounter (in pbCenter )*/
    {
        /* 18_22_22 */
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
            /* pbCenter */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001124)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000125C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x00001420)}

           /* pbCounter */
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000198)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_smbWriteArbiter function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_smbWriteArbiter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* smbWriteArbiter */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000004, 0x00000030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000114)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_packetWrite function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_packetWrite
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    static const GT_U32 step_gpcPacketWrite = 0x400;

    /* gpcGrpPacketWrite */
    /* gpcPacketWrite */
    {
        SMEM_CHUNK_BASIC_STC  grpChunksMem[]=
        {
            /* registers space */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001014, 0x00001018)}
        };
        GT_U32  grpNumOfChunks = sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_CHUNK_BASIC_STC  gpcChunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000015C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000208)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001010)}
        };
        GT_U32  gpcNumOfChunks = sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_CHUNK_BASIC_STC  workChunksMem[
            (sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC)) +
            ((sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC))*4)];
        GT_U32 instance, chank;

        /* copy GRP chanks to common array */
        for (chank = 0; (chank < grpNumOfChunks); chank++)
        {
            workChunksMem[chank] = grpChunksMem[chank];
        }
        for (instance = 0; (instance < 4); instance++)
        {
            for (chank = 0; (chank < gpcNumOfChunks); chank++)
            {
                /* duplicate GPC chanks inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank] =
                    gpcChunksMem[chank];
                /* adjust GPC chanks bases inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank].memFirstAddr =
                    gpcChunksMem[chank].memFirstAddr + (instance * step_gpcPacketWrite);
            }
        }

        smemInitMemChunk(
            devObjPtr, workChunksMem, (grpNumOfChunks + (gpcNumOfChunks * 4)), unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_packetRead function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_packetRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    static const GT_U32 step_gpcPacketRead  = 0x1000;

    /* gpcGrpPacketRead */
    /* gpcPacketRead */
    {
        static SMEM_CHUNK_BASIC_STC  grpChunksMem[]=
        {
            /* registers space */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002014, 0x00002014)}
        };
        static GT_U32  grpNumOfChunks = sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        static SMEM_CHUNK_BASIC_STC  gpcChunksMem[]=
        {
            /* registers space */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x000000CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x000001CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x000002CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000384)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000044C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000079C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000008BC)}
            /*debug*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000AB4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000AB8, 0x00000AEC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000AF8, 0x00000B14)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B1C, 0x00000B28)}
            /*interruts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E28)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002010)}
        };
        static GT_U32  gpcNumOfChunks = sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        SMEM_CHUNK_BASIC_STC  workChunksMem[
            (sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC)) +
            ((sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC))*2)];
        GT_U32 instance, chank;

        /* copy GRP chanks to common array */
        for (chank = 0; (chank < grpNumOfChunks); chank++)
        {
            workChunksMem[chank] = grpChunksMem[chank];
        }
        for (instance = 0; (instance < 2); instance++)
        {
            for (chank = 0; (chank < gpcNumOfChunks); chank++)
            {
                /* duplicate GPC chanks inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank] =
                    gpcChunksMem[chank];
                /* adjust GPC chanks bases inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank].memFirstAddr =
                    gpcChunksMem[chank].memFirstAddr + (instance * step_gpcPacketRead);
            }
        }

        smemInitMemChunk(
            devObjPtr, workChunksMem, (grpNumOfChunks + (gpcNumOfChunks * 2)), unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_cellRead function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_cellRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    static const GT_U32 step_gpcCellRead    = 0x100;

    /* gpcCellRead */
    /* gpcGrpCellRead */
    {
        SMEM_CHUNK_BASIC_STC  grpChunksMem[]=
        {
            /* registers space */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
        };
        GT_U32  grpNumOfChunks = sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_CHUNK_BASIC_STC  gpcChunksMem[]=
        {
            /* registers space */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000002C, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A8)}
        };
        GT_U32  gpcNumOfChunks = sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_CHUNK_BASIC_STC  workChunksMem[
            (sizeof(grpChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC)) +
            ((sizeof(gpcChunksMem)/sizeof(SMEM_CHUNK_BASIC_STC))*2)];
        GT_U32 instance, chank;

        /* copy GRP chanks to common array */
        for (chank = 0; (chank < grpNumOfChunks); chank++)
        {
            workChunksMem[chank] = grpChunksMem[chank];
        }
        for (instance = 0; (instance < 2); instance++)
        {
            for (chank = 0; (chank < gpcNumOfChunks); chank++)
            {
                /* duplicate GPC chanks inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank] =
                    gpcChunksMem[chank];
                /* adjust GPC chanks bases inside the unit */
                workChunksMem[grpNumOfChunks + (instance * gpcNumOfChunks) + chank].memFirstAddr =
                    gpcChunksMem[chank].memFirstAddr + (instance * step_gpcCellRead);
            }
        }

        smemInitMemChunk(
            devObjPtr, workChunksMem, (grpNumOfChunks + (gpcNumOfChunks * 2)), unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_npmMc function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_npmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* npmMc */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000158)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000410, 0x0000041C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000021C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001020)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001110)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001210)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemFalconUnitPacketBuffer_sbmMc function
* @endinternal
 *
*/
static void smemFalconUnitPacketBuffer_sbmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* sbmMc */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000070)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000160)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemFalconActiveWriteViaBar0 function
* @endinternal
*
* @brief   write a memory that is 'mirrored' to place in BAR0
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWriteViaBar0 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* call to scib , to allow active memory */
    scibMemoryClientRegWrite(devObjPtr->deviceId,SCIB_MEM_ACCESS_BAR0_E,address , 1 , inMemPtr);
}

/**
* @internal smemFalconActiveWritePci_e_ViaBar0 function
* @endinternal
*
* @brief   write a memory that is 'mirrored' to place in BAR0
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWritePci_e_ViaBar0 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* call to scib , to allow active memory */
    scibMemoryClientRegWrite(devObjPtr->deviceId,SCIB_MEM_ACCESS_BAR0_E,address & 0x3FFF , 1 , inMemPtr);
}

/**
* @internal smemFalconActiveReadViaBar0 function
* @endinternal
*
* @brief   read a memory that is 'mirrored' to place in BAR0
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
static void smemFalconActiveReadViaBar0 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* call to scib , to allow active memory */
    scibMemoryClientRegRead(devObjPtr->deviceId,SCIB_MEM_ACCESS_BAR0_E,address , 1, outMemPtr);
}

/**
* @internal smemFalconActiveReadPci_e_ViaBar0 function
* @endinternal
*
* @brief   read a memory that is 'mirrored' to place in BAR0
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
static void smemFalconActiveReadPci_e_ViaBar0 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* call to scib , to allow active memory */
    scibMemoryClientRegRead(devObjPtr->deviceId,SCIB_MEM_ACCESS_BAR0_E, address & 0x3FFF , 1, outMemPtr);
}

/**
* @internal smemFalconUnitPexConfigSpace function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PEX unit
*/
void smemFalconUnitPexConfigSpace
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Active access bind */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* mirror only the first 0x400 byte of PEX config space from BAR0 */
        {0x00000000 , 0xFFFFFC00, smemFalconActiveReadViaBar0, 0, smemFalconActiveWriteViaBar0, 0},
        /* all other registers will read ZEROs even if the BAR0 hold values in unit : smemFalconUnitBar0(...)*/
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00003FFC)}/*allow 'PCIx' extended 4K of 'PCI config space'*/
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    /* default values */
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
             {NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        /* the actual defaults are coming from : smemFalconUnitBar0(...) */
    }

}

/*************iATU registers*************/
#define iATU_REGION_CTRL_1_REG         0x100
#define iATU_REGION_CTRL_2_REG         0x104
#define iATU_LOWER_BASE_ADDRESS_REG    0x108
#define iATU_UPPER_BASE_ADDRESS_REG    0x10C
#define iATU_LIMIT_ADDRESS_REG         0x110
#define iATU_LOWER_TARGET_ADDRESS_REG  0x114
#define iATU_UPPER_TARGET_ADDRESS_REG  0x118

#define iATU_REGISTER_ADDRESS(_iWindow, _register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_iWindow)*0x200 + (_register))
/*************end of iATU registers*************/
/*************oATU registers*************/
#define oATU_REGION_CTRL_1_REG         0x0
#define oATU_REGION_CTRL_2_REG         0x4
#define oATU_LOWER_BASE_ADDRESS_REG    0x8
#define oATU_UPPER_BASE_ADDRESS_REG    0xC
#define oATU_LIMIT_ADDRESS_REG         0x10
#define oATU_LOWER_TARGET_ADDRESS_REG  0x14
#define oATU_UPPER_TARGET_ADDRESS_REG  0x18

#define oATU_REGISTER_ADDRESS(_iWindow, _register)  \
    (ATU_REGISTERS_OFFSET_IN_BAR0 + (_iWindow)*0x200 + (_register))
/*************end of oATU registers*************/

#define IATU_REG_GET(addr,value) value = *(devMemInfoPtr->common.iAtuWindowsMemPtr[win]+((addr-iATU_REGION_CTRL_1_REG)/4))
#define OATU_REG_GET(addr,value) value = *(devMemInfoPtr->common.oAtuWindowsMemPtr[win]+((addr-oATU_REGION_CTRL_1_REG)/4))

/*******************************************************************************
*   smemFalconBar2CnMiAtuConvert
*
* DESCRIPTION:
*       function to do 'address completion' from address in bar2 by using the iATU
*       in bar0
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       bar2_address - address in bar2 (with the 'prefix' of the 'BAR2' - base address of region 2)
* OUTPUTS:
*       newSwitchAddrPtr  - (pointer to) address of memory in the switch (final 'Cider' address).
*
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemFalconBar2CnMiAtuConvert
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  bar2_address,
    OUT GT_U32                 *newSwitchAddrPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    GT_U32  win,regValue;
    GT_U32  baseAddr;
    GT_U32  lastAddr;
    GT_U32  addrMask;
    GT_U32  newBaseAddr;
    GT_U32  numTimes;

    *newSwitchAddrPtr = SMAIN_NOT_VALID_CNS;
    /*start from last matched index*/
    win = devMemInfoPtr->common.iAtuWindowsLastIndex;

    /* the iATU memories are in the 'synopsys PEX' unit */
    for(numTimes = 0 ; numTimes < iATU_WINDOWS_NUM ; numTimes++,win++)
    {
        win &= (iATU_WINDOWS_NUM-1);/* wraparound */

        IATU_REG_GET(iATU_REGION_CTRL_2_REG,regValue);
        if(0 == (regValue & 0x80000000))
        {
            /* the window is not enabled */
            continue;
        }

        IATU_REG_GET(iATU_LOWER_BASE_ADDRESS_REG,baseAddr);

        if(bar2_address < baseAddr)
        {
            continue;
        }

        IATU_REG_GET(iATU_LIMIT_ADDRESS_REG,lastAddr);  /* base + size - 1 */

        if(bar2_address > lastAddr)
        {
            continue;
        }

        addrMask = lastAddr - baseAddr;

        IATU_REG_GET(iATU_LOWER_TARGET_ADDRESS_REG,newBaseAddr);

        /* we have a match in the window */
        *newSwitchAddrPtr = (newBaseAddr & (~addrMask)) | /* the new base address            */
                            (bar2_address & addrMask);    /* the offset from start of window */

        devMemInfoPtr->common.iAtuWindowsLastIndex = win;

        break;
    }

    return;
}

/**
* @internal smemFalconActiveWritePciConfigBarHighAddrReg function
* @endinternal
*
* @brief   write to high address of BAR0 or BAR2 register 'pci configuration space'
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWritePciConfigBarHighAddrReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  oldValue = * memPtr;
    GT_U32  newValue = * inMemPtr;
    GT_U32  isBar2 = ((GT_U32)param == 2) ;
    GT_U32  isBar0 = ((GT_U32)param == 0) ;

    simGeneralPrintf("BAR%d changed High addr from [0x%8.8x] to [0x%8.8x] \n",
        (GT_U32)param , oldValue , newValue);

    (* memPtr) = newValue;

    /* notify CSIB */
    if(isBar0 || isBar2)
    {
        SCIB_BIND_EXT_INFO_STC bindExtInfo;

        memset(&bindExtInfo,0,sizeof(bindExtInfo));

        if(isBar0)
        {
            bindExtInfo.update_bar0_base_high = GT_TRUE;
            bindExtInfo.bar0_base_high = newValue;
        }
        else
        if(isBar2)
        {
            bindExtInfo.update_bar2_base_high = GT_TRUE;
            bindExtInfo.bar2_base_high = newValue;
        }

        scibBindExt(devObjPtr->deviceId,&bindExtInfo);
    }
}
/**
* @internal smemFalconActiveWritePciConfigBarReg function
* @endinternal
*
* @brief   write to BAR0 or BAR2 register 'pci configuration space'
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWritePciConfigBarReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  oldValue = * memPtr;
    GT_U32  newValue = * inMemPtr;
    GT_U32  isBar2 = ((GT_U32)param == 2) ;
    GT_U32  isBar0 = ((GT_U32)param == 0) ;
    GT_U32  barSize = isBar2 ? devObjPtr->bar2_size :
                      isBar0 ? devObjPtr->bar0_size : 0;

    GT_U32  newBAR;

    /* clear the non-aligned bits to the size */
    newBAR = newValue & (~(barSize-1));

    (* memPtr) = newBAR | (oldValue & 0xF);/* save the value for the 'read' ,but keep lower 4 bits AS is */

    simGeneralPrintf("BAR%d changed Low addr from [0x%8.8x] to [0x%8.8x] \n",
        (GT_U32)param , oldValue & (0xFFFFFFF0) , newBAR);

    /* notify CSIB */
    if(isBar0 || isBar2)
    {
        SCIB_BIND_EXT_INFO_STC bindExtInfo;

        memset(&bindExtInfo,0,sizeof(bindExtInfo));

        if(isBar0)
        {
            bindExtInfo.update_bar0_base = GT_TRUE;
            bindExtInfo.bar0_base = newBAR;
        }
        else
        if(isBar2)
        {
            bindExtInfo.update_bar2_base = GT_TRUE;
            bindExtInfo.bar2_base = newBAR;
        }

        scibBindExt(devObjPtr->deviceId,&bindExtInfo);
    }
}

/**
* @internal smemFalconActiveWritePciConfigBarReg___NOT_VALID function
* @endinternal
*
* @brief   function for 'non-valid' BAR : write to BAR5 or BAR5 is ignored
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWritePciConfigBarReg___NOT_VALID (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* !!!! the write is ignored !!!! */
    /* so the read will always read 0 */
}

/**
* @internal smemFalconUnitBar0 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM RFU unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitBar0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Active access bind */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* 0x00000010 - BAR0 */
        { 0x00000010 , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarReg        , 0/*indication of BAR0 - low  addr of the bar */},
        { 0x00000014 , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarHighAddrReg, 0/*indication of BAR0 - high addr of the bar */},
        /* 0x00000018 - BAR2 */
        { 0x00000018 , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarReg        , 2/*indication of BAR2 - low  addr of the bar */},
        { 0x0000001c , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarHighAddrReg, 2/*indication of BAR2 - high addr of the bar */},
        /* 0x00000020 - BAR4 */
        { 0x00000020 , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarReg___NOT_VALID, 0},
        /* 0x00000024 - BAR5 */
        { 0x00000024 , SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePciConfigBarReg___NOT_VALID, 0},




    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Address space */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000003C)}/* 0x00-0x3c : pci config space */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000005C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000094, 0x000000A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000128)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000138, 0x000001F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000208, 0x0000020C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000214, 0x0000022C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000238, 0x00000240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000248, 0x00000250)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x00000750)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000080C, 0x00000814)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000081C, 0x0000081C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000088C, 0x00000890)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000008A8, 0x000008A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000008B4, 0x000008D8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000008E0, 0x000008E8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000008F0, 0x000008FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B40, 0x00000B48)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004010, 0x0000401C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004030, 0x00004030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000407C, 0x0000407C)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    /* ATU sub unit */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /*oATU 8 windows*/
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000000, ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000018)},FORMULA_SINGLE_PARAMETER(8,0x200)}
            /*iATU 64 windows*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000100, ATU_REGISTERS_OFFSET_IN_BAR0 + 0x00000118)},FORMULA_SINGLE_PARAMETER(64,0x200)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);

    }

    /* default values */
    if(unitPtr->unitDefaultRegistersPtr == NULL)
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[]  =
        {
            /* lower 0x00 - 0x3c addresses comes from the 'PCI configuration space'  */
            /*              values from 12.8T HW device after cpssInitSystem is done */
            /* register 0 is set by function smemChtInitPciRegistres in runtime with the 'current device type' */
            /* get value from Falcon HW , from command in linux :
               hexdump /sys/bus/pci/devices/0000\:01\:00.0/config
            0000000 840011ab  00100006  02000000  00000008
            0000010 f6400004  00000000  f6000004  00000000
            0000020 00000000  00000000  00000000  11ab11ab
            0000030 00000000  00000040  00000000  000001ff
            0000040 06035001  00000008  00000000  00000000
            0000050 00807005  00000000  00000000  00000000
            0000060 00000000  00000000  00000000  00000000
            0000070 00020010  00008fc0  000b2010  00473c24
            0000080 10230040  00000000  00000000  00000000
            0000090 00000000  0001001f  00000000  0180001e
            00000a0 011e0003  00000000  00000000  00000000
            00000b0 00000000  00000000  00000000  00000000

            0000100 14820001  00100000  00400000  00462030
            0000110 0000a000  0000e000  000000b4  40000001
            0000120 0000000f  f6600000  400028f0  00000000
            0000130 00000000  00000000  00000000  00000000
            0000140 00000000  00000000  15810019  00000000
            0000150 00000000  55005500  17c10026  00000000
            0000160 00000000  00000000  00000000  00000000
            0000170 00000000  00000000  0000f0f0  18c10027
            0000180 00000000  00009c38  00009c38  19c1001e
            0000190 00280a1b  00000000  00000028  0001000b
            00001a0 10040002  00000000  00000000  00000100
            00001b0 000007d0  00000000  00000000  00000000
            00001c0 00000000  00000000  00000000  00000000
            */

              {DUMMY_NAME_PTR_CNS,            0x00000000,       0x11ab  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000004,   0x00100006  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000008,   0x02000000  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000000c,   0x00000008  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000010,   0x00000004  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000018,   0x00000004  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000002c,   0x11ab11ab  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000034,   0x00000040  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000003c,   0x000001ff  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000040,   0x06035001  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000044,   0x00000008  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000050,   0x00807005  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000070,   0x00020010  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000074,   0x00008fc0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000078,   0x000b2010  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000007c,   0x00473c24  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000080,   0x10230040  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000094,   0x0001001f  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000009c,   0x0180001e  }
             ,{DUMMY_NAME_PTR_CNS,            0x000000a0,   0x011e0003  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000100,   0x14820001  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000104,   0x00100000  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000108,   0x00400000  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000010c,   0x00462030  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000110,   0x0000a000  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000114,   0x0000e000  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000118,   0x000000b4  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000011c,   0x40000001  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000120,   0x0000000f  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000124,   0xf6600000  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000128,   0x400028f0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000148,   0x15810019  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000154,   0x55005500  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000158,   0x17c10026  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000178,   0x0000f0f0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000017c,   0x18c10027  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000184,   0x00009c38  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000188,   0x00009c38  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000018c,   0x19c1001e  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000190,   0x00280a1b  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000198,   0x00000028  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000019c,   0x0001000b  }
             ,{DUMMY_NAME_PTR_CNS,            0x000001a0,   0x10040002  }
             ,{DUMMY_NAME_PTR_CNS,            0x000001ac,   0x00000100  }
             ,{DUMMY_NAME_PTR_CNS,            0x000001b0,   0x000007d0  }

             /* next are from Hawk Cider */

             ,{DUMMY_NAME_PTR_CNS,            0x00000218,         0x00180000,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x0000021c,         0x00000200,      1,    0x0  }
             ,{DUMMY_NAME_PTR_CNS,            0x00000224,         0x00fff000,      1,    0x0  }
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

/*0x0000020C + n*8: where n (0-5) represents BA*/
#define MG_ADDR_DECODE_WIN_BASE_ADDRESS_GET(value) value = *(devMemInfoPtr->common.mgSdmaWindowsMemArr[mgUnit].sdmaAddrDecoding[win].base_address)
/*0x00000210 + n*8: where n (0-5) represents BA*/
#define MG_ADDR_DECODE_WIN_SIZE_GET(value)         value = *(devMemInfoPtr->common.mgSdmaWindowsMemArr[mgUnit].sdmaAddrDecoding[win].size)
/*0x00000254 + n*4: where n (0-5) represents BA*/
#define MG_ADDR_DECODE_WIN_CONTROL_GET(value)      value = *(devMemInfoPtr->common.mgSdmaWindowsMemArr[mgUnit].sdmaAddrDecoding[win].window_control)

/*******************************************************************************
*   smemFalconDmaMgConvert
*
* DESCRIPTION:
*       function to do 'address convert' from address of 32bits that the registers of DMA
*       hold to be converted to 'pex addresses' , using the MG windows.
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       dma_address  - address of 32bits that the registers of DMA hold to be converted to 'pex addresses'
* OUTPUTS:
*       newMgAddr_lowAddr_Ptr   - (pointer to) the lower  32 bits of 'PEX address' that associated with the dma_address
*       newMgAddr_highAddr_Ptr  - (pointer to) the higher 32 bits of 'PEX address' that associated with the dma_address
*       xbarTargetLegPtr        - (pointer to) the MG XBAR target 'leg' to continue the 'address decoding'
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemFalconDmaMgConvert(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  dma_address,
    OUT GT_U32                 *newMgAddr_lowAddr_Ptr,
    OUT GT_U32                 *newMgAddr_highAddr_Ptr,
    OUT GT_U32                 *xbarTargetLegPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    GT_U32  mgUnit = smemGetCurrentMgUnitIndex(devObjPtr);
    GT_U32  controlRegValue,baseRegValue,sizeRegValue;
    GT_U32  Window_Remap_Enable;
    GT_U32  win;

    *newMgAddr_highAddr_Ptr = 0;

    /* the SDMA windows in the MG unit */
    for(win = 0 ; win < MG_SDMA_WINDOWS_NUM ; win++)
    {
        MG_ADDR_DECODE_WIN_CONTROL_GET(controlRegValue);
        if(controlRegValue & 0x1)
        {
            /* the window is not enabled ! (value 1 means 'NOT enabled') */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("MG DMA skip window[%d] (window is not enabled) \n",
            win));
            continue;
        }

        MG_ADDR_DECODE_WIN_BASE_ADDRESS_GET(baseRegValue);
        *xbarTargetLegPtr  = SMEM_U32_GET_FIELD(baseRegValue,0,4);

        MG_ADDR_DECODE_WIN_SIZE_GET(sizeRegValue);
        /*
            An address driven by one of the Unit's masters is considered as a window hit if:
            ((address | size) == (base | size).
        */
        if((dma_address/*upper 16 bits*/ | sizeRegValue/*upper 16 bits*/ | 0xFFFF/*lower 16 bits*/) !=
          (baseRegValue/*upper 16 bits*/ | sizeRegValue/*upper 16 bits*/ | 0xFFFF/*lower 16 bits*/))
        {
            /* not matched */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("MG DMA not matched : dma_address[0x%8.8x] on window[%d] \n",
            dma_address,
            win));
            continue;
        }

        Window_Remap_Enable = SMEM_U32_GET_FIELD(controlRegValue,3,1);
        if(!Window_Remap_Enable)
        {
            /* address kept 'as is' */
            *newMgAddr_lowAddr_Ptr  = dma_address;
            *newMgAddr_highAddr_Ptr = 0;
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("MG DMA NO convert : dma_address[0x%8.8x] matched window[%d] \n",
            dma_address,
            win));
            return;
        }

        /* we need to replace the 'base' of the dma_addr */
        /* Remap Address
            Used with the <Size> field to specify address bits[31:0] to be driven to the target interface.
            target_addr[31:16] = (addr[31:16] & size[15:0]) | (remap[15:0] & ~size[15:0])
        */

        *newMgAddr_lowAddr_Ptr  = (0xFFFF0000 & ((dma_address & sizeRegValue) | (controlRegValue & (~sizeRegValue)))) |
                                  (0x0000FFFF &   dma_address )  ;

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("MG DMA convert : dma_address[0x%8.8x] to addr_high[0x%8.8x],addr_low[0x%8.8x] (window[%d])\n",
            dma_address,
            *newMgAddr_highAddr_Ptr,
            *newMgAddr_lowAddr_Ptr,
            win));
        return;

    }

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("MG DMA : ERROR : not matched : dma_address[0x%8.8x] in any window \n",
    dma_address));

    *newMgAddr_lowAddr_Ptr = SMAIN_NOT_VALID_CNS;

    return;
}

/*******************************************************************************
*   smemFalconCm3MgConvert
*
* DESCRIPTION:
*       function for CM3 to do 'address convert' from address of 32 bits that the registers of DDR
*       hold to be converted to 'pex addresses' , using the CN3 MG windows.
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       ddr_address  - address of 32bits that the registers of DDR hold to be converted to 'pex addresses'
* OUTPUTS:
*       newMgAddr_lowAddr_Ptr   - (pointer to) the lower  32 bits of 'PEX address' that associated with the ddr_address
*       newMgAddr_highAddr_Ptr  - (pointer to) the higher 32 bits of 'PEX address' that associated with the ddr_address
*       xbarTargetLegPtr        - (pointer to) the MG XBAR target 'leg' to continue the 'address decoding'
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemFalconCm3MgConvert(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  dma_address,
    OUT GT_U32                 *newMgAddr_lowAddr_Ptr,
    OUT GT_U32                 *newMgAddr_highAddr_Ptr,
    OUT GT_U32                 *xbarTargetLegPtr
)
{
    GT_U32  controlRegValue,baseRegValue,sizeRegValue,highAddressRemapValue;
    GT_U32  Window_Remap_Enable;
    GT_U32  win;
    GT_U32  cm3_window_base,cm3_window_last;

    *newMgAddr_highAddr_Ptr = 0;
/*  ALL CM3 :
    0x00000000 -- 0x5fffffff: SRAM
    0x60000000 -- 0x7fffffff: MG0
    0x80000000 -- 0x9fffffff: MG0 moving window (with completion register)
    0xa0000000 -- 0xa00fffff: RUnit, internal access
    0xa0100000 -- 0xdfffffff: CM3 windows
*/
    cm3_window_base = 0xa0100000;
    cm3_window_last = 0xdfffffff;

    if((dma_address < cm3_window_base) || (dma_address > cm3_window_last))
    {
        /* no mapping */
        *newMgAddr_lowAddr_Ptr = SMAIN_NOT_VALID_CNS;
        return;
    }

    /* addresses in the window mapping are relative to the 'cm3_window_base' */
    dma_address -= cm3_window_base;

    /* the oATU memories are in the 'synopsys PEX' unit */
    for(win = 0 ; win < CM3_PCIE_WINDOWS_NUM ; win++)
    {
        smemRegGet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG.CM3.addressDecoding[win].CM3ExternalWindowControl,
            &controlRegValue);
        if(controlRegValue & 0x1)
        {
            /* the window is not enabled ! (value 1 means 'NOT enabled') */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("CM3 DMA skip window[%d] (window is not enabled) \n",
            win));
            continue;
        }

        smemRegGet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG.CM3.addressDecoding[win].CM3ExternalBaseAddress,
            &baseRegValue);
        smemRegGet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG.CM3.addressDecoding[win].CM3ExternalSize,
            &sizeRegValue);

        *xbarTargetLegPtr  = SMEM_U32_GET_FIELD(baseRegValue,0,4);
        /*
            An address driven by one of the Unit's masters is considered as a window hit if:
            ((address | size) == (base | size).
        */
        if((dma_address/*upper 16 bits*/ | sizeRegValue/*upper 16 bits*/ | 0xFFFF/*lower 16 bits*/) !=
          (baseRegValue/*upper 16 bits*/ | sizeRegValue/*upper 16 bits*/ | 0xFFFF/*lower 16 bits*/))
        {
            /* not matched */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("CM3 DMA not matched : dma_address[0x%8.8x] on window[%d] \n",
            dma_address + cm3_window_base,
            win));
            continue;
        }

        Window_Remap_Enable = SMEM_U32_GET_FIELD(controlRegValue,3,1);
        if(!Window_Remap_Enable)
        {
            /* address kept 'as is' */
            *newMgAddr_lowAddr_Ptr  = dma_address + cm3_window_base;
            *newMgAddr_highAddr_Ptr = 0;

            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("CM3 DMA NO convert : dma_address[0x%8.8x] matched window[%d] \n",
            dma_address + cm3_window_base,
            win));
            return;
        }

        smemRegGet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG.CM3.addressDecoding[win].CM3ExternalHighAddressRemap,
            &highAddressRemapValue);
        /* we need to replace the 'base' of the dma_addr */
        /* Remap Address
            Used with the <Size> field to specify address bits[31:0] to be driven to the target interface.
            target_addr[31:16] = (addr[31:16] & size[15:0]) | (remap[15:0] & ~size[15:0])
        */

        *newMgAddr_lowAddr_Ptr  = (0xFFFF0000 & ((dma_address & sizeRegValue) | (controlRegValue & (~sizeRegValue)))) |
                                  (0x0000FFFF &   dma_address )  ;

        *newMgAddr_highAddr_Ptr = highAddressRemapValue;

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("CM3 DMA convert : dma_address[0x%8.8x] to addr_high[0x%8.8x],addr_low[0x%8.8x] (window[%d])\n",
            dma_address + cm3_window_base,
            *newMgAddr_highAddr_Ptr,
            *newMgAddr_lowAddr_Ptr,
            win));

        return;

    }

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("CM3 DMA : ERROR : not matched : dma_address[0x%8.8x] in any window \n",
    dma_address + cm3_window_base));

    *newMgAddr_lowAddr_Ptr = SMAIN_NOT_VALID_CNS;

    return;
}

/*************cnm addr decoder registers*************/
#define CNM_ADDR_DECODE_WINDOW_ATTRIBUTES_AND_SIZE_HIGH   0x100  /*+ i*0x18: where i (0-15) represents Window number*/
#define CNM_ADDR_DECODE_WINDOW_SIZE_LOW                   0x104  /*+ i*0x18: where i (0-15) represents Window number*/
#define CNM_ADDR_DECODE_WINDOW_BASE_AND_REMAP_HIGH        0x108  /*+ i*0x18: where i (0-15) represents Window number*/
#define CNM_ADDR_DECODE_WINDOW_BASE_LOW                   0x10C  /*+ i*0x18: where i (0-15) represents Window number*/
#define CNM_ADDR_DECODE_WINDOW_REMAP_LOW                  0x110  /*+ i*0x18: where i (0-15) represents Window number*/
#define CNM_ADDR_DECODE_AXI_ATTRIBUTES                    0x114  /*+ i*0x18: where i (0-15) represents Window number*/

#define CNM_ADDR_DECODE_REG_GET(addr,value) value = *(devMemInfoPtr->common.cnmAddrDecodeAmb3WindowsMemPtr[win]+((addr-CNM_ADDR_DECODE_WINDOW_ATTRIBUTES_AND_SIZE_HIGH)/4))
#define AMB3    3

/*******************************************************************************
*   smemPhoenixSdmaCnMConvert
*
* DESCRIPTION:
*       function to do 'address convert' from address of 32bits that the registers of DMA
*       hold to be converted to 'pex addresses' , using the CnM 'address decoding' unit windows.
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       newMgAddr_lowAddr   - the lower  32 bits of 'PEX address' that associated with the dma_address
*       newMgAddr_highAddr  - the higher 32 bits of 'PEX address' that associated with the dma_address
*       xbarTargetLeg       - the MG XBAR target 'leg' to continue the 'address decoding'
* OUTPUTS:
*       newPexAddr_lowAddr_Ptr   - (pointer to) the lower  32 bits of 'PEX address' that associated with the dma_address
*       newPexAddr_highAddr_Ptr  - (pointer to) the higher 32 bits of 'PEX address' that associated with the dma_address
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemPhoenixSdmaCnMConvert(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  mgAddr_lowAddr,
    IN GT_U32                  mgAddr_highAddr,
    IN GT_U32                  xbarTargetLeg,
    OUT GT_U32                 *newPexAddr_lowAddr_Ptr,
    OUT GT_U32                 *newPexAddr_highAddr_Ptr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;/*used in CNM_ADDR_DECODE_REG_GET*/
    GT_U32  win;
    GT_U32  window_attributes_and_size_high,window_base_and_remap_high,window_base_low;
    GT_U32  window_size_low,window_remap_low;
    GT_U32  mgAddr16_47; /*bits 16..47 from MG address */
    GT_U32  mgAddr48_63; /*bits 48..63 from MG address */
    GT_U32  remapHighValue,highBaseValue;
    GT_U32  new_addr16_47; /*new address bits 16..47 */
    GT_U32  new_addr48_63; /*new address bits 48..63 */

    if(AMB3 != xbarTargetLeg)
    {
        /* to support more 'lags' need to enlarge cnmAddrDecodeAmb3WindowsMemPtr from 'single' to '9 clients':
            CPU
            CoreSight Trace
            AMB2
            AMB3
            iNIC
            XOR0
            XOR1
            PCIe
            GIC
        */
        skernelFatalError("smemPhoenixSdmaCnMConvert : not implemented AMB other than 'AMB3' ");
        return;
    }
                      /*bits 32:47 */           /*bits 16..31*/
    mgAddr16_47 = (mgAddr_highAddr << 16) | (mgAddr_lowAddr >> 16);
    mgAddr48_63 =  mgAddr_highAddr >> 16;

    /* the oATU memories are in the 'synopsys PEX' unit */
    for(win = 0 ; win < CNM_ADDR_DECODE_WINDOWS_NUM ; win++)
    {
        CNM_ADDR_DECODE_REG_GET(CNM_ADDR_DECODE_WINDOW_ATTRIBUTES_AND_SIZE_HIGH,window_attributes_and_size_high);

        if(0 == (window_attributes_and_size_high & 0x1))
        {
            /* the window is not enabled */
            continue;
        }

        CNM_ADDR_DECODE_REG_GET(CNM_ADDR_DECODE_WINDOW_BASE_AND_REMAP_HIGH,window_base_and_remap_high);
        /*high_base : bits 48:63*/
        highBaseValue = (window_base_and_remap_high & 0xFFFF);

        if(mgAddr48_63 < highBaseValue)
        {
            /* below high prefix */
            continue;
        }

                           /*high_base : bits 48:63*/
        if(mgAddr48_63 > highBaseValue +
                         (window_attributes_and_size_high >> 16)/*high_size*/)
        {
            /* above high prefix + high size */
            continue;
        }

        CNM_ADDR_DECODE_REG_GET(CNM_ADDR_DECODE_WINDOW_BASE_LOW,window_base_low);

        if(mgAddr48_63 == (window_base_and_remap_high & 0xFFFF))
        {
            if(mgAddr16_47 < window_base_low)
            {
                /* below low prefix */
                continue;
            }

            CNM_ADDR_DECODE_REG_GET(CNM_ADDR_DECODE_WINDOW_SIZE_LOW,window_size_low);

            if(mgAddr16_47 > (window_base_low + window_size_low))
            {
                /* below high prefix + high size */
                continue;
            }
        }


        /* found proper 'window' that matched the MG address */

        CNM_ADDR_DECODE_REG_GET(CNM_ADDR_DECODE_WINDOW_REMAP_LOW,window_remap_low);

        remapHighValue = window_base_and_remap_high >> 16;

        new_addr16_47 = (mgAddr16_47 - window_base_low) + window_remap_low;
        new_addr48_63 = (mgAddr48_63 - highBaseValue)   + remapHighValue;

        /* we have a match in the window */
        *newPexAddr_lowAddr_Ptr  = (mgAddr_lowAddr & 0xFFFF)/* bits 0..15*/ |
                                   (new_addr16_47 << 16);/*bits16..31*/
        *newPexAddr_highAddr_Ptr = (new_addr16_47 >> 16)/* bits 32..47*/ |
                                    new_addr48_63 << 16;/* bits 48..63*/

        return;
    }

    *newPexAddr_lowAddr_Ptr  = SMAIN_NOT_VALID_CNS;
    *newPexAddr_highAddr_Ptr = SMAIN_NOT_VALID_CNS;

    return;
}

/*******************************************************************************
*   smemFalconCnMoAtu
*
* DESCRIPTION:
*       function to do 'address completion' from address of 32bits coming from the CM3/MG-sdma
*       hold to be converted to 'pex addresses' , for that using the oATU in the CnM
*
* INPUTS:
*       devObjPtr        - pointer to device object.
*       mgXbarTargetLeg  - the MG XBAR target 'leg' to continue the 'address decoding'
*                           relevant to 'devObjPtr->supportSdmaCnMConvert' device (Phoenix/Ac5)
* OUTPUTS:
*       pexAddr_lowAddr_Ptr   - (pointer to) the lower  32 bits of 'PEX address'
*       pexAddr_highAddr_Ptr  - (pointer to) the higher 32 bits of 'PEX address'
*
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemFalconCnMoAtu(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  mgXbarTargetLeg,
    INOUT GT_U32                 *pexAddr_lowAddr_Ptr,
    INOUT GT_U32                 *pexAddr_highAddr_Ptr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;/*used in OATU_REG_GET*/
    GT_U32  win,regValue;
    GT_U32  atu_baseAddr_low,atu_baseAddr_high;
    GT_U32  lastAddr;
    GT_U32  addrMask;
    GT_U32  cnm_baseAddr_low,cnm_baseAddr_high;/* CnM addresses after CnM windows conversion */
    GT_U32  newBaseAddr_low,newBaseAddr_high;/* final addresses after the oATU conversion */

    if(devObjPtr->supportSdmaCnMConvert)/* Phoenix */
    {
        /* hold another 'address decoding' in the CnM : <CNM_IP>CNM_IP/<Address_Decoder> Address Decoder */

        /* additional CnM conversion */
        smemPhoenixSdmaCnMConvert(devObjPtr,*pexAddr_lowAddr_Ptr,*pexAddr_highAddr_Ptr,mgXbarTargetLeg,
            pexAddr_lowAddr_Ptr,pexAddr_highAddr_Ptr);
    }

    cnm_baseAddr_low  = *pexAddr_lowAddr_Ptr;
    cnm_baseAddr_high = *pexAddr_highAddr_Ptr;

    if(SMAIN_NOT_VALID_CNS == cnm_baseAddr_low)
    {
        /* the mapping not gone well */
        return;
    }


    /* the oATU memories are in the 'synopsys PEX' unit */
    for(win = 0 ; win < oATU_WINDOWS_NUM ; win++)
    {
        OATU_REG_GET(oATU_REGION_CTRL_2_REG,regValue);

        if(0 == (regValue & 0x80000000))
        {
            /* the window is not enabled */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu skip window[%d] (window is not enabled) \n",
            win));
            continue;
        }

        OATU_REG_GET(oATU_UPPER_BASE_ADDRESS_REG,atu_baseAddr_high);

        if(cnm_baseAddr_high != atu_baseAddr_high)
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu NO matched : cnm_baseAddr_high[0x%8.8x] not match on window[%d] \n",
            cnm_baseAddr_high,
            win));
            continue;
        }

        OATU_REG_GET(oATU_LOWER_BASE_ADDRESS_REG,atu_baseAddr_low);

        if(cnm_baseAddr_low < atu_baseAddr_low)
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu NO match : cnm_baseAddr_low[0x%8.8x] below the range in window[%d] \n",
            cnm_baseAddr_low,
            win));
            continue;
        }

        OATU_REG_GET(oATU_LIMIT_ADDRESS_REG,lastAddr);  /* base + size - 1 */

        if(lastAddr & atu_baseAddr_low & 0x80000000)
        {
            /* bit 31 set on both of them , all addresses above atu_baseAddr_low are allowed */
        }
        else
        if(cnm_baseAddr_low > (atu_baseAddr_low + lastAddr))
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu NO match : cnm_baseAddr_low[0x%8.8x] over the range in window[%d] \n",
            cnm_baseAddr_low,
            win));
            continue;
        }

        addrMask = (lastAddr - atu_baseAddr_low) - 1;

        OATU_REG_GET(oATU_LOWER_TARGET_ADDRESS_REG,newBaseAddr_low);

        OATU_REG_GET(oATU_UPPER_TARGET_ADDRESS_REG,newBaseAddr_high);

        /* we have a match in the window */
        *pexAddr_lowAddr_Ptr  = (newBaseAddr_low & (~addrMask)) | /* the new base address            */
                                (cnm_baseAddr_low & addrMask);     /* the offset from start of window */
        *pexAddr_highAddr_Ptr = newBaseAddr_high;

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu convert : cnm_baseAddr_high[0x%8.8x] cnm_baseAddr_low[0x%8.8x] to addr_high[0x%8.8x],addr_low[0x%8.8x] (window[%d])\n",
            cnm_baseAddr_high,
            cnm_baseAddr_low,
            *pexAddr_highAddr_Ptr,
            *pexAddr_lowAddr_Ptr,
            win));
        return;
    }

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("oAtu ERROR : not matched : cnm_baseAddr_high[0x%8.8x] cnm_baseAddr_low[0x%8.8x] in any window \n",
        cnm_baseAddr_high,
        cnm_baseAddr_low));
    return;
}
/*******************************************************************************
*   smemFalconDmaCnMoAtuConvert
*
* DESCRIPTION:
*       function to do 'address completion' from address of 32bits that the registers of DMA
*       hold to be converted to 'pex addresses' , for that using the MG windows and the oATU in the CnM
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       client       - the client that ask to access the DMA (like: SDMA/CM3)
*       dma_address  - address of 32bits that the registers of DMA hold to be converted to 'pex addresses'
* OUTPUTS:
*       newPexAddr_lowAddr_Ptr   - (pointer to) the lower  32 bits of 'PEX address' that associated with the dma_address
*       newPexAddr_highAddr_Ptr  - (pointer to) the higher 32 bits of 'PEX address' that associated with the dma_address
*
* RETURNS:
*        None
*
* COMMENTS:
*
*
*******************************************************************************/
static void smemFalconDmaCnMoAtuConvert(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_DMA_CNM_OATU_CLIENT_ENT client,
    IN GT_U32                  dma_address,
    OUT GT_U32                 *newPexAddr_lowAddr_Ptr,
    OUT GT_U32                 *newPexAddr_highAddr_Ptr
)
{
    GT_U32  mg_baseAddr_low,mg_baseAddr_high;/* MG addresses after MG windows conversion  */
    GT_U32  mgXbarTargetLeg = 0;/*the MG XBAR target 'leg' to continue the 'address decoding'*/

    *newPexAddr_lowAddr_Ptr  = SMAIN_NOT_VALID_CNS;
    *newPexAddr_highAddr_Ptr = SMAIN_NOT_VALID_CNS;

    if(client == SMEM_DMA_CNM_OATU_CLIENT_SDMA_E)
    {
        /* first need MG windows for SDMA conversion */
        smemFalconDmaMgConvert(devObjPtr,dma_address,&mg_baseAddr_low,&mg_baseAddr_high,&mgXbarTargetLeg);
    }
    else
    {
        /* first need MG windows for CM3 conversion */
        smemFalconCm3MgConvert(devObjPtr,dma_address,&mg_baseAddr_low,&mg_baseAddr_high,&mgXbarTargetLeg);
    }

    if(AMB3 != mgXbarTargetLeg)
    {
        /* not supported */
        *newPexAddr_lowAddr_Ptr  = SMAIN_NOT_VALID_CNS;
        *newPexAddr_highAddr_Ptr = SMAIN_NOT_VALID_CNS;
        return;
    }

    *newPexAddr_lowAddr_Ptr  = mg_baseAddr_low;
    *newPexAddr_highAddr_Ptr = mg_baseAddr_high;

    smemFalconCnMoAtu(devObjPtr,mgXbarTargetLeg,newPexAddr_lowAddr_Ptr,newPexAddr_highAddr_Ptr);

    return;
}
static char* mgUnitNames[16] = {
    STR(UNIT_MG    )
    ,STR(UNIT_MG_0_1)
    ,STR(UNIT_MG_0_2)
    ,STR(UNIT_MG_0_3)

    ,STR(UNIT_MG_1_0)
    ,STR(UNIT_MG_1_1)
    ,STR(UNIT_MG_1_2)
    ,STR(UNIT_MG_1_3)

    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG    ),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_0_1),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_0_2),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_0_3),2)

    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_1_0),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_1_1),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_1_2),2)
    ,ADD_INSTANCE_OF_TILE_TO_STR(STR(UNIT_MG_1_3),2)
};

static char* addrDecodeUnitNames[9] = {
     STR(UNIT_CNM_ADDR_DECODER  )
    ,STR(UNIT_CNM_ADDR_DECODER_1)
    ,STR(UNIT_CNM_ADDR_DECODER_2)
    ,STR(UNIT_CNM_ADDR_DECODER_3)
    ,STR(UNIT_CNM_ADDR_DECODER_4)
    ,STR(UNIT_CNM_ADDR_DECODER_5)
    ,STR(UNIT_CNM_ADDR_DECODER_6)
    ,STR(UNIT_CNM_ADDR_DECODER_7)
    ,STR(UNIT_CNM_ADDR_DECODER_8)
};

/**
* @internal smemFalconBar0Bar2MemInit function
* @endinternal
*
* @brief   init BAR0,BAR2 related parameters. NOTE: the actual memory space should
*          already been done by regular 'unit allocation'
*
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemFalconBar0Bar2MemInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  win,mgId;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    GT_U32  baseAddr;

    devObjPtr->bar0_size = _1M;/* constant not configurable */
    devObjPtr->bar2_size = _4M;/* default value. TBD active memory to update the value to SCIB via scibBindExt */

    SET_IF_ZERO_MAC(devMemInfoPtr->common.bar2CnMiAtuFunc       ,smemFalconBar2CnMiAtuConvert);
    SET_IF_ZERO_MAC(devMemInfoPtr->common.dmaCnMoAtuFunc        ,smemFalconDmaCnMoAtuConvert );

    /* iATU 64 windows */
    for(win = 0 ; win < iATU_WINDOWS_NUM ; win++)
    {
        devMemInfoPtr->common.iAtuWindowsMemPtr[win] =
            smemGenericFindMem(devObjPtr,SKERNEL_MEMORY_READ_BAR0_E,
                iATU_REGISTER_ADDRESS(win,iATU_REGION_CTRL_1_REG),
                1,NULL/*no active memory needed*/);
    }

    /* oATU 8 windows */
    for(win = 0 ; win < oATU_WINDOWS_NUM ; win++)
    {
        devMemInfoPtr->common.oAtuWindowsMemPtr[win] =
            smemGenericFindMem(devObjPtr,SKERNEL_MEMORY_READ_BAR0_E,
                oATU_REGISTER_ADDRESS(win,oATU_REGION_CTRL_1_REG),
                1,NULL/*no active memory needed*/);
    }

    /* per MG 6 windows */
    for(mgId = 0 ; mgId < devObjPtr->numOfMgUnits; mgId++)
    {
        /*per MG base address*/
        baseAddr = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,mgUnitNames[mgId]);
        for(win = 0 ; win < MG_SDMA_WINDOWS_NUM ; win++)
        {
            devMemInfoPtr->common.mgSdmaWindowsMemArr[mgId].sdmaAddrDecoding[win].base_address =
                smemMemGet(devObjPtr,baseAddr + 0x0000020C + win*8);

            devMemInfoPtr->common.mgSdmaWindowsMemArr[mgId].sdmaAddrDecoding[win].size =
                smemMemGet(devObjPtr,baseAddr + 0x00000210 + win*8);

            devMemInfoPtr->common.mgSdmaWindowsMemArr[mgId].sdmaAddrDecoding[win].window_control =
                smemMemGet(devObjPtr,baseAddr + 0x00000254 + win*4);
        }/*win*/
    }/*mgId*/

    if(devObjPtr->supportSdmaCnMConvert)/* Phoenix */
    {
        /* CnM address decode AMB3 windows */
        baseAddr = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,addrDecodeUnitNames[AMB3]);

        /* CnM address decode AMB3 windows */
        for(win = 0 ; win < CNM_ADDR_DECODE_WINDOWS_NUM ; win++)
        {
            devMemInfoPtr->common.cnmAddrDecodeAmb3WindowsMemPtr[win] =
                smemMemGet(devObjPtr,baseAddr + CNM_ADDR_DECODE_WINDOW_ATTRIBUTES_AND_SIZE_HIGH + win*0x18);
        }
    }


    return;
}

/**
* @internal smemFalconActiveWriteBar2Size function
* @endinternal
*
* @brief   write a register that allow to resize BAR2
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWriteBar2Size (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  oldValue = * memPtr;
    GT_U32  newValue = * inMemPtr;
    GT_U32  isBar2 = ((GT_U32)param == 2) ;
    GT_U32  ii,numBits;
    GT_U32  checkValue = newValue + 1;

    /* save the value */
    (* memPtr) = * inMemPtr;

    numBits = 0;
    for(ii = 0 ; ii < 32; ii++)
    {
        if(checkValue & (1<<ii))
        {
            numBits++;
        }
    }

    if(numBits != 1)
    {
        simGeneralPrintf("ERROR : BAD BAR%d 'mask + 1'=[0x%8.8x] (must hold single bit but hold [%d]bits)\n",
            (GT_U32)param , checkValue , numBits);

        return;/* not update the SCIB about wrong value */
    }

    simGeneralPrintf("BAR%d size changed from [%d MB] to [%d MB] \n",
        (GT_U32)param , (oldValue+1) >>20 , checkValue>>20);


    if(devObjPtr->deviceForceBar0Bar2)
    {
        SCIB_BIND_EXT_INFO_STC bindExtInfo;

        memset(&bindExtInfo,0,sizeof(bindExtInfo));

        if(isBar2)
        {
            bindExtInfo.update_bar2_size = GT_TRUE;
            bindExtInfo.bar2_size = devObjPtr->bar2_size = checkValue;
        }

        scibBindExt(devObjPtr->deviceId,&bindExtInfo);
    }
}
/**
* @internal smemFalconUnitCnmPexMacUnit function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM PEX MAX unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemFalconUnitCnmPexMacUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* Active access bind */
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* mirror the first 4K byte of PCI_e mac from BAR0 */
        /* /Eagle/CnM/<SW_CNM>CNM_IP/<SW_CNM> CNM_RFU/Units/CNM_Misc/System Soft Reset */
        {0x00000000 , 0xFFFFC000, smemFalconActiveReadPci_e_ViaBar0, 0, smemFalconActiveWritePci_e_ViaBar0, 0},
        {0x00004018/*0x1c164018*/ , SMEM_FULL_MASK_CNS, NULL,  0, smemFalconActiveWriteBar2Size, 2/*indication of BAR2*/},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Address space */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000FFFF)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* defaults after the first 4K that comes from : smemFalconUnitBar0(...) */
             {DUMMY_NAME_PTR_CNS,           0x00004018,         0x003fffff,      1,    0x0  }

            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

static void smemFalconUnitCnmSramUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    /* Address space */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
           {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 65536)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

}


/**
* @internal smemFalconSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemFalconSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_TX_FIFO)      ,smemFalconUnitTxFifo}
        ,{STR(UNIT_TX_DMA)      ,smemFalconUnitTxDma}
        ,{STR(UNIT_RX_DMA)       ,smemFalconUnitRxDma}
        ,{STR(UNIT_TXQ_PDX_PAC_0)        ,smemFalconUnitTxqPdxPac}
        ,{STR(UNIT_TXQ_PDX_PAC_1)        ,smemFalconUnitTxqPdxPac}
        ,{STR(UNIT_TXQ_PDX)        ,smemFalconUnitTxqPdx}
        ,{STR(UNIT_TXQ_PFCC)        ,smemFalconUnitTxqPfcc}
        ,{STR(UNIT_TXQ_PSI)        ,smemFalconUnitTxqPsi}
        ,{STR(UNIT_TXQ_PDS0)       ,smemFalconUnitTxqPds}
        ,{STR(UNIT_TXQ_SDQ0)       ,smemFalconUnitTxqSdq}
        ,{STR(UNIT_TXQ_QFC0)       ,smemFalconUnitTxqQfc}
        ,{STR(UNIT_BMA)            ,smemFalconUnitBma}
        ,{STR(UNIT_HBU)            ,smemFalconUnitHbu}
        ,{STR(UNIT_PB_CENTER_BLK)                   ,smemFalconUnitPacketBuffer_pbCenter}
        ,{STR(UNIT_PB_WRITE_ARBITER)                ,smemFalconUnitPacketBuffer_smbWriteArbiter}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)       ,smemFalconUnitPacketBuffer_packetWrite}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_1)       ,smemFalconUnitPacketBuffer_packetWrite}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_0)        ,smemFalconUnitPacketBuffer_packetRead}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_1)        ,smemFalconUnitPacketBuffer_packetRead}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_2)        ,smemFalconUnitPacketBuffer_packetRead}
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_3)        ,smemFalconUnitPacketBuffer_packetRead}
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_0)          ,smemFalconUnitPacketBuffer_cellRead}
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_1)          ,smemFalconUnitPacketBuffer_cellRead}
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_2)          ,smemFalconUnitPacketBuffer_cellRead}
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_3)          ,smemFalconUnitPacketBuffer_cellRead}
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_0)          ,smemFalconUnitPacketBuffer_npmMc}
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_1)          ,smemFalconUnitPacketBuffer_npmMc}
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_2)          ,smemFalconUnitPacketBuffer_npmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0_0)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0_1)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_1_0)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_1_1)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_2_0)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_2_1)          ,smemFalconUnitPacketBuffer_sbmMc}
        ,{STR(UNIT_EAGLE_D2D)                       ,smemFalconUnitEagleD2d}

        ,{STR(UNIT_BASE_RAVEN_0),smemFalconUnitRaven}

        ,{STR(UNIT_DFX_SERVER)  ,smemFalconUnitDfxServer}
        ,{STR(UNIT_CNM_RFU)     ,smemFalconUnitCnmRfu}
        ,{STR(UNIT_CNM_AAC)     ,smemFalconUnitCnmAac}
        ,{STR(UNIT_CNM_PEX_MAC) ,smemFalconUnitCnmPexMacUnit}
        ,{STR(UNIT_CNM_SRAM) ,smemFalconUnitCnmSramUnit}

        ,{STR(UNIT_IA)          ,smemFalconUnitIa}
        ,{STR(UNIT_CNM_PEX_COMPHY) ,smemFalconUnitCnmPexComphyUnit}

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    smemBobcat3SpecificDeviceUnitAlloc_DP_units(devObjPtr);
}


/**
* @internal smemFalconPexAndBar0DeviceUnitAlloc function
* @endinternal
*
* @brief   allocate 'PEX config space' and 'BAR0' -- if not allocated already
* @param[in] devObjPtr                - pointer to device object.
*
*/
void smemFalconPexAndBar0DeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;

    currUnitChunkPtr = &devMemInfoPtr->PEX_UnitMem;
    /* generate memory space for PEX server */
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemFalconUnitPexConfigSpace(devObjPtr,currUnitChunkPtr);
    }
    currUnitChunkPtr = &devMemInfoPtr->BAR0_UnitMem;
    /* generate memory space for BAR0 */
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemFalconUnitBar0(devObjPtr,currUnitChunkPtr);
    }
}

/**
* @internal smemFalconSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemFalconSpecificDeviceUnitAlloc
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
        SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &falcon_units[0];

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
    smemFalconPexAndBar0DeviceUnitAlloc(devObjPtr);
    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    smemFalconSpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemFalconSpecificDeviceUnitAlloc_SIP_units(devObjPtr);

}



/**
* @internal falcon_specialSingleInstanceAddressInfoGet function
* @endinternal
*
* @brief   the function check if the address (expected to be in Pipe0) belongs to
*         special memory space.
*         NOTE: currently implemented only for tables because the ARP/TS table is
*         shared between 2 units (although the HA unit is duplicated to 2 pipes)
*         if not --> return value of UNIT_TYPE_NOT_VALID
*         if yes -> return the 'instance type' (pipe 0/1/shared)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  to access to. (expected to be in Pipe0)
* @param[in] accessFromCpu            - indication that the CPU is accessing the memory.
* @param[in] tileId                   - the  of the address
*
* @param[out] newAddressPtr            - pointer to new address to access.
*/
static void  falcon_specialSingleInstanceAddressInfoGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_BOOL                 accessFromCpu,
    OUT GT_U32                 *newAddressPtr
)
{
    GT_U32  ii;
    SPECIAL_INSTANCE_TABLES_ARR_STC *specialTableInfoPtr;
    GT_U32  addrInTile0 = address % TILE_OFFSET;
    GT_U32  addrInPipe0;
    GT_U32  tileId = address / TILE_OFFSET;
    SHERED_MEM_TYPE_ENT sharedType = SHERED_MEM_TYPE_NOT_VALID;

    specialTableInfoPtr = &specialSingleInstanceTablesArr[0];

    if(addrInTile0 < PIPE_0_START_ADDR ||
       addrInTile0 > PIPE_1_END_ADDR)
    {
        /* the address not part of 'Control Pipe' : pipe 0 or pipe 1 */
        return;
    }

    if(addrInTile0 >= PIPE_1_START_ADDR)
    {
        addrInPipe0 = addrInTile0 - PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    {
        addrInPipe0 = addrInTile0;
    }

    for(ii = 0 ; specialTableInfoPtr->tableOffsetValid != 0; ii++,specialTableInfoPtr++)
    {
        if(addrInPipe0 >= specialTableInfoPtr->startAddr &&
           addrInPipe0 <= specialTableInfoPtr->lastAddr)
        {
            /* address belongs to 'Special tables' that number of instances
               that can be different than the number on instances of the UNIT ! */
            if(accessFromCpu == GT_TRUE)
            {
                sharedType =  specialTableInfoPtr->table_unitType_cpu_access;
            }
            else
            {
                sharedType =  specialTableInfoPtr->table_unitType_deviceAccess;
            }

            break;
        }
    }

    if(sharedType == SHERED_MEM_TYPE_PER_TILE_PIPE_0_ONLY_E)
    {
        /* the memory exists only in the local pipe 0 of each tile      */
        /* need to update the address to be in local pipe 0 of the tile */
        *newAddressPtr = addrInPipe0 + (tileId * TILE_OFFSET);
    }

    return;
}


/**
* @internal smemFalconConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Bobcat3 : Convert (dev,address) to new (dev,address).
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void smemFalconConvertDevAndAddrToNewDevAndAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT ** newDevObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
)
{
    GT_U32  newAddress;
    GT_U32 currentPipeId;
    GT_U32 pipeOffset , pipeIdOfAddress = 0;
    GT_U32 ravenId = 0xFFFF;
    GT_U32 tileId;

    *newDevObjPtrPtr = devObjPtr;

    if (((address >> 28) & 0x1) == 0)
    {
        tileId = address / TILE_OFFSET;
        /* check is address in raven's scope */
        switch (address & 0x0F800000)
        {
            case RAVEN_0_START_ADDR:
                ravenId = 0;
                break;
            case RAVEN_1_START_ADDR:
                ravenId = 1;
                break;
            case RAVEN_2_START_ADDR:
                ravenId = 2;
                break;
            case RAVEN_3_START_ADDR:
                ravenId = 3;
                break;
            default:
                ravenId = 0xFFFF;
        }
    }

    if (ravenId != 0xFFFF)
    {
        if((1 << tileId) & devObjPtr->mirroredTilesBmp)
        {
            ravenId = 3 - ravenId;
        }
        ravenId = ravenId + tileId * 4;
        if (devObjPtr->activeRavensBitmap & (1 << ravenId))
        {
            ravenId = ravenId;
        }
        else
        {
            /* kernel access */
            if(0 == IS_SKERNEL_OPERATION_MAC(accessType))
            {
                skernelFatalError("smemFalconConvertDevAndAddrToNewDevAndAddr : the device not supports raven %d\n", ravenId);
                return;
            }
        }
    }

    if(IS_DFX_OPERATION_MAC(accessType) &&
        SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr))
    {
        /* the DFX is only on 'pipe 0' */
        /* so no address modifications */
        *newAddressPtr = address;
        return;
    }

    if(0 == IS_SKERNEL_OPERATION_MAC(accessType))
    {
        /* the CPU access 'pipe 0/1' explicitly */
        /* so no address modifications */
        /* but still shared memories that CPU access address at unit x may be 'redirected' to other place */
        /* so need to allow call to falcon_specialSingleInstanceAddressInfoGet(...) */

        newAddress = address;
    }
    else
    {
        pipeOffset = internalFalconUnitPipeOffsetGet(devObjPtr, address,
            &pipeIdOfAddress,&currentPipeId,GT_FALSE);

        if(pipeIdOfAddress >= devObjPtr->numOfPipesPerTile) /* regardless to the 'current pipe Id' */
        {
            /* explicit access to other than tile 0 , do not modify */
            *newAddressPtr = address;
            return;
        }

        if(currentPipeId == 0)
        {
            /* no address update needed for pipe 0 accessing */
            *newAddressPtr = address;
            return;
        }

        newAddress = address + pipeOffset;
    }

    falcon_specialSingleInstanceAddressInfoGet(devObjPtr,
        newAddress,
        (0 == IS_SKERNEL_OPERATION_MAC(accessType)) ?
        GT_TRUE :   /* access from CPU */
        GT_FALSE,   /* access from DEVICE */
        &newAddress);

    *newAddressPtr = newAddress;

    return;
}

/**
* @internal smemFalconPreparePipe1Recognition function
* @endinternal
*
* @brief   prepare pipe 1 recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void smemFalconPreparePipe1Recognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;
    GT_U32  ii;
    SPECIAL_INSTANCE_TABLES_ARR_STC *specialTableInfoPtr;
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC *tableInfoPtr;
    SMEM_CHUNK_STC    *memChunkPtr;

    /* point pipe1 to look directly to pipe 0 memory */
    /* NOTE: the function smemFalconConvertDevAndAddrToNewDevAndAddr will help
       any memory access via pipe1 to access proper unit in the 'pipe 0 device' */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        smemFalconConvertDevAndAddrToNewDevAndAddr;


    /* init the <startAddr> , <lastAddr> in the 'special tables' */
    specialTableInfoPtr = &specialSingleInstanceTablesArr[0];
    for(ii = 0 ; specialTableInfoPtr->tableOffsetValid != 0; ii++,specialTableInfoPtr++)
    {
        tableInfoPtr = (SKERNEL_TABLE_4_PARAMETERS_INFO_STC*)
            ((GT_U8*)(void*)(&devObjPtr->tablesInfo)+ specialTableInfoPtr->tableOffsetInBytes);

        memChunkPtr = tableInfoPtr->commonInfo.memChunkPtr;

        specialTableInfoPtr->startAddr = memChunkPtr->memFirstAddr;
        specialTableInfoPtr->lastAddr  = memChunkPtr->memLastAddr;
    }

}

#define UNIT_IN_PIPE_1_MAC(orig_unitName) \
    orig_unitName##_INSTANCE_1

#define ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(orig_unitName , regDbUnitName, dpIndex) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName)))           \
    {                                                                                                                             \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName))]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                                           \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName[dpIndex]),                                                       \
            currUnitChunkPtr);                                                                                                    \
                                                                                                                                  \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_IN_PIPE_1_MAC(orig_unitName));                                    \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName[dpIndex]           ,unitBaseAddress);               \
    }

#define ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(target_unitName,regDbUnitName, dpIndex) \
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

#define UNIT_INSTANCE_IN_TILE_MAC(orig_unitName,instance,tile) \
    orig_unitName##_INSTANCE_##instance##_TILE_##tile


#define ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(orig_unitName , regDbUnitName, dpIndex,instance,tile) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_INSTANCE_IN_TILE_MAC(orig_unitName,instance,tile)))     \
    {                                                                                                                            \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_INSTANCE_IN_TILE_MAC(orig_unitName,instance,tile))]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                                           \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName[dpIndex]),                                                       \
            currUnitChunkPtr);                                                                                                    \
                                                                                                                                  \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_INSTANCE_IN_TILE_MAC(orig_unitName,instance,tile));               \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName[dpIndex]           ,unitBaseAddress);               \
    }

#define UNIT_IN_TILE_MAC(orig_unitName,tile) \
    orig_unitName##_TILE_##tile

#define ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(orig_unitName , regDbUnitName, dpIndex,tile) \
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IN_TILE_MAC(orig_unitName,tile)))     \
    {                                                                                                                             \
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IN_TILE_MAC(orig_unitName,tile))]; \
        smemGenericRegistersArrayAlignToUnit(devObjPtr,                                                                           \
            REG_PTR_AND_SIZE_MAC(regAddrDbSip5Ptr->regDbUnitName[dpIndex]),                                                       \
            currUnitChunkPtr);                                                                                                    \
                                                                                                                                  \
        unitBaseAddress = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_IN_TILE_MAC(orig_unitName,tile));               \
        SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,regDbUnitName[dpIndex]           ,unitBaseAddress);               \
    }


/**
* @internal smemFalconInitRegDbPipe0 function
* @endinternal
*
* @brief   Init RegDb of pipe 0.
*/
static void smemFalconInitRegDbPipe0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    /* set register addresses for pipe0 (SDQ[0..3])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ1 , SIP6_TXQ_SDQ ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ2 , SIP6_TXQ_SDQ ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ3 , SIP6_TXQ_SDQ ,3);/*DP[3]*/


    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC1 , SIP6_TXQ_QFC ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC2 , SIP6_TXQ_QFC ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC3 , SIP6_TXQ_QFC ,3);/*DP[3]*/


    /* set register addresses for pipe0 (sip6_rxDMA[0..3])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA   , sip6_rxDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_1 , sip6_rxDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_2 , sip6_rxDMA ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA_3 , sip6_rxDMA ,3);/*DP[3]*/

    /* set register addresses for pipe0 (sip6_txDMA[0..3])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA   , sip6_txDMA ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_1 , sip6_txDMA ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_2 , sip6_txDMA ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA_3 , sip6_txDMA ,3);/*DP[3]*/

    /* set register addresses for pipe0 (sip6_txFIFO[0..3])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_1 , sip6_txFIFO ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_2 , sip6_txFIFO ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO_3 , sip6_txFIFO ,3);/*DP[3]*/

     /* set register addresses for pipe0 (eagle_d2d[0..3])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_EAGLE_D2D   , eagle_d2d ,0);/*DP[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_EAGLE_D2D_1 , eagle_d2d ,1);/*DP[1]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_EAGLE_D2D_2 , eagle_d2d ,2);/*DP[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_EAGLE_D2D_3 , eagle_d2d ,3);/*DP[3]*/


    /* raven units are per tile, not per pipe. set here all raven in tile [0..3]*/
    /*pipe 0 units 0,1*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_0 , raven_d2d_pair ,0);/*Die[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_1 , raven_d2d_pair ,1);/*Die[1]*/
    /*pipe 1 units 2,3*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_2 , raven_d2d_pair ,2);/*Die[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_3 , raven_d2d_pair ,3);/*Die[3]*/

    /*pipe 0 units 0,1*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_0 , MG_RAVEN ,0);/*Die[0]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_1 , MG_RAVEN ,1);/*Die[1]*/
    /*pipe 1 units 2,3*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_2 , MG_RAVEN ,2);/*Die[2]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_0_MAC(UNIT_BASE_RAVEN_3 , MG_RAVEN ,3);/*Die[3]*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PREQ,PREQ);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EREP,EREP);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EM,EXACT_MATCH);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PHA,PHA);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_AAC, CNM.AAC);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_SHM,SHM);
}
/**
* @internal smemFalconInitRegDbPipe1 function
* @endinternal
*
* @brief   Init RegDb of pipe 1.
*/
static void smemFalconInitRegDbPipe1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA    , rxDMA       ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_1  , rxDMA       ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_2  , rxDMA       ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_3  , rxDMA       ,7);/*DP[7]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA    , sip6_rxDMA  ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_1  , sip6_rxDMA  ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_2  , sip6_rxDMA  ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_RX_DMA_3  , sip6_rxDMA  ,7);/*DP[7]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_IA        , ingressAggregator ,1);

    /* set register addresses for pipe0 (sip6_txDMA[4..7])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA   , sip6_txDMA ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA_1 , sip6_txDMA ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA_2 , sip6_txDMA ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_DMA_3 , sip6_txDMA ,7);/*DP[7]*/

    /* set register addresses for pipe0 (sip6_txFIFO[4..7])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO_1 , sip6_txFIFO ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO_2 , sip6_txFIFO ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TX_FIFO_3 , sip6_txFIFO ,7);/*DP[7]*/

    /* set register addresses for pipe0 (eagle_d2d[4..7])*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_EAGLE_D2D   , eagle_d2d ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_EAGLE_D2D_1 , eagle_d2d ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_EAGLE_D2D_2 , eagle_d2d ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_EAGLE_D2D_3 , eagle_d2d ,7);/*DP[7]*/

    /* set register addresses for pipe1 (SDQ[4..7]) */
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_SDQ1 , SIP6_TXQ_SDQ ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_SDQ2 , SIP6_TXQ_SDQ ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_SDQ3 , SIP6_TXQ_SDQ ,7);/*DP[7]*/

    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,4);/*DP[4]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_QFC1 , SIP6_TXQ_QFC ,5);/*DP[5]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_QFC2 , SIP6_TXQ_QFC ,6);/*DP[6]*/
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_1_MAC(UNIT_TXQ_QFC3 , SIP6_TXQ_QFC ,7);/*DP[7]*/



}
/* in tile 1 and tile 3 the DP units are mirrored inside the pipe*/
#define DP_MIRROR_UNITS_INDEX_MAC(dpIndex,offset,tileId) \
    (((tileId%2) == 0)? (dpIndex + offset) : ((4/*number of DP in pipe*/ -1) - (offset%4) + dpIndex))

#define DUP_UNITS_INSTANCE_IN_TILE(dpIndex,iaIndex,ravenIndex,instance,tileId)     \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA    , rxDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,0,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_1  , rxDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,1,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_2  , rxDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,2,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_3  , rxDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,3,tileId),instance,tileId);/*DP[]*/ \
                                                                                                               \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA    , sip6_rxDMA  ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,0,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_1  , sip6_rxDMA  ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,1,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_2  , sip6_rxDMA  ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,2,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_RX_DMA_3  , sip6_rxDMA  ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,3,tileId),instance,tileId);/*DP[]*/ \
                                                                                                               \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_IA        , ingressAggregator ,iaIndex,instance,tileId);     \
                                                                                                               \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_DMA    , sip6_txDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,0,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_DMA_1  , sip6_txDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,1,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_DMA_2  , sip6_txDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,2,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_DMA_3  , sip6_txDMA       ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,3,tileId),instance,tileId);/*DP[]*/ \
                                                                                                               \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_FIFO   , sip6_txFIFO      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,0,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_FIFO_1 , sip6_txFIFO      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,1,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_FIFO_2 , sip6_txFIFO      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,2,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TX_FIFO_3 , sip6_txFIFO      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,3,tileId),instance,tileId);/*DP[]*/ \
                                                                                                                    \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_EAGLE_D2D   , eagle_d2d      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,0,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_EAGLE_D2D_1 , eagle_d2d      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,1,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_EAGLE_D2D_2 , eagle_d2d      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,2,tileId),instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_EAGLE_D2D_3 , eagle_d2d      ,DP_MIRROR_UNITS_INDEX_MAC(dpIndex,3,tileId),instance,tileId);/*DP[]*/ \
                                                                                                                   \
    if(instance == 0)/* first pipe in tile*/                                                                       \
    {                                                                                                              \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_0 , raven_d2d_pair      ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,0,tileId),tileId);/*raven[]*/ \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_1 , raven_d2d_pair      ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,1,tileId),tileId);/*raven[]*/ \
                                                                                                                   \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_2 , raven_d2d_pair      ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,2,tileId),tileId);/*raven[]*/ \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_3 , raven_d2d_pair      ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,3,tileId),tileId);/*raven[]*/ \
                                                                                                                   \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_0 , MG_RAVEN            ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,0,tileId),tileId);/*raven[]*/ \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_1 , MG_RAVEN            ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,1,tileId),tileId);/*raven[]*/ \
                                                                                                                   \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_2 , MG_RAVEN            ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,2,tileId),tileId);/*raven[]*/ \
        ALIGN_REG_DB_TO_UNIT_IN_TILE_MAC(UNIT_BASE_RAVEN_3 , MG_RAVEN            ,DP_MIRROR_UNITS_INDEX_MAC(ravenIndex,3,tileId),tileId);/*raven[]*/ \
}                                                                                                              \
                                                                                                                   \
                                                                                                                   \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,dpIndex+0,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_SDQ1 , SIP6_TXQ_SDQ ,dpIndex+1,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_SDQ2 , SIP6_TXQ_SDQ ,dpIndex+2,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_SDQ3 , SIP6_TXQ_SDQ ,dpIndex+3,instance,tileId) /*DP[]*/ \
                                                                                                               \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,dpIndex+0,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_QFC1 , SIP6_TXQ_QFC ,dpIndex+1,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_QFC2 , SIP6_TXQ_QFC ,dpIndex+2,instance,tileId);/*DP[]*/ \
    ALIGN_REG_DB_TO_UNIT_IN_PIPE_IN_TILE_MAC(UNIT_TXQ_QFC3 , SIP6_TXQ_QFC ,dpIndex+3,instance,tileId) /*DP[]*/



/**
* @internal smemFalconInitRegDbTiles function
* @endinternal
*
* @brief   Init RegDb of Tiles 1..3
*/
static void smemFalconInitRegDbTiles
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    if(devObjPtr->numOfTiles >= 2)
    {
        DUP_UNITS_INSTANCE_IN_TILE(8 /*dpIndex*/,2/*iaIndex*/,4/*ravenIndex*/,0/*instance*/,1/*tileId*/);
        DUP_UNITS_INSTANCE_IN_TILE(12/*dpIndex*/,3/*iaIndex*/,6/*ravenIndex*/,1/*instance*/,1/*tileId*/);
    }

    if(devObjPtr->numOfTiles >= 3)
    {
        DUP_UNITS_INSTANCE_IN_TILE(16/*dpIndex*/,4/*iaIndex*/,8/*ravenIndex*/,0/*instance*/,2/*tileId*/);
        DUP_UNITS_INSTANCE_IN_TILE(20/*dpIndex*/,5/*iaIndex*/,10/*ravenIndex*/,1/*instance*/,2/*tileId*/);
    }

    if(devObjPtr->numOfTiles >= 4)
    {
        DUP_UNITS_INSTANCE_IN_TILE(24/*dpIndex*/,6/*iaIndex*/,12/*ravenIndex*/,0/*instance*/,3/*tileId*/);
        DUP_UNITS_INSTANCE_IN_TILE(28/*dpIndex*/,7/*iaIndex*/,14/*ravenIndex*/,1/*instance*/,3/*tileId*/);
    }

}


/**
* @internal smemFalconIsDeviceMemoryOwner function
* @endinternal
*
* @brief   Return indication that the device is the owner of the memory.
*         relevant to multi pipe where there is 'shared memory' between pipes.
*
* @retval GT_TRUE                  - the device is   the owner of the memory.
* @retval GT_FALSE                 - the device is NOT the owner of the memory.
*/
static GT_BOOL smemFalconIsDeviceMemoryOwner
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address
)
{
    return GT_TRUE;
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
    smemFalconInterruptTreeInit(devObjPtr);
}

/**
* @internal onEmulator_smemFalconInit function
* @endinternal
*
* @brief   Falcon 'on emulator' - remove the units that not exists in the Emulator.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void onEmulator_smemFalconInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* no need to remove any unit */
    /* the SERDESs SD unit exists , and CPSS allow only to access to 2 registers per port */
}

/**
* @internal internal_smemFalconInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
static void internal_smemFalconInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isFalcon = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP5_25_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP6_GET(devObjPtr)    = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isFalcon = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = falcon_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = falcon_duplicatedUnits;
    }

    if (isFalcon == GT_TRUE)
    {
        devObjPtr->devIsOwnerMemFunPtr = smemFalconIsDeviceMemoryOwner;
        devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;

        devObjPtr->devMemGetMgUnitIndexFromAddressPtr = smemFalconGetMgUnitIndexFromAddress;

        devObjPtr->numOfPipesPerTile = 2;
        devObjPtr->numOfTiles        = devObjPtr->deviceType == SKERNEL_FALCON_SINGLE_TILE ? 1 :
                                       devObjPtr->deviceType == SKERNEL_FALCON_2_TILES     ? 2 :
                                       4;
        devObjPtr->tileOffset        = TILE_OFFSET;
        devObjPtr->mirroredTilesBmp  = TILE_1_BIT | TILE_3_BIT;/* in Falcon tile 1,3 are mirror image of tile 0 */

        devObjPtr->numOfMgUnits = devObjPtr->numOfTiles * FALCON_NUM_MG_PER_TILE;/* 4 MGs per tile */

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = 4;/* (4 per pipe) - 8 RxDmas,TxDams and TxFifos units for the 3.2T device */
        /* !!! there is NO TXQ-dq in Falcon !!! */
        devObjPtr->multiDataPath.numTxqDq           = 0;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 0;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = devObjPtr->numOfTiles == 1 ? FALCON_3_2_PRIMARY_CPU_SDMA :
                                     devObjPtr->numOfTiles == 2 ? FALCON_6_4_PRIMARY_CPU_SDMA :
                                     FALCON_12_8_PRIMARY_CPU_SDMA ;

        /* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = devObjPtr->numOfTiles * devObjPtr->numOfPipesPerTile;
        devObjPtr->numOfPortsPerPipe =
            (devObjPtr->multiDataPath.maxDp*FALCON_PORTS_PER_DP);/*32*/

        devObjPtr->txqNumPorts = 1024;/* the limit on the 10 bits in SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM */

        devObjPtr->multiDataPath.maxIa = devObjPtr->numOfPipes;/* single IA per pipe */

        {
            GT_U32  index;
            GT_U32  tileId,localDpIndex,numDpsInTile;
            GT_U32  offset;

            /* NOTE: in FALCON the mirrored Tiles cause the DP[] units to hold swapped DMAs */
            numDpsInTile = devObjPtr->multiDataPath.maxDp * devObjPtr->numOfPipesPerTile;
            for(index = 0 ; index < (devObjPtr->multiDataPath.maxDp * devObjPtr->numOfPipes) ; index++)
            {
                tileId       = index/numDpsInTile;
                localDpIndex = index%numDpsInTile;

                /* each DP supports 8 ports + 1 CPU port (index 8) */
                devObjPtr->multiDataPath.info[index].dataPathFirstPort  = (localDpIndex + (tileId * numDpsInTile)) * FALCON_PORTS_PER_DP;
                devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = FALCON_PORTS_PER_DP;
                devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = FALCON_PORTS_PER_DP;
            }

            devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);
            for(index = 2 ; index < devObjPtr->numOfPipes; index += 2)
            {
                smemSetCurrentPipeId(devObjPtr,index);
                offset = smemFalconUnitPipeOffsetGet(devObjPtr,devObjPtr->memUnitBaseAddrInfo.lpm[0],NULL);
                /* every 2 pipes share the same LPM unit */
                devObjPtr->memUnitBaseAddrInfo.lpm[index / 2] = offset + devObjPtr->memUnitBaseAddrInfo.lpm[0];
            }
            /* restore pipe 0 */
            smemSetCurrentPipeId(devObjPtr,0);
        }

        if(devObjPtr->numOfTiles == 1)
        {
            devObjPtr->dma_specialPortMappingArr = falcon_3_2_DMA_specialPortMappingArr;
            devObjPtr->gop_specialPortMappingArr = falcon_3_2_GOP_specialPortMappingArr;
            devObjPtr->ravens_specialPortMappingArr = falcon_3_2_Ravens_specialPortMappingArr;
            devObjPtr->cpuPortSdma_specialPortMappingArr = falcon_3_2_cpuPortSdma_specialPortMappingArr;
        }
        else
        if(devObjPtr->numOfTiles == 2)
        {
            devObjPtr->dma_specialPortMappingArr = falcon_6_4_DMA_specialPortMappingArr;
            devObjPtr->gop_specialPortMappingArr = falcon_6_4_GOP_specialPortMappingArr;
            devObjPtr->ravens_specialPortMappingArr = falcon_6_4_Ravens_specialPortMappingArr;
            devObjPtr->cpuPortSdma_specialPortMappingArr = falcon_6_4_cpuPortSdma_specialPortMappingArr;
        }
        else
        {
            devObjPtr->dma_specialPortMappingArr = falcon_12_8_DMA_specialPortMappingArr;
            devObjPtr->gop_specialPortMappingArr = falcon_12_8_GOP_specialPortMappingArr;
            devObjPtr->ravens_specialPortMappingArr = falcon_12_8_Ravens_specialPortMappingArr;
            devObjPtr->cpuPortSdma_specialPortMappingArr = falcon_12_8_cpuPortSdma_specialPortMappingArr;
        }


        devObjPtr->tcam_numBanksForHitNumGranularity = 2; /* like BC2 */
        devObjPtr->tcamNumOfFloors   = 6;                 /* like Aldrin2 */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->support_remotePhysicalPortsTableMode = 1;

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,1024);/*512 in BC3, 128 in Aldrin2*/
        /*NOTE: support 8K eports like aldrin2.
        */

        /* Errata in Falcon */
        devObjPtr->errata.HA2CNC_swapIndexAndBc = 1;/* [JIRA] [SIPCTRL-74] Ha2CNC wrapper connections are turn around */
        devObjPtr->errata.PCL2CNC_ByteCountWithOut4BytesCrc = 1;/* [JIRA] [PCL-829] pcl clients count 4 bytes less that tti client in cnc counetrs in Falcon device */
        devObjPtr->errata.byte_count_missing_4_bytes = 1;
        devObjPtr->errata.fdbSaLookupNotUpdateAgeBitOnStaticEntry = 1;/*JIRA : [MT-396] : Falcon FDB does not refresh static entries*/


        /* the device not supports IPCL 0 (supports only IPCL1,2) */
        devObjPtr->isIpcl0NotValid  = 1;
        devObjPtr->numofTcamClients = FALCON_TCAM_NUM_OF_GROUPS_CNS;
        devObjPtr->numofIPclProfileId = 128;
        devObjPtr->numofEPclProfileId = 128;

        {
            GT_U32 t;
            for(t = 0; t < devObjPtr->numOfTiles; t++)
            {
                devObjPtr->memUnitBaseAddrInfo.raven[t * 4 + 0] = TILE_OFFSET * t + UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_0);
                devObjPtr->memUnitBaseAddrInfo.raven[t * 4 + 1] = TILE_OFFSET * t + UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_1);
                devObjPtr->memUnitBaseAddrInfo.raven[t * 4 + 2] = TILE_OFFSET * t + UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_2);
                devObjPtr->memUnitBaseAddrInfo.raven[t * 4 + 3] = TILE_OFFSET * t + UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_3);
            }

            devObjPtr->numOfRavens = devObjPtr->numOfTiles * 4;
            devObjPtr->activeRavensBitmap = (1 << devObjPtr->numOfRavens) - 1;
            {
                GT_U32  numOfLmus = 4; /* 2 per Raven , 2 Ravens per 'pipe' */
                GT_U32  lmuOffset       = 0x00430000;
                GT_U32  lmuOffset_unit1 = 0x00080000;

                devObjPtr->memUnitBaseAddrInfo.lmu[0] = devObjPtr->memUnitBaseAddrInfo.raven[0] + lmuOffset;
                devObjPtr->memUnitBaseAddrInfo.lmu[1] = devObjPtr->memUnitBaseAddrInfo.raven[0] + lmuOffset + lmuOffset_unit1;
                devObjPtr->memUnitBaseAddrInfo.lmu[2] = devObjPtr->memUnitBaseAddrInfo.raven[1] + lmuOffset;
                devObjPtr->memUnitBaseAddrInfo.lmu[3] = devObjPtr->memUnitBaseAddrInfo.raven[1] + lmuOffset + lmuOffset_unit1;

                devObjPtr->tablesInfo.lmuStatTable.commonInfo.multiInstanceInfo.numBaseAddresses = numOfLmus;
                devObjPtr->tablesInfo.lmuStatTable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lmu[0];
                devObjPtr->tablesInfo.lmuCfgTable.commonInfo.multiInstanceInfo.numBaseAddresses = numOfLmus;
                devObjPtr->tablesInfo.lmuCfgTable.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.lmu[0];
            }
        }

    }

    /* Errata in Falcon and above  */
    devObjPtr->errata.byte_count_missing_4_bytes_in_HA  = 1; /* [JIRA] [HA-3739] HA build TO_CPU DSA the <Pkt Orig BC> hold 4 bytes less than expected (missing 4 bytes of CRC) */
    devObjPtr->errata.byte_count_missing_4_bytes_in_L2i = 1; /* [JIRA] [L2I-794] Maximum Receive Unit (MRU) limit per eVLAN - 4 bytes unaccounted for */

    {
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort , 10);
        SET_IF_ZERO_MAC(devObjPtr->fdbMaxNumEntries , SMEM_MAC_TABLE_SIZE_256KB);/*128K in Aldrin2*/
        SET_IF_ZERO_MAC(devObjPtr->emMaxNumEntries, SMEM_MAC_TABLE_SIZE_128KB);

        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfLpmRams , 30);
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfEntriesBetweenRams , 32*1024);
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries,14*1024);

        SET_IF_ZERO_MAC(devObjPtr->defaultEPortNumEntries,SIP6_NUM_DEFAULT_E_PORTS_CNS);

        SET_IF_ZERO_MAC(devObjPtr->ipvxEcmpIndirectMaxNumEntries , 6*1024);/* 4 'pointers' in line */

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_SIP6_CNC_CLIENTS_BMP_ALL_CNS);

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.mllPairs , _16K);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2LttMll , _16K);
    }

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemFalconSpecificDeviceUnitAlloc;
    }

    /* function will be called from inside smemLion3Init(...) */
    if(devObjPtr->devMemSpecificDeviceMemInitPart2 == NULL)
    {
        devObjPtr->devMemSpecificDeviceMemInitPart2 = smemBobcat3SpecificDeviceMemInitPart2;
    }

    if(devObjPtr->devMemInterruptTreeInit == NULL)
    {
        devObjPtr->devMemInterruptTreeInit = internal_InterruptTreeInit;
    }

    if(devObjPtr->registersDfxDefaultsPtr == NULL)
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC registersDfxDefaultValueArr[] =
        {
            /* empty -- default registers need to come from function smemFalconUnitDfxServer*/
             {NULL, 0, 0x00000000, 0,    0x0                        }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element0_DfxDefault_RegistersDefaults =
            {registersDfxDefaultValueArr , NULL};

        devObjPtr->registersDfxDefaultsPtr = &element0_DfxDefault_RegistersDefaults;
    }

    if(devObjPtr->registersPexDefaultsPtr == NULL)
    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC registersDummyDefaultValueArr[] =
        {
            /* empty -- default registers need to come from function smemFalconUnitBar0*/
             {NULL, 0, 0x00000000, 0,    0x0                        }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element0_DummyDefault_RegistersDefaults =
            {registersDummyDefaultValueArr , NULL};

        devObjPtr->registersPexDefaultsPtr = &element0_DummyDefault_RegistersDefaults;
    }

    smemAldrin2Init(devObjPtr);

    /* init support for BAR0,BAR2 access */
    smemFalconBar0Bar2MemInit(devObjPtr);

    /* the device not supports traffic generator as the legacy devices did.
       need to implement TG of the D2D !
    */
    devObjPtr->trafficGeneratorSupport.tgSupport = 0;

    /* tables not exists (need to clear from legacy devices) */
    devObjPtr->tablesInfo.egressStc.commonInfo.baseAddress = 0;
    devObjPtr->tablesInfo.ingrStc.commonInfo.baseAddress = 0;
    /* steps between 'DX Queue Group Map 0' to 'DX Queue Group Map 1' ...*/
    /*  no use of 'DX Queue Group Map 1' ...
        devObjPtr->tablesInfo.txqPdxQueueGroupMap.paramInfo[1].step = 0x1000;*/

    /*pre-TTI Lookup Ingress ePort Table*/
    devObjPtr->tablesInfo.ttiPreTtiLookupIngressEPort.paramInfo[0].modulo =
        devObjPtr->defaultEPortNumEntries;

    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[1].modulo =
        devObjPtr->defaultEPortNumEntries;

    if (isFalcon == GT_TRUE)
    {
        /* init regDb of pipe0 DP units */
        smemFalconInitRegDbPipe0(devObjPtr);

        /* init regDb of pipe1 DP units */
        smemFalconInitRegDbPipe1(devObjPtr);

        /* init regDb of Tiles 1..3 for DP units */
        smemFalconInitRegDbTiles(devObjPtr);

        /* prepare pipe 1 recognition */
        smemFalconPreparePipe1Recognition(devObjPtr);

        /* check that no unit exceed the size of it's unit */
        smemGenericUnitSizeCheck(devObjPtr,falcon_units);

        if(simulationCheck_onEmulator())
        {
            /* remove units/memories that not exists */
            onEmulator_smemFalconInit(devObjPtr);
        }

        /* init the FIRMAWARE */
        simulationFalconFirmwareInit(devObjPtr);
    }

}

/* update a register in all pipes */
static void updateRegisterInAllPipes
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       regAddress,
    IN GT_U32                       startBit,
    IN GT_U32                       numBits,
    IN GT_U32                       value
)
{
    GT_U32  pipeId;

    /* need to update the register in all pipes */
    for(pipeId = 0 ; pipeId < devObjPtr->numOfPipes ; pipeId++)
    {
        smemSetCurrentPipeId(devObjPtr,pipeId);
        smemRegFldSet(devObjPtr,regAddress,startBit,numBits,value);
    }

    /* restore pipe 0 */
    smemSetCurrentPipeId(devObjPtr,0);
}

/**
* @internal egfLinkUpOnSlanBind function
* @endinternal
*
* @brief   debug - allow to set EGF link filter according to to Bind/Unbind SLAN to port.
*
* @param[in] devObjPtr                (pointer to) the device object
* @param[in] portNumber               - port number
* @param[in] bindRx                   - bind to Rx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] bindTx                   - bind to Tx direction ? GT_TRUE - yes , GT_FALSE - no
*                                       None
*/
static void egfLinkUpOnSlanBind
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx
)
{
    GT_U32  regAddress,startBit,value;
    /* EGF link down filter */

    /* set the SLAN ports in link up */
    regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(devObjPtr,portNumber/32);
    startBit = portNumber % 32;
    value = bindTx == GT_TRUE ? 1 : 0;

    /* need to update the register in the 2 pipes */
    updateRegisterInAllPipes(devObjPtr,regAddress,startBit,1,value);
}

/**
* @internal smemFalconInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemFalconInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* EGF link down filter */
    /* set the SLAN ports in link up */
    devObjPtr->devSlanBindPostFuncPtr = egfLinkUpOnSlanBind;

    /* <DeviceEn>*/
    smemDfxRegFldSet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, 1);

    {
        GT_U32  ii;
        GT_U32  regAddress;
        GT_U32  value;
        GT_U32 tileOffset , tileId;
        GT_U32  numDpsPerTile   = devObjPtr->numOfPipesPerTile * devObjPtr->multiDataPath.maxDp;
        GT_U32  numPortsPerTile = numDpsPerTile * FALCON_PORTS_PER_DP;
        GT_U32  localDpInTile;
        GT_U32  localPipeIdInTile;
        GT_U32 pipeOffset;
        GT_U32  localPortInTile;

        /* port per register */
        for(ii = 0 ; ii < (64 * devObjPtr->numOfTiles) ; ii++)
        {
            tileId = (ii / numPortsPerTile);

            localPortInTile = (ii % numPortsPerTile);
            localDpInTile   = localPortInTile / FALCON_PORTS_PER_DP;

            if(tileId & 1)
            {
                /* the DP units are mirrored in the tile : 1,3 */
                localDpInTile = 7 - localDpInTile;
            }

            localPipeIdInTile = localDpInTile / devObjPtr->multiDataPath.maxDp;

            regAddress = SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,
                ii%(FALCON_PORTS_PER_DP*devObjPtr->multiDataPath.maxDp));

            pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, regAddress , NULL);
            tileOffset = TILE_OFFSET * tileId;

            regAddress += localPipeIdInTile * pipeOffset;
            regAddress += tileOffset;

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
        for(ii = 0 ; ii < 64 ; ii++ , regAddress+=4)
        {
            value = ii % 32 ;/* local number*/
            updateRegisterInAllPipes(devObjPtr,regAddress,0,32,value);
        }
    }

    {
        GT_U32  ii;

        for(ii = 0 ; ii < (64*devObjPtr->numOfTiles) ; ii++)
        {
            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_PortEn_E,1);
            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_mru_E,0x2f9*2);
        }
    }

    {
        GT_U32  ii;
        GT_U32  value;
        GT_U32  regAddress = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,0);

        for(ii = 0 ; ii < 128 ; ii++)
        {
            /* EGF_QAG -  Target Port Mapper table */
            regAddress = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,ii);
            /* currently 1:1 */
            value = ii;

            updateRegisterInAllPipes(devObjPtr,regAddress,0,32,value);
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
        GT_U32  vidx = 0x7ff; /* 128 ports mode */
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

        for(ii = 0 ; ii < (devObjPtr->numOfPipes * devObjPtr->multiDataPath.maxDp); ii++)
        {
            /*enable ports at the SDQ */
            value = 0x1;

           for(jj=0;jj<SIP6_MAX_DP_PORTS_CNS;jj++)
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

        for(ii = 0 ; ii < (devObjPtr->numOfPipes * devObjPtr->multiDataPath.maxDp); ii++)
        {
            value = 0;
            for(jj = 0 ; jj < SIP6_MAX_DP_PORTS_CNS; jj++)
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
        GT_U32  tileId;

        for(tileId = 0 ; tileId < devObjPtr->numOfTiles;tileId++)
        {
            smemSetCurrentPipeId(devObjPtr,tileId * devObjPtr->numOfPipesPerTile);

            for(ii = 0 ; ii < 1024; ii++)
            {
                queue_base = 0;
                queue_pds_index = 0;

                if(devObjPtr->numOfTiles == 1)
                {
                    if(ii < 59)/* physical ports mapped 1:1 with global MAC/DMA*/
                    {
                        queue_base = (ii * 8) % (8*FALCON_PORTS_PER_DP);

                        queue_pds_index = ii / FALCON_PORTS_PER_DP;
                    }
                    else if (ii < 63 )
                    {
                        /*keep 0*/
                    }
                    else
                    if (ii == 63) /* SDMA CPU port */
                    {
                        queue_base = FALCON_PORTS_PER_DP * 8;
                        queue_pds_index = 4;/*DP[4]*/
                    }
                    else if (ii == 68) /* CPU network port */
                    {
                        queue_base = FALCON_PORTS_PER_DP * 8;
                        queue_pds_index = 7;/*DP[7]*/
                    }
                    else if (ii < 68)/*64,65,66,67*/
                    {
                        queue_base = 4 + (ii - 68);/*4,5,6,7*/
                        queue_pds_index = 7;/*DP[7]*/
                    }
                    else
                    {
                        /*keep 0*/
                    }
                }
                else
                {
                    /* keep simple 1:1 mapping ... lets check only 'regular' ports without CPU ports */
                    queue_base = (ii * 8) % (8*FALCON_PORTS_PER_DP);
                    queue_pds_index = ii / FALCON_PORTS_PER_DP;
                }

                value = 0;
                SMEM_U32_SET_FIELD(value,0,9,queue_base);
                SMEM_U32_SET_FIELD(value,9,4,0/*dp_core_local_trg_port -- not care for simulation ... so 0 */);
                SMEM_U32_SET_FIELD(value,13,3,queue_pds_index);
                SMEM_U32_SET_FIELD(value,16,2,tileId/*queue_pdx_index -- tileId*/);

                smemRegSet(devObjPtr ,SMEM_SIP6_TXQ_PDX_QUEUE_GROUP_MAP_TBL_MEM(devObjPtr,
                    ii,0),value);
            }
        }
    }

    /* restore pipe 0 */
    smemSetCurrentPipeId(devObjPtr,0);
#endif /*HELP_CPSS_IN_MISSING_CONFIG*/



}

/**
* @internal smemFalconInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemFalconInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    internal_smemFalconInit(devObjPtr);
}

/**
* @internal smemSip6DfxServerOwnDevNum function
* @endinternal
*
* @brief   function for sip6 devices to set/get the DFX <ownDev>.
*
* @param[in] devObjPtr     - pointer to device object.
* @param[in] setOrGet      - GT_TRUE-set , GT_FALSE-get
* @param[in/out] valuePtr  - on 'set' use '*valuePtr' , on 'get' return value in '*valuePtr'

*/
void smemSip6DfxServerOwnDevNum
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_BOOL                 setOrGet,
    IN /* or */ OUT GT_U32    *valuePtr
)
{
    GT_U32  regAddr,startBit;

    if (SMEM_CHT_IS_SIP6_20_GET(devObjPtr)) /*Harrier*/
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.
                         DFXServerUnitsBC2SpecificRegs.deviceCtrl6;
        startBit = 0;
    }
    else /*Hawk*/
    if (SMEM_CHT_IS_SIP6_15_GET(devObjPtr)) /*Phoenix*/
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.
                         DFXServerUnitsBC2SpecificRegs.deviceCtrl25;
        startBit = 22;
    }
    else /*Hawk*/
    if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.
                         DFXServerUnitsBC2SpecificRegs.deviceCtrl17;
        startBit = 11;
    }
    else/*Falcon*/
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.
                         DFXServerUnitsBC2SpecificRegs.deviceCtrl29,
        startBit = 12;
    }

    if(GT_TRUE == setOrGet)
    {
        smemDfxRegFldSet(devObjPtr,regAddr, startBit, 10, *valuePtr);
    }
    else
    {
        smemDfxRegFldGet(devObjPtr,regAddr, startBit, 10, valuePtr);
    }

    return;
}

/**
* @internal smemFalconInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemFalconInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 data;
    GT_U32 deviceId;
    GT_U32 vendorId;

    static int my_dummy = 0;

    smemAldrin2Init2(devObjPtr);

    data = 0;/* set to ZERO like MG and E2PHY defaults */
    smemSip6DfxServerOwnDevNum(devObjPtr,GT_TRUE,&data);

    if(my_dummy && devObjPtr->numOfTiles >= 2)
    {
        smemFalconInit_debug_allowTraffic(devObjPtr);
    }

    if(devObjPtr->numOfTiles == 2)
    {
        /* Get the 'devId' and 'revision'  */
        scibReadMemory(devObjPtr->deviceId, SMEM_CHT_DEVICE_ID_REG(devObjPtr), 1, &deviceId);
        /* Get the vendor ID */
        scibReadMemory(devObjPtr->deviceId, SMEM_CHT_VENDOR_ID_REG(devObjPtr), 1, &vendorId);

        data = (deviceId >> 4) << 16 | vendorId;
        switch (data)
        {
            case SKERNEL_98CX8525:
            case SKERNEL_98CX8535:
            case SKERNEL_98CX8514:
            case SKERNEL_98EX5614:
                /* active ravens are 0,2,4,5,7 */
                devObjPtr->activeRavensBitmap = 0xB5;
                break;
            default:
                devObjPtr->activeRavensBitmap = (1 << devObjPtr->numOfRavens) - 1;
        }
    }

}

/**
* @internal smemFalconCheckAndSendLoopBackChange function
* @endinternal
*
* @brief   Check if the port's loopback has been changed  and send message (to smain task) if needed
*         IMPORTANT : the function does the operation of:
*         memPtr = inMemPtr;
* @param[in] devObjPtr  - device object PTR.
* @param[in] port       - the port to check/update the link status and generate interrupt.
* @param[in] memPtr     - Pointer to the register's memory in the simulation.
* @param[in] inMemPtr   - Pointer to the memory to get register's content.
*/
static void smemFalconCheckAndSendLoopBackChange
(
    IN   SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN   GT_U32   port,
    IN   GT_U32 * memPtr,
    IN   GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;           /* buffer */
    GT_U32  newLinkValue;           /* new link value */
    GT_U8  * dataPtr;               /* pointer to the data in the buffer */
    GT_U32  dataSize;               /* data size */
    GT_U32  portInLoopbackOld;
    GT_U32  portInLoopback;
    GT_BOOL linkNativeStateUp;

    /* Get values before the 'write to the register' */
    portInLoopbackOld =
        snetChtPortMacFieldGet(devObjPtr, port, SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E);

    /* Update the values in the register due to 'write to the register' */
    *memPtr = *inMemPtr ;

    /* Get updated value 'written to the register' */
    portInLoopback =
        snetChtPortMacFieldGet(devObjPtr, port, SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E);

    /* Get native link state */
    linkNativeStateUp = (devObjPtr->portsArr[port].linkStateWhenNoForce == SKERNEL_PORT_NATIVE_LINK_UP_E) ? GT_TRUE : GT_FALSE;

    if (portInLoopbackOld != portInLoopback)
    {
        newLinkValue = (portInLoopback || linkNativeStateUp) ? 1 : 0;
    }
    else if (linkNativeStateUp)
    {
        newLinkValue = 1;
    }
    else
    {
        /* no need to change status */
        return;
    }

    /* Get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool, LINK_FORCE_MSG_SIZE);
    if (bufferId == NULL)
    {
        simWarningPrintf(" checkAndSendLinkChange : no buffers for link change \n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);
    /* copy port number and link status to buffer  */
    memcpy(dataPtr, &port , sizeof(GT_U32) );
    dataPtr+=sizeof(GT_U32);
    memcpy(dataPtr, &newLinkValue , sizeof(GT_U32) );

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_LINK_CHG_MSG_E;

    /* put buffer to queue                          */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal snetFalconLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*/
GT_VOID snetFalconLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
)
{
    GT_U32 portInGlobBmp;       /* Port's sum interrupt bmp in the global register */
    GT_U32 linkStateOld;        /* Link status before the change*/
    GT_U32 portInLoopback;
    GT_U32 bitIndex;

    if (!IS_CHT_VALID_PORT(devObjPtr, port))
    {
        /* port not exists */
        return;
    }

    if (devObjPtr->numOfPipes)
    {
        GT_U32  dummyPort;
        /* Update 'currentPipeId' and get new (local) port */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr, port, &dummyPort);
    }

    portInLoopback =
        snetChtPortMacFieldGet(devObjPtr, port, SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E);
    if(portInLoopback)
    {
        linkState = 1;/* treat it as force link (even though , lost it's SLAN connection)*/
    }

    linkStateOld =
        snetChtPortMacFieldGet(devObjPtr, port, SNET_CHT_PORT_MAC_FIELDS_LinkState_E);

    if (linkState == linkStateOld)
    {
        /* no need to change status */
        return;
    }

    snetChtPortMacFieldSet(devObjPtr, port,
                           SNET_CHT_PORT_MAC_FIELDS_LinkState_E,
                           linkState);

    /* Global Interrupt Cause register SUM bmp */
    portInGlobBmp = SMEM_CHT_PORT_CAUSE_INT_SUM_BMP(devObjPtr, port);

    /*In MTI we use bit[1] "link_ok_change" */
    bitIndex = CHT_IS_MTI_MAC_USED_MAC(devObjPtr,port) ? 1 : 3;

    /* The device supports the tree of the port interrupts. from the port's mac to MG unit (global summary) */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_CHT_PORT_INT_CAUSE_REG(devObjPtr, port),
                          SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devObjPtr, port),
                          (1 << bitIndex), /* 'LINK STATUS CHANGED' */
                          portInGlobBmp);
}


/**
* @internal snetFalconPortStatisticCounterUpdate function
* @endinternal
*
* @brief  Updates the offset from the start of the statistic memory for
*         specific counter type and port
*
* @param[in] devObjPtr                - (pointer to) device object
* @param[in] macPort                  - MAC port number
* @param[in] counterType              - statistic counter type
* @param[in] incValue                 - counter increment value
*/
GT_VOID snetFalconPortStatisticCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT counterType,
    IN GT_U32                           incValue
)
{
    DECLARE_FUNC_NAME(snetFalconPortStatisticCounterUpdate);
    GT_32   counterIndex;
    GT_U32  *internalHiWordMemPtr;
    GT_CHAR *counterName;
    GT_U64  orig64Value;
    GT_U64  new64Value;
    GT_U64  inc64Value;
    GT_U32  regAddr;
    GT_U32  loWord;
    GT_U32  usePmacCounters = 0;/* when preemption supported , need to know if to
                                   count to EMAC or to PMAC */

    if (counterType >= SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E)
    {
        skernelFatalError("snetFalconPortStatsticCounterRegisterAddress: counterType[%d] is out of range \n",
                          counterType);
        return;
    }

    counterName         = portMtiStatisticCountersInfoArr[counterType].name;
    counterIndex        = counterType;

    if(devObjPtr->portsArr[macPort].portSupportPreemption == GT_TRUE &&
       smemGetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL)/*is the traffic on PMAC*/)
    {
        /* check if the PMAC indeed configured to count to sperate counters or
          'share' it with the 'EMAC' of the port */
        if(snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_mti_mib_counter_is_emac_pmac_shared_E))
        {
            /* the PMAC counting is shared with the EMAC */
            usePmacCounters = 0;
        }
        else
        {
            /* the PMAC counting is in dedicated memory */
            usePmacCounters = 1;
        }
    }

    regAddr = FALCON_PORT_STATSTIC_COUNTER_REG_ADDR_GET(devObjPtr, macPort, counterType,usePmacCounters);
    /* Read statistic counters low 32 bits */
    smemRegGet(devObjPtr, regAddr, &loWord);

    if(!usePmacCounters)
    {
        /* Port page pointer */
        internalHiWordMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI_STATISTICS_COUNTERS_HI_WORD_START_E
                                      + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort));
    }
    else
    {
        /* PMAC : Port page pointer */
        internalHiWordMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI___PMAC___STATISTICS_COUNTERS_HI_WORD_START_E
                                      + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort));
    }
    /* Counter pointer */
    internalHiWordMemPtr += counterIndex;

    __LOG(("macPort[%d] : update PORT statistic counter [%s] - increment by [%d] from value: H[0x%8.8x] L[0x%8.8x]\n",
           macPort,
           counterName,
           incValue,
           internalHiWordMemPtr[0],
           loWord));

    orig64Value.l[0] = loWord;                  /* low word from register */
    orig64Value.l[1] = internalHiWordMemPtr[0]; /* high word from internal memory */

    inc64Value.l[0] = incValue;
    inc64Value.l[1] = 0;

    new64Value = prvSimMathAdd64(orig64Value, inc64Value);

    /* Update statistic counters current value low 32 bits */
    smemRegSet(devObjPtr, regAddr, new64Value.l[0]);

    /* Update high 32 bits word in internal memory */
    internalHiWordMemPtr[0] = new64Value.l[1];


    return;
}

/**
* @internal snetFalconPortStatisticFramesTransmittedCounterGet function
* @endinternal
*
* @brief   Gets port frames counter.
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] macPort                  - MAC port number
* @param[in] rx              -  if equal GT_TRUE then RX_aFramesReceivedOK is returned ,else TX_aFramesTransmittedOK.
* @param[out] valuePtr                  - (pointer to) counter value
*
*
*/

GT_STATUS snetFalconPortStatisticFramesTransmittedCounterGet
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_U32                           macPort,
    IN GT_BOOL                          rx,
    OUT  GT_U32                         *valuePtr
)
{
    GT_U32 regAddr;
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT counterType;

    counterType = (GT_TRUE==rx)?SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aFramesReceivedOK_E:
        SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aFramesTransmittedOK_E;

    regAddr = FALCON_PORT_STATSTIC_COUNTER_REG_ADDR_GET(devObjPtr, macPort,
       counterType ,GT_FALSE);

    /* Read statistic counters low 32 bits */
    smemRegGet(devObjPtr, regAddr, valuePtr);

    return GT_OK;
}


/**
* @internal smemFalconMtiPortByAddrGet function
* @endinternal
*
* @brief   Gets port number according to NTI memory register.
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] address                  - the address in the MTI memory space
* @param[in] mtiRegister              - the register in the MTI memory space
*
* @retval the portNum
*
*/
static GT_U32 smemFalconMtiPortByAddrGet
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32                  address,
    IN  MTI_ACTIVE_MEM_REG_ENT  mtiRegister
)
{
    GT_U32 portNum;
    GT_U32 tileIndex;
    GT_U32 channelIndex;
    GT_U32 unitIndexMask    = 0xF00000;
    GT_U32 channelIndexMask = 0x0F0000;
    GT_U32 dieIndex;
    GT_U32 dieIndexMask = 0x0F000000;
    GT_U32 portIndex = 0;
    GT_U32 portIndexMask = 0xFFF;
    GT_U32 portBaseAddress;
    GT_U32 isCpuPort;
    SPECIAL_PORT_MAPPING_ENHANCED_CNS *currInfoPtr;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        /* Hawk need to use smemGopPortByAddrGet(...) */
        skernelFatalError("smemFalconMtiPortByAddrGet : function only for Falcon with Ravens (not for Hawk) \n");
        return 0;
    }


    tileIndex       = (address / TILE_OFFSET);
    dieIndex        = (address & dieIndexMask) / 0x01000000;
    channelIndex    = (address & channelIndexMask) / 0x80000;
    isCpuPort       = (((address & unitIndexMask) >> 20) == 5) ? 1 : 0;

    if (tileIndex & 0x1)
    {
        /* Inverse die index for tiles 1,3 */
        dieIndex = 3 - dieIndex;
    }

    if(isCpuPort)
    {
        /* each Raven with single CPU port , the 'global port' number */
        if(!devObjPtr->ravens_specialPortMappingArr)
        {
            skernelFatalError("smemFalconMtiPortByAddrGet : for CPU port must have 'ravens_specialPortMappingArr' \n");
            return 0;
        }

        /* check for 'special' port numbers */
        currInfoPtr = &devObjPtr->ravens_specialPortMappingArr[0];

        for (/*already init*/ ; currInfoPtr->globalPortNumber != SMAIN_NOT_VALID_CNS ; currInfoPtr++)
        {
            if(currInfoPtr->unitIndex ==  ((tileIndex * 4) + dieIndex))
            {
                return currInfoPtr->globalPortNumber;
            }
        }

        skernelFatalError("smemFalconMtiPortByAddrGet : CPU port in Global Raven[%d] was not defined as Valid ! \n",
            dieIndex);
        return 0;
    }


    /* Get relative address offset */
    #define PORT_OFFSET(address)    \
        (((address) - portBaseAddress) & portIndexMask)

    switch (mtiRegister)
    {
        case MTI_ACTIVE_MEM_REG_STAT_CONTROL_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_STATS.control;
            portIndex = 0;

            break;
        case MTI_ACTIVE_MEM_REG_STAT_CAPTURE_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[0].MTI_STATS.counterCapture[0];
            portIndex = 0;

            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_64_STATUS_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.portStatus;
            portIndex = PORT_OFFSET(address) / (0x18);

            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_400_STATUS_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.segPortStatus;
            portIndex = PORT_OFFSET(address) / (0x14);

            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_64_INTERRUPT_MASK_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.portInterruptMask;
            portIndex = PORT_OFFSET(address) / (0x18);

            break;
        case MTI_ACTIVE_MEM_REG_PORT_MTI_400_INTERRUPT_MASK_E:
            portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_EXT_PORT.segPortInterruptMask;
            portIndex = PORT_OFFSET(address) / (0x14);

            break;
        case MTI_ACTIVE_MEM_REG_PCS_CONTROL1_E:
            portIndexMask =  0xFFFF;
            /* normalize the address to give ports 0..7 in raven 0 */
            address &= 0x00FFFFFF & ~(channelIndex * 0x80000);
            if (address == SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[0].PCS_400G_200G.control1)
            {
                /* ports 0 200G/400G */
                portIndex = 0;
            }
            else
            if (address == SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[4].PCS_400G_200G.control1)
            {
                /* ports 4 200G */
                portIndex = 4;
            }
            else
            {
                /* ports 0..7 50/100G */
                portBaseAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[0].MTI_WRAPPER.PCS_10TO100.control1;
                portIndex = 0;
                portIndex += PORT_OFFSET(address) / (0x1000);
            }

            break;
        default:
            skernelFatalError("smemFalconMtiPortByAddrGet : unknown MTI type[%d] address[0x%8.8x]\n",
                mtiRegister, address);
    }

    /* Calculate global port number */
    portNum = 64 * tileIndex + 16 * dieIndex + 8 * channelIndex + portIndex;

    return portNum;
}

/* Calculate global MAC port from pipe port and port bitmap */
#define GLOBAL_MAC_CALC(bmp, port) \
    switch (bmp) { \
        case 1: \
            port += 0; break; \
        case 2: \
            port += 1; break; \
        case 4: \
            port += 2; break; \
        case 8: \
            port += 3; break; \
        case 16: \
            port += 4; break; \
        case 32: \
            port += 5; break; \
        case 64: \
            port += 6; break; \
        case 128: \
            port += 7; break; \
        default: \
            skernelFatalError("smemFalconActiveReadMtiStatisticCounters : wrong port mask[0x%8.8x], address[0x%8.8x]\n", \
                bmp, address); \
    }

/* Calculate global MAC port from pipe port and port bitmap */
#define GLOBAL_MAC_CALC_SUPPORT_PREEMPTIVE(bmp, port) \
    switch (bmp) { \
        case 1: \
        case 2: \
            port += 0; break; \
        case 4: \
        case 8: \
            port += 1; break; \
        case 16: \
        case 32: \
            port += 2; break; \
        case 64: \
        case 128: \
            port += 3; break; \
        case 256: \
        case 512: \
            port += 4; break; \
        case 1024: \
        case 2048: \
            port += 5; break; \
        case 4096: \
        case 8192: \
            port += 6; break; \
        case 16384: \
        case 32768: \
            port += 7; break; \
        default: \
            skernelFatalError("smemFalconActiveReadMtiStatisticCounters : PMAC/EMAC : wrong port mask[0x%8.8x], address[0x%8.8x]\n", \
                bmp, address); \
    }
static GT_U32 smemFalconMtiJumpFromRepresentativeMacToOtherMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_TYPE_ENT      macType,
    IN GT_U32                  representativeMac,
    IN GT_U32                  numMacsToJump
)
{
    GT_U32  ii,jj;
    ENHANCED_PORT_INFO_STC portInfo;

    if(devObjPtr->numOfRavens) /*Falcon */
    {
        return representativeMac + numMacsToJump;
    }

    devObjPtr->devMemPortInfoGetPtr(devObjPtr,macType  ,representativeMac,&portInfo);

    for(jj = 0 ; jj < SIP6_MTI_BMP_PORTS_SIZE; jj ++)
    {
        if(portInfo.sip6_MTI_bmpPorts[jj] == 0)
        {
            continue;
        }
        for(ii = 0 ; ii < 32 ; ii++)
        {
            if(0 == ((1 << ii) & portInfo.sip6_MTI_bmpPorts[jj]))
            {
                continue;
            }

            if(numMacsToJump == 0)
            {
                return representativeMac + ii + jj*32; /* we found proper jump */
            }
            numMacsToJump--;
        }
    }

    skernelFatalError("smemFalconMtiJumpFromRepresentativeMacToOtherMac: not found mac to jump to \n");
    return 0;
}


/**
* @internal smemFalconActiveReadMtiStatisticCounters function
* @endinternal
*
* @brief   Statistic counters read.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter - global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Performs capturing of the statistic entry to internal memory on
*       first word read.
*       The capture operation is undo reset on last word read.
*
*/
void smemFalconActiveReadMtiStatisticCounters
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32  regAddr;                /* register address */
    GT_U32  *regPtr;                /* register pointer */
    GT_U32  *controlRegPtr;         /* control register pointer */
    GT_U32  macPort;                /* port number */
    GT_U32  *internalMemPtr;
    GT_U32  counterNumber;          /* RX/TX counter number */
    GT_U32  fieldVal;               /* register field value */
    GT_U32  counterIndex;
    GT_U32  localPort;
    GT_U32  sip6_MTI_EXTERNAL_representativePortIndex = 0;
    GT_U32  isCpuPort;
    GT_U32  cpuPortIndex = 0;
    SMEM_UNIT_TYPE_ENT unitType;
    GT_U32  supportPreemptionMac = (GT_U32)param;
    GT_U32  usePmacCounters = 0;/* when preemption supported , need to know if to use
                                   counters from EMAC or from the PMAC */

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        ENHANCED_PORT_INFO_STC portInfo;

        macPort =  smemGopPortByAddrGet(devObjPtr,address);
        unitType = SMEM_MTI_UNIT_TYPE_GET(devObjPtr,macPort);

        isCpuPort = (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E);

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,unitType,macPort,&portInfo);

        if(isCpuPort)
        {
            cpuPortIndex      = portInfo.simplePortInfo.unitIndex;
        }
        else
        {
            sip6_MTI_EXTERNAL_representativePortIndex = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;
        }

    }
    else
    {
        /* Returns MAC port per group of 8 ports */
        macPort = smemFalconMtiPortByAddrGet(devObjPtr, address, MTI_ACTIVE_MEM_REG_STAT_CAPTURE_E);
        isCpuPort = (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E);
        if(isCpuPort)
        {
            GT_U32  globalRavenNum;
            smemRavenGlobalPortConvertGet(devObjPtr,macPort,&globalRavenNum,NULL,NULL,NULL,NULL,NULL);
            cpuPortIndex = globalRavenNum;
            unitType = SKERNEL_PORT_STATE_MTI_CPU_E;
        }
        else
        {
            sip6_MTI_EXTERNAL_representativePortIndex = macPort/8;
            unitType = SMEM_UNIT_TYPE_MTI_MAC_50G_E;
        }
    }

    /* get control register value */
    if(isCpuPort)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[cpuPortIndex].cpu_MTI_STATS.control;
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_STATS.control;
    }

    if(((regAddr) & 0xFFF00000) != ((address) & 0xFFF00000))
    {
        /* patch for Hawk MIB : the 'ports' that are both MTI and USX , not hold
           dedicated MIB for USX */
        /* so if we are here the 'DB' hold address from other unit that we support !  */
        * outMemPtr = 0;
        return;
    }

    controlRegPtr = smemMemGet(devObjPtr, regAddr);

    fieldVal = SMEM_U32_GET_FIELD(*controlRegPtr, 0, supportPreemptionMac ? 16 : 8);

    if(fieldVal == 0)
    {
        /* triggering was not set.
           Return current value of registers */
        * outMemPtr = * memPtr;
        return;
    }

    /* calculate pipe MAC port to global port */
    localPort = 0;
    if(supportPreemptionMac)
    {
        /* there are 2 consecutive bits per port */
        GLOBAL_MAC_CALC_SUPPORT_PREEMPTIVE(fieldVal, localPort);
        /* bits 1,3,5,7,9,11,13,15 are the PMAC */
        usePmacCounters = (fieldVal & 0xAAAA) ? 1 : 0;
    }
    else
    {
        GLOBAL_MAC_CALC(fieldVal, localPort);
    }

    /* jump from macPort '+' localPort */
    macPort = smemFalconMtiJumpFromRepresentativeMacToOtherMac(devObjPtr,
        unitType,macPort,localPort);

    counterNumber = (devObjPtr->portsArr[macPort].mtiTxCountersCapture == 0) ?
        CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE :
        CHT_INTERNAL_SIMULATION_TX_STATISTIC_PAGE;

    /* Capture memory */
    if(isCpuPort)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[cpuPortIndex].cpu_MTI_STATS.counterCapture[0];
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_STATS.counterCapture[0];
    }
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* low 32 bits word index in the internal memory page */
    counterIndex = (address - regAddr) / 4;

    /* copy the value from capture memory */
    *outMemPtr = regPtr[counterIndex];

    /* high 32 bits word */
    if(isCpuPort)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[cpuPortIndex].cpu_MTI_STATS.datHiCdc;
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_STATS.datHiCdc;
    }
    regPtr = smemMemGet(devObjPtr, regAddr);

    if(!usePmacCounters)
    {
        /* port page pointer */
        internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI_STATISTICS_COUNTERS_CAPTURE_HI_WORD_START_E
                                          + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort));
    }
    else
    {
        /* PMAC : port page pointer */
        internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI___PMAC___STATISTICS_COUNTERS_CAPTURE_HI_WORD_START_E
                                          + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort));
    }

    /* High word index in internal memory */
    if (devObjPtr->portsArr[macPort].mtiTxCountersCapture)
    {
        internalMemPtr += CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE;
    }

    /* copy the upper 32 bits of 64 bit counters from internal memory*/
    *regPtr = internalMemPtr[counterIndex];

    /* Last word read */
    if (counterIndex == counterNumber - 1)
    {
        /* clear RX/TX relevant bit */
        SMEM_U32_SET_FIELD(*controlRegPtr, 30+devObjPtr->portsArr[macPort].mtiTxCountersCapture, 1, 0);
    }
}

/**
* @internal smemFalconActiveWriteMtiStatisticControl function
* @endinternal
*
* @brief   Write MTI statistic control register:
*       <MTIP_IP> Statistics Units/STATN_CONTROL
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemFalconActiveWriteMtiStatisticControl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  regAddr;                /* register address */
    GT_U32  *regPtr;                /* register pointer */
    GT_U32  macPort;                /* port number */
    GT_U32  *captureMemPtr;
    GT_U32  fieldVal;
    GT_U32  txBit;                  /* TX capture enable bit */
    GT_U32  counterNumber;          /* RX/TX counter number */
    GT_U32  localPort;
    GT_U32  sip6_MTI_EXTERNAL_representativePortIndex = 0;
    GT_U32  isCpuPort;
    GT_U32  cpuPortIndex = 0;
    SMEM_UNIT_TYPE_ENT unitType;
    GT_U32  supportPreemptionMac = (GT_U32)param;
    GT_U32  usePmacCounters = 0;/* when preemption supported , need to know if to use
                                   counters from EMAC or from the PMAC */
    GT_U32  cmd_clear; /* do we need to clear the counters after the read */

    DECLARE_FUNC_NAME(smemFalconActiveWriteMtiStatisticControl);

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        ENHANCED_PORT_INFO_STC portInfo;

        macPort =  smemGopPortByAddrGet(devObjPtr,address);

        unitType = SMEM_MTI_UNIT_TYPE_GET(devObjPtr,macPort);

        isCpuPort = (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E);

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,unitType,macPort,&portInfo);

        if(isCpuPort)
        {
            cpuPortIndex      = portInfo.simplePortInfo.unitIndex;
        }
        else
        {
            sip6_MTI_EXTERNAL_representativePortIndex = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;
        }

    }
    else
    {
        /* Returns MAC port per group of 8 ports */
        macPort = smemFalconMtiPortByAddrGet(devObjPtr, address, MTI_ACTIVE_MEM_REG_STAT_CONTROL_E);
        isCpuPort = (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E);
        if(isCpuPort)
        {
            GT_U32  globalRavenNum;
            smemRavenGlobalPortConvertGet(devObjPtr,macPort,&globalRavenNum,NULL,NULL,NULL,NULL,NULL);
            cpuPortIndex = globalRavenNum;
            unitType = SKERNEL_PORT_STATE_MTI_CPU_E;
        }
        else
        {
            sip6_MTI_EXTERNAL_representativePortIndex = macPort/8;
            unitType = SMEM_UNIT_TYPE_MTI_MAC_50G_E;
        }
    }


    /* Output value */
    *memPtr = *inMemPtr;

    /* RX/TX capture enable bits */
    fieldVal = SMEM_U32_GET_FIELD(*inMemPtr, 27, 2);
    if (fieldVal == 0 || fieldVal == 3)
    {
        __LOG(("Invalid RX/TX statistic capture state: RX[%d] TX[%d] \n",
              fieldVal & 1, fieldVal >> 1));
        return;
    }
    /* TX capture related bit */
    txBit = fieldVal >> 1;
    counterNumber = (txBit == 0) ?
        CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE :
        CHT_INTERNAL_SIMULATION_TX_STATISTIC_PAGE;

    fieldVal = SMEM_U32_GET_FIELD(*inMemPtr, 0, supportPreemptionMac ? 16 : 8);

    /* calculate pipe MAC port to global port */
    localPort = 0;
    if(supportPreemptionMac)
    {
        /* there are 2 consecutive bits per port */
        GLOBAL_MAC_CALC_SUPPORT_PREEMPTIVE(fieldVal, localPort);
        /* bits 1,3,5,7,9,11,13,15 are the PMAC */
        usePmacCounters = (fieldVal & 0xAAAA) ? 1 : 0;
    }
    else
    {
        GLOBAL_MAC_CALC(fieldVal, localPort);
    }

    /* jump from macPort '+' localPort */
    macPort = smemFalconMtiJumpFromRepresentativeMacToOtherMac(devObjPtr,
        unitType,
        macPort,localPort);

    /* Capture memory */
    if(isCpuPort)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[cpuPortIndex].cpu_MTI_STATS.counterCapture[0];
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_STATS.counterCapture[0];
    }

    if(((regAddr) & 0xFFF00000) != ((address) & 0xFFF00000))
    {
        /* patch for Hawk MIB : the 'ports' that are both MTI and USX , not hold
           dedicated MIB for USX */
        /* so if we are here the 'DB' hold address from other unit that we support !  */
        return;
    }

    captureMemPtr = smemMemGet(devObjPtr, regAddr);

    /* Port page pointer */
    if (txBit)
    {
        regAddr = CHT_MTI_PORTS_STATISTIC_TX_COUNTERS_BASE_ADDRESS_MAC(devObjPtr, macPort,usePmacCounters);
    }
    else
    {
        regAddr = CHT_MTI_PORTS_STATISTIC_RX_COUNTERS_BASE_ADDRESS_MAC(devObjPtr, macPort,usePmacCounters);
    }

    /* Port page pointer */
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* copy the values of MTI counter entry (low words) to capture memory */
    memcpy(captureMemPtr, regPtr, counterNumber * sizeof(GT_U32));

    if(!usePmacCounters)
    {
        /* copy from shadow of running counters of high 32 bits to the
                     shadow of captured counters of high 32 bits*/
        memcpy(CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI_STATISTICS_COUNTERS_CAPTURE_HI_WORD_START_E
                    + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
               CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI_STATISTICS_COUNTERS_HI_WORD_START_E
                    + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
               counterNumber * sizeof(GT_U32));
    }
    else
    {
        /* copy from shadow of running counters of high 32 bits to the
                     shadow of captured counters of high 32 bits*/
        memcpy(CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI___PMAC___STATISTICS_COUNTERS_CAPTURE_HI_WORD_START_E
                    + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
               CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI___PMAC___STATISTICS_COUNTERS_HI_WORD_START_E
                    + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
               counterNumber * sizeof(GT_U32));
    }

    /* clear the RX/TX relevant bit */
    SMEM_U32_SET_FIELD(*memPtr, 27 + txBit, 1, 0);

    /* Indicate RX/TX capture */
    devObjPtr->portsArr[macPort].mtiTxCountersCapture = txBit;

    cmd_clear = SMEM_U32_GET_FIELD(*memPtr, 30 + txBit, 1);

    if(cmd_clear)
    {
        /* Clear counters as we already copied them to the captured registers (and to the shadow of captured upper 32 bits) */
        memset(regPtr, 0, counterNumber * sizeof(GT_U32));
        /* Clear the upper 32 bits of each counters too */
        if(!usePmacCounters)
        {
            memset(CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI_STATISTICS_COUNTERS_HI_WORD_START_E
                        + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
                   0,
                   counterNumber * sizeof(GT_U32));
        }
        else
        {
            memset(CHT_INTERNAL_MEM_PTR(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_MTI___PMAC___STATISTICS_COUNTERS_HI_WORD_START_E
                        + (CHT_INTERNAL_SIMULATION_STATISTIC_NUM_REGISTERS_CNS * macPort)) + CHT_INTERNAL_SIMULATION_RX_STATISTIC_PAGE * txBit,
                   0,
                   counterNumber * sizeof(GT_U32));
        }
    }

}

/**
* @internal smemFalconActiveWritePcsControl1 function
* @endinternal
*
* @brief   Write Control 1 register in the MTI PCS registers:
*       <MTIP_IP> pcs10254050/Units %i %j/Registers/CONTROL1
*       <MTIP_IP> pcs10to100/Units %i %j/Registers/CONTROL1
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemFalconActiveWritePcsControl1
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port;             /* Port number */

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        port =  smemGopPortByAddrGet(devObjPtr,address);
    }
    else
    {
        port = smemFalconMtiPortByAddrGet(devObjPtr, address, (MTI_ACTIVE_MEM_REG_ENT)param);
    }

    /*  *memPtr = *inMemPtr ; is done inside : smemFalconCheckAndSendLoopBackChange */
    smemFalconCheckAndSendLoopBackChange(devObjPtr, port, memPtr, inMemPtr);
}

/**
* @internal smemFalconActiveWritePortInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemFalconActiveWritePortInterruptsMaskReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 port;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        port =  smemGopPortByAddrGet(devObjPtr,address);
    }
    else
    {
        port = smemFalconMtiPortByAddrGet(devObjPtr, address, (MTI_ACTIVE_MEM_REG_ENT)param);
    }

    /* Address of Port<n> interrupt cause register */
    dataArray[0] = SMEM_CHT_PORT_INT_CAUSE_REG(devObjPtr, port);
    /* Address of Port<n> table interrupt mask register */
    dataArray[1] = SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devObjPtr, port);
    /* Ports Summary Interrupt register SUM bit. */
    dataArray[2] = SMEM_CHT_PORT_CAUSE_INT_SUM_BMP(devObjPtr, port);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemFalconActiveWriteCpuPortInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemFalconActiveWriteCpuPortInterruptsMaskReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 unitIndex;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        ENHANCED_PORT_INFO_STC portInfo;
        GT_U32  macPort =  smemGopPortByAddrGet(devObjPtr,address);

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,macPort,&portInfo);

        unitIndex = portInfo.simplePortInfo.unitIndex;
    }
    else
    {
        unitIndex = (address >> 24) & 0x3;
    }


    /* Address of Port<n> interrupt cause register */
    dataArray[0] = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptCause;
    /* Address of Port<n> table interrupt mask register */
    dataArray[1] = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptMask;
    /* Ports Summary Interrupt register SUM bit. */
    dataArray[2] = SMEM_CHT_CPU_PORT_CAUSE_INT_SUM_BMP(devObjPtr);

    if (dataArray[0] == SMAIN_NOT_VALID_CNS ||
        dataArray[1] == SMAIN_NOT_VALID_CNS)
    {
        /* No CPU port for this Raven */
        return;
    }

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemFalconActiveWriteExactMatchMsg function
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
static void smemFalconActiveWriteExactMatchMsg(
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

    DECLARE_FUNC_NAME(smemFalconActiveWriteExactMatchMsg);

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
    if (rdWr)/* write the data from the msg registers to the FDB table  */
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
* @internal smemFalconActiveWriteExactMatchGlobalCfg1Reg
*           function
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
static void smemFalconActiveWriteExactMatchGlobalCfg1Reg
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

    /* data to be written */
    *memPtr = *inMemPtr;

    /* emSize */
    emSize = SMEM_U32_GET_FIELD(*inMemPtr, 11, 3);

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

    return;
}
/**
* @internal smemFalconActiveWriteExactMatchGlobalCfg2Reg
*           function
* @endinternal
*
* @brief   The function updates Exact Match number of banks according
*          to EM_Global_Configuration 2
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWriteExactMatchGlobalCfg2Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 banksNumber;
    GT_U32 emNumOfBanks;

    /* data to be written */
    *memPtr = *inMemPtr;

    /* emSize */
    banksNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 2);

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
* @internal smemFalconActiveWriteFdbGlobalCfg2Reg
*           function
* @endinternal
*
* @brief   The function updates FDB number of bits per bank according
*          to FDB_Global_Configuration 2
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemFalconActiveWriteFdbGlobalCfg2Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 numOfHashes;         /* Number of Multiple Hash Tables */

    /* data to be written */
    *memPtr = *inMemPtr;

    /* Number of Multiple Hash Tables */
    numOfHashes = SMEM_U32_GET_FIELD(*inMemPtr, 2, 2);

    switch(numOfHashes)
    {
        case 0:
            devObjPtr->fdbNumOfBanks = 4;
            break;
        case 1:
            devObjPtr->fdbNumOfBanks = 8;
            break;
        default:
            devObjPtr->fdbNumOfBanks = 16;
            break;
    }

    return;
}

static void smemFalconAacTableEntryWrite(
    IN  SKERNEL_DEVICE_OBJECT           * devObjPtr,
    IN  SMEM_GENERIC_HIDDEN_MEM_ENT     channelType,
    IN  SMEM_GENERIC_HIDDEN_MEM_STC     *infoPtr,
    IN  GT_U32                          addressMode,
    IN  GT_U32                          startAddress,
    IN  GT_U32                          tile,
    IN  GT_U32                          tileOffset,
    IN  GT_U32                          unit,
    IN  GT_U32                          unitOffset,
    IN  GT_U32                          wordIndex
)
{
    GT_U32                          regAddress;         /* register address */
    GT_U32                          * regDataPtr;       /* pointer to register data */

    if (addressMode == 0)
    {
        regDataPtr  = &infoPtr->startOfMemPtr[wordIndex];
        regAddress  = startAddress + tile * tileOffset + unit * unitOffset + wordIndex * 0x4;
    }
    else
    {
        startAddress = infoPtr->startOfMemPtr[wordIndex];
        regAddress  = startAddress + tile * tileOffset + unit * unitOffset;
        regDataPtr  = &infoPtr->startOfMemPtr[wordIndex+1];
    }

    smemReadWriteMem(SKERNEL_MEMORY_WRITE_E, devObjPtr, regAddress, 1, regDataPtr);
}

static void smemFalconAacChannelWrite(
    IN      SKERNEL_DEVICE_OBJECT           * devObjPtr,
    IN      AAC_ENGINE_REGISTER_TYPE_ENT    aacRegType,
    IN      GT_U32                          startAddress,
    INOUT   SMEM_GENERIC_HIDDEN_MEM_STC     *infoPtr
)
{
    GT_U32                          regAddress;         /* register address */
    GT_U32                          * regDataPtr;       /* pointer to register data */
    GT_U32                          * regStatusPtr;     /* pointer to register status */
    GT_U32                          fieldValue;
    GT_U32                          channelIndex;
    SMEM_GENERIC_HIDDEN_MEM_ENT     channelType;
    GT_U32                          tile;
    GT_U32                          unit;
    GT_U32                          unitOffset;
    GT_U32                          unitNumbers;
    GT_U32                          tileOffset;
    GT_U32                          tileNumbers;
    GT_U32                          numOfWords;
    GT_U32                          ii;
    GT_U32                          addressMode;         /* Direct/indirect */
    GT_U32                          addressModeStep;

#define AAC_CHANNEL_PARAM_SET(index)                                    \
    channelIndex = index;                                               \
    channelType = SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL##index##_E

    switch (aacRegType)
    {
        case AAC_ENGINE_ADDRESS_CHANNEL_0_E:
            AAC_CHANNEL_PARAM_SET(0);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_1_E:
            AAC_CHANNEL_PARAM_SET(1);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_2_E:
            AAC_CHANNEL_PARAM_SET(2);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_3_E:
            AAC_CHANNEL_PARAM_SET(3);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_4_E:
            AAC_CHANNEL_PARAM_SET(4);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_5_E:
            AAC_CHANNEL_PARAM_SET(5);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_6_E:
            AAC_CHANNEL_PARAM_SET(6);
            break;
        case AAC_ENGINE_ADDRESS_CHANNEL_7_E:
            AAC_CHANNEL_PARAM_SET(7);
            break;
        default:
            skernelFatalError("smemFalconFdbAacChannelWrite: not valid AAC address channel[0x%08x]\n", aacRegType);
            return;
    }

    /* Number of data words to be written in memory */
    numOfWords = infoPtr->currentWordIndex;

    /* AAC Engine <<%n>> Control */
    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineControl[channelIndex];
    regDataPtr = smemMemGet(devObjPtr, regAddress);

    /* AAC Addr Mode */
    addressMode = SMEM_U32_GET_FIELD(*regDataPtr, 0, 1);

    if (addressMode)
    {
        /* Indirect access */
        addressModeStep = 2;
    }
    else
    {
        /* Direct access */
        addressModeStep = 1;
    }
    /* AAC Engine Channel control */
    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineControl[channelIndex];
    regDataPtr = smemMemGet(devObjPtr, regAddress);

    /* AAC Engine <<%n>> Multi Casting Enable */
    fieldValue = SMEM_U32_GET_FIELD(*regDataPtr, 2, 1);

    /* MC enable - write in all tiles */
    if (fieldValue == 1)
    {
        /* AAC Engine <<%n>> MC External Offset */
        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCExternalOffset[channelIndex];
        regDataPtr = smemMemGet(devObjPtr, regAddress);
        tileOffset = *regDataPtr;

        /* AAC Engine <<%n>> MC Internal Offset */
        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCInternalOffset[channelIndex];
        regDataPtr = smemMemGet(devObjPtr, regAddress);
        unitOffset = *regDataPtr;

        /* AAC Engine <<%n>> MC Control */
        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEngineMCControl[channelIndex];
        regDataPtr = smemMemGet(devObjPtr, regAddress);

        /* AAC Engine <<%n>> MC External Iteration */
        tileNumbers = SMEM_U32_GET_FIELD(*regDataPtr,  0, 10);
        unitNumbers = SMEM_U32_GET_FIELD(*regDataPtr, 10, 10);

        /* At least one unit per tile */
        unitNumbers = (unitNumbers) ? unitNumbers : 1;

        for (tile = 0; tile < tileNumbers; tile++)
        {
            for (unit = 0; unit < unitNumbers; unit++)
            {
                for (ii = 0; ii < numOfWords; ii+=addressModeStep)
                {
                    smemFalconAacTableEntryWrite(devObjPtr, channelType, infoPtr,
                                                 addressMode, startAddress,
                                                 tile, tileOffset, unit, unitOffset, ii);
                }
            }
        }
    }
    else
    {
        /* Single tile write */
        for (ii = 0; ii < numOfWords; ii+=addressModeStep)
        {
            smemFalconAacTableEntryWrite(devObjPtr, channelType, infoPtr,
                                         addressMode, startAddress,
                                         0, 0, 0, 0, ii);
        }
    }

    /* reset current word index */
    infoPtr->currentWordIndex = 0;

    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEnginesStatus;
    regStatusPtr = smemMemGet(devObjPtr, regAddress);

    /* Clear Channel status - idle */
    SMEM_U32_SET_FIELD(*regStatusPtr, channelIndex, 1, 0);
}

/**
* @internal smemFalconActiveWriteAacTableMemory function
* @endinternal
*
* @brief   Write to the AAC memory registers - activate
*          update FDB AAC memory access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
*/
static void smemFalconActiveWriteAacTableMemory(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32                  address,
    IN  GT_U32                  memSize,
    IN  GT_U32                  *memPtr,
    IN  GT_UINTPTR              param,
    IN  GT_U32                * inMemPtr
)
{
    GT_U32                          regAddress;         /* register address */
    AAC_ENGINE_REGISTER_TYPE_ENT    aacRegType;         /* AAC register type */
    GT_U32                          * regStatusPtr;     /* pointer to register status */
    GT_U32                          regDataMask;        /* register data mask */
    SMEM_GENERIC_HIDDEN_MEM_ENT     currentChannel;     /* current channel */
    SMEM_GENERIC_HIDDEN_MEM_STC     *infoPtr;

    DECLARE_FUNC_NAME(smemFalconActiveWriteAacTableMemory);

#define AAC_DATA_CHANNEL_PARAM_SET(index)                                   \
    regDataMask = 1 << AAC_ENGINE_DATA_CHANNEL_##index##_E;                 \
    aacRegType = AAC_ENGINE_DATA_CHANNEL_##index##_E;                       \
    currentChannel = SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL##index##_E;    \
    if ((*regStatusPtr & regDataMask) == 0)                                 \
    {                                                                       \
        /* Set Channel status - busy */                                     \
        SMEM_U32_SET_FIELD(*regStatusPtr, index, 1, 1);                     \
    }

#define AAC_ADDRESS_CHANNEL_PARAM_SET(index)                                \
    aacRegType = AAC_ENGINE_ADDRESS_CHANNEL_##index##_E;                    \
    currentChannel = SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL##index##_E

    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.AAC.AACEnginesStatus;
    regStatusPtr = smemMemGet(devObjPtr, regAddress);

    /* data to be written */
    *memPtr = *inMemPtr;

    regDataMask = 1 << AAC_ENGINE_DATA_CHANNEL_1_E;
    switch (address & 0xFF)
    {
        case 0x50:
            AAC_DATA_CHANNEL_PARAM_SET(0);
            break;
        case 0x54:
            AAC_DATA_CHANNEL_PARAM_SET(1);
            break;
        case 0x58:
            AAC_DATA_CHANNEL_PARAM_SET(2);
            break;
        case 0x5c:
            AAC_DATA_CHANNEL_PARAM_SET(3);
            break;
        case 0x60:
            AAC_DATA_CHANNEL_PARAM_SET(4);
            break;
        case 0x64:
            AAC_DATA_CHANNEL_PARAM_SET(5);
            break;
        case 0x68:
            AAC_DATA_CHANNEL_PARAM_SET(6);
            break;
        case 0x6c:
            AAC_DATA_CHANNEL_PARAM_SET(7);
            break;
        case 0x80:
            AAC_ADDRESS_CHANNEL_PARAM_SET(0);
            break;
        case 0x84:
            AAC_ADDRESS_CHANNEL_PARAM_SET(1);
            break;
        case 0x88:
            AAC_ADDRESS_CHANNEL_PARAM_SET(2);
            break;
        case 0x8c:
            AAC_ADDRESS_CHANNEL_PARAM_SET(3);
            break;
        case 0x90:
            AAC_ADDRESS_CHANNEL_PARAM_SET(4);
            break;
        case 0x94:
            AAC_ADDRESS_CHANNEL_PARAM_SET(5);
            break;
        case 0x98:
            AAC_ADDRESS_CHANNEL_PARAM_SET(6);
            break;
        case 0x9c:
            AAC_ADDRESS_CHANNEL_PARAM_SET(7);
            break;
        default:
            skernelFatalError("smemFalconActiveWriteAacTableMemory: not AAC address [0x%08x]\n", address);
            return;
    }

    __LOG(("smemFalconActiveWriteAacTableMemory: AAC register [0x%08x] value [0x%08x]  \n", address,  *inMemPtr));

    infoPtr = SMEM_HIDDEN_MEM_INFO_GET(devObjPtr, currentChannel, 0);

    if (aacRegType < AAC_ENGINE_ADDRESS_CHANNEL_0_E)
    {
        smemGenericHiddenMemSet(devObjPtr, currentChannel, infoPtr->currentWordIndex, inMemPtr, 1);
        infoPtr->currentWordIndex++;
        if (infoPtr->currentWordIndex > infoPtr->numOfEntries)
        {
            skernelFatalError("smemFalconActiveWriteAacTableMemory : number of AAC transaction exceeded maximal value[%d] \n",
                infoPtr->numOfEntries);
        }
    }
    else
    {
        smemFalconAacChannelWrite(devObjPtr, aacRegType, *inMemPtr, infoPtr);
    }

    return;
}

static GT_U32 smemFalconTxqQfcCounterTable1Get(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 globalDmaPort
)
{
    if(globalDmaPort >= SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS)
    {
        skernelFatalError("smemFalconTxqQfcCounterTable1Get : globalDmaPort[%d] >= 'SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS[%d]' \n",
            globalDmaPort,
            SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS);
        return 0;
    }

    return devObjPtr->txqPortsArr[globalDmaPort].sip6NumPacketsPerTxDmaInTheTxq;
}

/**
* @internal smemFalconTxqQfcCounterTable1Update function
* @endinternal
*
* @brief   function to update the number of descriptors in the txq of a txdma port.
*
* @param[in] devObjPtr      - pointer to device object.
* @param[in] globalDmaPort  - global dma number
* @param[in] increment      - GT_TRUE - increment , GT_FALSE - decrement

*/
void smemFalconTxqQfcCounterTable1Update(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 globalDmaPort,
    IN GT_BOOL  increment
)
{

    if(globalDmaPort >= SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS)
    {
        skernelFatalError("smemFalconTxqQfcCounterTable1Update : globalDmaPort[%d] >= 'SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS[%d]' \n",
            globalDmaPort,
            SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS);
        return;
    }

    SCIB_SEM_TAKE;
    if(increment == GT_TRUE)
    {
        devObjPtr->txqPortsArr[globalDmaPort].sip6NumPacketsPerTxDmaInTheTxq ++;
    }
    else
    {
        if(devObjPtr->txqPortsArr[globalDmaPort].sip6NumPacketsPerTxDmaInTheTxq == 0)
        {
            skernelFatalError("smemFalconTxqQfcCounterTable1Update : globalDmaPort[%d] sip6NumPacketsPerTxDmaInTheTxq is 0 but need to decrement ?! \n",
                globalDmaPort);
        }
        else
        {
            devObjPtr->txqPortsArr[globalDmaPort].sip6NumPacketsPerTxDmaInTheTxq --;
        }
    }
    SCIB_SEM_SIGNAL;
}


/**
* @internal smemFalconActiveWriteTxqQfcCounterTable1IndirectReadAddress function
* @endinternal
*
* @brief   write to indirect mechanism to get the queue level in other register.
*       active memory on : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address
*       need to 'read' value to : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data
*           and set bit 17 in it
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemFalconActiveWriteTxqQfcCounterTable1IndirectReadAddress (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  tmpRegAddr;
    GT_U32  regAddr,offset,value,index;
    GT_U32  dpIndex = 0;
    GT_U32  globalDmaPort;
    GT_CHAR* unitName;
    GT_U32 readyBitIndex = param;
    GT_U32 tileId;

    /* Update the register value */
    *memPtr = *inMemPtr;

    index = (*memPtr) & 0x3FF;

    offset =  (SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address -
               SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data);

    regAddr = address - offset;

    if(devObjPtr->numOfPipes > 1)
    {
        /* strip the dpIndex and tile from the address */
        tmpRegAddr = regAddr % TILE_OFFSET;
        if(tmpRegAddr >= PIPE_1_START_ADDR && tmpRegAddr <= PIPE_1_END_ADDR)
        {
            tmpRegAddr -= PIPE_MEM_SIZE;
        }
    }
    else
    {
        tmpRegAddr = regAddr;
    }

    unitName = smemUnitNameByAddressGet(devObjPtr,tmpRegAddr);

    if     (0 == strcmp(unitName,STR(UNIT_TXQ_QFC0))){dpIndex = 0;}
    else if(0 == strcmp(unitName,STR(UNIT_TXQ_QFC1))){dpIndex = 1;}
    else if(0 == strcmp(unitName,STR(UNIT_TXQ_QFC2))){dpIndex = 2;}
    else                                             {dpIndex = 3;}

    if(devObjPtr->numOfTiles >= 2)
    {
        tileId = regAddr / devObjPtr->tileOffset;
        if((1<<tileId) & devObjPtr->mirroredTilesBmp)
        {
            /* mirrored tile ... need to swap the DPs within the pipe in the tile */
            dpIndex = (devObjPtr->multiDataPath.maxDp - (dpIndex+1));
        }
    }

    /* the caller of this function expect to get 'global' device port number */
    /* so lets convert it */
    smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
        devObjPtr,
        smemGetCurrentPipeId(devObjPtr), /* current pipe */
        dpIndex, /* DP unit local to the current pipe */
        index,
        GT_FALSE,/* CPU port not muxed ... not relevant */
        &globalDmaPort);

    /* get the number of descriptors in the queue of this dma */
    value = smemFalconTxqQfcCounterTable1Get(devObjPtr,globalDmaPort);
    value |= 1<<readyBitIndex;

    smemRegSet(devObjPtr,regAddr, value);
}

/**
* @internal smemFalconActiveWriteToTxDmaDebugConfig function
* @endinternal
*
* @brief   Falcon TxDma debug enable counters and reset counters register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemFalconActiveWriteToTxDmaEnableDebugCounters
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  regAddr,offset;
    GT_BIT  new_count_enable = (*inMemPtr) & 1;/* check bit 0 */

    /* data to be written */
    *memPtr = *inMemPtr;

    if(new_count_enable == 1)/* according to 'design team' it is done regardless to previous value ! */
    {
        /************************************/
        /* need to reset all debug counters */
        /************************************/

        /* calculate offset from the needed counter */
        offset = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].debug.enable_debug_Counters - /* address of this 'active' register in DP[0]*/
                 SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].TXD_Status.global_status.events_counters.pds_desc_counter; /* address of 'counter' in DP[0]*/
        regAddr = address - offset;

        /* clear the counter */
        smemRegSet(devObjPtr,regAddr, 0x0);
    }

    return;
}

void hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {

         {STR(UNIT_BMA)                            ,smemFalconUnitBma}

        ,{STR(UNIT_CNM_RFU)     ,smemFalconUnitCnmRfu}
        ,{STR(UNIT_CNM_AAC)     ,smemFalconUnitCnmAac}
         ,{STR(UNIT_CNM_SRAM)     ,smemFalconUnitCnmSramUnit}
        ,{STR(UNIT_CNM_PEX_MAC) ,smemFalconUnitCnmPexMacUnit}
        ,{STR(UNIT_IA)          ,smemFalconUnitIa}
        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    smemBobcat3SpecificDeviceUnitAlloc_DP_units(devObjPtr);
}
/**
* @internal smemFalconActiveWriteConfiProcessorGlobalConfiguration function
* @endinternal
*
* @brief   Activate CM3.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter - global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
*
*/
void smemFalconActiveWriteConfiProcessorGlobalConfiguration
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

    if(*memPtr&(1<<28))
    {
         microInitCm3Trigger();
    }
}


/**
* @internal smemFalconActiveWriteConfiProcessorIpcMemory function
* @endinternal
*
* @brief   Trigger IPC with CM3
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter - global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
*
*/
void smemFalconActiveWriteConfiProcessorIpcMemory
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    DECLARE_FUNC_NAME(smemFalconActiveWriteConfiProcessorIpcMemory);

     /* data to be written */
    *memPtr = *outMemPtr;

   __LOG(("IPC memory modification detected.\n"));

   microInitCm3Unq_pollingPerProjectUserFuntion();

}


