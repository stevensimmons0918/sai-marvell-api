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
* @file prvCpssDxChHwRegAddrFalcon.c
*
* @brief This file implement DB of units base addresses for Falcon.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwFalconInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define FALCON_PIPE_MEM_SIZE         0x08000000
#define FALCON_PIPE_0_START_ADDR     0x08000000/*start of pipe 0*/
#define FALCON_PIPE_0_END_ADDR       (FALCON_PIPE_0_START_ADDR + FALCON_PIPE_MEM_SIZE - 4)/* end of pipe 0 */
#define FALCON_PIPE_1_START_ADDR     0x10000000/*start of pipe 1*/
#define FALCON_PIPE_1_END_ADDR       (FALCON_PIPE_1_START_ADDR + FALCON_PIPE_MEM_SIZE - 4)/* end of pipe 1 */

/* offset between tile 0 to tile 1 to tile 2 to tile 3 */
#define FALCON_NUM_PIPES_PER_TILE       2

#define FALCON_PIPE_1_FROM_PIPE_0_OFFSET                            (FALCON_PIPE_1_START_ADDR - FALCON_PIPE_0_START_ADDR)
#define FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET       0x00400000

#define FALCON_D2D_CP_TILE_OFFSET             FALCON_TILE_OFFSET_CNS /*was 0x00080000 in Cider 171111*/
#define FALCON_D2D_CP_BASE_ADDR               0x002E0000             /*was 0x02800000 in Cider 171111*/
#define FALCON_D2D_CP_SIZE                    64                     /* was 512 (K)in Cider 171111 */

/* There are up to 16 LED servers/units in Falcon and only one LED interface */
#define PRV_CPSS_DXCH_FALCON_LED_UNIT_NUM_CNS                         16
/* There are up to 32 LED ports per one LED server.
   The only 17 LED ports (16 network ports + 1 CPU) are in use */
#define PRV_CPSS_DXCH_FALCON_LED_UNIT_PORTS_NUM_CNS                   32

/* encapsulate the pipeId and the unitType in the GT_U32     field */
/* pipeId must be >= 1 , unitType is of type PRV_CPSS_DXCH_UNIT_ENT */
#define FALCON_PIPE_OFFSET(pipeId,unitType) ((BIT_0 << ((pipeId) - 1)) + ((unitType)<<(pipeId)))

#define MIRRORED_TILES_BMP_CNS (BIT_1 | BIT_3)/* in Falcon tile 1,3 are mirror image of tile 0 */


#define UNIT___NOT_VALID PRV_CPSS_DXCH_UNIT___NOT_VALID___E

typedef enum{
    UNIT_DUPLICATION_TYPE_PER_PIPE_E,
    UNIT_DUPLICATION_TYPE_PER_TILE_E,
    UNIT_DUPLICATION_TYPE_PER_2_TILES_E,

    UNIT_DUPLICATION_TYPE___NOT_VALID__E

}UNIT_DUPLICATION_TYPE_ENT;

typedef struct{
    UNIT_DUPLICATION_TYPE_ENT   duplicationType;
    PRV_CPSS_DXCH_UNIT_ENT pipe0Id;
    GT_U32                 pipe0baseAddr;
    GT_U32                 sizeInBytes;
    PRV_CPSS_DXCH_UNIT_ENT nextPipeId[MAX_PIPES_CNS];/*must be terminated by UNIT___NOT_VALID or PRV_CPSS_DXCH_UNIT___NOT_VALID___E*/
}UNIT_IDS_IN_PIPES_STC;

#define MG_SIZE (1   * _1M)
#define NUM_MG_PER_CNM   8
/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/Projects/EAGLE/
Shared%20Documents/Design/AXI/Address%20Space/Eagle_Address_Space_falcon_171111.xlsm
*/
static const UNIT_IDS_IN_PIPES_STC falconUnitsIdsInPipes[] =
{
    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
    /* pipe0Id */                     /*pipe0baseAddr*/  /*sizeInBytes*/ /*pipe 1*/                     /* termination*/
     {UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LPM_E           ,0x08000000,     16  * _1M,{PRV_CPSS_DXCH_UNIT_LPM_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPVX_E          ,0x09000000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_IPVX_1_E      ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TCAM_E                       ,0x18000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_CNM_AAC_E    ,0x1C130000,     64  * _1K,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_EM_E                         ,0x188A0000,     64  * _1K ,{UNIT___NOT_VALID}    }/*EM unit*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_FDB_E                        ,0x188B0000,     64  * _1K ,{UNIT___NOT_VALID}    }/*MT unit*/

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_L2I_E           ,0x09800000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_L2I_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_CNC_0_E         ,0x0A000000,     256 * _1K,{PRV_CPSS_DXCH_UNIT_CNC_0_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_CNC_1_E         ,0x0A040000,     256 * _1K,{PRV_CPSS_DXCH_UNIT_CNC_1_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PCL_E           ,0x0A080000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_IPCL_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EPLR_E          ,0x0A100000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EPLR_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EOAM_E          ,0x0A200000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EOAM_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EPCL_E          ,0x0A300000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_EPCL_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PREQ_E          ,0x0A380000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_PREQ_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_ERMRK_E         ,0x0A400000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_ERMRK_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IOAM_E          ,0x0AB00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IOAM_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MLL_E           ,0x0AC00000,     2   * _1M,{PRV_CPSS_DXCH_UNIT_MLL_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPLR_E          ,0x0AE00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IPLR_0_1_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,0x0AF00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IPLR_1_1_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EQ_E            ,0x0B000000,     16  * _1M,{PRV_CPSS_DXCH_UNIT_EQ_1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,0x0C900000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EGF_QAG_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,0x0CA00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EGF_SHT_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,0x0CB00000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EGF_EFT_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TTI_E           ,0x0CC00000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_TTI_1_E       ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EREP_E          ,0x0D3F0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EREP_1_E      ,    UNIT___NOT_VALID}    }/* moved from 0x0C800000 */
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_BMA_E           ,0x0D400000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_BMA_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_HBU_E           ,0x0D500000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_HBU_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IA_E            ,0x0D510000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_IA_1_E        ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA_E         ,0x0D520000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA4_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA1_E        ,0x0D530000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA5_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA2_E        ,0x0D540000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA6_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA3_E        ,0x0D550000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA7_E      ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA_E         ,0x0D560000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA4_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,0x0D570000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO4_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA1_E        ,0x0D580000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA5_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E      ,0x0D590000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO5_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA2_E        ,0x0D5a0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA6_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E      ,0x0D5b0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO6_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA3_E        ,0x0D5c0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA7_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E      ,0x0D5d0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO7_E    ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,0x0D600000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E,0x0D620000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,0x0D640000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E,0x0D680000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS1_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,0x0D6c0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E,0x0D6d0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC1_E,UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E,0x0D700000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E,0x0D720000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E,0x0D740000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E,0x0D780000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS3_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E,0x0D7c0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E,0x0D7d0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC3_E,UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E   ,0x0D7F0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_4_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E   ,0x0D800000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_5_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E   ,0x0D810000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_6_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E   ,0x0D820000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_7_E   ,  UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_HA_E            ,0x0E000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_HA_1_E          ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PHA_E           ,0x0C000000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_PHA_1_E         ,  UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E       ,0x0A800000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI0_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E       ,0x0A810000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI1_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E   ,0x0D6E0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI0_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E   ,0x0D7E0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI1_E,  UNIT___NOT_VALID}  }

    /*********************************/
    /* per tile units (every 2 pipes)*/
    /*********************************/

    /* pipe0Id */                     /* pipe0baseAddr */  /*sizeInBytes*/ /* termination*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E               ,0x00000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E               ,0x01000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E               ,0x02000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E               ,0x03000000,     8   * _1M ,{UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI0_E               ,0x00500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI1_E               ,0x00501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI0_E               ,0x01500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI1_E               ,0x01501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI0_E               ,0x02500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI1_E               ,0x02501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI0_E               ,0x03500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI1_E               ,0x03501000,     4   * _1K ,{UNIT___NOT_VALID}    }


    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,0x18840000,    256  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E              ,0x18800000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E              ,0x18810000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E              ,0x18880000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_SHM_E                        ,0x18890000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,0x18900000,     1   * _1M ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x19000000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,0x19030000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x19040000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,0x19060000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E     ,0x19070000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x19080000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E   ,0x19090000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E  ,0x190A0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E     ,0x190C0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E     ,0x190D0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E   ,0x190E0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E   ,0x190F0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,0x19100000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E     ,0x19110000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E     ,0x19120000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x19130000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E     ,0x19140000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E     ,0x19150000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E     ,0x19160000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E     ,0x19170000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E     ,0x19180000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/

    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_DFX_SERVER_E                 ,PRV_CPSS_FALCON_DFX_BASE_ADDRESS_CNS,      1  * _1M ,{UNIT___NOT_VALID}    } /* in TILE0_CENTRAL*/
    /* TAI 0,1 are units per tile */
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TAI_E                        ,0x1BF00000,     64  * _1K,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TAI1_E                       ,0x1BF80000,     64  * _1K,{UNIT___NOT_VALID}    }

    /************************************************/
    /* per 2 tiles units (every 4 pipes)            */
    /* tile 0 uses MG_0_0 to MG_0_3 (4 MG units)    */
    /* tile 1 uses MG_1_0 to MG_1_3 (4 MG units)    */
    /************************************************/
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_E            ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*0,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_1_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*1,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_2_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*2,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_3_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*3,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_0_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*4,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_1_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*5,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_2_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*6,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_3_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*7,     MG_SIZE,{UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_CNM_RFU_E    ,0x1C100000,     64  * _1K,{UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_CNM_RUNIT_E  ,0x1e000000,     64  * _1K,{UNIT___NOT_VALID}    }
    /* SMI is in the CnM section */
    /* NOTE : SMI 2,3 are in 12.8T device only (in the second CNM unit) */
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_SMI_0_E      ,0x1C110000,     64  * _1K,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_SMI_1_E      ,0x1C120000,     64  * _1K,{UNIT___NOT_VALID}    }


    /*Eagle D2D-CP internal        524032        B                00280000        002FFEFF*/
    /*Eagle D2D-CP AMB internal        256       B                002FFF00        002FFFFF*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E    ,FALCON_D2D_CP_BASE_ADDR                          ,    FALCON_D2D_CP_SIZE  * _1K ,{UNIT___NOT_VALID}    }


#if 0
        /* the 'GOP' base address also used by : */
        GOP_BASE_ADDR_CNS
        /* at file cpssDriver\pp\interrupts\dxExMx\cpssDrvPpIntDefDxChFalcon.c */
#endif
#ifdef GM_USED
    /*************************************************************/
    /* GM Simulation use BC3 definitions for GOP/MIB/SERDES      */
    /*   per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_GOP_E           ,0x04000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_GOP1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MIB_E           ,0x04800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_MIB1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_SERDES_E        ,0x05000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_SERDES_1_E    ,    UNIT___NOT_VALID}    }
#endif
     /* must be last */
    ,{UNIT_DUPLICATION_TYPE___NOT_VALID__E, PRV_CPSS_DXCH_UNIT_LAST_E, 0,0,{UNIT___NOT_VALID}}
};

