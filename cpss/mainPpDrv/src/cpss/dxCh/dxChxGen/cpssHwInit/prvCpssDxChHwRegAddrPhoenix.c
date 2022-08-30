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
* @file prvCpssDxChHwRegAddrPhoenix.c
*
* @brief This file implement DB of units base addresses for Phoenix.
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
#define NUM_MG_PER_CNM   3
#define NUM_MGS_FOR_SDMA   2

/* There are up to 5 LED servers/units in Phoenix and only one LED interface */
/*
0x9E400000 + 0xa400000*(a-4): where a (5-5) represents 100G_LED
0x9E400000 + 0x7c00000+(a-2)*0x400000: where a (2-3) represents CPU_LED_MACRO
0x9E400000 + a*0x1000000: where a (0-1) represents USX_LED_MACRP
*/
#define PRV_CPSS_DXCH_PHOENIX_LED_UNIT_NUM_CNS                           5
/* There are up to 32 LED ports per one LED server.
   Only 27 LED ports (26 network ports + 1 CPU) are in use */
#define PRV_CPSS_DXCH_PHOENIX_LED_UNIT_PORTS_NUM_CNS                     32

#define GM_STEP_BETWEEN_MIB_PORTS   0x400
#define GM_STEP_BETWEEN_DPS         _64K


static GT_STATUS prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
);

