/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smemIronman.c
*
* DESCRIPTION:
*       Ironman memory mapping implementation
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/smem/smemIronman.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <common/Utils/Math/sMath.h>

/* single DP unit */
#define IRONMAN_NUM_DP_UNITS  1

/* the size in bytes of the MG unit */
#define MG_SIZE             _1M
/* base address of the CNM unit that is single for 2 tiles */
#define CNM_OFFSET_CNS       0x3C000000
/* base address of the MG 0_0 unit . MG_0_0 to MG_0_3 serve tile 0 */
#define MG_0_0_OFFSET_CNS    0x7F900000
/* used for GM devices */
GT_U32   simIronmanMgBaseAddr = MG_0_0_OFFSET_CNS;

#define TSU_0_BASE_ADDR 0x91000000

#ifndef _64K
    #define _64K (64*1024)
#endif

#define MAC_UNIT_SIZE _64K
#define PCS_UNIT_SIZE _64K

#define MTIP_USX_MAC_0_BASE_ADDR  0x0d600000
#define MTIP_USX_MAC_1_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 1)
#define MTIP_USX_MAC_2_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 2)
#define MTIP_USX_MAC_3_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 3)
#define MTIP_USX_MAC_4_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 4)
#define MTIP_USX_MAC_5_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 5)
#define MTIP_USX_MAC_6_BASE_ADDR  (MTIP_USX_MAC_0_BASE_ADDR + MAC_UNIT_SIZE * 6)

#define MTIP_UNIT_USX_PCS_0_BASE_ADDR  0x0d900000
#define MTIP_UNIT_USX_PCS_1_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 1)
#define MTIP_UNIT_USX_PCS_2_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 2)
#define MTIP_UNIT_USX_PCS_3_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 3)
#define MTIP_UNIT_USX_PCS_4_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 4)
#define MTIP_UNIT_USX_PCS_5_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 5)
#define MTIP_UNIT_USX_PCS_6_BASE_ADDR  (MTIP_UNIT_USX_PCS_0_BASE_ADDR  + PCS_UNIT_SIZE * 6)

#define MIF_0_BASE_ADDR           0x98600000
#define MIF_1_BASE_ADDR           0x99600000
#define MIF_2_BASE_ADDR           0x93600000
#define MIF_3_BASE_ADDR           0x92300000

#define ANP_0_BASE_ADDR           0x0d800000
#define ANP_1_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 1)
#define ANP_2_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 2)
#define ANP_3_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 3)
#define ANP_4_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 4)
#define ANP_5_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 5)
#define ANP_6_BASE_ADDR           (ANP_0_BASE_ADDR  + _64K * 6)

#define NUM_PORTS_PER_UNIT        8
#define NUM_PORTS_PER_CTSU_UNIT   64
#define NUM_PORTS_PER_USX_UNIT    8    /* 8 MACs in the USX unit  */

/* offsets within the : Hawk/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x00001000 /* currently use value like in phoenix  */
#define PORT0_100G_OFFSET         0x00002000 /* currently use value like in phoenix  */


#define START_MIB_RX_PER_PORT 0x00000100
#define SIZE_MIB_RX_PER_PORT  (43*4) /*decimal*/
#define SIZE_MIB_RX  (SIZE_MIB_RX_PER_PORT*NUM_PORTS_PER_UNIT*2/*EMAC/PMAC*/)
#define START_MIB_TX_PER_PORT (START_MIB_RX_PER_PORT + SIZE_MIB_RX)
#define SIZE_MIB_TX_PER_PORT  (34*4) /*decimal*/
#define SIZE_MIB_TX  (SIZE_MIB_TX_PER_PORT*NUM_PORTS_PER_UNIT*2/*EMAC/PMAC*/)
#define MAC_MIB_UNIT_SIZE       0x2000
#define IRONMAN_MAX_PROFILE_CNS   12


#define MAC_STEP_PORT_OFFSET      0x00001000

/********** start : PCS for 400G *********/
#define PCS_400G_OFFSET           0x00000000
#define PCS_200G_OFFSET           0x00001000
#define PCS_PORT0_100G_OFFSET     0x00002000
#define PCS_PORT1_50G_OFFSET      (PCS_PORT0_100G_OFFSET + PCS_STEP_PORT_OFFSET)
#define PCS_RS_FEC_OFFSET         0x0000a000
#define PCS_LPCS_OFFSET           0x0000b000

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

#define IRONMAN_PSI_SCHED_OFFSET         (GT_U32)0x00000300


/********** start : PCS for usx *********/
#define USX_PCS_PORTS_OFFSET      0x00001000
#define USX_PCS_RS_FEC_OFFSET     0x00003000
#define USX_PCS_LPCS_OFFSET       0x00000000
#define USX_PCS_USXM_OFFSET       0x00002000

#define USX_PCS_SERDES_OFFSET     0x00003000 /* for RS_FEC,RS_FEC_STAT,USXM */
#define USX_PCS_STEP_PORT_OFFSET  0x00000100
#define USX_LPCS_STEP_PORT_OFFSET  0x00000080

#define IRONMAN_PSI_SCHED_OFFSET         (GT_U32)0x00000300
#define NUM_PORTS_PER_DP_UNIT     56 /*+1 for CPU  */

/* use runtime parameter of number of DP channels (ports)   */
/* this to allow the Ironman to use the Phoenix units allocations */
/* as at this moment there is no Cider for the Ironman      */
#define RUNTIME_NUM_PORTS_PER_DP_UNIT \
    ((devObjPtr->multiDataPath.info[0].cpuPortDmaNum >= devObjPtr->multiDataPath.info[0].dataPathNumOfPorts) ? \
      devObjPtr->multiDataPath.info[0].cpuPortDmaNum  :                                                        \
      devObjPtr->multiDataPath.info[0].dataPathNumOfPorts)


static void smemIronmanSpecificDeviceUnitAlloc_TXQ_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
static void smemIronmanSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

static GT_U32 smemSip6_30FdbHsrPrpTimerGet
(
    IN struct SKERNEL_DEVICE_OBJECT_T * devObjPtr,
    IN GT_BOOL  useFactor
);

/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/
Projects/Ironman/Shared%20Documents/Design/Address%20Space/
Ironman_Address_Space_CC_200715.xlsm
*/
#define UNIT_INFO_MAC(baseAddr,unitName,size) \
     {baseAddr , STR(unitName)                                , size , 0}

/* the units of  */
static SMEM_GEN_UNIT_INFO_STC ironman_units[] =
{

    /* packet buffer subunits */
     UNIT_INFO_MAC(0x8cc00000, UNIT_PB_CENTER_BLK              ,1  *     _1M)
    ,UNIT_INFO_MAC(0x8ca00000, UNIT_PB_COUNTER_BLK             ,1  *     _1M)
    ,UNIT_INFO_MAC(0x8cd00000, UNIT_PB_GPC_GRP_PACKET_WRITE_0  ,1  *     _1M)
    ,UNIT_INFO_MAC(0x8cb00000, UNIT_PB_GPC_GRP_PACKET_READ_0   ,1  *     _1M)
    ,UNIT_INFO_MAC(0x8c900000, UNIT_PB_SHARED_MEMO_BUF_0       ,1  *     _1M)

    ,UNIT_INFO_MAC(0x8d400000,UNIT_TXQ_PDX            ,64 *     _1K)
    ,UNIT_INFO_MAC(0x8d200000,UNIT_TXQ_PFCC           ,64    * _1K)
    ,UNIT_INFO_MAC(0x8d600000,UNIT_TXQ_PSI            , 1  * _1M)

    ,UNIT_INFO_MAC(0x8d500000,UNIT_TXQ_SDQ0  ,128  *    _1K)
    ,UNIT_INFO_MAC(0x8d100000,UNIT_TXQ_PDS0  ,512  *    _1K)
    ,UNIT_INFO_MAC(0x8d300000,UNIT_TXQ_QFC0  ,64   *    _1K)

    ,UNIT_INFO_MAC(0x8d000000,UNIT_RX_DMA            , 1  *     _1M)

/*    ,UNIT_INFO_MAC(0x0cf00000,UNIT_IA                ,64  *     _1K)*/

    ,UNIT_INFO_MAC(0x8ce00000,UNIT_TX_DMA            , 1  *     _1M)
    ,UNIT_INFO_MAC(0x8cf00000,UNIT_TX_FIFO           , 1  *     _1M)

    ,UNIT_INFO_MAC(0x91100000,UNIT_PCA_PZ_ARBITER_I_0     ,512  * _1K )
    ,UNIT_INFO_MAC(0x90F00000,UNIT_PCA_SFF_0              , 1   * _1M )
    ,UNIT_INFO_MAC(TSU_0_BASE_ADDR, UNIT_PCA_CTSU_0       ,512  * _1K )

    ,UNIT_INFO_MAC(0x90d80000,UNIT_PCA_MACSEC_EXT_E_163_0 ,512 * _1K )
    ,UNIT_INFO_MAC(0x90700000,UNIT_PCA_MACSEC_EXT_I_163_0 ,512 * _1K )
    ,UNIT_INFO_MAC(0x91180000,UNIT_PCA_MACSEC_EXT_E_164_0 ,512 * _1K )
    ,UNIT_INFO_MAC(0x90900000,UNIT_PCA_MACSEC_EXT_I_164_0 ,512 * _1K )

    ,UNIT_INFO_MAC(0x90800000,UNIT_PCA_MACSEC_EIP_163_I_0 , 1  * _1M )
    ,UNIT_INFO_MAC(0x90e00000,UNIT_PCA_MACSEC_EIP_163_E_0 , 1  * _1M )
    ,UNIT_INFO_MAC(0x90a00000,UNIT_PCA_MACSEC_EIP_164_I_0 , 1  * _1M )
    ,UNIT_INFO_MAC(0x91200000,UNIT_PCA_MACSEC_EIP_164_E_0 , 1  * _1M )

    /* DFX */
    ,UNIT_INFO_MAC(0x84000000, UNIT_DFX_SERVER       ,2  * _1M)

    ,UNIT_INFO_MAC(0x84400000,UNIT_TTI               ,4   * _1M)

    /* TAI 0,1 */
    ,UNIT_INFO_MAC(0x8c400000,UNIT_TAI               ,    1  * _1M )
    ,UNIT_INFO_MAC(0x8c500000,UNIT_TAI_1             ,    1  * _1M )
    ,UNIT_INFO_MAC(0x8c600000,UNIT_TAI_2             ,    1  * _1M )
    ,UNIT_INFO_MAC(0x8c700000,UNIT_TAI_3             ,    1  * _1M )
    ,UNIT_INFO_MAC(0x8c800000,UNIT_TAI_4             ,    1  * _1M )

/*    ,UNIT_INFO_MAC(0x01d00000,UNIT_PPU               ,1   * _1M)*/

/*    ,UNIT_INFO_MAC(0x08000000,UNIT_PHA               ,8   * _1M)no such unit */
    ,UNIT_INFO_MAC(0x8bc00000,UNIT_EPLR              ,4   * _1M)
/*    ,UNIT_INFO_MAC(0x0a000000,UNIT_EOAM              ,1   * _1M)no such unit */
    ,UNIT_INFO_MAC(0x8b800000,UNIT_EPCL              ,4 *     _1M)
    ,UNIT_INFO_MAC(0x8a600000,UNIT_PREQ              ,2 * _1M)
    ,UNIT_INFO_MAC(0x8a800000,UNIT_ERMRK             ,8   * _1M)
    ,UNIT_INFO_MAC(0x8a400000,UNIT_EREP              ,2   * _1M)
    ,UNIT_INFO_MAC(0x85200000,UNIT_BMA               ,2   * _1M)
    ,UNIT_INFO_MAC(0x8c000000,UNIT_HBU               ,4   * _1M)
    ,UNIT_INFO_MAC(0x8b000000,UNIT_HA                ,8   * _1M)

    ,UNIT_INFO_MAC(0x84800000,UNIT_TCAM              ,8   * _1M)
/*    ,UNIT_INFO_MAC(0x05000000,UNIT_EM                ,4   * _1M) no such unit */
    ,UNIT_INFO_MAC(0x85400000,UNIT_FDB               ,4  *  _1M)
/*  ,UNIT_INFO_MAC(0x--------,UNIT_SHM               ,64  *     _1K) no such unit */

    ,UNIT_INFO_MAC(0x84200000,UNIT_IPCL              ,2 *   _1M)

    ,UNIT_INFO_MAC(0x85000000,UNIT_CNC               ,2 *   _1M)

    ,UNIT_INFO_MAC(0x8a100000,UNIT_EGF_QAG           ,1   * _1M)
    ,UNIT_INFO_MAC(0x8a200000,UNIT_EGF_SHT           ,2   * _1M)
    ,UNIT_INFO_MAC(0x8a000000,UNIT_EGF_EFT           ,1   * _1M)

    ,UNIT_INFO_MAC(0x89400000,UNIT_IPLR              ,4   * _1M)
    ,UNIT_INFO_MAC(0x89800000,UNIT_IPLR1             ,4   * _1M)
    ,UNIT_INFO_MAC(0x89c00000,UNIT_SMU               ,4   * _1M)

    ,UNIT_INFO_MAC(0x88000000,UNIT_EQ                ,16  * _1M)

    ,UNIT_INFO_MAC(0x89200000,UNIT_IOAM              ,1   * _1M)
    ,UNIT_INFO_MAC(0x89000000,UNIT_MLL               ,2   * _1M)

    ,UNIT_INFO_MAC(0x85800000,UNIT_IPVX              ,8   * _1M)
    ,UNIT_INFO_MAC(0x87000000,UNIT_LPM               ,16  * _1M)
    ,UNIT_INFO_MAC(0x86000000,UNIT_L2I               ,16  * _1M)

    ,UNIT_INFO_MAC(0x93700000,UNIT_LED_0             ,1 * _1M)
    ,UNIT_INFO_MAC(0x98700000,UNIT_LED_1             ,1 * _1M)
    ,UNIT_INFO_MAC(0x99700000,UNIT_LED_2             ,1 * _1M)

    /*NOTE: MG0 is part of the CnM : 0x3D000000 size 1M */
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 0*MG_SIZE ,UNIT_MG    /*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 1*MG_SIZE ,UNIT_MG_0_1/*CNM*/, MG_SIZE)
    ,UNIT_INFO_MAC(MG_0_0_OFFSET_CNS + 2*MG_SIZE ,UNIT_MG_0_2/*CNM*/, MG_SIZE)

    /* RFU is in the CnM section */
    ,UNIT_INFO_MAC(0x800B0000,UNIT_CNM_RFU    ,64   *     _1K) /*in CnM */
    /* MPP_RFU is in the CnM section */
    ,UNIT_INFO_MAC(0x80020000 ,UNIT_CNM_MPP_RFU,64   *     _1K) /*in CnM */

    /* SMI is in the CnM section */
    ,UNIT_INFO_MAC(0x80580000 ,UNIT_GOP_SMI_0  ,64   *     _1K) /*in CnM */
    ,UNIT_INFO_MAC(0x80590000 ,UNIT_GOP_SMI_1  ,64   *     _1K) /*in CnM */
     /* AAC is in the CnM section */
    ,UNIT_INFO_MAC(0x80030000,UNIT_CNM_AAC    ,64   *     _1K) /*in CnM */

    /* SRAM  is in the CnM section */
    ,UNIT_INFO_MAC(0x7FE00000,UNIT_CNM_SRAM    ,64   *     _1K) /*in CnM */

     /* PEX MAC is in the CnM section */
    ,UNIT_INFO_MAC(0x800A0000,UNIT_CNM_PEX_MAC   ,64   *     _1K)/*in CnM */

    /* TSU (PTP) - moved into PCA */

    ,UNIT_INFO_MAC(MTIP_USX_MAC_0_BASE_ADDR,  UNIT_USX_0_MAC_0            ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_1_BASE_ADDR,  UNIT_USX_MAC_1              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_2_BASE_ADDR,  UNIT_USX_MAC_2              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_3_BASE_ADDR,  UNIT_USX_MAC_3              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_4_BASE_ADDR,  UNIT_USX_MAC_4              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_5_BASE_ADDR,  UNIT_USX_MAC_5              ,64 * _1K)
    ,UNIT_INFO_MAC(MTIP_USX_MAC_6_BASE_ADDR,  UNIT_USX_MAC_6              ,64 * _1K)

    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_0_BASE_ADDR,  UNIT_USX_0_PCS_0            ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_1_BASE_ADDR,  UNIT_USX_PCS_1              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_2_BASE_ADDR,  UNIT_USX_PCS_2              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_3_BASE_ADDR,  UNIT_USX_PCS_3              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_4_BASE_ADDR,  UNIT_USX_PCS_4              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_5_BASE_ADDR,  UNIT_USX_PCS_5              ,32 * _1K)
    ,UNIT_INFO_MAC(MTIP_UNIT_USX_PCS_6_BASE_ADDR,  UNIT_USX_PCS_6              ,32 * _1K)

    /*MIF */
    ,UNIT_INFO_MAC(MIF_0_BASE_ADDR, UNIT_MIF_0              ,1   *     _1M)
    ,UNIT_INFO_MAC(MIF_1_BASE_ADDR, UNIT_MIF_1              ,1   *     _1M)
    ,UNIT_INFO_MAC(MIF_2_BASE_ADDR, UNIT_MIF_2              ,1   *     _1M)
    ,UNIT_INFO_MAC(MIF_3_BASE_ADDR, UNIT_MIF_3              ,1   *     _1M)

    /*ANP */
    ,UNIT_INFO_MAC(ANP_0_BASE_ADDR, UNIT_ANP_0              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_1_BASE_ADDR, UNIT_ANP_1              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_2_BASE_ADDR, UNIT_ANP_2              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_3_BASE_ADDR, UNIT_ANP_3              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_4_BASE_ADDR, UNIT_ANP_4              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_5_BASE_ADDR, UNIT_ANP_5              ,64   *     _1K)
    ,UNIT_INFO_MAC(ANP_6_BASE_ADDR, UNIT_ANP_6              ,64   *     _1K)

    /* must be last */
    ,{SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};

#define sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr 0

