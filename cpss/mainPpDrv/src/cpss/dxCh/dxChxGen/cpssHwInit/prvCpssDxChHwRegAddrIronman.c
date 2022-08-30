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
* @file prvCpssDxChHwRegAddrIronman.c
*
* @brief This file implement DB of units base addresses for Ironman.
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
#define NUM_MG_PER_CNM   1
#define NUM_MGS_FOR_SDMA   1

/* There are up to 5 LED servers/units in Ironman and only one LED interface */
/*
0x9E400000 + 0xa400000*(a-4): where a (5-5) represents 100G_LED
0x9E400000 + 0x7c00000+(a-2)*0x400000: where a (2-3) represents CPU_LED_MACRO
0x9E400000 + a*0x1000000: where a (0-1) represents USX_LED_MACRP
*/
#define PRV_CPSS_DXCH_IRONMAN_LED_UNIT_NUM_CNS                           3
/* There are up to 32 LED ports per one LED server.
   Only 27 LED ports (26 network ports + 1 CPU) are in use */
#define PRV_CPSS_DXCH_IRONMAN_LED_UNIT_PORTS_NUM_CNS                     32

#define GM_STEP_BETWEEN_MIB_PORTS   0x400
#define GM_STEP_BETWEEN_DPS         _64K

#define NO_ADDR/* the address not in Ironmanl_Address_Space_rev3.8.TF_211007 */
/*

Aligned by Ironmanl_Address_Space_rev3.8.TF_211007

*/
static const UNIT_IDS_STC ironmanLUnitsIdsInPipes[] =
{
    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
                                     /*pipe0baseAddr*//*sizeInBytes*/
     {PRV_CPSS_DXCH_UNIT_LPM_E           ,0x87000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPVX_E          ,0x85800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_L2I_E           ,0x86000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_CNC_0_E         ,0x85000000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCL_E           ,0x84200000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EPLR_E          ,0x8bc00000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EPCL_E          ,0x8b800000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PREQ_E          ,0x8a600000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_ERMRK_E         ,0x8a800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IOAM_E          ,PRV_CPSS_IRONMAN_IOAM_BASE_ADDRESS_CNS/*0x89200000*/,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_MLL_E           ,0x89000000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_E          ,0x89400000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,0x89800000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EQ_E            ,0x88000000,     16  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,0x8a100000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,0x8a200000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,0x8a000000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TTI_E           ,0x84400000,     4   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_SMU_E           ,0x89c00000,     4   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_EREP_E          ,0x8a400000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_BMA_E           ,0x85200000,     2   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_HBU_E           ,0x8c000000,     4   * _1M }
    /*,{PRV_CPSS_DXCH_UNIT_IA_E            ,0x0cf00000,     64  * _1K }*/

    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E         ,0x8d000000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E         ,0x8ce00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,0x8cf00000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,0x8d500000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,0x8d100000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,0x8d300000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_HA_E            ,0x8b000000,     8   * _1M }
/*    ,{PRV_CPSS_DXCH_UNIT_PHA_E           ,0x08000000,     8   * _1M }*/

    ,{PRV_CPSS_DXCH_UNIT_TCAM_E                       ,0x84800000,     8   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,0x8d400000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                   ,0x8d200000,     1   * _1M }
/*  ,{PRV_CPSS_DXCH_UNIT_SHM_E                        ,0x--------,     64  * _1K }*/
/*  ,{PRV_CPSS_DXCH_UNIT_EM_E                         ,0x05000000,      4  * _1M }*/
    ,{PRV_CPSS_DXCH_UNIT_FDB_E                        ,PRV_CPSS_IRONMAN_FDB_BASE_ADDRESS_CNS/*0x85400000*/,     4  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,0x8d600000,     1   * _1M }

    ,{PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x8cc00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_COUNTER_E                 ,0x8ca00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x8cd00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x8cb00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x8c900000,     1   * _1M }