/*
NOTE: copied from HAWK ... need to be aligned to actual addresses at some point.

*/
static const UNIT_IDS_STC phoenixUnitsIdsInPipes[] =
{
    /****************************************************************/
    /* NOTE: the table is sorted according to the base address ...  */
    /* BUT excluding the 'dummy' GOP addresses                      */
    /****************************************************************/


    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
                                     /*pipe0baseAddr*//*sizeInBytes*/
     {PRV_CPSS_DXCH_UNIT_LPM_E           ,0x87000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPVX_E          ,0x85800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_L2I_E           ,0x86000000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_CNC_0_E         ,0x84C00000,     256 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCL_E           ,0x84A00000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPLR_E          ,0x8F400000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EOAM_E          ,0x8F800000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_E          ,0x8EC00000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PREQ_E          ,0x8FC00000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_ERMRK_E         ,0x8E800000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IOAM_E          ,0x88800000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_MLL_E           ,0x88C00000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_E          ,0x88000000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,0x88400000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EQ_E            ,0x89000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,0x8B800000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,0x8BC00000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,0x8B400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TTI_E           ,0x84000000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PPU_E           ,0x84800000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_EREP_E          ,0x8D000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_BMA_E           ,0x8CE00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IA_E            ,0x91800000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E         ,0x91C00000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E         ,0x92000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,0x92400000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,0x95800000,     128  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,0x95400000,     512  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,0x95C00000,     64   * _1K }

    ,{PRV_CPSS_DXCH_UNIT_HA_E            ,0x90000000,     6   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PHA_E           ,0x8C000000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_HBU_E           ,0x8CC00000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TCAM_E                       ,0x8E000000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,0x96000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                   ,0x96400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EM_E                         ,0x8D400000,      4  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_FDB_E                        ,0x8D800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,0x96800000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x94000000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_COUNTER_E                 ,0x94200000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,0x93400000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x93800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,0x93C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x93000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,0x92C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x92800000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E                  ,0x9B000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         ,0x98800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E         ,0x9B400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  ,0x99400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E                  ,0x9A000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E                 ,0x9A200000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     ,0x99C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     ,0x9AC00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     ,0x99000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     ,0x98400000,     16  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E     ,0x98C00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E     ,0x99800000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E     ,0x98000000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E     ,0x9A800000,      1  * _1M }

    ,{PRV_CPSS_DXCH_UNIT_TTI_TAI0_E                   ,0x84400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXQS_TAI0_E                  ,0x96C00000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_TTI_TAI1_E                   ,0x84600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PHA_TAI0_E                   ,0x8C800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PHA_TAI1_E                   ,0x8CA00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI0_E               ,0x8F000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_HA_TAI1_E               ,0x8F200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX2_0_TAI0_E                ,0x9DC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX2_0_TAI1_E                ,0x9DE00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX2_1_TAI0_E                ,0x9EC00000,      64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_USX2_1_TAI1_E                ,0x9EE00000,      64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA0_TAI0_E                  ,0x9A400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA0_TAI1_E                  ,0x9A600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_L2I_TAI0_E                   ,0x85000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_L2I_TAI1_E                   ,0x85400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPUM_TAI0_E              ,0xA5400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPUM_TAI1_E              ,0xA5500000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPUC_TAI0_E              ,0xA5600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPUC_TAI1_E              ,0xA5700000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_100G_TAI0_E                  ,0xA8000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_100G_TAI1_E                  ,0xA8200000,     64  * _1K }


    ,{PRV_CPSS_DXCH_UNIT_DFX_SERVER_E    ,PRV_CPSS_PHOENIX_DFX_BASE_ADDRESS_CNS,      1  * _1M }

    ,{PRV_CPSS_DXCH_UNIT_TAI_E           ,0x84400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TAI1_E          ,0x84600000,     64  * _1K }

    ,{PRV_CPSS_DXCH_UNIT_MG_E            ,PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS+MG_SIZE*0,MG_SIZE }
    ,{PRV_CPSS_DXCH_UNIT_MG_0_1_E        ,PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS+MG_SIZE*1,MG_SIZE }
    ,{PRV_CPSS_DXCH_UNIT_MG_0_2_E        ,PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS+MG_SIZE*2,MG_SIZE }

    ,{PRV_CPSS_DXCH_UNIT_CNM_RFU_E       ,0x80010000,     64  * _1K }
    /* SMI is in the CnM section */
    ,{PRV_CPSS_DXCH_UNIT_SMI_0_E         ,0x80580000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_SMI_1_E         ,0x80590000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNM_MPP_RFU_E   ,0x80020000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_CNM_AAC_E       ,0x80030000,     64  * _1K }

    /* units that where in 'Raven' and now part of the 'device'  */

    /* LED */
/*UNIT_LED_USX2_0_LED  */   ,{PRV_CPSS_DXCH_UNIT_LED_0_E          ,0x9E400000     , 4   *     _1K}
/*UNIT_LED_USX2_1_LED  */   ,{PRV_CPSS_DXCH_UNIT_LED_1_E          ,0x9F400000     , 4   *     _1K}
/*UNIT_LED_MAC_CPU_LED0*/   ,{PRV_CPSS_DXCH_UNIT_LED_2_E          ,0xA6000000     , 4   *     _1K}
/*UNIT_LED_MAC_CPU_LED1*/   ,{PRV_CPSS_DXCH_UNIT_LED_3_E          ,0xA6400000     , 4   *     _1K}
/*UNIT_LED_MAC_100G_0 */    ,{PRV_CPSS_DXCH_UNIT_LED_4_E          ,0xA8800000     , 4   *     _1K}

    /* 100G mac */
    ,{PRV_CPSS_DXCH_UNIT_MAC_400G_0_E     ,0xA7800000     ,64   *     _1K}

    /* 100G pcs */
    ,{PRV_CPSS_DXCH_UNIT_PCS_400G_0_E     ,0xA7C00000     ,64   *     _1K}

    /* cpu mac*/
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E      ,0xA5000000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MAC_CPU_1_E      ,0xA5200000     ,16   *     _1K}
    /* cpu pcs */
    ,{PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E      ,0xA1000000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_PCS_CPU_1_E      ,0xA1400000     ,16   *     _1K}


    ,{PRV_CPSS_DXCH_UNIT_SERDES_E         ,0xA8C00000     ,512   *    _1K}
    ,{PRV_CPSS_DXCH_UNIT_SERDES_1_E       ,0xA9400000     ,512   *    _1K}
#if 0
    ,UNIT_INFO_MAC(0xA8C00000     ,UNIT_SERDES_SDW0        ,512   *     _1K     )
    ,UNIT_INFO_MAC(0xA9000000     ,UNIT_SERDES_SDW5        ,512   *     _1K     )
    ,UNIT_INFO_MAC(0xA9400000     ,UNIT_SERDES_SDW4_1      ,512   *     _1K     )
    ,UNIT_INFO_MAC(0xA9500000     ,UNIT_SERDES_SDW4_2      ,512   *     _1K     )
    ,UNIT_INFO_MAC(0xA9600000     ,UNIT_SERDES_SDW4_3      ,512   *     _1K     )
    ,UNIT_INFO_MAC(0xA9700000     ,UNIT_SERDES_SDW4_4      ,512   *     _1K     )
