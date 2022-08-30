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
* mvserdes28nmDb.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmDb.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>

#include <cpss/common/labServices/port/gop/port/private/mvHwsPortApCoCpuIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconGpSram.h>

/******************************** enums ***************************************/

/******************************** structures **********************************/

/******************************** globals *************************************/
MV_CFG_SEQ* hwsSerdesSeqDbPtr; /* used for sequence set/get */

#ifndef CO_CPU_RUN

static MV_OP_PARAMS SerdesSpeed_1_25gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0330, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF247, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C0, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0x0066, 0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x0800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x0087, 0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x0040, 0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0000, 0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_3_125gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0440, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF247, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C9, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0x88,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_3_75gSeq[] = {
    {SERDES_UNIT, 0x000, 0x04C8, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF247, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C9, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0x99,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_4_25gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0550, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF247, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C9, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xAA,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_5gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0198, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF147, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C9, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0x33,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_6_25gSeq[] = {
    {SERDES_UNIT, 0x000, 0x05D8, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF147, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17C9, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xBB,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_7_5gSeq[] = {
    {SERDES_UNIT, 0x000, 0x06E8, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xDD,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x800,  0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_10_3125gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0770, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xf047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xEE,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x1800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_11_25gSeq[] = {
    {SERDES_UNIT, 0x000, 0x07F8, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xFF,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x1800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_11_5625gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0770, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xFF,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x1800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_12_5gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0770, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xFF,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x1800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};
static MV_OP_PARAMS SerdesSpeed_12_1875gSeq[] = {
    {SERDES_UNIT, 0x000, 0x0770, 0x07F8}, /* Setting PIN_GEN_TX, PIN_GEN_RX */
    {SERDES_UNIT, 0x028, 0x000C, 0x001F}, /* PIN_FREF_SEL=C (156.25MHz) */
    {SERDES_PHY_UNIT, 0x018, 0x028F, 0xFFFF}, /* FFE_R=0x0, FFE_C=0xF, SQ_THRESH=0x2 */
    {SERDES_PHY_UNIT, 0x01C, 0xF047, 0xFFFF}, /* DFE_RES = 3.3mv */
    {SERDES_PHY_UNIT, 0x020, 0xAFC0, 0xFFFF}, /* DFE UPDAE all coefficient */
    {SERDES_PHY_UNIT, 0x034, 0x0DFE, 0xFFFF}, /* TX_AMP=31, AMP_ADJ=1 */
    {SERDES_PHY_UNIT, 0x038, 0x17D2, 0xFFFF}, /* MUPI/F=2, rx_digclkdiv=2 */
    {SERDES_PHY_UNIT, 0x084, 0x4030, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x088, 0x1026, 0xFFFF}, /* set password */
    {SERDES_PHY_UNIT, 0x094, 0x1FFF, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x098, 0xFF,   0xFFFF}, /* Set Gen_RX/TX */
    {SERDES_PHY_UNIT, 0x104, 0x2208, 0xFFFF}, /* EMPH0 enable, EMPH_mode=2 */
    {SERDES_PHY_UNIT, 0x134, 0x0052, 0xFFFF}, /* RX_IMP_VTH=2, TX_IMP_VTH=2 */
    {SERDES_PHY_UNIT, 0x13C, 0xA026, 0xFBFF}, /* clock set    */
    {SERDES_PHY_UNIT, 0x140, 0x1800, 0xFFFF}, /* clk 8T enable for 10G and up */
    {SERDES_PHY_UNIT, 0x154, 0x87,   0xFFFF}, /* rxdigclk_dv_force=1  */
    {SERDES_PHY_UNIT, 0x16C, 0x40,   0xC0  }, 
    {SERDES_PHY_UNIT, 0x184, 0x1093, 0xFFFF}, /* DTL_FLOOP_EN=0 */
    {SERDES_PHY_UNIT, 0x1A8, 0x4000, 0xFFFF}, /* Force ICP=7 */
    {SERDES_PHY_UNIT, 0x1C8, 0x07fe, 0xFFFF}, /* rxdigclk_dv_force=1*/
    {SERDES_PHY_UNIT, 0x39C, 0x0558, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x3BC, 0x4200, 0xFFFF}, 
    {SERDES_PHY_UNIT, 0x004, 0xFC8C, 0xFFFF}, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x370, 0x0,    0x4   }, /* PHY_MODE=0x4,FREF_SEL=0xC */
    {SERDES_PHY_UNIT, 0x108, 0xB0BA, 0xFFFF},
    {SERDES_PHY_UNIT, 0x10C, 0xE08C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x110, 0xCC8C, 0xFFFF},
    {SERDES_PHY_UNIT, 0x114, 0xA745, 0xFFFF}
};


/* SERDES_PT_AFTER_PATTERN_NORMAL */
static MV_OP_PARAMS serdesPtAfterPatternNormalParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0xE0,   0xF0},
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x0,    0x8000},
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x4000, 0x4000},
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x0,    0x4000},
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,            0x800,  0x800},
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,            0x0,    0x800}
};