/**
* @internal falconOffsetFromPipe0Get function
* @endinternal
*
* @brief   Falcon : (for pipeId != 0).Get The offset (in bytes) between the address
*         of register in pipe 0 of this unit to the register in the other pipe.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] pipeId                   - The PipeId. (in case of unit per tile --> use  = 2tileId)
*                                      NOTE: no meaning in calling the function with 'pipeId = 0'
* @param[in] pipe0Id                  - The unit name in Pipe0
*                                       number : The offset (in bytes) between the address of register in pipe 0 of this unit
*                                       to the register in the other pipe.
*/
 GT_U32   falconOffsetFromPipe0Get(
    IN GT_U32                   pipeId,
    IN PRV_CPSS_DXCH_UNIT_ENT   pipe0Id
)
{
    GT_U32  tileId       = pipeId / FALCON_NUM_PIPES_PER_TILE;
    GT_U32  tileOffset   = tileId * FALCON_TILE_OFFSET_CNS;
    GT_U32  pipeIdFactor = pipeId % FALCON_NUM_PIPES_PER_TILE;/* pipe 0,2,4,6 not need extra offset other than 'tile offset' */
    GT_U32  pipeOffset;
    GT_U32  instanceId;

    if(pipeId == 0)
    {
        /* 'no offset' from pipe 0 */
        return 0;
    }

    switch(pipe0Id)
    {
        case  PRV_CPSS_DXCH_UNIT_TTI_E              :
        case  PRV_CPSS_DXCH_UNIT_PCL_E              :
        case  PRV_CPSS_DXCH_UNIT_L2I_E              :
        case  PRV_CPSS_DXCH_UNIT_IPVX_E             :
        case  PRV_CPSS_DXCH_UNIT_IPLR_E             :
        case  PRV_CPSS_DXCH_UNIT_IPLR_1_E           :
        case  PRV_CPSS_DXCH_UNIT_IOAM_E             :
        case  PRV_CPSS_DXCH_UNIT_MLL_E              :
        case  PRV_CPSS_DXCH_UNIT_EQ_E               :
        case  PRV_CPSS_DXCH_UNIT_EGF_EFT_E          :
        case  PRV_CPSS_DXCH_UNIT_CNC_0_E            :
        case  PRV_CPSS_DXCH_UNIT_CNC_1_E            :
        case  PRV_CPSS_DXCH_UNIT_EGF_SHT_E          :
        case  PRV_CPSS_DXCH_UNIT_HA_E               :
        case  PRV_CPSS_DXCH_UNIT_PHA_E              :
        case  PRV_CPSS_DXCH_UNIT_ERMRK_E            :
        case  PRV_CPSS_DXCH_UNIT_EPCL_E             :
        case  PRV_CPSS_DXCH_UNIT_EPLR_E             :
        case  PRV_CPSS_DXCH_UNIT_EOAM_E             :
        case  PRV_CPSS_DXCH_UNIT_LPM_E              :
        case  PRV_CPSS_DXCH_UNIT_IA_E               :
        case  PRV_CPSS_DXCH_UNIT_EREP_E             :
        case  PRV_CPSS_DXCH_UNIT_PREQ_E             :
        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
        case  PRV_CPSS_DXCH_UNIT_RXDMA1_E           :
        case  PRV_CPSS_DXCH_UNIT_RXDMA2_E           :
        case  PRV_CPSS_DXCH_UNIT_RXDMA3_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA_E            :
        case  PRV_CPSS_DXCH_UNIT_TXDMA1_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA2_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA3_E           :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO_E          :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO1_E         :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO2_E         :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO3_E         :
        case  PRV_CPSS_DXCH_UNIT_BMA_E              :
        case  PRV_CPSS_DXCH_UNIT_EGF_QAG_E          :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E    :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E       :
        case PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E :
        case PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E :
        case PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E:
        case PRV_CPSS_DXCH_UNIT_HBU_E:
            pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
            break;

        /* we are emulating those addresses */
        case  PRV_CPSS_DXCH_UNIT_MIB_E              :
        case  PRV_CPSS_DXCH_UNIT_GOP_E              :
        case  PRV_CPSS_DXCH_UNIT_SERDES_E           :
        case  PRV_CPSS_DXCH_UNIT_LED_0_E            :
        case  PRV_CPSS_DXCH_UNIT_LED_1_E            :
            pipeOffset = FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET;
            break;

        /***********************/
        /* units per '2 tiles' */
        /***********************/
        case  PRV_CPSS_DXCH_UNIT_MG_E              :
        case  PRV_CPSS_DXCH_UNIT_SMI_0_E           :
        case  PRV_CPSS_DXCH_UNIT_SMI_1_E           :
            instanceId   = pipeId / (FALCON_NUM_PIPES_PER_TILE  * 2);
            tileOffset   = instanceId * FALCON_TILE_OFFSET_CNS;
            pipeIdFactor = 0;
            pipeOffset   = 0;
            break;

        /********************/
        /* units per 'tile' */
        /********************/
        default:
            /* all those considered to be in PIPE 0 (all those of shared in tile 0 too) !!! */
            /* so offset is '0' from pipe 0 */
            pipeOffset = 0;
            break;
    }

    if((1 << tileId) & MIRRORED_TILES_BMP_CNS)
    {
        /* handle swap of 'per pipe' in the mirrored tiles */
        pipeIdFactor = 1 - pipeIdFactor;
    }

    return tileOffset + (pipeOffset * pipeIdFactor);
}
/*
    structure to help convert Global to Local for DMA and for GOP ports
*/
typedef struct{
    GT_U32    dpIndex;
    GT_U32    localPortInDp;
    GT_U32    pipeIndex;
    GT_U32    localPortInPipe;
}SPECIAL_DMA_PORTS_STC;

#define LOCAL_GOP_CPU_NETWORK_PORT    32
#ifndef GM_USED
    /*The DP that hold MG0*/
    #define MG0_SERVED_DP   4      /* PRV_CPSS_DXCH_UNIT_MG_0_0_E */
    /*The DP that hold MG1*/
    #define MG1_SERVED_DP   5      /* PRV_CPSS_DXCH_UNIT_MG_0_1_E */
    /*The DP that hold MG2*/
    #define MG2_SERVED_DP   6      /* PRV_CPSS_DXCH_UNIT_MG_0_2_E */
    /*The DP that hold MG3*/
    #define MG3_SERVED_DP   7      /* PRV_CPSS_DXCH_UNIT_MG_0_3_E */
#else  /*GM_USED ... using single DP[0] (in pipe[0]) */
    #define MG0_SERVED_DP   0
    #define MG1_SERVED_DP   GT_NA
    #define MG2_SERVED_DP   GT_NA
    #define MG3_SERVED_DP   GT_NA
#endif /*GM_USED*/

/*The DP that hold MG4  */
#define MG4_SERVED_DP    11      /* local DP[4] in mirrored tile 1 */  /* PRV_CPSS_DXCH_UNIT_MG_1_0_E */
/*The DP that hold MG5*/
#define MG5_SERVED_DP    10      /* local DP[5] in mirrored tile 1 */  /* PRV_CPSS_DXCH_UNIT_MG_1_1_E */
/*The DP that hold MG6*/
#define MG6_SERVED_DP    9       /* local DP[6] in mirrored tile 1 */  /* PRV_CPSS_DXCH_UNIT_MG_1_2_E */
/*The DP that hold MG7*/
#define MG7_SERVED_DP    8       /* local DP[7] in mirrored tile 1 */  /* PRV_CPSS_DXCH_UNIT_MG_1_3_E */

/*The DP that hold MG8*/
#define MG8_SERVED_DP    (MG0_SERVED_DP + 16)
/*The DP that hold MG9*/
#define MG9_SERVED_DP    (MG1_SERVED_DP + 16)
/*The DP that hold MG10*/
#define MG10_SERVED_DP   (MG2_SERVED_DP + 16)
/*The DP that hold MG11*/
#define MG11_SERVED_DP   (MG3_SERVED_DP + 16)

/*The DP that hold MG12*/
#define MG12_SERVED_DP   (MG4_SERVED_DP + 16)
/*The DP that hold MG13*/
#define MG13_SERVED_DP   (MG5_SERVED_DP + 16)
/*The DP that hold MG14*/
#define MG14_SERVED_DP   (MG6_SERVED_DP + 16)
/*The DP that hold MG15*/
#define MG15_SERVED_DP   (MG7_SERVED_DP + 16)