#endif

    /* mif USX*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_0_E      ,0x9E000000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_1_E      ,0x9F000000     ,16   *     _1K}
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_2_E      ,0xA5800000     ,16   *     _1K}
    /* mif CPU*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E      ,0xA5C00000     ,16   *     _1K}
    /* mif 100G */
    ,{PRV_CPSS_DXCH_UNIT_MIF_400G_0_E     ,0xA8400000     ,16   *     _1K}

    /* anp 400G */
    ,{PRV_CPSS_DXCH_UNIT_ANP_400G_0_E     ,0xA7E00000     ,64   *     _1K}
    /* anp USX*/
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_0_E      ,0xA2800000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_1_E      ,0xA2C00000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_2_E      ,0xA3000000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_3_E      ,0xA3400000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_4_E      ,0x9F800000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_5_E      ,0x9FC00000     ,64   *     _1M}

    ,{PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E      ,0xA0000000     ,64   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_ANP_CPU_1_E      ,0xA0400000     ,64   *     _1M}

    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E  ,0x9D800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_0_E  ,0x9DA00000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_0_E  ,0x9E800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_1_E  ,0x9EA00000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_1_E  ,0xA4800000     ,64  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_1_E  ,0xA4C00000     ,64  *  _1K}

    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E  ,0xA3800000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_0_E  ,0xA3C00000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_0_E  ,0xA4000000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_1_E  ,0xA4400000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_1_E  ,0xA0800000     ,32  *  _1K}
    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_1_E  ,0xA0C00000     ,32  *  _1K}

#if 0
    /* anp CPU*/
    ,{PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E        ,0x1A600000     ,64   * _1K}
#endif

    /* dummy for MG in Ravens - for BWC interrupts */
    /* see use in file : cpssDriver\pp\interrupts\dxExMx\cpssDrvPpIntDefDxChPhoenix.c*/
    ,{PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS + PRV_CPSS_DXCH_UNIT_MG_E, 0x40000000, 4   * _1M     }

#ifdef GM_USED  /* NOTE : no GM for Phoenix */
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
#define MG1_SERVED_DP   0

#define FIRST_CPU_SDMA_DMA_NUM   (PHOENIX_PORTS_PER_DP_CNS-2)/*54*/
#define SECOND_CPU_SDMA_DMA_NUM  (PHOENIX_PORTS_PER_DP_CNS-1)/*55*/


/* NOTE : the device hold no 'CPU reduced ports' that are muxed with the SDMAs */
static const SPECIAL_DMA_PORTS_STC phoenix_SpecialDma_ports[]= {
   /*54*/{MG0_SERVED_DP,FIRST_CPU_SDMA_DMA_NUM /*54*//*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*55*/{MG1_SERVED_DP,SECOND_CPU_SDMA_DMA_NUM/*55*//*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   };
static const GT_U32 num_ports_phoenix_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(phoenix_SpecialDma_ports);

/**
* @internal prvCpssPhoenixUnitIdSizeInByteGet function
* @endinternal
*
* @brief   To get the size of the unit in bytes
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - the device number
* @param[in] prvUnitId                - Id of DxCh unit(one of the PRV_CPSS_DXCH_UNIT_ENT)
*
* @param[out] unitIdSize               - size of the unit in bytes
*/
static GT_STATUS prvCpssPhoenixUnitIdSizeInByteGet
(
    IN  GT_U8                      devNum,
    IN  PRV_CPSS_DXCH_UNIT_ENT     prvUnitId,
    OUT GT_U32                    *unitIdSizePtr
)
{
    const UNIT_IDS_STC   *unitIdInPipePtr = &phoenixUnitsIdsInPipes[0];
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

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "unit [%d] not supported in Phoenix",
        prvUnitId);
}

#define IS_SUPPORT_MAC_100G_MAC(macCapability)  \
    (MAC_CAPABILITY_50G_100G_E == macCapability)
typedef enum{
    MAC_CAPABILITY_50G_100G_E ,/* 50G ,100G (also lower than  50G)  */
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

    if(portMacNum >= FIRST_CPU_SDMA_DMA_NUM)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is unknown",
            portMacNum);
    }

    macInfoPtr->dpIndex       = portMacNum / PHOENIX_PORTS_PER_DP_CNS;
    macInfoPtr->localPortInDp = portMacNum % PHOENIX_PORTS_PER_DP_CNS;

    macInfoPtr->macCapability = MAC_CAPABILITY_UNKNOWN_E;

    switch (unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Unit[%d] is not supported", unitId);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E:
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
                case 50:
                case 51:
                case 52:
                case 53:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_50G_100G_E;
                    macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp-50;/*0..3*/
                    macInfoPtr->ciderUnit    = 0;
                    break;
                default:
                    macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is not implemented",
                        portMacNum);
            }
            break;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            switch(macInfoPtr->localPortInDp)
            {
                case 48:
                case 49:
                    macInfoPtr->macCapability    = MAC_CAPABILITY_CPU_E;
                    macInfoPtr->ciderIndexInUnit = 0;
                    macInfoPtr->ciderUnit    = macInfoPtr->localPortInDp-48;/*0..1*/
                    break;
                default:
                    macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is not implemented",
                        portMacNum);
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
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E:

            if (macInfoPtr->localPortInDp >= 48)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is not implemented in USX",
                    portMacNum);

            }
            macInfoPtr->ciderIndexInUnit = macInfoPtr->localPortInDp % 8;   /*0..7*/
            macInfoPtr->ciderUnit        = macInfoPtr->localPortInDp / 8;   /*0..5*/
            break;

        default:
            macInfoPtr->ciderIndexInUnit = 0;
            macInfoPtr->ciderUnit        = 0;
            break;
    }



    return GT_OK;
}