/* SERDES_PT_AFTER_PATTERN_TEST_SEQ */
static MV_OP_PARAMS serdesPtAfterPatternTestParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,  0x80E0, 0x80F0}
};

/* SERDES_SD_RESET_SEQ Sequence init */
static MV_OP_PARAMS serdesSdResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x8}
};

/* SERDES_SD_UNRESET_SEQ Sequence init */
static MV_OP_PARAMS serdesSdUnResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x8,    0x8}
};

/* SERDES_RF_RESET Sequence init */
static MV_OP_PARAMS serdesRfResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x40}
};

/* SERDES_RF_UNRESET Sequence init */
static MV_OP_PARAMS serdesRfUnResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x40,   0x40}
};

/* SERDES_SYNCE_RESET_SEQ Sequence init */
static MV_OP_PARAMS serdesSynceResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,  0x0,    0x8}
};

/* SERDES_SYNCE_UNRESET_SEQ Sequence init */
static MV_OP_PARAMS serdesSynceUnResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0xDD00, 0xFF00},
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_2,  0xB,    0xB}
};

/* SERDES_SERDES_POWER_DOWN_CTRL_SEQ Sequence init */
static MV_OP_PARAMS serdesPowerDownCtrlParams[] =
{
    {SERDES_UNIT,   SERDES_EXTERNAL_CONFIGURATION_0,    0x0,    0x1802},
    {SERDES_UNIT,   SD_METAL_FIX,                       0x100,  0xFFFF},
    {SERDES_UNIT,   SERDES_RESERVED_REGISTER_2,         0x0,    0xFFFF}
};

/* SERDES_SERDES_RXINT_UP Sequence init */
static MV_OP_PARAMS serdesRxIntUpParams[] =
{
    {SERDES_PHY_UNIT,   COM_H_28NM_PHY_Isolate_Reg,     0x400,  0x400},
    {SERDES_PHY_UNIT,   COM_H_28NM_SEL_BITS_Reg,        0x40,   0x40 },
    {SERDES_PHY_UNIT,   COM_H_28NM_PHY_Isolate_Reg,     0,      0x400}
};

/* SERDES_SERDES_RXINT_DOWN Sequence init */
static MV_OP_PARAMS serdesRxIntDownParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0,  0x10 },
    {SERDES_PHY_UNIT, COM_H_28NM_PHY_Isolate_Reg,   0,  0x400}
};

/* SERDES_SERDES_WAIT_PLL Sequence init */
static MV_OP_PARAMS serdesWaitPllParams[] =
{
    {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0X8000, 0x8000},
    {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0x4000, 0x4000},
    {SERDES_PHY_UNIT,   COM_H_28NM_KVCO_CAL_Reg,    0,      0x8000}    
};

/* SERDES_SD_LPBK_NORMAL_SEQ */
static MV_OP_PARAMS serdesLpbkNormalParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,           0,      0xF000},
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,     0,      0x8000},
    {SERDES_PHY_UNIT, COM_H_28NM_PU_PLL_Reg,             0,      0x800 }
};

/* SERDES_SD_ANA_TX_2_RX_SEQ */
static MV_OP_PARAMS serdesLpbkAnaTx2RxParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,       0x2000, 0x2000}
};