/*    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E                  ,0x0e100000,     64  * _1K }*/
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E                 ,0x91000000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  ,0x90f00000,     1   * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         ,0x91100000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     ,0x90d80000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     ,0x90700000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     ,0x91180000,     512 * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     ,0x90900000,     512 * _1K }

    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E     ,0x90800000,    1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E     ,0x90e00000,    1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E     ,0x90a00000,    1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E     ,0x91200000,    1    * _1M }

    ,{PRV_CPSS_DXCH_UNIT_DFX_SERVER_E    ,PRV_CPSS_IRONMAN_DFX_BASE_ADDRESS_CNS,      2  * _1M } /*0x84000000*/


    ,{PRV_CPSS_DXCH_UNIT_TAI_E           , 0x8c400000,     1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TAI1_E          , 0x8c500000,     1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TAI2_E          , 0x8c600000,     1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TAI3_E          , 0x8c700000,     1    * _1M }
    ,{PRV_CPSS_DXCH_UNIT_TAI4_E          , 0x8c800000,     1    * _1M }

NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MG_E            ,PRV_CPSS_IRONMAN_MG0_BASE_ADDRESS_CNS+MG_SIZE*0,MG_SIZE } /*0x7F900000*/

NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_CNM_RFU_E       ,0x800B0000      ,     64  * _1K }
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_CNM_MPP_RFU_E   ,0x80020000      ,     64  * _1K }
    /* SMI is in the CnM section */
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_SMI_0_E         ,0x80580000      ,     64  * _1K }
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_SMI_1_E         ,0x80590000      ,     64  * _1K }
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_CNM_AAC_E       ,0x80030000      ,     64  * _1K }

NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_CNM_RUNIT_E     ,0x7F010000      ,     64  * _1K }

    /* LED 400G */
    ,{PRV_CPSS_DXCH_UNIT_LED_0_E          ,0x93700000     , 1   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_LED_1_E          ,0x98700000     , 1   *     _1M}
    ,{PRV_CPSS_DXCH_UNIT_LED_2_E          ,0x99700000     , 1   *     _1M}


NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E  ,0x0d600000            ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_0_E  ,0x0d600000+_64K*1     ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_0_E  ,0x0d600000+_64K*2     ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_1_E  ,0x0d600000+_64K*3     ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_MAC_1_E  ,0x0d600000+_64K*4     ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_MAC_1_E  ,0x0d600000+_64K*5     ,64  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_3_MAC_0_E  ,0x0d600000+_64K*6     ,64  *  _1K}

NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E  ,0x0d900000            ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_0_E  ,0x0d900000+_64K*1     ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_0_E  ,0x0d900000+_64K*2     ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_1_E  ,0x0d900000+_64K*3     ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_1_PCS_1_E  ,0x0d900000+_64K*4     ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_2_PCS_1_E  ,0x0d900000+_64K*5     ,32  *  _1K}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_MTI_USX_3_PCS_0_E  ,0x0d900000+_64K*6     ,32  *  _1K}

    /* mif USX*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_0_E      ,0x98600000     ,1   *     _1M} /*epi_usx_pe_nic_macro*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_1_E      ,0x99600000     ,1   *     _1M} /*epi_usx_pe_1_macro*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_2_E      ,0x93600000     ,1   *     _1M} /*epi_usx_pe_2_macro*/
    ,{PRV_CPSS_DXCH_UNIT_MIF_USX_3_E      ,0x92300000     ,1   *     _1M} /*epi_cpu_port_macro*/


    /* anp USX*/
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_0_E      ,0x0d800000            ,64   *     _1M}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_1_E      ,0x0d800000+_64K*1     ,64   *     _1M}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_2_E      ,0x0d800000+_64K*2     ,64   *     _1M}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_3_E      ,0x0d800000+_64K*3     ,64   *     _1M}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_4_E      ,0x0d800000+_64K*4     ,64   *     _1M}
NO_ADDR    ,{PRV_CPSS_DXCH_UNIT_ANP_USX_5_E      ,0x0d800000+_64K*5     ,64   *     _1M}