/* offsets within the : Phoenix/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x00001000
#define PORT0_100G_OFFSET         0x00002000

#define CPU_MAC_EXT_BASE_OFFSET       0x00000000
#define CPU_MAC_PORT_OFFSET           0x00002000
#define CPU_MAC_MIB_OFFSET            0x00001000

#define PCS_PORT0_100G_OFFSET     0x00000000
#define PCS_PORT0_50G_OFFSET      0x00001000
#define PCS_RS_FEC_OFFSET         0x00005000
#define PCS_RS_FEC_STAT_OFFSET    0x00007000
#define PCS_LPCS_OFFSET           0x00006000

#define ANP_STEP_UNIT_OFFSET      0x00200000

static GT_U32 /*GT_UREG_DATA*/   phoenixUnitBaseAddrCalc(
    GT_U8                                   devNum,
    GT_U32/*HWS_UNIT_BASE_ADDR_TYPE_ENT*/   unitId,
    GT_U32/*GT_UOPT*/                       portNum
);

static GT_STATUS   prvCpssPhoenixRegDbInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN PRV_CPSS_REG_DB_TYPE_ENT    regDbType,
    IN PRV_CPSS_REG_DB_INFO_STC   *regDbInfoPtr
);

GT_VOID prvCpssPhoenixNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
)
{
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssHwsUnitBaseAddrCalcFunc)        = phoenixUnitBaseAddrCalc;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssRegDbInfoGetFunc)               = prvCpssPhoenixRegDbInfoGet;
    DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert;
}