/* SERDES_SD_DIG_RX_2_TX_SEQ */
static MV_OP_PARAMS serdesLpbkDigRx2TxParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_PU_PLL_Reg,              0x800,  0x800   },
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0xD0,   0xF0    },
    {SERDES_PHY_UNIT, COM_H_28NM_PT_PATTERN_SEL_Reg,      0x8000, 0x8000  },
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,              0x9000, 0x9000}
};

/* SERDES_SD_DIG_TX_2_RX_SEQ */
static MV_OP_PARAMS serdesLpbkDigTx2RxParams[] =
{
    {SERDES_PHY_UNIT, COM_H_28NM_SEL_BITS_Reg,    0x4000, 0x4000}
};

/* SERDES_TX_TRAINING_ENABLE_SEQ Sequence init */
static MV_OP_PARAMS serdesTxTrainingEnParams[] =
{
    {SERDES_PHY_UNIT, 0x188,    0x2A60, 0xFFFF},
    {SERDES_PHY_UNIT, 0x94,     0x1FFF, 0xFFFF},
    {SERDES_PHY_UNIT, 0x18,     0x28F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6047, 0xFFFF},
    {SERDES_PHY_UNIT, 0x20,     0xAFC0, 0xFFFF},
    {SERDES_PHY_UNIT, 0x38,     0xFD2,  0xFFFF},
    {SERDES_PHY_UNIT, 0x1A8,    0x4000, 0xFFFF},
    {SERDES_PHY_UNIT, 0x214,    0x2118, 0xFFFF},
    {SERDES_PHY_UNIT, 0x22C,    0x7470, 0xFFFF},
    {SERDES_PHY_UNIT, 0x330,    0xD8F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x36C,    0x3C4,  0xFFFF},
    {SERDES_PHY_UNIT, 0x280,    0xC8F3, 0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x2,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x0,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x1,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x0,    0xFFFF},
    {SERDES_PHY_UNIT, 0x278,    0x21F3, 0xFFFF},
    {SERDES_PHY_UNIT, 0x228,    0x2B40, 0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x1658, 0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x16D8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x294,    0x484,  0xFFFF},
    {SERDES_PHY_UNIT, 0x294,    0xFFF,  0xFFFF},
    {SERDES_PHY_UNIT, 0x18,     0x20F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x330,    0xD0F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x228,    0x2BC0, 0xFFFF},
    {SERDES_PHY_UNIT, 0x31C,    0x21B,  0xFFFF},
    {SERDES_PHY_UNIT, 0x228,    0x2FC0, 0xFFFF},
    {SERDES_PHY_UNIT, 0x228,    0x2F80, 0xFFFF},
    {SERDES_PHY_UNIT, 0x31C,    0x31B,  0xFFFF},
    {SERDES_PHY_UNIT, 0x214,    0x4418, 0xFFFF},
    {SERDES_PHY_UNIT, 0x8C,     0x7A,   0xFFFF},
    {SERDES_PHY_UNIT, 0x20C,    0x300,  0xFFFF},
    {SERDES_PHY_UNIT, 0x360,    0x5903, 0xFFFF},
    {SERDES_PHY_UNIT, 0x360,    0x1903, 0xFFFF},
    {SERDES_PHY_UNIT, 0x184,    0x1013, 0xFFFF},
    {SERDES_PHY_UNIT, 0x198,    0x1000, 0xFFFF},
    {SERDES_PHY_UNIT, 0x184,    0x1073, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x3A62, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x3A62, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6047, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6048, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x3A82, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x5093, 0xFFFF},
    {SERDES_PHY_UNIT, 0x278,    0x79F3, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1AC,    0x45D8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1AC,    0x45F8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xB656, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x5096, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xB796, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x5196, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xBD96, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xBDB6, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x51B6, 0xFFFF},
    {SERDES_PHY_UNIT, 0x29C,    0x3FF,  0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x16D8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x264,    0xBAD,  0xFFFF},
    {SERDES_PHY_UNIT, 0x284,    0x840,  0xC87C},
    {SERDES_PHY_UNIT, 0x32C,    0x4400, 0xFFFF},
    {SERDES_PHY_UNIT, 0x168,    0x5028, 0xFFFF},
    {SERDES_PHY_UNIT, 0x188,    0x2A60, 0xFFFF}
};

