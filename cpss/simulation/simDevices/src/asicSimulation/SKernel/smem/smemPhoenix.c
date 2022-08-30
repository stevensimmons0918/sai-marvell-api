/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemPhoenix.c
*
* DESCRIPTION:
*       Phoenix memory mapping implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemPhoenix.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <common/Utils/Math/sMath.h>

/* was 1K in Falcon */
#define SIP6_10_NUM_DEFAULT_E_PORTS_CNS        128

/* 3 MG units */
#define PHOENIX_NUM_MG_UNITS  3
/* the size in bytes of the MG unit */
#define MG_SIZE             _1M

/* base address of the MG 0_0 unit . MG_0_0 to MG_0_3 serve tile 0 */
#define MG_0_0_OFFSET_CNS    0x7F900000
/* base address of the MG 0_1 unit . MG_0_1 is part of tile 0 */
#define MG_0_1_OFFSET_CNS    (MG_0_0_OFFSET_CNS + 1*MG_SIZE)
/* used for GM devices */
GT_U32   simPhoenixMgBaseAddr = MG_0_0_OFFSET_CNS;

#define LMU_0_BASE_ADDR 0x9A000000 /*PCA with MACSEC*/

#define TSU_0_BASE_ADDR 0x9A200000 /*PCA with MACSEC*/

#define MAC_MTIP_MAC_0_BASE_ADDR        0xA7800000
#define MAC_MTIP_MAC_PCS_0_BASE_ADDR    0xA7C00000

#define MTIP_USX_0_MAC_0_BASE_ADDR      0x9D800000  /*MACs  0.. 7 */
#define MTIP_USX_1_MAC_0_BASE_ADDR      0x9DA00000  /*MACs  8..15 */
#define MTIP_USX_2_MAC_0_BASE_ADDR      0x9E800000  /*MACs 16..23 */
#define MTIP_USX_0_MAC_1_BASE_ADDR      0x9EA00000  /*MACs 24..31 */
#define MTIP_USX_1_MAC_1_BASE_ADDR      0xA4800000  /*MACs 32..39 */
#define MTIP_USX_2_MAC_1_BASE_ADDR      0xA4C00000  /*MACs 40..47 */

#define MTIP_UNIT_USX_0_PCS_0_BASE_ADDR 0xA3800000  /*MACs  0.. 7 */
#define MTIP_UNIT_USX_1_PCS_0_BASE_ADDR 0xA3C00000  /*MACs  8..15 */
#define MTIP_UNIT_USX_2_PCS_0_BASE_ADDR 0xA4000000  /*MACs 16..23 */
#define MTIP_UNIT_USX_0_PCS_1_BASE_ADDR 0xA4400000  /*MACs 24..31 */
#define MTIP_UNIT_USX_1_PCS_1_BASE_ADDR 0xA0800000  /*MACs 32..39 */
#define MTIP_UNIT_USX_2_PCS_1_BASE_ADDR 0xA0C00000  /*MACs 40..47 */

#define MIF_0_BASE_ADDR           0x9E000000  /*USX_MIF_0*/
#define MIF_1_BASE_ADDR           0x9F000000  /*USX_MIF_1*/
#define MIF_2_BASE_ADDR           0xA5800000  /*USX_MIF_2*/
#define MIF_3_BASE_ADDR           0xA5C00000  /*CPU_MIF_3*/
#define MIF_4_BASE_ADDR           0xA8400000  /*100G_MIF_4*/

#define ANP_0_BASE_ADDR           0xA2800000 /* ANP for USX 0 */
#define ANP_1_BASE_ADDR           0xA2C00000 /* ANP for USX 1 */
#define ANP_2_BASE_ADDR           0xA3000000 /* ANP for USX 2 */
#define ANP_3_BASE_ADDR           0xA3400000 /* ANP for USX 3 */
#define ANP_4_BASE_ADDR           0x9F800000 /* ANP for USX 4 */
#define ANP_5_BASE_ADDR           0x9FC00000 /* ANP for USX 5 */
#define ANP_6_BASE_ADDR           0xA0000000 /* ANP for CPU 0 */
#define ANP_7_BASE_ADDR           0xA0400000 /* ANP for CPU 1 */
#define ANP_8_BASE_ADDR           0xA7E00000 /* ANP for mti100 */


#define NUM_PORTS_PER_100G_UNIT   4    /* 4 MACs of 25G in the MAC 100 unit */
#define NUM_PORTS_PER_USX_UNIT    8    /* 8 MACs in the USX unit  */
#define NUM_PORTS_PER_DP_UNIT     55   /*54 + 2 for SDMA CPU */
#define NUM_PORTS_PER_CTSU_UNIT   64
#define PHOENIX_MAX_PROFILE_CNS   12

/* use runtime parameter of number of DP channels (ports)   */
/* this to allow the Ironman to use the Phoenix units allocations */
/* as at this moment there is no Cider for the Ironman      */
#define RUNTIME_NUM_PORTS_PER_DP_UNIT \
    ((devObjPtr->multiDataPath.info[0].cpuPortDmaNum >= devObjPtr->multiDataPath.info[0].dataPathNumOfPorts) ? \
      devObjPtr->multiDataPath.info[0].cpuPortDmaNum  :                                                        \
      devObjPtr->multiDataPath.info[0].dataPathNumOfPorts)


void update_mem_chunk_from_size_to_size(
    IN SMEM_CHUNK_BASIC_STC    chunksMemArr[],
    IN GT_U32                  numOfChunks,
    IN GT_U32                  origNumOfBytes,
    IN GT_U32                  newNumOfBytes
)
{
    GT_U32  ii;
    GT_U32  origNumOfRegistres = origNumOfBytes / 4;

    for(ii = 0 ; ii < numOfChunks; ii++)
    {
        if(chunksMemArr[ii].numOfRegisters == origNumOfRegistres)
        {
            chunksMemArr[ii].numOfRegisters = newNumOfBytes / 4;
        }
    }
}

void update_list_register_default_size_to_size(
    IN SMEM_REGISTER_DEFAULT_VALUE_STC    registersDefaultValueArr[],
    IN GT_U32                  origNumOfRepetitions,
    IN GT_U32                  newNumOfRepetitions
)
{
    GT_U32  ii;

    for(ii = 0 ; registersDefaultValueArr[ii].unitNameStrPtr ; ii++)
    {
        if(registersDefaultValueArr[ii].numOfRepetitions == origNumOfRepetitions)
        {
            registersDefaultValueArr[ii].numOfRepetitions = newNumOfRepetitions;
        }
    }
}

#define UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE(_chunksMem,_numOfChunks,_origNumOfBytes,_newNumOfBytes) \
        update_mem_chunk_from_size_to_size(_chunksMem,_numOfChunks,_origNumOfBytes,_newNumOfBytes)


#define UPDATE_LIST_REGISTER_DEFAULT_SIZE_TO_SIZE(_registersDefaultValueArr,_origNumOfRepetitions,_newNumOfRepetitions) \
        update_list_register_default_size_to_size(_registersDefaultValueArr,_origNumOfRepetitions,_newNumOfRepetitions)

/* offsets within the : Phoenix/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x00001000
#define PORT0_100G_OFFSET         0x00002000

#define MAC_STEP_PORT_OFFSET      0x00001000

/********** start : PCS for 100G *********/
#define PCS_PORT0_100G_OFFSET     0x00000000
#define PCS_PORT0_50G_OFFSET      PCS_STEP_PORT_OFFSET
#define PCS_PORT1_25G_OFFSET      (PCS_PORT0_50G_OFFSET+PCS_STEP_PORT_OFFSET)
#define PCS_RS_FEC_OFFSET         0x00005000
#define PCS_LPCS_OFFSET           0x00006000
#define LPCS_STEP_PORT_OFFSET     0x00000080

#define PCS_STEP_PORT_OFFSET      0x00001000
/********** end  : PCS for 100G *********/

/********** start : PCS for usx *********/
#define USX_PCS_PORTS_OFFSET      0x00001000
#define USX_PCS_RS_FEC_OFFSET     0x00003000
#define USX_PCS_LPCS_OFFSET       0x00000000
#define USX_PCS_USXM_OFFSET       0x00002000

#define USX_PCS_SERDES_OFFSET     0x00003000 /* for RS_FEC,RS_FEC_STAT,USXM */
#define USX_PCS_STEP_PORT_OFFSET  0x00000100
#define USX_LPCS_STEP_PORT_OFFSET  0x00000080
/********** end  : PCS for usx *********/


#define CPU_0_MAC_MTIP_MAC_BASE_ADDR      0xA5000000
#define CPU_0_MAC_MTIP_MAC_PCS_BASE_ADDR  0xA1000000

#define CPU_1_MAC_MTIP_MAC_BASE_ADDR      0xA5200000
#define CPU_1_MAC_MTIP_MAC_PCS_BASE_ADDR  0xA1400000

#define CPU_MAC_EXT_BASE_OFFSET       0x00000000
#define CPU_MAC_PORT_OFFSET           0x00002000
#define CPU_MAC_MIB_OFFSET            0x00001000

#define CPU_PCS_PORT_OFFSET           0x00000000
#define CPU_PCS_RS_FEC_OFFSET         0x00001000
#define CPU_PCS_LPCS_OFFSET           0x00002000
#define PHOENIX_PSI_SCHED_OFFSET         (GT_U32)0x00000300

#define MIF_GLOBAL_OFFSET             0x00000000
#define MIF_RX_OFFSET                 0x00000800
#define MIF_TX_OFFSET                 0x00001000

#define ANP_OFFSET                    0x00000000
#define ANP_STEP_PORT_OFFSET          0x000002a4
#define AN_OFFSET                     0x00004000
#define ANP_STEP_UNIT_OFFSET          0x00200000
#define AN_STEP_PORT_OFFSET           0x00001000


static void smemPhoenixActiveWriteMacSecCountersControl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);

/*
NOTE: addresses of Hawk are used as temporary 'fast' solution
*/
#define UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName)                                , size , 0}

/* the units of PHOENIX */
static SMEM_GEN_UNIT_INFO_STC phoenix_units[] =
{

    /* packet buffer subunits */
     UNIT_INFO_MAC(0x94000000, UNIT_PB_CENTER_BLK              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x94200000, UNIT_PB_COUNTER_BLK             ,64  *     _1K)

    ,UNIT_INFO_MAC(0x93400000, UNIT_PB_WRITE_ARBITER_0         ,64  *     _1K)
    ,UNIT_INFO_MAC(0x93800000, UNIT_PB_GPC_GRP_PACKET_WRITE_0  ,64  *     _1K)
    ,UNIT_INFO_MAC(0x93C00000, UNIT_PB_GPC_GRP_CELL_READ_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x93000000, UNIT_PB_GPC_GRP_PACKET_READ_0   ,64  *     _1K)
    ,UNIT_INFO_MAC(0x92C00000, UNIT_PB_NEXT_POINTER_MEMO_0     ,64  *     _1K)
    ,UNIT_INFO_MAC(0x92800000, UNIT_PB_SHARED_MEMO_BUF_0       ,64  *     _1K)
    /* TXQ subunits */
    ,UNIT_INFO_MAC(0x96000000,UNIT_TXQ_PDX            ,64 *     _1K)
    ,UNIT_INFO_MAC(0x96400000,UNIT_TXQ_PFCC           ,64    * _1K)
    ,UNIT_INFO_MAC(0x96800000,UNIT_TXQ_PSI            , 1  * _1M)

    ,UNIT_INFO_MAC(0x95800000,UNIT_TXQ_SDQ0  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x95400000,UNIT_TXQ_PDS0  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x95C00000,UNIT_TXQ_QFC0  ,64   *    _1K)

    /* DP units */
    ,UNIT_INFO_MAC(0x91C00000,UNIT_RX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x91800000,UNIT_IA                ,64  *     _1K)
    ,UNIT_INFO_MAC(0x92000000,UNIT_TX_DMA            ,64  *     _1K)
    ,UNIT_INFO_MAC(0x92400000,UNIT_TX_FIFO           ,64  *     _1K)

    /* PCA ubits */
    ,UNIT_INFO_MAC(0x9B000000,UNIT_PCA_BRG_0              ,16  * _1K )
    ,UNIT_INFO_MAC(0x98800000,UNIT_PCA_PZ_ARBITER_I_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x9B400000,UNIT_PCA_PZ_ARBITER_E_0     ,16  * _1K )
    ,UNIT_INFO_MAC(0x99400000,UNIT_PCA_SFF_0              ,16  * _1K )
    ,UNIT_INFO_MAC(LMU_0_BASE_ADDR, UNIT_PCA_LMU_0        ,64  * _1K )
    ,UNIT_INFO_MAC(TSU_0_BASE_ADDR, UNIT_PCA_CTSU_0       ,64  * _1K )


    ,UNIT_INFO_MAC(0x99C00000,UNIT_PCA_MACSEC_EXT_E_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x99000000,UNIT_PCA_MACSEC_EXT_I_163_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x9AC00000,UNIT_PCA_MACSEC_EXT_E_164_0 ,16  * _1K )
    ,UNIT_INFO_MAC(0x98400000,UNIT_PCA_MACSEC_EXT_I_164_0 ,16  * _1K )

    ,UNIT_INFO_MAC(0x98C00000,UNIT_PCA_MACSEC_EIP_163_I_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x99800000,UNIT_PCA_MACSEC_EIP_163_E_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x98000000,UNIT_PCA_MACSEC_EIP_164_I_0 ,1  * _1M )
    ,UNIT_INFO_MAC(0x9A800000,UNIT_PCA_MACSEC_EIP_164_E_0 ,1  * _1M )

    /* DFX */
    ,UNIT_INFO_MAC(0x94400000, UNIT_DFX_SERVER       ,1  * _1M)


    ,UNIT_INFO_MAC(0x84000000,UNIT_TTI               ,4   * _1M)

    /* Sip 6.10 PPU */
    ,UNIT_INFO_MAC(0x84800000,UNIT_PPU               ,1   * _1M)

    ,UNIT_INFO_MAC(0x8C000000,UNIT_PHA               ,8   * _1M)
    ,UNIT_INFO_MAC(0x8F400000,UNIT_EPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x8F800000,UNIT_EOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x8EC00000,UNIT_EPCL              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x8FC00000,UNIT_PREQ              ,512 *     _1K)
    ,UNIT_INFO_MAC(0x8E800000,UNIT_ERMRK             ,4   * _1M)
    ,UNIT_INFO_MAC(0x8D000000,UNIT_EREP              ,64  *     _1K)
    ,UNIT_INFO_MAC(0x8CE00000,UNIT_BMA               ,1   * _1M)
    ,UNIT_INFO_MAC(0x8CC00000,UNIT_HBU               ,64  *     _1K)
    ,UNIT_INFO_MAC(0x90000000,UNIT_HA                ,6   * _1M)/* in map 4M, in Cider - used more */

    ,UNIT_INFO_MAC(0x8E000000,UNIT_TCAM              ,8   * _1M)
    ,UNIT_INFO_MAC(0x8D400000,UNIT_EM                ,4   * _1M)
    ,UNIT_INFO_MAC(0x8D800000,UNIT_FDB               ,64  *     _1K)

    ,UNIT_INFO_MAC(0x84A00000,UNIT_IPCL              ,512 *     _1K)

    ,UNIT_INFO_MAC(0x84C00000,UNIT_CNC               ,256 *     _1K)

    ,UNIT_INFO_MAC(0x8B800000,UNIT_EGF_QAG           ,1   * _1M)
    ,UNIT_INFO_MAC(0x8BC00000,UNIT_EGF_SHT           ,2   * _1M)
    ,UNIT_INFO_MAC(0x8B400000,UNIT_EGF_EFT           ,64  *     _1K)

    ,UNIT_INFO_MAC(0x88000000,UNIT_IPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x88400000,UNIT_IPLR1             ,4   * _1M)

    ,UNIT_INFO_MAC(0x89000000,UNIT_EQ                ,16  * _1M)

    ,UNIT_INFO_MAC(0x88800000,UNIT_IOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x88C00000,UNIT_MLL               ,2   * _1M)

    ,UNIT_INFO_MAC(0x85800000,UNIT_IPVX              ,8   * _1M)
    ,UNIT_INFO_MAC(0x87000000,UNIT_LPM               ,16  * _1M)
    ,UNIT_INFO_MAC(0x86000000,UNIT_L2I               ,8   * _1M)

    ,UNIT_INFO_MAC(0x9E400000,UNIT_LED_USX2_0_LED    ,4 * _1K)
    ,UNIT_INFO_MAC(0x9F400000,UNIT_LED_USX2_1_LED    ,4 * _1K)
    ,UNIT_INFO_MAC(0xA6000000,UNIT_LED_MAC_CPU_LED0  ,4 * _1K)
    ,UNIT_INFO_MAC(0xA6400000,UNIT_LED_MAC_CPU_LED1  ,4 * _1K)
    ,UNIT_INFO_MAC(0xA8800000,UNIT_LED_MAC_100G_0    ,4 * _1K)


    /*NOTE: MG0 is part of the CnM : size 1M */
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG    /*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_0_1/*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 2*MG_SIZE ,UNIT_MG_0_2/*CNM*/, MG_SIZE)

    /* Runit is in the CnM section */
    ,UNIT_INFO_MAC(0x7F010000 ,UNIT_CNM_RUNIT    ,64   *     _1K) /*in CnM */

       /* SRAM  is in the CnM section */
    ,UNIT_INFO_MAC(0x7FF40000,UNIT_CNM_SRAM   ,192   *     _1K)/*in CnM */

    /* RFU is in the CnM section */
    ,UNIT_INFO_MAC(0x80010000 ,UNIT_CNM_RFU    ,64   *     _1K) /*in CnM */

    /* SMI is in the CnM section */
    ,UNIT_INFO_MAC(0x80580000 ,UNIT_GOP_SMI_0  ,64   *     _1K) /*in CnM */
    ,UNIT_INFO_MAC(0x80590000 ,UNIT_GOP_SMI_1  ,64   *     _1K) /*in CnM */
    /* MPP_RFU is in the CnM section */
    ,UNIT_INFO_MAC(0x80020000 ,UNIT_CNM_MPP_RFU,64   *     _1K) /*in CnM */
     /* AAC is in the CnM section */
    ,UNIT_INFO_MAC(0x80030000 ,UNIT_CNM_AAC    ,64   *     _1K) /*in CnM */

     /* Addr decoder units in the CnM section */
    ,UNIT_INFO_MAC(0x80400000 ,UNIT_CNM_ADDR_DECODER   /*CPU            */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80410000 ,UNIT_CNM_ADDR_DECODER_1 /*CoreSight Trace*/ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80420000 ,UNIT_CNM_ADDR_DECODER_2 /*AMB2           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80430000 ,UNIT_CNM_ADDR_DECODER_3 /*AMB3           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80440000 ,UNIT_CNM_ADDR_DECODER_4 /*iNIC           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80450000 ,UNIT_CNM_ADDR_DECODER_5 /*XOR0           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80460000 ,UNIT_CNM_ADDR_DECODER_6 /*XOR1           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80470000 ,UNIT_CNM_ADDR_DECODER_7 /*PCIe           */ ,64   *     _1K)/*in CnM */
    ,UNIT_INFO_MAC(0x80480000 ,UNIT_CNM_ADDR_DECODER_8 /*GIC            */ ,64   *     _1K)/*in CnM */

     /* PEX MAC is in the CnM section */
    ,UNIT_INFO_MAC(0x800A0000 ,UNIT_CNM_PEX_MAC   ,64   *     _1K)/*in CnM */

    /* LMU - moved into PCA */
    /* TSU (PTP) - moved into PCA */
    ,UNIT_INFO_MAC(MAC_MTIP_MAC_0_BASE_ADDR,UNIT_MAC_400G_0              ,64   *     _1K)

    ,UNIT_INFO_MAC(MAC_MTIP_MAC_PCS_0_BASE_ADDR,UNIT_PCS_400G_0              ,64   *     _1K)

    ,UNIT_INFO_MAC(CPU_0_MAC_MTIP_MAC_BASE_ADDR    ,UNIT_MAC_CPU              ,16   *     _1K)
    ,UNIT_INFO_MAC(CPU_0_MAC_MTIP_MAC_PCS_BASE_ADDR,UNIT_PCS_CPU              ,16   *     _1K)

    ,UNIT_INFO_MAC(CPU_1_MAC_MTIP_MAC_BASE_ADDR    ,UNIT_MAC_CPU_1              ,16   *     _1K)
    ,UNIT_INFO_MAC(CPU_1_MAC_MTIP_MAC_PCS_BASE_ADDR,UNIT_PCS_CPU_1              ,16   *     _1K)

    ,UNIT_INFO_MAC(0xA8C00000     ,UNIT_SERDES_SDW0        ,512   *     _1K     )       /* 28G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0xA9000000     ,UNIT_SERDES_SDW5        ,512   *     _1K     )       /* 28G Quad SD Wrapper */
    ,UNIT_INFO_MAC(0xA9400000     ,UNIT_SERDES_SDW4_1      ,512   *     _1K     )       /* 28G Single SD Wrapper */
    ,UNIT_INFO_MAC(0xA9500000     ,UNIT_SERDES_SDW4_2      ,512   *     _1K     )       /* 28G Single SD Wrapper */
    ,UNIT_INFO_MAC(0xA9600000     ,UNIT_SERDES_SDW4_3      ,512   *     _1K     )       /* 28G Single SD Wrapper */
    ,UNIT_INFO_MAC(0xA9700000     ,UNIT_SERDES_SDW4_4      ,512   *     _1K     )       /* 28G Single SD Wrapper */

    /*MIF */
    ,UNIT_INFO_MAC(MIF_0_BASE_ADDR, UNIT_MIF_0              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_1_BASE_ADDR, UNIT_MIF_1              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_2_BASE_ADDR, UNIT_MIF_2              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_3_BASE_ADDR, UNIT_MIF_3              ,16   *     _1K)
    ,UNIT_INFO_MAC(MIF_4_BASE_ADDR, UNIT_MIF_4              ,16   *     _1K)


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

    ,UNIT_INFO_MAC(0x84400000,  UNIT_TTI_TAI0_E               ,64  * _1K)
    ,UNIT_INFO_MAC(0x84600000,  UNIT_TTI_TAI1_E               ,64  * _1K)
    ,UNIT_INFO_MAC(0x85000000,  UNIT_TAI                      ,64  * _1K)
    ,UNIT_INFO_MAC(0x85400000,  UNIT_TAI_1                    ,64  * _1K)
    ,UNIT_INFO_MAC(0x8C800000,  UNIT_PHA_TAI0_E               ,64  * _1K)
    ,UNIT_INFO_MAC(0x8CA00000,  UNIT_PHA_TAI1_E               ,64  * _1K)
    ,UNIT_INFO_MAC(0x8F000000,  UNIT_EPCL_HA_TAI0_E           ,64  * _1K)
    ,UNIT_INFO_MAC(0x8F200000,  UNIT_EPCL_HA_TAI1_E           ,64  * _1K)
    ,UNIT_INFO_MAC(0x96C00000,  UNIT_TXQS_TAI0_E              ,64  * _1K)
    ,UNIT_INFO_MAC(0x9DC00000,  UNIT_USX2_0_TAI0_E            ,64  * _1K)
    ,UNIT_INFO_MAC(0x9DE00000,  UNIT_USX2_0_TAI1_E            ,64  * _1K)
    ,UNIT_INFO_MAC(0x9EC00000,  UNIT_USX2_1_TAI0_E            ,64  * _1K)
    ,UNIT_INFO_MAC(0x9EE00000,  UNIT_USX2_1_TAI1_E            ,64  * _1K)
    ,UNIT_INFO_MAC(0x9A400000,  UNIT_PCA0_TAI0_E              ,64  * _1K)
    ,UNIT_INFO_MAC(0x9A600000,  UNIT_PCA0_TAI1_E              ,64  * _1K)
    ,UNIT_INFO_MAC(0xA5400000,  UNIT_MAC_CPUM_TAI0_E          ,64  * _1K)
    ,UNIT_INFO_MAC(0xA5500000,  UNIT_MAC_CPUM_TAI1_E          ,64  * _1K)
    ,UNIT_INFO_MAC(0xA5600000,  UNIT_MAC_CPUC_TAI0_E          ,64  * _1K)
    ,UNIT_INFO_MAC(0xA5700000,  UNIT_MAC_CPUC_TAI1_E          ,64  * _1K)
    ,UNIT_INFO_MAC(0xA8000000,  UNIT_100G_TAI0_E              ,64  * _1K)
    ,UNIT_INFO_MAC(0xA8200000,  UNIT_100G_TAI1_E              ,64  * _1K)

    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

enum{
    sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr = 0,/*0..0*/
    sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr  = 1, /*1..6*/
    sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix___last      = 7
};