#if 0
/*SDW0*/,{PRV_CPSS_DXCH_UNIT_SDW0_0_E     ,0x00b00000       , 512   *     _1K}
/*SDW1*/,{PRV_CPSS_DXCH_UNIT_SDW1_0_E     ,0x0d100000       , 512   *     _1K}
/*SDW2*/,{PRV_CPSS_DXCH_UNIT_SDW0_1_E     ,0x0d180000       , 512   *     _1K}
/*SDW3*/,{PRV_CPSS_DXCH_UNIT_SDW1_1_E     ,0x0f100000       , 512   *     _1K}
/*SDW4*/,{PRV_CPSS_DXCH_UNIT_SDW0_2_E     ,0x0f180000       , 512   *     _1K}
#endif/*0*/

#ifdef GM_USED
    /*************************************************************/
    /* GM Simulation uses dummy MIB memory space (need to be     */
    /* synch with file GmProjectFunctions.cpp of GM)             */
    /*************************************************************/
    ,{PRV_CPSS_DXCH_UNIT_MIB_E,0xA0000000, GM_STEP_BETWEEN_DPS * 4     }/* single chunk , support 56 ports */
#endif


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

#define IM_L_CPU_SDMA_DMA_NUM   56
#define IM_S_CPU_SDMA_DMA_NUM   25