/* info about the 1/2.5/5G MACs */
static MAC_NUM_INFO_STC ironmanUsxPortsArr[] = {
    /*global*/      /*mac*/   /* tsu */   /*sip6_MTI_EXTERNAL_representativePortIndex*/
     {0  ,           {0, 0} ,    {0,  0}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{1  ,           {0, 1} ,    {0,  1}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{2  ,           {0, 2} ,    {0,  2}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{3  ,           {0, 3} ,    {0,  3}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{4  ,           {0, 4} ,    {0,  4}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{5  ,           {0, 5} ,    {0,  5}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{6  ,           {0, 6} ,    {0,  6}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{7  ,           {0, 7} ,    {0,  7}      ,0+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{ 8 ,           {1, 0} ,    {0,  8}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{ 9 ,           {1, 1} ,    {0,  9}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{10 ,           {1, 2} ,    {0, 10}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{11 ,           {1, 3} ,    {0, 11}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{12 ,           {1, 4} ,    {0, 12}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{13 ,           {1, 5} ,    {0, 13}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{14 ,           {1, 6} ,    {0, 14}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{15 ,           {1, 7} ,    {0, 15}      ,1+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{16 ,           {2, 0} ,    {0, 16}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{17 ,           {2, 1} ,    {0, 17}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{18 ,           {2, 2} ,    {0, 18}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{19 ,           {2, 3} ,    {0, 19}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{20 ,           {2, 4} ,    {0, 20}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{21 ,           {2, 5} ,    {0, 21}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{22 ,           {2, 6} ,    {0, 22}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{23 ,           {2, 7} ,    {0, 23}      ,2+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{24 ,           {3, 0} ,    {0, 24}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{25 ,           {3, 1} ,    {0, 25}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{26 ,           {3, 2} ,    {0, 26}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{27 ,           {3, 3} ,    {0, 27}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{28 ,           {3, 4} ,    {0, 28}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{29 ,           {3, 5} ,    {0, 29}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{30 ,           {3, 6} ,    {0, 30}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{31 ,           {3, 7} ,    {0, 31}      ,3+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{32 ,           {4, 0} ,    {0, 32}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{33 ,           {4, 1} ,    {0, 33}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{34 ,           {4, 2} ,    {0, 34}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{35 ,           {4, 3} ,    {0, 35}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{36 ,           {4, 4} ,    {0, 36}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{37 ,           {4, 5} ,    {0, 37}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{38 ,           {4, 6} ,    {0, 38}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{39 ,           {4, 7} ,    {0, 39}      ,4+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{40 ,           {5, 0} ,    {0, 40}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{41 ,           {5, 1} ,    {0, 41}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{42 ,           {5, 2} ,    {0, 42}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{43 ,           {5, 3} ,    {0, 43}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{44 ,           {5, 4} ,    {0, 44}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{45 ,           {5, 5} ,    {0, 45}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{46 ,           {5, 6} ,    {0, 46}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{47 ,           {5, 7} ,    {0, 47}      ,5+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{48 ,           {6, 0} ,    {0, 48}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{49 ,           {6, 1} ,    {0, 49}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{50 ,           {6, 2} ,    {0, 50}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{51 ,           {6, 3} ,    {0, 51}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{52 ,           {6, 4} ,    {0, 52}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{53 ,           {6, 5} ,    {0, 53}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }
    ,{54 ,           {6, 6} ,    {0, 54}      ,6+sip6_MTI_EXTERNAL_representativePortIndex_base_ironmanUsxPortsArr    }

    ,{SMAIN_NOT_VALID_CNS,{0,0},{0,0}}
};

/* number of units in the device */
#define IRONMAN_NUM_UNITS sizeof(ironman_units)/sizeof(ironman_units[0])

static SMEM_GEN_UNIT_INFO_STC SORTED___ironman_units[IRONMAN_NUM_UNITS] =
{
    /* sorted and build during smemIronmanInit(...) from ironman_units[] */
    {SMAIN_NOT_VALID_CNS,NULL,SMAIN_NOT_VALID_CNS}
};


/* NOTE: all units that are duplicated from pipe 0 to pipe 1 are added into this array in runtime !!!
    it is built from falcon_units[].orig_nameStr*/
static SMEM_UNIT_DUPLICATION_INFO_STC ironman_duplicatedUnits[] =
{
    /* those explicitly listed here need unit allocation as are not duplicated within each pipe */

    {STR(UNIT_USX_0_MAC_0)  ,6},  /* 6 more per device */
        {STR(UNIT_USX_MAC_1)},
        {STR(UNIT_USX_MAC_2)},
        {STR(UNIT_USX_MAC_3)},
        {STR(UNIT_USX_MAC_4)},
        {STR(UNIT_USX_MAC_5)},
        {STR(UNIT_USX_MAC_6)},

    {STR(UNIT_USX_0_PCS_0)  ,6},  /* 2 more per device */
        {STR(UNIT_USX_PCS_1)},
        {STR(UNIT_USX_PCS_2)},
        {STR(UNIT_USX_PCS_3)},
        {STR(UNIT_USX_PCS_4)},
        {STR(UNIT_USX_PCS_5)},
        {STR(UNIT_USX_PCS_6)},

    {STR(UNIT_PCA_MACSEC_EXT_I_163_0)  ,3},  /* 3 more per device */
        {STR(UNIT_PCA_MACSEC_EXT_E_163_0)},
        {STR(UNIT_PCA_MACSEC_EXT_I_164_0)},
        {STR(UNIT_PCA_MACSEC_EXT_E_164_0)},

    {STR(UNIT_MIF_0)  ,3},  /* 2 more per device */
        {STR(UNIT_MIF_1)},
        {STR(UNIT_MIF_2)},
        {STR(UNIT_MIF_3)},

    {STR(UNIT_ANP_0)  ,6},  /* 2 more per device */
        {STR(UNIT_ANP_1)},
        {STR(UNIT_ANP_2)},
        {STR(UNIT_ANP_3)},
        {STR(UNIT_ANP_4)},
        {STR(UNIT_ANP_5)},
        {STR(UNIT_ANP_6)},

    {STR(UNIT_TAI)  ,4},  /* 4 more per device */
        {STR(UNIT_TAI_1)},
        {STR(UNIT_TAI_2)},
        {STR(UNIT_TAI_3)},
        {STR(UNIT_TAI_4)},

    {STR(UNIT_LED_0)  ,2},  /* 2 more per device */
        {STR(UNIT_LED_1)},
        {STR(UNIT_LED_2)},

   {STR(UNIT_MG)        ,1}, /* 1 duplication of this unit */
        {STR(UNIT_MG_0_1)},

    {NULL,0} /* must be last */
};

#define CPU_SDMA    56
/* DMA : special ports mapping {global,local,DP}    */
static SPECIAL_PORT_MAPPING_CNS ironman_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
/*56*/ {CPU_SDMA/*global DMA port*/,CPU_SDMA/*local DMA port*/,0/*DP[0]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/**
* @internal smemIronmanGetMgUnitIndexFromAddress function
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
static GT_U32  smemIronmanGetMgUnitIndexFromAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  regAddress
)
{
    if(regAddress >= MG_0_0_OFFSET_CNS &&
       regAddress < (MG_0_0_OFFSET_CNS + MG_SIZE))
    {
        return (regAddress - MG_0_0_OFFSET_CNS)/MG_SIZE;
    }

    return SMAIN_NOT_VALID_CNS;
}

/**
* @internal smemIronmanGopPortByAddrGet function
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
static GT_U32 smemIronmanGopPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32 unitIndex;
    GT_U32 ii;
    GT_U32 localPortInUnit = 0;
    GT_U32 unitBase;

    for(ii = 0; ii < SIM_MAX_ANP_UNITS; ii++)
    {
        if(address >=  devObjPtr->memUnitBaseAddrInfo.anp[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.anp[ii] + _64K) )
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

    if(ii < SIM_MAX_ANP_UNITS)
    {
        /* matched as address in USX_ANP  */
        return ii*8;
    }
    else
    {
        /* match not found - continue */
    }
    /**********************/
    /* End of ANP section */
    /**********************/

    for(ii = 0 ;
        ii < 7;
        ii++)
    {
        /* usx macs */
        if(address >=  devObjPtr->memUnitBaseAddrInfo.macWrap[ii] &&
           address <  (devObjPtr->memUnitBaseAddrInfo.macWrap[ii] + MAC_UNIT_SIZE) )
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
           address <  (devObjPtr->memUnitBaseAddrInfo.macPcs[ii] + PCS_UNIT_SIZE) )
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

    if(ii == 7)
    {
        skernelFatalError("smemIronmanGopPortByAddrGet : unknown address [0x%8.8x] as 'Gop unit' \n",
            address);
        return 0;
    }

    /* matched as address in USX-MAC  */

    unitIndex = ii;

    for(ii = 0 ; ironmanUsxPortsArr[ii].globalMacNum != SMAIN_NOT_VALID_CNS; ii++)
    {
        if(ironmanUsxPortsArr[ii].macInfo.unitIndex   == unitIndex &&
           ironmanUsxPortsArr[ii].macInfo.indexInUnit == localPortInUnit)
        {
            return ironmanUsxPortsArr[ii].globalMacNum;
        }
    }

    skernelFatalError("smemIronmanGopPortByAddrGet : in USX mac :  unknown port for address [0x%8.8x] as 'Gop unit' \n",
        address);
    return 0;
}

/* MIF unit+channe to Global Mac port table */
typedef struct
{
    GT_U8 unit;          /*0xFF - end of list */
    GT_U8 channelBase;
    GT_U8 channelAmount;
    GT_U8 globalMacBase;
} MIF_TO_MAC_RANGE_MAP_STC;

/* generic table used for convertion to both sides */
static const MIF_TO_MAC_RANGE_MAP_STC smemChtIronmanMIFtoMAC_MappingArr[] =
{
     {0,  0,  8,  0}
    ,{0,  8,  8, 24}
    ,{0, 16,  1, 52}
    ,{1,  0,  8,  8}
    ,{1,  8,  8, 32}
    ,{1, 16,  1, 53}
    ,{2,  0,  8, 16}
    ,{2,  8,  8, 40}
    ,{2, 16,  1, 54}
    ,{3,  0,  4, 48}
    ,{3,  4,  4, 52}
    ,{0xFF, 0 , 0, 0}
};

/* table only for special preemptive ports having other generic mapping */
static const MIF_TO_MAC_RANGE_MAP_STC smemChtIronmanMIFtoMAC_MappingPreemptiveArr[] =
{
     {3,  4,  4, 52}
    ,{0xFF, 0 , 0, 0}
};


/**
* @internal smemChtIronmanMifUintChannelToGlobalMac function
* @endinternal
*
* @brief   Convert Global MAC to MIF Unit Channel.
*
* @param[in]   globalPort         - Global MAC index.
* @param[in]   isPreemtive        - 1 - preemtive, otherwize - 0.
* @param[out]  mifUnitPtr         - (Pointer to) MIF unit index.
* @param[out]  mifChannelPtr      - (Pointer to) MIF Channel index.
*/
GT_STATUS smemChtIronmanGlobalMacToMifUintChannel
(
    IN   GT_U32   globalMac,
    IN   GT_U32   isPreemtive,
    OUT  GT_U32   *mifUnitPtr,
    OUT  GT_U32   *mifChannelPtr
)
{
    static const MIF_TO_MAC_RANGE_MAP_STC *searchTabArr[2] =
    {
        smemChtIronmanMIFtoMAC_MappingPreemptiveArr,
        smemChtIronmanMIFtoMAC_MappingArr
    };
    GT_U32 searchTabIdx;
    GT_U32 searchTabStartIdx;
    const MIF_TO_MAC_RANGE_MAP_STC *searchTabPtr;
    GT_U32 ii;
    GT_U32 amount;
    GT_U32 base;

    searchTabStartIdx = (isPreemtive ? 0 : 1);
    for (searchTabIdx = searchTabStartIdx; (searchTabIdx < 2); searchTabIdx++)
    {
        searchTabPtr = searchTabArr[searchTabIdx];
        for (ii = 0;(searchTabPtr[ii].unit != 0xFF); ii++)
        {
            base = searchTabPtr[ii].globalMacBase;
            if (globalMac < base) continue;
            amount = searchTabPtr[ii].channelAmount;
            if (globalMac >= (base + amount)) continue;

            /* found */
            *mifUnitPtr = searchTabPtr[ii].unit;
            *mifChannelPtr = (globalMac - base) + searchTabPtr[ii].channelBase;
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}

/**
* @internal smemChtIronmanMifUintChannelToGlobalMac function
* @endinternal
*
* @brief   Convert MIF Unit Channel to Global MAC.
*
* @param[in]  mifUnit                  - MIF unit index.
* @param[in]  mifChannel               - MIF Channel index.
* @param[out] globalPortPtr            - Pointer Global MAC index.
*/
GT_STATUS smemChtIronmanMifUintChannelToGlobalMac
(
    IN  GT_U32   mifUnit,
    IN  GT_U32   mifChannel,
    OUT GT_U32   *globalMacPtr
)
{
    GT_U32 ii;
    GT_U32 amount;
    GT_U32 base;

    for (ii = 0;(smemChtIronmanMIFtoMAC_MappingArr[ii].unit != 0xFF); ii++)
    {
        if (mifUnit != smemChtIronmanMIFtoMAC_MappingArr[ii].unit) continue;
        base = smemChtIronmanMIFtoMAC_MappingArr[ii].channelBase;
        if (mifChannel < base) continue;
        amount = smemChtIronmanMIFtoMAC_MappingArr[ii].channelAmount;
        if (mifChannel >= (base + amount)) continue;

        /* found */
        *globalMacPtr = (mifChannel - base) + smemChtIronmanMIFtoMAC_MappingArr[ii].globalMacBase;
        return GT_OK;
    }
    return GT_NOT_FOUND;
}

/*******************************************************************************
*   smemIronmanPortInfoGet
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
static GT_STATUS smemIronmanPortInfoGet
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
    GT_STATUS   rc;
    GT_U32      mifUnit;
    GT_U32      mifChannel;
    GT_U32      isPreemtive;

    if(unitType == SMEM_UNIT_TYPE_MTI_MAC_50G_E ||
       unitType == SMEM_UNIT_TYPE_MTI_MAC_CPU_E)
    {
        /* support generic code , that check if port supports 50G or CPU port */
        /* currently we support only USX ports */
        return GT_NOT_FOUND;
    }

    switch(unitType)
    {
        case SMEM_UNIT_TYPE_EPI_MIF_TYPE_E:
            portInfoPtr->simplePortInfo.unitIndex   = SMEM_CHT_PORT_MTI_MIF_TYPE_8_E;
            portInfoPtr->simplePortInfo.indexInUnit = 0;/* not used */
            portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = 0;/*not used*/
            portInfoPtr->sip6_MTI_bmpPorts[0] = 0;
            portInfoPtr->sip6_MTI_bmpPorts[1] = 0;
            return GT_OK;
            /* break here can never be accessed */

        case SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E:
            isPreemtive = smemGetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL);
            rc = smemChtIronmanGlobalMacToMifUintChannel(
                portNum, isPreemtive, &mifUnit, &mifChannel);
            if (rc != GT_OK)
            {
                skernelFatalError(
                    "smemIronmanPortInfoGet : MAC [%d] map to MIF Channel not foud\n", portNum);
                return GT_NOT_FOUND;
            }

            portInfoPtr->simplePortInfo.unitIndex   = mifUnit;
            portInfoPtr->simplePortInfo.indexInUnit = mifChannel;
            portInfoPtr->sip6_MTI_bmpPorts[0] = 0;
            portInfoPtr->sip6_MTI_bmpPorts[1] = 0;
            portInfoPtr->sip6_MTI_EXTERNAL_representativePortIndex = mifUnit;
            return GT_OK;
            /* break here can never be accessed */

        default:
            break; /* treat other cases below */
    }

    /*special case*/
    if(unitType == SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E)
    {
        GT_U32  mifUnitId = portNum;/* !!! the parameter used as mifUnitId !!! */
        GT_U32  dpUnitIndex = 0;
        GT_U32  IN_indexInUnit = portInfoPtr->simplePortInfo.indexInUnit;/* !!! the parameter used as input !!! */
        GT_U32  OUT_indexInUnit = mifUnitId*16/*USX ports*/ + IN_indexInUnit;
        GT_BIT  isPreemptiveChannel = 0;

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
            jjMax = 1;
            sip6_MTI_bmpPorts = 0;
            break;
        case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
            sip6_MTI_bmpPorts = 0x000000ff;
            break;
        case SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E:
            skernelFatalError("smemIronmanPortInfoGet : 'LMU' is not exists in the device \n");
            return GT_NOT_FOUND;
        default:
            skernelFatalError("smemIronmanPortInfoGet : unknown type [%d] \n",unitType);
            return GT_NOT_FOUND;

    }

    for(jj = 0 ; jj < jjMax ; jj++)
    {
        switch(unitType)
        {
            case SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E:
            case SMEM_UNIT_TYPE_MTI_MAC_USX_E:
                tmpMacInfoPtr = ironmanUsxPortsArr;
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

static SMEM_UNIT_NAME_AND_INDEX_STC ironmanUnitNameAndIndexArr[IRONMAN_NUM_UNITS]=
{
    /* filled in runtime from ironman_units[] */
    /* must be last */
    {NULL ,                                SMAIN_NOT_VALID_CNS                     }
};
/* the addresses of the units that the ironman uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   ironmanUsedUnitsAddressesArray[IRONMAN_NUM_UNITS]=
{
    {0,0}    /* filled in runtime from ironman_units[] */
};

/* build once the sorted memory for the falcon .. for better memory search performance

    that use by :

    devObjPtr->devMemUnitNameAndIndexPtr      = ironmanUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = ironmanUsedUnitsAddressesArray;


*/
static void build_SORTED___ironman_units(void)
{
    GT_U32  numValidElem,ii;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &ironman_units[0];

    if(SORTED___ironman_units[0].base_addr != SMAIN_NOT_VALID_CNS)
    {
        /* already initialized */
        return;
    }

    memcpy(SORTED___ironman_units,ironman_units,sizeof(ironman_units));

    numValidElem = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        numValidElem++;
    }

    qsort(SORTED___ironman_units, numValidElem, sizeof(SMEM_GEN_UNIT_INFO_STC),
          sim_sip6_units_cellCompare);

}


static void buildDevUnitAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* build
        ironmanUsedUnitsAddressesArray - the addresses of the units that the Falcon uses
        ironmanUnitNameAndIndexArr - name of unit and index in ironmanUsedUnitsAddressesArray */
    GT_U32  ii;
    GT_U32  index;
    SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &SORTED___ironman_units[0];

    /* build once the sorted memory for the falcon .. for better memory search performance */
    build_SORTED___ironman_units();

    index = 0;
    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        ironmanUsedUnitsAddressesArray[index].unitBaseAddr = unitInfoPtr->base_addr ;
        ironmanUsedUnitsAddressesArray[index].unitSizeInBytes = unitInfoPtr->size;
        ironmanUnitNameAndIndexArr[index].unitNameIndex = index;
        ironmanUnitNameAndIndexArr[index].unitNameStr = unitInfoPtr->nameStr;
        index++;
    }

    if(index >= (sizeof(ironmanUnitNameAndIndexArr) / sizeof(ironmanUnitNameAndIndexArr[0])))
    {
        skernelFatalError("buildDevUnitAddr : over flow of units (3) \n");
    }
    /* indication of no more */
    ironmanUnitNameAndIndexArr[index].unitNameIndex = SMAIN_NOT_VALID_CNS;
    ironmanUnitNameAndIndexArr[index].unitNameStr = NULL;
    ironmanUsedUnitsAddressesArray[index].unitBaseAddr = SMAIN_NOT_VALID_CNS;
    ironmanUsedUnitsAddressesArray[index].unitSizeInBytes = 0;

    devObjPtr->devMemUnitNameAndIndexPtr = ironmanUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = ironmanUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = index+1;
    devObjPtr->devMemUnitPipeOffsetGet = NULL;/* no pipe offset */
    devObjPtr->support_memoryRanges = 1;

#if 0 /* check that the array is ascending ! (ironmanUsedUnitsAddressesArray) */
    for(ii = 0 ; ii < (index+1) ; ii++)
    {
        printf("unitBaseAddr = [0x%8.8x] \n",
            ironmanUsedUnitsAddressesArray[ii].unitBaseAddr);

        if(ii &&
            (ironmanUsedUnitsAddressesArray[ii].unitBaseAddr <=
             ironmanUsedUnitsAddressesArray[ii-1].unitBaseAddr))
        {
            printf("Error: at index[%d] prev index higher \n",ii);
            break;
        }
    }
#endif /*0*/
}

/* Active memory */

/**
* @internal smemChtActiveIronmanWriteMifChannelMappingReg function
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
void smemChtActiveIronmanWriteMifChannelMappingReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR   param,
    IN GT_U32 * inMemPtr
)
{
    GT_STATUS rc;
    SMEM_CHT_PORT_MTI_MIF_TYPE_ENT mifType = param;
    GT_U32   dpIndex;
    GT_U32   ii,mifUnitId,baseAddr,mifUnitIdMax=SIM_MAX_MIF_UNITS;
    GT_U32   local_mif_channel_number;/* local mif channel number */
    GT_U32   txDmaGlobalPortNum; /* global dma port   */
    GT_U32   old_reg_value; /* old register value */
    GT_U32   new_reg_value; /* new register value */
    GT_U32   old_local_dma_number; /* old DMA number before the change */
    GT_U32   new_local_dma_number; /* new DMA number after  the change */
    GT_U32   old_tx_enable; /* old TX enable before the change */
    GT_U32   new_tx_enable; /* new TX enable after  the change */
    GT_U32   globalMacPort;      /* global MAC number */
    GT_U32   pipeId;
    SKERNEL_PORT_MIF_INFO_STC   *portMifInfoPtr;
    static  GT_U32  mifTypeAddrOffset[SMEM_CHT_PORT_MTI_MIF_TYPE___last___E] = {0x00000000,0x00000080,0x00000100};
    static  GT_U32  mifTypeFactor[SMEM_CHT_PORT_MTI_MIF_TYPE___last___E] = {1,1,4};
    GT_U32  emptyIndex;
    GT_BIT  isPreemptiveChannel;
    ENHANCED_PORT_INFO_STC portInfo;

    old_reg_value = (*memPtr);
    new_reg_value = (*inMemPtr);

    /* data to be written */
    *memPtr = *inMemPtr;

    /* old value */
    old_local_dma_number = old_reg_value & 0x3f;/* 6 bits */
    old_tx_enable        = (old_reg_value >> 6) & 1;
    /* new value */
    new_local_dma_number = new_reg_value & 0x3f;/* 6 bits */
    new_tx_enable        = (new_reg_value >> 6) & 1;

    if ((old_local_dma_number == new_local_dma_number)
        && (old_tx_enable == new_tx_enable))
    {
        /* not changed */
        return;
    }

    for (mifUnitId = 0; (mifUnitId < mifUnitIdMax); mifUnitId++)
    {
        baseAddr = devObjPtr->memUnitBaseAddrInfo.mif[mifUnitId];
        if ((address >= baseAddr) && (address < (baseAddr + (16*_1K))))
        {
            /* found unit */
            break;
        }
    }
    if (mifUnitId == mifUnitIdMax)
    {
        /* not found ?! */
        skernelFatalError("smemChtActiveIronmanWriteMifChannelMappingReg : mifUnitId was not enough for address [0x%8.8x] \n",
            address);

        return ;
    }

    /* 0xA00 - is the offset of per channel configuration registers */
    local_mif_channel_number = (((address & 0x3FFF)/*16K*/ - mifTypeAddrOffset[mifType]) - 0xA00) / 4;
    if(mifTypeFactor[mifType] > 1)
    {
        local_mif_channel_number *= mifTypeFactor[mifType];
    }

    /* convert mif unit id and local mif channel to dpIndex and local dma port num */
    portInfo.simplePortInfo.indexInUnit = local_mif_channel_number; /* !!! used as INPUT !!! */
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_UNIT_ID_TO_DP_E,
        mifUnitId,/* !!! not using 'mac number' but rather mifUnitId !!! */
        &portInfo);
    dpIndex             = portInfo.simplePortInfo.unitIndex;
    isPreemptiveChannel = portInfo.sip6_MTI_bmpPorts[0] & 0x1;/* bit 0 information ! */

    pipeId = smemGetCurrentPipeId(devObjPtr);

    /* convert dpIndex and local mac port to global mac number */
    rc = smemChtIronmanMifUintChannelToGlobalMac(
        mifUnitId, local_mif_channel_number, &globalMacPort);
    if (rc != GT_OK)
    {
        /* not found ?! */
        skernelFatalError(
            "smemChtActiveIronmanWriteMifChannelMappingReg : mifUnitId %d mif_channel %d no MAC found \n",
            mifUnitId, local_mif_channel_number);

        return ;
    }

    /* cleanup DB for old mapping */
    if (old_tx_enable)
    {
        /* convert dpIndex and local dma channel to global dma channel */
        smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
            devObjPtr,
            pipeId, /* current pipe */
            dpIndex, /* DP unit local to the current pipe */
            old_local_dma_number,
            GT_FALSE,/* CPU port not muxed ... not relevant */
            &txDmaGlobalPortNum);

        portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];

        for (ii = 0 ; (ii < SKERNEL_PORT_MIF_INFO_MAX_CNS); ii++,portMifInfoPtr++)
        {
            if (portMifInfoPtr->txEnabled == 0)
            {
                continue;
            }

            if (portMifInfoPtr->txMacNum == globalMacPort)
            {
                /* used by current mac */
                break;
            }
        }

        if (ii == SKERNEL_PORT_MIF_INFO_MAX_CNS)
        {
            skernelFatalError(
                "smemChtActiveIrfonmanWriteMifChannelMappingReg : removed mapping MAC %d to TX_DMA %d not found \n",
                globalMacPort, txDmaGlobalPortNum);
            return;
        }

        /* the MAC not exists in the array. need to use the empty index */
        portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[ii];
        portMifInfoPtr->txEnabled = 0;
    }

    /* update DB for the new mapping */
    if (new_tx_enable)
    {
        /* convert dpIndex and local dma channel to global dma channel */
        smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
            devObjPtr,
            pipeId, /* current pipe */
            dpIndex, /* DP unit local to the current pipe */
            new_local_dma_number,
            GT_FALSE,/* CPU port not muxed ... not relevant */
            &txDmaGlobalPortNum);

        emptyIndex     = SMAIN_NOT_VALID_CNS;
        portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];

        for (ii = 0; (ii < SKERNEL_PORT_MIF_INFO_MAX_CNS); ii++,portMifInfoPtr++)
        {
            if (portMifInfoPtr->txEnabled == 0)
            {
                /* the entry not used */
                if (emptyIndex == SMAIN_NOT_VALID_CNS)
                {
                    /* empty - can be used */
                    emptyIndex = ii;
                }

                continue;
            }

            if (portMifInfoPtr->txMacNum == globalMacPort)
            {
                /* used already by current mac */
                emptyIndex = ii;
                break;
            }
        }

        if ((ii == SKERNEL_PORT_MIF_INFO_MAX_CNS) &&
           (emptyIndex == SMAIN_NOT_VALID_CNS))
        {
            skernelFatalError(
                "smemChtActiveIrfonmanWriteMifChannelMappingReg : SKERNEL_PORT_MIF_INFO_MAX_CNS [%d] is not enough ?! \n",
                SKERNEL_PORT_MIF_INFO_MAX_CNS);
            return;
        }

        /* the MAC not exists in the array. need to use the empty index */
        portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[emptyIndex];
        portMifInfoPtr->txEnabled = 1;
        portMifInfoPtr->txMacNum  = globalMacPort;
        portMifInfoPtr->mifType   = mifType;
        portMifInfoPtr->egress_isPreemptiveChannel = isPreemptiveChannel;
    }
}

/**
* @internal smemIronmanUnitInitTai function
* @endinternal
*
* @brief   Allocate address type specific memories - for the TAI units
*/
static void smemIronmanUnitInitTai
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

            /* dedicated register to emulate PHY timestamp queue with single entry */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000514)}


        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }
}

/**
* @internal smemIronmanSpecificDeviceUnitAlloc_TAI_units
*           function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemIronmanSpecificDeviceUnitAlloc_TAI_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
         {STR(UNIT_TAI)         ,smemIronmanUnitInitTai}
        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}

/**
* @internal smemIronmanUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/DP/<RXDMA> RXDMA IP TLU/Units/RxDMA IP Units*/
    /*IP: \Cider \EBU-IP \DP \RxDMA IP \RxDMA 8.0 - Ironman-L \RXDMA IP {IronmanL_20201124} \RXDMA IP TLU \Units \RxDMA IP Units*/
    /*Mask: \Cider \EBU-IP \DP \RxDMA IP \RxDMA 8.0 - Ironman-L \RXDMA IP IMXL {IronmanL_20201124}*/
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D80, 0x00001DA0)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000C00, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000013f0, 4*(NUM_PORTS_PER_DP_UNIT+1))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001500, 4*(NUM_PORTS_PER_DP_UNIT+1))}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        update_mem_chunk_from_size_to_size (chunksMem,numOfChunks,
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
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x00000036,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000037,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x000000C8,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000028,         0x0000ffff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002C,         0x00000008,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0x00600062,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x03d90007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0x0007ffff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000104,         0x00000002,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000108,         0x000000ff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000010c,         0x00003fff,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x00000407,     (NUM_PORTS_PER_DP_UNIT+1),    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x00000001,     (NUM_PORTS_PER_DP_UNIT+1),    0x4,        }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b04,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b08,         0x00000002,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b0c,         0x00000003,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b10,         0x00000004,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b14,         0x00000005,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b18,         0x00000006,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b1c,         0x00000007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b20,         0x00000008,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b24,         0x00000009,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b28,         0x0000000a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b2c,         0x0000000b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b30,         0x0000000c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b34,         0x0000000d,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b38,         0x0000000e,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b3c,         0x0000000f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b40,         0x00000010,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b44,         0x00000011,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b48,         0x00000012,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b4c,         0x00000013,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b50,         0x00000014,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b54,         0x00000015,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b58,         0x00000016,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b5c,         0x00000017,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b60,         0x00000018,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b64,         0x00000019,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b68,         0x0000001a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b6c,         0x0000001b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b70,         0x0000001c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b74,         0x0000001d,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b78,         0x0000001e,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b7c,         0x0000001f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b80,         0x00000020,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b84,         0x00000021,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b88,         0x00000022,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b8c,         0x00000023,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b90,         0x00000024,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b94,         0x00000025,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b98,         0x00000026,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b9c,         0x00000027,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000ba0,         0x00000028,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000ba4,         0x00000029,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000ba8,         0x0000002a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bac,         0x0000002b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bb0,         0x0000002c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bb4,         0x0000002d,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bb8,         0x0000002e,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bbc,         0x0000002f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bc0,         0x00000030,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bc4,         0x00000031,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bc8,         0x00000032,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bcc,         0x00000033,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bd0,         0x00000034,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bd4,         0x00000035,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bd8,         0x00000036,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000bdc,         0x00000037,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000be0,         0x00000038,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c00,         0x00000001,     (NUM_PORTS_PER_DP_UNIT+1),    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001000,         0x00000003,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001240,         0x00000800,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001244,         0x000086dd,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001258,         0x00008847,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000125c,         0x00008848,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013d0,         0x00018100,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013e0,         0x00008100,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x000013e4,         0x00008a88,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c00,         0xffffffff,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c04,         0xffffffff,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001c08,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d00,         0xffff0000,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d80,         0x00000005,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001d88,         0xffffffff,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e80,         0x00005555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e84,         0x00055555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e88,         0x55555555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e8c,         0x01555555,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e90,         0x01555555,      2,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e94,         0x00000015,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e98,         0x00005555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001e9c,         0x00055555,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001ea0,         0x55555555,      4,    0x4         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f08,         0x00000033,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f0c,         0x0000004c,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f88,         0x0000000b,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00001f8c,         0x0000002a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002008,         0x0000017f,      5,    0x10        }
            ,{DUMMY_NAME_PTR_CNS,            0x00002088,         0x00000006,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108,         0x000000ff,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000210c,         0x00000080,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002188,         0x000001ff,      2,    0x10        }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x0000000a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220c,         0x0000000a,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002288,         0x00000001,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x0000228c,         0x00000007,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002308,         0x0000001f,      1,    0x0         }
            ,{DUMMY_NAME_PTR_CNS,            0x00002388,         0x0000000f,      1,    0x0         }
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        update_list_register_default_size_to_size(myUnit_registersDefaultValueArr,
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
                            0x0000b00+n*0x4;
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
                            0x0000c00+n*0x4;
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
* @internal smemIronmanUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/DP/<TXDMA> TXD IP TLU/Units/TXD*/
    /*IP: \Cider \EBU-IP \DP \TxDMA IP \TxDMA 8.0 - Ironman-L \TXD IP {8.0.0} \TXD IP TLU \Units \TXD*/
    /*Mask: \Cider \EBU-IP \DP \TxDMA IP \TxDMA 8.0 - Ironman-L \TXD IP Ironman-L {8.0.0}*/

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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x0000062C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x0000072C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000092C)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003100, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003200, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003300, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003400, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003500, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003600, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003700, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007018, 0x0000701C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007020, 0x00007024)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        update_mem_chunk_from_size_to_size (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x0000651f,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00000036,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000604,         0x00000019,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000608,         0x00000011,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000060c,         0x00000007,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000610,         0x00000006,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000614,         0x00000004,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000061c,         0x00000003,      2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000624,         0x00000002,      3,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000700,         0x36843684,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000704,         0x20162016,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000708,         0x10921092,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000070c,         0x05880588,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x05040504,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x04200420,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x03360336,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x02520252,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x01680168,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000900,         0x01390389,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000904,         0x01390198,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000908,         0x00410102,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000090c,         0x00210054,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000910,         0x00210044,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000914,         0x00110030,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000918,         0x00110025,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000091c,         0x00030015,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000920,         0x00020011,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000924,         0x00020008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000928,         0x00020008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000092c,         0x00020008,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b00,         0x3d5a0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b04,         0x1ead0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b08,         0x0f570100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b0c,         0x07ac0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b10,         0x06230046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b14,         0x03d60046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b18,         0x03120046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b1c,         0x01890046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b20,         0x00c50046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b24,         0x00630046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b28,         0x00280046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000b2c,         0x00000046,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c00,         0x13a10100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c04,         0x09d00100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c08,         0x04e80100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c0c,         0x02740100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c10,         0x01f60100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c14,         0x013a0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c18,         0x00fb0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c1c,         0x007d0100,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000c20,         0x003e0100,      4,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000d00,         0x000000c0,     12,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000e00,         0x00c000a0,     12,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x00000003,     2,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002008,         0x00000008,     (NUM_PORTS_PER_DP_UNIT+1-2),  0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002200,         0x00001800,     (NUM_PORTS_PER_DP_UNIT+1),    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300,         0x00000007,     (NUM_PORTS_PER_DP_UNIT+1),    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002400,         0x00c000a0,     (NUM_PORTS_PER_DP_UNIT+1),    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000001,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00005000,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x00000801,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        update_list_register_default_size_to_size(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);
    }

    {/*start of unit sip6_txDMA[] */
        {/*start of unit globalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.globalConfigs.txDMAGlobalConfig = 0x00000000;
        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigurations */
            {/*0x00000600  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < IRONMAN_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqMaxCredits[p] =
                        0x00000600  + p*0x4;
                    }/* end of loop p */
            }/*0x00000600  + p*0x4*/
            {/*0x00000700 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < IRONMAN_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.sdqThresholdBytes[p] =
                        0x00000700 + p*0x4;
                    }/* end of loop p */
            }/*0x00000700 + p*0x4*/
            {/*0x00000900 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < IRONMAN_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.maxCellsCredits[p] =
                        0x00000900 + p*0x4;
                    }/* end of loop p */
            }/*0x00000900 + p*0x4*/
            {/*0x00000b00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < IRONMAN_MAX_PROFILE_CNS; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interPacketRateLimiter[p] =
                        0x00000b00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000b00 + p*0x4*/
             {/*0x00000c00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < IRONMAN_MAX_PROFILE_CNS; p++) {
                  SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.speedProfileConfigs.interCellRateLimiter[p] =
                        0x00000c00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000c00 + p*0x4*/

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
            {/*0x00002200  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= RUNTIME_NUM_PORTS_PER_DP_UNIT ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[0].configs.channelConfigs.interPacketRateLimiterConfig[p] =
                        0x00002200 +p*0x4;
                }/* end of loop n */
            }/*0x00002200  + p*0x4*/
        }/*end of unit channelConfigs */

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

/**
* @internal smemIronmanUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/DP/<TXFIFO> TXF TLU/Units/TXF*/
    /*IP:  \Cider \EBU-IP \DP \TxFIFO IP \TxFIFO 8.0 - Ironman-L \TXF IP {8.0.0} \TXF TLU \Units \TXF*/
    /*Mask: \Cider \EBU-IP \DP \TxFIFO IP \TxFIFO 8.0 - Ironman-L \TXF IP Ironman-L {8.0.0}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001300, (4*(NUM_PORTS_PER_DP_UNIT+1)))}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x0000500C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x0000510C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005180, 0x0000519C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005280, 0x00005290)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005300, 0x00005304)}

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        update_mem_chunk_from_size_to_size (chunksMem,numOfChunks,
            4*(NUM_PORTS_PER_DP_UNIT+1),
            4*(RUNTIME_NUM_PORTS_PER_DP_UNIT+1));

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x000001a1,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00001300,         0x00000010,      (NUM_PORTS_PER_DP_UNIT+1),    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0xffff0000,      1,    0x0  }
            ,{DUMMY_NAME_PTR_CNS,            0x00005100,         0x0000001f,      1,    0x0  }


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;

        update_list_register_default_size_to_size(myUnit_registersDefaultValueArr,
            NUM_PORTS_PER_DP_UNIT+1,
            RUNTIME_NUM_PORTS_PER_DP_UNIT+1);
    }
    {/*start of unit sip6_txFIFO[] */
        {/*start of unit globalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].configs.globalConfigs.globalConfig1 = 0x00000000;
        }/*end of unit globalConfigs */

        {/*start of unit debug*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].debug.statusBadAddr = 0x00005000;
        }/*end of unit debug*/

        {/* interrupts - manually added */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].interrupts.txf_interrupt_cause = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO[0].interrupts.txf_interrupt_mask = 0x00004004;
        }/* interrupts - manually added */

     }/*end of unit sip6_txFIFO[] */
}

/* Start of Packet Buffer */
/**
* @internal smemIronmanUnitPacketBuffer_pbCenter function
* @endinternal
 *
*/
static void smemIronmanUnitPacketBuffer_pbCenter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCenter */
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PB/<PB_CENTER> pb_center/PB_CENTER*/
    /*IP: \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.2 - Phoenix \PB_CENTER {PB_IronmanL_20201208} \pb_center \PB_CENTER*/
    /*Mask:  \Cider \EBU-IP \Packet Buffer \PB_CENTER \PB_CENTER 1.2 - Phoenix \PB_CENTER {PB_IronmanL_20201208}*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000112C)} /*interrupts*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000125C)} /*debug*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001300)} /*revision*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000140C, 0x0000141C)} /*main registers*/
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemIronmanUnitPacketBuffer_pbCounter function
* @endinternal
 *
*/
static void smemIronmanUnitPacketBuffer_pbCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* pbCounter*/
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PB/<PB_COUNTER> pb_counter/PB_COUNTER*/
    /*IP: \Cider \EBU-IP \Packet Buffer \PB_COUNTER \PB_COUNTER {PB_IronmanL_20201209}*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \PB_COUNTER \PB_COUNTER {PB_IronmanL_20201209} \pb_counter \PB_COUNTER*/
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
* @internal smemIronmanUnitPacketBuffer_packetWrite function
* @endinternal
 *
*/
static void smemIronmanUnitPacketBuffer_packetWrite
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketWrite */
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PB/<GPC_PACKET_WRITE> gpc_packet_write/GPC_PACKET_WRITE*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_WRITE \GPC_PACKET_WRITE 8.0 - Ironman-L \GPC_PACKET_WRITE {PB_IronmanL_20201208} \gpc_packet_write \GPC_PACKET_WRITE*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPC_PACKET_WRITE \GPC_PACKET_WRITE 8.0 - Ironman-L \GPC_PACKET_WRITE {PB_IronmanL_20201208*/
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* registers space */
              {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000018)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000040)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000015C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000208)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000030C, 0x0000030C)}
             ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x000005FC)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemIronmanUnitPacketBuffer_packetRead function
* @endinternal
 *
*/
static void smemIronmanUnitPacketBuffer_packetRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* gpcPacketRead */
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PB/<GPR> gpr/GPR*/
    /*IP: \Cider \EBU-IP \Packet Buffer \GPR \GPR 8.0 - Ironman-L \GPR IP {PB_IronmanL_20201026} \gpr \GPR*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \GPR \GPR 8.0 - Ironman-L \GPR IP {PB_IronmanL_20201026}*/
    {
        static SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Configuration */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
             /* DFX */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000020)}
            /* Debug */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000050, 0x0000005C)}
            /*Interrupts*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
            /* Status.global*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            /* Status.global,event_counters*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x0000022C)}
            /* Status.channels*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000350, 0x0000039C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000450, 0x00000530)}
        };
        static GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr, chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemIronmanUnitPacketBuffer_sbmMc function
* @endinternal
 *
*/
static void smemIronmanUnitPacketBuffer_sbmMc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* sbmMc */
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PB/<SMB_MC> smb_mc/SMB_MC/MC*/
    /*IP: \Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 8.0 - Ironman-L \SMB_MC {PB_Ironman_20201208} \smb_mc \SMB_MC \MC*/
    /*Mask: \Cider \EBU-IP \Packet Buffer \SMB_MC \SMB_MC 8.0 - Ironman-L \SMB_MC_Ironman {PB_Ironman_20201208}*/
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

/**
* @internal smemIronmanUnitPcaArbiter function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Arbiter
*/
static void smemIronmanUnitPcaArbiter
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PCA/<PizArb> PizArb IP TLU/pzarb*/
    /*IP: \Cider \EBU-IP \PCA \PizArb \PizArb 2.0 \PizArb {iRTL9.0 201021.0} \PizArb IP TLU \pzarb*/
    /*Mask: \Cider \EBU-IP \PCA \PizArb \PizArb 2.0 \PizArb {64-CH Ironmanl1} {iRTL9.0 201021.0}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x000000FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x000007FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B10)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Pizza arbiter slot configuration */
             {DUMMY_NAME_PTR_CNS,            0x00000300,         0x0000003F,      320,    0x4      }
            /* Pizza arbiter control */
             ,{DUMMY_NAME_PTR_CNS,           0x00000B0C,         0x00000140,        1,    0x0      }
             ,{DUMMY_NAME_PTR_CNS,           0x00000B10,         0xFFFF0000,        1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemIronmanUnitPcaSffDefaultLLs function
* @endinternal
*
* @brief   default LLs registers values for PCA Sff
*/
static GT_U32 smemIronmanUnitPcaSffDefaultLLs
(
    IN struct SMEM_REGISTER_DEFAULT_VALUE_STRUCT *defRegsPtr,
    IN GT_U32 repCount
)
{
    GT_UNUSED_PARAM (defRegsPtr);
    return repCount;
}

/**
* @internal smemIronmanUnitPcaSff function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA Sff
*/
static void smemIronmanUnitPcaSff
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PCA/<SFF> SFF IP TLU/Units*/
    /*IP: \Cider \EBU-IP \PCA \SFF IP \SFF 2.0 \SFF IP {IM-64 pre-init} \SFF IP TLU \Units*/
    /*Mask: \Cider \EBU-IP \PCA \SFF IP \SFF 2.0 \SFF IP Ironmanl1-64 {IM-64 pre-init}*/
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        /*Channel - Channel config*/
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x000006FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000008FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000BFC)} /* 128 entries */
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
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000018, 0x00000018)}
        /*SFF Interrupt Summary*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000024)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x0000003C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x0000004C)}
        /*Selected CHID Statistics*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000060)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000094)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*Global Configurations */
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00008013,       1,    0x0      }
            /* Channeel Control Enable */
            ,{DUMMY_NAME_PTR_CNS,            0x00000600,         0x00000000,      64,    0x4      } /* diasable statistics */
            /* Channel Control Configuration - zeros*/
            /* LLs */
            ,{DUMMY_NAME_PTR_CNS,            0x00000A00,         0x00000000,      128,   0x4, 0,0, 0,0, &smemIronmanUnitPcaSffDefaultLLs}
            /* Channel Occupancy staistics */
            ,{DUMMY_NAME_PTR_CNS,            0x00001800,         0x00003F00,      64,    0x4      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemIronmanUnitPcaMacsecExt function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PCA MACSEC Extension
*/
static void smemIronmanUnitPcaMacsecExt
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/PCA/<MACSEC> MACSEC Wrapper IP TLU/Units_%b_%a*/
    /*IP: Cider \EBU-IP \PCA \MACSEC Wrapper IP \MACSEC Wrapper IP 2.0 \MACSEC Wrapper IP {iRTL_111120.1} \MACSEC Wrapper IP TLU \Units*/
    /*Mask: \Cider \EBU-IP \PCA \MACSEC Wrapper IP \MACSEC Wrapper IP 2.0 \MACSEC Wrapper IP {64-CH Ironmanl1} {iRTL_111120.1}*/
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
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000031C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000330, 0x0000035C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x0000047C)}
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
}

/**
* @internal smemIronmanUnitMif function
* @endinternal
*
* @brief   Allocate address type specific memories - for the MIF
*/
static void smemIronmanUnitMif
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    /* /Cider/EBU/Ironmanl/Ironmanl {Current}/IronmanL/Core/EPI/<MIF> MIF/MIF registers_%a*/
    /*IP: \Cider \EBU-IP \PCA \MIF IP \MIF IP 2.0 \MIF IP {iRTL9.0 201103.0} \MIF \MIF registers*/
    /*Mask: \Cider \EBU-IP \PCA \MIF IP \MIF IP 2.0 \MIF IP {64-CH Ironmanl1} {iRTL9.0 201103.0}*/

    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
        {0x00000A00 , 0xffffff83, NULL, 0, smemChtActiveIronmanWriteMifChannelMappingReg,   SMEM_CHT_PORT_MTI_MIF_TYPE_8_E /*mif type*/},
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* MIF GLOBAL - start */
            /* interrupt */
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            /* config */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x00000028)}
            /* MIF GLOBAL - end */

            /* MIF Channel - start */
            /* interrupt */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x000001C0)}
            /* statistics */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000340)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x000003C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000440)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000480, 0x000004C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000580, 0x000005C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000640)}
            /*status*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000880, 0x000008C0)}
            /*config*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A80, 0x00000AC0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B40)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B80, 0x00000BC0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C40)}
            /* MIF Channel - end */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*Mif Metal Fix*/
             {DUMMY_NAME_PTR_CNS, 0x00000024,         0xFFFF0000,      1,    0x0            }
            /*Mif Rx Status*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000800,         0x00000020,      17,   0x4            }
            /*Mif Tx Status*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000880,         0x00000020,      17,   0x4            }
            /*Mif Link Status Filter*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000028,         0x00000005,      1,    0x0            }
            /*Mif status Fsm Control */
            ,{DUMMY_NAME_PTR_CNS, 0x00000C00,         0x00000008,      17,   0x4            }
            /*Mif Channel Mapping */
            ,{DUMMY_NAME_PTR_CNS, 0x00000A00,         0x00203F3F,      17,   0x4            }
            ,{NULL,                      0,                                      0x00000000,      0,    0x0            }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemIronmanSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemIronmanSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr[] = {
    {
        STR(UNIT_RX_DMA)                       ,smemIronmanUnitRxDma            }
        ,{STR(UNIT_TX_DMA)                     ,smemIronmanUnitTxDma            }
        ,{STR(UNIT_TX_FIFO)                    ,smemIronmanUnitTxFifo           }
        ,{STR(UNIT_PB_CENTER_BLK)              ,smemIronmanUnitPacketBuffer_pbCenter        }
        ,{STR(UNIT_PB_COUNTER_BLK)             ,smemIronmanUnitPacketBuffer_pbCounter       }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_WRITE_0)  ,smemIronmanUnitPacketBuffer_packetWrite     }
        ,{STR(UNIT_PB_GPC_GRP_PACKET_READ_0)   ,smemIronmanUnitPacketBuffer_packetRead      }
        ,{STR(UNIT_PB_SHARED_MEMO_BUF_0)       ,smemIronmanUnitPacketBuffer_sbmMc           }
        ,{STR(UNIT_PCA_PZ_ARBITER_I_0)         ,smemIronmanUnitPcaArbiter        }
        ,{STR(UNIT_PCA_SFF_0)                  ,smemIronmanUnitPcaSff            }
        ,{STR(UNIT_PCA_MACSEC_EXT_I_163_0)     ,smemIronmanUnitPcaMacsecExt      }
        ,{STR(UNIT_MIF_0)                      ,smemIronmanUnitMif               }
        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr, allocUnitsArr);
    smemPhoenixSpecificDeviceUnitAllocDerived_DP_units(devObjPtr, allocUnitsArr);
}

/**
* @internal smemIronmanSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemIronmanSpecificDeviceUnitAlloc
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
        SMEM_GEN_UNIT_INFO_STC   *unitInfoPtr = &ironman_units[0];

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
    smemHarrierPexAndBar0DeviceUnitAlloc(devObjPtr);

    /* some SIP from Ironman */
    smemIronmanSpecificDeviceUnitAlloc_SIP_units(devObjPtr);
    /* some TXQ from Ironman */
    smemIronmanSpecificDeviceUnitAlloc_TXQ_units(devObjPtr);

    smemIronmanSpecificDeviceUnitAlloc_TAI_units(devObjPtr);

    /*CnM from Phoenix*/
    smemPhoenixSpecificDeviceUnitAlloc_Cnm_units(devObjPtr);
    /*DP from Ironman*/
    smemIronmanSpecificDeviceUnitAlloc_DP_units(devObjPtr);
    /*rest from Harrier*/
    smemHarrierSpecificDeviceUnitAlloc_main(devObjPtr);

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
* @internal smemIronmanInitRegDbDpUnits function
* @endinternal
*
* @brief   Init RegDb for DP units
*/
static void smemIronmanInitRegDbDpUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
    GT_U32  unitBaseAddress;

    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_SDQ0 , SIP6_TXQ_SDQ ,0);/*DP[0]*/

    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TXQ_QFC0 , SIP6_TXQ_QFC ,0);/*DP[0]*/

    /* set register addresses for sip6_rxDMA[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_RX_DMA   , sip6_rxDMA ,0);/*DP[0]*/

    /* set register addresses for sip6_txDMA[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_DMA   , sip6_txDMA ,0);/*DP[0]*/

    /* set register addresses for sip6_txFIFO[0..2])*/
    ALIGN_REG_DB_TO_MULTI_INSTANCE_UNIT_IN_PIPE_0_MAC(UNIT_TX_FIFO   , sip6_txFIFO ,0);/*DP[0]*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_TXQ_PDX , SIP6_TXQ_PDX);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PREQ,PREQ);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EREP,EREP);
/*    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_EM,EXACT_MATCH);no such unit */
/*    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PHA,PHA);no such unit */
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_SHM,SHM);

/*    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_PPU,PPU);*/

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_AAC, CNM.AAC);
    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_CNM_MPP_RFU, CNM.MPP_RFU);

    ALIGN_REG_DB_TO_UNIT_MAC(UNIT_SMU , SMU);
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
    smemIronmanInterruptTreeInit(devObjPtr);
}

static void regAddr_anp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
)
{
    GT_U32  unitOffset;
    SMEM_CHT_PORT_ANP_UNIT_REG_STC  *unitDbPtr;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32 unitIndex;

    if(GT_OK != smemIronmanPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_USX_E,portNum,&portInfo))
    {
        return;
    }

    if(portInfo.simplePortInfo.indexInUnit != 0)
    {
        return;   /* USX : ANP functionality works on single lane ports */
    }

    unitIndex = portNum/8;

    unitDbPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip_6_10_ANP[unitIndex].ANP;
    unitOffset = devObjPtr->memUnitBaseAddrInfo.anp[unitIndex];

    unitDbPtr->portInterruptCause[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x100; /*0x00000100*/
    unitDbPtr->portInterruptMask[portInfo.simplePortInfo.indexInUnit] = unitOffset + (portInfo.simplePortInfo.indexInUnit) * 0x2a4 + 0x104; /*0x00000104*/
    unitDbPtr->interruptSummaryCause = unitOffset + 0x94; /*0x00000094*/
    unitDbPtr->interruptSummaryMask = unitOffset + 0x98; /*0x00000098*/
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
    GT_U32  unitOffset;
    SMEM_CHT_PORT_MTI_PORT_MIF_REG_STC  *portDbPtr;

    unitOffset = devObjPtr->memUnitBaseAddrInfo.mif[unitIndex];
    portDbPtr = &(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MIF);
    portDbPtr->mif_channel_mapping_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E  ] = unitOffset + 0x00000A00 + portIndex*0x4;
    portDbPtr->mif_link_fsm_control_register[SMEM_CHT_PORT_MTI_MIF_TYPE_8_E ] = unitOffset + 0x00000C00 + portIndex*0x4;
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

        for(index=0; index<NUM_PORTS_PER_CTSU_UNIT; index++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].timestampFrameCntrControl[index] = 0x00000500 + 0x4 * index + unitOffset;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.TSU_GLOBAL[unitIndex].timestampFrameCntr[index]        = 0x00000100 + 0x4 * index + unitOffset;
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
* @internal regAddr_MTIPMacWrap function
* @endinternal
*
* @brief   initialize the register DB - MTIP MAC
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            is_SMEM_UNIT_TYPE_MTI_MAC_USX_E - indication if using MAC_100 or MAC_USX
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_MTIPMacWrap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
    IN GT_U32 sip6_MTI_EXTERNAL_representativePortIndex,
    IN GT_U32 portIndex,
    IN GT_U32 unitIndex
)
{
    GT_U32  indexTo_macWrap = unitIndex;

    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macWrap[indexTo_macWrap] + MAC_EXT_BASE_OFFSET;

    if (portIndex == 0)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalResetControl          = 0x00000000 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryCause = 0x00000018 + unitOffset;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].MTI_GLOBAL.globalInterruptSummaryMask  = 0x0000001C + unitOffset;
    }
    regAddr_USX_MTIP_EXT(devObjPtr, portNum, portIndex, indexTo_macWrap);
    regAddr_100GMac      (devObjPtr,portNum,portIndex,indexTo_macWrap);
    smemChtGopMtiInitMacMibCounters(devObjPtr,portNum ,portIndex , sip6_MTI_EXTERNAL_representativePortIndex/*channelIndex*/ , 0/*dieIndex*/ , 0 /*globalRaven*/ , 0/*isCpuPort*/);
}
/**
* @internal regAddr_MTIPPcs function
* @endinternal
*
* @brief   initialize the register DB - MTIP  PCS
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            is_SMEM_UNIT_TYPE_MTI_MAC_USX_E - indication if using MAC_100 or MAC_USX
*            portIndex - local port in the unit
*            unitIndex - the unit index 0..3
*
*/
static void regAddr_MTIPPcs
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
    GT_U32  indexTo_macPcs = unitIndex;
    GT_U32  unitOffset = devObjPtr->memUnitBaseAddrInfo.macPcs[indexTo_macPcs];

    portFormula = USX_PCS_PORTS_OFFSET + (USX_PCS_STEP_PORT_OFFSET * portIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.control1 = 0x00000000 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_PCS.status1  = 0x00000004 + portFormula + unitOffset;

    portFormula = USX_PCS_LPCS_OFFSET  + (USX_LPCS_STEP_PORT_OFFSET * portIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.control1 = 0x00000000 + portFormula + unitOffset;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.USX_LSPCS.status1  = 0x00000004 + portFormula + unitOffset;

    if(portIndex == 0)
    {
        /* info of PCS that is per representative port */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePortIndex].PCS_common.LPCS_common.gmode = 0x000003e0 + portFormula + unitOffset;/*0x000063e0*/
    }
}

/**
* @internal smemIronmanGopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Phoenix.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemIronmanGopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  globalPort;
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
        devObjPtr->memUnitBaseAddrInfo.anp[numOfAnps++] =  ANP_6_BASE_ADDR; /* ANP for USX 6 */

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            regAddr_anp(devObjPtr, globalPort);
        }
    }

    /* MIF support */
    {
        /* MIF support */
        devObjPtr->memUnitBaseAddrInfo.mif[0] =  MIF_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.mif[1] =  MIF_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.mif[2] =  MIF_2_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.mif[3] =  MIF_3_BASE_ADDR;


        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != smemIronmanPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E,globalPort,&portInfo))
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

    /* NO LMU support */
    devObjPtr->numOfLmus = 0;

    /* TSU/PTP support */
    {
        devObjPtr->memUnitBaseAddrInfo.tsu[0] = TSU_0_BASE_ADDR;

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            if(GT_OK != smemIronmanPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_PCA_TSU_CHANNEL_E,globalPort,&portInfo))
            {
                continue;
            }

            regAddr_tsu(devObjPtr,
                globalPort,/* global port in the device */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);/* the unit index 0..3 */
        }
    }

    /* USX support */
    {
        GT_U32  unitIndex;

        /* support for the USX MAC */
        unitIndex=0;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_0_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_0_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_1_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_1_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_2_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_2_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_3_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_3_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_4_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_4_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_5_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_5_BASE_ADDR;
        unitIndex++;
        devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex]  = MTIP_USX_MAC_6_BASE_ADDR;
        devObjPtr->memUnitBaseAddrInfo.macMib [unitIndex]  = devObjPtr->memUnitBaseAddrInfo.macWrap[unitIndex] + MAC_MIB_OFFSET;
        devObjPtr->memUnitBaseAddrInfo.macPcs [unitIndex]  = MTIP_UNIT_USX_PCS_6_BASE_ADDR;

        for(globalPort = 0 ; globalPort < devObjPtr->portsNumber ; globalPort++)
        {
            GT_U32  is_SMEM_UNIT_TYPE_MTI_MAC_USX_E = (GT_OK == smemIronmanPortInfoGet(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_USX_E,globalPort,&portInfo)) ? 1 : 0;

            if(!is_SMEM_UNIT_TYPE_MTI_MAC_USX_E)
            {
                continue;
            }
            regAddr_MTIPMacWrap(devObjPtr,
                globalPort,/* global port in the device */
                is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
                portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);  /* for usx : the unit index 0..5 , for 100GMAc : index 0 */

            regAddr_MTIPPcs(devObjPtr,
                globalPort,/* global port in the device */
                is_SMEM_UNIT_TYPE_MTI_MAC_USX_E,
                portInfo.sip6_MTI_EXTERNAL_representativePortIndex,/* representative global port in the device (for 'shared' registers) */
                portInfo.simplePortInfo.indexInUnit, /* local  port in the unit */
                portInfo.simplePortInfo.unitIndex);  /* for usx : the unit index 0..5 , for 100GMAc : index 0 */
        }
    }
}