/**
* @internal phoenixUnitBaseAddrCalc function
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
GT_U32 /*GT_UREG_DATA*/   phoenixUnitBaseAddrCalc(
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
        if ((portNum <= 3) || ((portNum > 7) && (portNum <= 11)))
        {
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_SERDES_E), NULL);
            relativeAddr = ((portNum /8) * 0x400000); /* 28G Quad lanes Comphy SD */
        }
        else if(((portNum > 3) && (portNum <= 7)))
        {
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_SERDES_1_E), NULL);
            relativeAddr = (0x100000 * (portNum % 4)); /* 28G Single lane Comphy SD */
        }
        else
        {
            return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
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
                "HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E nor supported in Phoenix");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E nor supported in Phoenix");/* not supported !!! */
        case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E nor supported in Phoenix");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:   /* 0..255 */
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:   /* 0..255 */
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
            if(MAC_CAPABILITY_CPU_E      != macCapability)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(portNum == 48)
            {
                baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E), NULL);
            }
            else if(portNum == 49)
            {
                baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_CPU_1_E), NULL);
            }
            else
            {
                baseAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
            baseAddr = phoenixUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E, portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_PORT_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
            baseAddr = phoenixUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + CPU_MAC_EXT_BASE_OFFSET;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
            baseAddr = phoenixUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
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

            if(portNum == 48)
            {
                baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E), NULL);
            }
            else if(portNum == 49)
            {
                baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_CPU_1_E), NULL);
            }
            else
            {
                baseAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
            baseAddr = phoenixUnitBaseAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E,portNum);
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            baseAddr = phoenixUnitBaseAddrCalc(devNum,HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E,portNum);
            baseAddr += 0x2000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = MAC_MIB_OFFSET;/* shared to 8 ports */

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
                baseAddr = phoenixUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E , portNum);
                if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                {
                    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                }

                return baseAddr;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MAC_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = MAC_EXT_BASE_OFFSET;/* shared to 8 ports */

            return baseAddr + relativeAddr;


        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E   :
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_PORT0_100G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E    :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E    :
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_PORT0_50G_OFFSET + macInfo.ciderIndexInUnit * 0x1000;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E   :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E   :
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_LPCS_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_RS_FEC_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E:
            if(!IS_SUPPORT_MAC_100G_MAC(macCapability))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCS_400G_0_E + macInfo.ciderUnit), NULL);

            relativeAddr = PCS_RS_FEC_STAT_OFFSET;

            return baseAddr + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_400G_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_MIF_USX_0_E + (macInfo.ciderUnit / 2)), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_400G_0_E + macInfo.ciderUnit), NULL);
            return (baseAddr + 0x4000 + macInfo.ciderIndexInUnit * 0x1000);

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + macInfo.ciderUnit), NULL);
            return (baseAddr + 0x4000);

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E   :
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            return(baseAddr + 0x4000);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderUnit;
            baseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x2000 + macInfo.ciderIndexInUnit * 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E:
            if(portNum >= 48)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + macInfo.ciderUnit;
            baseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x3000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderUnit;
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, cpssUnitId, NULL);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x4000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E:
            cpssUnitId = PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + macInfo.ciderUnit;
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
* @internal prvDxChHwRegAddrPhoenixDbInit function
* @endinternal
*
* @brief   init the base address manager of the Phoenix device.
*         prvDxChPhoenixUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrPhoenixDbInit(GT_VOID)
{
    const UNIT_IDS_STC   *currEntryPtr = &phoenixUnitsIdsInPipes[0];
    GT_U32                  globalIndex = 0;/* index into prvDxChPhoenixUnitsIdUnitBaseAddrArr */
    GT_U32                  ii;
    GT_U32                  errorCase = 0;
    GT_U32                  maxUnits = PHOENIX_MAX_UNITS;

    /* bind HWS with function that calc base addresses of units */
    /* NOTE: bind was already done by 'Falcon'
       hwsFalconUnitBaseAddrCalcBind(phoenixUnitBaseAddrCalc); */

    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        if(globalIndex >= maxUnits)
        {
            errorCase = 1;
            goto notEnoughUnits_lbl;
        }

        /* add the unit in pipe 0 */
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChPhoenixUnitsIdUnitBaseAddrArr)[globalIndex].unitId = currEntryPtr->pipe0Id;
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChPhoenixUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr;
        globalIndex++;
    }

    if(globalIndex >= maxUnits)
    {
        errorCase = 2;
        goto notEnoughUnits_lbl;
    }

    /* set the 'last index as invalid' */
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChPhoenixUnitsIdUnitBaseAddrArr)[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChPhoenixUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;

    /* avoid warning if 'CPSS_LOG_ENABLE' not defined
      (warning: variable 'errorCase' set but not used [-Wunused-but-set-variable])
    */
    if(errorCase == 0)
    {
        errorCase++;
    }

    return GT_OK;

notEnoughUnits_lbl:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :PHOENIX_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,maxUnits , globalIndex+1);
}
/**
* @internal prvCpssDxChPhoenixHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
static PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChPhoenixHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    const UNIT_IDS_STC   *currEntryPtr;
    GT_U32  ii;

    devNum = devNum;
    currEntryPtr = &phoenixUnitsIdsInPipes[0];

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

/* check if register address is one 'per MG' SDMA , and allow to convert address
   of MG 0 to address of other MG  */