/* SERDES_TX_TRAINING_DISABLE_SEQ Sequence init */
static MV_OP_PARAMS serdesTxTrainingDisParams[] =
{
    {SERDES_UNIT,       0x8,    0x0,    0x20},
    {SERDES_UNIT,       0xC,    0x0,    0x100},
    {SERDES_PHY_UNIT,   0x22C,  0x7470, 0xFFFF},
    {SERDES_PHY_UNIT,   0x94,   0x1FFF, 0xFFFF},
    {SERDES_PHY_UNIT,   0x8C,   0x72,   0xFFFF},
    {SERDES_PHY_UNIT,   0x31C,  0,      0x200}
};

/* SERDES_RX_TRAINING_DISABLE_SEQ Sequence init */
static MV_OP_PARAMS serdesRxTrainingDisParams[] =
{
    {SERDES_PHY_UNIT, 0x54,     0x0,    0xFFFF},
    {SERDES_PHY_UNIT, 0x6C,     0x0,    0xFFFF},
    {SERDES_UNIT,     0x8,      0x0,    0x20},
    {SERDES_UNIT,     0xC,      0x0,    0x80},
    {SERDES_PHY_UNIT, 0x22C,    0x7470, 0xFFFF}
};

/* SERDES_RX_TRAINING_ENABLE_SEQ Sequence init */
static MV_OP_PARAMS serdesRxTrainingEnParams[] =
{
    {SERDES_PHY_UNIT, 0x188,    0x2A60, 0xFFFF},
    {SERDES_PHY_UNIT, 0x94,     0x1FFF, 0xFFFF},
    {SERDES_PHY_UNIT, 0x18,     0x468F, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6047, 0xFFFF},
    {SERDES_PHY_UNIT, 0x20,     0xAFC0, 0xFFFF},
    {SERDES_PHY_UNIT, 0x40,     0xFC0,  0xFFFF},
    {SERDES_PHY_UNIT, 0x48,     0x1788, 0xFFFF},
    {SERDES_PHY_UNIT, 0x50,     0x1689, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1A8,    0x4000, 0xFFFF},
    {SERDES_PHY_UNIT, 0x214,    0x2118, 0xFFFF},
    {SERDES_PHY_UNIT, 0x22C,    0x7470, 0xFFFF},
    {SERDES_PHY_UNIT, 0x330,    0xD8F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x36C,    0x3C4,  0xFFFF},
    {SERDES_PHY_UNIT, 0x280,    0xCA33, 0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x2,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x0,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x1,    0xFFFF},
    {SERDES_PHY_UNIT, 0x2BC,    0x0,    0xFFFF},
    {SERDES_PHY_UNIT, 0x278,    0x21F3, 0xFFFF},
    {SERDES_PHY_UNIT, 0x228,    0x2B40, 0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x1658, 0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x16D8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x18,     0x20F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x330,    0xD0F,  0xFFFF},
    {SERDES_PHY_UNIT, 0x20C,    0x300,  0xFFFF},
    {SERDES_PHY_UNIT, 0x184,    0x1013, 0xFFFF},
    {SERDES_PHY_UNIT, 0x198,    0x1000, 0xFFFF},
    {SERDES_PHY_UNIT, 0x184,    0x1073, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x6A62, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x6A62, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6047, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1C,     0x6048, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4648, 0xFFFF},
    {SERDES_PHY_UNIT, 0xEC,     0x6A82, 0xFFFF},
    {SERDES_PHY_UNIT, 0xF0,     0x4688, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x5093, 0xFFFF},
    {SERDES_PHY_UNIT, 0x278,    0x79F3, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1AC,    0x45D8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x1AC,    0x45F8, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xB657, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x5097, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xB7D7, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x51D7, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xBFD7, 0xFFFF},
    {SERDES_PHY_UNIT, 0x27C,    0xBFFF, 0xFFFF},
    {SERDES_PHY_UNIT, 0x340,    0x51FF, 0xFFFF},
    {SERDES_PHY_UNIT, 0x320,    0x16D4, 0xFFFF},
    {SERDES_PHY_UNIT, 0x168,    0x5028, 0xFFFF},
    {SERDES_PHY_UNIT, 0x264,    0xBAD,  0xFFFF},
    {SERDES_PHY_UNIT, 0x284,    0x4874, 0xC87C},
    {SERDES_PHY_UNIT, 0x32C,    0x4400, 0xFFFF},
    {SERDES_PHY_UNIT, 0x188,    0x2A60, 0xFFFF},
    {SERDES_PHY_UNIT, 0x94,     0x1FFF, 0xFFFF},
    {SERDES_PHY_UNIT, 0x8C,     0x72,   0xFFFF},
    {SERDES_PHY_UNIT, 0x54,     0x80E0, 0xFFFF},
    {SERDES_PHY_UNIT, 0x6C,     0xC4,   0xFFFF},
    {SERDES_PHY_UNIT, 0x22C,    0x7471, 0xFFFF},
    {SERDES_UNIT,     0xC,      0x80,   0x80  }
};