/* NOTE : the device hold no 'CPU reduced ports' that are muxed with the SDMAs */
static const SPECIAL_DMA_PORTS_STC ironman_l_SpecialDma_ports[]= {
   /*56*/{MG0_SERVED_DP,IM_L_CPU_SDMA_DMA_NUM /*56*//*localPortInDp*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   };
static const GT_U32 num_ports_ironman_l_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(ironman_l_SpecialDma_ports);


/**
* @internal prvCpssIronmanUnitIdSizeInByteGet function
* @endinternal
*
* @brief   To get the size of the unit in bytes
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - the device number
* @param[in] prvUnitId                - Id of DxCh unit(one of the PRV_CPSS_DXCH_UNIT_ENT)
*
* @param[out] unitIdSize               - size of the unit in bytes
*/
static GT_STATUS prvCpssIronmanUnitIdSizeInByteGet
(
    IN  GT_U8                      devNum,
    IN  PRV_CPSS_DXCH_UNIT_ENT     prvUnitId,
    OUT GT_U32                    *unitIdSizePtr
)
{
    const UNIT_IDS_STC   *unitIdInPipePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr;
    GT_U32 ii;

    for(ii = 0 ; unitIdInPipePtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,unitIdInPipePtr++)
    {
        if(unitIdInPipePtr->pipe0Id == prvUnitId)
        {
            *unitIdSizePtr = unitIdInPipePtr->sizeInBytes;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "unit [%d] not supported in Ironman",
        prvUnitId);
}

typedef enum{
    MAC_CAPABILITY_1G_E ,/* 1G ,2.5G , 5G */
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

    if(portMacNum >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portMacNum[%d] is unknown",
            portMacNum);
    }

    macInfoPtr->dpIndex       = 0;
    macInfoPtr->localPortInDp = portMacNum;

    macInfoPtr->macCapability = MAC_CAPABILITY_UNKNOWN_E;

    switch (unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E:
            macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
            return /* not error for the LOG */ GT_OUT_OF_RANGE;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E:
            macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
            return /* not error for the LOG */ GT_OUT_OF_RANGE;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E:
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            macInfoPtr->macCapability    =  MAC_CAPABILITY_1G_E;
            return /* not error for the LOG */ GT_OUT_OF_RANGE;

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

/* offsets within the : Ironman/Core/EPI/<400_MAC>MTIP IP 400 MAC WRAPPER/ */
#define MAC_EXT_BASE_OFFSET       0x00000000
#define MAC_MIB_OFFSET            0x00001000
#define PORT0_100G_OFFSET         0x00002000

#define CPU_MAC_EXT_BASE_OFFSET       0x00000000
#define CPU_MAC_PORT_OFFSET           0x00002000
#define CPU_MAC_MIB_OFFSET            0x00001000

#define PCS_PORT0_100G_OFFSET     0x00000000
#define PCS_PORT0_50G_OFFSET      0x00001000
#define PCS_RS_FEC_OFFSET         0x00005000
#define PCS_LPCS_OFFSET           0x00006000

#define ANP_STEP_UNIT_OFFSET      0x00200000


static GT_U32   getUsxPcsAddr(IN GT_U8  devNum, IN GT_U32 ciderUnit)
{
    GT_U32  id ;
    id = ciderUnit > 5 ?
        PRV_CPSS_DXCH_UNIT_MTI_USX_3_PCS_0_E + (ciderUnit-6) :
        PRV_CPSS_DXCH_UNIT_MTI_USX_0_PCS_0_E + ciderUnit;
    return prvCpssDxChHwUnitBaseAddrGet(devNum, id, NULL);
}
static GT_U32   getUsxMacAddr(IN GT_U8  devNum, IN GT_U32 ciderUnit)
{
    GT_U32  id = ciderUnit;
    id = ciderUnit > 5 ?
        PRV_CPSS_DXCH_UNIT_MTI_USX_3_MAC_0_E + (ciderUnit-6) :
        PRV_CPSS_DXCH_UNIT_MTI_USX_0_MAC_0_E + ciderUnit;
    return prvCpssDxChHwUnitBaseAddrGet(devNum, id, NULL);
}

/**
* @internal ironmanUnitBaseAddrCalc function
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
static GT_U32 /*GT_UREG_DATA*/   ironmanUnitBaseAddrCalc(
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

    if(unitId == HWS_UNIT_BASE_ADDR_TYPE_SERDES_E)
    {
                /*dummy*/
        baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_SERDES_E), NULL);
        relativeAddr = (0x100000 * (portNum % 8));

        return baseAddr + relativeAddr;
    }

    rc =  macInfoGet(devNum,portNum,unitId,&macInfo);
    if(rc != GT_OK)
    {
        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    dpIndex        = macInfo.dpIndex;

    switch(unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
            /* note : the 32 local ports share the same 'base' address !!! */
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E + dpIndex/*0..3*/), NULL);

            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E:  /* 0..15 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E not supported in Ironman");/* not supported !!! */
        case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E:   /* 0..31 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:   /* 0..255 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:   /* 0..255 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E not supported in Ironman");/* not supported !!! */

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
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E not supported in Ironman");/* not supported !!! */


        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E    :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E    :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E/HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E   :
        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E/HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E/HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E   :
            {
                GT_U32  id = macInfo.ciderUnit/2;
                id = id > 2 ? PRV_CPSS_DXCH_UNIT_MIF_USX_3_E + (id-3) : PRV_CPSS_DXCH_UNIT_MIF_USX_0_E + id;
                baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, id, NULL);
                return baseAddr;
            }

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_AN_400_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_AN_400_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E   :
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_SW_PTR_ENTRY_UNUSED,
                "HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E not supported in Ironman");/* not supported !!! */

        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E   :
        case HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E   :
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + macInfo.ciderUnit), NULL);
            return(baseAddr + 0x4000);

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E:
            baseAddr  = getUsxMacAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x2000 + macInfo.ciderIndexInUnit * 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E:
            baseAddr  = getUsxMacAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E:
            baseAddr  = getUsxMacAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E:
            baseAddr   = getUsxPcsAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x1000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E:
            baseAddr   = getUsxPcsAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x0000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E:
            baseAddr   = getUsxPcsAddr(devNum, macInfo.ciderUnit);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            baseAddr += 0x3000;
            return baseAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E:
            baseAddr   = getUsxPcsAddr(devNum, macInfo.ciderUnit);
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
* @internal prvDxChHwRegAddrIronmanLDbInit function
* @endinternal
*
* @brief   init the base address manager of the Ironman-L device.
*         prvDxChIronmanLUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Ironman-L.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvDxChHwRegAddrIronmanLDbInit(GT_VOID)
{
    const UNIT_IDS_STC   *currEntryPtr = &ironmanLUnitsIdsInPipes[0];
    GT_U32                  globalIndex = 0;/* index into prvDxChIronmanLUnitsIdUnitBaseAddrArr */
    GT_U32                  ii;
    GT_U32                  errorCase = 0;
    GT_U32                  maxUnits = IRONMAN_MAX_UNITS;

    /* bind HWS with function that calc base addresses of units */
    /* NOTE: bind was already done by 'Falcon'
       hwsFalconUnitBaseAddrCalcBind(ironmanUnitBaseAddrCalc); */

    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        if(globalIndex >= maxUnits)
        {
            errorCase = 1;
            goto notEnoughUnits_lbl;
        }

        /* add the unit in pipe 0 */
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChIronmanLUnitsIdUnitBaseAddrArr)[globalIndex].unitId = currEntryPtr->pipe0Id;
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChIronmanLUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr;
        globalIndex++;
    }

    if(globalIndex >= maxUnits)
    {
        errorCase = 2;
        goto notEnoughUnits_lbl;
    }

    /* set the 'last index as invalid' */
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChIronmanLUnitsIdUnitBaseAddrArr)[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChIronmanLUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;

    /* avoid warning if 'CPSS_LOG_ENABLE' not defined
      (warning: variable 'errorCase' set but not used [-Wunused-but-set-variable])
    */
    if(errorCase == 0)
    {
        errorCase++;
    }

    return GT_OK;

notEnoughUnits_lbl:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :IRONMAN_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,maxUnits , globalIndex+1);
}