/**
* @internal internal_smemIronmanInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
static void internal_smemIronmanInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isIronman = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP6_30_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        buildDevUnitAddr(devObjPtr);

        isIronman = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        /*devObjPtr->registersDefaultsPtr = &linkListElementsBobcat3_RegistersDefaults;*/
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = ironman_duplicatedUnits;
        devObjPtr->unitsDuplicationsPtr = ironman_duplicatedUnits;
    }

    if (isIronman == GT_TRUE)
    {
        devObjPtr->devMemGopRegDbInitFuncPtr = smemIronmanGopRegDbInit;
        devObjPtr->devIsOwnerMemFunPtr = NULL;
        devObjPtr->devFindMemFunPtr = (void *)smemGenericFindMem;
        devObjPtr->devMemPortInfoGetPtr =  smemIronmanPortInfoGet;

        devObjPtr->devMemGetMgUnitIndexFromAddressPtr = smemIronmanGetMgUnitIndexFromAddress;
        devObjPtr->devMemGopPortByAddrGetPtr = smemIronmanGopPortByAddrGet;
        devObjPtr->devMemMibPortByAddrGetPtr = smemIronmanGopPortByAddrGet;
        devObjPtr->devMemHsrPrpTimerGetFuncPtr = smemSip6_30FdbHsrPrpTimerGet;

        devObjPtr->numOfPipesPerTile = 0;/* no pipes */
        devObjPtr->numOfTiles        = 0;/* no tiles */
        devObjPtr->tileOffset        = 0;
        devObjPtr->mirroredTilesBmp  = 0;

        devObjPtr->numOfMgUnits = 1;/* single MG unit */

        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = IRONMAN_NUM_DP_UNITS;
        /* !!! there is NO TXQ-dq in Ironman (like in Hawk) !!! */
        devObjPtr->multiDataPath.numTxqDq           = 0;
        devObjPtr->multiDataPath.txqDqNumPortsPerDp = 0;

        devObjPtr->multiDataPath.supportRelativePortNum = 1;

        devObjPtr->supportTrafficManager_notAllowed = 1;

        devObjPtr->dmaNumOfCpuPort = CPU_SDMA;

        /* 'global' port in the egress RXDMA/TXDMA units */

        devObjPtr->numOfPipes = 0;
        devObjPtr->numOfPortsPerPipe = SMAIN_NOT_VALID_CNS;/* no 'pipes' */

        devObjPtr->txqNumPorts = 128;/* the limit on the 10 bits in SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM */

        devObjPtr->multiDataPath.maxIa = 1;/* single IA */

        {/* support the multi DP units */
            GT_U32  index = 0;

            devObjPtr->multiDataPath.info[index].dataPathFirstPort  = 0;/* without the 'CPU port' */
            devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = 56;/* without the 'CPU port' */
            devObjPtr->multiDataPath.info[index].cpuPortDmaNum      = CPU_SDMA;/* the 'CPU port' number  in the DP */
        }


        devObjPtr->memUnitBaseAddrInfo.lpm[0] = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_LPM);

        devObjPtr->dma_specialPortMappingArr = ironman_DMA_specialPortMappingArr;
        devObjPtr->gop_specialPortMappingArr    = NULL;
        devObjPtr->ravens_specialPortMappingArr = NULL;
        devObjPtr->cpuPortSdma_specialPortMappingArr = ironman_DMA_specialPortMappingArr;


        /* the device not supports IPCL 0 (supports only IPCL1,2) */
        devObjPtr->isIpcl0NotValid  = 1;/* like Falcon */
        devObjPtr->tcam_numBanksForHitNumGranularity = 2; /* like BC2 */
        devObjPtr->portMacSecondBase = 0;
        devObjPtr->portMacSecondBaseFirstPort = 0;

        devObjPtr->limitedNumOfParrallelLookups = 2;

        devObjPtr->support_remotePhysicalPortsTableMode = 0;/*ignore the mode*/

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,64);/*1024 in Falcon , 512 in BC3, 128 in Aldrin2*/
        SET_IF_ZERO_MAC(devObjPtr->defaultEPortNumEntries,64);/*was 1K in prev devices */
        SET_IF_ZERO_MAC(devObjPtr->TTI_myPhysicalPortAttributes_numEntries,
            devObjPtr->multiDataPath.info[0].dataPathNumOfPorts+1/*57*/);
        SET_IF_ZERO_MAC(devObjPtr->TTI_mac2me_numEntries,16);/*was 128 in all sip5 devices*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort,_1K);/*only 1K eports */

        SET_IF_ZERO_MAC(devObjPtr->lpmRam.perRamNumEntries, 1536);/* 1.5K in Ironman*/

        devObjPtr->cncNumOfUnits = 1;/* must set 1 CNC units otherwise smemLion3Init(...) will set it to 2 */
        devObjPtr->cncBlocksNum  = 8;/* only 8 blocks (previous devices have 16) */
        devObjPtr->cncClientInstances = 3;

        devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emMaxNumEntries  = SMEM_MAC_TABLE_SIZE_32KB;/*256K in Hawk*/
        devObjPtr->emAutoLearnNumEntries = 16 * _1K;           /* 64K in Hawk*/
        devObjPtr->limitedResources.eVid  = 4 * _1K + 512;     /*  8K in Hawk*/
        devObjPtr->limitedResources.stgId = 1 * _1K;           /*  4K in Hawk*/
        devObjPtr->limitedResources.nextHop     =  8*_1K;      /* 16K in Hawk*/

        devObjPtr->policerSupport.iplrTableSize =  2*_1K;      /*  4K in Hawk*/
        devObjPtr->policerSupport.numOfIpfix    =  4*_1K;      /* 64K in Hawk*/
        devObjPtr->tcamNumOfFloors              =  4; /*support 4 floors : each 3K of 10B = total 12K of 10B = 6K@20B */
        devObjPtr->limitedResources.mllPairs    =  4*_1K;        /*  8K in Hawk*/
        devObjPtr->limitedResources.l2LttMll    =  4*_1K;       /* 16K in Hawk*/
        devObjPtr->limitedResources.l3LttMll    =  4*_1K;       /*  4K in Hawk (the same)*/
        devObjPtr->limitedResources.l2Ecmp      =  8*_1K;       /*  8K in Hawk (the same)*/
        devObjPtr->limitedResources.l2LttEcmp   =  8*_1K;       /* like Hawk according to number of eports(8K) */
        devObjPtr->limitedResources.numOfArps   = 16*_1K;       /*256K in Hawk*/
        devObjPtr->limitedResources.ipvxEcmp    = 2*_1K;          /* 6K in Hawk : each line 2 entries */
        devObjPtr->limitedResources.preqSrfNum  = 1*_1K;
        devObjPtr->limitedResources.smuIrfNum   = 2*_1K;

        devObjPtr->numofTcamClients = IRONMAN_TCAM_NUM_OF_GROUPS_CNS;
    }

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemIronmanSpecificDeviceUnitAlloc;
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

    smemHarrierInit(devObjPtr);

    if (isIronman == GT_TRUE)
    {

        /* Hawk - bind special 'find memory' functions */
        smemHawkBindFindMemoryFunc(devObjPtr);

        /* Init RegDb for DP units */
        smemIronmanInitRegDbDpUnits(devObjPtr);

        /* check that no unit exceed the size of it's unit */
        smemGenericUnitSizeCheck(devObjPtr,ironman_units);

        /* NO the FIRMAWARE , as no 'PHA' unit */
    }

}