/* SERDES_CORE_RESET_SEQ Sequence init */
static MV_OP_PARAMS serdesCoreResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x0,    0x20},
};

/* SERDES_CORE_UNRESET_SEQ Sequence init */
static MV_OP_PARAMS serdesCoreUnResetParams[] =
{
    {SERDES_UNIT, SERDES_EXTERNAL_CONFIGURATION_1,  0x20,   0x20},
};

MV_CFG_SEQ hwsSerdes28nmSeqDb[MV_SERDES_LAST_SEQ] = {
    {serdesSdResetParams,      MV_SEQ_SIZE(serdesSdResetParams)      },     /* SERDES_SD_RESET_SEQ */
    {serdesSdUnResetParams,    MV_SEQ_SIZE(serdesSdUnResetParams)    },     /* SERDES_SD_UNRESET_SEQ */
    {serdesRfResetParams,      MV_SEQ_SIZE(serdesRfResetParams)      },     /* SERDES_RF_RESET_SEQ */
    {serdesRfUnResetParams,    MV_SEQ_SIZE(serdesRfUnResetParams)    },     /* SERDES_RF_UNRESET_SEQ */
    {serdesSynceResetParams,   MV_SEQ_SIZE(serdesSynceResetParams)   },     /* SERDES_SYNCE_RESET_SEQ */
    {serdesSynceUnResetParams, MV_SEQ_SIZE(serdesSynceUnResetParams) },     /* SERDES_SYNCE_UNRESET_SEQ */

    {NULL,                     0                                     },     /* SERDES_SERDES_POWER_UP_CTRL_SEQ */
    {serdesPowerDownCtrlParams,MV_SEQ_SIZE(serdesPowerDownCtrlParams)},     /* SERDES_SERDES_POWER_DOWN_CTRL_SEQ */
    {serdesRxIntUpParams,      MV_SEQ_SIZE(serdesRxIntUpParams)      },     /* SERDES_SERDES_RXINT_UP_SEQ */
    {serdesRxIntDownParams,    MV_SEQ_SIZE(serdesRxIntDownParams)    },     /* SERDES_SERDES_RXINT_DOWN_SEQ */
    {serdesWaitPllParams,      MV_SEQ_SIZE(serdesWaitPllParams)      },     /* SERDES_SERDES_WAIT_PLL_SEQ */
 
    {SerdesSpeed_1_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_1_25gSeq)     },     /* SERDES_SPEED_1_25G_SEQ               */
    {SerdesSpeed_3_125gSeq,    MV_SEQ_SIZE(SerdesSpeed_3_125gSeq)    },     /* SERDES_SPEED_3_125G_SEQ              */
    {SerdesSpeed_3_75gSeq,     MV_SEQ_SIZE(SerdesSpeed_3_75gSeq)     },     /* SERDES_SPEED_3_75G_SEQ               */
    {SerdesSpeed_4_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_4_25gSeq)     },     /* SERDES_SPEED_4_25G_SEQ               */
    {SerdesSpeed_5gSeq,        MV_SEQ_SIZE(SerdesSpeed_5gSeq)        },     /* SERDES_SPEED_5G_SEQ                  */
    {SerdesSpeed_6_25gSeq,     MV_SEQ_SIZE(SerdesSpeed_6_25gSeq)     },     /* SERDES_SPEED_6_25G_SEQ               */
    {SerdesSpeed_7_5gSeq,      MV_SEQ_SIZE(SerdesSpeed_7_5gSeq)      },     /* SERDES_SPEED_7_5G_SEQ                */
    {SerdesSpeed_10_3125gSeq,  MV_SEQ_SIZE(SerdesSpeed_10_3125gSeq)  },     /* SERDES_SPEED_10_3125G_SEQ            */

    {serdesLpbkNormalParams,   MV_SEQ_SIZE(serdesLpbkNormalParams)   },     /* SERDES_SD_LPBK_NORMAL_SEQ */
    {serdesLpbkAnaTx2RxParams, MV_SEQ_SIZE(serdesLpbkAnaTx2RxParams) },     /* SERDES_SD_ANA_TX_2_RX_SEQ */
    {serdesLpbkDigTx2RxParams, MV_SEQ_SIZE(serdesLpbkDigTx2RxParams) },     /* SERDES_SD_DIG_TX_2_RX_SEQ */
    {serdesLpbkDigRx2TxParams, MV_SEQ_SIZE(serdesLpbkDigRx2TxParams) },     /* SERDES_SD_DIG_RX_2_TX_SEQ */

    {serdesPtAfterPatternNormalParams, MV_SEQ_SIZE(serdesPtAfterPatternNormalParams)}, /* SERDES_PT_AFTER_PATTERN_NORMAL_SEQ */
    {serdesPtAfterPatternTestParams,   MV_SEQ_SIZE(serdesPtAfterPatternTestParams)  }, /* SERDES_PT_AFTER_PATTERN_TEST_SEQ */

    {serdesRxTrainingEnParams, MV_SEQ_SIZE(serdesRxTrainingEnParams) },     /* SERDES_RX_TRAINING_ENABLE_SEQ */
    {serdesRxTrainingDisParams,MV_SEQ_SIZE(serdesRxTrainingDisParams)},     /* SERDES_RX_TRAINING_DISABLE_SEQ */
    {serdesTxTrainingEnParams, MV_SEQ_SIZE(serdesTxTrainingEnParams) },     /* SERDES_TX_TRAINING_ENABLE_SEQ */
    {serdesTxTrainingDisParams,MV_SEQ_SIZE(serdesTxTrainingDisParams)},     /* SERDES_TX_TRAINING_DISABLE_SEQ */

    {SerdesSpeed_12_5gSeq,     MV_SEQ_SIZE(SerdesSpeed_12_5gSeq)     },     /* SERDES_SPEED_12_5G_SEQ */
    {NULL,                     0                                     },     /* SERDES_SPEED_3_3G_SEQ */

    {SerdesSpeed_11_5625gSeq,  MV_SEQ_SIZE(SerdesSpeed_11_5625gSeq)  },     /* SERDES_SPEED_11_5625G_SEQ */
    {NULL,                     0                                     },     /* SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ */
    {NULL,                     0                                     },     /* SERDES_SERDES_PARTIAL_POWER_UP_SEQ */

    {SerdesSpeed_11_25gSeq,    MV_SEQ_SIZE(SerdesSpeed_11_25gSeq)    },     /* SERDES_SPEED_11_25G_SEQ */
    {serdesCoreResetParams,    MV_SEQ_SIZE(serdesCoreResetParams)    },     /* SERDES_CORE_RESET_SEQ */
    {serdesCoreUnResetParams,  MV_SEQ_SIZE(serdesCoreUnResetParams)  },     /* SERDES_CORE_UNRESET_SEQ */

    {NULL,                     0                                     },     /* SERDES_FFE_TABLE_LR_SEQ */
    {NULL,                     0                                     },     /* SERDES_FFE_TABLE_SR_SEQ */
    {NULL,                     0                                     },     /* SERDES_SPEED_10_9375G_SEQ */
    {SerdesSpeed_12_1875gSeq,  MV_SEQ_SIZE(SerdesSpeed_12_1875gSeq)  },     /* SERDES_SPEED_12_1875G_SEQ */
    {NULL,                     0                                     },     /* SERDES_SPEED_5_625G_SEQ */
    {NULL,                     0                                     }      /* SERDES_SPEED_5_15625G_SEQ */
};