/* info about the 50/100/200/400G MACs */
static MAC_NUM_INFO_STC phoenix100GPortsArr[] = {
    /*global*/      /*mac*/   /* channelInfo */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {50 ,           {0,  0} ,    {0, 50}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr    }
    ,{51 ,           {0,  1} ,    {0, 51}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr    }
    ,{52 ,           {0,  2} ,    {0, 52}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr    }
    ,{53 ,           {0,  3} ,    {0, 53}       ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* info about the 1/2.5/5G MACs */
static MAC_NUM_INFO_STC phoenixUsxPortsArr[] = {
    /*global*/      /*mac*/   /* tsu */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {0  ,           {0, 0} ,    {0,  0}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{1  ,           {0, 1} ,    {0,  1}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{2  ,           {0, 2} ,    {0,  2}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{3  ,           {0, 3} ,    {0,  3}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{4  ,           {0, 4} ,    {0,  4}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{5  ,           {0, 5} ,    {0,  5}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{6  ,           {0, 6} ,    {0,  6}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{7  ,           {0, 7} ,    {0,  7}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{ 8 ,           {1, 0} ,    {0,  8}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{ 9 ,           {1, 1} ,    {0,  9}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{10 ,           {1, 2} ,    {0, 10}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{11 ,           {1, 3} ,    {0, 11}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{12 ,           {1, 4} ,    {0, 12}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{13 ,           {1, 5} ,    {0, 13}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{14 ,           {1, 6} ,    {0, 14}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{15 ,           {1, 7} ,    {0, 15}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{16 ,           {2, 0} ,    {0, 16}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{17 ,           {2, 1} ,    {0, 17}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{18 ,           {2, 2} ,    {0, 18}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{19 ,           {2, 3} ,    {0, 19}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{20 ,           {2, 4} ,    {0, 20}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{21 ,           {2, 5} ,    {0, 21}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{22 ,           {2, 6} ,    {0, 22}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{23 ,           {2, 7} ,    {0, 23}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{24 ,           {3, 0} ,    {0, 24}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{25 ,           {3, 1} ,    {0, 25}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{26 ,           {3, 2} ,    {0, 26}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{27 ,           {3, 3} ,    {0, 27}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{28 ,           {3, 4} ,    {0, 28}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{29 ,           {3, 5} ,    {0, 29}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{30 ,           {3, 6} ,    {0, 30}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{31 ,           {3, 7} ,    {0, 31}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{32 ,           {4, 0} ,    {0, 32}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{33 ,           {4, 1} ,    {0, 33}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{34 ,           {4, 2} ,    {0, 34}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{35 ,           {4, 3} ,    {0, 35}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{36 ,           {4, 4} ,    {0, 36}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{37 ,           {4, 5} ,    {0, 37}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{38 ,           {4, 6} ,    {0, 38}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{39 ,           {4, 7} ,    {0, 39}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{40 ,           {5, 0} ,    {0, 40}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{41 ,           {5, 1} ,    {0, 41}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{42 ,           {5, 2} ,    {0, 42}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{43 ,           {5, 3} ,    {0, 43}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{44 ,           {5, 4} ,    {0, 44}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{45 ,           {5, 5} ,    {0, 45}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{46 ,           {5, 6} ,    {0, 46}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }
    ,{47 ,           {5, 7} ,    {0, 47}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

#define CPU_PORT_DP_0     0

/* info about the CPU MACs */
/* NOTE : those 2 ports are not 'CPU' related and are not muxed to SDMA CPU ports */
static MAC_NUM_INFO_STC phoenixCpuPortsArr[] = {
    /*global*/       /*mac*/       /* DP unit */               /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {48,           {0, 0} ,    {CPU_PORT_DP_0, 48}      , 0    }
    ,{49,           {1, 0} ,    {CPU_PORT_DP_0, 49}      , 1    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* number of units in the device */
#define PHOENIX_NUM_UNITS sizeof(phoenix_units)/sizeof(phoenix_units[0])

static SMEM_GEN_UNIT_INFO_STC SORTED___phoenix_units[PHOENIX_NUM_UNITS] =
{
    /* sorted and build during smemPhoenixInit(...) from phoenix_units[] */
    {SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};


/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from falcon_units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC phoenix_duplicatedUnits[] =
{
    /* those explicitly listed here need unit allocation as are not duplicated within each pipe */
   {STR(UNIT_MG)        ,2}, /* 2 duplication of this unit */
        {STR(UNIT_MG_0_1)},
        {STR(UNIT_MG_0_2)},


    {STR(UNIT_MIF_0)  ,4},  /* 4 more per device */
        {STR(UNIT_MIF_1)},
        {STR(UNIT_MIF_2)},
        {STR(UNIT_MIF_3)},
        {STR(UNIT_MIF_4)},

    {STR(UNIT_MAC_CPU)  ,1},  /* 1 duplication of this unit  */
        {STR(UNIT_MAC_CPU_1)},

    {STR(UNIT_PCS_CPU)  ,1},  /* 1 duplication of this unit  */
        {STR(UNIT_PCS_CPU_1)},

        {STR(UNIT_ANP_0)  ,8},  /* 8 more per device */
            {STR(UNIT_ANP_1)},
            {STR(UNIT_ANP_2)},
            {STR(UNIT_ANP_3)},
            {STR(UNIT_ANP_4)},
            {STR(UNIT_ANP_5)},
            {STR(UNIT_ANP_6)},
            {STR(UNIT_ANP_7)},
            {STR(UNIT_ANP_8)},

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

    {STR(UNIT_PCA_PZ_ARBITER_I_0)  ,1},  /* 1 more per device */
        {STR(UNIT_PCA_PZ_ARBITER_E_0)},

    {STR(UNIT_PCA_MACSEC_EXT_I_163_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_MACSEC_EXT_E_163_0)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_0)},

    {STR(UNIT_LED_USX2_0_LED)  ,4},  /* 4 more per device */
        {STR(UNIT_LED_USX2_1_LED)},
        {STR(UNIT_LED_MAC_CPU_LED0)},
        {STR(UNIT_LED_MAC_CPU_LED1)},
        {STR(UNIT_LED_MAC_100G_0)},

    {STR(UNIT_SERDES_SDW0)       ,1},  /* 4 more per device */
        {STR(UNIT_SERDES_SDW5  )},

    {STR(UNIT_SERDES_SDW4_1),     3},
        {STR(UNIT_SERDES_SDW4_2)},
        {STR(UNIT_SERDES_SDW4_3)},
        {STR(UNIT_SERDES_SDW4_4)},

    {STR(UNIT_CNM_ADDR_DECODER)   ,8},  /* 8 more per device */
        {STR(UNIT_CNM_ADDR_DECODER_1)},
        {STR(UNIT_CNM_ADDR_DECODER_2)},
        {STR(UNIT_CNM_ADDR_DECODER_3)},
        {STR(UNIT_CNM_ADDR_DECODER_4)},
        {STR(UNIT_CNM_ADDR_DECODER_5)},
        {STR(UNIT_CNM_ADDR_DECODER_6)},
        {STR(UNIT_CNM_ADDR_DECODER_7)},
        {STR(UNIT_CNM_ADDR_DECODER_8)},

    {STR(UNIT_TAI)   ,20},  /* 20 more per device */
        {STR(UNIT_TXQS_TAI0_E)     },
        {STR(UNIT_TTI_TAI0_E)      },
        {STR(UNIT_TTI_TAI1_E)      },
        {STR(UNIT_PHA_TAI0_E)      },
        {STR(UNIT_PHA_TAI1_E)      },
        {STR(UNIT_EPCL_HA_TAI0_E)  },
        {STR(UNIT_EPCL_HA_TAI1_E)  },
        {STR(UNIT_USX2_0_TAI0_E)   },
        {STR(UNIT_USX2_0_TAI1_E)   },
        {STR(UNIT_USX2_1_TAI0_E)   },
        {STR(UNIT_USX2_1_TAI1_E)   },
        {STR(UNIT_PCA0_TAI0_E)     },
        {STR(UNIT_PCA0_TAI1_E)     },
        {STR(UNIT_TAI_1)           },
        {STR(UNIT_MAC_CPUM_TAI0_E) },
        {STR(UNIT_MAC_CPUM_TAI1_E) },
        {STR(UNIT_MAC_CPUC_TAI0_E) },
        {STR(UNIT_MAC_CPUC_TAI1_E) },
        {STR(UNIT_100G_TAI0_E)     },
        {STR(UNIT_100G_TAI1_E)     },

        {NULL,0} /* must be last */
};

/* the DP that hold MG0_0*/
#define CNM0_DP_FOR_MG_0   0

#define FIRST_CPU_SDMA    54 /* DP ports */

/* DMA : special ports mapping {global,local,DP}    */
static SPECIAL_PORT_MAPPING_CNS phoenix_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
/*54*/ {FIRST_CPU_SDMA+0/*global DMA port*/,FIRST_CPU_SDMA+0/*local DMA port*/,0/*DP[0]*/}
/*55*/,{FIRST_CPU_SDMA+1/*global DMA port*/,FIRST_CPU_SDMA+1/*local DMA port*/,0/*DP[0]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* multi-CPU SDMA MG : special ports mapping {global,NA,mgUnitIndex}    */
static SPECIAL_PORT_MAPPING_CNS phoenix_cpuPortSdma_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --NA;OUT       --mgUnitIndex;OUT */
/*54*/ {FIRST_CPU_SDMA+0/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,0/*MG[0]*/}
/*55*/,{FIRST_CPU_SDMA+1/*global DMA port*/,SMAIN_NOT_VALID_CNS/*NA*/,1/*MG[1]*/ }

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/**
* @internal smemPhoenixGetMgUnitIndexFromAddress function
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
static GT_U32  smemPhoenixGetMgUnitIndexFromAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  regAddress
)
{
    if(regAddress >= MG_0_0_OFFSET_CNS &&
       regAddress < (MG_0_0_OFFSET_CNS + (MG_SIZE*PHOENIX_NUM_MG_UNITS)))
    {
        return (regAddress - MG_0_0_OFFSET_CNS)/MG_SIZE;
    }

    return SMAIN_NOT_VALID_CNS;
}

/**
* @internal smemPhoenixGopPortByAddrGet function
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
static GT_U32 smemPhoenixGopPortByAddrGet
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

    for(ii = 0; ii < SIM_MAX_ANP_UNITS; ii++)
    {
        if(address >=  devObjPtr->memUnitBaseAddrInfo.anp[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.anp[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.anp[ii] + ANP_OFFSET;
            if(address >= unitBase + 0x00000100 &&
               address < (unitBase + 0x00000104 + 0x4*0x2a4))
            {
                /* 0x00000094 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Cause */
                /* 0x00000098 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[anpNum].ANP.Interrupt_Summary_Mask */
                localPortInUnit = (address - (unitBase + 0x00000100)) / 0x2a4;
                break;
            }
        }
    }

    unitIndex = ii;

    if(ii < 6)
    {
        /* matched as address in USX_ANP  */
        return ii*8;
    }
    else if(ii < 9)
    {
        /* matched ANP_CPU */
        return (48 + (ii- 7));
    }
    else if(ii < 10)
    {
        /* matched ANP_CPU */
        return (50+localPortInUnit);
    }
    else if(ii < SIM_MAX_ANP_UNITS)
    {
        skernelFatalError("smemPhoenixGopPortByAddrGet : in ANP: unknown port for address [0x%8.8x] as 'Gop unit' \n",
            address);
    }
    else
    {
        /* match not found - continue */
    }
    /**********************/
    /* End of ANP section */
    /**********************/


    for(ii = sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr ;
        ii < sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr;
        ii++)
    {
        /* 100G mac + usx macs */
        if(address >=  devObjPtr->memUnitBaseAddrInfo.macWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_MIB_OFFSET;

            if((address >= unitBase) && (address < (unitBase + 0x1000)))
            {
                /* mib unit */
                /* calculate the local port in the unit */
                if(address >= (unitBase + 0x00000100) &&
                   address <  (unitBase + 0x00000660))
                {
                    /* rx registers */
                    localPortInUnit = (address - unitBase) / (43 * 0x4);
                    break;
                }

                if(address >= (unitBase + 0x00000660))
                {
                    /* tx registers */
                    localPortInUnit = (address - unitBase) / (34 * 0x4);
                    break;
                }

                break;/* common to the unit */
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + PORT0_100G_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (4*MAC_STEP_PORT_OFFSET)))
            {
                /* 100G/50G port */
                localPortInUnit = (address - unitBase) / MAC_STEP_PORT_OFFSET;
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_EXT_BASE_OFFSET;
            if(address >= unitBase + 0x00000094 &&
               address < (unitBase + 0x00000098 + (4 * 0x18)))
            {
                /* 0x00000094 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
                /* 0x00000098 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
                localPortInUnit = (address - (unitBase + 0x00000094)) / 0x18;
                break;
            }

            break; /* common to the unit */
        }

        if(address >=  devObjPtr->memUnitBaseAddrInfo.macPcs[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_PORT0_50G_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (4*PCS_STEP_PORT_OFFSET)))
            {
                /* 50G/25G port */
                localPortInUnit = (address - unitBase) / PCS_STEP_PORT_OFFSET;
                break;
            }
            unitBase = devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_LPCS_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (4*LPCS_STEP_PORT_OFFSET)))
            {
                /* lpcs port */
                localPortInUnit = (address - unitBase) / LPCS_STEP_PORT_OFFSET;
                break;
            }
            break;
        }
    }

    unitIndex = ii;

    if(ii < sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr)
    {
        /* matched as address in MAC100G  */

        for(ii = 0 ; phoenix100GPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
        {
            if(phoenix100GPortsArr[ii].macInfo.unitIndex   == unitIndex &&
               phoenix100GPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
            {
                return phoenix100GPortsArr[ii].globalMacNum;
            }
        }
        skernelFatalError("smemPhoenixGopPortByAddrGet : in 100G mac : unknown port for address [0x%8.8x] as 'Gop unit' \n",
            address);
    }

    /* not matched yet */
    for(ii = 0; ii < 2; ii++)
    {
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

    unitIndex = ii;
    if(isCpuPort)
    {
        /* matched as address in CPU-MAC  */

        for(ii = 0 ; phoenixCpuPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
        {
            if(phoenixCpuPortsArr[ii].macInfo.unitIndex == unitIndex)
            {
                /* NOTE: will always be matched on ii=0 and never on ii=1 */
                return phoenixCpuPortsArr[ii].globalMacNum;
            }
        }

        skernelFatalError("smemPhoenixGopPortByAddrGet : in CPU mac : unknown cpu port for address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }

    /* not matched yet */

    for(ii = sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr ;
        ii < sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix___last;
        ii++)
    {
        /* 100G mac + usx macs */
        if(address >=  devObjPtr->memUnitBaseAddrInfo.macWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + _1M) )
        {
            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_MIB_OFFSET;

            if((address >= unitBase) && (address < (unitBase + 0x1000)))
            {
                /* mib unit */
                /* calculate the local port in the unit */
                if(address >= (unitBase + 0x00000100) &&
                   address <  (unitBase + 0x00000660))
                {
                    /* rx registers */
                    localPortInUnit = (address - unitBase) / (43 * 0x4);
                    break;
                }

                if(address >= (unitBase + 0x00000660))
                {
                    /* tx registers */
                    localPortInUnit = (address - unitBase) / (34 * 0x4);
                    break;
                }

                break;/* common to the unit */
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + PORT0_100G_OFFSET;
            if(address >= unitBase &&
               address < (unitBase + (NUM_PORTS_PER_USX_UNIT*MAC_STEP_PORT_OFFSET)))
            {
                /* USX port */
                localPortInUnit = (address - unitBase) / MAC_STEP_PORT_OFFSET;
                break;
            }

            unitBase = devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_EXT_BASE_OFFSET;
            if(address >= unitBase + 0x0000002c &&
               address < (unitBase + 0x00000050 + (NUM_PORTS_PER_USX_UNIT * 0x28)))
            {
                /* 0x00000038 : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
                /* 0x0000003c : SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
                localPortInUnit = (address - (unitBase + 0x0000002c)) / 0x28;
                break;
            }

            if(address >= unitBase + 0x00000170 &&
               address < (unitBase + 0x0000017c + (NUM_PORTS_PER_USX_UNIT * 0x10)))
            {
                localPortInUnit = (address - (unitBase + 0x0000017c)) / 0x10;
                break;
            }

            if(address >= unitBase + 0x000001f0 &&
               address < (unitBase + 0x00000200 + (NUM_PORTS_PER_USX_UNIT/4 * 0x14)))
            {
                localPortInUnit = ((address - (unitBase + 0x0000017c)) / 0x14) * 4;
                break;
            }

            if(address >= unitBase + 0x00000218 &&
               address < (unitBase + 0x0000021c + (NUM_PORTS_PER_USX_UNIT/4 * 0x14)))
            {
                localPortInUnit = ((address - (unitBase + 0x00000218)) / 0x14) * 4;
                break;
            }

            break; /* common to the unit */
        }

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

    if(ii == sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix___last)
    {
        skernelFatalError("smemPhoenixGopPortByAddrGet : unknown address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }

    /* matched as address in USX-MAC  */

    unitIndex = ii - sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr;

    for(ii = 0 ; phoenixUsxPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
    {
        if(phoenixUsxPortsArr[ii].macInfo.unitIndex   == unitIndex &&
           phoenixUsxPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
        {
            return phoenixUsxPortsArr[ii].globalMacNum;
        }
    }

    skernelFatalError("smemPhoenixGopPortByAddrGet : in USX mac :  unknown port for address [0x%8.8x] as 'Gop unit' \n",
        address);
    return 0;
}

/**
* @internal smemPhoenixMibPortByAddrGet function
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
static GT_U32 smemPhoenixMibPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    return smemPhoenixGopPortByAddrGet(devObjPtr,address);
}

/*******************************************************************************
*   smemPhoenixPortInfoGet
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
static GT_STATUS smemPhoenixPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SMEM_UNIT_TYPE_ENT      unitType,
    IN GT_U32                  portNum,
    OUT ENHANCED_PORT_INFO_STC *portInfoPtr
)
{
    GT_U32  ii,jj,jjMax = 1;
    MAC_NUM_INFO_STC   *tmpMacInfoPtr;
    GT_U32      sip6_MTI_bmpPorts[2];
    GT_U32      specific_sip6_MTI_EXTERNAL_representativePortIndex;


    /*special case*/
    if(unitType == SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E)
    {
        GT_U32  mifUnitId = portNum;/* !!! the parameter used as mifUnitId !!! */
        GT_U32  dpUnitIndex = 0;/*single DP */
        GT_U32  IN_indexInUnit = portInfoPtr->simplePortInfo.indexInUnit;/* !!! the parameter used as input !!! */
        GT_U32  OUT_indexInUnit=0;

        switch(mifUnitId)
        {
            case 0:/*USX_MIF_0*/ OUT_indexInUnit=0*16 + IN_indexInUnit;break;/*  0..15 */
            case 1:/*USX_MIF_1*/ OUT_indexInUnit=1*16 + IN_indexInUnit;break;/* 16..31 */
            case 2:/*USX_MIF_2*/ OUT_indexInUnit=2*16 + IN_indexInUnit;break;/* 32..47 */
            case 3:/*CPU_MIF_3*/ OUT_indexInUnit=  48 + IN_indexInUnit;break;/* 48..49 */
            case 4:/*100G_MIF_4*/OUT_indexInUnit=  50 + IN_indexInUnit;break;/* 50..53 */
            default:
                skernelFatalError("smemPhoenixPortInfoGet : unknown mifUnitId [%d] \n",mifUnitId);
                break;
        }

        portInfoPtr->simplePortInfo.unitIndex   = dpUnitIndex;
        portInfoPtr->simplePortInfo.indexInUnit = OUT_indexInUnit;/* the converted Cider index of port in DP to 'local MAC' */
        portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = 0;/*not used*/
        portInfoPtr->sip6_MTI_bmpPorts[0] = 0;/*not used*/
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
            sip6_MTI_bmpPorts[0] = 0;
            sip6_MTI_bmpPorts[1] = 0;
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
            /* GOP 1 */
            if((portNum >= 50) && (portNum <= 53))
            {
                sip6_MTI_bmpPorts[0] = 0x0000000f;/* ports 50..53 (start from '50 as bit 0') */
                sip6_MTI_bmpPorts[1] = 0x00000000;
            }
            else
            {
                return GT_NOT_FOUND;
            }
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
            if(portNum < 48)
            {
                sip6_MTI_bmpPorts[0] = 0x000000ff;/* 8 consecutive ports in the group */
                sip6_MTI_bmpPorts[1] = 0;
            }
            else
            {
                return GT_NOT_FOUND;
            }
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_CPU_E:
            if((portNum == 48) || (portNum == 49))
            {
                sip6_MTI_bmpPorts[0] = 0x1;/*single port in group*/
                sip6_MTI_bmpPorts[1] = 0;
            }
            else
            {
                return GT_NOT_FOUND;
            }
            break;
        default:
            skernelFatalError("smemPhoenixPortInfoGet : unknown type [%d] \n",unitType);
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
                    (jj == 0) ? phoenix100GPortsArr :
                    (jj == 1) ? phoenixUsxPortsArr  :
                                phoenixCpuPortsArr  ;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_50G_E:
                tmpMacInfoPtr = phoenix100GPortsArr;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
                tmpMacInfoPtr = phoenixUsxPortsArr;
                break;
            case SMEM_UNIT_TYPE_MTI_MAC_CPU_E:
                tmpMacInfoPtr = phoenixCpuPortsArr;
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
                    /* Phoenix use channel 0 in LMU for all channels */
                    portInfoPtr->simplePortInfo.indexInUnit = 0;
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
                    portInfoPtr->simplePortInfo.unitIndex   =
                        (jj == 0) /*100G*/? SMEM_CHT_PORT_MTI_MIF_TYPE_32_E :
                                            SMEM_CHT_PORT_MTI_MIF_TYPE_8_E;/* used as the 'value'*/
                    portInfoPtr->simplePortInfo.indexInUnit = 0;/* not used */
                    break;
                case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
                    /*
                    0x9E000000 + 0x7800000+(a-2)*0x0400000: where a (2-2) represents USX_MIF_2
                    0x9E000000 + 0xa400000*(a-3): where a (4-4) represents 100G_MIF_4
                    0x9E000000 + 0x7800000+(a-2)*0x0400000: where a (3-3) represents CPU_MIF_3
                    0x9E000000 + a*0x1000000: where a (0-1) represents USX_MIF_0_1
                    */
                    if(jj == 2)
                    {
                        /*The 2 CPUs connected to single MIF unit */
                        portInfoPtr->simplePortInfo.unitIndex   = 3; /*CPU_MIF_3*/
                        portInfoPtr->simplePortInfo.indexInUnit = ii;/* 0 or 1 */
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = 0;
                    }
                    else
                    if (jj == 1)
                    {
                        /*evey 2 USX units connected to single MIF unit */
                        portInfoPtr->simplePortInfo.unitIndex   = tmpMacInfoPtr->macInfo.unitIndex / 2;/*0..2*/
                        portInfoPtr->simplePortInfo.indexInUnit = /*0..15*/
                            8*(tmpMacInfoPtr->macInfo.unitIndex % 2) +/*0 or 8*/
                            tmpMacInfoPtr->macInfo.indexInUnit;/*0..7*/
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = portInfoPtr->simplePortInfo.unitIndex;/*0..2*/
                    }
                    else
                    {
                        /*100G mac */
                        portInfoPtr->simplePortInfo.unitIndex   = 4; /*100G_MIF_4*/
                        portInfoPtr->simplePortInfo.indexInUnit = ii;/* 0 .. 3 */
                        specific_sip6_MTI_EXTERNAL_representativePortIndex = portInfoPtr->simplePortInfo.unitIndex;/*4*/
                    }

                    /* set specific sip6_MTI_EXTERNAL_representativePort */
                    portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = specific_sip6_MTI_EXTERNAL_representativePortIndex;
                    portInfoPtr->sip6_MTI_bmpPorts[0] = sip6_MTI_bmpPorts[0];
                    portInfoPtr->sip6_MTI_bmpPorts[1] = sip6_MTI_bmpPorts[1];
                    return GT_OK;

                default:
                    portInfoPtr->simplePortInfo = tmpMacInfoPtr->macInfo;
                    break;
            }

            portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex =  tmpMacInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex;
            portInfoPtr->sip6_MTI_bmpPorts[0] = sip6_MTI_bmpPorts[0];
            portInfoPtr->sip6_MTI_bmpPorts[1] = sip6_MTI_bmpPorts[1];

            return GT_OK;
        }/* loop on ii*/
    } /* loop on jj*/


    return GT_NOT_FOUND;
}

/**
* @internal internalPhoenixUnitPipeOffsetGet function
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
static GT_U32 internalPhoenixUnitPipeOffsetGet
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

static SMEM_UNIT_NAME_AND_INDEX_STC phoenixUnitNameAndIndexArr[PHOENIX_NUM_UNITS]=
{
    /* filled in runtime from phoenix_units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};
/* the addresses of the units that the phoenix uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   phoenixUsedUnitsAddressesArray[PHOENIX_NUM_UNITS]=
{
    {0,0}    /* filled in runtime from phoenix_units[] */
};

/* build once the sorted memory for the falcon .. for better memory search performance

    that use by :

    devObjPtr->devMemUnitNameAndIndexPtr      = phoenixUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = phoenixUsedUnitsAddressesArray;


*/
static void build_SORTED___phoenix_units(void)
{
    GT_U32  numValidElem,ii;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &phoenix_units[0];

    if(SORTED___phoenix_units[0].base_addr != SMAIN_NOT_VALID_CNS)
    {
        /* already initialized */
        return;
    }

    memcpy(SORTED___phoenix_units,phoenix_units,sizeof(phoenix_units));

    numValidElem = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        numValidElem++;
    }

    qsort(SORTED___phoenix_units, numValidElem, sizeof(SMEM_GEN_UNIT_INFO_STC),
          sim_sip6_units_cellCompare);

}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        phoenixUsedUnitsAddressesArray - the addresses of the units that the Falcon uses
        phoenixUnitNameAndIndexArr - name of unit and index in phoenixUsedUnitsAddressesArray */
    GT_U32  ii;
    GT_U32  index;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &SORTED___phoenix_units[0];

    /* build once the sorted memory for the falcon .. for better memory search performance */
    build_SORTED___phoenix_units();

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        phoenixUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
        phoenixUsedUnitsAddressesArray[index].unitSizeInBytes = unitInfoPtr->size;
        phoenixUnitNameAndIndexArr[index].unitNameIndex = index;
        phoenixUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        index++;
    }

    if(index >= (sizeof(phoenixUnitNameAndIndexArr) / sizeof(phoenixUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    phoenixUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    phoenixUnitNameAndIndexArr[index].unitNameStr = NULL;
    phoenixUsedUnitsAddressesArray[index].unitBaseAddr = SMAIN_NOT_VALID_CNS;
    phoenixUsedUnitsAddressesArray[index].unitSizeInBytes = 0;

    devObjPtr->devMemUnitNameAndIndexPtr = phoenixUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = phoenixUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;/* no pipe offset */
    devObjPtr->support_memoryRanges = 1;

#if 0 /* check that the array is ascending ! (phoenixUsedUnitsAddressesArray) */
    for(ii = 0 ; ii < (index+1) ; ii++)
    {
        printf("unitBaseAddr = [0x%8.8x] \n",
            phoenixUsedUnitsAddressesArray[ii].unitBaseAddr);

        if(ii &&
            (phoenixUsedUnitsAddressesArray[ii].unitBaseAddr <=
             phoenixUsedUnitsAddressesArray[ii-1].unitBaseAddr))
        {
            printf("Error: at index[%d] prev index higher \n",ii);
            break;
        }
    }
#endif /*0*/
}

#define PHOENIX_PPG_OFFSET 0x80000
#define PHOENIX_PPN_OFFSET  0x4000
#define PHOENIX_PPN_NUM     2
#define PHOENIX_PPG_NUM     8
#define PHOENIX_TSU_OFFSET  0x4
#define PHOENIX_TSU_CHANNELS_PER_GROUP  16

/*ppn%n 0x00000000 + 0x80000 * m + 0x4000 * p: where p (0-7) represents PPN, where m (0-1) represents PPG*/
#define PHOENIX_PPN_REPLICATIONS PHOENIX_PPN_NUM,PHOENIX_PPG_OFFSET , PHOENIX_PPG_NUM,PHOENIX_PPN_OFFSET

#define PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS FORMULA_TWO_PARAMETERS(PHOENIX_PPN_NUM,PHOENIX_PPG_OFFSET , PHOENIX_PPG_NUM,PHOENIX_PPN_OFFSET)

/*ppg%g 0x00000000 + 0x80000 * g : where g (0-1) represents PPG*/
#define PHOENIX_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER FORMULA_SINGLE_PARAMETER(PHOENIX_PPN_NUM,PHOENIX_PPG_OFFSET)

#define PHOENIX_PPG_REPLICATION_ADDR(ppg)   \
    (PHOENIX_PPG_OFFSET * (ppg))

#define PHOENIX_PPN_REPLICATION_ADDR(ppg,ppn)   \
    (PHOENIX_PPG_REPLICATION_ADDR(ppg) + (PHOENIX_PPN_OFFSET * (ppn)))

#define PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,ppn) \
    /*PPN Internal Error Cause Register*/         \
    {0x00003040 + PHOENIX_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003044 + PHOENIX_PPN_REPLICATION_ADDR(ppg,ppn), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define PHOENIX_PPG_REPLICATIONS_ACTIVE_MEM(ppg) \
    /*PPG Interrupt Cause Register*/     \
    {0x0007FF00 + PHOENIX_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF04 + PHOENIX_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
    /*PPG internal error cause Register*/                                                                                              \
    {0x0007FF10 + PHOENIX_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x0007FF14 + PHOENIX_PPG_REPLICATION_ADDR(ppg), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},              \
                                        \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,0), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,1), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,2), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,3), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,4), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,5), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,6), \
    PHOENIX_PPN_REPLICATIONS_ACTIVE_MEM(ppg,7)

#define PHOENIX_TSU_CHANNEL_OFFSET_MAC(group, channel)  \
    (PHOENIX_TSU_OFFSET * ((group * PHOENIX_TSU_CHANNELS_PER_GROUP) + channel))

#define PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, channel) \
    /* Channel Interrupt Registers */         \
    {0x00003000 + PHOENIX_TSU_CHANNEL_OFFSET_MAC(group, channel), SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00003400 + PHOENIX_TSU_CHANNEL_OFFSET_MAC(group, channel), SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

#define PHOENIX_TSU_PER_GROUP_ACTIVE_MEM(group)  \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  0),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  1),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  2),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  3),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  4),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  5),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  6),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  7),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  8),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group,  9),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 10),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 11),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 12),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 13),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 14),   \
    PHOENIX_TSU_CHANNEL_ACTIVE_MEM(group, 15)

/**
* @internal smemHawkUnitPha function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PHA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitPha
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

    /* 2 PPG (with 8 PPNs each) */
    PHOENIX_PPG_REPLICATIONS_ACTIVE_MEM(0),/*include all it's PPN*/
    PHOENIX_PPG_REPLICATIONS_ACTIVE_MEM(1),/*include all it's PPN*/

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
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128,16)}, PHOENIX_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}  /*IMEM*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF00, 0x0007FF04)}, PHOENIX_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF10, 0x0007FF14)}, PHOENIX_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0007FF70, 0x0007FF70)}, PHOENIX_PPG_REPLICATIONS_FORMULA_SINGLE_PARAMETER}

            /* per PPN */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 2048)}      , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*scratchpad*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 20)}        , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet configuration*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002020, 4)}         , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet special registers*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002100, 64)}        , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet descriptor*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002200, 160)}       , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}  /*packet header*/
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003008)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003010, 0x00003010)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003020, 0x00003024)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003040, 0x00003044)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003050, 0x00003058)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003060, 0x00003068)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003070, 0x00003070)} , PHOENIX_PPN_REPLICATIONS_FORMULA_TWO_PARAMETERS}

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
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      PHOENIX_PPN_REPLICATIONS   }

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

    /*m * 0x80000: where m (0-1) represents PPG*/
    for(ppg = 0 ; ppg < PHOENIX_PPN_NUM; ppg++) /* per PPG */
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
        for(ppn = 0 ; ppn < PHOENIX_PPG_NUM; ppn++) /* per PPN (PER PPG) */
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
* @internal smemPhoenixUnitInitTai function
* @endinternal
*
* @brief   Allocate address type specific memories - for the TAI units
*/
static void smemPhoenixUnitInitTai
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
* @internal smemPhoenixSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
          {STR(UNIT_PHA)         ,smemPhoenixUnitPha}
         ,{STR(UNIT_TAI)         ,smemPhoenixUnitInitTai}
        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    smemHawkSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

/**
* @internal smemPhoenixUnitLmu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LMU unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitLmu
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
    IN GT_U32 unitIndex     /* the unit index 0..4 */
)
{
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.mif[unitIndex];
    GT_U32  isCpuPort = 0;
    GT_U32  ii;
    SMEM_CHT_PORT_MTI_PORT_MIF_REG_STC  *portDbPtr;
    SMEM_CHT_PORT_MTI_UNIT_MIF_REG_STC  *unitDbPtr;

    for(ii = 0 ; phoenixCpuPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
    {
        if(phoenixCpuPortsArr[ii].globalMacNum == portNum)
        {
            isCpuPort = 1;
            break;
        }
    }
    if(isCpuPort)
    {
        portDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[portIndex/*0/1*/].MIF_CPU;
        unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[sip6_MTI_EXTERNAL_representativePortIndex/*0*/].MIF_CPU_global;
    }
    else
    {

        portDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MIF;
        unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MIF_global;
    }

    if(portNum < 50)
    {
        /* 48 USX MAC ports and 2 CPU MAC ports */
        portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E  ] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000000 + portIndex*0x4;
        portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ] = unitOffset + MIF_TX_OFFSET     + 0x00000710 + portIndex*0x4;
    }
    else
    {
        /* 4 100G MAC ports*/
        portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E ] = unitOffset + MIF_GLOBAL_OFFSET + 0x00000080 + portIndex*0x4;
        portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_32_E] = unitOffset + MIF_TX_OFFSET     + 0x00000790 + portIndex*0x4;
    }

    if(portIndex == 0)
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

static void regAddr_anp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
)
{
    GT_U32  unitOffset;
    SMEM_CHT_PORT_ANP_UNIT_REG_STC  *unitDbPtr;
    GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_50G_E;
    GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_USX_E;
    GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_CPU_E;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32 unitIndex = 0;

    is_SMEM_UNIT_TYPE_MTI_MAC_50G_E = (GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_50G_E,portNum,&portInfo)) ? 1 : 0;
    is_SMEM_UNIT_TYPE_MTI_MAC_USX_E = (GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_USX_E,portNum,&portInfo)) ? 1 : 0;
    is_SMEM_UNIT_TYPE_MTI_MAC_CPU_E = (GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E,portNum,&portInfo)) ? 1 : 0;

    if(is_SMEM_UNIT_TYPE_MTI_MAC_CPU_E)
    {
        unitIndex = 6+(portNum-48); /* ports: 48..49  -> ANP[6..7]*/
    }
    else if(is_SMEM_UNIT_TYPE_MTI_MAC_50G_E)
    {
        unitIndex = 8; /* ports: 50-53  -> ANP[8]*/
    }
    else if(is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
    {
        if(portNum%8)
        {
            return;   /* ANP functionality works on single lane ports */
        }
        unitIndex = portNum/8; /* ports: 0..47 -> ANP[0..5]*/
    }
    else
        skernelFatalError("regAddr_anp : in ANP: MAC type for port %d \n",portNum);


    unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[unitIndex].ANP;
    unitOffset = devObjPtr->memUnitBaseAddrInfo.anp[unitIndex];

    unitDbPtr->portInterruptCause[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x100; /*0x00000100*/
    unitDbPtr->portInterruptMask[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x104; /*0x00000104*/
    unitDbPtr->interruptSummaryCause = unitOffset + 0x94; /*0x00000094*/
    unitDbPtr->interruptSummaryMask = unitOffset + 0x98; /*0x00000098*/

}


/**
* @internal smemPhoenixUnitTxqPdx function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pdx unit
*/
static void smemPhoenixUnitTxqPdx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*Alligned to  /Cider/EBU-IP/TXQ_IP/SIP7.2 (Phoenix)/TXQ_PR_IP/TXQ_PR_IP {7.2.3}/TXQ_PDX/PDX*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(txqPdxQueueGroupMap)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 256)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C04)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001010, 0x00001010)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030, 0x00001030)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001200)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001210, 0x00001210)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001220, 0x00001220)}
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
            for(i = 0; i < 2; i++)
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
            ,{DUMMY_NAME_PTR_CNS,           0x00001220,         0x00000f94,      1,    0x4    }/*pfcc_burst_fifo_thr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001210,         0x00000f94,      1,    0x4    }/*global_burst_fifo_thr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001200,         0x00000f94,      1,    0x4    }/*global_burst_fifo_available_entries*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
      static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
      unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitTxqPfcc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pfcc unit
*/
static void smemPhoenixUnitTxqPfcc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000108)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000031C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000444)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 384)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
        {
          static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
            {
                 {DUMMY_NAME_PTR_CNS,           0x00000000,         0x000000be,      1,    0x4    }/*global_pfcc_cfg*/
                ,{DUMMY_NAME_PTR_CNS,           0x0000000c,         0x0000ffff,      1,    0x4    }/*PFCC_metal_fix*/
                ,{DUMMY_NAME_PTR_CNS,           0x00000008,         0x00924924,      1,    0x4    }/*TC_to_pool_CFG*/
                ,{DUMMY_NAME_PTR_CNS,           0x00000400,         0x0000003f,      1,    0x4    }/*Source_Port_Requested_For_Read*/
                ,{NULL,                         0,                  0x00000000,      0,    0x0  }
            };
          static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
          unitPtr->unitDefaultRegistersPtr = &list;
        }

}



/**
* @internal smemPhoenixUnitTxqPsi function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq psi unit
*/
static void smemPhoenixUnitTxqPsi
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         /*PSI_REG*/
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000008, 4*(NUM_PORTS_PER_DP_UNIT+1))}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000238)}

         /*PDQ*/
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00000000, PHOENIX_PSI_SCHED_OFFSET+0x00000054)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00000800, PHOENIX_PSI_SCHED_OFFSET+0x0000081C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00000A00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00001A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00001E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00002200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00002600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00002A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00002E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00003200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00003600, PHOENIX_PSI_SCHED_OFFSET+0x0000360C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00003800, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00004800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00004C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00005000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00005400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00005800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00005C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00006000, PHOENIX_PSI_SCHED_OFFSET+0x0000600C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00006200, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00007200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00007600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00007A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00007E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00008200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00008600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00008A00, PHOENIX_PSI_SCHED_OFFSET+0x00008A0C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00008C00, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00009C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000A000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000A400, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000A800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000AC00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000B000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000B400, PHOENIX_PSI_SCHED_OFFSET+0x0000B404)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000B408, PHOENIX_PSI_SCHED_OFFSET+0x0000B40C)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000B600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000C000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0000E000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00010000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00012000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00014000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00016000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00016400, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00016600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00016A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00016E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00017200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00017600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00017A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00017E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00018200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00018600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00018A00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00018C00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00019000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00019400, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00019600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00019A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00019E00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001A000, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001B000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001D000, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001E000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001F000, PHOENIX_PSI_SCHED_OFFSET+0x0001F014)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001F200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001F600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001FA00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0001FE00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020A00, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020A10, PHOENIX_PSI_SCHED_OFFSET+0x00020A24)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020B00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020C00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020D00, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020D10, PHOENIX_PSI_SCHED_OFFSET+0x00020D14)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00020E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00021200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00021600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00021A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00021E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022640, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x000226C0, 64)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022740, PHOENIX_PSI_SCHED_OFFSET+0x00022764)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022770, 8)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022800, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022C00, PHOENIX_PSI_SCHED_OFFSET+0x00022C04)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00022E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00023200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00023600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00023A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00023E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024680, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024780, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024880, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x000248C0, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024900, PHOENIX_PSI_SCHED_OFFSET+0x00024934)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00024E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00025200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00025600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00025A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00025E00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026600, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026680, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026780, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026880, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x000268C0, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026900, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026920, 16)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026940, PHOENIX_PSI_SCHED_OFFSET+0x00026974)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00026A00, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00027000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029000, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029200, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029600, 512)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029A00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029B00, 128)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029C00, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029C40, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x00029C80, PHOENIX_PSI_SCHED_OFFSET+0x00029CBC)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0002A000, 4096)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0002C000, 32)}
         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PHOENIX_PSI_SCHED_OFFSET+0x0002C048, PHOENIX_PSI_SCHED_OFFSET+0x0002C17C)}
    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

    UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
        4*(NUM_PORTS_PER_DP_UNIT+1),
        4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

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
          ,{DUMMY_NAME_PTR_CNS,           0x00000000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrorStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000008+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*FirstExcp_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000010+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrCnt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000018+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ExcpCnt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000020+PHOENIX_PSI_SCHED_OFFSET,         0x00000037,      1,    0x8    }/*ExcpMask_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00000024+PHOENIX_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*ExcpMask_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000028+PHOENIX_PSI_SCHED_OFFSET,         0x00001033,      1,    0x8    }/*Identity_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000030+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ForceErr_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000038+PHOENIX_PSI_SCHED_OFFSET,         0x04081020,      1,    0x8    }/*ScrubSlots_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000003c+PHOENIX_PSI_SCHED_OFFSET,         0x00000004,      1,    0x8    }/*ScrubSlots_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000040+PHOENIX_PSI_SCHED_OFFSET,         0x00000007,      1,    0x8    }/*BPMsgFIFO_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00000044+PHOENIX_PSI_SCHED_OFFSET,         0x00000008,      1,    0x8    }/*BPMsgFIFO_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000048+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*TreeDeqEn_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000050+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PDWRREnReg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000800+PHOENIX_PSI_SCHED_OFFSET,         0x00500000,      1,    0x8    }/*PPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000808+PHOENIX_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*PPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000080c+PHOENIX_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*PPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000810+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PortExtBPEn_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000818+PHOENIX_PSI_SCHED_OFFSET,         0x00001001,      1,    0x8    }/*PBytePerBurstLimit_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003600+PHOENIX_PSI_SCHED_OFFSET,         0x071c0000,      1,    0x8    }/*CPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003608+PHOENIX_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*CPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000360c+PHOENIX_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*CPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006000+PHOENIX_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*BPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006008+PHOENIX_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*BPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000600c+PHOENIX_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*BPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008a00+PHOENIX_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*APerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008a08+PHOENIX_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*APerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00008a0c+PHOENIX_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*APerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b400+PHOENIX_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*QPerCtlConf_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b408+PHOENIX_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*QPerRateShap_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000b40c+PHOENIX_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*QPerRateShap_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f008+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ScrubDis_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f010+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      1,    0x8    }/*Plast_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001f014+PHOENIX_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*Plast_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a10+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00020a14+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a18+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a20+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020d10+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022740+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00022744+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022748+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022750+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022760+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024900+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00024904+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024908+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024910+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024920+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BBnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024930+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BPerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026940+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00026944+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026948+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026950+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*AMyQEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026960+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ABnkEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026970+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*APerStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c80+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QMemsEccErrStatus_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c88+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00029c8c+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029ca0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QBnkEccErrStatus_StartAddr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029ca8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank1EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029cb0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank2EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029cb8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*QueueBank3EccErrStatus*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c048+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*EccConfig_Addr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c050+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters0*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c058+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters1*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c060+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters2*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c068+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters3*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c070+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters4*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c078+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters5*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c080+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters6*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c088+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters7*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c090+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters8*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c098+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters9*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0a0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters10*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0a8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters11*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0b0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters12*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0b8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters13*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0c0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters14*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0c8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters15*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0d0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters16*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0d8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters17*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0e0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters18*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0e8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters19*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0f0+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters20*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c0f8+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters21*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c100+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters22*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c108+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters23*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c110+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters24*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c118+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters25*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c120+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters26*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c128+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters27*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c130+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters28*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c138+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters29*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c140+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters30*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c148+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters31*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c150+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters32*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c158+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters33*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c160+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters34*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c168+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters35*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c170+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters36*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c178+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ECC_Memory_Parameters37*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*PortEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00001e00+PHOENIX_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00001e04+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002200+PHOENIX_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002204+PHOENIX_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002a00+PHOENIX_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosLo*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002a04+PHOENIX_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00002e00+PHOENIX_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosHi*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00002e04+PHOENIX_PSI_SCHED_OFFSET,         0x00100010,      64,    0x8    }/*PortQuantumsPriosHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00003800+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*ClvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004800+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004c00+PHOENIX_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*ClvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005000+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00005004+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005400+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005800+PHOENIX_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*ClvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00005c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvltoPortAndBlvlRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00006200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*BlvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007600+PHOENIX_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*BlvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00007a04+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00007e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008200+PHOENIX_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*BlvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BLvltoClvlAndAlvlRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00008c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*AlvlEligPrioFunc_Entry*/
            ,{DUMMY_NAME_PTR_CNS,           0x00009c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a000+PHOENIX_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*AlvlTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a400+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0000a404+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000a800+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlDWRRPrioEn*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000ac00+PHOENIX_PSI_SCHED_OFFSET,         0x00000040,      64,    0x8    }/*AlvlQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ALvltoBlvlAndQueueRangeMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000b600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*QueueEligPrioFunc*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000c000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueEligPrioFuncPtr*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000e000+PHOENIX_PSI_SCHED_OFFSET,         0x0fff0fff,      512,    0x8    }/*QueueTokenBucketTokenEnDiv*/
            ,{DUMMY_NAME_PTR_CNS,           0x00010000+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00010004+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
            ,{DUMMY_NAME_PTR_CNS,           0x00012000+PHOENIX_PSI_SCHED_OFFSET,         0x00000040,      512,    0x8    }/*QueueQuantum*/
            ,{DUMMY_NAME_PTR_CNS,           0x00014000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueAMap*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016000+PHOENIX_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00016004+PHOENIX_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016400+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00016404+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016600+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio0*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio1*/
            ,{DUMMY_NAME_PTR_CNS,           0x00016e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio2*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017200+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio3*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017600+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio4*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio5*/
            ,{DUMMY_NAME_PTR_CNS,           0x00017e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio6*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018200+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortDefPrio7*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018600+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00018604+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018a00+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00018a04+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00018c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*CLvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019000+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019004+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019400+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019404+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019600+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*BlvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019a00+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019a04+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x00019e00+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x00019e04+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001a000+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*AlvlDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001b000+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001b004+PHOENIX_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001d000+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
            /**/,{DUMMY_NAME_PTR_CNS,           0x0001d004+PHOENIX_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001e000+PHOENIX_PSI_SCHED_OFFSET,         0x00000001,      512,    0x8    }/*QueueDef*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f200+PHOENIX_PSI_SCHED_OFFSET,         0x000001fe,      64,    0x8    }/*PortNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001f600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001fa00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x0001fe00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020200+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020600+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*PortWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020b00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PortBPFromSTF*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020c00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PortBPFromQMgr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020d00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*TMtoTMPortBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*ClvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021600+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00021e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022200+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*ClvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022640+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000226c0+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022770+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ClvlBPFromSTF*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022800+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMClvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00022e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*BlvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023600+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00023e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024200+PHOENIX_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*BlvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024680+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024780+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024880+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000248c0+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMBlvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00024e00+PHOENIX_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*AlvlNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025200+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlMyQ*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025600+PHOENIX_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus01*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025a00+PHOENIX_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus23*/
            ,{DUMMY_NAME_PTR_CNS,           0x00025e00+PHOENIX_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus45*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026200+PHOENIX_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus67*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026600+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*AlvlWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026680+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026780+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026880+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x000268c0+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026900+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026920+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00026a00+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*TMtoTMAlvlBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00027000+PHOENIX_PSI_SCHED_OFFSET,         0x00000002,      512,    0x8    }/*QueueNodeState*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      16,    0x8    }/*QueueWFS*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029200+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029600+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029a00+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029b00+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c00+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateLo*/
            ,{DUMMY_NAME_PTR_CNS,           0x00029c40+PHOENIX_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateHi*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002a000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*TMtoTMQueueBPState*/
            ,{DUMMY_NAME_PTR_CNS,           0x0002c000+PHOENIX_PSI_SCHED_OFFSET,         0x00000000,      4,    0x8    }/*QueuePerStatus*/
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPhoenixUnitTxqPds function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
static void smemPhoenixUnitTxqPds
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
     {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 32640)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 55232)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 13808)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037000, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00037800, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038000, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00038800, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039000, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00039800, 2048)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 4096)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041000, 0x00041034)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041040, 0x0004183C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00041900, 0x000420FC)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00043000, 0x00043018)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044000, 0x00044010)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00044020, 0x0004481C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000450F8, 0x00045118)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045120, 0x00045120)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045128, 0x00045128)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045130, 0x00045130)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004513C, 0x0004513C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045144, 0x00045144)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004514C, 0x00045158)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045164, 0x00045164)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0004516C, 0x0004516C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045174, 0x0004517C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045200, 0x00045204)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045300, 0x0004530C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045400, 0x0004540C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045440, 0x0004544C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045480, 0x0004548C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000454C0, 0x000454CC)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045500, 0x0004550C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00045540, 0x0004554C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00046000, 0x0004603C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00046100, 0x0004613C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00046200, 0x0004623C)}
    ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00046300, 0x0004633C)}
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
            ,{DUMMY_NAME_PTR_CNS,           0x00041010,         0x00000200,      1,    0x4    }/*Max_Num_Of_Long_Queues*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041014,         0x0000000f,      1,    0x4    }/*Tail_Size_for_PB_Wr*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041018,         0x00000014,      1,    0x4    }/*PID_Empty_Limit_for_PDX*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004101c,         0x00000014,      1,    0x4    }/*PID_Empty_Limit_for_PB*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041020,         0x0000000c,      1,    0x4    }/*NEXT_Empty_Limit_for_PDX*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041024,         0x0000000c,      1,    0x4    }/*NEXT_Empty_Limit_for_PB*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041030,         0x000000be,      1,    0x4    }/*PB_Full_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00041034,         0x0000000c,      1,    0x4    }/*PB_Read_Req_FIFO_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00043004,         0x00000002,      1,    0x4    }/*PDS_Interrupt_Summary_Mask*/
            ,{DUMMY_NAME_PTR_CNS,           0x00044000,         0x0000001f,      1,    0x4    }/*Idle_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004510c,         0x00000fff,      1,    0x4    }/*Free_Next_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045114,         0x00000fff,      1,    0x4    }/*Free_PID_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004517c,         0x000003ff,      1,    0x4    }/*FIFO_State_Latency_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00046100,         0x0000002a,      16,    0x4    }/*Profile_Long_Queue_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00046200,         0x00000026,      16,    0x4    }/*Profile_Head_Empty_Limit*/
            ,{DUMMY_NAME_PTR_CNS,           0x00046000,         0x00000001,      16,    0x4    }/*Profile_Long_Queue_Enable*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045300,         0x00000005,      1,    0x4    }/*pds_debug_configurations*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045308,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_lsb*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004530c,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_msb*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045508,         0x00000400,      1,    0x4    }/*pds_pb_write_req_count_type_ref*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004550c,         0x00003c00,      1,    0x4    }/*pds_pb_write_req_count_type_mask*/
            ,{DUMMY_NAME_PTR_CNS,           0x00045548,         0x00000400,      1,    0x4    }/*pds_pb_read_req_count_type_ref*/
            ,{DUMMY_NAME_PTR_CNS,           0x0004554c,         0x00003c00,      1,    0x4    }/*pds_pb_read_req_count_type_mask*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
      static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
      unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemPhoenixUnitTxqSdq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq sdq unit
*/
static void smemPhoenixUnitTxqSdq
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000504, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000700, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000017FC)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 4096)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004200, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004400, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004600, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004800, 4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 2*4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005600, 2*4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006300, 2*4*(NUM_PORTS_PER_DP_UNIT+1))}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000700C)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x0000710C)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007140, 0x0000714C)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007180, 0x0000718C)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000071C0, 0x000071CC)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008000, 0x00008024)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008030, 0x00008034)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008200, 448)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008400, 224)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008600, 448)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000, 4096)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B000, 2048)}
    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

    UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
        2*4*(NUM_PORTS_PER_DP_UNIT+1),
        2*4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

    UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
        4*(NUM_PORTS_PER_DP_UNIT+1),
        4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PortConfig*/
        {
            GT_U32  pt;
            for(pt = 0; pt <= RUNTIME_NUM_PORTS_PER_DP_UNIT; pt++)
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
        ,{DUMMY_NAME_PTR_CNS,           0x00004400,         0x00000007,      56,    0x4    }/*Port_Range_High*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004600,         0x00003a98,      56,    0x4    }/*Port_Back_Pressure_Low_Threshold*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004800,         0x00003a98,      56,    0x4    }/*Port_Back_Pressure_High_Threshold*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000007,      56,    0x4    }/*Port_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000500,         0x00000003,      1,    0x4    }/*QCN_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008000,         0x000201ff,      1,    0x4    }/*Sdq_Idle*/
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
        ,{DUMMY_NAME_PTR_CNS,           0x0000566c,         0x000000d8,      2,    0x4    }/*Sel_List_Range_Low_27*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005670,         0x000000e0,      2,    0x4    }/*Sel_List_Range_Low_28*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005674,         0x000000e8,      2,    0x4    }/*Sel_List_Range_Low_29*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005678,         0x000000f0,      2,    0x4    }/*Sel_List_Range_Low_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000567c,         0x000000f8,      2,    0x4    }/*Sel_List_Range_Low_31*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005680,         0x00000100,      2,    0x4    }/*Sel_List_Range_Low_32*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005684,         0x00000108,      2,    0x4    }/*Sel_List_Range_Low_33*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005688,         0x00000110,      2,    0x4    }/*Sel_List_Range_Low_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000568c,         0x00000118,      2,    0x4    }/*Sel_List_Range_Low_35*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005690,         0x00000120,      2,    0x4    }/*Sel_List_Range_Low_36*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005694,         0x00000128,      2,    0x4    }/*Sel_List_Range_Low_37*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005698,         0x00000130,      2,    0x4    }/*Sel_List_Range_Low_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000569c,         0x00000138,      2,    0x4    }/*Sel_List_Range_Low_39*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056a0,         0x00000140,      2,    0x4    }/*Sel_List_Range_Low_40*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056a4,         0x00000148,      2,    0x4    }/*Sel_List_Range_Low_41*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056a8,         0x00000150,      2,    0x4    }/*Sel_List_Range_Low_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056ac,         0x00000158,      2,    0x4    }/*Sel_List_Range_Low_43*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056b0,         0x00000160,      2,    0x4    }/*Sel_List_Range_Low_44*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056b4,         0x00000168,      2,    0x4    }/*Sel_List_Range_Low_45*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056b8,         0x00000170,      2,    0x4    }/*Sel_List_Range_Low_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056bc,         0x00000178,      2,    0x4    }/*Sel_List_Range_Low_47*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056c0,         0x00000180,      2,    0x4    }/*Sel_List_Range_Low_48*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056c4,         0x00000188,      2,    0x4    }/*Sel_List_Range_Low_49*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056c8,         0x00000190,      2,    0x4    }/*Sel_List_Range_Low_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056cc,         0x00000198,      2,    0x4    }/*Sel_List_Range_Low_51*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056d0,         0x000001a0,      2,    0x4    }/*Sel_List_Range_Low_52*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056d4,         0x000001a8,      2,    0x4    }/*Sel_List_Range_Low_53*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056d8,         0x000001b0,      2,    0x4    }/*Sel_List_Range_Low_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x000056dc,         0x000001b8,      2,    0x4    }/*Sel_List_Range_Low_55*/
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
        ,{DUMMY_NAME_PTR_CNS,           0x0000636c,         0x000000df,      2,    0x4    }/*Sel_List_Range_High_27*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006370,         0x000000e7,      2,    0x4    }/*Sel_List_Range_High_28*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006374,         0x000000ef,      2,    0x4    }/*Sel_List_Range_High_29*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006378,         0x000000f7,      2,    0x4    }/*Sel_List_Range_High_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000637c,         0x000000ff,      2,    0x4    }/*Sel_List_Range_High_31*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006380,         0x00000107,      2,    0x4    }/*Sel_List_Range_High_32*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006384,         0x0000010f,      2,    0x4    }/*Sel_List_Range_High_33*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006388,         0x00000117,      2,    0x4    }/*Sel_List_Range_High_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000638c,         0x0000011f,      2,    0x4    }/*Sel_List_Range_High_35*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006390,         0x00000127,      2,    0x4    }/*Sel_List_Range_High_36*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006394,         0x0000012f,      2,    0x4    }/*Sel_List_Range_High_37*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006398,         0x00000137,      2,    0x4    }/*Sel_List_Range_High_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000639c,         0x0000013f,      2,    0x4    }/*Sel_List_Range_High_39*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063a0,         0x00000147,      2,    0x4    }/*Sel_List_Range_High_40*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063a4,         0x0000014f,      2,    0x4    }/*Sel_List_Range_High_41*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063a8,         0x00000157,      2,    0x4    }/*Sel_List_Range_High_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063ac,         0x0000015f,      2,    0x4    }/*Sel_List_Range_High_43*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063b0,         0x00000167,      2,    0x4    }/*Sel_List_Range_High_44*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063b4,         0x0000016f,      2,    0x4    }/*Sel_List_Range_High_45*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063b8,         0x00000177,      2,    0x4    }/*Sel_List_Range_High_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063bc,         0x0000017f,      2,    0x4    }/*Sel_List_Range_High_47*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063c0,         0x00000187,      2,    0x4    }/*Sel_List_Range_High_48*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063c4,         0x0000018f,      2,    0x4    }/*Sel_List_Range_High_49*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063c8,         0x00000197,      2,    0x4    }/*Sel_List_Range_High_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063cc,         0x0000019f,      2,    0x4    }/*Sel_List_Range_High_51*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063d0,         0x000001a7,      2,    0x4    }/*Sel_List_Range_High_52*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063d4,         0x000001af,      2,    0x4    }/*Sel_List_Range_High_53*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063d8,         0x000001b7,      2,    0x4    }/*Sel_List_Range_High_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x000063dc,         0x000001bf,      2,    0x4    }/*Sel_List_Range_High_55*/
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
        ,{DUMMY_NAME_PTR_CNS,           0x00005078,         0x00000001,      2,    0x4    }/*Sel_List_Enable_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005088,         0x00000001,      2,    0x4    }/*Sel_List_Enable_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005098,         0x00000001,      2,    0x4    }/*Sel_List_Enable_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x000050a8,         0x00000001,      2,    0x4    }/*Sel_List_Enable_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000050b8,         0x00000001,      2,    0x4    }/*Sel_List_Enable_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000050c8,         0x00000001,      2,    0x4    }/*Sel_List_Enable_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000050d8,         0x00000001,      2,    0x4    }/*Sel_List_Enable_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007000,         0x00000005,      1,    0x4    }/*debug_config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007008,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_lsb*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000700c,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_msb*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002000,         0x60000000,      512,    0x8    }/*queue_cfg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b000,         0x00000381,      512,    0x4    }/*queue_elig_state*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
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
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_32_47_InterruptSummaryCause    = 0x00003808 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_48_63_InterruptSummaryCause    = 0x0000380C + unitOffset;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_0_15_InterruptSummaryMask      = 0x00003820 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_16_31_InterruptSummaryMask     = 0x00003824 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_32_47_InterruptSummaryMask     = 0x00003828 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].channel_48_63_InterruptSummaryMask     = 0x0000382C + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].ctsuGlobalInterruptSummaryCause        = 0x00000088 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCA_CTSU[unitIndex].ctsuGlobalInterrupSummarytMask         = 0x0000008C + unitOffset;

        for(index=0; index<NUM_PORTS_PER_CTSU_UNIT; index++)
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
* @internal regAddr_100G_MTIP_EXT function
* @endinternal
*
* @brief   initialize the register DB - 100G MAC - MTIP_EXT
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_100G_MTIP_EXT
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_EXT_BASE_OFFSET;

    /* Set formula per 50/100 ports*/
    portFormula = (0x18 * portIndex);
    /* start of unit MTI_EXT - per channel/port*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus          = 0x00000088 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause  = 0x00000094 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask   = 0x00000098 + portFormula + unitOffset;

}

/**
* @internal regAddr_USX_MTIP_EXT function
* @endinternal
*
* @brief   initialize the register DB - 100G MAC - MTIP_EXT
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_USX_MTIP_EXT
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_EXT_BASE_OFFSET;

    /* Set formula per USX ports*/
    portFormula = (0x28 * portIndex);
    /* start of unit MTI_EXT - per channel/port*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus          = 0x00000030 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause  = 0x00000038 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask   = 0x0000003C + portFormula + unitOffset;

}

/**
* @internal regAddr_100GMac function
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
static void regAddr_100GMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + PORT0_100G_OFFSET;
    GT_U32  portFormula;

    portFormula = (MAC_STEP_PORT_OFFSET * portIndex);

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig  = 0x00000008 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.frmLength      = 0x00000014 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr0       = 0x0000000C + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr1       = 0x00000010 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.status         = 0x00000040 + portFormula + unitOffset;

}
/**
* @internal regAddr_MTIP400GMacWrap function
* @endinternal
*
* @brief   initialize the register DB - MTIP 400G MAC
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            is_SMEM_UNIT_TYPE_MTI_MAC_USX_E - indication if using MAC_100 or MAC_USX
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
    GT_U32  indexTo_macWrap = is_SMEM_UNIT_TYPE_MTI_MAC_USX_E/*MTIP_USX_0_MAC_0_BASE_ADDR*/ ?
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr  :
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr;

    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[indexTo_macWrap] + MAC_EXT_BASE_OFFSET;

    if (is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
    {
        if (portIndex == 0)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalResetControl          = 0x00000000 + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause = 0x00000018 + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryMask  = 0x0000001C + unitOffset;
        }
        regAddr_USX_MTIP_EXT(devObjPtr, portNum, portIndex, indexTo_macWrap);
    }
    else
    {
        if(portIndex == 0)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalChannelControl        = 0x00000008 + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalResetControl          = 0x00000014 + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause = 0x0000004C + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryMask  = 0x00000050 + unitOffset;
        }
        regAddr_100G_MTIP_EXT(devObjPtr, portNum, portIndex, indexTo_macWrap);
    }

    regAddr_100GMac      (devObjPtr,portNum,portIndex,indexTo_macWrap);
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
*            is_SMEM_UNIT_TYPE_MTI_MAC_USX_E - indication if using MAC_100 or MAC_USX
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
    GT_U32  indexTo_macPcs = is_SMEM_UNIT_TYPE_MTI_MAC_USX_E ?
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr  :
            unitIndex + sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macPcs[indexTo_macPcs];

    if (is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)/*UNIT_USX_0_PCS_0*/
    {
        portFormula = USX_PCS_PORTS_OFFSET + (USX_PCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.status1  = 0x00000004 + portFormula + unitOffset;

        portFormula = USX_PCS_LPCS_OFFSET  + (USX_LPCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.status1  = 0x00000004 + portFormula + unitOffset;

        if(portIndex == 0)
        {
            /* info of PCS that is per representavive port */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].PCS_common.LPCS_common.gmode = 0x000003e0 + portFormula + unitOffset;/*0x000063e0*/
        }
    }
    else
    {
        /* Set formula per 50/100 ports : portIndex = 0..3 */
        portFormula = PCS_PORT0_50G_OFFSET + (PCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.status1  = 0x00000004 + portFormula + unitOffset;

        if (portIndex == 0)/* additional PCS when channel 0 work 100G KR4 (not for 40G KR4)*/
        {
            /* use the DB of 'PCS_400G_200G' as the 'extra' PSC */
            portFormula = PCS_PORT0_100G_OFFSET;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.control1 = 0x00000000 + portFormula + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.status1  = 0x00000004 + portFormula + unitOffset;
        }

        portFormula = PCS_LPCS_OFFSET + (LPCS_STEP_PORT_OFFSET * portIndex);
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.control1 = 0x00000000 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.status1  = 0x00000004 + portFormula + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.usxgmii_rep = 0x00000058 + portFormula + unitOffset;


        if(portIndex == 0)
        {
            /* info of PCS that is per representavive port */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].PCS_common.LPCS_common.gmode = 0x000003e0 + PCS_LPCS_OFFSET + unitOffset;/*0x000063e0*/
        }
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
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitFormula = devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[unitIndex];

    portFormula = unitFormula + CPU_MAC_EXT_BASE_OFFSET;

    /* MTIP IP CPU MAC WRAPPER {Current}/<MTIP CPU EXT> MTIP CPU EXT/MTIP CPU EXT Units */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portStatus          = 0x00000008  + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptCause  = 0x0000000C + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_EXT_PORT.portInterruptMask   = 0x00000010 + portFormula;

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_GLOBAL.control               = 0x00000000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_GLOBAL.globalResetControl    = 0x00000014 + portFormula;

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
    smemChtGopMtiInitMacMibCounters(devObjPtr,portNum ,0/*portIndex*/ , unitIndex/*channelIndex*/ , 0/*dieIndex*/ , 0 /*globalRaven*/ , 1/*isCpuPort*/);
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
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    GT_U32 unitIndex
)
{
    GT_U32  portFormula;
    GT_U32  unitFormula = devObjPtr->memUnitBaseAddrInfo.cpuMacPcs[unitIndex];

    portFormula = unitFormula + CPU_PCS_PORT_OFFSET;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuPcs.control1      = 0x00000000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[unitIndex].MTI_cpuPcs.status1       = 0x00000004 + portFormula;
}


/**
* @internal smemPhoenixGopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Phoenix.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemPhoenixGopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  unitIndex,globalPort;
    ENHANCED_PORT_INFO_STC portInfo;

    devObjPtr->portMacOffset = MAC_STEP_PORT_OFFSET;
    devObjPtr->portMacMask   = 0x7F;


    /* ANP support */
    {
        GT_U32  numOfAnps = 0;

        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_0_BASE_ADDR; /* ANP for USX 0 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_1_BASE_ADDR; /* ANP for USX 1 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_2_BASE_ADDR; /* ANP for USX 2 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_3_BASE_ADDR; /* ANP for USX 3 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_4_BASE_ADDR; /* ANP for USX 4 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_5_BASE_ADDR; /* ANP for USX 5 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_6_BASE_ADDR; /* ANP for CPU 0 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_7_BASE_ADDR; /* ANP for CPU 1 */
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_8_BASE_ADDR; /* ANP for MTI100 */

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            regAddr_anp(devObjPtr, globalPort);
        }
    }

    /* MIF support */
    {
        GT_U32  numOfMifs = 0;

        devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_0_BASE_ADDR; /*USX_MIF_0*/
        devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_1_BASE_ADDR; /*USX_MIF_1*/
        devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_2_BASE_ADDR; /*USX_MIF_2*/
        devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_3_BASE_ADDR; /*CPU_MIF_3*/
        devObjPtr->memUnitBaseAddrInfo.mif[numOfMifs++] =  MIF_4_BASE_ADDR; /*100G_MIF_4*/

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E,globalPort,&portInfo))
            {
                continue;
            }

            regAddr_mif(devObjPtr,
                globalPort,/* global port in the device */
                portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);  /* the unit index 0..4 */
        }
    }

    /* LMU support */
    {
        GT_U32  numOfLmus = 1;

        devObjPtr->memUnitBaseAddrInfo.lmu[0] = LMU_0_BASE_ADDR;
        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_LMU_1))
        {
            numOfLmus++;
            devObjPtr->memUnitBaseAddrInfo.lmu[1] = SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
            if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_LMU_2))
            {
                numOfLmus++;
                devObjPtr->memUnitBaseAddrInfo.lmu[2] =  SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
                if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_LMU_3))
                {
                    numOfLmus++;
                    devObjPtr->memUnitBaseAddrInfo.lmu[3] = SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
                }
            }
        }

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
        devObjPtr->memUnitBaseAddrInfo.tsu[0] = TSU_0_BASE_ADDR;
        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_CTSU_1))
        {
            devObjPtr->memUnitBaseAddrInfo.tsu[1] = SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
            if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_CTSU_2))
            {
                devObjPtr->memUnitBaseAddrInfo.tsu[2] = SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
                if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PCA_CTSU_3))
                {
                    devObjPtr->memUnitBaseAddrInfo.tsu[3] = SMAIN_NOT_VALID_CNS;/* cause error , as unit not exist */
                }
            }
        }

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E,globalPort,&portInfo))
            {
                continue;
            }

            regAddr_tsu(devObjPtr,
                globalPort,/* global port in the device */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */
        }
    }

    /* 100G MAC,PCS support */
    {
        GT_U32  unitIndex;

        unitIndex = sip6_MTI_EXTERNAL_representativePortIndex_base_phoenix100GPortsArr;
        /* MAC MIB support : needed by smemChtGopMtiInitMacMibCounters(...) */
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex] = MAC_MTIP_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MAC_MTIP_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex] = MAC_MTIP_MAC_PCS_0_BASE_ADDR;

        unitIndex = sip6_MTI_EXTERNAL_representativePortIndex_base_phoenixUsxPortsArr;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_0_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_0_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_0_PCS_0_BASE_ADDR;

        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_1_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_1_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_1_PCS_0_BASE_ADDR;

        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_2_MAC_0_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_2_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_2_PCS_0_BASE_ADDR;

        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_0_MAC_1_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_0_MAC_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_0_PCS_1_BASE_ADDR;

        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_1_MAC_1_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_1_MAC_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_1_PCS_1_BASE_ADDR;

        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex]  = MTIP_USX_2_MAC_1_BASE_ADDR + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] = MTIP_USX_2_MAC_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macPcs[unitIndex]  = MTIP_UNIT_USX_2_PCS_1_BASE_ADDR;

        /* CPU MAC,PCS support */
        {
            /* NOTE : the device hold actually two separate CPU ports*/
            unitIndex = 0;
            devObjPtr->memUnitBaseAddrInfo.cpuMacMib [unitIndex] = CPU_0_MAC_MTIP_MAC_BASE_ADDR + CPU_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[unitIndex] = CPU_0_MAC_MTIP_MAC_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.cpuMacPcs [unitIndex] = CPU_0_MAC_MTIP_MAC_PCS_BASE_ADDR;

            unitIndex++;
            devObjPtr->memUnitBaseAddrInfo.cpuMacMib [unitIndex] = CPU_1_MAC_MTIP_MAC_BASE_ADDR + CPU_MAC_MIB_OFFSET;
            devObjPtr->memUnitBaseAddrInfo.cpuMacWrap[unitIndex] = CPU_1_MAC_MTIP_MAC_BASE_ADDR;
            devObjPtr->memUnitBaseAddrInfo.cpuMacPcs [unitIndex] = CPU_1_MAC_MTIP_MAC_PCS_BASE_ADDR;
        }

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_50G_E = (GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_50G_E,globalPort,&portInfo)) ? 1 : 0;
            GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_USX_E = (GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_USX_E,globalPort,&portInfo)) ? 1 : 0;

            if(is_SMEM_UNIT_TYPE_MTI_MAC_50G_E ||
               is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
            {
                regAddr_MTIP400GMacWrap(devObjPtr,
                    globalPort,/* global port in the device */
                    is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);  /* for usx : the unit index 0..5 , for 100GMAc : index 0 */

                regAddr_MTIP400GPcs(devObjPtr,
                    globalPort,/* global port in the device */
                    is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
                    portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                    portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                    portInfo.simplePortInfo.unitIndex);  /* for usx : the unit index 0..5 , for 100GMAc : index 0 */
            }

            if(GT_OK == smemPhoenixPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E,globalPort,&portInfo))
            {
                /* CPU MAC,PCS support */
                regAddr_MTIPCpuMacWrap(devObjPtr, globalPort ,portInfo.simplePortInfo.unitIndex);
                regAddr_MTIPCpuPcs    (devObjPtr,             portInfo.simplePortInfo.unitIndex);
            }
        }
    }
}