static const SPECIAL_DMA_PORTS_STC falcon_3_2_SpecialDma_ports[]= {
#ifndef GM_USED
   /*64*/{0,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*65*/{2,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*66*/{4,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*67*/{6,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/

   /*68*/{MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*69*/{MG1_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*70*/{MG2_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/}, /*CPU SDMA MG 2*/
   /*71*/{MG3_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/}  /*CPU SDMA MG 3*/
#else  /*GM_USED ... using single DP[0] (in pipe[0]) */
   /* 8*/{MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 0*/
#endif /*GM_USED*/
   };
static const GT_U32 num_ports_falcon_3_2_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_3_2_SpecialDma_ports);

static const SPECIAL_DMA_PORTS_STC falcon_6_4_SpecialDma_ports[]= {
   /*128*/{ 0,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*129*/{ 2,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*130*/{ 4,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*131*/{ 6,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*132*/{ 8,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*133*/{10,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*134*/{12,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,3/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*135*/{14,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,3/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/

   /*136*/{ MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*137*/{ MG1_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*138*/{ MG2_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 2*/
   /*139*/{ MG3_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 3*/

   /*140*/{ MG7_SERVED_DP/* 8*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 7*/
   /*141*/{ MG6_SERVED_DP/* 9*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 6*/
   /*142*/{ MG5_SERVED_DP/*10*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 5*/
   /*143*/{ MG4_SERVED_DP/*11*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 4*/

   };
static const GT_U32 num_ports_falcon_6_4_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_6_4_SpecialDma_ports);

static const SPECIAL_DMA_PORTS_STC falcon_12_8_SpecialDma_ports[]= {
   /*256*/{ 0,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*257*/{ 2,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*258*/{ 4,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*259*/{ 6,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*260*/{ 8,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*261*/{10,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*262*/{12,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,3/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*263*/{14,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,3/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/

   /*264*/{16,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,4/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*265*/{18,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,4/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*266*/{20,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*267*/{22,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*268*/{24,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*269*/{26,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/
   /*270*/{28,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,7/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+0/*localPortInPipe*/},/*CPU network port*/
   /*271*/{30,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,7/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT+1/*localPortInPipe*/},/*CPU network port*/

   /*272*/{ MG0_SERVED_DP/* 4*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*273*/{ MG1_SERVED_DP/* 5*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*274*/{ MG2_SERVED_DP/* 6*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 2*/
   /*275*/{ MG3_SERVED_DP/* 7*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 3*/

   /*276*/{ MG7_SERVED_DP/* 8*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 7*/
   /*277*/{ MG6_SERVED_DP/* 9*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 6*/
   /*278*/{ MG5_SERVED_DP/*10*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 5*/
   /*279*/{ MG4_SERVED_DP/*11*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 6*/

   /*280*/{ MG8_SERVED_DP /*20*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 8 */
   /*281*/{ MG9_SERVED_DP /*21*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 9 */
   /*282*/{ MG10_SERVED_DP/*22*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 10*/
   /*283*/{ MG11_SERVED_DP/*23*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 11*/

   /*284*/{ MG15_SERVED_DP/*24*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 15*/
   /*285*/{ MG14_SERVED_DP/*25*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 14*/
   /*286*/{ MG13_SERVED_DP/*26*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 13*/
   /*287*/{ MG12_SERVED_DP/*27*/,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 12*/


   };
static const GT_U32 num_ports_falcon_12_8_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_12_8_SpecialDma_ports);

#define GOP_OFFSET_BETWEEN_PORTS        0x1000
#define SERDES_OFFSET_BETWEEN_PORTS     0x1000
#define MIB_OFFSET_BETWEEN_PORTS         0x400

/**
* @internal falconGopRegAddrCalc function
* @endinternal
*
* @brief   Calculate GOP/MIB/SERDES base address in Falcon
*
* @param[in] devNum                   - device Id
* @param[in] macNum                   - global GOP/MIB/SERDES number
* @param[in] unitId                   - the unit id (GOP/MIB/SERDES)
* @param[in] perPortOffset            - offset in bytes between ports
*                                       base address of the SERDES
*/
static GT_U32 falconGopRegAddrCalc
(
    IN GT_U8     devNum,
    IN GT_U32    macNum,
    IN PRV_CPSS_DXCH_UNIT_ENT     unitId,
    IN GT_U32    perPortOffset
)
{
    GT_STATUS   rc;
    GT_U32 pipeIndex;
    GT_U32 localMacPortNum;
    GT_U32 baseAddr,pipeOffset;

    /* convert the global MAC to pipeId and local MAC */
    rc = prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
        macNum,&pipeIndex,&localMacPortNum);
    if(rc != GT_OK)
    {
        /* not valid address */
        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);
    pipeOffset = falconOffsetFromPipe0Get(pipeIndex,unitId);

    return baseAddr + (perPortOffset * localMacPortNum) + pipeOffset;
}

/**
* @internal prvCpssDxChFalconPortMacDeviceMapGet function
* @endinternal
*
* @brief   Returns tile and raven number mapped to current port MAC
*
* @param[in] devNum                   - device Id
* @param[in] portMacNum               - global MAC number
*
* @param[out] ravenNumPtr             - (pointer to) raven number
* @param[out] localRavenPtr           - (pointer to) local raven in tile
* @param[out] tileNumPtr              - (pointer to) tile number
*
* */
GT_VOID prvCpssDxChFalconPortMacDeviceMapGet
(
    IN GT_U8    devNum,
    IN GT_U32   portMacNum,
    OUT GT_U32  *ravenNumPtr,
    OUT GT_U32  *localRavenPtr,
    OUT GT_U32  *tileNumPtr
)
{
    GT_U32 ravenNum;
    GT_U32 localRaven;
    GT_U32 tileNum;
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);

    if (portMacType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        ravenNum = portMacNum - PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
    }
    else
    {
        ravenNum = portMacNum  / 16;
    }

    localRaven = ravenNum % 4;
    tileNum = ravenNum / 4;

    if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        localRaven = 3 - localRaven;
    }

    /* Tile and Raven number mapped to current port MAC */
    *ravenNumPtr    = ravenNum;
    *localRavenPtr  = localRaven;
    *tileNumPtr     = tileNum;

    return;
}

/**
* @internal prvCpssFalconUnitIdSizeInByteGet function
* @endinternal
*
* @brief   To get the size of the unit in bytes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] prvUnitId                - Id of DxCh unit(one of the PRV_CPSS_DXCH_UNIT_ENT)
*
* @param[out] unitIdSize               - size of the unit in bytes
*/
static GT_STATUS prvCpssFalconUnitIdSizeInByteGet
(
    IN  GT_U8                      devNum,
    IN  PRV_CPSS_DXCH_UNIT_ENT     prvUnitId,
    OUT GT_U32                    *unitIdSizePtr
)
{
    const UNIT_IDS_IN_PIPES_STC   *unitIdInPipePtr = &falconUnitsIdsInPipes[0];
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

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "unit [%d] not supported in Falcon",
        prvUnitId);
}

/**
* @internal mpfsRegisterIndexCalc function
* @endinternal
*
* @brief   Calculate MPFS register index in Falcon
*
* @param[in] devNum                   - device Id
* @param[in] portMacNum               - global MAC number
*/
static GT_U32 mpfsRegisterIndexCalc
(
    IN GT_U8    devNum,
    IN GT_U32   portMacNum
)
{
    GT_U32 mpfMum;
    GT_U32 mpfsMum;
    GT_U32 ravenNum;
    GT_U32 localRaven;
    GT_U32 tileNum;
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    prvCpssDxChFalconPortMacDeviceMapGet(devNum, portMacNum, &ravenNum, &localRaven, &tileNum);

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);
    if (portMacType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        return PRV_DXCH_FALCON_MPFS_CPU_MAC(tileNum, localRaven);
    }

    mpfMum  = (portMacNum % 16) / 8;
    mpfsMum = (portMacNum % 8);
    if (portMacType == PRV_CPSS_PORT_MTI_400_E)
    {
        mpfsMum = (mpfsMum == 0) ? 8 : 9;
    }

    return PRV_DXCH_FALCON_MPFS_NUM_MAC(tileNum, localRaven, mpfMum, mpfsMum);
}

/**
* @internal falconUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS to call to cpss to use for Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] unitId                   - the unit : MIB/SERDES/GOP
* @param[in] portNum                  - the global port num (MAC number)
*                                       The address of the port in the unit
*/
static GT_U32 /*GT_UREG_DATA*/   falconUnitBaseAddrCalc(
    GT_U8                                   devNum,
    GT_U32/*HWS_UNIT_BASE_ADDR_TYPE_ENT*/   unitId,
    GT_U32/*GT_UOPT*/                       portNum
)
{
    GT_STATUS   rc;
    GT_U32 localD2d, localRaven, tileNum, tileOffset, baseAddr;
    GT_U32 relativeAddr, localPortIndex, ravenNum=0;
    GT_U32 dpIndex,localDmaNumber,numRegularPorts;
    GT_U32 representativeIndex;

    switch(unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
            rc = DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum)(devNum,
                portNum,&dpIndex,&localDmaNumber);
            if(rc != GT_OK)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(localDmaNumber == FALCON_PORTS_PER_DP_CNS)
            {
                /* The 'CPU port' (port 16 of the Raven) */
                relativeAddr = 0x00515000;
                ravenNum = dpIndex / 2;/* 2 DP[] per Raven */
            }
            else
            {
                /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MPF>MPF/<MPF> <TSU IP> PTP IP/TSU IP Units %a %b*/
                localPortIndex = portNum  % 16;
                ravenNum = portNum  / 16;

                relativeAddr = 0x00405000 + 0x00080000 * ((localPortIndex) / 8) + 0x00004000 * (localPortIndex  % 8);
            }

            localRaven = ravenNum % 4;
            tileNum    = ravenNum / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E:  /* 0..15 */
            localRaven = portNum % 4;
            tileNum = portNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E:   /* 0..31 */
            localD2d = portNum % 8;
            tileNum = portNum  / 8;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localD2d = 7 - localD2d;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + localD2d), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E:   /* 0..31 */
            localRaven = (portNum % 8) / 2;
            tileNum = portNum  / 8;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = 0x00680000 + prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            if(portNum % 2 == 1)
            {
                baseAddr += 0x00010000; /* offset of D2D_1 relating to D2D_0 */
            }
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E:   /* 0..255 */
            if(portNum >= 256)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            if(localPortIndex < 8)
            {
                relativeAddr = 0 + localPortIndex * 0x1000;
            }
            else
            {
                relativeAddr = 0x00080000 + (localPortIndex - 8) * 0x1000;
            }
            relativeAddr += 0x00444000; /* start address of MAC100 unit */

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E:   /* 0..255 */
            if(portNum >= 256)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            switch(localPortIndex)
            {
                case 0:
                    relativeAddr = 0;
                    break;
                case 4:
                    relativeAddr = 0x00001000;
                    break;
                case 8:
                    relativeAddr = 0x00080000;
                    break;
                case 12:
                    relativeAddr = 0x00081000;
                    break;
                default:
                    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            relativeAddr += 0x00440000; /* start address of MAC400 unit */

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E:
            numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
            if((portNum < numRegularPorts) || (portNum >= (numRegularPorts + PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts)))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            localPortIndex = 16;
            ravenNum = portNum - numRegularPorts;
            localRaven = ravenNum % 4;
            tileNum    = ravenNum / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            relativeAddr = 0x0;

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_CPU_MPFS_E:
            baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + 0x00516000;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E:
            baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + 0x00518000;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E:
            baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + 0x00519000;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E:
            baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E , portNum);
            if(baseAddr ==  PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            return baseAddr  + 0x0051c000;


        case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E:
            if(portNum >= 256)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            relativeAddr = 0x00450000 + 0x80000 * (localPortIndex / 8);

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_GOP_E:
            return falconGopRegAddrCalc(devNum,
                portNum,PRV_CPSS_DXCH_UNIT_GOP_E,
                GOP_OFFSET_BETWEEN_PORTS);

        case HWS_UNIT_BASE_ADDR_TYPE_MIB_E:
            return (GT_UREG_DATA)falconGopRegAddrCalc(devNum,
                portNum,PRV_CPSS_DXCH_UNIT_MIB_E,
                MIB_OFFSET_BETWEEN_PORTS);

        case HWS_UNIT_BASE_ADDR_TYPE_SERDES_E:   /* 0..255 */

            switch (PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
                    numRegularPorts = PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev - 2;
                    if(portNum >= (numRegularPorts + PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }

                    if(portNum < numRegularPorts)
                    {
                        ravenNum = portNum  / 16;
                        if(ravenNum == 0)
                            ravenNum = 0;
                        else if(ravenNum == 1)
                            ravenNum = 2;
                        else if(ravenNum == 2)
                            ravenNum = 4;
                        else if(ravenNum == 3)
                            ravenNum = 5;
                        else if(ravenNum == 4)
                            ravenNum = 7;
                        else
                            return PRV_CPSS_SW_PTR_ENTRY_UNUSED;

                        localPortIndex = portNum  % 16;
                    }
                    else
                    {
                        if(portNum == 80)
                        {
                            ravenNum = 5;
                        }
                        if(portNum == 81)
                        {
                            ravenNum = 7;
                        }
                        localPortIndex = 16;
                    }

                    localRaven = ravenNum % 4;

                    break;

                default:

                    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
                    if(portNum >= (numRegularPorts + PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }

                    if(portNum < numRegularPorts)
                    {
                        ravenNum = portNum  / 16;
                        localPortIndex = portNum  % 16;
                    }
                    else  /* CPU ports */
                    {
                        ravenNum   = portNum - numRegularPorts;
                        localPortIndex = 16;
                    }
                    localRaven = ravenNum % 4;

                    break;
            }

            tileNum = ravenNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            relativeAddr = 0x00340000 + localPortIndex * 0x1000;
            return baseAddr + tileOffset + relativeAddr; /* Note , if the relativ address calculation changes, update hwsRavenSerdesAddrCalc :)*/

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E:
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_DFX_E:

            numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
            if(portNum >= (numRegularPorts + PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts))
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            if(portNum < numRegularPorts)
            {
                ravenNum = portNum  / 16;
                localRaven = ravenNum % 4;
                representativeIndex = (portNum  % 16)/8; /* 8 first ports use MSDB unit 0
                                                      next  8 ports use MSDB unit 1 */
            }
            else  /* CPU ports */
            {
                representativeIndex = 0;/* use MSDB unit 0 */
                ravenNum   = portNum - numRegularPorts;
                localRaven = ravenNum % 4;
            }

            tileNum = ravenNum  / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            if(unitId == HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E)
            {
                relativeAddr = 0x508000; /* start address of MSDB unit */
                relativeAddr+= 0x1000 * representativeIndex;
            }
            else /*HWS_UNIT_BASE_ADDR_TYPE_RAVEN_DFX_E*/
            {
                relativeAddr = 0x700000; /* start address of RAVEN DFX unit */
            }
            return  baseAddr + tileOffset+relativeAddr ;

        case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E:
            rc = DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum)(devNum,
                portNum,&dpIndex,&localDmaNumber);
            if(rc != GT_OK)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(localDmaNumber == FALCON_PORTS_PER_DP_CNS)
            {
                /* The 'CPU port' (port 16 of the Raven) is in the separate MTIP CPU EXT Units */
                relativeAddr = 0x0051C000;
                ravenNum = dpIndex / 2;/* 2 DP[] per Raven */
            }
            else
            {
                localPortIndex = portNum  % 16;
                ravenNum = portNum  / 16;

                relativeAddr = 0x00478000 + 0x00080000 * ((localPortIndex) / 8);
            }

            localRaven = ravenNum % 4;
            tileNum    = ravenNum / 4;

            if(tileNum >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
            {
                return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            return baseAddr + tileOffset + relativeAddr;


            case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E :
                baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E , portNum/16/*raven index*/);
                localPortIndex = portNum  % 16;
                relativeAddr  = 0x00080000 * (localPortIndex / 8);
                relativeAddr += (localPortIndex % 8) * 0x4000;
                relativeAddr += 0x406000;

                return baseAddr + relativeAddr;


            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E    :
            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E   :
            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E   :
            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E   :
            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E:
            case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E:
                baseAddr = falconUnitBaseAddrCalc( devNum , HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E , portNum/16/*raven index*/);
                localPortIndex = portNum  % 16;
                relativeAddr  = 0x00080000 * (localPortIndex / 8);

                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E)
                {
                    if(1 != (localPortIndex % 2))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }
                    relativeAddr += (localPortIndex % 8) * 0x1000;
                    relativeAddr += 0x00464000;
                }
                else
                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E)
                {
                    if(0 != (localPortIndex % 2))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }
                    relativeAddr += (localPortIndex % 8) * 0x1000;
                    relativeAddr += 0x00464000;
                }
                else
                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E)
                {
                    if(4 != (localPortIndex % 8))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }
                    relativeAddr += 0x00461000;
                }
                else
                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E)
                {
                    if(0 != (localPortIndex % 8))
                    {
                        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
                    }
                    relativeAddr += 0x00460000;
                }
                else
                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E)
                {
                    relativeAddr += 0x00474000;
                }
                else
                if(unitId == HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E)
                {
                    relativeAddr += 0x00470000;
                }


                return baseAddr + relativeAddr;

        default:
            break;
    }
    /* indicate error */
    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
}

/**
* @internal prvDxChHwRegAddrFalconDbInit function
* @endinternal
*
* @brief   init the base address manager of the Falcon device.
*         prvDxChFalconUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrFalconDbInit(GT_VOID)
{
    const UNIT_IDS_IN_PIPES_STC   *currEntryPtr = &falconUnitsIdsInPipes[0];
    GT_U32                  pipeOffset;
    GT_U32                  globalIndex = 0;/* index into prvDxChFalconUnitsIdUnitBaseAddrArr */
    GT_U32                  ii,jj,kk;
    GT_U32                  tile0_startGlobalIndex,tile0_endGlobalIndex;
    GT_U32                  errorCase = 0;

    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        tile0_startGlobalIndex = globalIndex;

        if(globalIndex >= FALCON_MAX_UNITS)
        {
            errorCase = 1;
            goto notEnoughUnits_lbl;
        }

        /* add the unit in pipe 0 */
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitId = currEntryPtr->pipe0Id;
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr;
        globalIndex++;

        /****************************/
        /* support multi-pipe units */
        /****************************/
        if(currEntryPtr->duplicationType == UNIT_DUPLICATION_TYPE_PER_PIPE_E)
        {
            /*jj is pipeId-1*/
            for(jj = 0; jj < MAX_PIPES_CNS ; jj++)
            {
                if ( currEntryPtr->nextPipeId[jj] == UNIT___NOT_VALID )
                {
                    break;
                }
                if ( globalIndex >= FALCON_MAX_UNITS )
                {
                    errorCase = 2;
                    goto notEnoughUnits_lbl;
                }

                pipeOffset = falconOffsetFromPipe0Get((jj+1),currEntryPtr->pipe0Id);
                /* add the units in other pipes */
                PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitId = currEntryPtr->nextPipeId[jj];
                PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr + pipeOffset;
                globalIndex++;
            }
        }

        tile0_endGlobalIndex = globalIndex;
        /****************************/
        /* support multi-tile units */
        /****************************/
        /* start from tile 1 , because tile 0 we already have */
        for(jj = 1 ; jj < 4/*tiles*/ ; jj++)
        {
            if(currEntryPtr->duplicationType == UNIT_DUPLICATION_TYPE_PER_2_TILES_E &&
                (jj & 1))/* odd tile */
            {
                /* unit exits in tile 0 and tile 2 */
                continue;
            }

            /* duplicate all units of 'tile 0' into those for tile 1..3 */
            for(kk = tile0_startGlobalIndex ; kk < tile0_endGlobalIndex; kk++)
            {
                if(globalIndex >= FALCON_MAX_UNITS)
                {
                    errorCase = 3;
                    goto notEnoughUnits_lbl;
                }

                PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitId       =
                    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[kk].unitId +    /* unitId in tile 0 */
                    PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS * jj; /* unitId offset from tile 0 */

                PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr =
                    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[kk].unitBaseAdrr + /* address in tile 0 */
                    FALCON_TILE_OFFSET_CNS * jj;                           /* address offset from tile 0 */


                globalIndex++;
            }
        }
    }

    /* set the 'last index as invalid' */
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;
    globalIndex++;

    if(globalIndex >= FALCON_MAX_UNITS)
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
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :FALCON_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,FALCON_MAX_UNITS , globalIndex+1);
}
/**
* @internal prvCpssDxChFalconHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
static PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChFalconHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    GT_U32 pipeId;
    GT_U32 pipeOffset;
    GT_U32 addrInPipe0; /*address in pipe 0 of the register */
    const UNIT_IDS_IN_PIPES_STC   *currEntryPtr;
    GT_U32  ii;
    GT_U32  tileId;
    PRV_CPSS_DXCH_UNIT_ENT  tileStartUnits;

    tileId = regAddr / FALCON_TILE_OFFSET_CNS;

    /******************************************/
    /* strip the tile offset from the regAddr */
    /******************************************/
    if(tileId)
    {
        if(tileId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_DXCH_UNIT_LAST_E,
                "address [0x%8.8x] is not supported by the device \n",
                regAddr);
        }

        regAddr       -= tileId*FALCON_TILE_OFFSET_CNS;
        tileStartUnits =  tileId * PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E;
    }
    else
    {
        tileStartUnits =  0;
    }

    if(regAddr >= FALCON_PIPE_0_START_ADDR && regAddr <= FALCON_PIPE_0_END_ADDR)
    {
        pipeId = 0;
        /* the address of pipe 0 hold 'clone' in pipe 1 */
        pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    if(regAddr >= FALCON_PIPE_1_START_ADDR && regAddr <= FALCON_PIPE_1_END_ADDR)
    {
        pipeId = 1;
        /* the address already in pipe 1 ... */
        pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    if((regAddr >= 0x04000000 && regAddr <= 0x07FFFFFC))
    {
        /* support dummy range of :
        dummy (expected in raven)
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_GOP_E           ,0x04000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_GOP1_E        ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MIB_E           ,0x04800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_MIB1_E        ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_SERDES_E        ,0x05000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_SERDES_1_E    ,    UNIT___NOT_VALID}    }

        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_0_E         ,0x05800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_2_E       ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_1_E         ,0x06000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_3_E       ,    UNIT___NOT_VALID}    }
        */
        if(regAddr & FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET)
        {
            pipeId = 1;
        }
        else
        {
            pipeId = 0;
        }

        pipeOffset = FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    {
        /* in shared memory (per tile) */
        pipeId = 0;
        pipeOffset = 0;
    }

    addrInPipe0 = regAddr - pipeId * pipeOffset;
    currEntryPtr = &falconUnitsIdsInPipes[0];

    /****************************************/
    /* search for addrInPipe0 in the ranges */
    /****************************************/
    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        /* add the unit in pipe 0 */
        if(addrInPipe0 >= currEntryPtr->pipe0baseAddr  &&
           addrInPipe0 <  (currEntryPtr->pipe0baseAddr + currEntryPtr->sizeInBytes))
        {
            /* found the proper range */

            if(pipeId == 0)
            {
                return currEntryPtr->pipe0Id + tileStartUnits;
            }

            return currEntryPtr->nextPipeId[pipeId-1] + tileStartUnits;
        }
    }

    /* not found ! */
    return PRV_CPSS_DXCH_UNIT_LAST_E;
}

/*check if register address is in IA unit (ingress Aggregator) and need 'port group' aware (pipes)
    GT_TRUE  -     need 'port group' aware. (pipes)
    GT_FALSE - not need 'port group' aware. (need to be duplicated to ALL port groups (pipes))
*/
/*
    the 'caller' gives address in ingressAggregator[0] and a specific portGroupId (PipeId),
    and the 'engine' will calculate the address for the actual portGroupId (PipeId).

    meaning that the addresses in the ingressAggregator[x] where x!= 0 are not used !!!
*/
static GT_BOOL  falcon_supportDuplicate_portGroup_iaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPipeAddrArr[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholds0), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholds1), 1}
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].PIPPrioThresholdsRandomization), 1}*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPPfcCounterThresholds),1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPPfcGlobalFillLevelThresholds),1}
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.GlobalTcPfcTriggerVectorsConfig),1}*/
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPTcPfcTriggerVectorsConfig[0]),1}*/
        /*,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(ingressAggregator[0].pfcTriggering.PIPTcPfcTriggerVectorsConfig[1]),1}*/

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPipeAddrArr,regAddr);
}

/* number of rx/tx DMA and txfifo 'per DP unit' */
#define FALCON_NUM_PORTS_DMA   (FALCON_PORTS_PER_DP_CNS + 1)

/*check if register address is per port in RxDMA unit */
static GT_BOOL  falcon_mustNotDuplicate_rxdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {

         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.cutThrough.channelCTConfig                ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelToLocalDevSourcePort ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.PCHConfig                   ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelGeneralConfigs       ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelCascadePort          ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg), FALCON_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x10000; /* offset between units */

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
static GT_BOOL  falcon_mustNotDuplicate_txdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.channelReset                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.interGapConfigitation        ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.rateLimiterEnable            ), FALCON_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x20000; /* offset between units */

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  falcon_mustNotDuplicate_txfifoAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile ),  FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase ),  FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.tagFIFOBase  ),  FALCON_NUM_PORTS_DMA}

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

    const GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0x20000; /* offset between units */

    /* check global */
    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/* check if register address is per tile*/
static GT_BOOL  falcon_mustDuplicate_pdxAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
    {
        static const PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
        {         /* table type */                                        /* number of entries */
            {CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,    0/*take from tabled DB*/},

            /* must be last */
            {CPSS_DXCH_TABLE_LAST_E , 0}
        };

        return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
    }









/* check if register address is per port in TTI[0] unit AND must not be duplicated to TTI[1] */
static GT_BOOL  falcon_mustNotDuplicate_ttiAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
       /* no need to duplicate those registers in DB ... the caller will always call
          specific to portGroup , and without 'multiple' port groups
          {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes) , 288}
       */
         /* must be last */
        {END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/* SHM : check if register address is not to be duplicated */
static GT_BOOL  falcon_mustNotDuplicate_shmAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC regAddrArr[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.fdb_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.em_illegal_address ), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.arp_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_aging_illegal_address), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,regAddrArr,regAddr);
}



/* the LPM table duplicated per tile */
/* the pipe 0,1 duplicated by HW (simulation share it) */
/* the pipe 2,3 duplicated by HW (simulation share it) */
/* the pipe 4,5 duplicated by HW (simulation share it) */
/* the pipe 6,7 duplicated by HW (simulation share it) */
static GT_BOOL  falcon_mustNotDuplicatePerPipe_lpmMemAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static const PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        /* the LPM table is duplicated by the HW (but not by simulation)
           within the single tile */
        /* but the SW need to duplicate the table per tile !!! */
        {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,    0/*take from tabled DB*/}

        /* must be last */
        ,{CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

/* check if register address is one of FUQ related ... need special treatment */
static GT_BOOL  falcon_fuqSupport_mgAddr(
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

/* check if register address is one of AUQ related ... need special treatment */
static GT_BOOL  falcon_auqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC auqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQBaseAddr), 1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,auqRegAddrArr,regAddr);
}

/* check if register address is one 'per MG' SDMA , and allow to convert address
   of MG 0 to address of other MG  */
static GT_BOOL  falcon_convertPerMg_mgSdmaAddr(
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
static GT_BOOL  falcon_allowDuplicatePerMg_mgSdmaAddr(
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

/* check if register address is one MG 'per tile' that need duplication  ... need special treatment */
static GT_BOOL  falcon_mustDuplicatePerTile_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static const PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIManagement    ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIAddress       ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIConfiguration ), 1}*/
/*not relevant*/    /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.addrCompletion), 1}*/
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.sampledAtResetReg     ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.metalFix              ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.globalControl         ), 1}
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQBaseAddr           ), 1}*/
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQControl            ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.interruptCoalescing   ), 1}
        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.lastReadTimeStampReg  ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.extendedGlobalControl ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.generalConfigurations ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.genxsRateConfig       ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(twsiReg.serInitCtrl   ), 1}
/*not dup*/ /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.dummyReadAfterWriteReg), 1}*/

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
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
    OUT   GT_BOOL                  *unitPerPipePtr,
    OUT   GT_BOOL                  *unitPer2TilesPtr,
    OUT   GT_BOOL                  *unitPerMgPtr
)
{
    GT_U32  ii;
    GT_U32  stepTiles;
    GT_U32  relativeRegAddr,pipe0Addr;
    GT_U32  tileId;
    GT_U32  unitsIndex    = *unitsIndexPtr;
    GT_U32  dupIndex      = *dupIndexPtr;
    GT_BOOL dupWasDone    = GT_FALSE;
    GT_BOOL unitPerPipe   = GT_FALSE;
    GT_BOOL unitPer2Tiles = GT_FALSE;
    GT_BOOL unitPerMg     = GT_FALSE;
    GT_U32  tileOffset;
    GT_U32  portGroupIndex;
    GT_U32  mgId,isSdmaRegister = 0,isSdmaRegisterAllowDup = 0;
    GT_U32  isAuq = 0 , isFuq = 0 , isGeneralPerTile = 0;


    if(GT_TRUE == falcon_fuqSupport_mgAddr(devNum, regAddr))
    {
        isFuq = 1;
    }
    else
    if(GT_TRUE == falcon_auqSupport_mgAddr(devNum, regAddr))
    {
        isAuq = 1;
    }
    else
    if(GT_TRUE == falcon_mustDuplicatePerTile_mgAddr(devNum, regAddr))
    {
        isGeneralPerTile = 1;
    }
    else
    if (GT_TRUE == falcon_convertPerMg_mgSdmaAddr(devNum, regAddr))
    {
        /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */
        isSdmaRegister = 1;
    }
    else
    if (GT_TRUE == falcon_allowDuplicatePerMg_mgSdmaAddr(devNum, regAddr))
    {
        isSdmaRegister = 1;
        isSdmaRegisterAllowDup = 1;
    }
    else
    {
        return GT_FALSE;
    }

    if(isFuq || isAuq || isGeneralPerTile)
    {
        GT_U32  for2MG_pipeToMg[4]={1,4,5};
        GT_U32  for1MG_pipeToMg[1]={4};
        /* FUQ registers need special manipulations */

        /* NOTE: we assume that every access to WRITE to those register is done
           using 'port group' indication that is not 'unaware' !!!! */
        /* we allow 'read' / 'check status' on all port groups */

        /* in tile 0 : we get here with the address of 'MG_0_0' but we may need to
           convert it to address of 'MG_0_1'
           in tile 1 : need to convert to 'MG_1_0' or 'MG_1_1'
           in tile 2 : need to add '2 tiles' offset (like in tile 0)
           in tile 3 : need to add '2 tiles' offset (like in tile 1)
        */
        GT_U32  arrSize;
        GT_U32 *pipeToMg;
        GT_U32  is2MGsPerTile = isFuq || isAuq;


        if(is2MGsPerTile)
        {
            /* FUQ is in 2 MGs */
            /* some AUQ registers needed in FUQ operation even if no AUQ*/
            pipeToMg =  for2MG_pipeToMg;
            arrSize = sizeof(for2MG_pipeToMg)/sizeof(for2MG_pipeToMg[0]);
        }
        else
        {
            pipeToMg =  for1MG_pipeToMg;
            arrSize = sizeof(for1MG_pipeToMg)/sizeof(for1MG_pipeToMg[0]);
        }

        if(is2MGsPerTile)
        {
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*for pipe 1*/
        }
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_0_E;/*for pipe 2*/
        if(is2MGsPerTile)
        {
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_1_E;/*for pipe 3*/
        }

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        /* support the tiles 0,1 */
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex] = pipeToMg[portGroupIndex]; /*per mg*/
            portGroupIndex++;
        }

        stepTiles = 2;
        /* support the tiles 2,3 */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            /* tile offset */
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex] = ii == 0 ? 8 : 8 + pipeToMg[(portGroupIndex % arrSize)];/* pipe 4..7 */
                portGroupIndex++;
            }
        }

        dupWasDone  = GT_TRUE;
        unitPerMg = GT_TRUE;
    }

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

            portGroupIndex = 0;

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
            additionalBaseAddrPtr[dupIndex++] = GT_NA;/* will cause fatal error */
            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = portGroupId;

            dupWasDone  = GT_TRUE;
            unitPer2Tiles = GT_TRUE;/* allow code into switch(portGroupId)  */

            *unitsIndexPtr = unitsIndex;
            *dupIndexPtr   = dupIndex;
            *dupWasDonePtr = dupWasDone;
            *unitPerPipePtr  = unitPerPipe;
            *unitPer2TilesPtr= unitPer2Tiles;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TRUE, "ERROR : the MG register [0x%x] must not be duplicated 'unaware' to all MG units",
                regAddr);
        }


        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_2_E;/*MG2*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_3_E;/*MG3*/
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles >= 2)
        {
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_0_E;/*MG4*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_1_E;/*MG5*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_2_E;/*MG6*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_3_E;/*MG7*/
        }

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        mgId = 1;
        /* support the tiles 0,1 : MG 1..7 */
        /* the additionalBaseAddrPtr[] not holding the 'MG 0' so we skip index 0 */
        for(ii = 1; ii < 4/* first tile */ ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++; /*per MG*/
        }

        /* the SDMAs of MG[4..7] are connected to DP[11..8] (reverse order) -
           but we want to expose 'units in accending' DP order
           so need to 'swap the MG units' */
        for(ii = 4; ii < unitsIndex /*second tile*/; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = 11-ii; /*per MG*/
        }

        mgId = unitsIndex;

        stepTiles = 2;
        /* support the tiles 2,3 */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            /* tile offset */
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

            for(ii = 0; ii < 4 /* third tile */; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++;/* MG 8..11 */
            }

            /* the SDMAs of MG[12..15] are connected to DP[27..24] (reverse order) -
               but we want to expose 'units in accending' DP order
               so need to 'swap the MG units' */
            for(ii = 4; ii < unitsIndex /* forth tile */; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex++] = 19-ii;/* MG 15..12 */
            }

            mgId = 2*unitsIndex;

        }

        dupWasDone  = GT_TRUE;
        unitPer2Tiles = GT_FALSE;/* NOT per 2 tiles , but per MG */
        unitPerMg = GT_TRUE;
    }

    *unitsIndexPtr = unitsIndex;
    *dupIndexPtr   = dupIndex;
    *dupWasDonePtr = dupWasDone;
    *unitPerPipePtr  = unitPerPipe;
    *unitPer2TilesPtr= unitPer2Tiles;
    *unitPerMgPtr= unitPerMg;


    return GT_TRUE;
}