/**
* @internal smemIronmanInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemIronmanInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    internal_smemIronmanInit(devObjPtr);
}


/**
* @internal smemIronmanInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
void smemIronmanInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* let phoenix init it's logic for traffic */
    smemPhoenixInit_debug_allowTraffic(devObjPtr);

}

/**
* @internal smemIronmanInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemIronmanInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  currAddr;
    static int my_dummy = 0;

    smemHarrierInit2(devObjPtr);

    /* call to 'get'+'set' the register , to trigger the active memory of :
        smemSip6_30ActiveWriteFdbHsrPrpGlobalConfigReg(...)

        so next info will be with 'default' value :
        devObjPtr->hsrPrpTimer[tileId].factor
    */
    currAddr = SMEM_SIP6_30_FDB_UNIT_HSR_PRP_GLOBAL_CONFIG_REG(devObjPtr);
    smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);


    if(my_dummy)
    {
        smemIronmanInit_debug_allowTraffic(devObjPtr);
    }
}


/**

* @internal smemIronmanUnitTxqQfc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
static void smemIronmanUnitTxqQfc
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
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000,0x0000171C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x000000E8)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100,0x0000013C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x000002B0)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400,0x00000504)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000510,0x000005F0)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600,0x00000638)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800,0x00000814)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800,0x0000187C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900,0x00001984)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000,0x0000271C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000,0x000030E0)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300,0x0000331C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400,0x00003400)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500,0x000035E0)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003600,0x0000361C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000,0x000040E4)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004100,0x00004108)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
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
            ,{DUMMY_NAME_PTR_CNS,           0x00004004,         0x0001ffff,      57,    0x4    }/*Port_HR_Counters_Threshold*/
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