/**
* @internal smemPhoenixUnitMac400GWrap function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC 400G unit wrapper
*/
static void smemPhoenixUnitMac400GWrap
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
        {MAC_MIB_OFFSET + 0x00000020, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000040, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000080, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        /* Statistic counters - active memory write control */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.control */
        {MAC_MIB_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},


        /* MIB - END */

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause */
        {MAC_EXT_BASE_OFFSET + 0x0000004C, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},


        /*0x00000094 + 0x18*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*2, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*3, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
#if 0 /*only 4 ports*/
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*5, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*6, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000094 + 0x18*7, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
#endif

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*2, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*3, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
#if 0 /*only 4 ports*/
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*4, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*5, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*6, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000098 + 0x18*7, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
#endif

#if 0 /* no segmented ports */
        /*0x00000068 + 0x14*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause */
        {MAC_EXT_BASE_OFFSET + 0x00000068 + 0x14*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000068 + 0x14*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask */
        {MAC_EXT_BASE_OFFSET + 0x0000006c + 0x14*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000006c + 0x14*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
#endif

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)


    /* MIB - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_MIB_OFFSET + 0x00000000, MAC_MIB_OFFSET + 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_MIB_OFFSET + 0x0000001C, MAC_MIB_OFFSET + 0x00000A9C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    /* MIB - END */

    /* MAC 100G - start */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC : ports 0..3 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000000, PORT0_100G_OFFSET + 0x00000014)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x0000001C, PORT0_100G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000030, PORT0_100G_OFFSET + 0x00000048)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x00000054, PORT0_100G_OFFSET + 0x00000074)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PORT0_100G_OFFSET + 0x0000007C, PORT0_100G_OFFSET + 0x000000A0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET)}
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
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MAC_EXT_BASE_OFFSET + 0x00000000, MAC_EXT_BASE_OFFSET + 0x0000036C)}
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
             {DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000008,         0x5a700000,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,           MAC_EXT_BASE_OFFSET + 0x00000084,         0x007AFD10,      8,    0x18  }
            /* MAC 400G EXT - end */

            /* MAC 100G - start */
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000000,         0x00010200,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000008,         0x00000800,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000014,         0x00000600,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x0000001c,         0x00000008,      2,    0x4  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000030,         0x00001440,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000044,         0x0000000c,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            ,{DUMMY_NAME_PTR_CNS,           PORT0_100G_OFFSET + 0x00000080,         0x00000100,      1,    0x0  ,NUM_PORTS_PER_100G_UNIT , MAC_STEP_PORT_OFFSET}
            /* MAC 100G - end */

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitPcs400G function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC 400G unit wrapper
*/
static void smemPhoenixUnitPcs400G
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{


    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* support single port in 100G PCS */
        {PCS_PORT0_100G_OFFSET                             + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        /* support 4 ports in 25G/50G PCS  */
        {PCS_PORT0_50G_OFFSET  + PCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_50G_OFFSET  + PCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_50G_OFFSET  + PCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_PORT0_50G_OFFSET  + PCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        /* support 4 ports in LPCS  */
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {PCS_LPCS_OFFSET      + LPCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
             /* PCS-100G - start (ports = 0) */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000000, PCS_PORT0_100G_OFFSET + 0x00000020)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000038, PCS_PORT0_100G_OFFSET + 0x0000003C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000080, PCS_PORT0_100G_OFFSET + 0x000000B4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x000000C8, PCS_PORT0_100G_OFFSET + 0x000000D4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000320, PCS_PORT0_100G_OFFSET + 0x0000036C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000640, PCS_PORT0_100G_OFFSET + 0x0000068C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000800, PCS_PORT0_100G_OFFSET + 0x0000080C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000820, PCS_PORT0_100G_OFFSET + 0x00000840)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_100G_OFFSET + 0x00000900, PCS_PORT0_100G_OFFSET + 0x0000099C)}}
             /* PCS-100G - end */
             /* PCS-50G - start (ports = 0,2) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000000, PCS_PORT0_50G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000038, PCS_PORT0_50G_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000080, PCS_PORT0_50G_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x000000C8, PCS_PORT0_50G_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x000000D0, PCS_PORT0_50G_OFFSET + 0x000000D0)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000320, PCS_PORT0_50G_OFFSET + 0x0000032C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000640, PCS_PORT0_50G_OFFSET + 0x0000064C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000800, PCS_PORT0_50G_OFFSET + 0x0000080C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000820, PCS_PORT0_50G_OFFSET + 0x00000840)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT0_50G_OFFSET + 0x00000900, PCS_PORT0_50G_OFFSET + 0x0000091C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
             /* PCS-50G - end */
             /* PCS-25G - start (ports = 1,3) */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000000, PCS_PORT1_25G_OFFSET + 0x00000020)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000038, PCS_PORT1_25G_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000080, PCS_PORT1_25G_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x000000C8, PCS_PORT1_25G_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x000000D0, PCS_PORT1_25G_OFFSET + 0x000000D0)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000320, PCS_PORT1_25G_OFFSET + 0x0000032C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000640, PCS_PORT1_25G_OFFSET + 0x0000064C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000800, PCS_PORT1_25G_OFFSET + 0x0000080C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000820, PCS_PORT1_25G_OFFSET + 0x00000840)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_PORT1_25G_OFFSET + 0x00000900, PCS_PORT1_25G_OFFSET + 0x0000091C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET))}
             /* PCS-25G - end */
             /* PCS-RSFEC - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000000, PCS_RS_FEC_OFFSET + 0x00000044)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000100, PCS_RS_FEC_OFFSET + 0x00000118)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_100G_UNIT - 1), 0x20)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000200, PCS_RS_FEC_OFFSET + 0x00000234)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (PCS_RS_FEC_OFFSET + 0x00000300, PCS_RS_FEC_OFFSET + 0x00000318)}}
             /* PCS-RSFEC - end */
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
            /*100G*/
             {DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000000,         0x0000204c,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000010,         0x0000003d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000020,         0x000081b1,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000088,         0x0000804f,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000008c,         0x0000cab6,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000090,         0x0000b44d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000094,         0x000003c8,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000098,         0x00008884,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000009c,         0x000085a3,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000a0,         0x000006bb,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000a4,         0x00000349,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x000000b4,         0x00008000,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000808,         0x00003fff,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000080c,         0x00009999,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000820,         0x000068c1,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000824,         0x00000021,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000828,         0x0000719d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000082c,         0x0000008e,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000830,         0x00004b59,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000834,         0x000000e8,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000838,         0x0000954d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000083c,         0x0000007b,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000840,         0x00000303,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000900,         0x000068c1,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000904,         0x00000021,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000908,         0x0000719d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000090c,         0x0000008e,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000910,         0x00004b59,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000914,         0x000000e8,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000918,         0x0000954d,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000091c,         0x0000007b,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000920,         0x000007f5,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000924,         0x00000009,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000928,         0x000014dd,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000092c,         0x000000c2,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000930,         0x00004a9a,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000934,         0x00000026,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000938,         0x0000457b,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000093c,         0x00000066,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000940,         0x000024a0,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000944,         0x00000076,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000948,         0x0000c968,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000094c,         0x000000fb,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000950,         0x00006cfd,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000954,         0x00000099,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000958,         0x000091b9,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000095c,         0x00000055,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000960,         0x0000b95c,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000964,         0x000000b2,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000968,         0x0000f81a,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000096c,         0x000000bd,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000970,         0x0000c783,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000974,         0x000000ca,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000978,         0x00003635,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000097c,         0x000000cd,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000980,         0x000031c4,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000984,         0x0000004c,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000988,         0x0000d6ad,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000098c,         0x000000b7,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000990,         0x0000665f,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000994,         0x0000002a,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x00000998,         0x0000f0c0,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_100G_OFFSET + 0x0000099c,         0x000000e5,      1,    0x0            ,(1) , (2*PCS_STEP_PORT_OFFSET)}
            /*100G*/

            /*50G*/
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000000,         0x0000204c,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000008,         0x00000001,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000010,         0x00000035,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000014,         0x00000008,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000001c,         0x00000004,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000020,         0x00008191,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000088,         0x0000804f,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000008c,         0x0000cab6,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000090,         0x0000b44d,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000094,         0x000003c8,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000098,         0x00008884,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000009c,         0x000085a3,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x000000a0,         0x000006bb,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x000000a4,         0x00000349,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x000000b4,         0x00008000,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000804,         0x00000300,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000808,         0x00003fff,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000080c,         0x00009999,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000820,         0x00007690,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000824,         0x00000047,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000828,         0x0000c4f0,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000082c,         0x000000e6,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000830,         0x000065c5,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000834,         0x0000009b,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000838,         0x000079a2,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000083c,         0x0000003d,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000840,         0x00000303,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000900,         0x00007690,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000904,         0x00000047,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000908,         0x0000c4f0,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000090c,         0x000000e6,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000910,         0x000065c5,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000914,         0x0000009b,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x00000918,         0x000079a2,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
            ,{DUMMY_NAME_PTR_CNS,            PCS_PORT0_50G_OFFSET + 0x0000091c,         0x0000003d,      1,    0x0            ,(NUM_PORTS_PER_100G_UNIT / 2) , (2*PCS_STEP_PORT_OFFSET)}
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
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000004,         0x00008002,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000100,         0x00008002,      3,    0x20           }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000228,         0x00000033,      4,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,            PCS_RS_FEC_OFFSET + 0x00000300,         0x00000003,      1,    0x0            }

            /*rsfec*/

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPhoenixUnitMacCpuWrap function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the MAC CPU unit wrapper
*/
static void smemPhoenixUnitMacCpuWrap
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
        {CPU_MAC_EXT_BASE_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},

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
* @internal smemPhoenixUnitPcsCpu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCS CPU unit wrapper
*/
static void smemPhoenixUnitPcsCpu
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000800, CPU_PCS_PORT_OFFSET+0x0000080C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_PORT_OFFSET+0x00000820, CPU_PCS_PORT_OFFSET+0x00000848)}
            /* PCS-CPU - end */

            /* rsfec-CPU - start */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000000, CPU_PCS_RS_FEC_OFFSET+0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (CPU_PCS_RS_FEC_OFFSET+0x00000028, CPU_PCS_RS_FEC_OFFSET+0x00000044)}
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
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_PORT_OFFSET+0x00000804,         0x00000100,      1,    0x0     }
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
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_RS_FEC_OFFSET+0x00000200,         0x00000000,      1,    0x0   }
            ,{DUMMY_NAME_PTR_CNS,            CPU_PCS_RS_FEC_OFFSET+0x00000304,         0x00000000,      1,    0x0   }
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
* @internal smemPhoenixUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x00001D84, 0xFFFFFFFF  , NULL, 0, smemFalconActiveWriteRxDmaDebugClearAllCounters, 0, NULL},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000002C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000118)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001080, 0x00001244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001258, 0x000013EF)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000014F0, 0x000014FC)}
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

    /* per channel */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000120, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000900, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000A00, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000B00, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000013f0, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001500, 4*(NUM_PORTS_PER_DP_UNIT+1))}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
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
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x00000407,     (NUM_PORTS_PER_DP_UNIT+1),    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000001,     (NUM_PORTS_PER_DP_UNIT+1),    0x4,      2,    0x200}
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

        UPDATE_LIST_REGISTER_DEFAULT_SIZE_TO_SIZE(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);

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
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {  /* manually fixed from : for(n = 0 ; n <= 63 ; n++) { */
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
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {
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
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelToLocalDevSourcePort[n] =
                            0x0000a00+n*0x4;
                    }/* end of loop n */
                }/*00004a8+n*0x4*/
                {/*00003a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.PCHConfig[n] =
                            0x0000900+n*0x4;
                    }/* end of loop n */
                }/*00003a8+n*0x4*/
                {/*00005a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].configs.channelConfig.channelGeneralConfigs[n] =
                            0x0000b00+n*0x4;
                    }/* end of loop n */
                }/*00005a8+n*0x4*/
                {/*00002a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; n++) {
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

        {/* interrupts - manually added */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_0_cause        = 0x00001C80;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_0_mask         = 0x00001C84;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_1_cause        = 0x00001C88;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_1_mask         = 0x00001C8C;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_2_cause        = 0x00001C90;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_2_mask         = 0x00001C94;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_cause  = 0x00001CA0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA[0].interrupts.rxdma_interrupt_summary_mask   = 0x00001CA4;
        }/* interrupts - manually added */
    }/*end of unit sip6_rxDMA[] */
}