/* allow the DB of register to NOT hold all the addresses of all port groups */
/* but the caller must used 'aware' call with specific port group indication */
static GT_BOOL per_pipe_UnitDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    INOUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT  unitId,
    IN    PRV_CPSS_DXCH_UNIT_ENT  pipe1UnitId,
    INOUT PRV_CPSS_DXCH_UNIT_ENT  usedUnits[/*PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS*/],
    INOUT GT_U32                  *unitsIndexPtr,
    INOUT GT_U32                  *additionalBaseAddrPtr,
    INOUT GT_U32                  *dupIndexPtr,
    OUT   GT_BOOL                  *dupWasDonePtr,
    OUT   GT_BOOL                  *unitPerPipePtr
)
{
    GT_U32  ii;
    GT_U32  relativeRegAddr,pipe0Addr;
    GT_U32  tileId;
    GT_U32  unitsIndex    = *unitsIndexPtr;
    GT_U32  dupIndex      = *dupIndexPtr;
    GT_BOOL dupWasDone    = GT_TRUE;
    GT_BOOL unitPerPipe   = GT_TRUE;
    GT_U32  tileOffset;
    GT_U32  pipeId,portGroupIndex;

    /* NOTE: we assume that every access to WRITE to those register is done
       using 'port group' indication that is not 'unaware' !!!! */
    /* we allow 'read' / 'check status' on all port groups */

    if(portGroupId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : the register [0x%x] must not be duplicated 'unaware' to all pipes for unit[%d]",
            regAddr,
            unitId);
    }

    /* in tile 0 : we get here with the address of 'pipe 0' but we may need to
       convert it to address of 'pipe 1'
    */
    usedUnits[unitsIndex++] = pipe1UnitId;/*for pipe 1*/

    relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

    portGroupIndex = 0;
    pipeId = 1;
    /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
    for(ii = 1; ii < unitsIndex ; ii++)
    {
        additionalBaseAddrPtr[dupIndex++] =
            prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
            relativeRegAddr;/* baseOfUnit + 'relative' offset */

        additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId++; /*per pipe*/
    }

    /* support the other tiles */
    for(tileId = 1 ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId ++)
    {
        /* tile offset */
        tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

        for(ii = 0; ii < unitsIndex ; ii++)
        {
            pipe0Addr = ii == 0 ?
                regAddr :/* index 0 is 'orig' regAddr */
                additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                               address of the dup units in pipes 0,1 */

            additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId++;/* pipe 2..7 */
        }
    }

    *unitsIndexPtr = unitsIndex;
    *dupIndexPtr   = dupIndex;
    *dupWasDonePtr = dupWasDone;
    *unitPerPipePtr  = unitPerPipe;

    return GT_TRUE;
}