static GT_BOOL  phoenix_convertPerMg_mgSdmaAddr(
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
static GT_BOOL  phoenix_allowDuplicatePerMg_mgSdmaAddr(
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


    if (GT_TRUE == phoenix_convertPerMg_mgSdmaAddr(devNum, regAddr))
    {
        /* allow MG[1] to be able to use MG[0] addresses !        */
        isSdmaRegister = 1;
    }
    else
    if (GT_TRUE == phoenix_allowDuplicatePerMg_mgSdmaAddr(devNum, regAddr))
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
* @internal prvCpssPhoenixDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Phoenix device.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P.
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
GT_BOOL prvCpssPhoenixDuplicatedMultiPortGroupsGet_byDevNum
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

    /* call direct to prvCpssDxChPhoenixHwRegAddrToUnitIdConvert ... not need to
       get to it from prvCpssDxChHwRegAddrToUnitIdConvert(...) */
    unitId = prvCpssDxChPhoenixHwRegAddrToUnitIdConvert(devNum, regAddr);
    /* set the 'orig' unit at index 0 */
    unitsIndex = 0;
    usedUnits[unitsIndex++] = unitId;

    switch(unitId)
    {
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
* @internal prvCpssPhoenixOffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
static GT_U32   prvCpssPhoenixOffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
)
{
    /* use 'PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]' because it may be called from the cpssDriver
       before the 'PRV_CPSS_PP_MAC(devNum)' initialized ! */
    switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            break;
        default:
            /* was not implemented */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA, LOG_ERROR_NO_MSG);
    }

    switch(instance0UnitId)
    {
        default:
            break;/* no distance */

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E               :
        {
            /* MG 0,1 - supported for SDMA */
            /* MG2 is for CM3 usage , and the main CPU not use it */

            if(instanceId >= NUM_MGS_FOR_SDMA)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA, LOG_ERROR_NO_MSG);
            }

            return  (MG_SIZE  * instanceId);
        }
    }

    return 0;/* no distance */
}


/**
* @internal phoenixSpecialPortsMapGet function
* @endinternal
*
* @brief   Phoenix : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS phoenixSpecialPortsMapGet
(
    OUT const SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    *specialDmaPortsPtrPtr   =           phoenix_SpecialDma_ports;
    *numOfSpecialDmaPortsPtr = num_ports_phoenix_SpecialDma_ports;

    return GT_OK;
}

/**
* @internal prvCpssPhoenixDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Phoenix : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssPhoenixDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
)
{
    devNum = devNum;
    /* device with single DP */
    if(dpIndexPtr)
    {
        *dpIndexPtr     = 0;
    }
    if(localDmaNumPtr)
    {
        *localDmaNumPtr = globalDmaNum;
    }

    return GT_OK;
}

/**
* @internal prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Phoenix : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
static GT_STATUS prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
)
{
    devNum = devNum;

    /* device with single DP  */
    *globalDmaNumPtr = localDmaNum;

    if(dpIndex != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "dpIndex [%d] must be 0",
            dpIndex);
    }

    return GT_OK;
}

/**
* @internal prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Phoenix : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
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
static GT_STATUS prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  globalDmaNum;

    /* check validity and get 'first globalDmaNum' */
    rc = prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert(devNum,
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

    /* no muxing in Phoenix */

    return /* not error for the LOG */ GT_NO_MORE;
}

/* return the BMP of local macs that may effect the local mac */
static GT_U32   macBmpEclipseSerdesBehindGet(IN GT_U32 portMacNum)
{
    /* the speeds in Phoenix uses single 25G serdeses */
    /* port 50 can              use 2 or 4 serdeses */
    /* port 52 can              use 2      serdeses */
    if(portMacNum >= 52)
    {
        return BIT_0 | BIT_2;
    }
    if(portMacNum >= 50)
    {
        return BIT_0;
    }
    return BIT_0;/*only 'self'*/
}

/* return the BMP of local macs that may be effected by the local mac */
static GT_U32   macBmpEclipseSerdesAHeadGet(
    IN GT_U32 portMacNum,
    IN GT_U32 numOfActLanes)
{
    GT_U32  maxLanes = 1;

    if(portMacNum == 52)
    {
        maxLanes = 2;
    }
    else
    if(portMacNum == 50)
    {
        maxLanes = 4;
    }

    if(numOfActLanes > 1 && (numOfActLanes & 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA,
            "portMacNum [%d] not support odd [%d] lanes ",
            portMacNum,numOfActLanes);
    }

    if(numOfActLanes > maxLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA,
            "portMacNum [%d] not support [%d] lanes (max is [%d])",
            portMacNum,numOfActLanes,maxLanes);
    }

    return (1 << numOfActLanes) - 1;
}
/* return the SERDES global id for the global MAC */
static GT_U32  macSerdesGet(
    IN GT_U32                      portMacNum
)
{
    GT_U32  globalSerdes;

    if(portMacNum >= 54)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA,
            "portMacNum [%d] not supported",
            portMacNum);
    }

    if(portMacNum < 48)/*0..47*/
    {
        globalSerdes = portMacNum / 8;   /* first 6 serdeses */
    }
    else  /*48..53*/
    {
        globalSerdes = 6 + (portMacNum - 48); /* last 6 serdeses */
    }

    return globalSerdes;
}