* @internal smemIronmanUnitTxqPds function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pds unit
*/
static void smemIronmanUnitTxqPds
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 401024)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00062000, 1824)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00063000, 1824)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064000, 0x00064000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064008, 0x00064008)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064038, 0x0006407C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00064900, 0x0006501C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00066000, 0x00066018)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067000, 0x00067004)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00067010, 0x00067010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000680F8, 0x000680F8)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068104, 0x00068104)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068110, 0x00068114)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068120, 0x0006815C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000681A0, 0x000681DC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068300, 0x0006830C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068400, 0x0006840C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00068480, 0x0006848C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000684C0, 0x000684CC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00069300, 0x0006933C)}

    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    /*simulation defaults*/
    {
      static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,           0x00064008,         0x00006010,      1,    0x4    }/*Max_PDS_size_limit_for_pdx*/
            ,{DUMMY_NAME_PTR_CNS,           0x00064000,         0x0000ffff,      1,    0x4    }/*PDS_Metal_Fix*/
            ,{DUMMY_NAME_PTR_CNS,           0x00064038,         0x00000040,      1,    0x4    }/*L2_CutThrough_Byte_Count_Enqueue*/
            ,{DUMMY_NAME_PTR_CNS,           0x00066004,         0x00000002,      1,    0x4    }/*PDS_Interrupt_Summary_Mask*/
            ,{DUMMY_NAME_PTR_CNS,           0x00067000,         0x00000001,      1,    0x4    }/*Idle_Register*/
            ,{DUMMY_NAME_PTR_CNS,           0x00068114,         0x00007fff,      1,    0x4    }/*Free_PID_FIFO_Min_Peak*/
            ,{DUMMY_NAME_PTR_CNS,           0x00068300,         0x00000005,      1,    0x4    }/*pds_debug_configurations*/
            ,{DUMMY_NAME_PTR_CNS,           0x00068308,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_lsb*/
            ,{DUMMY_NAME_PTR_CNS,           0x0006830c,         0xffffffff,      1,    0x4    }/*pds_debug_cycles_to_count_msb*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
        };
      static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
      unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemIronmanUnitTxqSdq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq sdq unit
*/
static void smemIronmanUnitTxqSdq
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000004)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000214)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x000005E4)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000700, 0x000007E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x0000171C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 3648)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x000040E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004200, 0x000042E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004400, 0x000044E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004600, 0x000046E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004800, 0x000048E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x000050E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x000052E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005400, 0x000054E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005600, 0x000056E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005800, 0x000058E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x0000601C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x000071C4)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007600, 0x000077C4)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00008300, 0x000084C4)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A000, 0x0000A00C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A100, 0x0000A10C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A140, 0x0000A14C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A180, 0x0000A18C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000A1C0, 0x0000A1CC)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B000, 0x0000B024)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B030, 0x0000B038)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B100, 0x0000B10C)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B200, 456)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B400, 228)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000B600, 456)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000B800, 0x0000B8E0)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000C000, 3648)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000E000, 1824)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 59392)}




    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
    smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

    {
        /*PortConfig*/
        {
            GT_U32  pt;
            for(pt = 0; pt <= NUM_PORTS_PER_DP_UNIT; pt++)
            {
                /*0x00004000 + pt*0x4: where pt (0-56) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortConfig[pt] =
                    0x00004000+ pt*0x4;

                /*0x00004200 + pt*0x4: where pt (0-56) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeLow[pt] =
                    0x00004200+ pt*0x4;

                /*0x00004400  + pt*0x4: where pt (0-56) represents PORT*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ[0].PortConfig.PortRangeHigh[pt] =
                    0x00004400 + pt*0x4;
            }
        }
    }

 {
  static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
    {
         {DUMMY_NAME_PTR_CNS,           0x00000004,         0x00000046,      1,    0x4    }/*global_config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x0000ffff,      1,    0x4    }/*SDQ_Metal_Fix*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004400,         0x00000007,      57,    0x4    }/*Port_Range_High*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004600,         0x00003a98,      57,    0x4    }/*Port_Back_Pressure_Low_Threshold*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004800,         0x00003a98,      57,    0x4    }/*Port_Back_Pressure_High_Threshold*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004000,         0x00000007,      57,    0x4    }/*Port_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000500,         0x00000003,      1,    0x4    }/*QCN_Config*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b000,         0x000201ff,      1,    0x4    }/*Sdq_Idle*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007604,         0x00000008,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_1*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007608,         0x00000010,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_2*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000760c,         0x00000018,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_3*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007610,         0x00000020,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_4*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007614,         0x00000028,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_5*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007618,         0x00000030,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_6*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000761c,         0x00000038,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_7*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007620,         0x00000040,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_8*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007624,         0x00000048,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_9*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007628,         0x00000050,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_10*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000762c,         0x00000058,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_11*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007630,         0x00000060,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_12*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007634,         0x00000068,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_13*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007638,         0x00000070,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_14*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000763c,         0x00000078,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_15*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007640,         0x00000080,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_16*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007644,         0x00000088,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_17*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007648,         0x00000090,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_18*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000764c,         0x00000098,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_19*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007650,         0x000000a0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_20*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007654,         0x000000a8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_21*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007658,         0x000000b0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_22*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000765c,         0x000000b8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007660,         0x000000c0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_24*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007664,         0x000000c8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_25*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007668,         0x000000d0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_26*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000766c,         0x000000d8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_27*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007670,         0x000000e0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_28*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007674,         0x000000e8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_29*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007678,         0x000000f0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000767c,         0x000000f8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_31*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007680,         0x00000100,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_32*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007684,         0x00000108,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_33*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007688,         0x00000110,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000768c,         0x00000118,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_35*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007690,         0x00000120,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_36*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007694,         0x00000128,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_37*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007698,         0x00000130,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000769c,         0x00000138,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_39*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076a0,         0x00000140,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_40*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076a4,         0x00000148,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_41*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076a8,         0x00000150,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076ac,         0x00000158,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_43*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076b0,         0x00000160,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_44*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076b4,         0x00000168,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_45*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076b8,         0x00000170,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076bc,         0x00000178,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_47*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076c0,         0x00000180,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_48*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076c4,         0x00000188,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_49*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076c8,         0x00000190,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076cc,         0x00000198,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_51*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076d0,         0x000001a0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_52*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076d4,         0x000001a8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_53*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076d8,         0x000001b0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076dc,         0x000001b8,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_55*/
        ,{DUMMY_NAME_PTR_CNS,           0x000076e0,         0x000001c0,      2,    (0x39)*(0x4)    }/*Sel_List_Range_Low_56*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008300,         0x00000007,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_0*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008304,         0x0000000f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_1*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008308,         0x00000017,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_2*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000830c,         0x0000001f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_3*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008310,         0x00000027,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_4*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008314,         0x0000002f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_5*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008318,         0x00000037,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_6*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000831c,         0x0000003f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_7*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008320,         0x00000047,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_8*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008324,         0x0000004f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_9*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008328,         0x00000057,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_10*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000832c,         0x0000005f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_11*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008330,         0x00000067,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_12*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008334,         0x0000006f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_13*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008338,         0x00000077,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_14*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000833c,         0x0000007f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_15*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008340,         0x00000087,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_16*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008344,         0x0000008f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_17*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008348,         0x00000097,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_18*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000834c,         0x0000009f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_19*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008350,         0x000000a7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_20*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008354,         0x000000af,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_21*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008358,         0x000000b7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_22*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000835c,         0x000000bf,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008360,         0x000000c7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_24*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008364,         0x000000cf,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_25*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008368,         0x000000d7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_26*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000836c,         0x000000df,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_27*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008370,         0x000000e7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_28*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008374,         0x000000ef,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_29*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008378,         0x000000f7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000837c,         0x000000ff,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_31*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008380,         0x00000107,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_32*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008384,         0x0000010f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_33*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008388,         0x00000117,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000838c,         0x0000011f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_35*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008390,         0x00000127,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_36*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008394,         0x0000012f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_37*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008398,         0x00000137,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000839c,         0x0000013f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_39*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083a0,         0x00000147,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_40*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083a4,         0x0000014f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_41*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083a8,         0x00000157,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083ac,         0x0000015f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_43*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083b0,         0x00000167,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_44*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083b4,         0x0000016f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_45*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083b8,         0x00000177,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083bc,         0x0000017f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_47*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083c0,         0x00000187,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_48*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083c4,         0x0000018f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_49*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083c8,         0x00000197,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083cc,         0x0000019f,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_51*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083d0,         0x000001a7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_52*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083d4,         0x000001af,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_53*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083d8,         0x000001b7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083dc,         0x000001bf,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_55*/
        ,{DUMMY_NAME_PTR_CNS,           0x000083e0,         0x000001c7,      2,    (0x39)*(0x4)    }/*Sel_List_Range_High_56*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007000,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_0*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007004,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_1*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007008,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_2*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000700c,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_3*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007018,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_6*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007028,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_10*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007038,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_14*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007048,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_18*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007058,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_22*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007068,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_26*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007078,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_30*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007088,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_34*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007098,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_38*/
        ,{DUMMY_NAME_PTR_CNS,           0x000070a8,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_42*/
        ,{DUMMY_NAME_PTR_CNS,           0x000070b8,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_46*/
        ,{DUMMY_NAME_PTR_CNS,           0x000070c8,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_50*/
        ,{DUMMY_NAME_PTR_CNS,           0x000070d8,         0x00000001,      2,    (0x39)*(0x4)    }/*Sel_List_Enable_54*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a000,         0x00000005,      1,    0x4    }/*debug_config*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a008,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_lsb*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a00c,         0xffffffff,      1,    0x4    }/*debug_cycles_to_count_msb*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006010,         0x00100000,      1,    0x4    }/*Config_Change_Time_Ext_Acc*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002000,         0x60000000,      456,    0x8    }/*queue_cfg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000e000,         0x00000381,      456,    0x4    }/*queue_elig_state*/
        ,{NULL,                         0,                  0x00000000,      0,    0x0  }
    };
  static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
  unitPtr->unitDefaultRegistersPtr = &list;
 }
}

static void smemIronmanUnitTxqPsi
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
       /*PSI_REG*/
       {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x000000E8)}
       ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x00000238)}
       /*PDQ*/


      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000028, IRONMAN_PSI_SCHED_OFFSET+0x0000002C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000038,IRONMAN_PSI_SCHED_OFFSET+ 0x0000003C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000048, IRONMAN_PSI_SCHED_OFFSET+0x0000004C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000050, IRONMAN_PSI_SCHED_OFFSET+0x00000054)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000200, IRONMAN_PSI_SCHED_OFFSET+0x00000204)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000208, IRONMAN_PSI_SCHED_OFFSET+0x0000020C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000210, IRONMAN_PSI_SCHED_OFFSET+0x00000214)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000218, IRONMAN_PSI_SCHED_OFFSET+0x0000021C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000220, IRONMAN_PSI_SCHED_OFFSET+0x00000224)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000800, IRONMAN_PSI_SCHED_OFFSET+0x00000804)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000808, IRONMAN_PSI_SCHED_OFFSET+0x0000080C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000820, IRONMAN_PSI_SCHED_OFFSET+0x00000824)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00000A00, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00001A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00001E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00002200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00002600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00002C00, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00003200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00003600, IRONMAN_PSI_SCHED_OFFSET+0x00003604)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00003608, IRONMAN_PSI_SCHED_OFFSET+0x0000360C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00003800, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00004800, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00004C00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00005000, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00005400, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00005800, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00005C00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00006000, IRONMAN_PSI_SCHED_OFFSET+0x00006004)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00006008, IRONMAN_PSI_SCHED_OFFSET+0x0000600C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00006200, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00007200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00007600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00007A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00007E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00008200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00008600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00008A00, IRONMAN_PSI_SCHED_OFFSET+0x00008A04)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00008A08, IRONMAN_PSI_SCHED_OFFSET+0x00008A0C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00008C00, 1024)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00009C00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000A000, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000A400, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000A800, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000AC00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000B000, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000B400, IRONMAN_PSI_SCHED_OFFSET+0x0000B404)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000B408, IRONMAN_PSI_SCHED_OFFSET+0x0000B40C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000B600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000C000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0000E000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00010000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00012000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00014000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00016000, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00016400, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00018600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00018A00, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00019000, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00019400, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00019A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00019E00, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001B000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001D000, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001F008, IRONMAN_PSI_SCHED_OFFSET+0x0001F00C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001F010, IRONMAN_PSI_SCHED_OFFSET+0x0001F014)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001F200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001F600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001FA00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0001FE00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020A00, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020A10, IRONMAN_PSI_SCHED_OFFSET+0x00020A14)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020C00, 8)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020D10, IRONMAN_PSI_SCHED_OFFSET+0x00020D14)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00020E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00021200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00021600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00021A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00021E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022600, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022640, 64)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x000226C0, 64)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022740, IRONMAN_PSI_SCHED_OFFSET+0x00022744)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022C00, IRONMAN_PSI_SCHED_OFFSET+0x00022C04)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00022E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00023200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00023600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00023A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00023E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024600, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024680, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024780, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024880, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x000248C0, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024900, IRONMAN_PSI_SCHED_OFFSET+0x00024904)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024930, IRONMAN_PSI_SCHED_OFFSET+0x00024934)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00024E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00025200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00025600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00025A00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00025E00, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026600, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026680, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026780, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026880, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x000268C0, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026900, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026920, 16)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026940, IRONMAN_PSI_SCHED_OFFSET+0x00026944)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00026970,IRONMAN_PSI_SCHED_OFFSET+ 0x00026974)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00027000, 4096)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029000, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029200, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029600, 512)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029A00, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029B00, 128)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029C00, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029C40, 32)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (IRONMAN_PSI_SCHED_OFFSET+0x00029C88, IRONMAN_PSI_SCHED_OFFSET+0x00029C8C)}
      ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (IRONMAN_PSI_SCHED_OFFSET+0x0002C000, 16)}
     };

     GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
     smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);

     {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
         {DUMMY_NAME_PTR_CNS,           0x00000800+IRONMAN_PSI_SCHED_OFFSET,         0x00500000,      1,    0x8    }/*PPerCtlConf_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000808+IRONMAN_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*PPerRateShap_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000080c+IRONMAN_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*PPerRateShap_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020a10+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00020a14+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*PPerRateShapInt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000820+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*Port_Pizza_Last_Slice*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b400+IRONMAN_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*QPerCtlConf_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b408+IRONMAN_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*QPerRateShap_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000b40c+IRONMAN_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*QPerRateShap_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029c88+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00029c8c+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*QPerRateShapInt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008a00+IRONMAN_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*APerCtlConf_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008a08+IRONMAN_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*APerRateShap_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00008a0c+IRONMAN_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*APerRateShap_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026940+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00026944+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*APerRateShapInt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006000+IRONMAN_PSI_SCHED_OFFSET,         0x0e390000,      1,    0x8    }/*BPerCtlConf_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006008+IRONMAN_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*BPerRateShap_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000600c+IRONMAN_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*BPerRateShap_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024900+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00024904+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*BPerRateShapInt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00003600+IRONMAN_PSI_SCHED_OFFSET,         0x071c0000,      1,    0x8    }/*CPerCtlConf_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00003608+IRONMAN_PSI_SCHED_OFFSET,         0x066f0002,      1,    0x8    }/*CPerRateShap_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000360c+IRONMAN_PSI_SCHED_OFFSET,         0x0000016e,      1,    0x8    }/*CPerRateShap_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022740+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00022744+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*CPerRateShapInt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000028+IRONMAN_PSI_SCHED_OFFSET,         0x00001033,      1,    0x8    }/*Identity_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000038+IRONMAN_PSI_SCHED_OFFSET,         0x04081020,      1,    0x8    }/*ScrubSlots_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000003c+IRONMAN_PSI_SCHED_OFFSET,         0x00000004,      1,    0x8    }/*ScrubSlots_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000048+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      1,    0x8    }/*TreeDeqEn_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000050+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PDWRREnReg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001f008+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*Global_Cfg*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrorStatus_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000208+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*FirstExcp_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000210+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ErrCnt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000218+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*ExcpCnt_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000220+IRONMAN_PSI_SCHED_OFFSET,         0x00000037,      1,    0x8    }/*ExcpMask_Addr*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00000224+IRONMAN_PSI_SCHED_OFFSET,         0x0000ffff,      1,    0x8    }/*ExcpMask_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001f010+IRONMAN_PSI_SCHED_OFFSET,         0x0000003f,      1,    0x8    }/*Plast_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020d10+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PPerStatus_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022c00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*CPerStatus_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024930+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*BPerStatus_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026970+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*APerStatus_Addr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00000a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*PortEligPrioFunc_Entry*/
        ,{DUMMY_NAME_PTR_CNS,           0x00001a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortEligPrioFuncPtr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00001e00+IRONMAN_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00001e04+IRONMAN_PSI_SCHED_OFFSET,         0x00000001,      64,    0x8    }/*PortTokenBucketTokenEnDiv*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002200+IRONMAN_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00002204+IRONMAN_PSI_SCHED_OFFSET,         0x0001ffff,      64,    0x8    }/*PortTokenBucketBurstSize*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortDWRRPrioEn*/
        ,{DUMMY_NAME_PTR_CNS,           0x00002c00+IRONMAN_PSI_SCHED_OFFSET,         0x00001010,      128,    0x8    }/*PortQuantumsPriosLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00003200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortRangeMap*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*QueueEligPrioFunc*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000c000+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueEligPrioFuncPtr*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000e000+IRONMAN_PSI_SCHED_OFFSET,         0x0fff0fff,      512,    0x8    }/*QueueTokenBucketTokenEnDiv*/
        ,{DUMMY_NAME_PTR_CNS,           0x00010000+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00010004+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      512,    0x8    }/*QueueTokenBucketBurstSize*/
        ,{DUMMY_NAME_PTR_CNS,           0x00012000+IRONMAN_PSI_SCHED_OFFSET,         0x00001040,      512,    0x8    }/*QueueQuantum*/
        ,{DUMMY_NAME_PTR_CNS,           0x00014000+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      512,    0x8    }/*QueueAMap*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008c00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*AlvlEligPrioFunc_Entry*/
        ,{DUMMY_NAME_PTR_CNS,           0x00009c00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlEligPrioFuncPtr*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a000+IRONMAN_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*AlvlTokenBucketTokenEnDiv*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a400+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0000a404+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*AlvlTokenBucketBurstSize*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000a800+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlDWRRPrioEn*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000ac00+IRONMAN_PSI_SCHED_OFFSET,         0x00001040,      64,    0x8    }/*AlvlQuantum*/
        ,{DUMMY_NAME_PTR_CNS,           0x0000b000+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ALvltoBlvlAndQueueRangeMap*/
        ,{DUMMY_NAME_PTR_CNS,           0x00006200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*BlvlEligPrioFunc_Entry*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlEligPrioFuncPtr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007600+IRONMAN_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*BlvlTokenBucketTokenEnDiv*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00007a04+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlTokenBucketBurstSize*/
        ,{DUMMY_NAME_PTR_CNS,           0x00007e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlDWRRPrioEn*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008200+IRONMAN_PSI_SCHED_OFFSET,         0x00001040,      64,    0x8    }/*BlvlQuantum*/
        ,{DUMMY_NAME_PTR_CNS,           0x00008600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BLvltoClvlAndAlvlRangeMap*/
        ,{DUMMY_NAME_PTR_CNS,           0x00003800+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      128,    0x8    }/*ClvlEligPrioFunc_Entry*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004800+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlEligPrioFuncPtr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00004c00+IRONMAN_PSI_SCHED_OFFSET,         0x0fff0fff,      64,    0x8    }/*ClvlTokenBucketTokenEnDiv*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005000+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00005004+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlTokenBucketBurstSize*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005400+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlDWRRPrioEn*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005800+IRONMAN_PSI_SCHED_OFFSET,         0x00001040,      64,    0x8    }/*ClvlQuantum*/
        ,{DUMMY_NAME_PTR_CNS,           0x00005c00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvltoPortAndBlvlRangeMap*/
        ,{DUMMY_NAME_PTR_CNS,           0x00016000+IRONMAN_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00016004+IRONMAN_PSI_SCHED_OFFSET,         0x07ffffff,      64,    0x8    }/*PortShpBucketLvls*/
        ,{DUMMY_NAME_PTR_CNS,           0x00016400+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00016404+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*PortShaperBucketNeg*/
        ,{DUMMY_NAME_PTR_CNS,           0x00018600+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00018604+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*ClvlShpBucketLvls*/
        ,{DUMMY_NAME_PTR_CNS,           0x00018a00+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00018a04+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*CLevelShaperBucketNeg*/
        ,{DUMMY_NAME_PTR_CNS,           0x00019000+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00019004+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*BlvlShpBucketLvls*/
        ,{DUMMY_NAME_PTR_CNS,           0x00019400+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00019404+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*BLevelShaperBucketNeg*/
        ,{DUMMY_NAME_PTR_CNS,           0x00019a00+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00019a04+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      64,    0x8    }/*AlvlShpBucketLvls*/
        ,{DUMMY_NAME_PTR_CNS,           0x00019e00+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x00019e04+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      2,    0x8    }/*ALevelShaperBucketNeg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001b000+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0001b004+IRONMAN_PSI_SCHED_OFFSET,         0x003fffff,      512,    0x8    }/*QueueShpBucketLvls*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001d000+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
        /**/,{DUMMY_NAME_PTR_CNS,           0x0001d004+IRONMAN_PSI_SCHED_OFFSET,         0xffffffff,      16,    0x8    }/*QueueShaperBucketNeg*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001f200+IRONMAN_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*PortNodeState*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001f600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*PortMyQ*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001fa00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus01*/
        ,{DUMMY_NAME_PTR_CNS,           0x0001fe00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020200+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus45*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020600+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*PortRRDWRRStatus67*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*PortWFS*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020c00+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      1,    0x8    }/*PortBPFromQMgr*/
        ,{DUMMY_NAME_PTR_CNS,           0x00027000+IRONMAN_PSI_SCHED_OFFSET,         0x00000002,      512,    0x8    }/*QueueNodeState*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029000+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      16,    0x8    }/*QueueWFS*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029200+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029600+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      64,    0x8    }/*QueueL0ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029a00+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029b00+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*QueueL1ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029c00+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00029c40+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*QueueL2ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x0002c000+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*QueuePerStatus*/
        ,{DUMMY_NAME_PTR_CNS,           0x00020e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*ClvlNodeState*/
        ,{DUMMY_NAME_PTR_CNS,           0x00021200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*ClvlMyQ*/
        ,{DUMMY_NAME_PTR_CNS,           0x00021600+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus01*/
        ,{DUMMY_NAME_PTR_CNS,           0x00021a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00021e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus45*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022200+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*ClvlRRDWRRStatus67*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*ClvlWFS*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022640+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x000226c0+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      8,    0x8    }/*ClvlL0ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00022e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*BlvlNodeState*/
        ,{DUMMY_NAME_PTR_CNS,           0x00023200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*BlvlMyQ*/
        ,{DUMMY_NAME_PTR_CNS,           0x00023600+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus01*/
        ,{DUMMY_NAME_PTR_CNS,           0x00023a00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00023e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus45*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024200+IRONMAN_PSI_SCHED_OFFSET,         0x00000fff,      64,    0x8    }/*BlvlRRDWRRStatus67*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*BlvlWFS*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024680+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024780+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*BlvlL0ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024880+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x000248c0+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*BlvlL1ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00024e00+IRONMAN_PSI_SCHED_OFFSET,         0x00000002,      64,    0x8    }/*AlvlNodeState*/
        ,{DUMMY_NAME_PTR_CNS,           0x00025200+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      64,    0x8    }/*AlvlMyQ*/
        ,{DUMMY_NAME_PTR_CNS,           0x00025600+IRONMAN_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus01*/
        ,{DUMMY_NAME_PTR_CNS,           0x00025a00+IRONMAN_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus23*/
        ,{DUMMY_NAME_PTR_CNS,           0x00025e00+IRONMAN_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus45*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026200+IRONMAN_PSI_SCHED_OFFSET,         0x0003ffff,      64,    0x8    }/*AlvlRRDWRRStatus67*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026600+IRONMAN_PSI_SCHED_OFFSET,         0x00000000,      2,    0x8    }/*AlvlWFS*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026680+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026780+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      16,    0x8    }/*AlvlL0ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026880+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x000268c0+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      4,    0x8    }/*AlvlL1ClusterStateHi*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026900+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateLo*/
        ,{DUMMY_NAME_PTR_CNS,           0x00026920+IRONMAN_PSI_SCHED_OFFSET,         0x00040000,      2,    0x8    }/*AlvlL2ClusterStateHi*/
        ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


static void smemIronmanUnitTxqPdx
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000,512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(20, 4),SMEM_BIND_TABLE_MAC(txqPdxQueueGroupMap)},
      {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 512)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C0C)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030, 0x00001030)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001200)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001210, 0x00001210)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001220, 0x00001220)}
     ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x0000130C)}
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
* @internal smemIronmanUnitTxqPfcc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the txq pfcc unit
*/
static void smemIronmanUnitTxqPfcc
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitPtr
)
{
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000,0x00000010)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100,0x00000108)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200,0x00000214)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300,0x0000031C)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400,0x00000444)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500,0x00000548)},
          {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000,324)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
        {
          static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
          {
             {DUMMY_NAME_PTR_CNS,           0x00000000,         0x000000a0,      1,    0x4    }/*global_pfcc_cfg*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000010,         0x0000ffff,      1,    0x4    }/*PFCC_metal_fix*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000008,         0x00924924,      1,    0x4    }/*TC_to_pool_CFG*/
            ,{DUMMY_NAME_PTR_CNS,           0x0000000c,         0x0001f000,      1,    0x4    }/*Total_virtual_buffer_limit_CFG*/
            ,{DUMMY_NAME_PTR_CNS,           0x00000400,         0x0000003f,      1,    0x4    }/*Source_Port_Requested_For_Read*/
            ,{NULL,                         0,                  0x00000000,      0,    0x0  }
          };
          static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
          unitPtr->unitDefaultRegistersPtr = &list;
        }

}