/**
* @internal prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; AC5P; AC5X; Harrier; Ironman.
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
GT_BOOL prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
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
    GT_BOOL unitPerPipe;    /*indication to use 'per pipe'    units */
    GT_BOOL unitPerTile;    /*indication to use 'per tile'    units */
    GT_BOOL unitPer2Tiles;  /*indication to use 'per 2 tiles' units */
    GT_BOOL unitPerMg;      /*indication to use 'per MG'      units */
    GT_BOOL dupWasDone;     /*indication that dup per pipe/tile/2 tiles was already done */
    GT_U32  stepTiles;      /* steps between tiles */
    PRV_CPSS_DXCH_UNIT_ENT  usedUnits[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];
    GT_U32  unitsIndex; /* index in usedUnits */
    GT_U32  pipe0Addr;/* address of the duplicated unit in pipe 0 */
    GT_U32  pipeId;/* pipeId iterator */
    GT_U32  tileId;/* tileId iterator */
    GT_U32  pipeOffset;/* offset of current pipe from pipe 0 */
    GT_U32  relativeRegAddr;/* relative register address to it's unit */
    const GT_U32    maskUnitsPerTile = 0x00000055;/* bits : 0,2,4,6 */
    GT_U32  forcePipesBmp_arp_ts_table = 0;

    if(!PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        *portGroupsBmpPtr = BIT_0;
        /* the GM device not supports pipe 1 and also not support multiple DP units */
        /* so do not give any duplications */
        return GT_FALSE;
    }

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    unitPerPipe = GT_FALSE;
    unitPerTile = GT_FALSE;
    unitPer2Tiles = GT_FALSE;
    unitPerMg = GT_FALSE;
    dupWasDone = GT_FALSE;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];
    dupIndex = 0;

    /* call direct to prvCpssDxChFalconHwRegAddrToUnitIdConvert ... not need to
       get to it from prvCpssDxChHwRegAddrToUnitIdConvert(...) */
    unitId = prvCpssDxChFalconHwRegAddrToUnitIdConvert(devNum, regAddr);
    /* set the 'orig' unit at index 0 */
    unitsIndex = 0;
    usedUnits[unitsIndex++] = unitId;

    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_IA_E                          :
            if(GT_TRUE == falcon_supportDuplicate_portGroup_iaAddr(devNum, regAddr))
            {
                if(GT_FALSE == per_pipe_UnitDuplicatedMultiPortGroupsGet(
                    devNum,portGroupId,regAddr,additionalRegDupPtr,
                    unitId,PRV_CPSS_DXCH_UNIT_IA_1_E/*pipe1*/,
                    usedUnits,&unitsIndex,additionalBaseAddrPtr,&dupIndex,
                    &dupWasDone,&unitPerPipe))
                {
                    /* not supporting duplication -- should not happen */
                    return GT_FALSE;
                }
                /* all needed parameters already set inside per_pipe_UnitDuplicatedMultiPortGroupsGet */
            }
            else
            {
                /* duplicate to all port groups */
                unitPerPipe = GT_TRUE;
            }

            break;

        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == falcon_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA3_E;

            unitPerPipe = GT_TRUE;

            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == falcon_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA3_E;

            unitPerPipe = GT_TRUE;

            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == falcon_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E;

            unitPerPipe = GT_TRUE;

            break;

        case PRV_CPSS_DXCH_UNIT_LPM_E:
            if(GT_TRUE == falcon_mustNotDuplicatePerPipe_lpmMemAddr(devNum, regAddr))
            {
                /* the LPM table duplicated per tile */
                /* the pipe 0,1 duplicated by HW (simulation share it) */
                /* the pipe 2,3 duplicated by HW (simulation share it) */
                /* the pipe 4,5 duplicated by HW (simulation share it) */
                /* the pipe 6,7 duplicated by HW (simulation share it) */

                /* !!! TRICK !!! */
                unitPerTile = GT_TRUE;
            }
            else
            {
                /* the other tables duplicated per pipe !!! */
                unitPerPipe = GT_TRUE;
            }

            break;
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E                :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_LPM_1_E:
            /* this unit is not for duplication */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_SERDES_E                      :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_MIB_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_GOP_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_TAI_E                         :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_TAI1_E                         :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E:
            return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_TTI_E                         :
            if(GT_TRUE == falcon_mustNotDuplicate_ttiAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_EQ_E                         :
            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_HA_E                          :
            unitPerPipe = GT_TRUE;
            if((regAddr & 0x00FFFFFF) >= 0x00200000)
            {
                /* ERR - 3996522 */
                /* WA for TS/ARP/NAT table to access only the even port groups */
                /* needed because HW under traffic not like writes from pipe 0,2,4,6 !!! */
                /* it may hange the shared memory that is accessed via the HA unit that serves traffic */
                /* odd pipes (1,3,5,7) : NOTE: due to mirrored tiles pipe 3 is bit '2' and pipe '7' is bit 6 */
                /* so bits : 1,2,5,6 are 0x66 (instead of 0xAA for bits : 1,3,5,7) */
                forcePipesBmp_arp_ts_table = 0x66;
            }
            break;



        case PRV_CPSS_DXCH_UNIT_BMA_E                         :
        case PRV_CPSS_DXCH_UNIT_PHA_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_SHT_E                     :
        case PRV_CPSS_DXCH_UNIT_EGF_QAG_E                     :
        case PRV_CPSS_DXCH_UNIT_PCL_E                         :
        case PRV_CPSS_DXCH_UNIT_L2I_E                         :
        case PRV_CPSS_DXCH_UNIT_IPVX_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_1_E                      :
        case PRV_CPSS_DXCH_UNIT_IOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_MLL_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_EFT_E                     :
        case PRV_CPSS_DXCH_UNIT_CNC_0_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_1_E                       :
        case PRV_CPSS_DXCH_UNIT_ERMRK_E                       :
        case PRV_CPSS_DXCH_UNIT_EPCL_E                        :
        case PRV_CPSS_DXCH_UNIT_EPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_EOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_EREP_E                        :
        case PRV_CPSS_DXCH_UNIT_PREQ_E                        :
        case PRV_CPSS_DXCH_UNIT_HBU_E                         :
            /* ALL those are units that ALL it's config should be also on pipe 1 */
            /* NOTE: if one of those units hold some special registers ....
                     take it out of this generic case */
            unitPerPipe = GT_TRUE;
            break;

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E                         :
            if(GT_FALSE == mgUnitDuplicatedMultiPortGroupsGet(
                devNum,portGroupId,regAddr,additionalRegDupPtr,unitId,
                usedUnits,&unitsIndex,additionalBaseAddrPtr,&dupIndex,
                &dupWasDone,&unitPerPipe,&unitPer2Tiles,&unitPerMg))
            {
                return GT_FALSE;
            }

            break;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E     :/*per tile */
            unitPerTile = GT_TRUE;
            break;
        /************/
        /* per tile */
        /************/
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                     :
        case PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                     :
              return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                     :
            if(GT_TRUE == falcon_mustDuplicate_pdxAddr(devNum, regAddr))
            {
                /* the address is  for duplication */
                unitPerTile = GT_TRUE;
            }
            else
            {
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_SHM_E                         :
            if(GT_TRUE == falcon_mustNotDuplicate_shmAddr(devNum, regAddr))
            {
                /* the address is not for duplication */
                return GT_FALSE;
            }

            unitPerTile = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_TCAM_E                        :
        case PRV_CPSS_DXCH_UNIT_FDB_E                         :
        case PRV_CPSS_DXCH_UNIT_EM_E                          :
        case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E                  :

            unitPerTile = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E                 :/* Falcon Packet Buffer Subunits */
        case PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E          :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E     :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E     :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E      :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E        :
            unitPerTile = GT_TRUE;
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
    if(unitPerTile == GT_TRUE || unitPer2Tiles == GT_TRUE)
    {
        if(unitPer2Tiles == GT_TRUE)
        {
            /***************************************/
            /* start duplication for 'per 2 tiles' */
            /***************************************/
            stepTiles = 2;
        }
        else
        {
            /************************************/
            /* start duplication for 'per tile' */
            /************************************/
            stepTiles = 1;
        }

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 2)
        {
            /* no duplications needed */
            return GT_FALSE;
        }

        pipe0Addr = regAddr;
        /* instance 0 already in the array */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            pipeOffset = prvCpssSip6TileOffsetGet(devNum,tileId);
            additionalBaseAddrPtr[dupIndex++] = pipe0Addr + pipeOffset;
            additionalRegDupPtr->portGroupsArr[(tileId/stepTiles)-1] = 2*tileId; /*2 pipes per tile*/
        }
    }
    else
    if(unitPerPipe == GT_TRUE)
    {
        /************************************/
        /* start duplication for 'per pipe' */
        /************************************/

        /* need to double the total indexes
           if unitsIndex = 1 meaning   'only' orig = 1 access --> need to become 2 access by setting unitsIndex = 2
           if unitsIndex = 3 meaning 2 dups + orig = 3 access --> need to become 6 access by setting unitsIndex = 6
        */

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        /*******************************************/
        /* first handle the duplications in pipe 0 */
        /*******************************************/
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        pipeId = 0;
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId;
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */
        }

        /**********************************************/
        /* now handle the duplications in other pipes */
        /**********************************************/
        for(pipeId = 1 ; pipeId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes ; pipeId++)
        {
            if(forcePipesBmp_arp_ts_table && (0 == (forcePipesBmp_arp_ts_table & (1<<pipeId))))
            {
                /* skip this pipe as it is forbidden to be used for this memory ! */
                continue;
            }

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipe 0 */

                /* jump from address of pipe0 to one of other pipe */
                pipeOffset = falconOffsetFromPipe0Get(pipeId,usedUnits[ii]);
                if(forcePipesBmp_arp_ts_table && (pipeId & 1)/*odd*/)
                {
                    /* make it even */
                    additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId^1;
                }
                else
                {
                    additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId;
                }
                additionalBaseAddrPtr[dupIndex++] = pipe0Addr +
                    pipeOffset;
            }
        }
    }

    if((unitPerTile == GT_TRUE)   || /* tile duplication */
       (unitPer2Tiles == GT_TRUE) || /* 2 tiles duplication */
       (unitPerPipe == GT_TRUE) ||   /* pipe indication  */
       (unitPerMg == GT_TRUE))         /* MG   indication  */
    {
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
                if(forcePipesBmp_arp_ts_table)
                {
                    /* pipe 0 is not member , so orig address is not relevant */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                    *portGroupsBmpPtr = maskUnitsPerTile;
                }
                else
                {
                    additionalRegDupPtr->use_originalAddressPortGroup = GT_TRUE;
                    additionalRegDupPtr->originalAddressPortGroup = 0; /* pipe 0 */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                }

                break;
            case 0:
                if (forcePipesBmp_arp_ts_table)
                {
                    *portGroupsBmpPtr = BIT_0;
                    /* access only to this pipe 1 */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    /* the only valid address is the one in additionalBaseAddrPtr[0] */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                }
                else
                {
                    *portGroupsBmpPtr = BIT_0;
                    dupIndex = 0;/* access only to pipe 0 --> no duplications */
                }
                break;
            default:
                if((unitPerMg == GT_FALSE && portGroupId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes) ||
                   (unitPerMg == GT_TRUE  && portGroupId < PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits))
                {
                    *portGroupsBmpPtr = (BIT_0 << portGroupId);
                    /* access only to this pipe (1/2/3/4/5/6/7) */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    /* the only valid address is the one in additionalBaseAddrPtr[0] */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                }
                else
                {
                    /* should not get here */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : unsupported port group id [%d] ",
                        portGroupId);
                }
                break;
        }

    }
    else
    {
        /* the other units not care about multi-pipe */
    }

    if(dupIndex == 0)
    {
        return GT_FALSE;
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
        /**********************************************/
        /* mask the pipes BMP with existing pipes BMP */
        /**********************************************/
        *portGroupsBmpPtr &= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
        if((unitPerTile == GT_TRUE) || (unitPer2Tiles == GT_TRUE))
        {
            *portGroupsBmpPtr &= maskUnitsPerTile;
        }
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;
    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}