GT_STATUS hwsComPhyH28nmSeqInit(void)
{
    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    mvCfgSerdesSeqExecInit(hwsSerdes28nmSeqDb, (sizeof(hwsSerdes28nmSeqDb) / sizeof(MV_CFG_SEQ)));

    /* this global parameter is used for sequence set/get */
    hwsSerdesSeqDbPtr = hwsSerdes28nmSeqDb;

    return GT_OK;
}


#else
/* on CO CPU code */
GT_STATUS hwsComPhyH28nmSeqInit(void)
{
    hwsGenSeqInit(hwsSerdes28nmSeqDb, MV_SERDES_LAST_SEQ);
    return GT_OK;
}

#endif /* CO_CPU_RUN */

#ifdef CO_CPU_RUN
void coCpuSeqRead(GT_U8 devNum, MV_HWS_COM_PHY_H_SUB_SEQ seqId, GT_U32 lineNum, MV_EL_DB_OPERATION *oper, MV_OP_PARAMS *params)
{
    MV_CFG_SEQ *seq;
    GT_U32 nextOperAddr, data;
    GT_U8 i;

    seq = &hwsSerdes28nmSeqDb[seqId];
    nextOperAddr = (GT_U32)(seq->cfgSeq);

    for (i = 0; i < lineNum; i++)
    {
        /* find an operation N lineNum */
        mvApGetSharedMem(devNum, devNum,nextOperAddr + 4, &data);
        nextOperAddr = data & 0xFFFF;
    }
    /* read operaion line from GP_SRAM; */
    coCpuSeqReadOperation(devNum,nextOperAddr,oper,params);

    return;
}
#endif