/**
* @internal smemIronmanSpecificDeviceUnitAlloc_TXQ_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemIronmanSpecificDeviceUnitAlloc_TXQ_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr []=
    {
        {STR(UNIT_TXQ_PSI)       ,smemIronmanUnitTxqPsi}
       ,{STR(UNIT_TXQ_PDS0)       ,smemIronmanUnitTxqPds}
       ,{STR(UNIT_TXQ_SDQ0)       ,smemIronmanUnitTxqSdq}
       ,{STR(UNIT_TXQ_QFC0)       ,smemIronmanUnitTxqQfc}
       ,{STR(UNIT_TXQ_PDX)       ,smemIronmanUnitTxqPdx}
       ,{STR(UNIT_TXQ_PFCC)       ,smemIronmanUnitTxqPfcc}

        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}

/**
* @internal smemSip6_30ActiveWritePntReadyReg function
* @endinternal
*
* @brief   Write the L2i HSR/PRP PNT Ready registers.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemSip6_30ActiveWritePntReadyReg(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemSip6_30ActiveWritePntReadyReg);

    GT_U32  regAddr,pnt_number_of_ready_entries;

    GT_U32  oldReady = (*memPtr)   & 0x1;
    GT_U32  newReady = (*inMemPtr) & 0x1;

    /* data to be written */
    *memPtr = *inMemPtr;

    if(oldReady == newReady)
    {
        /* no change */
        return;
    }

    regAddr = SMEM_SIP6_30_L2I_UNIT_HSR_PRP_PNT_READY_COUNTER_REG(devObjPtr);
    smemRegGet(devObjPtr,regAddr,&pnt_number_of_ready_entries);

    if(oldReady == 0)
    {
        /* the old is 'Not Ready' , so the new is 'Ready' */
        /* --> increment the counter of 'Ready' */
        pnt_number_of_ready_entries++;
        __LOG(("counter 'PNT ready counter' after increment is [%d] \n",pnt_number_of_ready_entries));
    }
    else
    {
        /* the old 'Ready' , so the new is 'Not Ready' */
        /* --> decrement the counter of 'Ready' */
        if(pnt_number_of_ready_entries != 0)
        {
            pnt_number_of_ready_entries--;
            __LOG(("counter 'PNT ready counter' after decrement is [%d] \n",pnt_number_of_ready_entries));
        }
        else
        {
            __LOG(("WARNING : counter 'PNT ready counter' already 0 , so can't decrement \n"));
        }
    }
    smemRegSet(devObjPtr,regAddr,pnt_number_of_ready_entries);
    __LOG_PARAM(pnt_number_of_ready_entries);

    /* the CPU have updated the number of 'ready' ,
       but the interrupt will come only from a packet that is in the unit */
    return;
}

/**
* @internal smemIronmanUnitL2i function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the L2i unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitL2i
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    L2I_ACTIVE_MEM_MAC, /*sip5 active memories */

    /* HSR PRP Proxy Node Table Counter register (Read Only) */
    {0x000030A0, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteToReadOnlyReg, 0},
    /* HSR PRP Proxy Node Table Ready Entry <%i> :
       0x00002A00 + 0x4*i: where i (0-127) represents reg_num */
    {0x00002A00, SMEM_FULL_MASK_CNS-((128*4)-1), NULL, 0, smemSip6_30ActiveWritePntReadyReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x0000225C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000022A0, 0x000022A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000022C0, 0x000022DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002300, 0x0000230C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x000029FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002A00, 0x00002DFC)}

            /*Ingress Bridge physical Port Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00200000 ,256 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortEntry)}
            /*Ingress Bridge physical Port Rate Limit Counters Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00201000 ,256 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(22, 4),SMEM_BIND_TABLE_MAC(bridgePhysicalPortRateLimitCountersEntry)}
            /* Source Trunk Attribute Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00202000 ,256 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressTrunk)}
            /* ingress EPort learn prio Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00210000 ,1024 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(40, 8),SMEM_BIND_TABLE_MAC(bridgeIngressEPortLearnPrio)}
            /* Ingress Span State Group Index Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00218000 , 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12 , 4),SMEM_BIND_TABLE_MAC(ingressSpanStateGroupIndex)}
            /* Ingress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00240000 , 16384 ),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(stp)}
            /* Ingress Port Membership Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00340000 , 36864), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64 , 8),SMEM_BIND_TABLE_MAC(bridgeIngressPortMembership)}
            /* Ingress Vlan Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00440000 , 73728), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(102 , 16),SMEM_BIND_TABLE_MAC(vlan)}
            /*Bridge Ingress ePort Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00500000 ,16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80 ,16),SMEM_BIND_TABLE_MAC(bridgeIngressEPort)}
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


    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x12806004,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00002240,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000ffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x22023924,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x001f803f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000120,         0x52103210,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000130,         0x00001ffe,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x05f205f2,      4,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000160,         0x0001ffff,      2,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001200,         0x04d85f41,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001204,         0x38027027,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001208,         0x02002019,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000120c,         0x00ffffff,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x000012c0,         0xffffffff,      8,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0xffffffff,     32,    0x4     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002000,         0x0000007f,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002010,         0x0000ffff,      1,    0x0     }

            ,{DUMMY_NAME_PTR_CNS,            0x00002204,         0x000403F0,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002208,         0x00002004,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x0000220C,         0x3FF00000,      1,    0x0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002300,         0x0000000A,      4,    0x4     }


            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemIronmanUnitEgfSht function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-SHT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitEgfSht
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* Egress vlan table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,36864 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egressVlan)}
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            /* NOTE: according to Design team the RAM was removed , although CIDER keep showing this table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 131072), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egfShtEportEVlanFilter)}
#endif /*0*/
            /* L2 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00040000 ,32768+8*64  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(l2PortIsolation)}
            /* L3 Port Isolation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00060000 ,32768+8*64  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(l3PortIsolation)}
            /* Egress EPort table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00078000 , 4096  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(29, 4),SMEM_BIND_TABLE_MAC(egfShtEgressEPort)}
            /* EVlan Attribute table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00080000 , 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanAttribute)}
            /* EVlan Spanning table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00088000 , 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(egfShtEVlanSpanning)}
            /* Non Trunk Members 2 Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00090000 , 2048  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(nonTrunkMembers2)}
             /* Device Map Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A8000 ,  16384 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(deviceMapTable)}
             /* Vid Mapper Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000A0000 ,  32768 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfShtVidMapper)}
            /* Non Trunk Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000AC000 ,  2048  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(nonTrunkMembers)}
             /* Designated Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000AD000 ,   1024 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(designatedPorts)}
            /* Egress Spanning Tree State Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000B0000 , 8192   ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(egressStp)}
            /* Multicast Groups Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000C0000 , 32768  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(mcast)}
            /* Source ID Members Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x000F0000 , 32768  ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8),SMEM_BIND_TABLE_MAC(sst)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100000, 0x00100010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100020, 0x00100020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100030, 0x00100030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100040, 0x00100044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001000C0, 0x001000C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100140, 0x00100144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001001C0, 0x001001C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100240, 0x00100244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001002C0, 0x001002C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100340, 0x00100344)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001003C0, 0x001003CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001004C0, 0x001004C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100540, 0x00100544)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001005C0, 0x001005DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001007C0, 0x0010083C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00100FC0, 0x00100FC4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00101040, 0x00101044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x001010C0, 0x001010C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00101140, 0x00101140)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,            0x00100000,         0x01fff007,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100004,         0x00000001,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100008,         0x0000000f,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x0010000c,         0x00000078,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100030,         0xffff0000,      1,    0x0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100040,         0xffffffff,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100240,         0xffffffff,      2,    0x4    }
            ,{DUMMY_NAME_PTR_CNS,            0x00100340,         0xffffffff,      2,    0x4    }

             /* Device Map Table */
            /* Default value of 'UplinkIsTrunk - 0x1 = Trunk;' like in legacy devices */
            ,{DUMMY_NAME_PTR_CNS,           0x000A8000,         0x00000002,   16384/4,    0x4   }
             /* Designated Port Table */
            ,{DUMMY_NAME_PTR_CNS,           0x000AD000,         0xffffffff,    1024/4,    0x4   }
#if 0 /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
            /* Eport EVlan Filter*/
            ,{DUMMY_NAME_PTR_CNS,           0x00020000,         0xffffffff,  131072/4,    0x4   }
#endif
            /* L2 port isolation Table */
            ,{DUMMY_NAME_PTR_CNS,           0x00040000,         0xffffffff,   32768/4,    0x4   }
            /* L3 port isolation Table */
            ,{DUMMY_NAME_PTR_CNS,           0x00060000,         0xffffffff,   32768/4,    0x4   }
            /* Source ID Members Table */
            ,{DUMMY_NAME_PTR_CNS,           0x000F0000,         0xffffffff,   32768/4,    0x4   }

#if 0   /* the Design not initialize the vidx 0xFFF entry any more ! */
            /* Multicast Groups Table */
            /* vidx 0xfff - 128 members */
            ,{DUMMY_NAME_PTR_CNS, 0x000C0000 + (0xfff * 0x10),  0xffffffff,         4,    0x4   }

        /* the Design not initialize the vlan 1 entry any more ! */
            /* Egress vlan table */
            /* vlan 1 members (64 port )*/
            ,{DUMMY_NAME_PTR_CNS, 0x00000000 + (1 * 0x8),      0xffffffff,         4,    0x4   }