/**
* @internal falconOffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
/*static */GT_U32   falconOffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
)
{
    GT_U32  numOfDpPerPipe,numOfPipesPerTile,mirroredTilesBmp;
    GT_U32  relativeInstanceInPipe0;
    GT_U32  pipe0UnitId , pipeId , tileId;
    GT_U32  instance0BaseAddr,instanceInPipe0BaseAddr,offsetFromPipe0;

    /* use 'PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]' because it may be called from the cpssDriver
       before the 'PRV_CPSS_PP_MAC(devNum)' initialized ! */
    numOfDpPerPipe = 4;
    numOfPipesPerTile = 2;
    mirroredTilesBmp  = MIRRORED_TILES_BMP_CNS;

    switch(instance0UnitId)
    {
        /************/
        /* per pipe */
        /************/
        default:
            pipe0UnitId = instance0UnitId;
            pipeId      = instanceId;
            break;

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E               :
        {
            /* tile 0 : MG 0,1,2,3 - serve 'tile 0' located at tile 0 memory */
            /* tile 1 : MG 0,1,2,3 - serve 'tile 1' located at tile 0 memory */
            /* tile 2 : MG 0,1,2,3 - serve 'tile 2' located at tile 2 memory */
            /* tile 3 : MG 0,1,2,3 - serve 'tile 3' located at tile 2 memory */
            GT_U32  tileOffset = ((2 * FALCON_TILE_OFFSET_CNS) * (instanceId / NUM_MG_PER_CNM));
            GT_U32  internalCnmOffset = (MG_SIZE            * (instanceId % NUM_MG_PER_CNM));

            return tileOffset + internalCnmOffset;
        }

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_SMI_0_E           :
        case  PRV_CPSS_DXCH_UNIT_SMI_1_E           :
            pipe0UnitId = instance0UnitId;
            pipeId      = 2 * numOfPipesPerTile * instanceId;
            break;

        /************/
        /* per tile */
        /************/
        case  PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E:
        case  PRV_CPSS_DXCH_UNIT_TCAM_E             :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_E          :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E    :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E    :
        case  PRV_CPSS_DXCH_UNIT_FDB_E              :
        case  PRV_CPSS_DXCH_UNIT_EM_E               :
        case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E        :
        case  PRV_CPSS_DXCH_UNIT_SHM_E              :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PSI_E          :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E         :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E     :
            pipe0UnitId = instance0UnitId;
            pipeId      = 2 * instanceId;
            break;

        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

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
            pipeId                  = instanceId / numOfDpPerPipe;
            tileId = pipeId / numOfPipesPerTile;

            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

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
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

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
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

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
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

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
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E; break;
            }
            break;

         case  PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E; break;
            }
            break;
    }

    /* we got here for units that are per DP */
    /* 1. get the offset of the unit from first instance in Pipe 0 */
    /*  1.a get address of first  instance */
    instance0BaseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,instance0UnitId,NULL);
    /*  1.b get address of needed instance (in pipe 0)*/
    instanceInPipe0BaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,pipe0UnitId,NULL);

    /* 2. get the offset of the unit from Pipe 0                   */
    offsetFromPipe0 = falconOffsetFromPipe0Get(pipeId,pipe0UnitId);

    return (instanceInPipe0BaseAddr - instance0BaseAddr) + offsetFromPipe0;
}


/**
* @internal falconSpecialPortsMapGet function
* @endinternal
*
* @brief   Falcon : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS falconSpecialPortsMapGet
(
    IN  GT_U8   devNum,
    OUT const SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            *specialDmaPortsPtrPtr   =           falcon_3_2_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_3_2_SpecialDma_ports;
            break;
        case 2:
            *specialDmaPortsPtrPtr   =           falcon_6_4_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_6_4_SpecialDma_ports;
            break;
        case 4:
            *specialDmaPortsPtrPtr   =           falcon_12_8_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_12_8_SpecialDma_ports;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "not supported number of tiles [%d]",
                PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    return GT_OK;
}


/**
* @internal prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   Falcon : convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    const SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    GT_U32  numPortsInPipe;

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;
    numPortsInPipe  = numRegularPorts / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;

    if(globalMacPortNum < numRegularPorts)
    {
        if(pipeIndexPtr)
        {
            *pipeIndexPtr       = globalMacPortNum / numPortsInPipe;
        }

        if(localMacPortNumPtr)
        {
            *localMacPortNumPtr = globalMacPortNum % numPortsInPipe;
        }
    }
    else
    {
        rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(globalMacPortNum >= (numRegularPorts + numOfSpecialDmaPorts))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global MAC number [%d] is >= [%d] (the max)",
                globalMacPortNum,
                numRegularPorts + numOfSpecialDmaPorts);
        }

        if(specialDmaPortsPtr[globalMacPortNum-numRegularPorts].localPortInPipe == GT_NA)
        {
            /* the global port is NOT valid global GOP (only valid DMA port) */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global MAC number [%d] is not valid",
                globalMacPortNum);
        }

        if(pipeIndexPtr)
        {
            *pipeIndexPtr       = specialDmaPortsPtr[globalMacPortNum-numRegularPorts].pipeIndex;
        }

        if(localMacPortNumPtr)
        {
            *localMacPortNumPtr = specialDmaPortsPtr[globalMacPortNum-numRegularPorts].localPortInPipe;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToLocalNumInDpConvert
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
            *dpIndexPtr     = globalDmaNum / FALCON_PORTS_PER_DP_CNS;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr = globalDmaNum % FALCON_PORTS_PER_DP_CNS;
        }
    }
    else
    {
        rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
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
* @internal prvCpssFalconGlobalDpToTileAndLocalDp function
* @endinternal
*
* @brief   Falcon : convert the global DP number in device to local DP number and tile
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDpIndex             - the DP global number.
*
* @param[out] localDpIndexPtr               - (pointer to) the local Data Path (DP) Index
* @param[out] tileIndexPtr           - (pointer to) the  tile number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGlobalDpToTileAndLocalDp
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDpIndex,
    OUT  GT_U32 * localDpIndexPtr,
    OUT  GT_U32  * tileIndexPtr
)
{
    GT_U32 tileId,localDpIndexInTile;
    GT_U32  numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ? PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
    GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                             numOfTiles;

    if(globalDpIndex>PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "globalDpIndex [%d] > [%d] (the max)",
            globalDpIndex,PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp);
    }


    tileId = globalDpIndex / numDpPerTile;

    if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        localDpIndexInTile = globalDpIndex % numDpPerTile;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        localDpIndexInTile = (numDpPerTile - 1) - (globalDpIndex % numDpPerTile);
    }

    *localDpIndexPtr = localDpIndexInTile;
    *tileIndexPtr = tileId;


    return GT_OK;
}

/**
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
static GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert
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

    if(localDmaNum > FALCON_PORTS_PER_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "local DMA number [%d] > [%d] (the max)",
            localDmaNum,FALCON_PORTS_PER_DP_CNS);
    }

    if(localDmaNum < FALCON_PORTS_PER_DP_CNS)
    {
        *globalDmaNumPtr = (FALCON_PORTS_PER_DP_CNS*dpIndex) + localDmaNum;
        return GT_OK;
    }

    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
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
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Falcon : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
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
static GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext
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
    rc = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert(devNum,
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
    if(localDmaNum < FALCON_PORTS_PER_DP_CNS)
    {
        /* regular port without muxing */

        return /* not error for the LOG */ GT_NO_MORE;
    }

    /* use the pointer as 'IN' parameter */
    globalDmaNum = *globalDmaNumPtr;

    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
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

/**
* @internal prvCpssFalconDmaGlobalDmaMuxed_getNext function
* @endinternal
*
* @brief   Falcon : support 'mux' of SDMA CPU port or network CPU port.
*          the function return 'next' global DMA port number that may share the
*          same local dma number in the same DP.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaGlobalDmaMuxed_getNext
(
    IN  GT_U8   devNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32 globalDmaNum,dpIndex,localDmaNum;

    globalDmaNum = *globalDmaNumPtr;

    /* first get the dpIndex and localDmaNum of the global DMA port number */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,globalDmaNum,&dpIndex,&localDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check if the dpIndex and localDmaNum hold other muxed global DMA port number */
    return prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert_getNext(devNum,dpIndex,localDmaNum,globalDmaNumPtr);
}

/**
* @internal prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to TileId and to local
*         DMA number in the Local DataPath (DP index in tile).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] tileIdPtr                - (pointer to) the tile Id
*                                      dpIndexPtr          - (pointer to) the Data Path (DP) Index (local DP in the tile !!!)
*                                      localDmaNumPtr      - (pointer to) the DMA local number (local DMA in the DP !!!)
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *tileIdPtr,
    OUT GT_U32  *localDpIndexInTilePtr,
    OUT GT_U32  *localDmaNumInDpPtr
)
{
    GT_STATUS rc;
    GT_U32  tileId;
    GT_U32  globalDpIndex;
    GT_U32  temp_localDmaNumInDp;
    GT_U32  localDpIndexInTile , localDmaNumInDp;
    GT_U32  numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ? PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
    GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                           numOfTiles;

    /* get global DP[] and local DMA in the global DP */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,globalDmaNum,
        &globalDpIndex,&temp_localDmaNumInDp);
    if(rc != GT_OK)
    {
        return rc;
    }

    tileId = globalDpIndex / numDpPerTile;

    if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        localDpIndexInTile = globalDpIndex % numDpPerTile;
        localDmaNumInDp    = temp_localDmaNumInDp;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        localDpIndexInTile = (numDpPerTile - 1) - (globalDpIndex % numDpPerTile);

        localDmaNumInDp = temp_localDmaNumInDp;
    }

    if(tileIdPtr)
    {
        *tileIdPtr = tileId;
    }

    if(localDpIndexInTilePtr)
    {
        *localDpIndexInTilePtr = localDpIndexInTile;
    }

    if(localDmaNumInDpPtr)
    {
        *localDmaNumInDpPtr = localDmaNumInDp;
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_U32  globalDpIndex;
    GT_U32  temp_localDmaNumInDp;
    GT_U32  numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ? PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
    GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                           numOfTiles;

    if(numOfTiles <= 1)
    {
        /* also support for Hawk */
        /* no Tile conversion needed */
        return prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(devNum,
            localDpIndexInTile,localDmaNumInDp,globalDmaNumPtr);
    }

    if(localDpIndexInTile >= numDpPerTile)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "localDpIndexInTile[%d] must not be '>=' than [%d]",
            localDpIndexInTile,numDpPerTile);
    }

    if(localDmaNumInDp > FALCON_PORTS_PER_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "localDmaNumInDp[%d] must not be '>' than [%d]",
            localDmaNumInDp,FALCON_PORTS_PER_DP_CNS);
    }