/**
* @internal smemPhoenixUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitTxFifo
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000002EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001100, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001200, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
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

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

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
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00000008,     NUM_PORTS_PER_DP_UNIT+1,    0x4  }
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

        UPDATE_LIST_REGISTER_DEFAULT_SIZE_TO_SIZE(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);
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
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.speedProfileConfigs.tagFIFODepth[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/

        }/*end of unit speedProfileConfigs */

        {/*start of unit channelConfigs */
            {/*0x00001000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.speedProfile[p] =
                        0x00001000 +p*0x4;
                }/* end of loop n */
            }/*0x00001000  + p*0x4*/
            {/*0x00001100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00001100  +p*0x4;
                }/* end of loop n */
            }/*0x00001100   + p*0x4*/
            {/*0x00001200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.channelConfigs.tagFIFOBase[p] =
                        0x00001200  +p*0x4;
                }/* end of loop n */
            }/*0x00001200   + p*0x4*/

        }/*end of unit channelConfigs */

        {/*start of unit debug*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].debug.statusBadAddr = 0x00005000;
        }/*end of unit debug*/

        {/* interrupts - manually added */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].interrupts.txf_interrupt_cause = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].interrupts.txf_interrupt_mask = 0x00004004;
        }/* interrupts - manually added */

     }/*end of unit sip6_txFIFO[] */

}