#endif
            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {/*start of unit EGF_sht */
        GT_U32  numOfPorts = 64;

        {/*start of unit global */
            {/*start of unit miscellaneous */
/*              SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.miscellaneous.badAddrLatchReg = 0x00101140;*/

            }/*end of unit miscellaneous */


            {/*01007c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/2); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportAssociatedVid1[n] =
                        0x01007c0+n*0x4;
                }/* end of loop n */
            }/*01007c0+n*0x4*/
            {/*0100040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.UCSrcIDFilterEn[n] =
                        0x0100040+0x4*n;
                }/* end of loop n */
            }/*0100040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MESHIDConfigs = 0x00100004;
            {/*01010c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MCLocalEn[n] =
                        0x01010c0+0x4*n;
                }/* end of loop n */
            }/*01010c0+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsMask = 0x00100020;
            {/*0101040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        0x0101040+0x4*n;
                }/* end of loop n */
            }/*0101040+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTGlobalConfigs = 0x00100000;
            {/*01002c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpStateMode[n] =
                        0x01002c0+0x4*n;
                }/* end of loop n */
            }/*01002c0+0x4*n*/
            {/*0100340+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpState[n] =
                        0x0100340+0x4*n;
                }/* end of loop n */
            }/*0100340+0x4*n*/
            {/*01001c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        0x01001c0+0x4*n;
                }/* end of loop n */
            }/*01001c0+0x4*n*/
            {/*01003c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/16); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportPortIsolationMode[n] =
                        0x01003c0+0x4*n;
                }/* end of loop n */
            }/*01003c0+0x4*n*/
            {/*0100540+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        0x0100540+0x4*n;
                }/* end of loop n */
            }/*0100540+0x4*n*/
            {/*01005c0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/8); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ePortMeshID[n] =
                        0x01005c0+n * 0x4;
                }/* end of loop n */
            }/*01005c0+n * 0x4*/
            {/*01004c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        0x01004c0+0x4*n;
                }/* end of loop n */
            }/*01004c0+0x4*n*/
            {/*01000c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        0x01000c0+0x4*n;
                }/* end of loop n */
            }/*01000c0+0x4*n*/
            {/*0100240+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        0x0100240+0x4*n;
                }/* end of loop n */
            }/*0100240+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportVlanEgrFiltering = 0x0010000c;
            {/*0100fc0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportDropOnEportVid1Mismatch[n] =
                        0x0100fc0+n * 0x4;
                }/* end of loop n */
            }/*0100fc0+n * 0x4*/
            {/*0100140+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < (numOfPorts/32); n++) {
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
* @internal smemIronmanUnitEgfQag function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-QAG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
void smemIronmanUnitEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /*eVLAN Tag Command Table*/
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 147456),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8),SMEM_BIND_TABLE_MAC(egfQagEVlanDescriptorAssignmentAttributes)}
             /*ePort Descriptor Assignment Attributes Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 8192 ), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(46, 8),SMEM_BIND_TABLE_MAC(egfQagEgressEPort)}
            /* TC_DP_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(egfQagTcDpMapper)}
            /* VOQ_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00094000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(7, 4)}
            /* Port_Target_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00096000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(egfQagPortTargetAttribute)}
            /* Port_Enq_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00097000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(egfQagTargetPortMapper)}
            /* Port_Source_Attributes_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00098000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(9, 4) ,SMEM_BIND_TABLE_MAC(egfQagPortSourceAttribute)}
            /* EVIDX_Activity_Status_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00099000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}
            /* Cpu_Code_To_Loopback_Mapper_Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0009A000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4), SMEM_BIND_TABLE_MAC(egfQagCpuCodeToLbMapper)}

            /*VLAN Q Offset Mapping Table  Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A8000,18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8, 4), SMEM_BIND_TABLE_MAC(egfQagVlanQOffsetMappingTable)}

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
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /* Cut Through Configuration */
            {DUMMY_NAME_PTR_CNS,    0x000A0A04,         0x0000fFFF,      1,    0x0}
           ,{DUMMY_NAME_PTR_CNS,    0x00097000,         0x00010000,      64,  0x4}
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
* @internal smemIronmanUnitEgfEft function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EGF-EFT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitEgfEft
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
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(6, 4) , SMEM_BIND_TABLE_MAC(secondTargetPort)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001030, 0x00001034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010A0, 0x000010A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000010B0, 0x000010B0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001110, 0x00001110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001130, 0x00001130)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001140, 0x00001140)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x00001354)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002108, 0x00002108)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021D0, 0x000021D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002200, 0x00002204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002280, 0x00002280)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000301C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003080, 0x00003084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003180, 0x0000319C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006000, 0x00006004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006080, 0x00006084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006100, 0x00006104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006180, 0x00006184)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006200, 0x00006204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006280, 0x00006284)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006380, 0x00006384)}
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
            ,{DUMMY_NAME_PTR_CNS,            0x00001020,         0x08e01000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001030,         0x0000009c,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001110,         0xffff0000,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00001130,         0x00000001,      2,    0xfd0    }
            ,{DUMMY_NAME_PTR_CNS,            0x00001150,         0x00000fff,    128,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00002108,         0x00000808,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x000021d0,         0x00002040,      2,    0xb0     }
            ,{DUMMY_NAME_PTR_CNS,            0x00002204,         0xffffffff,      16,   0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x00000010,      8,    0x4      }
            ,{DUMMY_NAME_PTR_CNS,            0x00006000,         0x00018801,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00006004,         0x000FFF39,      1,    0x0      }
            ,{DUMMY_NAME_PTR_CNS,            0x00006440,         0x000FFF39,      1,    0x0      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }


    {/*start of unit EGF_eft */
        GT_U32  eftNumPorts = 64;
        {/*start of unit mcFIFO */
            {/*start of unit mcFIFOConfigs */
                {/*0002200+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n < eftNumPorts/32 ; n++) {
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
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
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
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedMCFilterEn[n] =
                        0x0006100+0x4*n;
                }/* end of loop n */
            }/*0006100+0x4*n*/
            {/*0006180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedBCFilterEn[n] =
                        0x0006180+0x4*n;
                }/* end of loop n */
            }/*0006180+0x4*n*/
            {/*0006080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unknownUCFilterEn[n] =
                        0x0006080+0x4*n;
                }/* end of loop n */
            }/*0006080+0x4*n*/
            {/*0006380+4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        0x0006380+4*n;
                }/* end of loop n */
            }/*0006380+4*n*/
            {/*0006200+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[n] =
                        0x0006200+0x4*n;
                }/* end of loop n */
            }/*0006200+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersGlobal = 0x00006000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersEnable = 0x00006004;

        }/*end of unit egrFilterConfigs */


        {/*start of unit deviceMapConfigs */
            {/*0003080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localSrcPortMapOwnDevEn[n] =
                        0x0003080+0x4*n;
                }/* end of loop n */
            }/*0003080+0x4*n*/
            {/*0003100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localTrgPortMapOwnDevEn[n] =
                        0x0003100+0x4*n;
                }/* end of loop n */
            }/*0003100+0x4*n*/
            {/*0003180+4*n*/
                GT_U32    n;
                for(n = 0 ; n < eftNumPorts/8 ; n++) {
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
* @internal smemIronmanUnitTti function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the TTI unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemIronmanUnitTti
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001EC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000218)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000220)}
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001430, 0x000014E0)}
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
            /*IP2ME*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x000040BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x0000703C)}

            /* Default Port Protocol eVID and QoS Configuration Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(portProtocolVidQoSConf)}
            /* PCL User Defined Bytes Configuration Memory -- 70 udb's in each entry */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(840, 128),SMEM_BIND_TABLE_MAC(ipclUserDefinedBytesConf)}
            /* TTI User Defined Bytes Configuration Memory -- TTI keys based on UDB's : 8 entries support 8 keys*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 1280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384, 64),SMEM_BIND_TABLE_MAC(ttiUserDefinedBytesConf)}
            /* VLAN Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 16384), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(13, 4),SMEM_BIND_TABLE_MAC(ingressVlanTranslation)}
            /* Port to Queue Translation Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(10, 4),SMEM_BIND_TABLE_MAC(ttiPort2QueueTranslation)}
            /*Physical Port Attributes*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(92, 16),SMEM_BIND_TABLE_MAC(ttiPhysicalPortAttribute)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110000, 0x00110100)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00110500, 0x00110500)}

            /*Physical Port Attributes 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00160000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(251, 32),SMEM_BIND_TABLE_MAC(ttiPhysicalPort2Attribute)}

            /*Default ePort Attributes (pre-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00210000, 2048), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(155, 32),SMEM_BIND_TABLE_MAC(ttiPreTtiLookupIngressEPort)}
            /*ePort Attributes (post-tti lookup eport table)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00240000, 4096), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(ttiPostTtiLookupIngressEPort)}
            /* QCN to Pause Timer Map*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 256), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(ttiQcnToPauseTimerMap)}

         };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            /* (new table in sip6) vrf_id eVlan Mapping Table */
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000, 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4),SMEM_BIND_TABLE_MAC(ttiVrfidEvlanMapping)}
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
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x30002503,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x00000020,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x65586558,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x0000000e,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000024,         0x1001ffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000028,         0x000088e7,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x88488847,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000034,         0x65586558,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000038,         0x00003232,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000003c,         0x0000000d,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000040,         0xff000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x00000001,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000004c,         0xff020000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000050,         0xff000000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000054,         0xffffffff,      3,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000060,         0x00001800,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000068,         0x186db81b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000006c,         0x00000007,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000070,         0x00008906,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x000fff00,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000088,         0x3fffffff,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000000f0,         0xffffffff,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000100,         0xffffffff,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000110,         0x0000004b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000118,         0x00001320,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000011c,         0x0000001b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000140,         0x20a6c01b,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000144,         0x24924924,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000014c,         0x00ffffff,      2,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000154,         0x0000311f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0fffffff,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000188,         0x1a00003c,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000001e4,         0x00000002,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000001e8,         0xa6000104,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000001ec,         0x000002a8,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000200,         0x030022f3,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000204,         0x00400000,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000208,         0x12492492,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000020c,         0x00000092,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000210,         0x0180c200,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000220,         0x000388fb,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000300,         0x81008100,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000310,         0xffffffff,      2,    0x8 }
            ,{DUMMY_NAME_PTR_CNS,            0x00000340,         0x0000000f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001050,         0x000001fc,      8,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000013bc,         0x0000003f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000013f8,         0x0000ffff,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x000013fc,         0x000001da,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001500,         0x00602492,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00001600,         0x00000fff,     16,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00001608,         0xffff1fff,     16,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x0000160c,         0xffffffff,     16,    0x20}
            ,{DUMMY_NAME_PTR_CNS,            0x00003000,         0x88f788f7,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x013f013f,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x0000300c,         0x00000570,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00003094,         0x88b588b5,      1,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00007000,         0x00224fd0,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00007020,         0x08002001,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00007030,         0x00050000,      4,    0x4 }
            ,{DUMMY_NAME_PTR_CNS,            0x00110500,         0x88a8893f,      1,    0x4 }

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
* @internal smemIronmanUnitIpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the IPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitIpcl
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
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000005C, 0x00000060)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000078)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x00000100)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000110, 0x0000011C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000130, 0x00000130)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000148)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000158)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000005C0, 0x000005FC),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(pearsonHash)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x0000073C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000744, 0x0000077C)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000850)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000860, 0x000008B0)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000CFC)}

           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000,  512),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, 32),SMEM_BIND_TABLE_MAC(crcHashMode)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(148, 32),SMEM_BIND_TABLE_MAC(crcHashMask)}
/*         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000,   0)}*//*IPCL0 Source Port Configuration*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 256)}/*IPCL1 Source Port Configuration*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 256)}/*IPCL2 Source Port Configuration*/
            /* next are set below as formula of 3 tables
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 33280)}
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 33280)}
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 33280)}
            */
/*         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000,    0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(490,64),SMEM_BIND_TABLE_MAC(ipcl0UdbSelect)}*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000, 7168),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(490,64),SMEM_BIND_TABLE_MAC(ipcl1UdbSelect)}
/*         ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00048000,    0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(490,64),SMEM_BIND_TABLE_MAC(ipcl2UdbSelect)}*/

/*           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 0),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25,4),SMEM_BIND_TABLE_MAC(ipcl0UdbReplacement)}*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00052000, 448),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25,4),SMEM_BIND_TABLE_MAC(ipcl1UdbReplacement)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00054000, 448),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25,4),SMEM_BIND_TABLE_MAC(ipcl2UdbReplacement)}

/*           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00003000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl0SourcePortConfig)}*/
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl1SourcePortConfig)}
           ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00005000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(24,4),SMEM_BIND_TABLE_MAC(ipcl2SourcePortConfig)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

    }

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* IPCL0,IPCL1,IPCL2 Configuration Table */
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00020000, 33280), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(33, 8),SMEM_BIND_TABLE_MAC(pclConfig)}, FORMULA_SINGLE_PARAMETER(2 , 0x00010000)}
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
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x05e10001,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x02801000,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x0000ffff,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000028,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000020,         0x0000004a,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000038,         0x00000fff,      2,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000044,         0x3fffffff,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x0000ffff,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000730,         0x00ff0080,      1,    0x4  }
            ,{DUMMY_NAME_PTR_CNS,            0x00000738,         0x00080008,      1,    0x4  }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };



        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

}

/**
* @internal smemIronmanUnitEpcl function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the EPCL unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitEpcl
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
            {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000074, 0x00000074)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000000C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000110)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x0000015C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000164, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000210, 0x00000214)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000254)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000260, 0x0000026C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x0000041C)}
            /* this is registers but treated as memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000514),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(31, 4),SMEM_BIND_TABLE_MAC(epclUdbReplacement)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 16640),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(31, 4),SMEM_BIND_TABLE_MAC(epclConfigTable)}
/*          ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 6144),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(364, 64),SMEM_BIND_TABLE_MAC(epclUdbSelect)}*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18, 4),SMEM_BIND_TABLE_MAC(epclTargetPhysicalPortMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00013000, 256),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(17, 4),SMEM_BIND_TABLE_MAC(epclSourcePhysicalPortMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00014000, 8192),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4),SMEM_BIND_TABLE_MAC(epclPortLatencyMonitoring)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00016000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(14, 4),SMEM_BIND_TABLE_MAC(epclExactMatchProfileIdMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000, 228) ,SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(9, 4), SMEM_BIND_TABLE_MAC(queueGroupLatencyProfileConfigTable)}
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
            ,{DUMMY_NAME_PTR_CNS,            0x00000170,         0x0000FFFF,      4,    0x4      }
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
* @internal smemIronmanUnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr              - pointer to the unit chunk
*/
static void smemIronmanUnitHa
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
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x0000006C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000080, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000140, 0x00000144)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000016C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C0, 0x000003C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003D0, 0x000003D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000410)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000420, 0x00000428)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000430, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x0000053C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000550, 0x00000560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000570, 0x00000570)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000590, 0x000005AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000720)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x000009FC)}

            /*HA Physical Port Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(81, 16), SMEM_BIND_TABLE_MAC(haEgressPhyPort1)}
             /*EPCL User Defined Bytes Configuration Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(720,128), SMEM_BIND_TABLE_MAC(haEpclUserDefinedBytesConfig)}
             /*HA Physical Port Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(101, 16), SMEM_BIND_TABLE_MAC(haEgressPhyPort2)}
            /*HA QoS Profile to EXP Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00008000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(3, 4), SMEM_BIND_TABLE_MAC(haQosProfileToExp)}
             /*HA Global MAC SA Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00009000, 2048),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(48, 8), SMEM_BIND_TABLE_MAC(haGlobalMacSa)}
            /*PTP Domain table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 2560) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(35, 8), SMEM_BIND_TABLE_MAC(haPtpDomain)}
            /*Generic TS Profile table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(537,128), SMEM_BIND_TABLE_MAC(tunnelStartGenericIpProfile) }
            /*HA Egress ePort Attribute Table 2*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00060000, 4096),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(25, 4), SMEM_BIND_TABLE_MAC(haEgressEPortAttr2)}
            /*HA Egress ePort Attribute Table 1*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(120,16), SMEM_BIND_TABLE_MAC(haEgressEPortAttr1)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             /*EVLAN Table (was 'vlan translation' in legacy device)*/
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 36864),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36, 8), SMEM_BIND_TABLE_MAC(egressVlanTranslation)}
            /*VLAN MAC SA Table (was 'VLAN/Port MAC SA Table' in legacy device)*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 18432), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(12, 4), SMEM_BIND_TABLE_MAC(vlanPortMacSa)}
            /*Router ARP DA and Tunnel Start Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 32768),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(384,64), SMEM_BIND_TABLE_MAC(arp)/*tunnelStart*/}
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
             {DUMMY_NAME_PTR_CNS,            0x00000000,         0x00101010,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x03fdd003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000000c,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000080,         0xff000000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000084,         0x00000001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x0000008c,         0xff020000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0xffffffff,      4,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000000a4,         0x00000003,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x000003d0,         0x00000008,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000408,         0x81000000,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000410,         0x00010000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000424,         0x00110000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000500,         0x00008100,      8,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000550,         0x88488847,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000554,         0x00008100,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000558,         0x000022f3,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x0000055c,         0x00006558,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000570,         0x000c0000,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000710,         0x0000ffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000714,         0x000037bf,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000718,         0x80747874,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x0000071c,         0x00000803,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,            0x00000720,         0x00000080,      1,    0x4}

            ,{NULL,                          0x00000000,         0x00000000,      0,    0x0}
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list =
                {myUnit_registersDefaultValueArr,NULL};

        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemSip6_30ActiveWriteSmuSngIrfEntry function
* @endinternal
*
* @brief   Write the Smu Sng Irf entry.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemSip6_30ActiveWriteSmuSngIrfEntry(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  tempEntryArr[2];
    GT_U32  origEntryArr[2];
    GT_U32  streamNumber = (address & 0x3FFF)>>2;/* entry index for the LOG*/
    GT_U32  tmpValue,ii;
    GT_U32  *internalDataPtr = smemTableInternalDataPtrGet(devObjPtr,address - 4/*start of entry*/);
    GT_U32  roFields[] = {
         SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E
        ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E
        /* must be last */
        ,SMAIN_NOT_VALID_CNS
    };

    /*the entry as the CPU wanted to set the entry */
    tempEntryArr[0] = internalDataPtr[0]; /* word in internalDataOffset */
    tempEntryArr[1] =          *inMemPtr;

    /*the entry as the exists before we modify it */
    origEntryArr[0] = memPtr[-1]; /* word in the orig entry */
    origEntryArr[1] = memPtr[ 0]; /* word in the orig entry */

    /* we need to make sure that 'RO' (read only) are not modified in the process ! */
    for(ii = 0 ; roFields[ii] != SMAIN_NOT_VALID_CNS ; ii++)
    {
        tmpValue =
        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,origEntryArr,
            streamNumber,
            roFields[ii]);

        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,tempEntryArr,
            streamNumber,
            roFields[ii],
            tmpValue);
    }

    if(SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
        tempEntryArr,
        streamNumber,
        SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E))
    {
        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            tempEntryArr,
            streamNumber,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E,
            0x0);
        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            tempEntryArr,
            streamNumber,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E,
            0x0);
    }

    /* copy to the actual entry place */
    memPtr[-1] = tempEntryArr[0];
    memPtr[ 0] = tempEntryArr[1];

    return;
}

/**
* @internal smemIronmanUnitSmu function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the SMU unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitSmu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

        /* SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(smuSngIrf) : support 'RO' fields */
        /* 0x00004000 .. 0x00007ffc */
        {0x00004004, 0xFFFFC004, NULL, 0 ,smemSip6_30ActiveWriteSmuSngIrfEntry, 0},
        /*SMEM_BIND_TABLE_MAC(smuIrfCounters) 0x00020000..0x0003fffc
          all fields are ROC */
        {0x00020000, 0xFFFE0000, smemChtActiveReadCntrs, 0, smemChtActiveWriteToReadOnlyReg,0},

        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001008, 0x00001008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x0000111C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00004000, 16384),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36, 8),SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(smuSngIrf)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 65536),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(146, 32),SMEM_BIND_TABLE_MAC(smuIrfCounters)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040000, 0x00040010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040018, 0x00040018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040020, 0x00040020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040028, 0x00040028)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040030, 0x00040030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040038, 0x00040038)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040050, 0x00040050)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040060, 0x00040060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040070, 0x00040070)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00040150, 0x0004017C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00041000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00042000, 928) ,SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(72, 16)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00044000, 1856),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64, 8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00050000, 59392),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 237568),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(120, 16)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             {DUMMY_NAME_PTR_CNS,     0x00000000,         0x0000ffff,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,     0x00000200,         0x00000248,      2,    0x4}
            ,{DUMMY_NAME_PTR_CNS,     0x00001000,         0x00000006,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,     0x00001008,         0x00000aaa,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,     0x00040004,         0x00720001,      1,    0x4}
            ,{DUMMY_NAME_PTR_CNS,     0x00040150,         0x3fffffff,     12,    0x4}

            /* SNG_IRF table (smuSngIrf) */
            ,{DUMMY_NAME_PTR_CNS,     0x00004000,         0x0001FFFE,  2*_1K,    0x8}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }

    {
        GT_U32 n;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.smuMiscellaneous.smuBadAddrLatchReg = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.smuMiscellaneous.smuInterruptCause  = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.smuMiscellaneous.smuInterruptMask   = 0x0000000C;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.sipIngressArbiterConfig.smuIngressArbiterWeights    = 0x00000200;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.sipIngressArbiterConfig.smuIngressArbiterPriorities = 0x00000204;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.irfSng.irfSngGlobalConfig          = 0x00001000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.irfSng.irfSngCountersFieldsConfig  = 0x00001008;
        for(n = 0 ; n <= 63 ; n++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.irfSng.irfAgeBit[n]          = 0x00001020 + n*0x4;
        }

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcGlobalConfig              = 0x00040000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcGlobalScanConfig          = 0x00040004;
        for(n = 0 ; n <= 1 ; n++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcTodOnly[n] = 0x00040008 + 0x04*n;
        }
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigTodMsbHigh        = 0x00040010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigTodMsbLow         = 0x00040018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigTodLsb            = 0x00040020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigTableSetConfig    = 0x00040028;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigValidPendingStatus= 0x00040030;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcReconfigTodOffset         = 0x00040038;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcConfigChangeTimeLsbBits   = 0x00040050;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcConfigChangeTimeMidBits   = 0x00040060;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcConfigChangeTimeMsmBits   = 0x00040070;

        for(n = 0 ; n <= 11 ; n++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SMU.streamGateControl.sgcGateId2TableSetConfig[n] = 0x00040150 + 0x4*n;
        }
    }
}