/**
* @internal prvDxChHwRegAddrIronmanDbInit function
* @endinternal
*
* @brief   init the base address manager of the Ironman device.
*         prvDxChIronmanLUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrIronmanDbInit(GT_VOID)
{
    GT_STATUS rc;

    /* init the DB of IM-L */
    rc = prvDxChHwRegAddrIronmanLDbInit();
    return rc;
}

/**
* @internal prvCpssDxChIronmanHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
static PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChIronmanHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    const UNIT_IDS_STC   *currEntryPtr;
    GT_U32  ii;

    devNum = devNum;
    currEntryPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr;

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

/**
* @internal prvCpssIronmanOffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
static GT_U32   prvCpssIronmanOffsetFromFirstInstanceGet
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
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            break;
        default:
            /* was not implemented */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA, LOG_ERROR_NO_MSG);
    }

        GT_UNUSED_PARAM(instanceId);
        GT_UNUSED_PARAM(instance0UnitId);

    return 0;/* no distance */
}


/**
* @internal ironmanSpecialPortsMapGet function
* @endinternal
*
* @brief   Ironman : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS ironmanSpecialPortsMapGet
(
    OUT const SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    *specialDmaPortsPtrPtr   =           ironman_l_SpecialDma_ports;
    *numOfSpecialDmaPortsPtr = num_ports_ironman_l_SpecialDma_ports;
    return GT_OK;
}

/**
* @internal prvCpssIronmanDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Ironman : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssIronmanDmaGlobalNumToLocalNumInDpConvert
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
* @internal prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Ironman : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
static GT_STATUS prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert
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
* @internal prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Ironman : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
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
static GT_STATUS prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert_getNext
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
    rc = prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert(devNum,
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

    /* no muxing in Ironman */

    return /* not error for the LOG */ GT_NO_MORE;
}

/* return the BMP of local macs that may effect the local mac */
static GT_U32   macBmpEclipseSerdesBehindGet(IN GT_U32 portMacNum)
{
    /* the speeds in Ironman uses single serdeses */
    portMacNum = portMacNum;

    return BIT_0;/*only 'self'*/
}