/**
* @internal smemPhoenixUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitTxDma
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000002EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x0000082C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000092C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E2C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002100, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002200, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002300, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002400, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003100, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003200, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003300, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003400, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003500, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003600, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003700, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
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

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

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
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x00000008,     NUM_PORTS_PER_DP_UNIT+1,    0x4      }
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
            ,{DUMMY_NAME_PTR_CNS,            0x00002300,         0x18000000,     NUM_PORTS_PER_DP_UNIT+1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400,         0x00000007,     NUM_PORTS_PER_DP_UNIT+1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003700,         0x00008000,     NUM_PORTS_PER_DP_UNIT+1,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004000,         0x00000003,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00004008,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000400c,         0x0fa00200,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00005000,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x00000801,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        UPDATE_LIST_REGISTER_DEFAULT_SIZE_TO_SIZE(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);
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
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqMaxCredits[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/
            {/*0x00000800 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqThresholdBytes[p] =
                        0x00000800 + p*0x4;
                    }/* end of loop p */
            }/*0x00000800 + p*0x4*/
            {/*0x00000900 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxWordCredits[p] =
                        0x00000900 + p*0x4;
                    }/* end of loop p */
            }/*0x00000900 + p*0x4*/
            {/*0x00000A00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxCellsCredits[p] =
                        0x00000A00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000A00 + p*0x4*/
            {/*0x00000B00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxDescCredits[p] =
                        0x00000B00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000B00 + p*0x4*/
            {/*0x00000C00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interPacketRateLimiter[p] =
                        0x00000C00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000C00 + p*0x4*/
             {/*0x00000D00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < PHOENIX_MAX_PROFILE_CNS; p++) {
                  SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interCellRateLimiter[p] =
                        0x00000D00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000D00 + p*0x4*/

        }/*end of unit speedProfileConfigurations */

        {/*start of unit channelConfigs */
            {/*0x00002000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.speedProfile[p] =
                        0x00002000 +p*0x4;
                }/* end of loop n */
            }/*0x00002000  + p*0x4*/
            {/*0x00002100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.channelReset[p] =
                        0x00002100  +p*0x4;
                }/* end of loop n */
            }/*0x00002100   + p*0x4*/
            {/*0x00002200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.descFIFOBase[p] =
                        0x00002200  +p*0x4;
                }/* end of loop n */
            }/*0x00002200   + p*0x4*/
            {/*0x00002300  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.interPacketRateLimiterConfig[p] =
                        0x00002300 +p*0x4;
                }/* end of loop n */
            }/*0x00002300  + p*0x4*/
            {/*0x00002400  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
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

        {/* interrupts - manually added */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].interrupts.txd_interrupt_cause = 0x00006000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].interrupts.txd_interrupt_mask = 0x00006004;
        }/* interrupts - manually added */
    }/*end of unit sip6_txDMA[] */
}

/* Start of Packet Buffer */
/**
* @internal smemPhoenixUnitPacketBuffer_pbCenter function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_pbCenter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCenter */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/PB_CENTER/<PB_CENTER> pb_center*/
    /*IP: \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.2 - Phoenix \PB_CENTER {PB_Phoenix_20190806} \pb_center*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.2 - Phoenix \PB_CENTER {PB_Phoenix_20190806}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000112C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000125C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001300)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000140C, 0x0000141C)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemPhoenixUnitPacketBuffer_pbCounter function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_pbCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCounter*/
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/PB_CENTER/<PB_COUNTER> pb_counter*/
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
* @internal smemPhoenixUnitPacketBuffer_smbWriteArbiter function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_smbWriteArbiter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* smbWriteArbiter */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<SMB_WRITE> smb_write_arbiter*/
    /*IP: \Cider \EBU-IP \Packet Buffer \SMB_WRITE_ARBITER \SMB_WRITE_ARBITER 1.2 Phoenix \SMB_WRITE_ARBITER {PB_Phoenix_20190806} \smb_write_arbiter*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \SMB_WRITE_ARBITER \SMB_WRITE_ARBITER 1.2 Phoenix \SMB_WRITE_ARBITER_Phoenix {PB_Phoenix_20190806}*/
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
* @internal smemPhoenixUnitPacketBuffer_packetWrite function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_packetWrite
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketWrite */
    /* gpcPacketWrite - Hawk has no grouping units - just used uxisting name */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<GPC_PACKET_WRITE> gpc_packet_write*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_WRITE \GPC_PACKET_WRITE 1.0 - Phoenix \GPC_PACKET_WRITE {PB_Phoenix_20191126} \gpc_packet_write*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000020)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000015C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000208)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000030C)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemPhoenixUnitPacketBuffer_packetRead function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_packetRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketRead */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<GPC_PACKET_READ> gpc_packet_read*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_READ \GPC_PACKET_READ 1.1 - Phoenix \GPC_PACKET_READ {PB_Phoenix_20191126} \gpc_packet_read*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_READ \GPC_PACKET_READ 1.1 - Phoenix \GPC_PACKET_READ {PB_Phoenix_20191126}*/
    {
        static SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Configuration */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
             /* Packet Count Configuration */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000014)}
            /* pizza arbiter */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x000003FC)}
            /* Channel Configuration */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x000005E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x000006E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x000007E0)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000010E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x000011E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001220)}
            /*debug. Credit Counters. Channel Credits*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001260, 0x00001278)}
            /*debug. Credit Counters*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001284, 0x00001290)}
            /*debug. Latency Statistics*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001294, 0x000012C8)}
            /*Interrupts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x000013E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x000014E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001538)}
        };
        static GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemPhoenixUnitPacketBuffer_cellRead function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_cellRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcCellRead */
    /*Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<GPC_CELL_READ> gpc_cell_read*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_CELL_READ \GPC_CELL_READ_Phoenix \GPC_CELL_READ {PB_Phoenix_20190806} \gpc_cell_read*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_CELL_READ \GPC_CELL_READ_Phoenix \GPC_CELL_READ {PB_Phoenix_20190806*/
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
* @internal smemPhoenixUnitPacketBuffer_npmMc function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_npmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* npmMc */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<NPM_MC> npm_mc*/
    /*IP: \Cider \EBU-IP \Packet Buffer \NPM_MC \NPM_MC 1.2 - Phoenix \NPM_MC {PB_Phoenix_20191121} \npm_mc*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \NPM_MC \NPM_MC 1.2 - Phoenix \NPM_MC {PB_Phoenix_20191121}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /*General*/
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
             /*Interrupt*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000158)}
             /*Free Lists*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000200)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000210)}
             /*Memory Units*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000300)}
             /*Reference Counters*/
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000410, 0x00000410)}
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
* @internal smemPhoenixUnitPacketBuffer_sbmMc function
* @endinternal
 *
*/
static void smemPhoenixUnitPacketBuffer_sbmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* sbmMc */
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PB/<SMB_MC> smb_mc*/
    /*IP: \Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 1.1 Phoenix \SMB_MC {PB_Phoenix_20191126} \smb_mc*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 1.1 Phoenix \SMB_MC_Phoenix {PB_Phoenix_20191126}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000038)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000006C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000074)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000084)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C4)}
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
* @internal smemPhoenixUnitSerdes_x4 function
* @endinternal
*
* @brief   Allocate address type specific memories - SERDESes
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitSerdes_x4
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
#define SMEM_PER_LANE_MASK_CNS (0xFFFFFFF3)/* support 4 lanes */

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {0x00000220, SMEM_PER_LANE_MASK_CNS, smemChtActiveReadConst, 0xc ,NULL,0},
    {0x00000080, SMEM_PER_LANE_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwTxRxTraining,0},

    {0x20000 + 0xA224 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0xA358 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0x22F8 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum2,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
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

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020000, 0x00020080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020200, 0x00020274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020400, 0x0002049C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020800, 0x0002089C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00021000, 0x0002103C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022000, 0x00022034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002203C, 0x00022058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022060, 0x00022088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022100, 0x00022150)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022158, 0x00022158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022160, 0x00022174)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022200, 0x0002220C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022230, 0x0002231C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022400, 0x00022424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022430, 0x000224C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000224D0, 0x00022520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022530, 0x00022530)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022540, 0x00022554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022560, 0x00022580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000225F0, 0x000225F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022600, 0x00022618)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022620, 0x00022624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022630, 0x00022648)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x0002407C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024200, 0x0002425C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00028200, 0x00028358)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A000, 0x0002A000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A008, 0x0002A01C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A024, 0x0002A030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A200, 0x0002A244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A2DC, 0x0002A2F8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A300, 0x0002A340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A348, 0x0002A34C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A354, 0x0002A3B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A3F0, 0x0002A3FC)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E000+0x20000, 0x0000Effc+0x20000)}/*FW download memory : C112GX4_XDATA_CMN_BASE_ADDR  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000+0x20000, 0x00006ffc+0x20000)}/*FW download memory : C112GX4_XDATA_LANE_BASE_ADDR */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00010000+0x20000, 0x0001fffc+0x20000)}/*FW download memory : C28GP4X1_FW_BASE_ADDR */
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,          0x00000000,         0x00080000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000004,         0x00000038,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0000000c,         0x00030000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000014,         0x80000400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000040,         0x03a00000,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000500,         0xffff0000,      1,    0x4}

            ,{DUMMY_NAME_PTR_CNS,          0x00020004,         0x0000005c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020008,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002000c,         0x00000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020010,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002001c,         0x000000e0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020020,         0x00000083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020024,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002002c,         0x0000003c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020030,         0x00000086,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020034,         0x000000c6,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020038,         0x00000078,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002003c,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020040,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020044,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020048,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020054,         0x0000001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020058,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002005c,         0x0000000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020060,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020068,         0x00000010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002006c,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020070,         0x000000a0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020074,         0x000000e4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020078,         0x000000f0,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020200,         0x0000006c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020204,         0x0000000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020208,         0x000000f1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002020c,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020210,         0x0000000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020214,         0x00000030,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020218,         0x0000001b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002021c,         0x00000081,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020220,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020224,         0x00000017,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020228,         0x00000081,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002022c,         0x00000033,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020230,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020234,         0x0000005c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020238,         0x000000ee,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020240,         0x00000078,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020248,         0x0000007f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002024c,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020258,         0x000000e0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002025c,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020260,         0x00000090,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020264,         0x000000b0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00020268,         0x000000f0,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021000,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021004,         0x00000028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021008,         0x000000f3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002100c,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021010,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021014,         0x00000026,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021018,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021020,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00021024,         0x000000f0,      7,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022000,         0x29000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022004,         0x00100009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002201c,         0x003c0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022020,         0x20000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022024,         0x00000400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022040,         0x00000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002204c,         0x0000ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022060,         0x00001000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022064,         0x45cf0004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022068,         0x002492c9,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002206c,         0x00cc3e82,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022070,         0x05340138,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022074,         0x00b40000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002207c,         0x3001ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022080,         0x04a80000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022084,         0x0009e000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022100,         0x00304000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002211c,         0xe8000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022120,         0x741ffef0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022124,         0x33008042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022134,         0x040fea99,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022138,         0x2b9a2789,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022144,         0x10000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002214c,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022150,         0x001f0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022160,         0x2604e800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022164,         0x02024400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022168,         0x0000064f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002216c,         0x0b680000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022170,         0x00070e66,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022208,         0x80000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002220c,         0x00fffc00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022278,         0x00010001,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022290,         0x00050005,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022294,         0x00000005,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002229c,         0x0000000a,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000222dc,         0x00400080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000222e4,         0x10000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000222f8,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022300,         0x02492000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022304,         0x02003010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022400,         0x00f014f1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022404,         0x00401040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022408,         0x00800000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002240c,         0x003f1080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022410,         0x00000035,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022418,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002241c,         0x24000006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022420,         0x68222291,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022424,         0x666a6888,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022540,         0x66120083,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022544,         0x08282155,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002254c,         0x0f0b0804,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022550,         0xffd38d47,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022554,         0x0fff0047,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022610,         0x10000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022614,         0x07c00000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022630,         0x18000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002263c,         0x8c000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022640,         0x04298bbc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00022648,         0x04000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024000,         0x00410000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024008,         0x4a810000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002400c,         0x1f080601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024010,         0x00040013,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024014,         0x68140000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024018,         0x00ec0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002401c,         0x0cf80000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024020,         0x00000b68,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024024,         0x83ed0000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024028,         0x02015041,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002402c,         0x09e40962,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024030,         0x07e70a2d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024034,         0x001e09a1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024038,         0x020002c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002403c,         0x01800240,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024040,         0x00050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024044,         0x02450006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024048,         0x00070186,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002404c,         0x000003c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024050,         0x058f4b4f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024054,         0x00000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024058,         0x000013ed,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002405c,         0x09e40962,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024060,         0x07e70a2d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024064,         0x001e09a1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024068,         0x020002c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002406c,         0x01800240,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024070,         0x00050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024074,         0x02450006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024078,         0x00070186,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002407c,         0x000003c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024200,         0x00291000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024204,         0x40110000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024208,         0x00000087,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002420c,         0x162820c1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024210,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024214,         0x101f0818,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024218,         0x02021f1f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002421c,         0x0303fcfa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024220,         0x3014011e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024228,         0x00010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002422c,         0x19300000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024230,         0x08180100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024234,         0x301f101f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024238,         0x00000205,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002423c,         0x00000022,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024244,         0x00010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024248,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00024258,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002425c,         0x00001654,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026008,         0x00060000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002600c,         0xff0e0300,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026014,         0x00000c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026018,         0x00070008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026028,         0x030101f3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002602c,         0x280001f3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026030,         0xf2000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026038,         0x0c070021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026044,         0x40000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002604c,         0x3f01a200,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026050,         0x92800ffd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026058,         0x44013000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002605c,         0x00070400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026060,         0x04060400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026064,         0x3d1e4335,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026068,         0x000000fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002606c,         0x1e3d0032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026070,         0x00000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026074,         0x00001e1e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026078,         0x1e141414,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026084,         0x00000ffc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026088,         0x0000fe3c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002608c,         0xf4d80040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026090,         0x04050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026098,         0x04040000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002609c,         0x08020000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000260a0,         0x00000053,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000260a4,         0x415884c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000260b4,         0x0f0f6c9f,      5,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000260fc,         0x000000aa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026304,         0x04010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026308,         0x00010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002630c,         0x0f000f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026310,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026314,         0x01000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026318,         0x01010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002631c,         0x02080307,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026320,         0x01000404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026324,         0x05040601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026328,         0x00030203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002632c,         0x0a020132,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026330,         0x00000015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026334,         0x01000000,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002633c,         0x01010101,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026344,         0x02030101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026348,         0x0000a01b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026354,         0x03010103,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026358,         0x00010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002635c,         0x0f000b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026360,         0x01010203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026364,         0x01000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026368,         0x01010300,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002636c,         0x010b0206,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026370,         0x03000606,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026374,         0x05040601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026378,         0x00010203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002637c,         0x4d02017d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026380,         0x0100001a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026388,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002638c,         0x01010101,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026394,         0x03030101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026398,         0x0000a021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263a4,         0x02010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263a8,         0x00000303,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263ac,         0x00010001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263b0,         0x01010203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263b4,         0x01000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263b8,         0x01010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263bc,         0x02080105,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263c0,         0x01000404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263c4,         0x05040601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263c8,         0x01010203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263cc,         0xb3020089,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263d0,         0x01000015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263d4,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263d8,         0x01000100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263dc,         0x01010101,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263e4,         0x04020001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263e8,         0x0000a01b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263f4,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263f8,         0x00010303,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000263fc,         0x00010001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026400,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026404,         0x01000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026408,         0x01010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002640c,         0x02080004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026410,         0x01000404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026414,         0x05040601,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026418,         0x01010403,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002641c,         0xb3020089,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026420,         0x01000015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026424,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026428,         0x00010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002642c,         0x01010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026434,         0x07010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026438,         0x0000a01b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026444,         0x00010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026448,         0x00010606,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002644c,         0x00010001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026450,         0x02000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026454,         0x01000088,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026458,         0x01010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002645c,         0x02080400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026460,         0x01000404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026464,         0x05040600,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026468,         0x01010403,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002646c,         0xb3020089,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026470,         0x01000015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026474,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026478,         0x00010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002647c,         0x01010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026484,         0x0b000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026488,         0x0000a01b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026764,         0x20202020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026768,         0x20200000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002676c,         0x00002020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026770,         0x20202020,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026774,         0x20200000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026784,         0x20202000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002678c,         0x00202020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026794,         0x16160404,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026798,         0x04041616,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002679c,         0x16161616,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267a0,         0x01010000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267a4,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267a8,         0x31310101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267ac,         0x03033131,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267b0,         0x03030303,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267b8,         0x08080303,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267bc,         0x08080808,     14,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000267f4,         0x00020808,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00026800,         0x00040004,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028200,         0x00000090,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028208,         0x00000060,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002820c,         0x00000099,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028210,         0x00000055,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028214,         0x0000006b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028218,         0x0000001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002821c,         0x00000028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028220,         0x00000084,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028224,         0x000000f1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028228,         0x00000071,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002822c,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028230,         0x00000099,      8,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028250,         0x00000058,      8,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028270,         0x00000099,      6,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028288,         0x000000cc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002828c,         0x0000004c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028290,         0x00000068,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028294,         0x00000070,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028298,         0x0000006c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002829c,         0x0000006e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282a0,         0x00000010,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282a4,         0x00000018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282ac,         0x000000f1,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282b0,         0x00000068,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282b4,         0x000000fb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282bc,         0x00000072,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282c4,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282c8,         0x0000001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282cc,         0x00000076,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282d0,         0x000000a9,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282d4,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282d8,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282dc,         0x00000046,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282e0,         0x00000092,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282e8,         0x00000038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282ec,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282f0,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282f4,         0x00000028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x000282f8,         0x000000f2,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028304,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028310,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028314,         0x00000028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028318,         0x00000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002831c,         0x000000c4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028320,         0x000000c0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028328,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028330,         0x00000040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028334,         0x000000df,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028338,         0x000000f0,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028344,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x00028348,         0x00000008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002834c,         0x000000f0,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a008,         0x40980000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a00c,         0x05cf0000,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a010,         0x40006400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a01c,         0x58000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a024,         0x20cc3e80,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a02c,         0x002049c2,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a030,         0x001804d4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a21c,         0x00002a80,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a220,         0x00000555,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a224,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a22c,         0x00000004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a308,         0x0000ff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a314,         0x0c040324,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a318,         0x00001325,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a31c,         0x02000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a320,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a328,         0x00000800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a32c,         0x0000004f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a330,         0x08007080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a334,         0x0080001f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a338,         0x00000200,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a340,         0x00310a14,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a358,         0xffffffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a364,         0x00000400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a36c,         0xff800000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a388,         0x80000014,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a398,         0x00010001,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a3f8,         0x43004f10,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002a3fc,         0x40100000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e000,         0x00400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e004,         0x00010014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e008,         0x000a0028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e00c,         0x04010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e010,         0xda000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e014,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e018,         0x000009af,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e020,         0x00840100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e024,         0x00020015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e028,         0x000a0029,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e02c,         0x0401000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e030,         0xb6000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e034,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e038,         0x000009fb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e040,         0x004e0203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e044,         0x00010018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e048,         0x000c0031,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e04c,         0x06010008,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e050,         0xf9000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e054,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e058,         0x00000bcd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e060,         0x00500303,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e064,         0x01010019,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e068,         0x000d0032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e06c,         0x06010006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e070,         0xda000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e074,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e078,         0x00000c1b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e080,         0x00a50403,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e084,         0x0102001a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e088,         0x000d0034,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e08c,         0x06010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e090,         0xc9000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e094,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e098,         0x00000c7b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a0,         0x00b00504,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a4,         0x0102001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a8,         0x000e0037,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0ac,         0x06010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b0,         0xb6000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b4,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b8,         0x00000d51,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c0,         0x00b40604,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c4,         0x0102001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c8,         0x000e0038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0cc,         0x06010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d0,         0xe3000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d4,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d8,         0x00000d9d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e0,         0x00aa0703,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e4,         0x0102001b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e8,         0x000d0035,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0ec,         0x0601000a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f0,         0xb9000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f4,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f8,         0x00000cdb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e100,         0x00200000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e104,         0x00020014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e108,         0x000a0028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e10c,         0x0401000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e110,         0xda000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e114,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e118,         0x000009af,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e120,         0x00210103,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e124,         0x00020015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e128,         0x000a0029,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e12c,         0x0401000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e130,         0xb6000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e134,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e138,         0x000009fb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e140,         0x00270203,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e144,         0x00020018,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e148,         0x000c0031,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e14c,         0x0601000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e150,         0xf9000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e154,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e158,         0x00000bcd,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e160,         0x00140303,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e164,         0x01010019,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e168,         0x000d0032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e16c,         0x06010006,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e170,         0xda000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e174,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e178,         0x00000c1b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e180,         0x003e0403,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e184,         0x0103001a,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e188,         0x000d0034,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e18c,         0x0601000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e190,         0xc9004201,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e194,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e198,         0x00000c7b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a0,         0x002c0504,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a4,         0x0102001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a8,         0x000e0037,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1ac,         0x06010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b0,         0xb6000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b4,         0x02000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b8,         0x00000d51,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c0,         0x002d0604,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c4,         0x0102001c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c8,         0x000e0038,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1cc,         0x06010009,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d0,         0xe3000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d4,         0x02000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d8,         0x00000d9d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e0,         0x00400703,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e4,         0x0103001b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e8,         0x000d0035,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1ec,         0x0601000c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f0,         0xb9008101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f4,         0x03000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f8,         0x00000cdb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e200,         0x01010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e204,         0x00400040,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e208,         0x2800fb0c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e20c,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e210,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e218,         0x0009af00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e224,         0x01010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e228,         0x00420042,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e22c,         0x2900fd0c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e230,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e234,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e23c,         0x0009fb00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e248,         0x01010102,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e24c,         0x004e004e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e250,         0x3100fa0e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e254,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e258,         0x00020002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e260,         0x000bcd00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e26c,         0x01010103,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e270,         0x00500050,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e274,         0x3200fb0e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e278,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e27c,         0x00020002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e284,         0x000c1b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e290,         0x01010100,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e294,         0x00100010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e298,         0x2800fb0d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e29c,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2a0,         0x00020003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2a8,         0x0009af03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2b4,         0x02010101,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2b8,         0x00210021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2bc,         0x2900fd0e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2c0,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2c4,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2cc,         0x0009fb03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2d8,         0x02010102,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2dc,         0x00270027,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e0,         0x3100fa0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e4,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e8,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2f0,         0x000bcd03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2fc,         0x01010103,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e300,         0x00140014,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e304,         0x3200fb0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e308,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e30c,         0x00010003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e314,         0x000c1b03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e320,         0x01000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e324,         0x00280028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e328,         0x2800fa0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e32c,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e330,         0x00010001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e338,         0x0009af00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e344,         0x04000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e348,         0x00a500a5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e34c,         0x2900fc0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e350,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e354,         0x00040003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e35c,         0x0009fb00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e368,         0x04000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e36c,         0x00c300c3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e370,         0x3100f90f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e374,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e378,         0x00040003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e380,         0x000bcd00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e38c,         0x01000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e390,         0x00320032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e394,         0x3200fa0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e398,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e39c,         0x00010001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3a4,         0x000c1b00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b0,         0x04000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b4,         0x00280028,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b8,         0x2800fa0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3bc,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3c0,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3c8,         0x0009af03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3d4,         0x04000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3d8,         0x00290029,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3dc,         0x2900fc0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e0,         0x00020500,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e4,         0x39040003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e8,         0x00033903,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3ec,         0x0009fb03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3f8,         0x04000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3fc,         0x00310031,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e400,         0x3100f90f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e404,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e408,         0xa8030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e40c,         0x0000a800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e410,         0x000bcd03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e41c,         0x04000003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e420,         0x00320032,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e424,         0x3200fa0f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e428,         0x01010700,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e42c,         0x00030003,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e434,         0x000c1b03,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c0,         0x1f1f1f00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c4,         0x20212021,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c8,         0x01011010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5cc,         0x20202020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5d8,         0x04000400,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e604,         0x00000080,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e608,         0x008040fc,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e60c,         0x03fee000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e610,         0x000a14a0,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e614,         0x0e0f0c00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e618,         0x03050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e620,         0x06131015,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e624,         0x06050000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e628,         0x14000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e634,         0x20202020,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e644,         0x20041000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e648,         0x09090000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e654,         0xe4120000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e658,         0xc43c0010,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e65c,         0x00000190,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e660,         0x4b6400fa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e664,         0x7d4b01f4,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e668,         0x101900fa,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e66c,         0x09090909,      6,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e684,         0x800b0007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e688,         0x80060004,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e68c,         0x80000000,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e690,         0x800b0002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6ac,         0x12640400,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6dc,         0x0000aa00,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6ec,         0x000000aa,      1,    0x4 }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