/*      looking at the 'DPs' per tile :
        4        5        6        7        7        6        5        4
                        CP1                        CP1
3                                                                      3
2        CP0                                                  CP0      2
1                        T0                        T1                  1
0                                                                      0
0                                                                      0
1                        T1                        T0                  1
2        CP0                                                  CP0      2
3                                                                      3
                        CP1                        CP1
        4        5        6        7        7        6        5        4
*/
/*      looking at the 'DPs' per device :
        4        5        6        7        8        9        10       11
                        CP1                        CP2
3                                                                      12
2        CP0                                                 CP3       13
1                        T0                        T1                  14
0                                                                      15
31                                                                     16
30                       T3                        T2                  17
29        CP7                                                CP4       18
28                                                                     19
                        CP6                        CP5
        27      26       25      24        23       22        21       20
*/


    if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        globalDpIndex = localDpIndexInTile + (tileId * numDpPerTile);
        temp_localDmaNumInDp = localDmaNumInDp;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        globalDpIndex = ((numDpPerTile - 1) - localDpIndexInTile) + (tileId * numDpPerTile);
        temp_localDmaNumInDp = localDmaNumInDp;
    }

    return prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(devNum,
        globalDpIndex,temp_localDmaNumInDp,globalDmaNumPtr);
}

/**
* @internal prvCpssSip6LmuPhysicalPortNumToLmuIndexConvert function
* @endinternal
** @brief   sip6 LMU unit : convert the global physical port number to global LMU unit in the device and channel Id in it.
*         this is needed to access the registers in PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->LMU[lmuIndex]

*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - The PP's device number.
* @param[in] portMacNum           - the global mac port number.
* @param[out] lmuIndexPtr         - (pointer to) the global LMU unit index in the device.
* @param[out] localChannelInLmuPtr- (pointer to) the local channel that the port uses in the LMU unit.
* @return - GT_OK on success
*/
static GT_STATUS lmuPhysicalPortNumToLmuIndexConvert
(
    IN  GT_U8                 devNum,
    IN  GT_U32                portMacNum,
    OUT GT_U32                *lmuIndexPtr,
    OUT GT_U32                *localChannelInLmuPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          localDp;
    GT_U32                          lmuNum;
    GT_U32                          channel;
    GT_U32                          ravenNum;
    GT_U32                          tileNum;

    rc = prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert(
        devNum, portMacNum, &tileNum, &localDp, &channel);
    if (GT_OK != rc )
    {
        return rc;
    }

    ravenNum = localDp >> 1;/* /2 */
    lmuNum   = localDp &  1;/* %2 */
    if((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        /* mirrored tile - but the Raven is not mirrored */
        /* so revert the 'local DP index' that is already mirrored ! */
        lmuNum = 1 - lmuNum;
    }

    *lmuIndexPtr           = PRV_DXCH_FALCON_LMU_NUM_MAC(tileNum, ravenNum, lmuNum);
    *localChannelInLmuPtr  = channel;

    return GT_OK;
}


/**
* @internal falconRegDbInfoGet function
* @endinternal
*
* @brief   function to get the info to index in 'reg DB'
*
* @param[in] devNum          - The PP's device number.
* @param[in] portMacNum      - the global mac port number.
* @param[out] regDbType      - the type of regDbInfo.
* @param[out] regDbInfoPtr   - (pointer to) the reg db info
* @return - GT_OK on success
*/
static GT_STATUS   falconRegDbInfoGet(
    IN GT_U8                       devNum,
    IN GT_U32                      portMacNum,
    IN PRV_CPSS_REG_DB_TYPE_ENT    regDbType,
    IN PRV_CPSS_REG_DB_INFO_STC   *regDbInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      lmuIndex,localChannelInLmu;

    switch(regDbType)
    {
        case PRV_CPSS_REG_DB_TYPE_MTI_MIB_COUNTER_SHARED_E:
            /* shared per 8 ports , and index : /8 */
            regDbInfoPtr->regDbIndex       = portMacNum / 8;
            regDbInfoPtr->ciderIndexInUnit = portMacNum % 8;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E:
            regDbInfoPtr->regDbIndex       = portMacNum;/* index in regsAddrPtr->GOP.MTI[portNum].MTI_EXT */
            regDbInfoPtr->ciderIndexInUnit = portMacNum % 8;/* the local port index for Cider address calculations  */
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_LMU_E:
            rc = lmuPhysicalPortNumToLmuIndexConvert(devNum,portMacNum,&lmuIndex,&localChannelInLmu);
            if(rc != GT_OK)
            {
                return rc;
            }
            regDbInfoPtr->regDbIndex       = lmuIndex;
            regDbInfoPtr->ciderIndexInUnit = localChannelInLmu;
            break;
        case PRV_CPSS_REG_DB_TYPE_MTI_MPFS_E:
            regDbInfoPtr->regDbIndex       = mpfsRegisterIndexCalc(devNum,portMacNum);
            regDbInfoPtr->ciderIndexInUnit = 0;/* unused */
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(regDbType);
    }

    return GT_OK;
}



extern GT_U32 debug_force_numOfDp_get(void);
extern GT_U32 debug_force_numOfPipes_get(void);

GT_VOID prvCpssFalconNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
)
{
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssHwsUnitBaseAddrCalcFunc)  = falconUnitBaseAddrCalc;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->prvCpssRegDbInfoGetFunc) = falconRegDbInfoGet;
    DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssFalconDmaGlobalNumToLocalNumInDpConvert;
    DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(devNum) = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert;
}


/**
* @internal prvCpssFalconInitParamsSet function
* @endinternal
*
* @brief   Falcon : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconInitParamsSet
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
    GT_U32  debug_force_numOfPipes = debug_force_numOfPipes_get();
    GT_U32  lanesNumInDev;
    GT_U32  numMgUnitsPerTile = 4;
    GT_U32  numMgUnits = numMgUnitsPerTile * devPtr->multiPipe.numOfTiles;
    GT_U32  auqPortGroupIndex = 0,fuqPortGroupIndex = 0;
    GT_U32  frameLatencyArrFalcon[] =  {1955,1955,1955,1953,1978,1979,1950,1953,1974,1974,1979,1981,1955,1953,1974,1976};


    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits = numMgUnits;
    for(ii = 0 ; ii < numMgUnits; ii++)
    {
        if(0 == (ii % numMgUnitsPerTile))
        {
            /* AUQ per 4 MGs */
            /* each serve tile */
            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp |= BIT_0 << auqPortGroupIndex;

            /* the port groups bmp are 'per pipe' so every 2 pipes we have AUQ */
            auqPortGroupIndex += 2;
        }

        if((0 == (ii % numMgUnitsPerTile)) ||
           (1 == (ii % numMgUnitsPerTile)))
        {
            /* 2 FUQ per 4 MGs */
            /* each serve pipe */
            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp |= BIT_0 << fuqPortGroupIndex;

            /* the port groups bmp are 'per pipe' so every pipe we have FUQ */
            fuqPortGroupIndex++;
        }
    }

    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgCoreClock         = 312500000;

    devPtr->multiPipe.numOfPipesPerTile = 2;
    devPtr->multiPipe.tileOffset        = FALCON_TILE_OFFSET_CNS;
    devPtr->multiPipe.mirroredTilesBmp  = MIRRORED_TILES_BMP_CNS;/* in Falcon tile 1,3 are mirror image of tile 0 */

    devPtr->multiPipe.numOfPipes =
        devPtr->multiPipe.numOfTiles *
        devPtr->multiPipe.numOfPipesPerTile;

    dxDevPtr->hwInfo.multiDataPath.maxDp = devPtr->multiPipe.numOfPipes * 4;/* 4 DP[] per pipe */
    /* number of GOP ports per pipe , NOT including the 'CPU Port' . */
    devPtr->multiPipe.numOfPortsPerPipe = FALCON_PORTS_PER_DP_CNS * 4;/* 4 DP[] per pipe */

    if(debug_force_numOfPipes)
    {
        devPtr->multiPipe.numOfPipes = debug_force_numOfPipes;
    }

    if(debug_force_numOfDp)
    {
        dxDevPtr->hwInfo.multiDataPath.maxDp = debug_force_numOfDp;
        devPtr->multiPipe.numOfPortsPerPipe = FALCON_PORTS_PER_DP_CNS *
            (dxDevPtr->hwInfo.multiDataPath.maxDp /
             devPtr->multiPipe.numOfPipes);
    }

    PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * FALCON_RAVENS_PER_TILE;

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfSegmenetedPorts =
        PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens * 2;/* 2 segmented ports per Raven */

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts =
        PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens;/* CPU port per Raven - regardless to connectivity to the 'front panel'  */

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts =
        dxDevPtr->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;

    lanesNumInDev = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts +
                    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numOfNetworkCpuPorts;

    if(debug_force_numOfDp)
    {
        lanesNumInDev = debug_force_numOfDp * FALCON_PORTS_PER_DP_CNS;
    }

    switch (PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = 80+2;
            break;
        default:
            PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = lanesNumInDev;
            break;
    }

    /* led info */
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum = PRV_CPSS_DXCH_FALCON_LED_UNIT_NUM_CNS;
    PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts   = PRV_CPSS_DXCH_FALCON_LED_UNIT_PORTS_NUM_CNS;


    /* get info for 'SDMA CPU' port numbers */
    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->port.numRegularPorts;

    index = 0;
    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].localPortInPipe == GT_NA)
        {
            /* this is valid 'SDMA CPU' port */

            if(index >= CPSS_MAX_SDMA_CPU_PORTS_CNS)
            {
                CPSS_TBD_BOOKMARK_FALCON
                /* The CPSS is not ready yet to more SDMAs */
                break;
            }
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum = numRegularPorts + ii;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].valid  = GT_TRUE;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].usedAsCpuPort = GT_FALSE;/* the 'port mapping' should bind it */
            /* parameter called 'dqNum' but should be considered as 'DP index' */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dqNum  = specialDmaPortsPtr[ii].dpIndex;
            index++;
        }
    }

    /* NOTE: in FALCON the mirrored Tiles cause the DP[] units to hold swapped DMAs */
    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = ii * FALCON_PORTS_PER_DP_CNS;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = FALCON_PORTS_PER_DP_CNS + 1;/* support CPU port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum = FALCON_PORTS_PER_DP_CNS;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks = 30;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock = 14*_1K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap = _32K;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = 640;/* falcon : 640 lines , in non shared block */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  = 20;/* falcon : up to 20 blocks from non shared (to fill to total of 30) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = 30;/* falcon : 30 blocks supported */

    /* PHA info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg = 3;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn = 9;


    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts = 72;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues = CPSS_DXCH_SIP6_SDQ_QUEUE_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups = CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxPizzaSize =  33;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.sip6TxNumOfSpeedProfiles = MAX_SPEED_PROFILE_NUM_FALCON_CNS;

    /* SBM info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm = 9;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm = 4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.bmpSbmSupportMultipleArp = BIT_8;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm = _24K;/*96K in 4 SBMs*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.numLanesPerPort = 8;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId = 4;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors = 6;/* 6 *3K = 18K@10B */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lmuInfo.numLmuUnits = 2*PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens;

    PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits = 2;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart  =  _32K;/* used only by ENTRY_TYPE_TUNNEL_START_E */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb          = _256K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm           = _128K;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStgMembers   = _2K;/*according to 128 ports mode (for 64 mode it is 4K STGs) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesSrcIdMembers = _2K;/*according to 128 ports mode (for 64 mode it is 4K SRC IDs) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesNonTrunkMembers = _2K;/*according to 128 ports mode (for 64 mode it is 4K Trunk IDs) */

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssUnitIdSizeInByteGetFunc        = prvCpssFalconUnitIdSizeInByteGet;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssOffsetFromFirstInstanceGetFunc = falconOffsetFromFirstInstanceGet;

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.prvCpssHwRegAddrToUnitIdConvertFunc   = prvCpssDxChFalconHwRegAddrToUnitIdConvert;
    prvCpssFalconNonSharedHwInfoFuncPointersSet(devNum);

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits = PRV_CPSS_NUM_OF_TAI_IN_FALCON_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr = cpssOsMalloc((PRV_CPSS_NUM_OF_TAI_IN_FALCON_CNS-1)*sizeof(GT_U32));
    cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr, frameLatencyArrFalcon, sizeof(frameLatencyArrFalcon));
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr = NULL;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz = 500000;

    return GT_OK;
}


#define TXFIFO_BASE_ADDR_GET(devNum,txFifoUnitIndex)    \
    (prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL) + \
     prvCpssSip6OffsetFromFirstInstanceGet(devNum,txFifoUnitIndex,PRV_CPSS_DXCH_UNIT_TX_FIFO_E))

#define TXDMA_BASE_ADDR_GET(devNum,txDmaUnitIndex)    \
    (prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL) + \
     prvCpssSip6OffsetFromFirstInstanceGet(devNum,txDmaUnitIndex,PRV_CPSS_DXCH_UNIT_TXDMA_E))