/**
* @internal prvCpssPhoenixRegDbInfoGet function
* @endinternal
*
* @brief   function to get the info to index in 'reg DB'
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum          - The PP's device number.
* @param[in] portMacNum      - the global mac port number.
* @param[out] regDbType      - the type of regDbInfo.
* @param[out] regDbInfoPtr   - (pointer to) the reg db info
* @return - GT_OK on success
*/
GT_STATUS   prvCpssPhoenixRegDbInfoGet(
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
        case PRV_CPSS_REG_DB_TYPE_MTI_USX_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
        case PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E:
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
        case PRV_CPSS_REG_DB_TYPE_PB_CHANNEL_ID_E:
            if(portMacNum < 48)
            {
                unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E;
            }
            else if(portMacNum < 50)
            {
                unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E;
            }
            else
            {
                unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E;
            }
            break;
        case PRV_CPSS_REG_DB_TYPE_FIRST_SERDES_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_SERDES_E;
            break;
        case PRV_CPSS_REG_DB_TYPE_USX_MTI_MIB_COUNTER_SHARED_E:
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E;
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
            regDbInfoPtr->regDbIndex       = macInfo.ciderUnit;
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;
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
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesBehindGet(portMacNum);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = portMacNum < 50 ? portMacNum : 50/*4 serdeses that can be used together*/;
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesAHeadGet(portMacNum ,
                regDbInfoPtr->startMacIn_macBmpEclipseSerdes/*numOfActLanes*/);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = portMacNum;/*unlike PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E*/
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

/**
* @internal prvCpssPhoenixInitParamsSet function
* @endinternal
*
* @brief  Phoenix  : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssPhoenixInitParamsSet
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  ii,index;
    GT_U32  numMgUnitsPerTile = NUM_MGS_FOR_SDMA;
    GT_U32  numMgUnits = numMgUnitsPerTile;
    GT_U32  frameLatencyArrPhoenix[] =  {0x797,0x797,0x797,0x795,0x794,0x796,0x796,0x796,0x794,0x792};
    GT_U32  frameLatencyFracArrPhoenix[] =  {0xBC, 0xBC, 0xBC, 0xD5, 0x00, 0x30, 0x30, 0x30, 0x00, 0x20};

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

    dxDevPtr->hwInfo.multiDataPath.supportMultiDataPath = 0;
    dxDevPtr->hwInfo.multiDataPath.rxNumPortsPerDp = PHOENIX_PORTS_PER_DP_CNS;
    dxDevPtr->hwInfo.multiDataPath.txNumPortsPerDp = PHOENIX_PORTS_PER_DP_CNS;

    dxDevPtr->hwInfo.multiDataPath.maxDp = 1;/* single DP in the device */

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts = 0;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts = 2;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts = PHOENIX_PORTS_PER_DP_CNS-2/*2 SDMAs*/;


    PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = 12/*25G*/;

    /* led info */
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum = PRV_CPSS_DXCH_PHOENIX_LED_UNIT_NUM_CNS;
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts   = PRV_CPSS_DXCH_PHOENIX_LED_UNIT_PORTS_NUM_CNS;


    /* get info for 'SDMA CPU' port numbers */
    rc = phoenixSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

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
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum = specialDmaPortsPtr[ii].localPortInDp;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].valid  = GT_TRUE;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].usedAsCpuPort = GT_FALSE;/* the 'port mapping' should bind it */
            /* parameter called 'dqNum' but should be considered as 'DP index' */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dqNum  = specialDmaPortsPtr[ii].dpIndex;
            /* since single DP the global and local are the same */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].localDmaNum = dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum;
            if(dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum == SECOND_CPU_SDMA_DMA_NUM)
            {
                dxDevPtr->hwInfo.cpuPortInfo.info[index].use_cpuPortDmaNum1 = GT_TRUE;
            }
            index++;
        }
    }

    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = ii * PHOENIX_PORTS_PER_DP_CNS;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = PHOENIX_PORTS_PER_DP_CNS;/* support CPU port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum      = FIRST_CPU_SDMA_DMA_NUM; /*54*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum1     = SECOND_CPU_SDMA_DMA_NUM;/*55*/
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks = 30;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 1*_1K;/*10K  in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap = _32K;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = 640;/* phoenix : 640 lines , in non shared block  */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  =  2;/* phoenix : 2  blocks for 'small' from non shared     */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = 30;/* phoenix : 30 blocks supported (28 of 1K + 2 of 640) */

    /* PHA info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg =  2; /*  4 in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn =  8; /* 10 in Hawk */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts = 56;/* 108 in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues = 64*8;/* (32*8) = 256 in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups = 64;/* 128 in Hawk */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxPizzaSize =  124; /* 92 in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxNumOfSpeedProfiles = MAX_SPEED_PROFILE_NUM_HAWK_CNS;

    /* SBM info : not using 'Shared memory' ! */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed = 1;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm       = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm  = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.bmpSbmSupportMultipleArp = 0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm     = 0;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort = 8;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId = 3;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors = 6;/* 6 *3K = 18K@10B = 9K@20B */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups = 3;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits = 1;/* single LMU unit for all ports */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum = _16K;

    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits = 1;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEportLttEcmp =    0; /* like Hawk not limited .. so according to eports (8K) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL2Ecmp       =  _8K; /* Hawk  8K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL3Ecmp       =  _4K; /* Hawk 12K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIpNh         =  _8K; /* Hawk 16K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt       = _12K; /* Hawk 16K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllPairs     =  _4K; /* Hawk  8K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering =  _2K; /* Hawk  4K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrMetering =  _2K; /* Hawk  4K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrIpfix    = _16K; /* Hawk 64K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrIpfix    = _16K; /* Hawk 64K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam          =  _2K; /* Hawk  2K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTxqQueue     =    0; /* not relevant to sip6 devices */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesProtectionLoc=  _2K; /* Hawk  2K */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb          = _32K; /* Hawk 256K , in shared tables (in phoenix no shared tables)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm           = _32K; /* Hawk 256K , in shared tables (in phoenix no shared tables)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart  =  _4K; /* Hawk 64K , in shared tables (in phoenix no shared tables)*//* used only by ENTRY_TYPE_TUNNEL_START_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesPortIsolation= _4K+128; /* used only by CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E/CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx         = _12K; /* used only by CPSS_DXCH_TABLE_MULTICAST_E */

    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vid              =  _6K; /* Hawk not limited .. 8K */
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vidx             = _12K; /* Hawk 12K */
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).stgId            =  _1K; /* Hawk not limited .. 4K */
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).l2LttMll         = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt;

    /* the only DP 0 has MACSec */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp = 0x1;


    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits = PRV_CPSS_NUM_OF_TAI_IN_PHOENIX_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr = cpssOsMalloc((PRV_CPSS_NUM_OF_TAI_IN_PHOENIX_CNS-1)*sizeof(GT_U32));
    cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr, frameLatencyArrPhoenix, sizeof(frameLatencyArrPhoenix));
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr = cpssOsMalloc((PRV_CPSS_NUM_OF_TAI_IN_PHOENIX_CNS-1)*sizeof(GT_U32));
    cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr, frameLatencyFracArrPhoenix, sizeof(frameLatencyFracArrPhoenix));
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz = 800000;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssUnitIdSizeInByteGetFunc        = prvCpssPhoenixUnitIdSizeInByteGet;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssOffsetFromFirstInstanceGetFunc = prvCpssPhoenixOffsetFromFirstInstanceGet;
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssPhoenixDmaGlobalNumToLocalNumInDpConvert;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc = prvCpssPhoenixDmaLocalNumInDpToGlobalNumConvert_getNext;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssHwRegAddrToUnitIdConvertFunc   = prvCpssDxChPhoenixHwRegAddrToUnitIdConvert;
    prvCpssPhoenixNonSharedHwInfoFuncPointersSet(devNum);

    return GT_OK;
}