/**
* @internal smemPhoenixUnitSerdes_x1 function
* @endinternal
*
* @brief   Allocate address type specific memories - SERDESes
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitSerdes_x1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {0x00000220, SMEM_FULL_MASK_CNS, smemChtActiveReadConst, 0xc ,NULL,0},
    {0x00000080, SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwTxRxTraining,0},

    {0x20000 + 0xA224 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0xA358 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum1,0},
    {0x20000 + 0x22F8 , SMEM_FULL_MASK_CNS, NULL, 0 ,smemHawkActiveWriteToSdwExpectedChecksum2,0},



    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
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

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020000, 0x00020008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020010, 0x00020080)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020200, 0x00020274)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020400, 0x0002049C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00020800, 0x0002089C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00021000, 0x0002103C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022000, 0x00022034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002203C, 0x00022058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022060, 0x00022088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022100, 0x00022150)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022158, 0x00022158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022160, 0x00022174)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022200, 0x0002220C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022230, 0x0002231C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022400, 0x00022424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022430, 0x000224C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000224D0, 0x00022520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022530, 0x00022530)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022540, 0x00022554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022560, 0x00022580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000225F0, 0x000225F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022600, 0x00022618)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022620, 0x00022624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00022630, 0x00022648)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x0002407C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024200, 0x0002425C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00028200, 0x00028358)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A000, 0x0002A000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A008, 0x0002A01C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A024, 0x0002A030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A200, 0x0002A244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A2DC, 0x0002A2F8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A300, 0x0002A340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A348, 0x0002A34C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A354, 0x0002A3B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0002A3F0, 0x0002A3FC)}

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
             {DUMMY_NAME_PTR_CNS,          0x00000000,         0x00080000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000004,         0x00000038,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0000000c,         0x00030000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000014,         0x80000400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000040,         0x03a00000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00000500,         0xffff0000,      1,    0x4}

            ,{DUMMY_NAME_PTR_CNS,          0x00020000,         0x0000000f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020004,         0x0000005c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020008,         0x00000040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020010,         0x00000010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002001c,         0x000000e0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020020,         0x00000083,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020024,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002002c,         0x0000003c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020030,         0x00000086,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020034,         0x000000c6,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020038,         0x00000078,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002003c,         0x00000022,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020040,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020044,         0x00000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020048,         0x000000fc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020054,         0x0000001f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020058,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002005c,         0x0000000f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020060,         0x000000fc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020068,         0x00000010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002006c,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020070,         0x000000a0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020074,         0x000000e4,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020078,         0x000000f0,      3,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020200,         0x0000006c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020204,         0x0000000f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020208,         0x000000f1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002020c,         0x000000c4,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020210,         0x0000000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020214,         0x00000030,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020218,         0x0000001b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002021c,         0x00000081,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020220,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020224,         0x00000017,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020228,         0x00000081,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002022c,         0x00000033,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020230,         0x00000040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020234,         0x0000005c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020238,         0x000000ee,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020240,         0x00000078,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020248,         0x0000007f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002024c,         0x00000040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020258,         0x000000e0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002025c,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020260,         0x00000090,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020264,         0x000000b0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00020268,         0x000000f0,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021000,         0x00000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021004,         0x00000028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021008,         0x000000f3,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002100c,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021010,         0x00000070,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021014,         0x00000026,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021018,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021020,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00021024,         0x000000f0,      7,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022000,         0x29000800,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022004,         0x00100009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002201c,         0x003c0000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022020,         0x20000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022024,         0x00000400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022040,         0x00000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002204c,         0x0000ff00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022060,         0x00001000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022064,         0x45cf0004,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022068,         0x002492c9,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002206c,         0x00cc3e82,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022070,         0x05340138,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022074,         0x00b40000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002207c,         0x3001ff00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022080,         0x04a80000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022084,         0x0009e000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022100,         0x00304000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002211c,         0xe8000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022120,         0x741ffef0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022124,         0x33008042,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022134,         0x040fea99,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022138,         0x2b9a2789,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022144,         0x10000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002214c,         0x00000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022150,         0x001f0000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022160,         0x2604e800,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022164,         0x02024400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022168,         0x0000064f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002216c,         0x0b680000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022170,         0x00070e66,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022208,         0x80000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002220c,         0x00fffc00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022278,         0x00010001,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022290,         0x00050005,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022294,         0x00000005,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002229c,         0x0000000a,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000222dc,         0x00400080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000222e4,         0x10000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000222f8,         0xffffffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022300,         0x02492000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022304,         0x02003010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022400,         0x00f014f1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022404,         0x00401040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022408,         0x00800000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002240c,         0x003f1080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022410,         0x00000035,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022418,         0x00000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002241c,         0x24000006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022420,         0x68222291,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022424,         0x666a6888,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022540,         0x66120083,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022544,         0x08282155,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002254c,         0x0f0b0804,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022550,         0xffd38d47,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022554,         0x0fff0047,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022610,         0x10000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022614,         0x07c00000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022630,         0x18000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002263c,         0x8c000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022640,         0x04298bbc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00022648,         0x04000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024000,         0x00410000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024008,         0x4a810000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002400c,         0x1f080601,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024010,         0x00040013,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024014,         0x68140000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024018,         0x00ec0000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002401c,         0x0cf80000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024020,         0x00000b68,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024024,         0x83ed0000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024028,         0x02015041,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002402c,         0x09e40962,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024030,         0x07e70a2d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024034,         0x001e09a1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024038,         0x020002c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002403c,         0x01800240,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024040,         0x00050000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024044,         0x02450006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024048,         0x00070186,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002404c,         0x000003c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024050,         0x058f4b4f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024054,         0x00000100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024058,         0x000013ed,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002405c,         0x09e40962,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024060,         0x07e70a2d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024064,         0x001e09a1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024068,         0x020002c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002406c,         0x01800240,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024070,         0x00050000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024074,         0x02450006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024078,         0x00070186,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002407c,         0x000003c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024200,         0x00291000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024204,         0x40110000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024208,         0x00000087,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002420c,         0x162820c1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024210,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024214,         0x101f0818,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024218,         0x02021f1f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002421c,         0x0303fcfa,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024220,         0x3014011e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024228,         0x00010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002422c,         0x19300000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024230,         0x08180100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024234,         0x301f101f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024238,         0x00000205,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002423c,         0x00000022,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024244,         0x00010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024248,         0xffffffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00024258,         0x00000020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002425c,         0x00001654,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026008,         0x00060000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002600c,         0xff0e0300,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026014,         0x00000c00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026018,         0x00070008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026028,         0x030101f3,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002602c,         0x280001f3,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026030,         0xf2000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026038,         0x0c070021,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026044,         0x40000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002604c,         0x3f01a200,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026050,         0x92800ffd,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026058,         0x44013000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002605c,         0x00070400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026060,         0x04060400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026064,         0x3d1e4335,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026068,         0x000000fc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002606c,         0x1e3d0032,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026070,         0x00000003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026074,         0x00001e1e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026078,         0x1e141414,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026084,         0x00000ffc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026088,         0x0000fe3c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002608c,         0xf4d80040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026090,         0x04050000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026098,         0x04040000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002609c,         0x08020000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000260a0,         0x00000053,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000260a4,         0x415884c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000260b4,         0x0f0f6c9f,      5,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000260fc,         0x000000aa,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026304,         0x04010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026308,         0x00010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002630c,         0x0f000f00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026310,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026314,         0x01000088,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026318,         0x01010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002631c,         0x02080307,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026320,         0x01000404,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026324,         0x05040601,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026328,         0x00030203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002632c,         0x0a020132,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026330,         0x00000015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026334,         0x01000000,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002633c,         0x01010101,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026344,         0x02030101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026348,         0x0000a01b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026354,         0x03010103,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026358,         0x00010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002635c,         0x0f000b00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026360,         0x01010203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026364,         0x01000088,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026368,         0x01010300,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002636c,         0x010b0206,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026370,         0x03000606,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026374,         0x05040601,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026378,         0x00010203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002637c,         0x4d02017d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026380,         0x0100001a,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026388,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002638c,         0x01010101,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026394,         0x03030101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026398,         0x0000a021,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263a4,         0x02010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263a8,         0x00000303,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263ac,         0x00010001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263b0,         0x01010203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263b4,         0x01000088,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263b8,         0x01010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263bc,         0x02080105,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263c0,         0x01000404,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263c4,         0x05040601,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263c8,         0x01010203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263cc,         0xb3020089,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263d0,         0x01000015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263d4,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263d8,         0x01000100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263dc,         0x01010101,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263e4,         0x04020001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263e8,         0x0000a01b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263f4,         0x01010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263f8,         0x00010303,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000263fc,         0x00010001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026400,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026404,         0x01000088,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026408,         0x01010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002640c,         0x02080004,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026410,         0x01000404,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026414,         0x05040601,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026418,         0x01010403,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002641c,         0xb3020089,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026420,         0x01000015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026424,         0x01010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026428,         0x00010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002642c,         0x01010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026434,         0x07010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026438,         0x0000a01b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026444,         0x00010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026448,         0x00010606,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002644c,         0x00010001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026450,         0x02000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026454,         0x01000088,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026458,         0x01010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002645c,         0x02080400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026460,         0x01000404,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026464,         0x05040600,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026468,         0x01010403,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002646c,         0xb3020089,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026470,         0x01000015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026474,         0x01010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026478,         0x00010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002647c,         0x01010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026484,         0x0b000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026488,         0x0000a01b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026764,         0x20202020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026768,         0x20200000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002676c,         0x00002020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026770,         0x20202020,      2,    0x8}
            ,{DUMMY_NAME_PTR_CNS,          0x00026774,         0x20200000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002677c,         0x20202020,      2,    0xc}
            ,{DUMMY_NAME_PTR_CNS,          0x00026784,         0x20202000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002678c,         0x00202020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026794,         0x16160404,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026798,         0x04041616,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002679c,         0x16161616,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267a0,         0x01010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267a4,         0x01010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267a8,         0x31310101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267ac,         0x03033131,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267b0,         0x03030303,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267b8,         0x08080303,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267bc,         0x08080808,     14,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000267f4,         0x00020808,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00026800,         0x00040004,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028200,         0x00000090,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028208,         0x00000060,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002820c,         0x00000099,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028210,         0x00000055,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028214,         0x0000006b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028218,         0x0000001f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002821c,         0x00000028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028220,         0x00000084,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028224,         0x000000f1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028228,         0x00000071,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002822c,         0x0000000e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028230,         0x00000099,      8,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028250,         0x00000058,      8,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028270,         0x00000099,      6,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028288,         0x000000cc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002828c,         0x0000004c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028290,         0x00000068,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028294,         0x00000070,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028298,         0x0000006c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002829c,         0x0000006e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282a0,         0x00000010,      2,    0x8}
            ,{DUMMY_NAME_PTR_CNS,          0x000282a4,         0x00000018,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282ac,         0x000000f1,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282b0,         0x00000068,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282b4,         0x000000fb,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282bc,         0x00000072,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282c4,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282c8,         0x0000001c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282cc,         0x00000076,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282d0,         0x000000a9,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282d4,         0x00000020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282d8,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282dc,         0x00000046,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282e0,         0x00000092,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282e8,         0x00000038,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282ec,         0x0000000e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282f0,         0x00000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282f4,         0x00000028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282f8,         0x000000f2,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x000282fc,         0x00000080,      2,    0xc}
            ,{DUMMY_NAME_PTR_CNS,          0x00028304,         0x00000020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028310,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028314,         0x00000028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028318,         0x00000003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002831c,         0x000000c4,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028320,         0x000000c0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028328,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028330,         0x00000040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028334,         0x000000df,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028338,         0x000000f0,      3,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028344,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x00028348,         0x00000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002834c,         0x000000f0,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a008,         0x40980000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a00c,         0x05cf0000,      2,    0x8}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a010,         0x40006400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a01c,         0x58000800,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a024,         0x20cc3e80,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a02c,         0x002049c2,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a030,         0x001804d4,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a21c,         0x00002a80,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a220,         0x00000555,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a224,         0xffffffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a22c,         0x00000004,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a308,         0x0000ff00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a314,         0x0c040324,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a318,         0x00001325,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a31c,         0x02000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a320,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a328,         0x00000800,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a32c,         0x0000004f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a330,         0x08007080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a334,         0x0080001f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a338,         0x00000200,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a340,         0x00310a14,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a358,         0xffffffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a364,         0x00000400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a36c,         0xff800000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a388,         0x80000014,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a398,         0x00010001,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a3f8,         0x43004f10,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002a3fc,         0x40100000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e000,         0x00400000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e004,         0x00010014,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e008,         0x000a0028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e00c,         0x04010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e010,         0xda000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e014,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e018,         0x000009af,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e020,         0x00840100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e024,         0x00020015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e028,         0x000a0029,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e02c,         0x0401000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e030,         0xb6000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e034,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e038,         0x000009fb,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e040,         0x004e0203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e044,         0x00010018,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e048,         0x000c0031,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e04c,         0x06010008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e050,         0xf9000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e054,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e058,         0x00000bcd,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e060,         0x00500303,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e064,         0x01010019,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e068,         0x000d0032,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e06c,         0x06010006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e070,         0xda000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e074,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e078,         0x00000c1b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e080,         0x00a50403,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e084,         0x0102001a,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e088,         0x000d0034,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e08c,         0x06010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e090,         0xc9000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e094,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e098,         0x00000c7b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a0,         0x00b00504,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a4,         0x0102001c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0a8,         0x000e0037,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0ac,         0x06010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b0,         0xb6000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b4,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0b8,         0x00000d51,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c0,         0x00b40604,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c4,         0x0102001c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0c8,         0x000e0038,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0cc,         0x06010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d0,         0xe3000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d4,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0d8,         0x00000d9d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e0,         0x00aa0703,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e4,         0x0102001b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0e8,         0x000d0035,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0ec,         0x0601000a,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f0,         0xb9000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f4,         0x00000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e0f8,         0x00000cdb,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e100,         0x00200000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e104,         0x00020014,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e108,         0x000a0028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e10c,         0x0401000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e110,         0xda000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e114,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e118,         0x000009af,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e120,         0x00210103,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e124,         0x00020015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e128,         0x000a0029,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e12c,         0x0401000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e130,         0xb6000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e134,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e138,         0x000009fb,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e140,         0x00270203,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e144,         0x00020018,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e148,         0x000c0031,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e14c,         0x0601000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e150,         0xf9000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e154,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e158,         0x00000bcd,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e160,         0x00140303,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e164,         0x01010019,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e168,         0x000d0032,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e16c,         0x06010006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e170,         0xda000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e174,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e178,         0x00000c1b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e180,         0x003e0403,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e184,         0x0103001a,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e188,         0x000d0034,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e18c,         0x0601000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e190,         0xc9004201,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e194,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e198,         0x00000c7b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a0,         0x002c0504,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a4,         0x0102001c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1a8,         0x000e0037,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1ac,         0x06010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b0,         0xb6000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b4,         0x02000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1b8,         0x00000d51,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c0,         0x002d0604,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c4,         0x0102001c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1c8,         0x000e0038,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1cc,         0x06010009,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d0,         0xe3000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d4,         0x02000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1d8,         0x00000d9d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e0,         0x00400703,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e4,         0x0103001b,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1e8,         0x000d0035,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1ec,         0x0601000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f0,         0xb9008101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f4,         0x03000007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e1f8,         0x00000cdb,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e200,         0x01010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e204,         0x00400040,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e208,         0x2800fb0c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e20c,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e210,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e218,         0x0009af00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e224,         0x01010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e228,         0x00420042,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e22c,         0x2900fd0c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e230,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e234,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e23c,         0x0009fb00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e248,         0x01010102,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e24c,         0x004e004e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e250,         0x3100fa0e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e254,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e258,         0x00020002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e260,         0x000bcd00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e26c,         0x01010103,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e270,         0x00500050,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e274,         0x3200fb0e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e278,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e27c,         0x00020002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e284,         0x000c1b00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e290,         0x01010100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e294,         0x00100010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e298,         0x2800fb0d,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e29c,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2a0,         0x00020003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2a8,         0x0009af03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2b4,         0x02010101,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2b8,         0x00210021,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2bc,         0x2900fd0e,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2c0,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2c4,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2cc,         0x0009fb03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2d8,         0x02010102,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2dc,         0x00270027,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e0,         0x3100fa0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e4,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2e8,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2f0,         0x000bcd03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e2fc,         0x01010103,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e300,         0x00140014,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e304,         0x3200fb0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e308,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e30c,         0x00010003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e314,         0x000c1b03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e320,         0x01000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e324,         0x00280028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e328,         0x2800fa0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e32c,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e330,         0x00010001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e338,         0x0009af00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e344,         0x04000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e348,         0x00a500a5,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e34c,         0x2900fc0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e350,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e354,         0x00040003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e35c,         0x0009fb00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e368,         0x04000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e36c,         0x00c300c3,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e370,         0x3100f90f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e374,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e378,         0x00040003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e380,         0x000bcd00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e38c,         0x01000003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e390,         0x00320032,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e394,         0x3200fa0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e398,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e39c,         0x00010001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3a4,         0x000c1b00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b0,         0x04000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b4,         0x00280028,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3b8,         0x2800fa0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3bc,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3c0,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3c8,         0x0009af03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3d4,         0x04000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3d8,         0x00290029,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3dc,         0x2900fc0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e0,         0x00020500,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e4,         0x39040003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3e8,         0x00033903,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3ec,         0x0009fb03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3f8,         0x04000002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e3fc,         0x00310031,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e400,         0x3100f90f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e404,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e408,         0xa8030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e40c,         0x0000a800,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e410,         0x000bcd03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e41c,         0x04000003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e420,         0x00320032,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e424,         0x3200fa0f,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e428,         0x01010700,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e42c,         0x00030003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e434,         0x000c1b03,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c0,         0x1f1f1f00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c4,         0x20212021,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5c8,         0x01011010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5cc,         0x20202020,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e5d8,         0x04000400,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e604,         0x00000080,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e608,         0x008040fc,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e60c,         0x03fee000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e610,         0x000a14a0,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e614,         0x0e0f0c00,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e618,         0x03050000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e620,         0x06131015,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e624,         0x06050000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e628,         0x14000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e634,         0x20202020,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e644,         0x20041000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e648,         0x09090000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e654,         0xe4120000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e658,         0xc43c0010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e65c,         0x00000190,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e660,         0x4b6400fa,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e664,         0x7d4b01f4,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e668,         0x101900fa,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e66c,         0x09090909,      6,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e684,         0x800b0007,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e688,         0x80060004,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e68c,         0x80000000,      2,    0x8}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e690,         0x800b0002,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6ac,         0x12640400,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6dc,         0x0000aa00,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,          0x0002e6ec,         0x000000aa,      1,    0x4}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitTxqQfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq qfc unit - not generated from Cider , but to support 'all' memory space. till cpss be aligned.
*/
static void smemPhoenixUnitTxqQfc
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
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 8 + 4*(NUM_PORTS_PER_DP_UNIT+1))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000013C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000360)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000420)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000424, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000050c, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000638)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000814)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x000017FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x0000187C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001984)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 8*(4*(NUM_PORTS_PER_DP_UNIT+1)))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x0000331C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003400)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003500, 4*(NUM_PORTS_PER_DP_UNIT+1))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600, 0x0000361C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 4 + 4*(NUM_PORTS_PER_DP_UNIT+1))}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100, 0x00004108)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            8+4*(NUM_PORTS_PER_DP_UNIT+1),
            8+4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            8*4*(NUM_PORTS_PER_DP_UNIT+1),
            8*4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            4+4*(NUM_PORTS_PER_DP_UNIT+1),
            4+4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        UPDATE_MEM_CHUNK_FROM_SIZE_TO_SIZE (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }


    /*simulation defaults*/
    {
      static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x4    }/*qfc_metal_fix*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000004,         0x30000000,      1,    0x4    }/*Global_PFC_conf*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000608,         0x0000007f,      1,    0x4    }/*Uburst_Event_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000610,         0x00000078,      1,    0x4    }/*Xoff_Size_Indirect_Read_Access*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000800,         0x00020000,      1,    0x0    }/*manual entry in order to get 0 */
            ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000078,      1,    0x4    }/*HR_Counters_Indirect_Read_Access*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004004,         0x0001ffff,      (NUM_PORTS_PER_DP_UNIT+1),    0x4    }/*Port_HR_Counters_Threshold*/
            ,{DUMMY_NAME_PTR_CNS,           0x00004108,         0x0001ffff,      1,    0x4    }/*HR_Counter_Min_Peak*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        UPDATE_LIST_REGISTER_DEFAULT_SIZE_TO_SIZE(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);
    }

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Data    = 0x00000808;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC[0].Counter_Table_1_Indirect_Read_Address = 0x0000080C;

}
/**
* @internal smemPhoenixUnitMif function
* @endinternal
*
* @brief   Allocate address type specific memories - for the MIF
*/
static void smemPhoenixUnitMif
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
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_RX_OFFSET + 0x000000A0, MIF_RX_OFFSET + 0x000000DC)}}
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
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000000A0, MIF_TX_OFFSET + 0x000000DC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x000000E0, MIF_TX_OFFSET + 0x000000E4)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000100, MIF_TX_OFFSET + 0x0000012C)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000200, MIF_TX_OFFSET + 0x00000260)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (MIF_TX_OFFSET + 0x00000290, MIF_TX_OFFSET + 0x000002AC)}}
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
            ,{DUMMY_NAME_PTR_CNS,        MIF_RX_OFFSET     + 0x000000A0,         0x00000020,      16,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_RX_OFFSET     + 0x000000E0,         0x00000020,      2,    0x4            }

            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000020,         0x00000020,      25,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000000A0,         0x00000020,      16,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000000E0,         0x00000020,      2,    0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000700,         0x00000005,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000710,         0x00000008,      25,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x00000790,         0x00000008,      16,   0x4            }
            ,{DUMMY_NAME_PTR_CNS,        MIF_TX_OFFSET     + 0x000007D0,         0x00000008,      2,    0x4            }

            ,{NULL,                      0,                                      0x00000000,      0,    0x0            }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitAnp function