/**
* @internal prvCpssSip6TxDmaDebugChannelCountersEnableSet function
* @endinternal
*
* @brief    enable the specific localDma debug counter in the TxDma unit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txDmaUnitIndex           - the  TxDma unit index (0..numOfPipes*4)
* @param[in] localDma                 - the local DMA (0xffffffff means all)
* @param[in] enable                   - enable / disable
*/
GT_STATUS   prvCpssSip6TxDmaDebugChannelCountersEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   txDmaUnitIndex,
    IN GT_U32   localDma,
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc;
    /* Cider path : Eagle/Pipe/TxDMA IP/TxDMA IP TLU/Units/TXD/Debug */
    GT_U32  infoArr[] = {
        /*address , value */
        0x0000700C, BIT_0 /*set in run time */     ,    /* disable / enable  counting (also trigger 'reset' all counters) */
        /* must be last */
        GT_NA, GT_NA
    };
    GT_U32  regAddr,value,ii;
    GT_U32  baseAddr;

    baseAddr = TXDMA_BASE_ADDR_GET(devNum, txDmaUnitIndex);

    if(enable == GT_TRUE)
    {
        infoArr[1] = BIT_0;

        regAddr = baseAddr + 0x00007004;   /* TXD Debug Configuration Register */
        if(localDma != 0xFFFFFFFF)
        {
            /* count specific port */
            value   = (2 << 10)/*cell_to_count*/ | localDma << 4;/*<count_all_ports>=0*/
        }
        else
        {
            /* count all ports */
            value   = (2 << 10)/*cell_to_count*/ | 1;/*<count_all_ports>=1*/
        }

        rc = prvCpssDrvHwPpWriteRegister(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        infoArr[1] = 0;
    }

    for(ii = 0 ; infoArr[ii] !=  GT_NA ; ii += 2)
    {
        regAddr = infoArr[ii+0] + baseAddr ;
        value   = infoArr[ii+1];

        rc = prvCpssDrvHwPpWriteRegister(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}
/**
* @internal prvCpssSip6TxDmaDebugCounterPdsDescCounterGet function
* @endinternal
*
* @brief    Get the debug counter in the TxDma unit of the 'pds desc counter'
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txDmaUnitIndex           - the  TxDma unit index (0..numOfPipes*4)
* @param[out] realCounterPtr          - (pointer to) upper 16 bits are : PDS   Real Descriptors Counter (    include dummy)
*                                       ignored if NULL
* @param[out] totalCounterPtr         - (pointer to) lower 16 bits are : Total Real Descriptors Counter (not include dummy)
*                                       ignored if NULL
*
*/
GT_STATUS   prvCpssSip6TxDmaDebugCounterPdsDescCounterGet
(
    IN GT_U8    devNum,
    IN GT_U32   txDmaUnitIndex,
    OUT GT_U32   *realCounterPtr,
    OUT GT_U32   *totalCounterPtr
)
{
    GT_STATUS rc;
    GT_U32  value;
    GT_U32  regAddr;
    GT_U32  baseAddr;

    baseAddr = TXDMA_BASE_ADDR_GET(devNum, txDmaUnitIndex);

    regAddr = baseAddr + 0x00003034;

    rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(realCounterPtr)
    {
        *realCounterPtr  = value >>    16;/*upper 16 bits are : PDS Real  Descriptors Counter (    include dummy)*/
    }
    if(totalCounterPtr)
    {
        *totalCounterPtr = value & 0xFFFF;/*lower 16 bits are : PDS Total Descriptors Counter (not include dummy)*/
    }

    return GT_OK;
}

/**
* @internal prvCpssSip6TxFifoDebugCountersEnableSet function
* @endinternal
*
* @brief    enable the debug counter in the TxFifo unit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txFifoUnitIndex          - the  TxFifo unit index (0..numOfPipes*4)
*/
GT_STATUS   prvCpssSip6TxFifoDebugCountersEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   txFifoUnitIndex,
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc;
    /* Cider path : Eagle/Pipe/<TXFIFO> TxFIFO IP TLU/Units_%x/TxFIFO IP Unit/Debug */
    GT_U32  infoArr[] = {
        /*address , value */
        0x00005004, BIT_0      ,    /*enable counting*/
        0x0000500C, 0xffffffff ,    /*0xffffffff for threshold*/
        0x00005008, BIT_8      ,    /*count 'all channels'*/
        /* must be last */
        GT_NA, GT_NA
    };
    GT_U32  regAddr,value,ii;
    GT_U32  baseAddr;

    infoArr[1] = enable;/* enable/disable the counting  */

    baseAddr = TXFIFO_BASE_ADDR_GET(devNum, txFifoUnitIndex);

    for(ii = 0 ; infoArr[ii] !=  GT_NA ; ii += 2)
    {
        regAddr = infoArr[ii+0] + baseAddr ;
        value   = infoArr[ii+1];

        rc = prvCpssDrvHwPpWriteRegister(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssSip6TxFifoDebugCountersDump function
* @endinternal
*
* @brief    dump the debug counter in the TxFifo unit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txFifoUnitIndex          - the  TxFifo unit index (0..numOfPipes*4)
*/
GT_STATUS   prvCpssSip6TxFifoDebugCountersDump
(
    IN GT_U8    devNum,
    IN GT_U32   txFifoUnitIndex

)
{
    GT_STATUS   rc;
    GT_U32  regAddr,value,ii;
    GT_U32  baseAddr;
    GT_CHAR*  counterNames[8] = {
       /*0*/  "descriptors read by read control"
       /*1*/  ,"cells split from the descriptors"
       /*2*/  ,"words generate in pre-fetch FIFO"
       /*3*/  ,"SOP read control derive to WB"
       /*4*/  ,"cycles from first descriptor"
       /*5*/  ,"Aligner input packets"
       /*6*/  ,"Aligner output packets"
       /*7*/  ,"Aligner output byte count"
    };

    baseAddr = TXFIFO_BASE_ADDR_GET(devNum, txFifoUnitIndex);

    baseAddr += 0x00005180;

    for(ii = 0 ; ii < 8 ; ii++)
    {
        regAddr = baseAddr + 4*ii;
        rc = prvCpssDrvHwPpReadRegister(devNum,regAddr,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("counter[%s] at [0x%8.8x] with value [0x%8.8x] \n",
            counterNames[ii],regAddr,value);
    }

    return GT_OK;
}

GT_VOID prvCpssFalconTaiRegistersDump
(
    IN GT_U8    devNum
)
{
    GT_U32 gopIndex;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 totalTaiGop = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * 9;

    for (gopIndex = 0; gopIndex < totalTaiGop; gopIndex++)
    {
        cpssOsPrintf("================= TAI Index [%d] ======================= \n", gopIndex % 9);

        cpssOsPrintf("TAIInterruptCause         [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAIInterruptCause, regsAddrPtr->GOP.TAI[gopIndex][1].TAIInterruptCause);
        cpssOsPrintf("TAIInterruptMask          [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAIInterruptMask, regsAddrPtr->GOP.TAI[gopIndex][1].TAIInterruptMask);
        cpssOsPrintf("TAICtrlReg0               [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAICtrlReg0, regsAddrPtr->GOP.TAI[gopIndex][1].TAICtrlReg0);
        cpssOsPrintf("TAICtrlReg1               [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAICtrlReg1, regsAddrPtr->GOP.TAI[gopIndex][1].TAICtrlReg1);
        cpssOsPrintf("timeCntrFunctionConfig0   [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].timeCntrFunctionConfig0, regsAddrPtr->GOP.TAI[gopIndex][1].timeCntrFunctionConfig0);
        cpssOsPrintf("triggerGenerationControl   [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].triggerGenerationControl, regsAddrPtr->GOP.TAI[gopIndex][1].triggerGenerationControl);
        cpssOsPrintf("clockControl   [0x%8.8x] [0x%8.8x] \n",
                     regsAddrPtr->GOP.TAI[gopIndex][0].clockControl, regsAddrPtr->GOP.TAI[gopIndex][1].clockControl);
    }
}

GT_VOID prvCpssFalconPtpRegistersDump
(
    IN GT_U8    devNum
)
{
    GT_U32 portNum;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 totalPorts = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * 64;

    for (portNum = 0; portNum < totalPorts; portNum++)
    {
        if (portNum % 4 == 0)
        {
            cpssOsPrintf("================= PTP Index [%d] ======================= \n", portNum % totalPorts);

            cpssOsPrintf("PTPInterruptCause         [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptCause,    regsAddrPtr->GOP.PTP[portNum][1].PTPInterruptCause);
            cpssOsPrintf("PTPInterruptMask          [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl,       regsAddrPtr->GOP.PTP[portNum][1].PTPGeneralCtrl);
            cpssOsPrintf("TAICtrlReg0               [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl,       regsAddrPtr->GOP.PTP[portNum][1].PTPGeneralCtrl);
            cpssOsPrintf("TAICtrlReg1               [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].NTPPTPOffsetLow,      regsAddrPtr->GOP.PTP[portNum][1].NTPPTPOffsetLow);
            cpssOsPrintf("timeCntrFunctionConfig0   [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].txPipeStatusDelay,    regsAddrPtr->GOP.PTP[portNum][1].txPipeStatusDelay);
            cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].totalPTPPktsCntr,     regsAddrPtr->GOP.PTP[portNum][1].totalPTPPktsCntr);
            cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n",
                         regsAddrPtr->GOP.PTP[portNum][0].egressTimestampQueue, regsAddrPtr->GOP.PTP[portNum][1].egressTimestampQueue);
        }
    }
}

#define FALCON_MTI_REG_PRINT_MAC(__name) \
        cpssOsPrintf("%s %08X\n", #__name, regsAddrPtr->GOP.MTI[portNum].__name);

void prvCpssFalconGopMtiRegsDump
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 portNum;
    for(portNum = 0; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        /* skip non exists MAC ports */
        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum,portNum);

        cpssOsPrintf("++++ Port %3d\n", portNum);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.commandConfig);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.frmLength);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.macAddr0);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.macAddr1);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.status);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl01PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl23PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl45PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl67PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl01QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl23QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl45QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.cl67QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_MAC.rxPauseStatus);

        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.commandConfig);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.frmLength);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.macAddr0);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.macAddr1);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.status);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl01PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl23PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl45PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl67PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl01QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl23QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl45QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.cl67QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI100_MAC.rxPauseStatus);

        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.commandConfig);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.frmLength);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.macAddr0);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.macAddr1);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.status);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl01PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl23PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl45PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl67PauseQuanta);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl01QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl23QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl45QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.cl67QuantaThresh);
        FALCON_MTI_REG_PRINT_MAC(MTI400_MAC.rxPauseStatus);

        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.portControl);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.portPeerDelay);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.segPortControl);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.xoffOverride);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.segXoffOverride);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.pauseOverride);
        FALCON_MTI_REG_PRINT_MAC(MTI_EXT.segPauseOverride);

        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_EXT.portControl);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_EXT.pauseOverride);
        FALCON_MTI_REG_PRINT_MAC(MTI_CPU_EXT.xoffOverride);
    }
}

/**
* @internal prvCpssFalconRavenMemoryAccessCheck function
* @endinternal
*
* @brief   Check memory access to Ravens in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum     - the device number
* @param[in] tileId     - tile ID
* @param[in] unitIndex  - unit index
*
* @param[out] skipUnitPtr  - (pointer to) skip status to Raven device
*/
GT_VOID prvCpssFalconRavenMemoryAccessCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileId,
    IN  PRV_CPSS_DXCH_UNIT_ENT  ravenId,
    OUT GT_BOOL                *skipUnitPtr
)
{
    GT_U32  ravenIndex;
    GT_BOOL skipUnit;

    skipUnit = GT_FALSE;

    switch(ravenId)
    {
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E:
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E:
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E:
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E:
            if((1 << tileId) & MIRRORED_TILES_BMP_CNS)
            {
                ravenIndex = 3 - (ravenId - PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E);
            }
            else
            {
                ravenIndex = ravenId - PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E;
            }
            ravenIndex = ravenIndex + tileId * 4;
            if ((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap & (1 << ravenIndex)) == 0) /* Not existing Raven */
            {
                skipUnit = GT_TRUE;
            }
            break;
        default:
            break;
    }
    *skipUnitPtr = skipUnit;
}

/**
* @internal prvCpssFalconRavenMemoryAddressSkipCheck function
* @endinternal
*
* @brief   Check skipping of non-valid Raven memory addresses in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum         - the device number
* @param[in] address        - memory address in Raven device to be sampled
* @param[in] skipUnitPtr    - pointer to) memory address status
*                               GT_TRUE     - the address should be skipped
*                               GT_FALSE    - the address should NOT be skipped
*/
GT_VOID prvCpssFalconRavenMemoryAddressSkipCheck
(
    IN GT_U8                   devNum,
    IN GT_U32                  address,
    INOUT GT_BOOL             *skipUnitPtr
)
{
    GT_U32  tileId;
    PRV_CPSS_DXCH_UNIT_ENT ravenId;

    /* Falcon device tile offset */
    #define FALCON_TILE_OFFSET_CNS      0x20000000
    /* Falcon device Ravens base addresses */
    #define RAVEN_0_START_ADDR_CNS      0x00000000
    #define RAVEN_1_START_ADDR_CNS      0x01000000
    #define RAVEN_2_START_ADDR_CNS      0x02000000
    #define RAVEN_3_START_ADDR_CNS      0x03000000

    *skipUnitPtr = GT_FALSE;

    if (address == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *skipUnitPtr = GT_TRUE;
        return;
    }

    /* Check access to all units in Ravens in all tiles */
    if (((address >> 28) & 0x1) == 0)
    {
        tileId = address / FALCON_TILE_OFFSET_CNS;
        /* check is address in raven's scope */
        switch (address & 0x0F800000)
        {
            case RAVEN_0_START_ADDR_CNS:
                ravenId = PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E;
                break;
            case RAVEN_1_START_ADDR_CNS:
                ravenId = PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E;
                break;
            case RAVEN_2_START_ADDR_CNS:
                ravenId = PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E;
                break;
            case RAVEN_3_START_ADDR_CNS:
                ravenId = PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E;
                break;
            default:
                return;
        }

        prvCpssFalconRavenMemoryAccessCheck(devNum, tileId, ravenId, skipUnitPtr);
    }

}