/* return the BMP of local macs that may be effected by the local mac */
static GT_U32   macBmpEclipseSerdesAHeadGet(
    IN GT_U32 portMacNum,
    IN GT_U32 numOfActLanes)
{
    GT_U32  maxLanes = 1;

    if(numOfActLanes > maxLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA,
            "portMacNum [%d] not support [%d] lanes (max is [%d])",
            portMacNum,numOfActLanes,maxLanes);
    }

    portMacNum = portMacNum;

    return (1 << numOfActLanes) - 1;
}
/* return the SERDES global id for the global MAC */
static GT_U32  macSerdesGet(
    IN GT_U32                      portMacNum
)
{
    GT_U32  globalSerdes;

    if(portMacNum >= 55)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA,
            "portMacNum [%d] not supported",
            portMacNum);
    }

    globalSerdes = portMacNum / 8;   /* first 6 serdeses */

    return globalSerdes;
}


/**
* @internal prvCpssIronmanRegDbInfoGet function
* @endinternal
*
* @brief   function to get the info to index in 'reg DB'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum          - The PP's device number.
* @param[in] portMacNum      - the global mac port number.
* @param[out] regDbType      - the type of regDbInfo.
* @param[out] regDbInfoPtr   - (pointer to) the reg db info
* @return - GT_OK on success
*/
static GT_STATUS   prvCpssIronmanRegDbInfoGet(
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
            unitId = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E;
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
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = portMacNum;
            break;
        case PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = macInfo.ciderIndexInUnit;/* the local port index for Cider address calculations  */
            regDbInfoPtr->macBmpEclipseSerdes = macBmpEclipseSerdesAHeadGet(portMacNum ,
                regDbInfoPtr->startMacIn_macBmpEclipseSerdes/*numOfActLanes*/);
            regDbInfoPtr->startMacIn_macBmpEclipseSerdes = portMacNum;
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

/* list of the Ironman-L : common */
static const CPSS_PP_DEVICE_TYPE devs_ironman_l_common[]=
{
    PRV_CPSS_IRONMAN_L_ALL_DEVICES,
    LAST_DEV_IN_LIST_CNS /* must be last one */
 };

/* max lines in Phoenix */
#define IRONMAN_L_MAX_LPM_CNS ((_1K*28)+(2*640))


/* Ironman - fine tuning table  sizes according to devices flavors */
static const FINE_TUNING_STC ironmanTables[]=
{
    { /* IronMan : common */
        devs_ironman_l_common,
        {
            NULL,  /*enhancedInfoPtr*/
            {
                /* bridge section                             */
                _32K,            /*fdb                        */
                _4K,            /*vidxNum                    */
                _1K,             /*stgNum                     */
                                 /*                           */
                /* TCAM section                               */
                0,               /* GT_U32 router;            */
                12*_1K,/*6K@20B*//* GT_U32 tunnelTerm;        */
                                 /*                           */
                /* tunnel section                             */
                (_4K/*ARPs*//4),   /*GT_U32  tunnelStart;   */
        /*_4K*/AUTO_CALC_FIELD_CNS,/*GT_U32  routerArp;     */
                                 /*                          */
                /* ip section                                */
                (_1K + 512),     /*GT_U32  routerNextHop;    */
                _2K,             /*GT_U32  ecmpQos;          */
                _4K,             /*GT_U32  mllPairs;         */
                                 /*                           */
                /* PCL section                                */
                PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC,/*GT_U32 policyTcamRaws;    */

                                 /*                           */
                /* CNC section                                */
                CNC_AUTO_CALC,  /* GT_U32 cncBlocks;          */
                _8K,/*single unit*//* GT_U32 cncBlockNumEntries; */

                /*  Policer Section                           */
                /* NOTE: 2K is shared memory between IPLR0,1,EPLR (unlike legacy devices) */
                _2K,           /*policersNum                  */
                512,           /*egressPolicersNum            */
                                 /*                           */
                /* trunk Section                              */
                L2_ECMP_TRUNK_ENTRIES_MAC(TRUNK_NUM_MAX_AUTO_CALC),/*trunk,L2Ecmp - members (using hwInfo.parametericTables.numEntriesL2Ecmp)*/
                                 /*                           */
                /* Transmit Descriptors                       */
                40*_1K,             /*GT_U32 transmitDescr;     */  CPSS_TBD_BOOKMARK_PHOENIX
                                 /*                           */
                /* Buffer Memory                              */
                2*_1M,              /*GT_U32 bufferMemory;      */
                TX_QUEUE_NUM_8_CNS,  /* txQueuesNum - TX Queues number */
                PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(IRONMAN_L_MAX_LPM_CNS),       /* GT_U32  lpmRam,  */
                AUTO_CALC_FIELD_CNS, /* pipeBWCapacityInGbps auto calculated by hwPpPhase1Part2Enhanced_BandWidthByDevRevCoreClockGet */
                NA_TABLE_CNS,        /* bpePointToPointEChannels  */
                0,                   /*emNum                      */
                _4K,                 /* vrfIdNum                                    */
                128                  /* oamEntriesNum                               */
            },

            {
                /* TR101 Feature support */
                GT_TRUE, /*tr101Supported*/

                /* VLAN translation support */
                GT_TRUE, /*vlanTranslationSupported*/

                /* Policer Ingress second stage support flag                  */
                GT_TRUE, /*iplrSecondStageSupported*/

                /* trunk CRC hash support flag */
                GT_TRUE, /*trunkCrcHashSupported*/

                /* TM support flag */
                GT_FALSE  /* TmSupported */
            }
        }
    }
};
static GT_U32 ironmanTables_size = sizeof(ironmanTables)/sizeof(ironmanTables[0]);

/* list the TAI '0' units in the Ironman */
static const PRV_CPSS_DXCH_UNIT_ENT  ironmanTai0UnitsArray[] =
{
    PRV_CPSS_DXCH_UNIT_TAI_E            ,
    PRV_CPSS_DXCH_UNIT_TAI1_E           ,
    PRV_CPSS_DXCH_UNIT_TAI2_E           ,
    PRV_CPSS_DXCH_UNIT_TAI3_E           ,
    PRV_CPSS_DXCH_UNIT_TAI4_E
};

GT_VOID prvCpssIronmanNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
)
{
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssHwsUnitBaseAddrCalcFunc)        = ironmanUnitBaseAddrCalc;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssRegDbInfoGetFunc)               = prvCpssIronmanRegDbInfoGet;
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssIronmanDmaGlobalNumToLocalNumInDpConvert;
    DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(devNum)= prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert;
}