* @endinternal
*
* @brief   Allocate address type specific memories - for the Anp
*/
static void smemPhoenixUnitAnp
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
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

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* ANP - start */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ANP_OFFSET + 0x00000000, ANP_OFFSET + 0x000000B8)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (ANP_OFFSET + 0x00000100, ANP_OFFSET + 0x000003a0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, ANP_STEP_PORT_OFFSET)}
             /* ANP - end */

            /* AN - start */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000000, AN_OFFSET + 0x0000000c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000014, AN_OFFSET + 0x00000018)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000038, AN_OFFSET + 0x0000006c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x000000c0, AN_OFFSET + 0x000000c4)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000800, AN_OFFSET + 0x0000086c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (AN_OFFSET + 0x00000880, AN_OFFSET + 0x0000088c)}, FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_100G_UNIT, AN_STEP_PORT_OFFSET)}
             /* AN - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000000,         0x00000246,      1,    0x0            }

            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000004,         0x00000003,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000008,         0x03020100,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x0000000c,         0x07060504,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000070,         0x0000008a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000074,         0x0b41002b,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000078,         0x0000002a,      1,    0x0            }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000120,         0x009c009c,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000124,         0x003000cc,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x00000128,         0x00000001,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001bc,         0x0000000e,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001c4,         0x05004000,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001c8,         0x00000020,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001cc,         0x00040000,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001d0,         0x00022810,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001d4,         0x0f00c000,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001d8,         0x10000000,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001dc,         0x00000818,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001e4,         0xf3f10000,      4,    0x2a4          }
            ,{DUMMY_NAME_PTR_CNS,        ANP_OFFSET     + 0x000001e8,         0x00000002,      4,    0x2a4          }

            ,{NULL,                      0,                                   0x00000000,      0,    0x0            }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemPhoenixUsxMac function
* @endinternal
*
* @brief   Allocate address type specific memories - for the USX MAC
*/
static void smemPhoenixUsxMac
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
        {MAC_MIB_OFFSET + 0x00000020, 0xFFFFFFE0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000040, 0xFFFFFFC0, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        {MAC_MIB_OFFSET + 0x00000080, 0xFFFFFF80, smemFalconActiveReadMtiStatisticCounters, 0, NULL , 0},
        /* Statistic counters - active memory write control */
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_STATS.control */
        {MAC_MIB_OFFSET + 0x0000000C, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWriteMtiStatisticControl, 0},


        /* MIB - END */

        /*0x00000094 + 0x18*portIndex + unitOffset*/
        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause */
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*1, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*2, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*3, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*4, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*5, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*6, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x00000038 + 0x28*7, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},


        /* SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask */
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*0, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*1, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*2, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*3, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*4, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*5, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*6, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},
        {MAC_EXT_BASE_OFFSET + 0x0000003c + 0x28*7, SMEM_FULL_MASK_CNS, NULL, 0, smemFalconActiveWritePortInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* MIB - start */
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x1000 + 0x00000000, 0x1000 + 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x1000 + 0x0000001C, 0x1000 + 0x00000A9C)}
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
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x2000 + 0x00000000, 0x2000 + 0x00000014)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x2000 + 0x0000001C, 0x2000 + 0x00000020)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x2000 + 0x00000030, 0x2000 + 0x00000048)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT , MAC_STEP_PORT_OFFSET)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x2000 + 0x00000054, 0x2000 + 0x000000A0)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT , MAC_STEP_PORT_OFFSET)}
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
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* per MAC : ports 0..7 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000028)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000002C, 0x00000050)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT   , 0x28)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000016c, 0x0000016c)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000170, 0x0000017c)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT   , 0x10)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001f0, 0x00000200)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT/4 , 0x14)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000218, 0x00000220)} , FORMULA_SINGLE_PARAMETER(NUM_PORTS_PER_USX_UNIT/4 , 0xc)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);
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

            /*100G*/
            ,{DUMMY_NAME_PTR_CNS,            0x00002080,         0x100,      1,    0x0            ,(NUM_PORTS_PER_USX_UNIT) , (MAC_STEP_PORT_OFFSET)}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUsxPcs function
* @endinternal
*
* @brief   Allocate address type specific memories - for the USX PCS
*/
static void smemPhoenixUsxPcs
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* LPCS : 8 ports */
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 4 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 5 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 6 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_LPCS_OFFSET  + USX_LPCS_STEP_PORT_OFFSET * 7 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},

        /* PCS : 8 ports */
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 0 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 1 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 2 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 3 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 4 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 5 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 6 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},
        {USX_PCS_PORTS_OFFSET  + USX_PCS_STEP_PORT_OFFSET * 7 + 0x00000000, SMEM_FULL_MASK_CNS , NULL, 0, smemFalconActiveWritePcsControl1, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
             /* PCS start */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000000, USX_PCS_PORTS_OFFSET + 0x00000024)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000038, USX_PCS_PORTS_OFFSET + 0x0000003C)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x00000080, USX_PCS_PORTS_OFFSET + 0x000000B4)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x000000C8, USX_PCS_PORTS_OFFSET + 0x000000C8)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_PCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_PORTS_OFFSET + 0x000000F0, USX_PCS_PORTS_OFFSET + 0x000000FC)} , FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_PCS_STEP_PORT_OFFSET))}
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
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x00000000, USX_PCS_LPCS_OFFSET + 0x00000020)}, FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_LPCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x00000040, USX_PCS_LPCS_OFFSET + 0x0000005C)}, FORMULA_SINGLE_PARAMETER((NUM_PORTS_PER_USX_UNIT ) , (USX_LPCS_STEP_PORT_OFFSET))}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x000003E0, USX_PCS_LPCS_OFFSET + 0x000003EC)}}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (USX_PCS_LPCS_OFFSET + 0x000003F0, USX_PCS_LPCS_OFFSET + 0x000003F4)}}
             /* PCS-LPCS - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /* usx lpcs */
              {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00001140,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000009,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00004950,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00004d54,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000010,         0x000001a0,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x00000004,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000001,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x000012d0,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x0000004c,         0x00000013,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             ,{DUMMY_NAME_PTR_CNS,            0x0000005c,         0x0000000c,      NUM_PORTS_PER_USX_UNIT,    USX_LPCS_STEP_PORT_OFFSET}
             /* usx pcs */
             ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00002040,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001010,         0x00000017,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001014,         0x00000008,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x00008081,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001088,         0x0000804f,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x0000108c,         0x0000cab6,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001090,         0x0000b44d,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001094,         0x000003c8,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x00001098,         0x00008884,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x0000109c,         0x000085a3,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010a0,         0x000006bb,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010a4,         0x00000349,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010b4,         0x00008000,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010f4,         0x00004901,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010f8,         0x00000303,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             ,{DUMMY_NAME_PTR_CNS,            0x000010fc,         0x00000007,      NUM_PORTS_PER_USX_UNIT,    USX_PCS_STEP_PORT_OFFSET }
             /* usx - rsfec */
             ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x00008003,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x0000320c,         0x00000140,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00003228,         0x00000026,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x0000322c,         0x00000008,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00003300,         0x00000003,      1,    0x0}
             /* usx usxm */
             ,{DUMMY_NAME_PTR_CNS,            0x0000200c,         0x00001004,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002040,         0x000068c1,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002044,         0x00000021,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002048,         0x0000c4f0,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x0000204c,         0x000000e6,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002050,         0x000065c5,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002054,         0x0000009b,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x00002058,         0x000079a2,      1,    0x0}
             ,{DUMMY_NAME_PTR_CNS,            0x0000205c,         0x0000003d,      1,    0x0}


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}



/**
* @internal smemPhoenixUnitPcaCtsu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA CTSU unit
*/
static void smemPhoenixUnitPcaCtsu
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        PHOENIX_TSU_PER_GROUP_ACTIVE_MEM(0),
        PHOENIX_TSU_PER_GROUP_ACTIVE_MEM(1),
        PHOENIX_TSU_PER_GROUP_ACTIVE_MEM(2),
        PHOENIX_TSU_PER_GROUP_ACTIVE_MEM(3),
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        /* aligned to /Cider/EBU-IP/PCA/CTSU IP/CTSU 1.0/CTSU {1.0.9} */
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
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
* @internal smemPhoenixUnitPcaBrg function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Bridge
*/
static void smemPhoenixUnitPcaBrg
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*aligned to /Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}*/
    /*IP: \Cider \EBU-IP \PCA \BRG IP \BRG 1.0 \BRG {1.0.19} \BRG \dp2sdb \BRG*/
    /*Mask: \Cider \EBU-IP \PCA \BRG IP \BRG 1.0 \BRG {Phoenix1 64-CH} {1.0.19}*/

    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /* Global */
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
        /* Rx Channel */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x000001FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000023C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x000003BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x0000047C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000043C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000004C0, 0x000004FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000580, 0x000005BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000640, 0x0000067C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000060C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x0000062C)}
        /* Rx Global */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000754)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000780, 0x00000780)}
        /* Tx Channel */
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000008FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x000009FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x000011FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x000013FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x0000153C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001580, 0x000015BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001600, 0x0000163C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001680, 0x000016BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x0000173C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001780, 0x000017BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x0000183C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001880, 0x000018BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x0000190C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001920, 0x0000192C)}
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
* @internal smemPhoenixUnitPcaArbiter function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Arbiter
*/
static void smemPhoenixUnitPcaArbiter
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /*/Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PCA/<PIZ_ARB> PizArb IP TLU*/
    /*IP: \Cider \EBU-IP \PCA \PizArb \PizArb 1.0 \PizArb {1.5.3} \PizArb IP TLU \pzarb*/
    /*Mask: \Cider \EBU-IP \PCA \PizArb \PizArb 1.0 \PizArb {1.5.3}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000006FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B10)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B14, 0x00000C10)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Pizza arbiter slot configuration */
             {DUMMY_NAME_PTR_CNS,            0x00000300,         0x0000013F,      256,    0x4      }
            /* Pizza arbiter control */
             ,{DUMMY_NAME_PTR_CNS,            0x00000B0C,         0x0000005B,      1,    0x4      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitPcaSffDefaultLLs function
* @endinternal
*
* @brief   default LLs registers values for PCA Sff
*/
static GT_U32 smemPhoenixUnitPcaSffDefaultLLs
(
    IN struct SMEM_REGISTER_DEFAULT_VALUE_STRUCT *defRegsPtr,
    IN GT_U32 repCount
)
{
    GT_UNUSED_PARAM (defRegsPtr);
    return repCount;
}

/**
* @internal smemPhoenixUnitPcaSff function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Sff
*/
static void smemPhoenixUnitPcaSff
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/EBU/AC5X/AC5X {Phoenix_TF_Rev1.2(191229)}/Phoenix/Core/PCA/<SFF> SFF IP TLU */
    /*IP: \Cider \EBU-IP \PCA \SFF IP \SFF 1.0 \SFF IP {rtlf-2.0} \SFF IP TLU */
    /*Mask: \Cider \EBU-IP \PCA \SFF IP \SFF 1.0 \SFF IP Phoenix-64 {rtlf-2.0}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /*Channel - Channel config*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x000006FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000008FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000DFC)} /* 256 entries */
        /*Chan Interrupt*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000023C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000280, 0x000002BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000033C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x000003BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x0000043C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x000004BC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000053C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000580, 0x000005BC)}
        /*Chan Statistics*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x000018FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A00, 0x00001AFC)}
        /*Chan Status*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x000012FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001400, 0x000014FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001600, 0x000016FC)}
        /*Glob Config*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
        /*Glob Interrupts*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000014)}
        /*Glob Status*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000018, 0x0000001C)}
        /*SFF Interrupt Summary*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x0000003C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x0000004C)}
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
            ,{DUMMY_NAME_PTR_CNS,            0x00000A00,         0x00000000,      256,   0x4, 0,0, 0,0, &smemPhoenixUnitPcaSffDefaultLLs}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitPcaMacsecExt function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC Extension
*/
static void smemPhoenixUnitPcaMacsecExt
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
/*
IP Mask: \Cider \EBU-IP \PCA \MACSEC EXT IP \MACSEC EXT 1.0 \MACSEC EXT IP phoenix {1.1.14}
Unit:    \Cider \EBU-IP \PCA \MACSEC EXT IP \MACSEC EXT 1.0 \MACSEC EXT IP {1.1.14} \MACSEC EXT IP TLU \Units \MacSec General

*/
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
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000358)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}

/**
* @internal smemPhoenixUnitPcaMacsecEip163 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-163 device (MACSec Classifier)
*/
static void smemPhoenixUnitPcaMacsecEip163
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr,
    IN GT_BOOL                isEgress
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000D810, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
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
            /* CHAN_block_stat_sum0 + sum1: Per channel statistics summary registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000E800, 0x0000E804)}
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
            /* Channels 32 - 63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001A000, 0x0001A7DC)}
            /* CHAN_Counters_32_63 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001E000, 0x0001E7F4)}
            /* Channel_AIC_page_1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001F400, 0x0001FBFC)}
            /* Input_adapter_page1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001FC80, 0x0001FC94)}
            /* Channel_control_page1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001FE00, 0x0001FE7C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
        {
            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArrIngr[] =
            {
                 /*                             LSBits of 'Addr',     val,                     */
                 {DUMMY_NAME_PTR_CNS,            0x0000d874,         0x01400200,      1,    0x0}   /* TCAM_COUNT_OPTIONS2 */
                ,{DUMMY_NAME_PTR_CNS,            0x0000d878,         0x40000001,      1,    0x0}   /* TCAM_COUNT_OPTIONS  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000d87c,         0x022226d9,      1,    0x0}   /* TCAM_COUNT_VERSION  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x40800010,      1,    0x0}   /* EIP163_CONFIG2      */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x48010000,      1,    0x0}   /* EIP163_CONFIG       */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02435CA3,      1,    0x0}   /* EIP163_VERSION      */
                ,{NULL,                          0,                  0x00000000,      0,    0x0}
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC listIngr = {myUnit_registersDefaultValueArrIngr,NULL};

            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArrEgr[] =
            {
                 /*                             LSBits of 'Addr',     val,                     */
                 {DUMMY_NAME_PTR_CNS,            0x0000d874,         0x01400200,      1,    0x0}   /* TCAM_COUNT_OPTIONS2 */
                ,{DUMMY_NAME_PTR_CNS,            0x0000d878,         0x40000001,      1,    0x0}   /* TCAM_COUNT_OPTIONS  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000d87c,         0x022226d9,      1,    0x0}   /* TCAM_COUNT_VERSION  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x40800010,      1,    0x0}   /* EIP163_CONFIG2      */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x88010000,      1,    0x0}   /* EIP163_CONFIG       */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02435CA3,      1,    0x0}   /* EIP163_VERSION      */
                ,{NULL,                          0,                  0x00000000,      0,    0x0}
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC listEgr = {myUnit_registersDefaultValueArrEgr,NULL};

            unitPtr->unitDefaultRegistersPtr = isEgress ? &listEgr : &listIngr;
        }
    }
}

/**
* @internal smemPhoenixUnitPcaMacsecEip163Ing function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-163 Ingress device (MACSec Classifier)
*/
static void smemPhoenixUnitPcaMacsecEip163Ing
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    smemPhoenixUnitPcaMacsecEip163(devObjPtr,unitPtr,GT_FALSE);
}

/**
* @internal smemPhoenixUnitPcaMacsecEip163Egr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-163 Egress device (MACSec Classifier)
*/
static void smemPhoenixUnitPcaMacsecEip163Egr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    smemPhoenixUnitPcaMacsecEip163(devObjPtr,unitPtr,GT_TRUE);
}


/**
* @internal smemPhoenixUnitPcaMacsecEip164 function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-164 device (MACSec Transform)
*/
static void smemPhoenixUnitPcaMacsecEip164
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr,
    IN GT_BOOL                isEgress
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        /* all counters cleared by writing 0 to enable register*/
        {0x0000E010, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},
        {0x0000E410, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},
        {0x0000E610, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},
        {0x0000E810, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},
        {0x0000EA10, 0x1FFFF  , NULL, 0, smemPhoenixActiveWriteMacSecCountersControl, 0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
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
            /* SAM_NM_PARAMS_page1: Classifier control/status 32:63 registers */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015000, 0x0001507C)}
            /* channels page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00015800, 0x00015BF8)}
            /* Post_processor_page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001F148, 0x0001F15B)}
            /* AIC page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001F400, 0x0001FBFF)}
            /* Input adapter page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001FC40, 0x0001FC57)}
            /* channels page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001FE00, 0x0001FE7F)}
            /* system page1 */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001FFE0, 0x0001FFE0)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
        {
            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArrIngr[] =
            {
                 /*                             LSBits of 'Addr',     val,                      */
                 {DUMMY_NAME_PTR_CNS,            0x00005ff8,         0xa0000008,      1,    0x0}   /* EIP66_CONTROL  */
                ,{DUMMY_NAME_PTR_CNS,            0x00005ffc,         0x0141bd42,      1,    0x0}   /* EIP66_VERSION  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x40808040,      1,    0x0}   /* EIP164_CONFIG2 */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x40800040,      1,    0x0}   /* EIP164_CONFIG  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02445BA4,      1,    0x0}   /* EIP164_VERSION */
                ,{NULL,                          0,                  0x00000000,      0,    0x0}
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC listIngr = {myUnit_registersDefaultValueArrIngr,NULL};

            static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArrEgr[] =
            {
                 /*                             LSBits of 'Addr',     val,                      */
                 {DUMMY_NAME_PTR_CNS,            0x00005ff8,         0xa0000008,      1,    0x0}   /* EIP66_CONTROL  */
                ,{DUMMY_NAME_PTR_CNS,            0x00005ffc,         0x0141bd42,      1,    0x0}   /* EIP66_VERSION  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff4,         0x40808040,      1,    0x0}   /* EIP164_CONFIG2 */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fff8,         0x80800040,      1,    0x0}   /* EIP164_CONFIG  */
                ,{DUMMY_NAME_PTR_CNS,            0x0000fffc,         0x02445BA4,      1,    0x0}   /* EIP164_VERSION */
                ,{NULL,                          0,                  0x00000000,      0,    0x0}
            };
            static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC listEgr = {myUnit_registersDefaultValueArrEgr,NULL};

            unitPtr->unitDefaultRegistersPtr = isEgress ? &listEgr : &listIngr;
        }
    }
}

/**
* @internal smemPhoenixUnitPcaMacsecEip164Ing function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-164 Ingress device (MACSec Classifier)
*/
static void smemPhoenixUnitPcaMacsecEip164Ing
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    smemPhoenixUnitPcaMacsecEip164(devObjPtr,unitPtr,GT_FALSE);
}

/**
* @internal smemPhoenixUnitPcaMacsecEip164Egr function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC EIP-164 Egress device (MACSec Classifier)
*/
static void smemPhoenixUnitPcaMacsecEip164Egr
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    smemPhoenixUnitPcaMacsecEip164(devObjPtr,unitPtr,GT_TRUE);
}

/**
* @internal smemPhoenixUnitLed function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the LED
*/
static void smemPhoenixUnitLed
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000A4)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000160)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000220)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}

/**
* @internal smemPhoenixUnitDfxServer function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the DFX unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitDfxServer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG() */
    {0x000f800c, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteDfxServerResetControlReg, 0},
    /* SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl50 */
    {0x000f8d68, SMEM_FULL_MASK_CNS, NULL, 0 , smemSip6_30ActiveWriteDfxServerDeviceCtrl50Reg, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8000, 0x000F8018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8020, 0x000F8038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8040, 0x000F8050)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000F8D00, 0x000F8D68)}
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
            ,{DUMMY_NAME_PTR_CNS,            0x000F80DC,         0x00020000,      1,    0x0 }
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
            ,{DUMMY_NAME_PTR_CNS,            0x000f8d50,         0xffff0000,      1,    0x0 }

         ,{NULL,    0, 0x00000000,       0,    0x0 }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemPhoenixUnitCnmMppRfu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM MPP RFU unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemPhoenixUnitCnmMppRfu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000008)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000011C)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001008)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {/*start of unit cnm_regs */
        {/*start of unit CNM_MPP_RFU */

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl0_7                          = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl8_15                         = 0x00000104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl16_23                        = 0x00000108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl24_31                        = 0x0000010c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl32_39                        = 0x00000110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl40_47                        = 0x00000114;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl48_55                        = 0x00000118;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNM.MPP_RFU.MPPControl56_63                        = 0x0000011c;
        } /*end of unit MPP_RFU */
    }/* end of unit cnm_regs */

}

/**
* @internal smemPhoenixUnitCnmRunit function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNM Runit unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemPhoenixUnitCnmRunit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008100, 0x00008174)},
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000081F0, 0x000081FC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemPhoenixUnitCnmAddrDecodeUnit function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CnM Addr decoder unit
* @param[in] devObjPtr                - pointer to device memory.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemPhoenixUnitCnmAddrDecodeUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    SIM_TBD_BOOKMARK /* need to get defaults as in Emulator of Phoenix (all ZERO in Cider) */

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000027C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }
}