GT_STATUS hwsComPhy28nmSeqGet(MV_HWS_COM_PHY_H_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
    if (seqLine == NULL)
    {
    return GT_BAD_PTR;
    }
    if (seqType >= MV_SERDES_LAST_SEQ)
    {
    return GT_NO_SUCH;
    }

    if (hwsSerdesSeqDbPtr[seqType].cfgSeqSize <= lineNum)
    {
    return GT_NO_MORE;
    }
#ifdef CO_CPU_RUN
    {
        MV_EL_DB_OPERATION oper;

        hwsOsMemSetFuncPtr(&curSerdesOpParams, 0, sizeof(curSerdesOpParams));

        coCpuSeqRead(0, seqType, lineNum, &oper, &curSerdesOpParams);
        seqLine->op = oper;
        seqLine->params = &curSerdesOpParams;
    }
#else
    {
      *seqLine = hwsSerdesSeqDbPtr[seqType].cfgSeq[lineNum];
    }
#endif
    return GT_OK;
}


#ifdef CO_CPU_RUN
GT_STATUS hwsSeqIntCpuInit(void)
{
    GT_U8 i;

    GT_U32 data;

    GT_U32 serdesSeqPtr = CO_CPU_SERDES_SEQ_PTR;

    /* copy all relevant sequences to shared memory from hwsSerdes28nmSeqDb[MV_SERDES_LAST_SEQ] */
    for (i = 0; i < MV_SERDES_LAST_SEQ; i++) {

        /* init sequence array with size and pointer*/
        mvApSetSharedMem(0, 0, serdesSeqPtr, data);
        hwsSerdes28nmSeqDb[i].cfgSeqSize = data & 0xFFFF;
        hwsSerdes28nmSeqDb[i].cfgSeq = (MV_OP_PARAMS  *)(data >> 16);
        serdesSeqPtr += 4;
    }

    return GT_OK;
}
#endif