/**
* @internal prvCpssIronmanInitParamsSet function
* @endinternal
*
* @brief  Ironman  : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssIronmanInitParamsSet
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
    GT_U32  ePorts;/*number of eports*/

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
    dxDevPtr->hwInfo.multiDataPath.rxNumPortsPerDp = 57;
    dxDevPtr->hwInfo.multiDataPath.txNumPortsPerDp = dxDevPtr->hwInfo.multiDataPath.rxNumPortsPerDp;

    dxDevPtr->hwInfo.multiDataPath.maxDp = 1;/* single DP in the device */

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts = 0;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts = 0;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts = 55;


    PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = 12/*25G*/;

    /* led info */
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum = PRV_CPSS_DXCH_IRONMAN_LED_UNIT_NUM_CNS;
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts   = PRV_CPSS_DXCH_IRONMAN_LED_UNIT_PORTS_NUM_CNS;

    /* get info for 'SDMA CPU' port numbers */
    rc = ironmanSpecialPortsMapGet(&specialDmaPortsPtr,&numOfSpecialDmaPorts);
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
            index++;
        }
    }

    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = 0;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = dxDevPtr->hwInfo.multiDataPath.rxNumPortsPerDp;/* support CPU port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum      = IM_L_CPU_SDMA_DMA_NUM;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks = 9;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 1536;/*10K  in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap = _32K;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = 640;/* ironman : 640 lines , in non shared block  */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  =  0;/* ironman : 2  blocks for 'small' from non shared     */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared =  9;/* ironman : 30 blocks supported (28 of 1K + 2 of 640) */

    /* NO PHA support !!! */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg =  0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn =  0;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts = 57;/* 108 in Hawk */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues = 57*8;/* (32*8) = 256 in Hawk */
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

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.limitedNumOfParrallelLookups = 2;/* instead of 4 like in legacy devices */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.limitedNumOfParrallelLookups =
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.limitedNumOfParrallelLookups;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.numDefaultEports = 64;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_myPhysicalPortAttributes_numEntries = 64;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries = 16;/*128 in all other sip5 devices*/

    /* limit the number of PCLs to 2 instead of 3 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass = GT_TRUE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.limitedNumOfParrallelLookups =
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.limitedNumOfParrallelLookups;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId = 1;/*single group*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors = 4;/* 4 *3K = 12K@10B = 6K@20B */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxClientGroups = 1;/*single group !*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits = 0;/* NO LMU units */

    /* no EOAM support */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.oam.notSupportEgressOam  = GT_TRUE;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.preqInfo.portMappingNum = 8;/*8 registers (not 128 like previous devices)*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum = _2K;

    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits  = 1;
    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlocks = 8;

    ePorts = _1K;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEportLttEcmp =  _1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL2Ecmp       =  _1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL3Ecmp       =  _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIpNh         =  (_1K + 512);
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt       =  _4K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllPairs     =  _4K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering =  _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrMetering =
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering;/* IPLR and EPLR are 'shared' */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrIpfix    =  _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrIpfix    =  _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam          =  128;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTxqQueue     =    0; /* not relevant to sip6 devices */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesProtectionLoc=  ePorts/8;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb          = _32K; /* Hawk 256K , in shared tables (in ironman no shared tables)*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm           =    0;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart  = (_4K/4); /* used only by ENTRY_TYPE_TUNNEL_START_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesPortIsolation= _4K+64; /* used only by CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E/CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx         = _4K; /* used only by CPSS_DXCH_TABLE_MULTICAST_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesLpmVrfId     = _4K; /* used only by CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E / ipv6 / fcoe */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVlanMembers  = _4K + 512;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStream       = _2K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf    = _1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist= _1K;

    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).ePort            = ePorts;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).phyPort          = 64;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vid              = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVlanMembers;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vidx             = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).stgId            = _1K;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).l2LttMll         = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt;
    PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).trunkId          = 256;

    /* the only DP 0 has MACSec */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp = 0x1;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.myFamilyFineTuningPtr  = ironmanTables;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.myFamilyFineTuningSize = ironmanTables_size;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.devUnitsInfoPtr        = ironmanLUnitsIdsInPipes;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits = 1; /* max number of GOPs; in Ironman only single GOP 0 */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.taiUnitsPtr   = ironmanTai0UnitsArray;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais = NUM_ELEMENTS_IN_ARR_MAC(ironmanTai0UnitsArray);
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr = NULL;        /* CPSS_TBD_BOOKMARK_IRONMAN */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr = NULL;    /* CPSS_TBD_BOOKMARK_IRONMAN */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz = 600000; /* IronMan use Core Clock for TAIs */

    /*In Ironman the IA unit not exists , and there is no 'ingress oversubscribe' in the device*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pipInfo.isIaUnitNotSupported  = GT_TRUE;

    /* PPU unit not supported */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ppuInfo.notSupported = GT_TRUE;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssUnitIdSizeInByteGetFunc        = prvCpssIronmanUnitIdSizeInByteGet;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssOffsetFromFirstInstanceGetFunc = prvCpssIronmanOffsetFromFirstInstanceGet;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc = prvCpssIronmanDmaLocalNumInDpToGlobalNumConvert_getNext;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssHwRegAddrToUnitIdConvertFunc   = prvCpssDxChIronmanHwRegAddrToUnitIdConvert;

    /* state that the devices supports the preemption */
    PRV_CPSS_PP_MAC(devNum)->preemptionSupported = GT_TRUE;

    prvCpssIronmanNonSharedHwInfoFuncPointersSet(devNum);

    return GT_OK;
}