/**
* @internal smemPhoenixSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_PCA_LMU_0)     ,smemPhoenixUnitLmu              }
        ,{STR(UNIT_RX_DMA)        ,smemPhoenixUnitRxDma            }
        ,{STR(UNIT_TX_FIFO)       ,smemPhoenixUnitTxFifo           }
        ,{STR(UNIT_TX_DMA)        ,smemPhoenixUnitTxDma            }
        ,{STR(UNIT_PB_CENTER_BLK)              ,smemPhoenixUnitPacketBuffer_pbCenter        }
        ,{STR(UNIT_PB_COUNTER_BLK)             ,smemPhoenixUnitPacketBuffer_pbCounter       }
        ,{STR(UNIT_PB_WRITE_ARBITER_0)         ,smemPhoenixUnitPacketBuffer_smbWriteArbiter }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,smemPhoenixUnitPacketBuffer_packetWrite     }
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_0)     ,smemPhoenixUnitPacketBuffer_cellRead        }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_0)   ,smemPhoenixUnitPacketBuffer_packetRead      }
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_0)     ,smemPhoenixUnitPacketBuffer_npmMc           }
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0)       ,smemPhoenixUnitPacketBuffer_sbmMc           }
        ,{STR(UNIT_TXQ_PDX)       ,smemPhoenixUnitTxqPdx           }
        ,{STR(UNIT_TXQ_PFCC)      ,smemPhoenixUnitTxqPfcc          }
        ,{STR(UNIT_TXQ_PSI)       ,smemPhoenixUnitTxqPsi           }
        ,{STR(UNIT_TXQ_SDQ0)      ,smemPhoenixUnitTxqSdq           }
        ,{STR(UNIT_TXQ_PDS0)      ,smemPhoenixUnitTxqPds           }
        ,{STR(UNIT_MAC_400G_0)    ,smemPhoenixUnitMac400GWrap      }
        ,{STR(UNIT_PCS_400G_0)    ,smemPhoenixUnitPcs400G          }
        ,{STR(UNIT_MAC_CPU)       ,smemPhoenixUnitMacCpuWrap       }
        ,{STR(UNIT_PCS_CPU)       ,smemPhoenixUnitPcsCpu           }
        ,{STR(UNIT_SERDES_SDW0)   ,smemPhoenixUnitSerdes_x4        }
        ,{STR(UNIT_SERDES_SDW4_1) ,smemPhoenixUnitSerdes_x1        }
        ,{STR(UNIT_TXQ_QFC0)      ,smemPhoenixUnitTxqQfc           }
        ,{STR(UNIT_PCA_CTSU_0)    ,smemPhoenixUnitPcaCtsu          }
        ,{STR(UNIT_PCA_BRG_0)     ,smemPhoenixUnitPcaBrg           }
        ,{STR(UNIT_PCA_PZ_ARBITER_I_0)  ,smemPhoenixUnitPcaArbiter }
        ,{STR(UNIT_PCA_SFF_0)     ,smemPhoenixUnitPcaSff           }
        ,{STR(UNIT_PCA_MACSEC_EXT_I_163_0)     ,smemPhoenixUnitPcaMacsecExt}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_I_0)     ,smemPhoenixUnitPcaMacsecEip163Ing}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_I_0)     ,smemPhoenixUnitPcaMacsecEip164Ing}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_E_0)     ,smemPhoenixUnitPcaMacsecEip163Egr}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_E_0)     ,smemPhoenixUnitPcaMacsecEip164Egr}
        ,{STR(UNIT_LED_USX2_0_LED)             ,smemPhoenixUnitLed}
        ,{STR(UNIT_MIF_0)         ,smemPhoenixUnitMif              }
        ,{STR(UNIT_ANP_0)         ,smemPhoenixUnitAnp              }
        ,{STR(UNIT_USX_0_MAC_0)   ,smemPhoenixUsxMac               }
        ,{STR(UNIT_USX_0_PCS_0)   ,smemPhoenixUsxPcs               }
        ,{STR(UNIT_DFX_SERVER)    ,smemPhoenixUnitDfxServer        }

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units(devObjPtr);
}

/**
* @internal smemPhoenixSpecificDeviceUnitAllocDerived_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
*         Evaluation version of smemPhoenixSpecificDeviceUnitAlloc_DP_units
*         replacing derived units by original.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] skipUnitsArrPtr          - pointer to array of aleady allocated units to skip
*/
void smemPhoenixSpecificDeviceUnitAllocDerived_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_DEVICE_UNIT_ALLOCATION_STC *skipUnitsArrPtr
)
{
    static const SMEM_DEVICE_UNIT_ALLOCATION_STC patterAllocUnitsArr[] = {
         {STR(UNIT_PCA_LMU_0)     ,smemPhoenixUnitLmu              }
        ,{STR(UNIT_RX_DMA)        ,smemPhoenixUnitRxDma            }
        ,{STR(UNIT_TX_FIFO)       ,smemPhoenixUnitTxFifo           }
        ,{STR(UNIT_TX_DMA)        ,smemPhoenixUnitTxDma            }
        ,{STR(UNIT_PB_CENTER_BLK)              ,smemPhoenixUnitPacketBuffer_pbCenter        }
        ,{STR(UNIT_PB_COUNTER_BLK)             ,smemPhoenixUnitPacketBuffer_pbCounter       }
        ,{STR(UNIT_PB_WRITE_ARBITER_0)         ,smemPhoenixUnitPacketBuffer_smbWriteArbiter }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,smemPhoenixUnitPacketBuffer_packetWrite     }
        ,{STR(UNIT_PB_GPC_GRP_CELL_READ_0)     ,smemPhoenixUnitPacketBuffer_cellRead        }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_0)   ,smemPhoenixUnitPacketBuffer_packetRead      }
        ,{STR(UNIT_PB_NEXT_POINTER_MEMO_0)     ,smemPhoenixUnitPacketBuffer_npmMc           }
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0)       ,smemPhoenixUnitPacketBuffer_sbmMc           }
        ,{STR(UNIT_TXQ_PDX)       ,smemPhoenixUnitTxqPdx           }
        ,{STR(UNIT_TXQ_PFCC)      ,smemPhoenixUnitTxqPfcc          }
        ,{STR(UNIT_TXQ_PSI)       ,smemPhoenixUnitTxqPsi           }
        ,{STR(UNIT_TXQ_SDQ0)      ,smemPhoenixUnitTxqSdq           }
        ,{STR(UNIT_TXQ_PDS0)      ,smemPhoenixUnitTxqPds           }
        ,{STR(UNIT_MAC_400G_0)    ,smemPhoenixUnitMac400GWrap      }
        ,{STR(UNIT_PCS_400G_0)    ,smemPhoenixUnitPcs400G          }
        ,{STR(UNIT_MAC_CPU)       ,smemPhoenixUnitMacCpuWrap       }
        ,{STR(UNIT_PCS_CPU)       ,smemPhoenixUnitPcsCpu           }
        ,{STR(UNIT_SERDES_SDW0)   ,smemPhoenixUnitSerdes_x4        }
        ,{STR(UNIT_SERDES_SDW4_1) ,smemPhoenixUnitSerdes_x1        }
        ,{STR(UNIT_TXQ_QFC0)      ,smemPhoenixUnitTxqQfc           }
        ,{STR(UNIT_PCA_CTSU_0)    ,smemPhoenixUnitPcaCtsu          }
        ,{STR(UNIT_PCA_BRG_0)     ,smemPhoenixUnitPcaBrg           }
        ,{STR(UNIT_PCA_PZ_ARBITER_I_0)  ,smemPhoenixUnitPcaArbiter }
        ,{STR(UNIT_PCA_SFF_0)     ,smemPhoenixUnitPcaSff           }
        ,{STR(UNIT_PCA_MACSEC_EXT_I_163_0)     ,smemPhoenixUnitPcaMacsecExt}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_I_0)     ,smemPhoenixUnitPcaMacsecEip163Ing}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_I_0)     ,smemPhoenixUnitPcaMacsecEip164Ing}
        ,{STR(UNIT_PCA_MACSEC_EIP_163_E_0)     ,smemPhoenixUnitPcaMacsecEip163Egr}
        ,{STR(UNIT_PCA_MACSEC_EIP_164_E_0)     ,smemPhoenixUnitPcaMacsecEip164Egr}
        ,{STR(UNIT_LED_USX2_0_LED)             ,smemPhoenixUnitLed}
        ,{STR(UNIT_MIF_0)         ,smemPhoenixUnitMif              }
        ,{STR(UNIT_ANP_0)         ,smemPhoenixUnitAnp              }
        ,{STR(UNIT_USX_0_MAC_0)   ,smemPhoenixUsxMac               }
        ,{STR(UNIT_USX_0_PCS_0)   ,smemPhoenixUsxPcs               }
        ,{STR(UNIT_DFX_SERVER)    ,smemPhoenixUnitDfxServer        }

        /* must be last */
        ,{NULL,NULL}
    };

    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[sizeof(patterAllocUnitsArr)/sizeof(patterAllocUnitsArr[0])];
    GT_U32                          src, skip, dst, skipFound;
    GT_CHAR*                        unitNameStr;

    dst = 0;
    for (src = 0; (1/*no break condition*/); src++)
    {
        unitNameStr = patterAllocUnitsArr[src].unitNameStr;
        if (unitNameStr == NULL)
        {
            allocUnitsArr[dst].unitNameStr   = NULL;
            allocUnitsArr[dst].allocUnitFunc = NULL;
            break;
        }
        skipFound = 0;
        for (skip = 0; (skipUnitsArrPtr[skip].unitNameStr); skip++)
        {
            if (0 == strcmp(skipUnitsArrPtr[skip].unitNameStr, unitNameStr))
            {
                skipFound = 1;
                break;
            }
        }
        if (skipFound) continue;

        allocUnitsArr[dst].unitNameStr   = patterAllocUnitsArr[src].unitNameStr;
        allocUnitsArr[dst].allocUnitFunc = patterAllocUnitsArr[src].allocUnitFunc;
        dst++;
    }


    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);

    hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units(devObjPtr);
}

/**
* @internal smemPhoenixSpecificDeviceUnitAlloc_Cnm_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_Cnm_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_CNM_MPP_RFU)      ,smemPhoenixUnitCnmMppRfu        }
        ,{STR(UNIT_CNM_ADDR_DECODER) ,smemPhoenixUnitCnmAddrDecodeUnit}
        ,{STR(UNIT_CNM_RUNIT)        ,smemPhoenixUnitCnmRunit}

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}


/**
* @internal smemPhoenixSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemPhoenixSpecificDeviceUnitAlloc
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
        SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &phoenix_units[0];

        for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
        {
            jj = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,unitInfoPtr->base_addr);

            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[jj];

            if(currUnitChunkPtr->chunkIndex != jj)
            {
                skernelFatalError("smemPhoenixSpecificDeviceUnitAlloc : not matched index");
            }
            currUnitChunkPtr->numOfUnits = 1;/*unitInfoPtr->size;*/
        }
    }

    /*allocate 'PEX config space' and 'BAR0' -- if not allocated already*/
    smemFalconPexAndBar0DeviceUnitAlloc(devObjPtr);

    smemPhoenixSpecificDeviceUnitAlloc_Cnm_units(devObjPtr);

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    smemPhoenixSpecificDeviceUnitAlloc_DP_units(devObjPtr);

    smemPhoenixSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
}

/**
* @internal smemPhoenixConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Phoenix : Convert (dev,address) to new (dev,address).
*         needed for multi-MG device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
static void smemPhoenixConvertDevAndAddrToNewDevAndAddr
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

    pipeOffset = internalPhoenixUnitPipeOffsetGet(devObjPtr, address,
        &pipeIdOfAddress,&currentPipeId,GT_FALSE);

    newAddress = address + pipeOffset;

    *newAddressPtr = newAddress;

    return;
}

/**
* @internal smemPhoenixPrepareMultiMgRecognition function
* @endinternal
*
* @brief   prepare multi MG units recognition
*
* @param[in] devObjPtr                - pointer to device object of pipe 0
*/
static void smemPhoenixPrepareMultiMgRecognition
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    /* NOTE: the function smemPhoenixConvertDevAndAddrToNewDevAndAddr will help
       any memory access to 'MG0' address to access proper MG unit */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc =
        smemPhoenixConvertDevAndAddrToNewDevAndAddr;
}


/**
* @internal smemPhoenixActiveWriteMacSecCountersControl function
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
static void smemPhoenixActiveWriteMacSecCountersControl (
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
* @internal smemPhoenixInitRegDbDpUnits function
* @endinternal
*
* @brief   Init RegDb for DP units
*/
static void smemPhoenixInitRegDbDpUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    /* set register addresses for SDQ[0] */
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,0);/*DP[0]*/

    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,0);/*DP[0]*/


    /* set register addresses for sip6_rxDMA[0])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA   , sip6_rxDMA ,0);/*DP[0]*/

    /* set register addresses for sip6_txDMA[0])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA   , sip6_txDMA ,0);/*DP[0]*/

    /* set register addresses for sip6_txFIFO[0])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,0);/*DP[0]*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_TXQ_PDX , SIP6_TXQ_PDX);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PREQ,PREQ);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EREP,EREP);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EM,EXACT_MATCH);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PHA,PHA);
/*    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_SHM,SHM);*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PPU,PPU);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_AAC, CNM.AAC);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_MPP_RFU, CNM.MPP_RFU);

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
    smemPhoenixInterruptTreeInit(devObjPtr);
}

/**
* @internal internal_smemPhoenixInit function
* @endinternal
*
* @brief   Init memory module for a Phoenix device.
*/
static void internal_smemPhoenixInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isPhoenix = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP6_10_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP6_15_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isPhoenix = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = phoenix_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = phoenix_duplicatedUnits;
    }

    if (isPhoenix == GT_TRUE)
    {
        devObjPtr->devMemGopRegDbInitFuncPtr = smemPhoenixGopRegDbInit;
        devObjPtr->devIsOwnerMemFunPtr = NULL;
        devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;
        devObjPtr->devMemPortInfoGetPtr =  smemPhoenixPortInfoGet;

        devObjPtr->devMemGetMgUnitIndexFromAddressPtr = smemPhoenixGetMgUnitIndexFromAddress;
        devObjPtr->devMemGopPortByAddrGetPtr = smemPhoenixGopPortByAddrGet;
        devObjPtr->devMemMibPortByAddrGetPtr = smemPhoenixMibPortByAddrGet;

        devObjPtr->numOfPipesPerTile = 0;/* no pipes */
        devObjPtr->numOfTiles        = 0;/* no tiles */
        devObjPtr->tileOffset        = 0;
        devObjPtr->mirroredTilesBmp  = 0;

        devObjPtr->numOfMgUnits = PHOENIX_NUM_MG_UNITS;/* 3 MG units : 2 connected to SDMA and both to DP[0] -- the single DP ! */

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  0;/* not support multi data path ... as only single DP */
        devObjPtr->multiDataPath.maxDp = 1;             /* single DP (Hawk hold 4 DPs) */
        /* !!! there is NO TXQ-dq in Phoenix (like in Flacon) !!! */
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

        devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);

        devObjPtr->dma_specialPortMappingArr = phoenix_DMA_specialPortMappingArr;/* only the SDMA ports */
        devObjPtr->gop_specialPortMappingArr = NULL;/* dummy - empty                      no reduced port */
        devObjPtr->ravens_specialPortMappingArr = NULL;
        devObjPtr->cpuPortSdma_specialPortMappingArr = phoenix_cpuPortSdma_specialPortMappingArr;


        devObjPtr->tcam_numBanksForHitNumGranularity = 2; /* like BC2 */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->support_remotePhysicalPortsTableMode = 1;

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);/*1024 in Falcon , 512 in BC3, 128 in Aldrin2*/
        /*NOTE: support 8K eports like Falcon.
        */

#if 0   /* Phoenix not supports cnc2,3 */
        {
            GT_U32  index;
            /* bind CNC uploads of CNC[0,1] to MG[0] and CNC[2,3] to MG[1]*/
            for(index = 0 ; index < 4 ; index++)
            {
                devObjPtr->cncUnitInfo[index].mgUnit = (index < 2) ? 0 : 1;
            }
        }
#endif /* 0 */

        devObjPtr->cncNumOfUnits = 1;/* must set 1 CNC units otherwise smemLion3Init(...) will set it to 2 */

        devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emMaxNumEntries  = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emAutoLearnNumEntries = 16 * _1K;           /* 64K in Hawk*/
        devObjPtr->limitedResources.eVid  = 6 * _1K;           /*  8K in Hawk*/
        devObjPtr->limitedResources.stgId = 1 * _1K;           /*  4K in Hawk*/
        devObjPtr->limitedResources.nextHop     =  8*_1K;      /* 16K in Hawk*/

        devObjPtr->policerSupport.iplrTableSize =  2*_1K;      /*  4K in Hawk*/
        devObjPtr->policerSupport.numOfIpfix    = 16*_1K;      /* 64K in Hawk*/
        devObjPtr->tcamNumOfFloors   = 6; /*(16 in Hawk) support 6 floors : each 3K of 10B = total 18K of 10B = 9K@20B */
        devObjPtr->limitedResources.mllPairs    =  4*_1K;        /*  8K in Hawk*/
        devObjPtr->limitedResources.l2LttMll    = 12*_1K;       /* 16K in Hawk*/
        devObjPtr->limitedResources.l3LttMll    =  4*_1K;       /*  4K in Hawk (the same)*/
        devObjPtr->limitedResources.l2Ecmp      =  8*_1K;       /*  8K in Hawk (the same)*/
        devObjPtr->limitedResources.l2LttEcmp   =  8*_1K;       /* like Hawk according to number of eports(8K) */
        devObjPtr->limitedResources.numOfArps   = 16*_1K;       /*256K in Hawk*/
        devObjPtr->limitedResources.ipvxEcmp    = 2*_1K;          /* 6K in Hawk : each line 2 entries */

        devObjPtr->supportSdmaCnMConvert = 1;
        devObjPtr->numofTcamClients = PHOENIX_TCAM_NUM_OF_GROUPS_CNS;
    }

    SET_IF_ZERO_MAC(devObjPtr->ipvxEcmpIndirectMaxNumEntries , 4*_1K);/* 6K in falcon */
    SET_IF_ZERO_MAC(devObjPtr->defaultEPortNumEntries,SIP6_10_NUM_DEFAULT_E_PORTS_CNS);/*1K in Falcon*/

    SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries,1*_1K);/* 10K in Hawk*/


#if 0 /* like Falcon */
    {

        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfLpmRams , 30);
        SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfEntriesBetweenRams , 32*1024);

        SET_IF_ZERO_MAC(devObjPtr->cncClientSupportBitmap,SNET_SIP6_CNC_CLIENTS_BMP_ALL_CNS);
    }
#endif

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemPhoenixSpecificDeviceUnitAlloc;
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

    smemHawkInit(devObjPtr);

    if (isPhoenix == GT_TRUE)
    {

        /* Hawk - bind special 'find memory' functions */
        smemHawkBindFindMemoryFunc(devObjPtr);

        /* Init RegDb for DP units */
        smemPhoenixInitRegDbDpUnits(devObjPtr);

        /* prepare multi MG units recognition */
        smemPhoenixPrepareMultiMgRecognition(devObjPtr);

        /* check that no unit exceed the size of it's unit */
        smemGenericUnitSizeCheck(devObjPtr,phoenix_units);

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
    GT_U32  origPortNum = portNumber;
    /* EGF link down filter */

    if(portNumber >= 59 ) /* convert MAC to skip the 'physical ports hole' 59..63 */
    {
        portNumber += 5;
    }

    if(portNumber >= 128)
    {
        skernelFatalError("egfLinkUpOnSlanBind : unknown portNumber [%d] \n",
            origPortNum);
    }


    /* set the SLAN ports in link up */
    regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(devObjPtr,portNumber/32);
    startBit = portNumber % 32;
    value = bindTx == GT_TRUE ? 1 : 0;

    /* need to update the register in the 2 pipes */
    updateRegisterInAllPipes(devObjPtr,regAddress,startBit,1,value);
}

/**
* @internal smemPhoenixInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
void smemPhoenixInit_debug_allowTraffic
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
        GT_U32  numPortsPerTile = devObjPtr->multiDataPath.maxDp * RUNTIME_NUM_PORTS_PER_DP_UNIT;

        /* port per register */
        for(ii = 0 ; ii < numPortsPerTile ; ii++)
        {
            regAddress = SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,
                ii%(RUNTIME_NUM_PORTS_PER_DP_UNIT*devObjPtr->multiDataPath.maxDp));

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
                SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E,1);
            snetChtPortMacFieldSet(devObjPtr, ii,
                SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E,1);
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

            /* set only 10 bits , to not harm other defaults */
            updateRegisterInAllPipes(devObjPtr,regAddress,0,10,value);
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

           for(jj=0;jj<RUNTIME_NUM_PORTS_PER_DP_UNIT;jj++)
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
            for(jj = 0 ; jj < RUNTIME_NUM_PORTS_PER_DP_UNIT; jj++)
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

        /*for(tileId = 0 ; tileId < devObjPtr->numOfTiles;tileId++)*/
        {
            for(ii = 0 ; ii < 64 /*support only 64 in Phoenix*/; ii++)
            {
                queue_base = 0;
                queue_pds_index = 0;

                if(ii <= RUNTIME_NUM_PORTS_PER_DP_UNIT)/* physical ports mapped 1:1 with global MAC/DMA*/
                {
                    queue_base = 8 * ii;
                    queue_pds_index = 0;/*DP[0]*/
                }
                else if (ii == 63) /* SDMA CPU port */
                {
                    queue_base = RUNTIME_NUM_PORTS_PER_DP_UNIT * 8;
                    queue_pds_index = 0;/*DP[0]*/
                }
                else
                {
                    /*keep 0*/
                }

                value = 0;
                SMEM_U32_SET_FIELD(value,0,9,queue_base);
                SMEM_U32_SET_FIELD(value,9,6,0/*dp_core_local_trg_port -- not care for simulation ... so 0 */);
                SMEM_U32_SET_FIELD(value,15,3,queue_pds_index);
                SMEM_U32_SET_FIELD(value,18,2,0/*queue_pdx_index -- tileId*/);

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

    /* handle the MIF unit */
#if 0
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
#endif /*0*/

    {
        GT_U32  ii;
        GT_U32  mifType;
        GT_U32  local_mif_Rx_channel_id_number;
        GT_U32  regAddr,regValue;

        GT_U32  sip6_MTI_EXTERNAL_representativePort;
        ENHANCED_PORT_INFO_STC portInfo;
#if 0
        SKERNEL_PORT_MIF_INFO_STC   *portMifInfoPtr;
#endif
        for(ii = 0 ; ii < 48 ; ii++)
        {
            if(!IS_CHT_VALID_PORT(devObjPtr,ii))
            {
                continue;
            }
            devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E  ,ii,&portInfo);
            local_mif_Rx_channel_id_number =  portInfo.simplePortInfo.indexInUnit;

            /* MIF hold dedicated logic for representative port (apply to 'CPU port' too!) */
            sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;

#if 0
            portMifInfoPtr = &devObjPtr->portsArr[ii].mifInfo[0];
            portMifInfoPtr->txEnabled = GT_TRUE;
            portMifInfoPtr->mifType   = 0;
            portMifInfoPtr->txMacNum  = ii;
            portMifInfoPtr->egress_isPreemptiveChannel  = 0;
#endif /*0*/
            for(mifType = 0 ; mifType < 1; mifType++)
            {
                regAddr = MTI_PORT_MIF(devObjPtr,ii).mif_channel_mapping_register[mifType];
                /*update the register via SCIB for active memory */
                scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
                /*<mif_is_clock_enable>*/
                SMEM_U32_SET_FIELD(regValue,18,1,1); /*clock enable*/
                /*local_dma_port_number*/
                SMEM_U32_SET_FIELD(regValue, 0,6,ii); /*local_dma_port_number-Tx*/
                /*local_dma_port_number*/
                SMEM_U32_SET_FIELD(regValue, 8,6,ii); /*local_dma_port_number-Rx*/
                scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);

                regAddr = MTI_PORT_MIF(devObjPtr,ii).mif_link_fsm_control_register[mifType];
                /*update the register via SCIB for active memory */
                scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
                /*<mif_is_channel_force_link_down>*/
                SMEM_U32_SET_FIELD(regValue,1,2,0); /*Rx enable*/
                scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);


                regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_rx_control_register[mifType];
                /*update the register via SCIB for active memory */
                scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
                /*<mif_is_Rx_channel_enable>*/
                SMEM_U32_SET_FIELD(regValue,local_mif_Rx_channel_id_number,1,1); /*Rx enable*/
                scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);


                regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_tx_control_register[mifType];
                /*update the register via SCIB for active memory */
                scibReadMemory (devObjPtr->deviceId, regAddr , 1,&regValue);
                /* NOTE : for preemption channel the 'local_mif_channel_number' is the 'preemption index' */
                SMEM_U32_SET_FIELD(regValue,local_mif_Rx_channel_id_number,1,1); /*Rx enable*/
                scibWriteMemory(devObjPtr->deviceId, regAddr , 1,&regValue);


            }
        }

    }

}

/**
* @internal smemPhoenixInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemPhoenixInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    internal_smemPhoenixInit(devObjPtr);
}
/**
* @internal smemPhoenixInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemPhoenixInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    static int my_dummy = 0;

    {
        /* fix FDB table size (SAR) */
        smemRegFldSet(devObjPtr ,
            SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr),
            11,3,3);/* support for SMEM_MAC_TABLE_SIZE_32KB */
    }

    smemHawkInit2(devObjPtr);

    if(my_dummy)
    {
        smemPhoenixInit_debug_allowTraffic(devObjPtr);
    }
}

/**
* @internal smemSip6_30ActiveWriteDfxServerDeviceCtrl50Reg function
* @endinternal
*
* @brief   Dfx Server 'device ctrl 50' register. allow to
*         trigger TAI sync to all TAI units with single trigger.
*         (bit 10)
*/
void smemSip6_30ActiveWriteDfxServerDeviceCtrl50Reg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  origValue = *memPtr;
    GT_U32  bitNum = 10;

    *memPtr = *inMemPtr;/* save value that written by the CPU*/

    /* <ptp_internal_trigger >
       This field enables triggering the PTP mechanism
       (instead of the PTP_PULSE pad) */
    if(0 == SMEM_U32_GET_FIELD((*memPtr) ,bitNum ,1))
    {
        /* no more to do */
        return;
    }

    if(SMEM_U32_GET_FIELD(origValue ,bitNum ,1))
    {
        /* the bit was set before the 'current write' , so do not trigger new action */
        return;
    }

    /* update all TAI units about the TOD function triggering */
    smemLion3TodFuncUpdate_ALL(devObjPtr);

    /* DO NOT clear the bit ... because it is 'RW' */
}