/**
* @internal smemIronmanUnitCnc function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the CNC unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitCnc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    ACTIVE_MEM_CNC_COMMON_MAC,
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000024, 0x00000024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000030, 0x00000030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000040, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x00000104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000180, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000190, 0x00000190)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001A4, 0x000001A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001B8, 0x000001B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001CC, 0x000001CC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001E0, 0x000001E8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000310, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001098, 0x00001098)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001198, 0x000011A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000011B8, 0x000011C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000011D8, 0x000011E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000011F8, 0x00001200)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001218, 0x00001220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001238, 0x00001240)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001258, 0x00001260)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001278, 0x00001280)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001398, 0x00001398)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001498, 0x00001514)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001598, 0x000015A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000015B8, 0x000015C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000015D8, 0x000015E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000015F8, 0x00001600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001618, 0x00001620)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001638, 0x00001640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001658, 0x00001660)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001678, 0x00001680)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001798, 0x000017A0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017B8, 0x000017C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017D8, 0x000017E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000017F8, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001818, 0x00001820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001838, 0x00001840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001858, 0x00001860)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001878, 0x00001880)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001998, 0x000019B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A18, 0x00001A34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001A98, 0x00001A9C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001C98, 0x00001CAC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CC8, 0x00001CDC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001CF8, 0x00001D0C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D28, 0x00001D3C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D58, 0x00001D6C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001D88, 0x00001D9C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001DB8, 0x00001DCC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001DE8, 0x00001DFC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 8192 * 8), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(64,8),SMEM_BIND_TABLE_MAC(cncMemory)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            /*CNC Global Configuration Register*/
             {DUMMY_NAME_PTR_CNS,     0x00000000,         0x03E647CB,      1,    0x4}
            /*CNC Metal Fix Register*/
            ,{DUMMY_NAME_PTR_CNS,     0x00000180,         0xFFFF0000,      1,    0x4}
            /*CNC Blocks Rate Limit Fifo Thr*/
            ,{DUMMY_NAME_PTR_CNS,     0x00001098,         0xFFFFFFFF,      1,    0x4}

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };

        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/*******************************************************************************
*  smemSip6_30ActiveWritePreqSrfConfigMemory
*
* DESCRIPTION:
*      active Write to memory : support 'RO' fields , when write to preqSrfConfig
*
* INPUTS:
*       devObjPtr   - device object PTR.
*       address     - Address for ASIC memory.
*       memSize     - size of the requested memory
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static void smemSip6_30ActiveWritePreqSrfConfigMemory(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,  /*target*/
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr/*source*/
)
{
    GT_U32  tempEntryArr[3];
    GT_U32  origEntryArr[3];
    GT_U32  srfNumber = (address & 0xFFF0)>>4;/* entry index for the LOG*/
    GT_U32  tmpValue,ii;
    GT_U32  *internalDataPtr = smemTableInternalDataPtrGet(devObjPtr,address - 8/*start of entry*/);
    GT_U32  roFields[] = {
         SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E
        ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E
        ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E
        /* must be last */
        ,SMAIN_NOT_VALID_CNS
    };

    /*the entry as the CPU wanted to set the entry */
    tempEntryArr[0] = internalDataPtr[0]; /* word in internalDataOffset */
    tempEntryArr[1] = internalDataPtr[1]; /* word in internalDataOffset */
    tempEntryArr[2] =          *inMemPtr;

    /*the entry as the exists before we modify it */
    origEntryArr[0] = memPtr[-2]; /* word in the orig entry */
    origEntryArr[1] = memPtr[-1]; /* word in the orig entry */
    origEntryArr[2] = memPtr[ 0]; /* word in the orig entry */

    /* we need to make sure that 'RO' (read only) are not modified in the process ! */
    for(ii = 0 ; roFields[ii] != SMAIN_NOT_VALID_CNS ; ii++)
    {
        tmpValue =
        SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,origEntryArr,
            srfNumber,
            roFields[ii]);

        SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,tempEntryArr,
            srfNumber,
            roFields[ii],
            tmpValue);
    }

    /* copy to the actual entry place */
    memPtr[-2] = tempEntryArr[0];
    memPtr[-1] = tempEntryArr[1];
    memPtr[ 0] = tempEntryArr[2];
}


/*******************************************************************************
*  smemSip6_30ActiveWritePreqSrfDaemonsMemory
*
* DESCRIPTION:
*      active Write to memory : support 'RO' fields , when write to preqDaemons
*
* INPUTS:
*       devObjPtr   - device object PTR.
*       address     - Address for ASIC memory.
*       memSize     - size of the requested memory
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static void smemSip6_30ActiveWritePreqSrfDaemonsMemory(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,  /*target*/
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr/*source*/
)
{
    GT_U32  tempEntryArr[3];
    GT_U32  origEntryArr[3];
    GT_U32  srfNumber = (address & 0xFFF0)>>4;/* entry index for the LOG*/
    GT_U32  tmpValue,ii;
    GT_U32  *internalDataPtr = smemTableInternalDataPtrGet(devObjPtr,address - 8/*start of entry*/);
    GT_U32  roFields[] = {
         SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E
        ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E
        ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E
        /* must be last */
        ,SMAIN_NOT_VALID_CNS
    };

    /*the entry as the CPU wanted to set the entry */
    tempEntryArr[0] = internalDataPtr[0]; /* word in internalDataOffset */
    tempEntryArr[1] = internalDataPtr[1]; /* word in internalDataOffset */
    tempEntryArr[2] =          *inMemPtr;

    /*the entry as the exists before we modify it */
    origEntryArr[0] = memPtr[-2]; /* word in the orig entry */
    origEntryArr[1] = memPtr[-1]; /* word in the orig entry */
    origEntryArr[2] = memPtr[ 0]; /* word in the orig entry */

    /* we need to make sure that 'RO' (read only) are not modified in the process ! */
    for(ii = 0 ; roFields[ii] != SMAIN_NOT_VALID_CNS ; ii++)
    {
        tmpValue =
        SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,origEntryArr,
            srfNumber,
            roFields[ii]);

        SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(devObjPtr,tempEntryArr,
            srfNumber,
            roFields[ii],
            tmpValue);
    }

    /* copy to the actual entry place */
    memPtr[-2] = tempEntryArr[0];
    memPtr[-1] = tempEntryArr[1];
    memPtr[ 0] = tempEntryArr[2];
}

/*******************************************************************************
*  smemSip6_30ActiveWritePreqSrfCountersMemory
*
* DESCRIPTION:
*      active Write to memory : support 'RO' fields , when write to preqSrfCounters
*
* INPUTS:
*       devObjPtr   - device object PTR.
*       address     - Address for ASIC memory.
*       memSize     - size of the requested memory
*       memPtr      - Pointer to the register's memory in the simulation.
*       param       - Registers' specific parameter.
*       inMemPtr    - Pointer to the memory to get register's content.
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static void smemSip6_30ActiveWritePreqSrfCountersMemory(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,  /*target*/
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr/*source*/
)
{
    GT_U32  tempEntryArr[5];
    GT_U32  origEntryArr[5];
    GT_U32  srfNumber = (address & 0xFFE0)>>5;/* entry index for the LOG*/
    GT_U32  tmpValue,ii;
    GT_U32  *internalDataPtr = smemTableInternalDataPtrGet(devObjPtr,address - 0x10/*start of entry*/);
    GT_U32  roFields[] = {
         SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E
        ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E
        ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E
        ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E
        ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E
        /* must be last */
        ,SMAIN_NOT_VALID_CNS
    };

    /*the entry as the CPU wanted to set the entry */
    tempEntryArr[0] = internalDataPtr[0]; /* word in internalDataOffset */
    tempEntryArr[1] = internalDataPtr[1]; /* word in internalDataOffset */
    tempEntryArr[2] = internalDataPtr[2]; /* word in internalDataOffset */
    tempEntryArr[3] = internalDataPtr[3]; /* word in internalDataOffset */
    tempEntryArr[4] =          *inMemPtr;

    /*the entry as the exists before we modify it */
    origEntryArr[0] = memPtr[-4]; /* word in the orig entry */
    origEntryArr[1] = memPtr[-3]; /* word in the orig entry */
    origEntryArr[2] = memPtr[-2]; /* word in the orig entry */
    origEntryArr[3] = memPtr[-1]; /* word in the orig entry */
    origEntryArr[4] = memPtr[ 0]; /* word in the orig entry */

    /* we need to make sure that 'RO' (read only) are not modified in the process ! */
    for(ii = 0 ; roFields[ii] != SMAIN_NOT_VALID_CNS ; ii++)
    {
        tmpValue =
        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,origEntryArr,
            srfNumber,
            roFields[ii]);

        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,tempEntryArr,
            srfNumber,
            roFields[ii],
            tmpValue);
    }

    /* copy to the actual entry place */
    memPtr[-4] = tempEntryArr[0];
    memPtr[-3] = tempEntryArr[1];
    memPtr[-2] = tempEntryArr[2];
    memPtr[-1] = tempEntryArr[3];
    memPtr[ 0] = tempEntryArr[4];
}


/**
* @internal smemIronmanUnitPreq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PREQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemIronmanUnitPreq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
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

        /* PREQ Unit Interrupt Cause */
        /*PREQ.preqInterrupts.preqInterruptCause*/
        {0x00000600, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg,0},
        /*PREQ.preqInterrupts.preqInterruptMask*/
        {0x00000604, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

        /* SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqSrfConfig) : support 'RO' fields */
        {0x00080008,0xFFFF000F, NULL, 0, smemSip6_30ActiveWritePreqSrfConfigMemory,0},
        /* SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqDaemons) : support 'RO' fields */
        {0x00090008,0xFFFF000F, NULL, 0, smemSip6_30ActiveWritePreqSrfDaemonsMemory,0},
        /* SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqSrfCounters) : support 'RO' fields */
        {0x000B0010,0xFFFF001F, NULL, 0, smemSip6_30ActiveWritePreqSrfCountersMemory,0},


        END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    }
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000054, 0x00000054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000100, 0x0000012C)}
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

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x0000111C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x0000127C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x0000137C)}

            /*Queue Offset Profile Mapping */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x0000097C)}

            /* PREQ Queue Port Mapping Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00010000, 228),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC( 26,  4), SMEM_BIND_TABLE_MAC(preqQueuePortMapping)}

            /* PREQ Profiles Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00030000, 4096),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(67, 16), SMEM_BIND_TABLE_MAC(preqProfiles)}
            /* PREQ Queue Configurations Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00040000, 12288),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC( 88, 16), SMEM_BIND_TABLE_MAC(preqQueueConfiguration)}
            /* PREQ Port Profile Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00050000,  256),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(92, 16), SMEM_BIND_TABLE_MAC(preqPortProfile)}
            /* PREQ Target Physical Port Table */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00060000, 256),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC( 12,  4), SMEM_BIND_TABLE_MAC(preqTargetPhyPort)}

            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00070000, 32768),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(76, 16),SMEM_BIND_TABLE_MAC(preqSrfMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 16384),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(91, 16),SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqSrfConfig)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 16384),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(79, 16),SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqDaemons)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000A0000, 16384),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(128, 16),SMEM_BIND_TABLE_MAC(preqHistoryBuffer)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000B0000, 32768),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(160, 32),SMEM_BIND_TABLE_WITH_LIMITED_WRITE_MAC(preqSrfCounters)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C0000, 128),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(preqZeroBitVector0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x000C1000, 128),
                SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(16, 4),SMEM_BIND_TABLE_MAC(preqZeroBitVector1)}


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
    /*FRE*/
    {
        GT_U32 ii;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.freSrfGlobalConfig0 = 0x00001100;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.freSrfGlobalConfig1 = 0x00001104;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.freGlobalConfig     = 0x00001108;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.LatentErrorPeriod   = 0x0000110C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.RestartPeriod       = 0x00001110;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.ArbiterPriority     = 0x00001114;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.ArbiterWeights      = 0x00001118;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.DaemonsRangeControl = 0x0000111C;
        for(ii = 0 ; ii < 32 ; ii++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.ErrorDetected[ii]   = 0x00001200 + ii * 4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.FRE.TakeAny      [ii]   = 0x00001300 + ii * 4;
        }
    }

    {/*preqInterrupts*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.preqInterrupts.preqInterruptCause =  0x00000600;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.preqInterrupts.preqInterruptMask  =  0x00000604;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.preqInterrupts.preqBadAddressLatch  =  0x00000608;
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
              /* preqTargetPhyPort initial values */
              {DUMMY_NAME_PTR_CNS,           0x00060000,         0x00000001,      64,    0x4   }

             ,{DUMMY_NAME_PTR_CNS,           0x00000000,         0x00000003,      1,    0x4    }/*Global_Config*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000054,         0x0000ffff,      1,    0x4    }/*PREQ_Metal_Fix*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000100,         0x00000050,      1,    0x4    }/*Fifos_Depth*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000828,         0x0000001f,      1,    0x4    }/*ingress_pkt_config*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000860,         0x0000003f,      1,    0x4    }/*query_fifo_min_peak*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000404,         0x000fffff,      1,    0x4    }/*global_tail_drop_limit*/
             ,{DUMMY_NAME_PTR_CNS,           0x0000040c,         0x000fffff,      2,    0x4    }/*global_tail_drop_limit*/
             ,{DUMMY_NAME_PTR_CNS,           0x0000041c,         0x000fffff,      2,    0x4    }/*mc_td_configuration*/
             ,{DUMMY_NAME_PTR_CNS,           0x00000828,         0x0000001F,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00000860,         0x0000007F,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00001104,         0x00020000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00001108,         0x0000000C,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x0000110C,         0x03D09000,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00001110,         0x0061A800,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00001114,         0x01012492,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x00001118,         0x04924924,      1,    0x4   }
             ,{DUMMY_NAME_PTR_CNS,           0x0000111C,         0x000FFC00,      1,    0x4   }


             ,{NULL,            0,           0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}


#define FDB_TILE_ID_GET(dev) \
    dev->numOfPipesPerTile ? \
        smemGetCurrentPipeId(dev) / dev->numOfPipesPerTile : \
        0
/*******************************************************************************
*   smemSip6_30FdbHsrPrpTimerGet
*
* DESCRIPTION:
*       Function to get the FDB HSR/PRP running timer.
*
* INPUTS:
*       devObjPtr - (pointer to) the device object
*       useFactor - the timer need to apply factor or not
*           GT_TRUE   - the timer is in 22 bits , without the 'timer factor'.
*                       (as exists in the timer register)
*           GT_FALSE  - the timer is in 32 bits 'micro seconds' , after applying
*                       the 'timer factor'
*        NOTE: the 'factor' is (80/40/20/10 micro-sec , from the <Timer Tick Time>
*               field in register 'HSR PRP Global Configuration')
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the timer (with/without factor).
*       if using factor --> the value is in micro seconds.
*
* COMMENTS:
*
*******************************************************************************/
static GT_U32 smemSip6_30FdbHsrPrpTimerGet
(
    IN struct SKERNEL_DEVICE_OBJECT_T * devObjPtr,
    IN GT_BOOL  useFactor
)
{
    GT_U32  currTimeNoFactor,currTimeWithFactor;
    GT_U32  tileId;
    GT_U32  factor;

    tileId = FDB_TILE_ID_GET(devObjPtr);

    factor = devObjPtr->hsrPrpTimer[tileId].factor;

    if(devObjPtr->hsrPrpTimer[tileId].pausedTimeNoFactor)
    {
        currTimeNoFactor   = devObjPtr->hsrPrpTimer[tileId].pausedTimeNoFactor;
        currTimeWithFactor = currTimeNoFactor * factor;
    }
    else
    {
        /* remove the time since 'base' was set by SW */
        currTimeWithFactor = SNET_GET_TIME_IN_MICROSEC_MAC();
        currTimeNoFactor   = currTimeWithFactor / factor;

        currTimeNoFactor &= 0x003FFFFF;/* fit into 22 bits*/
    }

    return (useFactor == GT_TRUE) ? currTimeWithFactor : currTimeNoFactor;
}

/**
* @internal smemSip6_30ActiveWriteFdbHsrPrpTimerReg function
* @endinternal
*
* @brief   Write the FDB HSR/PRP Timer register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemSip6_30ActiveWriteFdbHsrPrpTimerReg(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  oldValue  = * memPtr;
    GT_U32  tileId,timer;
    GT_U32  oldPause,newPause;

    /* data to be written */
    *memPtr = *inMemPtr;
    if(!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ||
       !devObjPtr->devMemHsrPrpTimerGetFuncPtr)
    {
        return;
    }

    oldPause = SMEM_U32_GET_FIELD( oldValue ,22,1);
    newPause = SMEM_U32_GET_FIELD( (* memPtr) ,22,1);

    tileId = FDB_TILE_ID_GET(devObjPtr);
    /*hsr_timer_pause*/
    if( oldPause == 0 &&
        newPause == 1 )
    {
        /* we need to pause the time, so get current value into the register */
        /* get the timer */
        timer = devObjPtr->devMemHsrPrpTimerGetFuncPtr(devObjPtr,GT_FALSE/*without factor*/);

        /* save the timer without factor */
        devObjPtr->hsrPrpTimer[tileId].pausedTimeNoFactor = timer;
    }
    else
    if( newPause == 0 )
    {
        devObjPtr->hsrPrpTimer[tileId].pausedTimeNoFactor = 0;
    }

    return;
}
/**
* @internal smemSip6_30ActiveReadFdbHsrPrpTimerReg function
* @endinternal
*
* @brief   Read the FDB HSR/PRP Timer register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output the 'running' Timer.
*/
void smemSip6_30ActiveReadFdbHsrPrpTimerReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32  timer;

    if(!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ||
       !devObjPtr->devMemHsrPrpTimerGetFuncPtr)
    {
        * outMemPtr = * memPtr;
        return;
    }

    if(SMEM_U32_GET_FIELD((* memPtr),22,1))/*hsr_timer_pause*/
    {
        /* the timer is paused , and kept as was when this was set to 1 */
        * outMemPtr = * memPtr;
        return;
    }

    /* get the timer */
    timer = devObjPtr->devMemHsrPrpTimerGetFuncPtr(devObjPtr,GT_FALSE/*without factor*/);

    SMEM_U32_SET_FIELD((* memPtr),0,22,timer);

    * outMemPtr = * memPtr;
    return;
}

/**
* @internal smemSip6_30ActiveWriteFdbHsrPrpGlobalConfigReg function
* @endinternal
*
* @brief   Write the FDB HSR/PRP global config.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemSip6_30ActiveWriteFdbHsrPrpGlobalConfigReg(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  tileId;

    /* data to be written */
    *memPtr = *inMemPtr;

    tileId = FDB_TILE_ID_GET(devObjPtr);

    switch(SMEM_U32_GET_FIELD((*memPtr) ,0,2))
    {
        case 0 : devObjPtr->hsrPrpTimer[tileId].factor = 80; break;
        case 1 : devObjPtr->hsrPrpTimer[tileId].factor = 40; break;
        case 2 : devObjPtr->hsrPrpTimer[tileId].factor = 20; break;
        default: devObjPtr->hsrPrpTimer[tileId].factor = 10; break;
    }

    return;
}


/**
* @internal smemIronmanSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemIronmanSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_DEVICE_UNIT_ALLOCATION_STC allocUnitsArr []=
    {
         {STR(UNIT_L2I)         ,smemIronmanUnitL2i}
        ,{STR(UNIT_EGF_SHT)     ,smemIronmanUnitEgfSht}
        ,{STR(UNIT_EGF_QAG)     ,smemIronmanUnitEgfQag}
        ,{STR(UNIT_EGF_EFT)     ,smemIronmanUnitEgfEft}
        ,{STR(UNIT_TTI)         ,smemIronmanUnitTti}
        ,{STR(UNIT_IPCL)        ,smemIronmanUnitIpcl}
        ,{STR(UNIT_EPCL)        ,smemIronmanUnitEpcl}
        ,{STR(UNIT_SMU)         ,smemIronmanUnitSmu}
        ,{STR(UNIT_HA)          ,smemIronmanUnitHa}
        ,{STR(UNIT_CNC)         ,smemIronmanUnitCnc}
        ,{STR(UNIT_PREQ)        ,smemIronmanUnitPreq}


        /* must be last */
        ,{NULL,NULL}
    };

    smemGenericUnitAlloc(devObjPtr,allocUnitsArr);
}

