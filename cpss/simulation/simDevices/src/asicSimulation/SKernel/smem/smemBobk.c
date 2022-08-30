/******************************************************************************
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
* @file smemBobk.c
*
* @brief Bobk memory mapping implementation
*
* @version   4
********************************************************************************
*/

#include <asicSimulation/SKernel/smem/smemBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>

/* port per DP --without-- the CPU port */
#define PORTS_PER_DP_CNS    12

/* not used memory */
#define DUMMY_UNITS_BASE_ADDR_CNS(index)        0x70000000 + UNIT_BASE_ADDR_MAC(2*index)

/* addresses of units that exists in bobk-aldrin but not in bobk */
#define BOBK_POE_UNIT_BASE_ADDR_CNS             0x61000000
#define BOBK_RX_DMA_2_UNIT_BASE_ADDR_CNS        0x69000000
#define BOBK_TX_DMA_2_UNIT_BASE_ADDR_CNS        0x6B000000
#define BOBK_TX_FIFO_2_UNIT_BASE_ADDR_CNS       0x6A000000



/* addresses of units that exists in bobk but not in bobcat2 */
#define BOBK_RX_DMA_1_UNIT_BASE_ADDR_CNS        0x68000000
#define BOBK_TX_DMA_1_UNIT_BASE_ADDR_CNS        0x66000000
#define BOBK_TX_FIFO_1_UNIT_BASE_ADDR_CNS       0x67000000
#define BOBK_ETH_TX_FIFO_1_UNIT_BASE_ADDR_CNS   0x62000000
#define BOBK_RX_DMA_GLUE_UNIT_BASE_ADDR_CNS     0x63000000
#define BOBK_TX_DMA_GLUE_UNIT_BASE_ADDR_CNS     0x64000000
#define BOBK_TAI_UNIT_BASE_ADDR_CNS             0x65000000
#define BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(index)  0x54000000 + (UNIT_BASE_ADDR_MAC((2*index)))
#define BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(index)  0x21000000 + (UNIT_BASE_ADDR_MAC((2*index)))
#define BOBK_GOP_LED_4_UNIT_BASE_ADDR_CNS       0x50000000

/* will set ports 0..47 */
/* instead of calling:
    {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x10180800 ,0)},
                FORMULA_TWO_PARAMETERS(30, 0x4, 56, 0x00001000)}

    need to call:
    BOBK_PORTS_LOW_PORTS__SUPPORT_FORMULA_1_PARAMS(0x10180800 ,0,
        30, 0x4)
*/
#define BOBK_PORTS_LOW_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    BOBK_PORTS_LOW_PORTS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define BOBK_PORTS_LOW_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr   ,numBytes)},                 \
        FORMULA_TWO_PARAMETERS(48, BOBCAT2_PORT_OFFSET_CNS,numSteps1, stepSize1)}



/* will set ports 56..59,64..71 , for ports with 'steps' */
/* macro to be used by other macros ..  */
#define ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(portsStep , startAddr,numBytes,numSteps1,stepSize1) \
    /* ports 56..59 */                                                         \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC((startAddr+BOBCAT2_PORT_56_START_OFFSET_CNS)   ,numBytes)}, \
        FORMULA_TWO_PARAMETERS((4/(portsStep)), (BOBCAT2_PORT_OFFSET_CNS*(portsStep)),numSteps1, stepSize1)}     \
    /* port 62 */                                                          \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC((startAddr+BOBCAT2_PORT_56_START_OFFSET_CNS + (BOBCAT2_PORT_OFFSET_CNS*(62-56)))   ,numBytes)}, \
        FORMULA_TWO_PARAMETERS((1/(portsStep)), (BOBCAT2_PORT_OFFSET_CNS*(portsStep)),numSteps1, stepSize1)}     \
    /* port 64..71 */                                                          \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC((startAddr+BOBCAT2_PORT_56_START_OFFSET_CNS + (BOBCAT2_PORT_OFFSET_CNS*(64-56)))   ,numBytes)}, \
        FORMULA_TWO_PARAMETERS((8/(portsStep)), (BOBCAT2_PORT_OFFSET_CNS*(portsStep)),numSteps1, stepSize1)}

/* will set ports 56..59,64..71 */
/* instead of calling:
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC((0x10180800 + 0x00200000)   ,0)},
                FORMULA_TWO_PARAMETERS(30, 0x4, 16, 0x00001000)},

    need to call:
    BOBK_PORTS_LOW_PORTS__SUPPORT_FORMULA_1_PARAMS(0x10180800 ,0,
        30, 0x4)
*/
#define BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(1 , startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define BOBK_PORTS_HIGH_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(1 , startAddr,numBytes,numSteps1,stepSize1)

/* set ports in step of 2 ports :
will set ports 56,58(range 56..59),64,66,68,70(range 64..71) */
#define BOBK_PORTS_HIGH_PORTS_STEP_2_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(2 , startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define BOBK_PORTS_HIGH_PORTS_STEP_2_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(2 , startAddr,numBytes,numSteps1,stepSize1)


/* will set GOPs 0..11 */
#define BOBK_PORTS_LOW_GOPS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr   ,numBytes)}, \
        FORMULA_TWO_PARAMETERS(12, (BOBCAT2_PORT_OFFSET_CNS * 4),numSteps1, stepSize1)}

#define BOBK_PORTS_LOW_GOPS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    BOBK_PORTS_LOW_GOPS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)


/* will set GOPS 14,16..17 */
#define BOBK_PORTS_HIGH_GOPS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    ___INTERNAL_BOBK_PORTS_HIGH_PORTS__PORTS_STEP(4 , startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

/* will set mib ports 0..47 */
/* instead of calling:
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000,0)}, FORMULA_TWO_PARAMETERS(128/4 , 0x4 , 56 , BOBCAT2_MIB_OFFSET_CNS)}

    need to call:
    BOBK_PORTS_LOW_MIB_PORTS__BY_END_ADDR(0x00000000 ,0,
        128/4, 0x4)
*/
#define BOBK_PORTS_LOW_MIB_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr   ,(((endAddr)-(startAddr)) + 4))},                            \
        FORMULA_TWO_PARAMETERS(48, BOBCAT2_MIB_OFFSET_CNS,numSteps1, stepSize1)}
/* will set mib ports 56..59,64..71 */
/* instead of calling:
            {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 +BOBCAT2_MIB_PORT_56_START_OFFSET_CNS,0)}, FORMULA_TWO_PARAMETERS(128/4 , 0x4 , 16 , BOBCAT2_MIB_OFFSET_CNS)}

    need to call:
    BOBK_PORTS_HIGH_MIB_PORTS__BY_END_ADDR((0x00000000 ) ,0,
        128/4, 0x4)
*/
#define BOBK_PORTS_HIGH_MIB_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1)\
    /* mib ports 56..59 */                                                         \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC((startAddr+BOBCAT2_MIB_PORT_56_START_OFFSET_CNS)   ,(((endAddr)-(startAddr)) + 4))}, \
        FORMULA_TWO_PARAMETERS(4, BOBCAT2_MIB_OFFSET_CNS,numSteps1, stepSize1)}            \
    /* mib ports 62 */                                                          \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(((startAddr+BOBCAT2_MIB_PORT_56_START_OFFSET_CNS) + (BOBCAT2_MIB_OFFSET_CNS*(62-56)))   ,(((endAddr)-(startAddr)) + 4))},\
        FORMULA_TWO_PARAMETERS(1, BOBCAT2_MIB_OFFSET_CNS,numSteps1, stepSize1)}            \
    /* mib ports 64..71 */                                                          \
    ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(((startAddr+BOBCAT2_MIB_PORT_56_START_OFFSET_CNS) + (BOBCAT2_MIB_OFFSET_CNS*(64-56)))   ,(((endAddr)-(startAddr)) + 4))},\
        FORMULA_TWO_PARAMETERS(8, BOBCAT2_MIB_OFFSET_CNS,numSteps1, stepSize1)}

#define BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    BOBK_ALDRIN_XLG_PORTS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define BOBK_ALDRIN_XLG_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr   ,numBytes)},                 \
        FORMULA_TWO_PARAMETERS(32, BOBCAT2_PORT_OFFSET_CNS,numSteps1, stepSize1)}

#define BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    BOBK_ALDRIN_PORTS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define BOBK_ALDRIN_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr   ,numBytes)},                 \
        FORMULA_TWO_PARAMETERS(33, BOBCAT2_PORT_OFFSET_CNS,numSteps1, stepSize1)}

typedef enum{
    /* units that exists in bobcat2 */
    BOBK_UNIT_MG_E,
    BOBK_UNIT_IPLR1_E,
    BOBK_UNIT_TTI_E,
    BOBK_UNIT_L2I_E,
    BOBK_UNIT_IPVX_E,
    BOBK_UNIT_BM_E,
    BOBK_UNIT_BMA_E,
    BOBK_UNIT_FDB_E,
    BOBK_UNIT_EPLR_E,
    BOBK_UNIT_MPPM_E,
    BOBK_UNIT_GOP_E,
    BOBK_UNIT_XG_PORT_MIB_E,
    BOBK_UNIT_SERDES_E,
    BOBK_UNIT_RX_DMA_E,
    BOBK_UNIT_TX_DMA_E,
    BOBK_UNIT_TXQ_QUEUE_E,
    BOBK_UNIT_TXQ_LL_E,
    BOBK_UNIT_TXQ_DQ_E,
    BOBK_UNIT_CPFC_E,
    BOBK_UNIT_MLL_E,
    BOBK_UNIT_IOAM_E,
    BOBK_UNIT_EOAM_E,
    BOBK_UNIT_LPM_E,
    BOBK_UNIT_EQ_0_E,/*part 0 of EQ*/
    BOBK_UNIT_EQ_1_E,/*part 1 of EQ*/
    BOBK_UNIT_FDB_TABLE_0_E,
    BOBK_UNIT_FDB_TABLE_1_E,
    BOBK_UNIT_EGF_EFT_E,
    BOBK_UNIT_EGF_EFT_1_E,
    BOBK_UNIT_EGF_QAG_E,
    BOBK_UNIT_EGF_QAG_1_E,
    BOBK_UNIT_EGF_SHT_E,
    BOBK_UNIT_EGF_SHT_1_E,
    BOBK_UNIT_EGF_SHT_2_E,
    BOBK_UNIT_EGF_SHT_3_E,
    BOBK_UNIT_EGF_SHT_4_E,
    BOBK_UNIT_EGF_SHT_5_E, /*!!! EMPTY !!!*/
    BOBK_UNIT_EGF_SHT_6_E,
    BOBK_UNIT_EGF_SHT_7_E, /*!!! EMPTY !!!*/
    BOBK_UNIT_EGF_SHT_8_E,
    BOBK_UNIT_EGF_SHT_9_E,
    BOBK_UNIT_EGF_SHT_10_E,
    BOBK_UNIT_EGF_SHT_11_E,
    BOBK_UNIT_EGF_SHT_12_E,
    BOBK_UNIT_HA_E,
    BOBK_UNIT_IPCL_E,
    BOBK_UNIT_IPLR0_E,
    BOBK_UNIT_EPCL_E,
    BOBK_UNIT_TCAM_E,
    BOBK_UNIT_TM_HOSTING_E,
    BOBK_UNIT_ERMRK_E,
    BOBK_UNIT_CNC_0_E,
    BOBK_UNIT_CNC_1_E,
    BOBK_UNIT_TMDROP_E,
    BOBK_UNIT_TXQ_QCN_E,
    BOBK_UNIT_TX_FIFO_E,
    BOBK_UNIT_ETH_TX_FIFO_E,
    BOBK_UNIT_TM_FCU_E,
    BOBK_UNIT_TM_INGRESS_GLUE_E,
    BOBK_UNIT_TM_EGRESS_GLUE_E,
    BOBK_UNIT_TMQMAP_E,
    /* units that are new in bobk */
    BOBK_UNIT_RX_DMA_1_E,
    BOBK_UNIT_TX_DMA_1_E,
    BOBK_UNIT_TX_FIFO_1_E,
    BOBK_UNIT_ETH_TX_FIFO_1_E,
    BOBK_UNIT_RX_DMA_GLUE_E,
    BOBK_UNIT_TX_DMA_GLUE_E,
    BOBK_UNIT_TAI_E,
    BOBK_UNIT_GOP_SMI_0_E,
    BOBK_UNIT_GOP_SMI_1_E,
    BOBK_UNIT_GOP_SMI_2_E,
    BOBK_UNIT_GOP_SMI_3_E,
    BOBK_UNIT_GOP_LED_0_E,
    BOBK_UNIT_GOP_LED_1_E,
    BOBK_UNIT_GOP_LED_2_E,
    BOBK_UNIT_GOP_LED_3_E,
    BOBK_UNIT_GOP_LED_4_E,
    /* units that are new in bobk-aldrin */
    BOBK_UNIT_RX_DMA_2_E,
    BOBK_UNIT_TX_DMA_2_E,
    BOBK_UNIT_TX_FIFO_2_E,
    BOBK_UNIT_POE_E,

    BOBK_UNIT_LAST_E
}SMEM_BOBK_UNIT_NAME_ENT;

static SMEM_UNIT_NAME_AND_INDEX_STC bobkUnitNameAndIndexArr[]=
{
    /* units that exists in bobcat2 */
    {STR(UNIT_MG),                         BOBK_UNIT_MG_E                    },
    {STR(UNIT_IPLR1),                      BOBK_UNIT_IPLR1_E                 },
    {STR(UNIT_TTI),                        BOBK_UNIT_TTI_E                   },
    {STR(UNIT_L2I),                        BOBK_UNIT_L2I_E                   },
    {STR(UNIT_IPVX),                       BOBK_UNIT_IPVX_E                  },
    {STR(UNIT_BM),                         BOBK_UNIT_BM_E                    },
    {STR(UNIT_BMA),                        BOBK_UNIT_BMA_E                   },
    {STR(UNIT_FDB),                        BOBK_UNIT_FDB_E                   },
    {STR(UNIT_MPPM),                       BOBK_UNIT_MPPM_E                  },
    {STR(UNIT_EPLR),                       BOBK_UNIT_EPLR_E                  },
    {STR(UNIT_CNC),                        BOBK_UNIT_CNC_0_E                 },
    {STR(UNIT_GOP),                        BOBK_UNIT_GOP_E                   },
    {STR(UNIT_XG_PORT_MIB),                BOBK_UNIT_XG_PORT_MIB_E           },
    {STR(UNIT_SERDES),                     BOBK_UNIT_SERDES_E                },
    {STR(UNIT_EQ),                         BOBK_UNIT_EQ_0_E                  },
    {STR(UNIT_IPCL),                       BOBK_UNIT_IPCL_E                  },
    {STR(UNIT_IPLR),                       BOBK_UNIT_IPLR0_E                 },
    {STR(UNIT_MLL),                        BOBK_UNIT_MLL_E                   },
    {STR(UNIT_EPCL),                       BOBK_UNIT_EPCL_E                  },
    {STR(UNIT_HA),                         BOBK_UNIT_HA_E                    },
    {STR(UNIT_RX_DMA),                     BOBK_UNIT_RX_DMA_E                },
    {STR(UNIT_TX_DMA),                     BOBK_UNIT_TX_DMA_E                },

    {STR(UNIT_IOAM),                       BOBK_UNIT_IOAM_E          },
    {STR(UNIT_EOAM),                       BOBK_UNIT_EOAM_E          },
    {STR(UNIT_LPM),                        BOBK_UNIT_LPM_E           },
    {STR(UNIT_EQ_1),                       BOBK_UNIT_EQ_1_E          },
    {STR(UNIT_FDB_TABLE_0),                BOBK_UNIT_FDB_TABLE_1_E   },
    {STR(UNIT_FDB_TABLE_1),                BOBK_UNIT_FDB_TABLE_1_E   },
    {STR(UNIT_EGF_EFT),                    BOBK_UNIT_EGF_EFT_E       },
    {STR(UNIT_EGF_EFT_1),                  BOBK_UNIT_EGF_EFT_1_E     },
    {STR(UNIT_EGF_QAG),                    BOBK_UNIT_EGF_QAG_E       },
    {STR(UNIT_EGF_QAG_1),                  BOBK_UNIT_EGF_QAG_1_E     },
    {STR(UNIT_EGF_SHT),                    BOBK_UNIT_EGF_SHT_E       },
    {STR(UNIT_EGF_SHT_1),                  BOBK_UNIT_EGF_SHT_1_E     },
    {STR(UNIT_EGF_SHT_2),                  BOBK_UNIT_EGF_SHT_2_E     },
    {STR(UNIT_EGF_SHT_3),                  BOBK_UNIT_EGF_SHT_3_E     },
    {STR(UNIT_EGF_SHT_4),                  BOBK_UNIT_EGF_SHT_4_E     },
    {STR(UNIT_EGF_SHT_5),                  BOBK_UNIT_EGF_SHT_5_E     },
    {STR(UNIT_EGF_SHT_6),                  BOBK_UNIT_EGF_SHT_6_E     },
    {STR(UNIT_EGF_SHT_7),                  BOBK_UNIT_EGF_SHT_7_E     },
    {STR(UNIT_EGF_SHT_8),                  BOBK_UNIT_EGF_SHT_8_E     },
    {STR(UNIT_EGF_SHT_9),                  BOBK_UNIT_EGF_SHT_9_E     },
    {STR(UNIT_EGF_SHT_10),                 BOBK_UNIT_EGF_SHT_10_E    },
    {STR(UNIT_EGF_SHT_11),                 BOBK_UNIT_EGF_SHT_11_E    },
    {STR(UNIT_EGF_SHT_12),                 BOBK_UNIT_EGF_SHT_12_E    },
    {STR(UNIT_TCAM),                       BOBK_UNIT_TCAM_E          },
    {STR(UNIT_TM_HOSTING),                 BOBK_UNIT_TM_HOSTING_E    },
    {STR(UNIT_ERMRK),                      BOBK_UNIT_ERMRK_E         },
    {STR(UNIT_CNC_1),                      BOBK_UNIT_CNC_1_E         },

    {STR(UNIT_TMDROP),                     BOBK_UNIT_TMDROP_E        },

    {STR(UNIT_TXQ_QUEUE),                  BOBK_UNIT_TXQ_QUEUE_E     },
    {STR(UNIT_TXQ_LL),                     BOBK_UNIT_TXQ_LL_E        },
    {STR(UNIT_TXQ_DQ),                     BOBK_UNIT_TXQ_DQ_E        },
    {STR(UNIT_CPFC),                       BOBK_UNIT_CPFC_E          },
    {STR(UNIT_TXQ_QCN),                    BOBK_UNIT_TXQ_QCN_E       },

    {STR(UNIT_TX_FIFO),                    BOBK_UNIT_TX_FIFO_E       },
    {STR(UNIT_ETH_TX_FIFO),                BOBK_UNIT_ETH_TX_FIFO_E   },

    {STR(UNIT_TM_FCU),                     BOBK_UNIT_TM_FCU_E         },
    {STR(UNIT_TM_INGRESS_GLUE),            BOBK_UNIT_TM_INGRESS_GLUE_E},
    {STR(UNIT_TM_EGRESS_GLUE),             BOBK_UNIT_TM_EGRESS_GLUE_E },
    {STR(UNIT_TMQMAP),                     BOBK_UNIT_TMQMAP_E         },
    /* units that are new in bobk */
    {STR(UNIT_RX_DMA_1),                   BOBK_UNIT_RX_DMA_1_E       },
    {STR(UNIT_TX_DMA_1),                   BOBK_UNIT_TX_DMA_1_E       },
    {STR(UNIT_TX_FIFO_1),                  BOBK_UNIT_TX_FIFO_1_E      },
    {STR(UNIT_ETH_TX_FIFO_1),              BOBK_UNIT_ETH_TX_FIFO_1_E  },
    {STR(UNIT_RX_DMA_GLUE),                BOBK_UNIT_RX_DMA_GLUE_E    },
    {STR(UNIT_TX_DMA_GLUE),                BOBK_UNIT_TX_DMA_GLUE_E    },
    {STR(UNIT_TAI),                        BOBK_UNIT_TAI_E            },
    {STR(UNIT_GOP_SMI_0),                  BOBK_UNIT_GOP_SMI_0_E      },
    {STR(UNIT_GOP_SMI_1),                  BOBK_UNIT_GOP_SMI_1_E      },
    {STR(UNIT_GOP_SMI_2),                  BOBK_UNIT_GOP_SMI_2_E      },
    {STR(UNIT_GOP_SMI_3),                  BOBK_UNIT_GOP_SMI_3_E      },
    {STR(UNIT_GOP_LED_0),                  BOBK_UNIT_GOP_LED_0_E      },
    {STR(UNIT_GOP_LED_1),                  BOBK_UNIT_GOP_LED_1_E      },
    {STR(UNIT_GOP_LED_2),                  BOBK_UNIT_GOP_LED_2_E      },
    {STR(UNIT_GOP_LED_3),                  BOBK_UNIT_GOP_LED_3_E      },
    {STR(UNIT_GOP_LED_4),                  BOBK_UNIT_GOP_LED_4_E      },

    /* units that are new in bobk-aldrin */
    {STR(UNIT_RX_DMA_2),                   BOBK_UNIT_RX_DMA_2_E       },
    {STR(UNIT_TX_DMA_2),                   BOBK_UNIT_TX_DMA_2_E       },
    {STR(UNIT_TX_FIFO_2),                  BOBK_UNIT_TX_FIFO_2_E      },
    {STR(UNIT_POE),                        BOBK_UNIT_POE_E            },

    {NULL ,                                SMAIN_NOT_VALID_CNS        }
};
/* the addresses of the units that the bobk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   bobkUsedUnitsAddressesArray[BOBK_UNIT_LAST_E]=
{
    /* units that exists in bobcat2 */
     {BOBCAT2_MG_UNIT_BASE_ADDR_CNS                  /*BOBK_UNIT_MG_E,           */               ,0}
    ,{BOBCAT2_IPLR1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_IPLR1_E,        */               ,0}
    ,{BOBCAT2_TTI_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_TTI_E,          */               ,0}
    ,{BOBCAT2_L2I_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_L2I_E,          */               ,0}
    ,{BOBCAT2_IPVX_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IPVX_E,         */               ,0}
    ,{BOBCAT2_BM_UNIT_BASE_ADDR_CNS                  /*BOBK_UNIT_BM_E,           */               ,0}
    ,{BOBCAT2_BMA_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_BMA_E,          */               ,0}
    ,{BOBCAT2_FDB_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_FDB_E,          */               ,0}
    ,{BOBCAT2_EPLR_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EPLR_E,         */               ,0}
    ,{BOBCAT2_MPPM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_MPPM_E,         */               ,0}
    ,{BOBCAT2_GOP_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_GOP_E,          */               ,0}
    ,{BOBCAT2_MIB_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_XG_PORT_MIB_E,  */               ,0}
    ,{BOBCAT2_SERDES_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_SERDES_E,       */               ,0}
    ,{BOBCAT2_RX_DMA_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_RX_DMA_E,       */               ,0}
    ,{BOBCAT2_TX_DMA_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_DMA_E,       */               ,0}
    ,{BOBCAT2_TXQ_QUEUE_UNIT_BASE_ADDR_CNS           /*BOBK_UNIT_TXQ_QUEUE_E,    */               ,0}
    ,{BOBCAT2_TXQ_LL_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TXQ_LL_E,       */               ,0}
    ,{BOBCAT2_TXQ_DQ_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TXQ_DQ_E,       */               ,0}
    ,{BOBCAT2_CPFC_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_CPFC_E,         */               ,0}
    ,{BOBCAT2_MLL_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_MLL_E,          */               ,0}
    ,{BOBCAT2_IOAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IOAM_E,         */               ,0}
    ,{BOBCAT2_EOAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EOAM_E,         */               ,0}
    ,{BOBCAT2_LPM_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_LPM_E,          */               ,0}
    ,{BOBCAT2_EQ_UNIT_BASE_ADDR_CNS(0)               /*BOBK_UNIT_EQ_0_E,         */               ,0}
    ,{BOBCAT2_EQ_UNIT_BASE_ADDR_CNS(1)               /*BOBK_UNIT_EQ_1_E,         */               ,0}
    ,{BOBCAT2_FDB_TABLE_BASE_ADDR_CNS(0)             /*BOBK_UNIT_FDB_TABLE_0_E,  */               ,0}
    ,{BOBCAT2_FDB_TABLE_BASE_ADDR_CNS(1)             /*BOBK_UNIT_FDB_TABLE_1_E,  */               ,0}
    ,{BOBCAT2_EGF_EFT_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_EFT_E,    */                 ,0}
    ,{BOBCAT2_EGF_EFT_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_EFT_1_E,    */               ,0}
    ,{BOBCAT2_EGF_QAG_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_QAG_E,    */                 ,0}
    ,{BOBCAT2_EGF_QAG_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_QAG_1_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_SHT_E,    */                 ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_SHT_1_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(2 )         /*BOBK_UNIT_EGF_SHT_2_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(3 )         /*BOBK_UNIT_EGF_SHT_3_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(4 )         /*BOBK_UNIT_EGF_SHT_4_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(5 )         /*BOBK_UNIT_EGF_SHT_5_E,    *//*!!! EMPTY !!!*/,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(6 )         /*BOBK_UNIT_EGF_SHT_6_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(7 )         /*BOBK_UNIT_EGF_SHT_7_E,    *//*!!! EMPTY !!!*/,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(8 )         /*BOBK_UNIT_EGF_SHT_8_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(9 )         /*BOBK_UNIT_EGF_SHT_9_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(10)         /*BOBK_UNIT_EGF_SHT_10_E,   */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(11)         /*BOBK_UNIT_EGF_SHT_11_E,   */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(12)         /*BOBK_UNIT_EGF_SHT_12_E,   */               ,0}
    ,{BOBCAT2_HA_UNIT_BASE_ADDR_CNS(0)               /*BOBK_UNIT_HA_E,           */               ,0}
    ,{BOBCAT2_IPCL_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IPCL_E,         */               ,0}
    ,{BOBCAT2_IPLR0_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_IPLR0_E,         */              ,0}
    ,{BOBCAT2_EPCL_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EPCL_E,         */               ,0}
    ,{BOBCAT2_TCAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_TCAM_E,         */               ,0}
    ,{BOBCAT2_TM_HOSTING_UNIT_BASE_ADDR_CNS          /*BOBK_UNIT_TM_HOSTING_E,   */               ,0}
    ,{BOBCAT2_ERMRK_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_ERMRK_E,        */               ,0}
    ,{BOBCAT2_CNC_0_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_CNC_0_E,        */               ,0}
    ,{BOBCAT2_CNC_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_CNC_1_E,        */               ,0}
    ,{BOBCAT2_TMDROP_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TMDROP_E,       */               ,0}
    ,{BOBCAT2_TXQ_QCN_UNIT_BASE_ADDR_CNS             /*BOBK_UNIT_TXQ_QCN_E,      */               ,0}
    ,{BOBCAT2_TX_FIFO_UNIT_BASE_ADDR_CNS             /*BOBK_UNIT_TX_FIFO_E,      */               ,0}
    ,{BOBCAT2_ETH_TX_FIFO_UNIT_BASE_ADDR_CNS         /*BOBK_UNIT_ETH_TX_FIFO_E,  */               ,0}
    ,{BOBCAT2_TM_FCU_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TM_FCU_E,       */               ,0}
    ,{BOBCAT2_TM_INGRESS_GLUE_UNIT_BASE_ADDR_CNS     /*BOBK_UNIT_TM_INGRESS_GLUE_E,*/             ,0}
    ,{BOBCAT2_TM_EGRESS_GLUE_UNIT_BASE_ADDR_CNS      /*BOBK_UNIT_TM_EGRESS_GLUE_E, */             ,0}
    ,{BOBCAT2_TMQMAP_GLUE_UNIT_BASE_ADDR_CNS         /*BOBK_UNIT_TMQMAP_E,       */               ,0}
     /* units that are new in bobk */
    ,{BOBK_RX_DMA_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_RX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_DMA_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_TX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_FIFO_1_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_FIFO_1_E,    */               ,0}
    ,{BOBK_ETH_TX_FIFO_1_UNIT_BASE_ADDR_CNS          /*BOBK_UNIT_ETH_TX_FIFO_1_E,*/               ,0}
    ,{BOBK_RX_DMA_GLUE_UNIT_BASE_ADDR_CNS            /*BOBK_UNIT_RX_DMA_GLUE_E,  */               ,0}
    ,{BOBK_TX_DMA_GLUE_UNIT_BASE_ADDR_CNS            /*BOBK_UNIT_TX_DMA_GLUE_E,  */               ,0}
    ,{BOBK_TAI_UNIT_BASE_ADDR_CNS                    /*BOBK_UNIT_TAI_E,          */               ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(0)             /*BOBK_UNIT_GOP_SMI_0_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(1)             /*BOBK_UNIT_GOP_SMI_1_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(2)             /*BOBK_UNIT_GOP_SMI_2_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(3)             /*BOBK_UNIT_GOP_SMI_3_E,      */             ,0}

    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(0)             /*BOBK_UNIT_GOP_LED_0_E,      */             ,0}
    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(1)             /*BOBK_UNIT_GOP_LED_1_E,      */             ,0}
    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(2)             /*BOBK_UNIT_GOP_LED_2_E,      */             ,0}
    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(3)             /*BOBK_UNIT_GOP_LED_3_E,      */             ,0}
    ,{BOBK_GOP_LED_4_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_GOP_LED_4_E,      */             ,0}
    /* units that are new in bobk-aldrin */
    ,{BOBK_RX_DMA_2_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_RX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_DMA_2_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_TX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_FIFO_2_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_FIFO_1_E,    */               ,0}
};

/* the addresses of the units that the bobk uses */
static SMEM_UNIT_BASE_AND_SIZE_STC   bobkAldrinUsedUnitsAddressesArray[BOBK_UNIT_LAST_E]=
{
    /* units that exists in bobcat2 */
     {BOBCAT2_MG_UNIT_BASE_ADDR_CNS                  /*BOBK_UNIT_MG_E,           */               ,0}
    ,{BOBCAT2_IPLR1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_IPLR1_E,        */               ,0}
    ,{BOBCAT2_TTI_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_TTI_E,          */               ,0}
    ,{BOBCAT2_L2I_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_L2I_E,          */               ,0}
    ,{BOBCAT2_IPVX_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IPVX_E,         */               ,0}
    ,{BOBCAT2_BM_UNIT_BASE_ADDR_CNS                  /*BOBK_UNIT_BM_E,           */               ,0}
    ,{BOBCAT2_BMA_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_BMA_E,          */               ,0}
    ,{BOBCAT2_FDB_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_FDB_E,          */               ,0}
    ,{BOBCAT2_EPLR_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EPLR_E,         */               ,0}
    ,{BOBCAT2_MPPM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_MPPM_E,         */               ,0}
    ,{BOBCAT2_GOP_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_GOP_E,          */               ,0}
    ,{BOBCAT2_MIB_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_XG_PORT_MIB_E,  */               ,0}
    ,{BOBCAT2_SERDES_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_SERDES_E,       */               ,0}
    ,{BOBCAT2_RX_DMA_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_RX_DMA_E,       */               ,0}
    ,{BOBCAT2_TX_DMA_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_DMA_E,       */               ,0}
    ,{BOBCAT2_TXQ_QUEUE_UNIT_BASE_ADDR_CNS           /*BOBK_UNIT_TXQ_QUEUE_E,    */               ,0}
    ,{BOBCAT2_TXQ_LL_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TXQ_LL_E,       */               ,0}
    ,{BOBCAT2_TXQ_DQ_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TXQ_DQ_E,       */               ,0}
    ,{BOBCAT2_CPFC_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_CPFC_E,         */               ,0}
    ,{BOBCAT2_MLL_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_MLL_E,          */               ,0}
    ,{BOBCAT2_IOAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IOAM_E,         */               ,0}
    ,{BOBCAT2_EOAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EOAM_E,         */               ,0}
    ,{BOBCAT2_LPM_UNIT_BASE_ADDR_CNS                 /*BOBK_UNIT_LPM_E,          */               ,0}
    ,{BOBCAT2_EQ_UNIT_BASE_ADDR_CNS(0)               /*BOBK_UNIT_EQ_0_E,         */               ,0}
    ,{BOBCAT2_EQ_UNIT_BASE_ADDR_CNS(1)               /*BOBK_UNIT_EQ_1_E,         */               ,0}
    ,{BOBCAT2_FDB_TABLE_BASE_ADDR_CNS(0)             /*BOBK_UNIT_FDB_TABLE_0_E,  */               ,0}
    ,{BOBCAT2_FDB_TABLE_BASE_ADDR_CNS(1)             /*BOBK_UNIT_FDB_TABLE_1_E,  */               ,0}
    ,{BOBCAT2_EGF_EFT_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_EFT_E,    */                 ,0}
    ,{BOBCAT2_EGF_EFT_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_EFT_1_E,    */               ,0}
    ,{BOBCAT2_EGF_QAG_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_QAG_E,    */                 ,0}
    ,{BOBCAT2_EGF_QAG_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_QAG_1_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(0 )         /*BOBK_UNIT_EGF_SHT_E,    */                 ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(1 )         /*BOBK_UNIT_EGF_SHT_1_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(2 )         /*BOBK_UNIT_EGF_SHT_2_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(3 )         /*BOBK_UNIT_EGF_SHT_3_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(4 )         /*BOBK_UNIT_EGF_SHT_4_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(5 )         /*BOBK_UNIT_EGF_SHT_5_E,    *//*!!! EMPTY !!!*/,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(6 )         /*BOBK_UNIT_EGF_SHT_6_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(7 )         /*BOBK_UNIT_EGF_SHT_7_E,    *//*!!! EMPTY !!!*/,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(8 )         /*BOBK_UNIT_EGF_SHT_8_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(9 )         /*BOBK_UNIT_EGF_SHT_9_E,    */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(10)         /*BOBK_UNIT_EGF_SHT_10_E,   */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(11)         /*BOBK_UNIT_EGF_SHT_11_E,   */               ,0}
    ,{BOBCAT2_EGF_SHT_UNIT_BASE_ADDR_CNS(12)         /*BOBK_UNIT_EGF_SHT_12_E,   */               ,0}
    ,{BOBCAT2_HA_UNIT_BASE_ADDR_CNS(0)               /*BOBK_UNIT_HA_E,           */               ,0}
    ,{BOBCAT2_IPCL_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_IPCL_E,         */               ,0}
    ,{BOBCAT2_IPLR0_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_IPLR0_E,         */              ,0}
    ,{BOBCAT2_EPCL_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_EPCL_E,         */               ,0}
    ,{BOBCAT2_TCAM_UNIT_BASE_ADDR_CNS                /*BOBK_UNIT_TCAM_E,         */               ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_TM_HOSTING_E,   */               ,0}
    ,{BOBCAT2_ERMRK_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_ERMRK_E,        */               ,0}
    ,{BOBCAT2_CNC_0_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_CNC_0_E,        */               ,0}
    ,{BOBCAT2_CNC_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_CNC_1_E,        */               ,0}
    ,{BOBCAT2_TMDROP_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TMDROP_E,       */               ,0}
    ,{BOBCAT2_TXQ_QCN_UNIT_BASE_ADDR_CNS             /*BOBK_UNIT_TXQ_QCN_E,      */               ,0}
    ,{BOBCAT2_TX_FIFO_UNIT_BASE_ADDR_CNS             /*BOBK_UNIT_TX_FIFO_E,      */               ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_ETH_TX_FIFO_E,  */               ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_TM_FCU_E,       */               ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_TM_INGRESS_GLUE_E,*/             ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_TM_EGRESS_GLUE_E, */             ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_TMQMAP_E,       */               ,0}
     /* units that are new in bobk */
    ,{BOBK_RX_DMA_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_RX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_DMA_1_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_TX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_FIFO_1_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_FIFO_1_E,    */               ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_ETH_TX_FIFO_1_E,*/               ,0}
    ,{BOBK_RX_DMA_GLUE_UNIT_BASE_ADDR_CNS            /*BOBK_UNIT_RX_DMA_GLUE_E,  */               ,0}
    ,{BOBK_TX_DMA_GLUE_UNIT_BASE_ADDR_CNS            /*BOBK_UNIT_TX_DMA_GLUE_E,  */               ,0}
    ,{BOBK_TAI_UNIT_BASE_ADDR_CNS                    /*BOBK_UNIT_TAI_E,          */               ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(0)             /*BOBK_UNIT_GOP_SMI_0_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(1)             /*BOBK_UNIT_GOP_SMI_1_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(2)             /*BOBK_UNIT_GOP_SMI_2_E,      */             ,0}
    ,{BOBK_GOP_SMI_UNIT_BASE_ADDR_CNS(3)             /*BOBK_UNIT_GOP_SMI_3_E,      */             ,0}

    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(0)             /*BOBK_UNIT_GOP_LED_0_E,      */             ,0}
    ,{BOBK_GOP_LED_UNIT_BASE_ADDR_CNS(1)             /*BOBK_UNIT_GOP_LED_1_E,      */             ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_GOP_LED_2_E,      */             ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_GOP_LED_3_E,      */             ,0}
    ,{SMAIN_NOT_VALID_CNS                            /*BOBK_UNIT_GOP_LED_4_E,      */             ,0}
    /* units that are new in bobk-aldrin */
    ,{BOBK_RX_DMA_2_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_RX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_DMA_2_UNIT_BASE_ADDR_CNS               /*BOBK_UNIT_TX_DMA_1_E,     */               ,0}
    ,{BOBK_TX_FIFO_2_UNIT_BASE_ADDR_CNS              /*BOBK_UNIT_TX_FIFO_1_E,    */               ,0}
    ,{BOBK_POE_UNIT_BASE_ADDR_CNS                    /*BOBK_UNIT_POE_E,          */               ,0}
};

BUILD_STRING_FOR_UNIT_NAME(UNIT_MG);
BUILD_STRING_FOR_UNIT_NAME(UNIT_IPLR1);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TTI);
BUILD_STRING_FOR_UNIT_NAME(UNIT_L2I);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_IPVX);                               */
BUILD_STRING_FOR_UNIT_NAME(UNIT_BM);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_BMA);                                */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_FDB);                                */
BUILD_STRING_FOR_UNIT_NAME(UNIT_MPPM);
BUILD_STRING_FOR_UNIT_NAME(UNIT_EPLR);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_CNC);                                */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP);                                */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_XG_PORT_MIB);                        */
BUILD_STRING_FOR_UNIT_NAME(UNIT_SERDES);
BUILD_STRING_FOR_UNIT_NAME(UNIT_EQ);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_IPCL);                               */
BUILD_STRING_FOR_UNIT_NAME(UNIT_IPLR);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_MLL);                                */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EPCL);                               */
BUILD_STRING_FOR_UNIT_NAME(UNIT_HA);
BUILD_STRING_FOR_UNIT_NAME(UNIT_RX_DMA);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_DMA);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_IOAM);                               */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EOAM);                               */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_LPM);                                */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EQ_1);                               */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_FDB_TABLE_0);                        */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_FDB_TABLE_1);                        */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_EFT);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_EFT_1);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_QAG);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_QAG_1);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_1);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_2);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_3);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_4);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_5);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_6);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_7);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_8);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_9);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_10);                         */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_11);                         */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_12);                         */
BUILD_STRING_FOR_UNIT_NAME(UNIT_TCAM);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TM_HOSTING);                         */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_ERMRK);                              */
BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_CNC_1);                              */
BUILD_STRING_FOR_UNIT_NAME(UNIT_TMDROP);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TXQ_QUEUE);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TXQ_LL);                             */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ);                             */
BUILD_STRING_FOR_UNIT_NAME(UNIT_CPFC);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TXQ_QCN);                            */
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_FIFO);
BUILD_STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TM_FCU);                             */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TM_INGRESS_GLUE);                    */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TM_EGRESS_GLUE);                     */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TMQMAP);                             */
/* units that are new in bobk */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_RX_DMA_1);                           */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_DMA_1);                           */
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_FIFO_1);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_DMA_2);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_FIFO_1);                          */
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_FIFO_2);
BUILD_STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TAI);
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_SMI_0);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_SMI_1);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_SMI_2);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_SMI_3);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_LED_0);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_LED_1);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_LED_2);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_LED_3);                          */
/*BUILD_STRING_FOR_UNIT_NAME(UNIT_GOP_LED_4);                          */
BUILD_STRING_FOR_UNIT_NAME(DFX_EXT_SERVER);
BUILD_STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ);


/**
* @internal smemBobkUnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        /* ports 56..59 , 64..71 */
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PTP */
            BOBK_PORTS_HIGH_PORTS__BY_NUM_BYTES(0x00180800,0,31, 0x4)

            /* Mac-TG Generator */
            ,BOBK_PORTS_HIGH_PORTS__BY_NUM_BYTES(0x00180C00,0,52, 0x4)

            /* GIG */
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x00000000, 0x00000094,0, 0)
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000000A0, 0x000000A4,0, 0)
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000000C0, 0x000000D8,0, 0)

            /* MPCS */
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x00180008, 0x00180014, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x00180030, 0x00180030, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x0018003C, 0x001800C8, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x001800D0, 0x00180120, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x00180128, 0x0018014C, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x0018015C, 0x0018017C, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_NUM_BYTES(0x00180200, 256, 0, 0 )

            /* XPCS IP */
            /* ports in steps of 2 */
            ,BOBK_PORTS_HIGH_PORTS_STEP_2_PORTS__BY_END_ADDR(0x00180400,0x00180424,0,0)
            ,BOBK_PORTS_HIGH_PORTS_STEP_2_PORTS__BY_END_ADDR(0x0018042C,0x0018044C,0,0)
            /* 6 lanes */
            ,BOBK_PORTS_HIGH_PORTS_STEP_2_PORTS__BY_NUM_BYTES(0x00180450,0x0044, 6 , 0x44)

            /* XLG */
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000c0000,0x000c0024, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000c002C,0x000c0060, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000c0068,0x000c0088, 0, 0 )
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x000c0090,0x000c0098, 0, 0 )

            /*FCA*/
            ,BOBK_PORTS_HIGH_PORTS__BY_END_ADDR(0x00180600, 0x00180718,0, 0)

        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }

    if (IS_PORT_0_EXISTS(devObjPtr))
    {
        /* ports 0..47 */
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PTP */
            BOBK_PORTS_LOW_PORTS__BY_NUM_BYTES(0x00180800,0,31, 0x4)

            /* Mac-TG Generator */
            ,BOBK_PORTS_LOW_PORTS__BY_NUM_BYTES(0x00180C00,0,52, 0x4)

            /* GIG */
            ,BOBK_PORTS_LOW_PORTS__BY_END_ADDR(0x00000000, 0x00000094,0, 0)
            ,BOBK_PORTS_LOW_PORTS__BY_END_ADDR(0x000000A0, 0x000000A4,0, 0)
            ,BOBK_PORTS_LOW_PORTS__BY_END_ADDR(0x000000C0, 0x000000D8,0, 0)

            /* MPCS */
            /* no mpcs for ports 0..47 */

            /* XLG */
            /* no XLG for ports 0..47 */

            /*FCA*/
            ,BOBK_PORTS_LOW_PORTS__BY_END_ADDR(0x00180600, 0x00180718,0, 0)

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
        SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

        /*add the tmp unit chunks to the main unit */
        smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
    }
}

/**
* @internal smemBobkAldrinUnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk-aldrin GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkAldrinUnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    #define ALDRIN_NUM_GOP_PORTS_GIG   33
    #define ALDRIN_NUM_GOP_PORTS_XLG   32

    {
        /* ports 0..33 */
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PTP */
             BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180800,0,31, 0x4)

            /* Mac-TG Generator */
            ,BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180C00,0,52, 0x4)

            /* GIG */
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00000000, 0x00000094, 0, 0)
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x000000A0, 0x000000A4, 0, 0)
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x000000C0, 0x000000D8, 0, 0)

            /* MPCS */
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180008, 0x00180014, 0, 0 )
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180030, 0x00180030, 0, 0 )
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x0018003C, 0x001800C8, 0, 0 )
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x001800D0, 0x00180120, 0, 0 )
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180128, 0x0018014C, 0, 0 )
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x0018015C, 0x0018017C, 0, 0 )
            ,BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180200, 256, 0, 0 )

            /* XPCS IP */
            /* ports 0..32 in steps of 2 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180400, 0x00180424)}, FORMULA_SINGLE_PARAMETER(ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018042C, 0x0018044C)}, FORMULA_SINGLE_PARAMETER(ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
            /* 6 lanes */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180450, 0x0044)}, FORMULA_TWO_PARAMETERS(6 , 0x44  , ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}

            /* XLG */
            ,BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0000,0x000c0024, 0, 0 )
            ,BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c002C,0x000c0060, 0, 0 )
            ,BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0068,0x000c0088, 0, 0 )
            ,BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0090,0x000c0098, 0, 0 )

            /*FCA*/
            ,BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180600, 0x00180718,0, 0)

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/
            /* PTP */
             {DUMMY_NAME_PTR_CNS,           0x00180808,         0x00004D94,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180870,         0x000083aa,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180874,         0x00007e5d,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180878,         0x00000040,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }

        /*Giga*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000000,         0x00008be5,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000004,         0x00000003,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00001004,         0x00000083,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00002004,         0x00000103,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00003004,         0x00000183,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00004004,         0x00000203,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00005004,         0x00000283,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00006004,         0x00000303,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00007004,         0x00000383,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00008004,         0x00000403,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00009004,         0x00000483,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000A004,         0x00000503,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000B004,         0x00000583,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000C004,         0x00000603,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000D004,         0x00000683,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000E004,         0x00000703,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x0000F004,         0x00000783,     1,    0x1000                                             }
            ,{DUMMY_NAME_PTR_CNS,            0x00010004,         0x00000803,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00011004,         0x00000883,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00012004,         0x00000903,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00013004,         0x00000983,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00014004,         0x00000A03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00015004,         0x00000A83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00016004,         0x00000B03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00017004,         0x00000B83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00018004,         0x00000C03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00019004,         0x00000C83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001A004,         0x00000D03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001B004,         0x00000D83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001C004,         0x00000E03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001D004,         0x00000E83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001E004,         0x00000F03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001F004,         0x00000F83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00020004,         0x00001003,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008,         0x0000c048,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c,         0x0000b8e8,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c,         0x00000052,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002c,         0x0000000c,      2,    0x18,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030,         0x0000c815,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048,         0x00000300,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094,         0x00000001,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c0,         0x00001004,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c4,         0x00000100,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c8,         0x000001fd,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

            ,{DUMMY_NAME_PTR_CNS,            0x00000090,         0x0000FD9A,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018,         0x00004b4d,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014,         0x000008c4,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d4,         0x000000ff,     ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

        /*XLG*/
                                        /* set 32 ports as XLG */
            ,{DUMMY_NAME_PTR_CNS,            0x000c0008,         0x00000400,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0010,         0x00007BE3,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c001c,         0x00006000,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0030,         0x000007ec,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0080,         0x00001000,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0084,         0x00005210,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000C0000,         0x00000200,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000C0004,         0x000002F9,     ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
        /*FCA*/
            ,{DUMMY_NAME_PTR_CNS,            0x00180000+0x600,   0x00000011,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180004+0x600,   0x00002003,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180054+0x600,   0x00000001,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180058+0x600,   0x0000c200,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0018005c+0x600,   0x00000180,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0018006c+0x600,   0x00008808,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180070+0x600,   0x00000001,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180104+0x600,   0x0000ff00,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
        /* MPCS */
            ,{DUMMY_NAME_PTR_CNS,            0x00180014,         0x00003c20,      ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobkUnitLpSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk LpSerdes unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitLpSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    if (IS_PORT_0_EXISTS(devObjPtr))
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* serdes external registers SERDESes for ports 0...47 */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 0x44)} , FORMULA_SINGLE_PARAMETER(12 , 0x1000)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000 + 20 * 0x1000, 0x44 )} , FORMULA_SINGLE_PARAMETER(1 , 0)/*single group*/}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000 + 24 * 0x1000, 0x44 )} , FORMULA_SINGLE_PARAMETER(12 , 0x1000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
    else
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* reduced silicon */
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000 + 20 * 0x1000, 0x44 )} , FORMULA_SINGLE_PARAMETER(1 , 0)/*single group*/}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000 + 24 * 0x1000, 0x44 )} , FORMULA_SINGLE_PARAMETER(12 , 0x1000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemBobkAldrinUnitLpSerdes function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk LpSerdes unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkAldrinUnitLpSerdes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
    {
        /* serdes external registers SERDESes for ports 0...32 */
        BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00000000, 0x44, 0, 0 )
    };

    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

    smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);

}

/**
* @internal smemBobkUnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*/
static void smemBobkUnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        /* steps between each port */
        devObjPtr->xgCountersStepPerPort   = BOBCAT2_MIB_OFFSET_CNS;
        devObjPtr->xgCountersStepPerPort_1 = BOBCAT2_MIB_OFFSET_CNS;
        /* offset ot table xgPortMibCounters_1 */
        devObjPtr->offsetToXgCounters_1 = BOBCAT2_MIB_PORT_56_START_OFFSET_CNS;
        devObjPtr->startPortNumInXgCounters_1 = 56;

        if (0 == IS_PORT_0_EXISTS(devObjPtr))
        {
            devObjPtr->tablesInfo.xgPortMibCounters.commonInfo.firstValidAddress =
                UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB) +
                devObjPtr->offsetToXgCounters_1;
        }


        unitPtr->chunkType = SMEM_UNIT_CHUNK_TYPE_8_MSB_E;

        /* chunks with formulas */
        {
            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                /* will set mib ports 56..59,64..71 */
                BOBK_PORTS_HIGH_MIB_PORTS__BY_END_ADDR(0x00000000 ,0,128/4 , 0x4)
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

            smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
        }

        if (IS_PORT_0_EXISTS(devObjPtr))
        {
            SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
            {
                /* will set mib ports 0..47 */
                BOBK_PORTS_LOW_MIB_PORTS__BY_END_ADDR(0x00000000 ,0,128/4 , 0x4)
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);
            SMEM_UNIT_CHUNKS_STC    tmpUnitChunk;

            smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, &tmpUnitChunk);

            /*add the tmp unit chunks to the main unit */
            smemInitMemCombineUnitChunks(devObjPtr,unitPtr,&tmpUnitChunk);
        }
    }

    {
        SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
        GT_U32  unitIndex;
        /* bobk with many ports */
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB);
        devMemInfoPtr->unitMemArr[unitIndex+ 0].hugeUnitSupportPtr =
        devMemInfoPtr->unitMemArr[unitIndex+ 1].hugeUnitSupportPtr =
            &devMemInfoPtr->unitMemArr[unitIndex];
    }
}

/**
* @internal smemBobkAldrinUnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*/
static void smemBobkAldrinUnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    /* steps between each port */
    devObjPtr->xgCountersStepPerPort   = BOBCAT2_MIB_OFFSET_CNS;
    devObjPtr->xgCountersStepPerPort_1 = 0;/* not valid */
    /* offset of table xgPortMibCounters_1 */
    devObjPtr->offsetToXgCounters_1 = 0;/* not valid */
    devObjPtr->startPortNumInXgCounters_1 = 0;/* not valid */

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* base of 0x00000000 */
            /* ports 0..32 - each port has a total of 30 counters: two 64-bit counters and 28 32-bit counters */
              {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000 ,0)}, FORMULA_TWO_PARAMETERS(128/4, 0x4, 33, BOBCAT2_MIB_OFFSET_CNS)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemBobkUnitMppm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk MPPM unit
*
* @param[in] devObjPtr                - pointer to common device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitMppm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {   /*bobcat2*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001F0, 0x000001F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000378)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000510, 0x00000518)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000614)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x00000624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000804)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000810, 0x00000814)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000910, 0x00000914)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E30, 0x00000E34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x00001054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x00001154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x0000200C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000202C, 0x00002030)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x00002084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020B0, 0x000020B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002120, 0x00002124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002170, 0x00002174)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021B0, 0x000021B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021C0, 0x000021C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003230, 0x00003234)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003420, 0x00003420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003480, 0x00003480)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000034E0, 0x000034E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003540, 0x00003540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003580, 0x00003580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000035E0, 0x000035E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003640, 0x00003640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}


static void smemBobkAldrinUnitMppm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {   /*Aldrin*/
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000120, 0x00000124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x00000154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001F0, 0x000001F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000378)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000400)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000510, 0x00000518)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000600)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000614)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000620, 0x00000624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000804)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000810, 0x00000814)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000900, 0x00000904)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000910, 0x00000914)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000A00, 0x00000A04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000B04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000C00, 0x00000C04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000D00, 0x00000D04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000E04)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E30, 0x00000E34)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001050, 0x00001054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001150, 0x00001154)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001200, 0x00001204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000202C, 0x00002034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002080, 0x00002088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020B0, 0x000020B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002120, 0x00002134)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002170, 0x00002184)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021B0, 0x000021B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000021C0, 0x000021C4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003030, 0x00003034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003200, 0x00003204)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003230, 0x00003234)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003400, 0x00003408)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003420, 0x00003420)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003480, 0x00003480)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000034E0, 0x000034E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003540, 0x00003540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003580, 0x00003580)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000035E0, 0x000035E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003640, 0x00003640)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004000)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobkUnitRxDma function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk RX DMA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitRxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000040)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000084, 0x00000088)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000A0, 0x000000A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C4, 0x000000D0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000E4, 0x00000160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000174, 0x00000180)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000198, 0x00000294)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002F0, 0x000002FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000320, 0x0000032C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000350, 0x00000350)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000374, 0x00000374)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000398, 0x000003AC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003C8, 0x000003D4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003E8, 0x000003F4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000408, 0x00000414)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000428, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000043C, 0x00000448)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000450, 0x0000045C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000004D0, 0x000005E0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000630, 0x0000063C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000710, 0x00000714)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000720, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000730, 0x00000738)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000740, 0x00000740)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000750, 0x00000750)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000804, 0x00000924)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000950, 0x00000A70)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000B00, 0x00000C20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000E00, 0x00000F28)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000120C, 0x0000132C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001528, 0x0000154C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001704, 0x0000170C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001720, 0x00001744)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001790, 0x00001790)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800, 0x00001800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001820, 0x00001820)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001830, 0x00001834)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001840, 0x00001840)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001850, 0x0000185C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001864, 0x00001868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001870, 0x00001870)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001C20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001E00, 0x00001F20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000020F0, 0x000020F0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x00002220)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002400, 0x00002520)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002A00, 0x00002B58)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002B60, 0x00002B60)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003620)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003A00, 0x00003B20)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004080, 0x00004084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000040A0, 0x000040A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000040C0, 0x000040C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005000)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobkUnitRxDmaGlue function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk RX DMA Glue unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitRxDmaGlue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000098)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000160, 0x00000160)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}


/**
* @internal smemAldrinUnitRxDmaGlue function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk RX DMA Glue unit (MC-Rx-DMA)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemAldrinUnitRxDmaGlue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000031C)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}


/**
* @internal smemBobkAldrinUnitTxQDq function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk Aldrin TxQ-DQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkAldrinUnitTxQDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
        {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC   (0x00000000, 0x00000120)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000208, 0x00000248)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000260, 0x00000268)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000002C0, 0x000003DC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000400, 0x0000051C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000600, 0x00000604)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000610, 0x00000650)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000660, 0x00000660)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000670, 0x00000670)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000680, 0x00000680)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000690, 0x00000690)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000800, 0x00000800)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000808, 0x00000808)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000900, 0x00000904)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000920, 0x00000924)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000A00, 0x00000A00)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000B00, 0x00000C24)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001000, 0x00001000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001008, 0x00001010)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001040, 0x0000115C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001240, 0x0000133C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001380, 0x0000149C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001500, 0x00001500)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001510, 0x0000162C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001650, 0x000017CC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000017F0, 0x000017F0)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001800, 0x00001800)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00001904, 0x00001A28)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00002000, 2304), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(168,32),SMEM_BIND_TABLE_MAC(Scheduler_State_Variable)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00004000, 0x0000400C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00004100, 0x0000421C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00004300, 0x0000441C)}
         /*Priority Token Bucket Configuration*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00006000, 4608), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(488,64)}
         /*Port Token Bucket Configuration*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000A000, 576), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(61,8)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C000, 0x0000C000)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C008, 0x0000C044)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C080, 0x0000C09C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C100, 0x0000C13C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C180, 0x0000C37C)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000C500, 0x0000C8FC)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000CA00, 0x0000CA08)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000D000, 0x0000D004)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000D010, 0x0000D014)}
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x0000D040, 0x0000D0BC)}
         /*Egress STC Table*/
        ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0000D800, 1152), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(80,16),SMEM_BIND_TABLE_MAC(egressStc)}
    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

    smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
}

/**
* @internal smemBobkAldrinUnitPoe function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk Aldrin POE unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkAldrinUnitPoe
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    SMEM_CHUNK_BASIC_STC  chunksMem[]=
    {
         {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x0000000C)}
    };
    GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

    smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
}



/**
* @internal smemBobkUnitTxDmaGlue function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk TX DMA Glue unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitTxDmaGlue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000098)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}



/**
* @internal smemBobkUnitTai function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk TAI unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitTai
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000100)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobkUnitMg function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the Bobk MG unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitMg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000008, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000010, 0x00000044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000004C, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000084)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000090, 0x00000148)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000150, 0x0000017C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000190, 0x00000198)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000001A0, 0x00000288)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000290, 0x00000298)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002A0, 0x000002A8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002B0, 0x000002B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002C0, 0x000002C8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000002D0, 0x00000324)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000330, 0x00000330)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000338, 0x00000338)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000340, 0x00000344)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000360, 24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000380, 0x00000388)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000390, 0x00000390)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003A0, 0x000003C0)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000003F0, 0x000003FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000540)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000604)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000610, 0x00000654)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000670, 0x000006B4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000830, 0x00000830)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000b64, 0x00000b64)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000b78, 0x00000b7c)}
            /* sdma */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002600, 0x00002684)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000026C0, 0x000026DC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002700, 0x00002708)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002710, 0x00002718)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002720, 0x00002728)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002730, 0x00002738)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002740, 0x00002748)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002750, 0x00002758)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002760, 0x00002768)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002770, 0x00002778)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002780, 0x00002780)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002800, 0x00002800)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000280C, 0x00002868)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002870, 0x000028F4)}
            /*xsmi*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00030000, 8192)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00032000, 8192)}
            /* conf processor */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 131072)}
            /*TWSI*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00080000, 0x0008000C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0008001C, 0x0008001C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00091000, 0x00091010)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0009101C, 0x0009101C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000910C0, 0x000910E0)}

            /* dummy memory for IPC between CPSS and WM , see smemChtActiveWriteIpcCpssToWm(...) */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000FFF00, 0x000FFFFC)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}


/**
* @internal smemBobkUnitTxDma function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk TX DMA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitTxDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x00000014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000020, 0x0000011C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x00000248)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000500, 0x00000504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001020, 0x00001024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x0000393C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004000, 0x00004158)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00004160, 0x00004160)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005000, 0x00005014)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0000501C, 0x00005078)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005080, 0x0000508C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005100, 0x00005104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005120, 0x00005124)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005200, 0x00005324)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005500, 0x00005624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005700, 0x00005824)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00005900, 0x00005A24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006100, 0x00006104)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00006300, 0x00006304)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007000, 0x00007004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007100, 0x00007224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007300, 0x00007300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007500, 0x00007504)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00007700, 0x00007700)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

/**
* @internal smemBobkUnitTxFifo function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk TX Fifo unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000012C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000400, 0x00000434)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000440, 0x00000444)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000508, 0x0000050C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000518, 0x0000051C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000600, 0x00000724)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000800, 0x00000958)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000960, 0x00000960)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001000, 0x00001024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001100, 0x00001224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001300, 0x00001424)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001500, 0x00001624)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001700, 0x00001824)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900, 0x00001A24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001B00, 0x00001C24)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003000, 0x00003004)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003100, 0x00003224)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003300, 0x00003300)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00010000, 1648)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00012000, 8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00014000, 0x00014000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00015A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00017A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00020000, 2256)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00022000, 4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00024000, 0x00024000)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00025A00, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027000, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027200, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027400, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027600, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027800, 296)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00027A00, 296)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);
        smemInitMemChunk(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}



/**
* @internal smemBobkUnitBm function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk BM unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitBm
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000000, 0x00000008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000010, 0x00000018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000020, 0x00000020)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000030, 0x00000034)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000050, 0x00000060)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000200, 0x00000228)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000300, 0x00000314)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000400, 0x0000040C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000420, 0x0000042C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000440, 0x0000044C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000460, 0x0000046C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000480, 0x0000048C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004A0, 0x000004A4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004C0, 0x000004C8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004D0, 0x000004D8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x000004E0, 0x000004E4)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC  (0x00000500, 0x0000074C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00120000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00200000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00220000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 65536)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobkUnitBma function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk BMA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitBma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000000, 65536)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00019000, 0x0001900C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00019014, 0x00019018)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00019020, 0x00019044)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0001904C, 0x00019058)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00019070, 0x00019070)}
            /*Virtual => Physical source port mapping*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001A000, 1024),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(8 , 4),SMEM_BIND_TABLE_MAC(bmaPortMapping)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001A400, 1024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001A800, 1024)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x0001AC00, 1024)}
        };

        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}

/**
* @internal smemBobkPolicerTablesSupport function
* @endinternal
*
* @brief   manage Policer tables:
*         1. iplr 0 - all tables
*         2. iplr 1 - none
*         3. eplr  - other names for meter+counting
* @param[in] numOfChunks              - pointer to device object.
*                                      unitPtr - pointer to the unit chunk
* @param[in] plrUnit                  - PLR unit
*/
void smemBobkPolicerTablesSupport
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  numOfChunks,
    INOUT SMEM_CHUNK_BASIC_STC  chunksMem[],
    IN SMEM_SIP5_PP_PLR_UNIT_ENT   plrUnit
)
{
    GT_BIT  eplr  = (plrUnit == SMEM_SIP5_PP_PLR_UNIT_EPLR_E) ? 1 : 0;
    GT_BIT  iplr1 = (plrUnit == SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E) ? 1 : 0;
    GT_U32  ii;
    GT_U32  numOfMeters = eplr ?
            devObjPtr->policerSupport.eplrTableSize :
            devObjPtr->policerSupport.iplrTableSize;
    GT_BIT  updateTableSizeAccordingToMeters;

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) && eplr)
    {
        /* the EPLR like IPLR1 is sharing with IPLR0 */
        iplr1 = 1;
    }

    for(ii = 0 ; ii < numOfChunks ; ii ++)
    {
        updateTableSizeAccordingToMeters = 0;
        if (iplr1)
        {
             /* in sip 6 each PLR unit has it's own instance of the metering configuration table */
            switch(chunksMem[ii].tableOffsetInBytes)
            {
                case FIELD_OFFSET_IN_STC_MAC(policer1Config,SKERNEL_TABLES_INFO_STC):
                    updateTableSizeAccordingToMeters = 1;
                    break;
                default:
                    /* make sure that table are not bound to PLR1 (only to iplr0) */
                    chunksMem[ii].tableOffsetValid = 0;
                    chunksMem[ii].tableOffsetInBytes = 0;
                    break;
             }
        }
        else if(eplr)
        {
            switch(chunksMem[ii].tableOffsetInBytes)
            {
                case FIELD_OFFSET_IN_STC_MAC(policer,SKERNEL_TABLES_INFO_STC):
                    chunksMem[ii].tableOffsetInBytes =
                        FIELD_OFFSET_IN_STC_MAC(egressPolicerMeters,SKERNEL_TABLES_INFO_STC);
                    updateTableSizeAccordingToMeters = 1;
                    break;
                case FIELD_OFFSET_IN_STC_MAC(policerCounters,SKERNEL_TABLES_INFO_STC):
                    chunksMem[ii].tableOffsetInBytes =
                        FIELD_OFFSET_IN_STC_MAC(egressPolicerCounters,SKERNEL_TABLES_INFO_STC);
                    updateTableSizeAccordingToMeters = 1;
                    break;
                case FIELD_OFFSET_IN_STC_MAC(policerConfig,SKERNEL_TABLES_INFO_STC):
                    chunksMem[ii].tableOffsetInBytes =
                        FIELD_OFFSET_IN_STC_MAC(egressPolicerConfig,SKERNEL_TABLES_INFO_STC);
                    updateTableSizeAccordingToMeters = 1;
                    break;
                default:
                    /* make sure that table are not bound to EPLR (only to iplr0) */
                    chunksMem[ii].tableOffsetValid = 0;
                    chunksMem[ii].tableOffsetInBytes = 0;
                    break;
            }
        }
        else /* iplr0 */
        {
            switch(chunksMem[ii].tableOffsetInBytes)
            {
                case FIELD_OFFSET_IN_STC_MAC(policer,SKERNEL_TABLES_INFO_STC):
                case FIELD_OFFSET_IN_STC_MAC(policerCounters,SKERNEL_TABLES_INFO_STC):
                case FIELD_OFFSET_IN_STC_MAC(policerConfig,SKERNEL_TABLES_INFO_STC):
                    updateTableSizeAccordingToMeters = 1;
                    break;
                default:
                    break;
            }
        }

        if(updateTableSizeAccordingToMeters)
        {
            chunksMem[ii].numOfRegisters = numOfMeters * (chunksMem[ii].enrtyNumBytesAlignement / 4);
        }
    }
}

/**
* @internal smemBobkUnitPolicerUnify function
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
static void smemBobkUnitPolicerUnify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN SMEM_SIP5_PP_PLR_UNIT_ENT   plrUnit
)
{
    {
        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000, 0x0000003C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000048, 0x00000054)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000060, 0x00000068)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000070, 0x00000070)}
            /*registers -- not table/memory !! -- Policer Table Access Data<%n> */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000074 ,8*4),SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTblAccessData)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000B0, 0x000000B8)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000000C0, 0x000001BC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000200, 0x0000020C)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000220, 0x00000244)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000250, 0x00000294)}
            /*Policer Timer Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000300, 36), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerTimer)}
            /*Policer Descriptor Sample Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000400, 96), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4)}
            /*Policer Management Counters Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000500, 192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(74,16),SMEM_BIND_TABLE_MAC(policerManagementCounters)}
            /*IPFIX wrap around alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00000800, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixWaAlert)}
            /*IPFIX aging alert Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00001000, 1024), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerIpfixAgingAlert)}
            /*registers (not memory) : Port%p and Packet Type Translation Table*/
            /*registers -- not table/memory !! -- Port%p and Packet Type Translation Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001800 , 0x000019FC), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(32,4),SMEM_BIND_TABLE_MAC(policerMeterPointer)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002000, 0x00002008)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00002100, 0x000026FC)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003500, 0x00003500)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003510, 0x00003510)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003520, 0x00003524)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003550, 0x00003554)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003560, 0x00003560)}
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00003604, 0x0000360C)}


            /*Ingress Policer Re-Marking Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00080000, 8192), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(51,8),SMEM_BIND_TABLE_MAC(policerReMarking)}
            /*Hierarchical Policing Table*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00090000, 32768), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(18,4), SMEM_BIND_TABLE_MAC(policerHierarchicalQos)}

            /* NOTE: the number of bytes allocated for the meter/counting tables
              are calculated inside policerTablesSupport(...) so the value we see
              here are overridden !!!*/

            /*Metering Token Bucket Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00100000, 196608), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(211,32),SMEM_BIND_TABLE_MAC(policer)}
            /*Counting Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00140000, 196608), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(229,32),SMEM_BIND_TABLE_MAC(policerCounters)}
            /*Metering Configuration Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00300000, 49152) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(51,8),SMEM_BIND_TABLE_MAC(policerConfig)}
            /*Metering Conformance Level Sign Memory*/
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00400000, 24576) , SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(2,4),SMEM_BIND_TABLE_MAC(policerConformanceLevelSign)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        smemBobkPolicerTablesSupport(devObjPtr,numOfChunks,chunksMem,plrUnit);

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);

        /* eport related */
        {
            GT_U32  index;
            GT_U32  numEPorts = devObjPtr->limitedResources.ePort;

            SMEM_CHUNK_BASIC_STC  chunksMem[]=
            {
                /*e Attributes Table*/
                {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00040000, 65536), SET_SMEM_ENTRY_SIZE_AND_ALIGN_MAC(36,8),SMEM_BIND_TABLE_MAC(policerEPortEVlanTrigger)}
            };
            GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

            smemBobkPolicerTablesSupport(devObjPtr,numOfChunks,chunksMem,plrUnit);

            index = 0;
            chunksMem[index].numOfRegisters = numEPorts * (chunksMem[index].enrtyNumBytesAlignement / 4);

            smemUnitChunkAddBasicChunk(devObjPtr,unitPtr,
                ARRAY_NAME_AND_NUM_ELEMENTS_MAC(chunksMem));
        }


    }
}

static void smemBobkUnitIplr0
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    smemBobkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E);
}

static void smemBobkUnitIplr1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    smemBobkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E);
}

static void smemBobkUnitEplr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    smemBobkUnitPolicerUnify(devObjPtr,unitPtr,SMEM_SIP5_PP_PLR_UNIT_EPLR_E);
}

/**
* @internal smemBobkActiveWritePolicerMeteringBaseAddr function
* @endinternal
*
* @brief   update the base address for metering for the unit.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemBobkActiveWritePolicerMeteringBaseAddr
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  stage = (GT_U32)param;

    *memPtr = *inMemPtr;

    SMEM_LION3_POLICER_METERING_BASE_ADDR____FROM_DB(devObjPtr, stage) =
        (*memPtr) & 0x00FFFFFF;/*24 bits*/
}
/**
* @internal smemBobkActiveWritePolicerCountingBaseAddr function
* @endinternal
*
* @brief   update the base address for counting for the unit.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemBobkActiveWritePolicerCountingBaseAddr
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  stage = (GT_U32)param;

    *memPtr = *inMemPtr;

    SMEM_LION3_POLICER_COUNTING_BASE_ADDR____FROM_DB(devObjPtr, stage) =
        (*memPtr) & 0x0000FFFF;/*16 bits*/
}

/* bind the iplr0 unit to it's active mem */
static void bindUnitIplr0ActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC2_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E),

    {0x0000002C, SMEM_FULL_MASK_CNS, NULL, 0 , smemLion3ActiveWriteHierarchicalPolicerControl,  0},

    /*SMEM_LION3_POLICER_METERING_BASE_ADDR_REG*/
    /*Policer Metering Base Address*/
    {0x00000028, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerMeteringBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E},
    /*SMEM_LION3_POLICER_COUNTING_BASE_ADDR_REG*/
    /*Policer Counting Base Address*/
    {0x00000024, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerCountingBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_IPLR_0_E},

    /* iplr0 policer table */
    {0x00100000, 0xFFFC0000, smemXCatActiveReadIplr0Tables, 0 , smemXCatActiveWriteIplr0Tables,  0},
    /* iplr0 policerCounters table */
    {0x00140000, 0xFFFC0000, smemXCatActiveReadIplr0Tables, 0 , smemXCatActiveWriteIplr0Tables,  0},
    /* iplr0 policer config table */
    {0x00300000, 0xFFFF0000, smemXCatActiveReadIplr0Tables, 0 , smemXCatActiveWriteIplr0Tables,  0},
    /* iplr0 Metering Conformance Level Sign Memory */
    {0x00400000, 0xFFFF0000, smemXCatActiveReadIplr0Tables, 0 , smemXCatActiveWriteIplr0Tables,  0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}
/* bind the iplr1 unit to it's active mem */
static void bindUnitIplr1ActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC2_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E),

    /*SMEM_LION3_POLICER_METERING_BASE_ADDR_REG*/
    /*Policer Metering Base Address*/
    {0x00000028, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerMeteringBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E},
    /*SMEM_LION3_POLICER_COUNTING_BASE_ADDR_REG*/
    /*Policer Counting Base Address*/
    {0x00000024, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerCountingBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_IPLR_1_E},

    /* iplr1 policer table */
    {0x00100000, 0xFFFC0000, smemXCatActiveReadIplr1Tables, 0 , smemXCatActiveWriteIplr1Tables,  0},
    /* iplr1 policerCounters table */
    {0x00140000, 0xFFFC0000, smemXCatActiveReadIplr1Tables, 0 , smemXCatActiveWriteIplr1Tables,  0},
    /* iplr1 policer config table */
    {0x00300000, 0xFFFF0000, smemXCatActiveReadIplr1Tables, 0 , smemXCatActiveWriteIplr1Tables,  0},
    /* iplr1 Metering Conformance Level Sign Memory */
    {0x00400000, 0xFFFF0000, smemXCatActiveReadIplr1Tables, 0 , smemXCatActiveWriteIplr1Tables,  0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)



}
/* bind the eplr unit to it's active mem */
static void bindUnitEplrActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

/*policer*/
    /* add common lines of all policers */
    ACTIVE_MEM_POLICER_BC2_COMMON_MAC(SMEM_SIP5_PP_PLR_UNIT_EPLR_E),

    /*SMEM_LION3_POLICER_METERING_BASE_ADDR_REG*/
    /*Policer Metering Base Address*/
    {0x00000028, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerMeteringBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_EPLR_E},
    /*SMEM_LION3_POLICER_COUNTING_BASE_ADDR_REG*/
    /*Policer Counting Base Address*/
    {0x00000024, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerCountingBaseAddr,  SMEM_SIP5_PP_PLR_UNIT_EPLR_E},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}

/* bind the GOP unit to it's active mem */
/*GOP - copied from BC2 and TAI under GOP removed */
#define BOBCAT2_GOP_64_PORTS_MASK_CNS   0xFFFC0FFF  /* support ports 0..55  in steps of 0x1000 */
#define BOBCAT2_GOP_16_PORTS_MASK_CNS   0xFFFF0FFF  /* support ports 56..71 in steps of 0x1000 */
/* active memory entry for GOP registers */
#define GOP_PORTS_0_71_ACTIVE_MEM_MAC(relativeAddr,readFun,readFunParam,writeFun,writeFunParam)    \
    {relativeAddr, BOBCAT2_GOP_64_PORTS_MASK_CNS, readFun, readFunParam , writeFun, writeFunParam}, \
    {relativeAddr+BOBCAT2_PORT_56_START_OFFSET_CNS, BOBCAT2_GOP_16_PORTS_MASK_CNS, readFun, readFunParam , writeFun, writeFunParam}
#if 0  /* for debugging */
void smemChtActiveWrite__dummy (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  port = (address / 0x1000) & 0x7f;

    printf("port %d changing [0x%x] \n",
        port,
        (* memPtr)^(* inMemPtr));

    *memPtr = *inMemPtr ;
}
#endif

/**
* @internal smemBobKActiveWriteMacXlgCtrl4 function
* @endinternal
*
* @brief   The function check if the mac mode of the XLG is actually 1G mode.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemBobKActiveWriteMacXlgCtrl4 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  port;

    /* data to be written */
    *memPtr = *inMemPtr;

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr,address);

    if(port >= devObjPtr->portsNumber)
    {
        skernelFatalError("smemBobKActiveWriteMacXlgCtrl4: port [%d] out of range \n",port);
    }

    if(devObjPtr->portsArr[port].supportMultiState != GT_TRUE)
    {
        return;
    }

    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,port))
    {
        return; /* the 100G MAC used ... don't change it's mode */
    }

    /*<mac_mode_dma_1g>*/
    if(SMEM_U32_GET_FIELD((*inMemPtr), 12, 1))
    {
        /*
            0x1 = GigMAC; Gigabit MAC is active.
            Possible ports speeds are 10/100/1000 Mbps
        */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_GE_STACK_A1_E;
    }
    else
    {
        /* 0x0 = XgMAC; XgMAC; XG MAC is active.
            Possible ports speeds are 10/20/40 Gbps and HGL
        */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_XG_E;
    }

    return;
}


static void bindUnitGopActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

#if 0  /* for debugging */
    /* Port<n>  MAC Control Register1 */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x00000004,NULL, 0 , smemChtActiveWrite__dummy, 0),
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0004,NULL, 0 , smemChtActiveWrite__dummy, 0),
#endif

/*GOP*/
    /* Port<n> Auto-Negotiation Configuration Register */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x0000000C,NULL, 0 , smemChtActiveWriteForceLinkDown, 0),
    /* Port MAC Control Register2 */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x00000008,NULL, 0 , smemChtActiveWriteMacGigControl2, 0),
    /* Port<n> Interrupt Cause Register  */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x00000020,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0),
    /* Tri-Speed Port<n> Interrupt Mask Register */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x00000024,NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0),
    /* Port MAC Control Register0 */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0000,NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0),
    /*Port LPI Control 1 */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000000C4, NULL, 0 , smemBobcat2ActiveWritePortLpiCtrlReg , 1),
    /*Port LPI Status */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000000CC, smemBobcat2ActiveReadPortLpiStatusReg, 0 , smemChtActiveWriteToReadOnlyReg , 0),

    /* XG Port<n> Interrupt Cause Register  */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0014,smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0),
    /* XG Port<n> Interrupt Mask Register */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0018, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0),
    /* stack gig ports - Port<n> Interrupt Cause Register  */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0020, smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0),
    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0024, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0),
    /* stack gig ports - Port<n> Auto-Negotiation Configuration Register */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C000C, NULL, 0 , smemChtActiveWriteForceLinkDown, 0),
    /*Port MAC Control Register3*/
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C001C, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0),
    /* Port MAC Control Register4 : SMEM_XCAT_XG_MAC_CONTROL4_REG */
    GOP_PORTS_0_71_ACTIVE_MEM_MAC(0x000C0084, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0),

/* also in GOP unit */
    /* PTP subunit */
    /* PTP Interrupt Cause Register */
    {0x00000800, 0xFF000FFF,
        smemChtActiveReadIntrCauseReg, 9, smemChtActiveWriteIntrCauseReg, 0},
    /* PTP Interrupt Mask Register */
    {0x00000800 + 0x04, 0xFF000FFF,
        NULL, 0, smemLion3ActiveWriteGopPtpInterruptsMaskReg, 0},

    /* PTP General Control */
    {0x00000800 + 0x08, 0xFF000FFF, NULL, 0, smemLion3ActiveWriteGopPtpGeneralCtrlReg, 0},

    /* PTP TX Timestamp Queue0 reg2 */
    {0x00000800 + 0x14, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},
    /* PTP TX Timestamp Queue1 reg2 */
    {0x00000800 + 0x20, 0xFF000FFF, smemLion3ActiveReadGopPtpTxTsQueueReg2Reg, 0, NULL, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}

/* bind the GOP unit to it's active mem */
static void bindUnitTaiActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
    /* TAI subunit */
    /* Time Counter Function Configuration 0 - Function Trigger */
    {0x10, 0xFFFFFFFF, NULL, 0 , smemLion3ActiveWriteTodFuncConfReg, 0},
    /* time Capture Value 0 Frac Low */
    {0x84, 0xFFFFFFFF, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 0 , smemChtActiveWriteToReadOnlyReg, 0},
    /* time Capture Value 1 Frac Low */
    {0xA0, 0xFFFFFFFF, smemBobcat2ActiveReadTodTimeCaptureValueFracLow, 1 , smemChtActiveWriteToReadOnlyReg, 0},
    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)
}

/* bind the MPPM unit to it's active mem */
static void bindUnitMppmActiveMem(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /*ECC Interrupt Cause Register*/
    {0x00002000, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x00002004, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},
    /* General Interrupts Cause Register */
    {0x000021c0, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0},
    {0x000021c4, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

}


/**
* @internal bindActiveMemoriesOnUnits function
* @endinternal
*
* @brief   bind active memories on units
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void bindActiveMemoriesOnUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO  * devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitIplr0ActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR1)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitIplr1ActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EPLR)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitEplrActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitGopActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitTaiActiveMem(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
    if(IS_VALID_AND_EMPTY_ACTIVE_MEM_ARR_FOR_CHUNK_MAC(currUnitChunkPtr))
    {
        bindUnitMppmActiveMem(devObjPtr,currUnitChunkPtr);
    }
}


/**
* @internal smemBobkUnitHa function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the HA unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void smemBobkUnitHa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
            {DUMMY_NAME_PTR_CNS, 0x00000000, 0x00109010, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000004, 0x03FDD003, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000008, 0x00000004, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000070, 0x00040000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000080, 0xFF000000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000084, 0x00000001, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000008c, 0xFF020000, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000090, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000094, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x00000098, 0xFFFFFFFF, 1,     0x0}
           ,{DUMMY_NAME_PTR_CNS, 0x0000009c, 0xFFFFFFFF, 1,     0x0}
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
           ,{NULL,               0,          0x00000000, 0,     0x0}
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}

/**
* @internal smemBobkSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemBobkSpecificDeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MG)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitMg(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitGop(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitLpSerdes(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitXGPortMib(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitMppm(devObjPtr,currUnitChunkPtr);
    }

    if(devObjPtr->multiDataPath.info[0].dataPathNumOfPorts)
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitRxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxFifo(devObjPtr,currUnitChunkPtr);
        }
    }

    if(devObjPtr->multiDataPath.info[1].dataPathNumOfPorts)
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_1)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitRxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxFifo(devObjPtr,currUnitChunkPtr);
        }
        if(devObjPtr->supportTrafficManager)
        {
            unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_ETH_TX_FIFO_1);
            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
            if(currUnitChunkPtr->numOfChunks == 0)
            {
                smemBobcat2UnitEthTxFifo(devObjPtr,currUnitChunkPtr);
            }
        }
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_GLUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitRxDmaGlue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_GLUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitTxDmaGlue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitTai(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BM);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitBm(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitBma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitIplr0(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR1);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitIplr1(devObjPtr, currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitEplr(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_HA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitHa(devObjPtr,currUnitChunkPtr);
    }

}

/**
* @internal smemBobkAldrinSpecificDeviceUnitAlloc function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
static void smemBobkAldrinSpecificDeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    /* allocate the specific units that we NOT want the bc2_init , lion3_init , lion2_init
       to allocate. */

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MG)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitMg(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitGop(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitLpSerdes(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitXGPortMib(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_MPPM)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitMppm(devObjPtr,currUnitChunkPtr);
    }
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TXQ_DQ)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitTxQDq(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_POE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkAldrinUnitPoe(devObjPtr,currUnitChunkPtr);
    }

    if(devObjPtr->multiDataPath.info[0].dataPathNumOfPorts)
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitRxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxFifo(devObjPtr,currUnitChunkPtr);
        }
    }

    if(devObjPtr->multiDataPath.info[1].dataPathNumOfPorts)
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_1)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitRxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxFifo(devObjPtr,currUnitChunkPtr);
        }
    }

    if(devObjPtr->multiDataPath.info[2].dataPathNumOfPorts)
    {
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_2)];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitRxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_2);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxDma(devObjPtr,currUnitChunkPtr);
        }

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO_2);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            smemBobkUnitTxFifo(devObjPtr,currUnitChunkPtr);
        }
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_GLUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemAldrinUnitRxDmaGlue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_GLUE)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitTxDmaGlue(devObjPtr,currUnitChunkPtr);
    }

    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TAI)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitTai(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BM);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitBm(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_BMA);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitBma(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitIplr0(devObjPtr,currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_IPLR1);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitIplr1(devObjPtr, currUnitChunkPtr);
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_EPLR);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitEplr(devObjPtr,currUnitChunkPtr);
    }


    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_HA)];
    if(currUnitChunkPtr->numOfChunks == 0)
    {
        smemBobkUnitHa(devObjPtr,currUnitChunkPtr);
    }

}

/**
* @internal smemBobkInitInterrupts function
* @endinternal
*
* @brief   Init interrupts for a device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemBobkInitInterrupts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
}

/**
* @internal smemBobkInitPostBobcat2 function
* @endinternal
*
* @brief   init after calling bobcat2 init.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemBobkInitPostBobcat2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  taiBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_TAI);
    GT_U32  ii;
    GT_U32* regsAddrPtr32;
    GT_U32  regsAddrPtr32Size;
    GT_U32  firstAddress;

    if(devObjPtr->supportSingleTai)
    {
        /* fix db of registers of TAI to align to new single base address */

        regsAddrPtr32 = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
        regsAddrPtr32Size = sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0])/sizeof(GT_U32);

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            /* fix addresses in [0][0],[0][1]..[0][4] */
            regsAddrPtr32Size *= 5;/*support TAI 0..4 of the MASTER */
        }
        else if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* fix addresses in [0][0],[0][1] */
            regsAddrPtr32Size *= 2;/*support TAI 0,1 of the MASTER */
        }
        firstAddress = regsAddrPtr32[0];
        for(ii = 0 ; ii < regsAddrPtr32Size; ii++ )
        {
            regsAddrPtr32[ii] = taiBaseAddr + (regsAddrPtr32[ii] - firstAddress);
        }

        /* invalidate all other registers !!! in the TAI  */
        regsAddrPtr32Size = sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI)/sizeof(GT_U32);
        /* continue with the ii .... */
        for(/* ii continue */; ii < regsAddrPtr32Size; ii++ )
        {
            regsAddrPtr32[ii] = SMAIN_NOT_VALID_CNS;
        }

    }

    if(SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_LMS))
    {
        /* remove all LMS registers */
        regsAddrPtr32 = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS);
        regsAddrPtr32Size = sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS)/sizeof(GT_U32);

        for(ii = 0 ; ii < regsAddrPtr32Size; ii++ )
        {
            regsAddrPtr32[ii] = SMAIN_NOT_VALID_CNS;
        }
    }

    devObjPtr->errata.ipLpmActivityStateDoesNotWorks = 0;
}

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC sip5_15_registersDefaultValueArr[] =
{
     {&STRING_FOR_UNIT_NAME(UNIT_TCAM),            0x00504010,         0x00000006,      1,    0x0      }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TTI),            0x00000140,         0x20a6c01b,       1,    0x0      }

    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x0000000c,         0xffff0000,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00000010,         0x00019000,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00000060,         0x00001318,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00000064,         0x000003E8,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00000294,         0x0001ffff,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00002004 ,        0x00014800,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00002008 ,        0x00010040,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00002100 ,        0x0001ffff,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00002300 ,        0x0001ffff,      1,    0x0    )
    ,DEFAULT_REG_FOR_PLR_UNIT_MAC(                0x00003604 ,        0x000000bf,      1,    0x0    )

    ,{NULL,            0,         0x00000000,      0,    0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC sip5_15_gop_registersDefaultValueArr[] =
{
    /* PTP related ports 0-47 */
     {&STRING_FOR_UNIT_NAME(UNIT_GOP),            0x00180878,         0x00000040,      48,   0x1000 }
    /* PTP related ports 56-59 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),            0x00380878,         0x00000040,      4,   0x1000 }
    /* PTP related ports 62 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),            0x00386878,         0x00000040,      1,   0x1000 }
    /* PTP related ports 64-71 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),            0x00388878,         0x00000040,      8,   0x1000 }

    ,{NULL,            0,         0x00000000,      0,    0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC sip5_16_registersDefaultValueArr[] =
{
     {&STRING_FOR_UNIT_NAME(UNIT_TCAM),            0x00504010,         0x00000003,      1,      0x0  }
    ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),          0x00000004,         0x00008801,     33,   0x1000  }
    /*TTI.TTI_Internal.MetalFixRegister*/
    ,{&STRING_FOR_UNIT_NAME(UNIT_TTI),             0x000013F8,         0x000ffff,      1,      0x0  }
    ,{&STRING_FOR_UNIT_NAME(UNIT_L2I),             0x00000024,         0x00000000,     1,      0x1000}
    ,{&STRING_FOR_UNIT_NAME(UNIT_L2I),             0x00000000,         0x02806004,     1,      0x0  }
   /* must be last */
   ,{NULL,            0,         0x00000000,      0,    0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC sip5_15_serdes_registersDefaultValueArr[] =
{
    /* SERDES registers */
    {&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000000,         0x00000800,   12  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000004,         0x88000001,   12  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000008,         0x00000100,   12  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00054000,         0x00000800,    1  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00054004,         0x88000001,    1  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00054008,         0x00000100,    1  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00058000,         0x00000800,   12  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00058004,         0x88000001,   12  ,   0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00058008,         0x00000100,   12  ,   0x1000    }
   /* must be last */
   ,{NULL,            0,         0x00000000,      0,    0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC bobk_fix_registersDefaultValueArr[] =
{
    {&STRING_FOR_UNIT_NAME(UNIT_L2I),           0x00000024,         0x00000000,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_L2I),           0x00000000,         0x02806004,     1,      0x0   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_TAI),           0x00000008,         0x00000006,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_MG),            0x00000058,         0x08088002,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E0,         0x0000E038,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E4,         0x0000E439,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E8,         0x0000E83A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009EC,         0x0000EC3B,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A00,         0x00010040,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A04,         0x00010441,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A08,         0x00010842,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A0C,         0x00010C43,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A10,         0x00011044,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A14,         0x00011445,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A18,         0x00011846,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A1C,         0x00011C47,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A68,         0x0000445A,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000850,         0x828180C8,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000854,         0x86858483,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000858,         0x8A898887,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000085C,         0x8E8D8C8B,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000860,         0x9291908F,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000864,         0x96959493,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000868,         0x9A999897,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000086C,         0x9E9D9C9B,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000870,         0xA2A1A09F,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000874,         0xA6A5A4A3,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000878,         0xAAA9A8A7,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000087C,         0xAEADACAB,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000880,         0xB2B1B0AF,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000884,         0xB6B5B4B3,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000888,         0xBAB9B8B7,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000088C,         0xBEBDBCBB,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000890,         0xC2C1C0BF,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000894,         0xC6C5C4C3,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000898,         0x8180C8C7,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000089C,         0x85848382,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008A0,         0x89888786,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008A4,         0x8D8C8B8A,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008A8,         0x91908F8E,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008AC,         0x95949392,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008B0,         0x99989796,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008B4,         0x9D9C9B9A,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008B8,         0xA1A09F9E,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008BC,         0xA5A4A3A2,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008C0,         0xA9A8A7A6,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008C4,         0xADACABAA,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008C8,         0xB1B0AFAE,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008CC,         0xB5B4B3B2,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008D0,         0xB9B8B7B6,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008D4,         0xBDBCBBBA,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008D8,         0xC1C0BFBE,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008DC,         0xC5C4C3C2,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008E0,         0x80C8C7C6,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008E4,         0x84838281,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008E8,         0x88878685,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008EC,         0x8C8B8A89,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008F0,         0x908F8E8D,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008F4,         0x94939291,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008F8,         0x98979695,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x000008FC,         0x9C9B9A99,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000900,         0xA09F9E9D,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000904,         0xA4A3A2A1,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000908,         0xA8A7A6A5,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000090C,         0xACABAAA9,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000910,         0xB0AFAEAD,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000914,         0xB4B3B2B1,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000918,         0xB8B7B6B5,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000091C,         0xBCBBBAB9,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000920,         0xC0BFBEBD,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000924,         0xC4C3C2C1,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000928,         0xC8C7C6C5,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000092C,         0x83828180,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000930,         0x87868584,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000934,         0x8B8A8988,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000938,         0x8F8E8D8C,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000093C,         0x93929190,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000940,         0x97969594,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000944,         0x9B9A9998,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000948,         0x9F9E9D9C,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x0000094C,         0xA3A2A1A0,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000950,         0xA7A6A5A4,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000954,         0xABAAA9A8,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_ETH_TX_FIFO),   0x00000958,         0xAFAEADAC,     1,      0x0   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_MPPM),          0x00000200,         0x00004006,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_MPPM),          0x00000208,         0x83828180,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_MPPM),          0x0000020C,         0x83828381,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_MPPM),          0x00000210,         0x83828180,     91,     0x4   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),          0x00500000,         0x000FFFFF,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),          0x00500004,         0x00000000,     4,      0x4   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),          0x00500100,         0x00000000,     6,      0x8   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_EQ),            0x0000D010,         0x000001E0,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_EQ),            0x00005030,         0x000001E0,     1,      0x0   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00200090,         0x0000FD9A,     4,      0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00206090,         0x0000FD9A,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00208090,         0x0000FD9A,     8,      0x1000   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C0000,         0x00000200,     4,      0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C8000,         0x00000200,     8,      0x1000   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00201004,         0x00000083,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00202004,         0x00000103,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00203004,         0x00000183,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00208004,         0x00000403,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00209004,         0x00000483,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020A004,         0x00000503,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020B004,         0x00000583,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020C004,         0x00000603,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020D004,         0x00000683,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020E004,         0x00000703,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020F004,         0x00000783,     1,      0x0   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00380808,         0x00004D94,     4,      0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00388808,         0x00004D94,     8,      0x1000   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00206004,         0x00000303,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020600C,         0x0000B8E8,     1,      0x0   }


   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C0084,         0x00005210,     4,      0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C8084,         0x00005210,     8,      0x1000   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C001C,         0x00002000,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C301C,         0x00002000,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C801C,         0x00002000,     8,      0x1000   }

   ,{NULL,                                      0,                  0x00000000,     0,      0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC bobk_caelum_fix_registersDefaultValueArr[] =
{
    {&STRING_FOR_UNIT_NAME(UNIT_MG),            0x00000058,         0x08488002,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000904,         0x00000401,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000908,         0x00000802,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000090C,         0x00000C03,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000910,         0x00001004,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000914,         0x00001405,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000918,         0x00001806,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000091C,         0x00001C07,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000920,         0x00002008,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000924,         0x00002409,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000928,         0x0000280A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000092C,         0x00002C0B,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000930,         0x0000300C,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000934,         0x0000340D,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000938,         0x0000380E,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000093C,         0x00003C0F,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000940,         0x00004010,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000944,         0x00004411,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000948,         0x00004812,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000094C,         0x00004C13,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000950,         0x00005014,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000954,         0x00005415,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000958,         0x00005816,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000095C,         0x00005C17,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000960,         0x00006018,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000964,         0x00006419,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000968,         0x0000681A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000096C,         0x00006C1B,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000970,         0x0000701C,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000974,         0x0000741D,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000978,         0x0000781E,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000097C,         0x00007C1F,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000980,         0x00008020,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000984,         0x00008421,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000988,         0x00008822,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000098C,         0x00008C23,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000990,         0x00009024,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000994,         0x00009425,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000998,         0x00009826,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x0000099C,         0x00009C27,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009A0,         0x0000A028,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009A4,         0x0000A429,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009A8,         0x0000A82A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009AC,         0x0000AC2B,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009B0,         0x0000B02C,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009B4,         0x0000B42D,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009B8,         0x0000B82E,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009BC,         0x0000BC2F,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E0,         0x0000E038,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E4,         0x0000E439,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009E8,         0x0000E83A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x000009EC,         0x0000EC3B,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A68,         0x0000445A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A68,         0x0000F85A,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A00,         0x00010040,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A04,         0x00010441,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A08,         0x00010842,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A0C,         0x00010C43,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A10,         0x00011044,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A14,         0x00011445,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A18,         0x00011846,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),          0x00000A1C,         0x00011C47,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),   0x00000000,         0x0000BE00,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),   0x00000008,         0x000002EE,     19,     0x8   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),   0x0000000C,         0x000003BB,     18,     0x8   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),   0x00000000,         0x0000BE00,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),   0x00000008,         0x000002EE,     19,     0x8   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),   0x0000000C,         0x000003BB,     18,     0x8   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA),        0x0000300C,         0x00000C03,     8,      0x20,   9,  0x100    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO_1),     0x0000071C,         0x0000001B,     1,      0x0   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00000090,         0x0000FD9A,     48,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00200090,         0x0000FD9A,     4,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00206090,         0x0000FD9A,     1,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x00208090,         0x0000FD9A,     8,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0000000C,         0x0000B8E8,     48,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x0020600C,         0x0000B8E8,     1,      0x0   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C0084,         0x00005210,     4,     0x1000   }
   ,{&STRING_FOR_UNIT_NAME(UNIT_GOP),           0x002C8084,         0x00005210,     8,     0x1000   }

   ,{&STRING_FOR_UNIT_NAME(UNIT_L2I),           0x00000024,         0x00000000,     1,      0x1000    }
   ,{&STRING_FOR_UNIT_NAME(UNIT_L2I),           0x00000000,         0x02806004,     1,      0x1000    }

   ,{&STRING_FOR_UNIT_NAME(UNIT_TAI),           0x00000008,         0x00000006,     1,      0x1000    }

   ,{NULL,                                      0,                  0x00000000,     0,      0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC bobk_fix_DfxRegistersDefaultValueArr[] =
{
    {&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000AA080,         0x21F88000,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8204,         0x01EC0061,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8010,         0x00000073,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F82E0,         0x2375E013,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8250,         0x03D77FC0,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F828C,         0x000019E1,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F800C,         0x00440002,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F829C,         0x7F833047,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8200,         0x00F97C00,     1,      0x0     }

   ,{NULL,                                      0,                  0x00000000,     0,      0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC bobk_caelum_fix_DfxRegistersDefaultValueArr[] =
{
    {&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F800C,         0x00440002,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8010,         0x0000087F,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8014,         0x00001113,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8200,         0x00C07E00,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8204,         0x000D0071,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F828C,         0x000019E1,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F829C,         0x7F833007,     1,      0x0     }
   ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),     0x000F8250,         0x03D77FC0,     1,      0x0     }
   ,{NULL,                                      0,                  0x00000000,     0,      0x0      }
};

extern SMEM_REGISTER_DEFAULT_VALUE_STC bobcat2_gop_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC sip5_gop_registersDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC registersDfxDefaultValueArr[];
extern SMEM_REGISTER_DEFAULT_VALUE_STC sip_5_25_registersDfxDefaultValueArr[];

static SMEM_REGISTER_DEFAULT_VALUE_STC bobcat3_registersDefaultValueArr[]=
{
     {&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),     0x00004300,         0x000000ff,     72,         0x4   }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),     0x00010100+72*4,    0x000000ff,     (96-72),    0x4   }
    /*Group_%n_client_enable -- hard wire values for bobcat3 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00500000+4*0,    1<<(0+5) | 1<<0,     1,    0x0         }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00500000+4*1,    1<<(1+5) | 1<<1,     1,    0x0         }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00500000+4*2,    1<<(2+5) | 1<<2,     1,    0x0         }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00500000+4*3,    1<<(3+5) | 1<<3,     1,    0x0         }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00500000+4*4,    1<<(4+5) | 1<<4,     1,    0x0         }
    /* Bobcat3 with 12 active floors (0xc) -- although Cider shows 0x12 !!! (bug in Cider) */
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),       0x00504010,         0x0000000c,      1,    0x0      }

    ,{NULL,                                     0,                  0x00000000,     0,      0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC aldrin_fix_registersDefaultValueArr[] =
{
     {&STRING_FOR_UNIT_NAME(UNIT_MG),           0x00000058,        0x08088002,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000000,        0x00000001,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000904,        0x00000401,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000908,        0x00000802,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000090C,        0x00000C03,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000910,        0x00001004,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000914,        0x00001405,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000918,        0x00001806,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000091C,        0x00001C07,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000920,        0x00002008,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000924,        0x00002409,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000928,        0x0000280A,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000092C,        0x00002C0B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000930,        0x0000300C,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000934,        0x0000340D,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000938,        0x0000380E,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000093C,        0x00003C0F,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000940,        0x00004010,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000944,        0x00004411,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000948,        0x00004812,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000094C,        0x00004C13,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000950,        0x00005014,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000954,        0x00005415,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000958,        0x00005816,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000095C,        0x00005C17,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000960,        0x00006018,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000964,        0x00006419,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000968,        0x0000681A,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000096C,        0x00006C1B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000970,        0x0000701C,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000974,        0x0000741D,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000978,        0x0000781E,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x0000097C,        0x00007C1F,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_CPFC),         0x00000980,        0x00008020,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TAI),          0x00000008,        0x00000006,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_HA),           0x00000424,        0x00110000,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017A4,        0xB7B6B5B4,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017A8,        0xBBBAB9B8,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017AC,        0xBFBEBDBC,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017B0,        0xC3C2C1C0,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017B4,        0xC7C6C5C4,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017B8,        0x83828180,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017BC,        0x87868584,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017C0,        0x8B8A8988,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017C4,        0x8F8E8D8C,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017C8,        0x93929190,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TXQ_DQ),       0x000017CC,        0x97969594,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),  0x00000000,        0x0002FE00,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),  0x00000008,        0x000059AC,     26,     0xC    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),  0x0000000C,        0x00004D66,     26,     0xC    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA_GLUE),  0x00000010,        0x00006B35,     26,     0xC    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),  0x00000000,        0x00017F00,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),  0x00000008,        0x000059AC,     13,     0xC    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),  0x0000000C,        0x00004D66,     12,     0xC    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_GLUE),  0x00000010,        0x00006B35,     12,     0xC    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA),       0x0000370C,        0x00000C03,     4,     0x20    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA),       0x0000380C,        0x00000C03,     8,     0x20    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_2),     0x0000390C,        0x00000C03,     1,     0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_DMA_2),     0x000037CC,        0x00000C03,     1,     0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x000006E0,        0x0000002D,     1,     0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x000006E4,        0x0000001B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x000006E8,        0x00000025,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x000006EC,        0x0000001B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000700,        0x0000002D,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000704,        0x0000001B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000708,        0x00000025,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x0000070C,        0x0000001B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000710,        0x0000002D,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000714,        0x0000001B,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x00000718,        0x00000025,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO),      0x0000071C,        0x0000001B,     1,      0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TX_FIFO_2),    0x00000720,        0x0000001B,     1,     0x0    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00500000,        0x000FFFFF,     1,      0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00500004,        0x00000000,     4,      0x4    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00500100,        0x00000000,     1,      0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00500108,        0x00000000,     1,      0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00500110,        0x00000000,     1,      0x0    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_TCAM),         0x00503000,        0x00000043,     1,      0x0    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_EQ),          0x0000D010,        0x000001E0,     1,     0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_EQ),          0x00005030,        0x000001E0,     1,     0x1000    }

    ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),       0x00000000,        0x000068C0,     1,     0x1000    }

    ,{NULL,                                     0,                 0x00000000,     0,      0x0      }
};

/*extern*/ SMEM_REGISTER_DEFAULT_VALUE_STC aldrin_fix_DfxRegistersDefaultValueArr[] =
{
     {&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F800C,        0x00440002,     1,      0x0     }
    ,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F8074,        0x63414819,     1,      0x0     }
    /*,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F8204,        0x01800000,     1,      0x0     }*/
    /*,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F8010,        0x0000087E,     1,      0x0     }*/
    /*,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F8250,        0x03D77FC0,     1,      0x0     }*/
    /*,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F827C,        0x04600000,     1,      0x0     }*/
    /*,{&STRING_FOR_UNIT_NAME(DFX_EXT_SERVER),    0x000F8280,        0x83003E06,     1,      0x0     }*/
    ,{NULL,                                     0,                 0x00000000,     0,      0x0      }
};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC bobcat3_fix_RegistersDefaults =
    {bobcat3_registersDefaultValueArr, NULL};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC bobk_fix_RegistersDefaults =
    {bobk_fix_registersDefaultValueArr, NULL};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC bobcat2_gop_RegistersDefaults =
    {bobcat2_gop_registersDefaultValueArr, &bobk_fix_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC sip5_gop_RegistersDefaults =
    {sip5_gop_registersDefaultValueArr, &bobcat2_gop_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element6_RegistersDefaults =
    {sip5_15_gop_registersDefaultValueArr, &sip5_gop_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element5_RegistersDefaults =
    {sip5_15_serdes_registersDefaultValueArr, &element6_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  bobk_RegistersDefaults =
    {bobcat2B0_gop_registersDefaultValueArr, &element5_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element3_RegistersDefaults =
    {bobcat2Additional_registersDefaultValueArr, &bobk_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element2_RegistersDefaults =
    {sip5_15_registersDefaultValueArr, &element3_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element1_RegistersDefaults =
    {sip5_10_registersDefaultValueArr,&element2_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   linkListElementsBobk_RegistersDefaults =
    {sip5_registersDefaultValueArr,    &element1_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  bobk_fix_DfxDefault_RegistersDefaults =
    {bobk_fix_DfxRegistersDefaultValueArr , NULL};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  element0_DfxDefault_RegistersDefaults =
    {registersDfxDefaultValueArr , &bobk_fix_DfxDefault_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  sip_5_25_DfxDefault_RegistersDefaults =
    {sip_5_25_registersDfxDefaultValueArr , &bobk_fix_DfxDefault_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_fix_RegistersDefaults =
    {aldrin_fix_registersDefaultValueArr, NULL};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_RegistersDefaults =
    {sip5_16_registersDefaultValueArr, &aldrin_fix_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_element3_RegistersDefaults =
    {bobcat2Additional_registersDefaultValueArr, &aldrin_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_element2_RegistersDefaults =
    {sip5_15_registersDefaultValueArr, &aldrin_element3_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_element1_RegistersDefaults =
    {sip5_10_registersDefaultValueArr,&aldrin_element2_RegistersDefaults};
static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   aldrin_linkListElements_RegistersDefaults =
    {sip5_registersDefaultValueArr,    &aldrin_element1_RegistersDefaults};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  aldrin_fix_DfxDefault_RegistersDefaults =
    {aldrin_fix_DfxRegistersDefaultValueArr , NULL};


/* registers of sip5_10/sip5 that not exists any more ... will be skipped in registers default values */
static SMEM_REGISTER_DEFAULT_VALUE_STC sip5_15_non_exists_registersDefaultValueArr[] =
{
     /*9 MSBits of 'Addr', 23 LSBits of 'Addr',     val,    repeat,    skip*/
     {&STRING_FOR_UNIT_NAME(UNIT_MG),            0x00000390,         0x000fffff,      1,    0x0      }

    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA),         0x00001700,         0x00008021,      1,    0x0      }
    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA),         0x000019b0,         0x0601e114,      1,    0x0      }
    ,{&STRING_FOR_UNIT_NAME(UNIT_RX_DMA),         0x000019b4,         0x003896cb,      1,    0x0      }

    ,{&STRING_FOR_UNIT_NAME(UNIT_BM),             0x00000030 + 2*4,     0x3fff0000,   (3-2),    0x4    }/* only 2 in bobk */
    ,{&STRING_FOR_UNIT_NAME(UNIT_BM),             0x00000420 + 4*4,     0x00031490,   (6-4),    0x4    }/* only 4 in bobk */
    ,{&STRING_FOR_UNIT_NAME(UNIT_BM),             0x00000440 + 4*4,     0x000024c9,   (6-4),    0x4    }/* only 4 in bobk */
    ,{&STRING_FOR_UNIT_NAME(UNIT_BM),             0x000004a0 + 2*4,     0x00080000,   (3-2),    0x4    }/* only 2 in bobk */

     /* must be last */
    ,{NULL,            0,         0x00000000,      0,    0x0      }
};


/* registers of non exists ports 0..47 ... will be skipped in registers default values */
static SMEM_REGISTER_DEFAULT_VALUE_STC bobk_non_exists_port_0_47_registersDefaultValueArr[] =
{
    /* GOP */
    /* remove ports 0..47 */ /* SMAIN_NOT_VALID_CNS - indication for range according to step */            /*start index*//*end index*/   /*range*/       /*step of ports*/
     {&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_0_START_OFFSET_CNS             ,  0 , 47 , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_0_START_OFFSET_CNS + 0x00180000,  0 , 47 , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_0_START_OFFSET_CNS + 0x000c0000,  0 , 47 , BOBCAT2_PORT_OFFSET_CNS , 1}
    /* SERDES */
    ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000000,         0x00000800,   12  ,   0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000004,         0x88000001,   12  ,   0x1000    }
    ,{&STRING_FOR_UNIT_NAME(UNIT_SERDES),         0x00000008,         0x00000100,   12  ,   0x1000    }

     /* must be last */
    ,{NULL,            0,         0x00000000,      0,    0x0      }
};

/* registers of non exists ports 48..55 ... will be skipped in registers default values */
static SMEM_REGISTER_DEFAULT_VALUE_STC bobk_non_exists_port_48_55_registersDefaultValueArr[] =
{
    /* GOP */
    /* remove ports 48 .. 55 */ /* SMAIN_NOT_VALID_CNS - indication for range according to step */            /*start index*//*end index*/   /*range*/       /*step of ports*/
     {&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_48_START_OFFSET_CNS             ,  (48-48) , (55-48) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_48_START_OFFSET_CNS + 0x00180000,  (48-48) , (55-48) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_48_START_OFFSET_CNS + 0x000c0000,  (48-48) , (55-48) , BOBCAT2_PORT_OFFSET_CNS , 1}

    /* remove ports 60 .. 61 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS            ,  (60-56)  , (61-56) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS+ 0x00180000,  (60-56)  , (61-56) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS+ 0x000c0000,  (60-56)  , (61-56) , BOBCAT2_PORT_OFFSET_CNS , 1}

    /* remove ports 63 .. 63 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS            ,  (63-56)  , (63-56) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS+ 0x00180000,  (63-56)  , (63-56) , BOBCAT2_PORT_OFFSET_CNS , 1}
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS , BOBCAT2_PORT_56_START_OFFSET_CNS+ 0x000c0000,  (63-56)  , (63-56) , BOBCAT2_PORT_OFFSET_CNS , 1}

    /* remove XPCS for port 62 */
    /* Address range: 0x10386400 - 0x103865E8 */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS, BOBCAT2_PORT_56_START_OFFSET_CNS + 0x00180400 + (62-56) * BOBCAT2_PORT_OFFSET_CNS, 0, 0, BOBCAT2_XPCS_ADDRESS_RANGE_CNS, 1}

    /* remove XLG MAC for port 62 */
    /* Address range: 0x102C6000 - 0x102C609C */
    ,{&STRING_FOR_UNIT_NAME(UNIT_GOP), SMAIN_NOT_VALID_CNS, BOBCAT2_PORT_56_START_OFFSET_CNS + 0x000c0000 + (62-56) * BOBCAT2_PORT_OFFSET_CNS, 0, 0, BOBCAT2_XLG_MAC_ADDRESS_RANGE_CNS, 1}

     /* must be last */
    ,{NULL,            0,         0x00000000,      0,    0x0      }
};

static SMEM_REGISTER_DEFAULT_VALUE_STC bobk_aldrin_non_exists_registersDefaultValueArr[] =
{
    {&STRING_FOR_UNIT_NAME(UNIT_TMDROP),     0x00000020,         0x006d2240,      1,    0x0     }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TMDROP),     0x00000024,         0x00ff6b64,      1,    0x0     }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TMDROP),     0x00000028,         0x0001d138,      1,    0x0     }
   ,{&STRING_FOR_UNIT_NAME(UNIT_TMDROP),     0x00000038,         0x0000ffff,      1,    0x0     }

     /* must be last */
   ,{NULL,            0,         0x00000000,      0,    0x0      }
};


static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   bobk_non_exists_port_48_55_linkListElements_RegistersDefaults[] =
{
    {bobk_non_exists_port_48_55_registersDefaultValueArr,    NULL}
};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   bobk_non_exists_linkListElements_RegistersDefaults[] =
{
    {sip5_15_non_exists_registersDefaultValueArr,    bobk_non_exists_port_48_55_linkListElements_RegistersDefaults}
};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   bobk_cetus_non_exists_linkListElements_RegistersDefaults[] =
{
    {bobk_non_exists_port_0_47_registersDefaultValueArr , bobk_non_exists_linkListElements_RegistersDefaults}
};

static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC   bobk_aldrin_non_exists_linkListElements_RegistersDefaults[] =
{
    {bobk_aldrin_non_exists_registersDefaultValueArr , bobk_non_exists_linkListElements_RegistersDefaults}
};

/* DMA : special ports mapping {global,local,DP}*/
static SPECIAL_PORT_MAPPING_CNS ALDRIN_DMA_specialPortMappingArr[] = {
    /* --globalPortNumber;IN  --localPortNumber;OUT       --unitIndex;OUT */
     {32/*global DMA port*/,62/*local DMA port*/,2/*DP[]*/},
     {72/*global DMA port*/,72/*local DMA port*/,2/*DP[]*/}

    ,{SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS,SMAIN_NOT_VALID_CNS}  /* must be last */
};

/* local DMAs existing in aldrin DP 0 and 1  */
static GT_U32 aldrin_localDma_dp_0_1_Arr[12] = {56, 57, 58, 59, 64, 65, 66, 67, 68, 69, 70, 71};

/* local DMAs existing in aldrin DP 2 */
static GT_U32 aldrin_localDma_dp_2_Arr[10] = {56, 57, 58, 59, 64, 65, 66, 67, 62, 72};

static SMEM_UNIT_DUPLICATION_INFO_STC BOBK_duplicatedUnits[] =
{
    /* 'pairs' of duplicated units */
    {STR(UNIT_ETH_TX_FIFO)  ,1} ,{STR(UNIT_ETH_TX_FIFO_1) },
    {STR(UNIT_TX_FIFO)      ,1} ,{STR(UNIT_TX_FIFO_1)     },
    {STR(UNIT_TX_DMA)       ,1} ,{STR(UNIT_TX_DMA_1)      },
    {STR(UNIT_RX_DMA)       ,1} ,{STR(UNIT_RX_DMA_1)      },

    {NULL,0} /* must be last */
};

static SMEM_UNIT_DUPLICATION_INFO_STC BOBK_aldrin_duplicatedUnits[] =
{
    {STR(UNIT_RX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_RX_DMA_1)},
        {STR(UNIT_RX_DMA_2)},

    {STR(UNIT_TX_FIFO) ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_FIFO_1)},
        {STR(UNIT_TX_FIFO_2)},

    {STR(UNIT_TX_DMA)  ,2}, /* 2 duplication of this unit */
        {STR(UNIT_TX_DMA_1)},
        {STR(UNIT_TX_DMA_2)},

    {NULL,0} /* must be last */
};

/**
* @internal smemBobkSpecificDeviceMemInitPart2 function
* @endinternal
*
* @brief   specific part 2 of initialization that called from init 1 of
*         Lion3
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkSpecificDeviceMemInitPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* bind active memories to units ... need to be before
        smemBobcat2SpecificDeviceMemInitPart2 ... so bc2 will not override it */
    bindActiveMemoriesOnUnits(devObjPtr);

    /* call bobcat2 */
    smemBobcat2SpecificDeviceMemInitPart2(devObjPtr);
}

/**
* @internal smemBobkActiveWriteRxDmaPIPDropGlobalCounterTrigger function
* @endinternal
*
* @brief   sip5_15 : This trigger needs to be activated before reading the
*         PIP Priority Drop Global Counter
*         (this is a trigger for all priority counters).
*         This is a self cleared bit
* @param[in] devObjPtr                - Device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemBobkActiveWriteRxDmaPIPDropGlobalCounterTrigger (
    IN  SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN  GT_U32                 address,
    IN  GT_U32                 memSize,
    IN  GT_U32                *memPtr,
    IN  GT_UINTPTR             param,
    IN  GT_U32                *inMemPtr
)
{
    GT_U32  unitIndex;

    *memPtr = *inMemPtr;

    /*check bit 0 <Drop_Global_Counter_Trigger> */
    if(SMEM_U32_GET_FIELD((*memPtr),0,1) == 0)
    {
        /* no more to do */
        return;
    }

    unitIndex = smemBobcat2RxDmaUnitIndexFromAddressGet(devObjPtr,address);

    /* copy from the shadow of the 'per DP unit' ALL 3 counters of 64 bits ,
       into the actual counters memory , and reset the shadow */
    smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersCopyToRealCounterAndResetShadow(devObjPtr,unitIndex);

    /* reset bit 0 <Drop_Global_Counter_Trigger> */
    SMEM_U32_SET_FIELD((*memPtr),0,1,0);

    return;
}

/**
* @internal smemBobkInit function
* @endinternal
*
* @brief   Init memory module for a bobk device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_BOOL isBobk = GT_FALSE;

    /* state the supported features */
    SMEM_CHT_IS_SIP5_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_10_GET(devObjPtr) = 1;
    SMEM_CHT_IS_SIP5_15_GET(devObjPtr) = 1;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        devObjPtr->devMemUnitNameAndIndexPtr = bobkUnitNameAndIndexArr;
        devObjPtr->genericUsedUnitsAddressesArray = bobkUsedUnitsAddressesArray;
        devObjPtr->genericNumUnitsAddresses = BOBK_UNIT_LAST_E;

        isBobk = GT_TRUE;
    }

    if(devObjPtr->registersDefaultsPtr == NULL)
    {
        devObjPtr->registersDefaultsPtr = &linkListElementsBobk_RegistersDefaults;
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            bobk_fix_RegistersDefaults.nextPtr = &bobcat3_fix_RegistersDefaults;
        }
        else
        {
            bobk_fix_RegistersDefaults.nextPtr = NULL;
        }
        if(devObjPtr->deviceType == SKERNEL_BOBK_CAELUM)
        {
            bobk_fix_RegistersDefaults.currDefaultValuesArrPtr = bobk_caelum_fix_registersDefaultValueArr;
        }
    }

    if(devObjPtr->registersDfxDefaultsPtr == NULL)
    {
        if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
        {
            devObjPtr->registersDfxDefaultsPtr = &sip_5_25_DfxDefault_RegistersDefaults;
        }
        else
        {
            devObjPtr->registersDfxDefaultsPtr = &element0_DfxDefault_RegistersDefaults;
        }
        if(devObjPtr->deviceType == SKERNEL_BOBK_CAELUM)
        {
            bobk_fix_DfxDefault_RegistersDefaults.currDefaultValuesArrPtr = bobk_caelum_fix_DfxRegistersDefaultValueArr;
        }
    }

    if(devObjPtr->registersDefaultsPtr_unitsDuplications == NULL)
    {
        devObjPtr->registersDefaultsPtr_unitsDuplications = BOBK_duplicatedUnits;
    }


    if (isBobk == GT_TRUE)
    {
        /* state 'data path' structure */
        devObjPtr->multiDataPath.supportMultiDataPath =  1;
        devObjPtr->multiDataPath.maxDp = 2;/* 2 DP units for the device */

        devObjPtr->multiDataPath.info[0].dataPathFirstPort = 0;
        devObjPtr->multiDataPath.info[0].dataPathNumOfPorts = 56;

        devObjPtr->multiDataPath.info[1].dataPathFirstPort = 56;
        devObjPtr->multiDataPath.info[1].dataPathNumOfPorts = 16 + 2;/* the CPU,TM */
        devObjPtr->multiDataPath.info[1].cpuPortDmaNum = 72;
        devObjPtr->multiDataPath.info[1].tmDmaNum = 73;

        devObjPtr->tcam_numBanksForHitNumGranularity = 6;
        SET_IF_ZERO_MAC(devObjPtr->tcamNumOfFloors,6);
    }

    devObjPtr->supportSingleTai = 1;
    SET_IF_ZERO_MAC(devObjPtr->numOfTaiUnits ,1);
    SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfLpmRams , 20);
    SET_IF_ZERO_MAC(devObjPtr->lpmRam.numOfEntriesBetweenRams , 16*1024);

    devObjPtr->supportNat66 = 1;

    if (isBobk == GT_TRUE)
    {
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort,7);
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.stgId,10); /*1K*/
        SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr , 14);/*16K*/

        SET_IF_ZERO_MAC(devObjPtr->limitedResources.eVid,4*1024 + 512); /*4.5k*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort,6*1024);/*6k*/
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2Ecmp,4*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.nextHop,8*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.mllPairs,4*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2LttMll,8*1024);
        SET_IF_ZERO_MAC(devObjPtr->limitedResources.l3LttMll,4*1024);


        if (IS_PORT_0_EXISTS(devObjPtr))
        {
            devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_64KB;
            devObjPtr->lpmRam.perRamNumEntries = 6*1024;

            /* list of removed registers that should not be accessed */
            devObjPtr->registersDefaultsPtr_ignored =
                bobk_non_exists_linkListElements_RegistersDefaults;
        }
        else /* reduced silicon */
        {
            devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_32KB;
            devObjPtr->lpmRam.perRamNumEntries = 2*1024;

            /* DP 0 is not valid !!! */
            devObjPtr->multiDataPath.info[0].dataPathFirstPort = SMAIN_NOT_VALID_CNS;

            /* list of removed registers that should not be accessed */
            devObjPtr->registersDefaultsPtr_ignored =
                bobk_cetus_non_exists_linkListElements_RegistersDefaults;
        }



        SET_IF_ZERO_MAC(devObjPtr->policerSupport.eplrTableSize   , (4 * _1K));
        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrTableSize   , (6 * _1K));
        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[0] , 4888);
        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[1] , 1000);
        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[2] , /*256 */
            devObjPtr->policerSupport.iplrTableSize -
            (devObjPtr->policerSupport.iplrMemoriesSize[0] + devObjPtr->policerSupport.iplrMemoriesSize[1]));

        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplr0TableSize ,
                        devObjPtr->policerSupport.iplrMemoriesSize[1] +
                        devObjPtr->policerSupport.iplrMemoriesSize[2]) ;
        SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplr1TableSize ,
                    devObjPtr->policerSupport.iplrTableSize -
                    devObjPtr->policerSupport.iplr0TableSize);

    }

    devObjPtr->policerSupport.tablesBaseAddrSetByOrigDev = 1;

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemBobkSpecificDeviceUnitAlloc;
    }

    /* function will be called from inside smemLion3Init(...) */
    if(devObjPtr->devMemSpecificDeviceMemInitPart2 == NULL)
    {
        devObjPtr->devMemSpecificDeviceMemInitPart2 = smemBobkSpecificDeviceMemInitPart2;
    }

    smemBobcat2Init(devObjPtr);

    smemBobkInitPostBobcat2(devObjPtr);

    /* Init the bobk interrupts */
    smemBobkInitInterrupts(devObjPtr);

}

static void onEmulator_smemBobkAldrinInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemBobkAldrinInit function
* @endinternal
*
* @brief   Init memory module for a bobk-aldrin device.
*/
void smemBobkAldrinInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_IS_SIP5_16_GET(devObjPtr) = 1;

    devObjPtr->devMemUnitNameAndIndexPtr = bobkUnitNameAndIndexArr;
    devObjPtr->genericUsedUnitsAddressesArray = bobkAldrinUsedUnitsAddressesArray;
    devObjPtr->genericNumUnitsAddresses = BOBK_UNIT_LAST_E;

    devObjPtr->registersDefaultsPtr_unitsDuplications = BOBK_aldrin_duplicatedUnits;
    /* list of removed registers that should not be accessed */
    devObjPtr->registersDefaultsPtr_ignored =
        bobk_aldrin_non_exists_linkListElements_RegistersDefaults;

    /* state 'data path' structure */
    devObjPtr->multiDataPath.supportMultiDataPath =  1;
    devObjPtr->multiDataPath.maxDp = 3;/* 3 DP units for the device */
    {
        GT_U32  index;
        for(index = 0 ; index < devObjPtr->multiDataPath.maxDp ; index++)
        {
            /* first two DP have 12 ports. last one have 10 ports. */
            devObjPtr->multiDataPath.info[index].dataPathFirstPort  = PORTS_PER_DP_CNS*index;
            devObjPtr->multiDataPath.info[index].dataPathNumOfPorts = PORTS_PER_DP_CNS;
        }
    }
    devObjPtr->multiDataPath.info[2].dataPathNumOfPorts = 10;
    devObjPtr->multiDataPath.info[0].localDmaMapArr = aldrin_localDma_dp_0_1_Arr;
    devObjPtr->multiDataPath.info[1].localDmaMapArr = aldrin_localDma_dp_0_1_Arr;
    devObjPtr->multiDataPath.info[2].localDmaMapArr = aldrin_localDma_dp_2_Arr;
    devObjPtr->multiDataPath.info[2].cpuPortDmaNum = 72;
    devObjPtr->multiDataPath.supportRelativePortNum = 1;

    devObjPtr->supportTrafficManager_notAllowed = 1;

    devObjPtr->tcam_numBanksForHitNumGranularity = 6;
    SET_IF_ZERO_MAC(devObjPtr->tcamNumOfFloors,6);

    devObjPtr->dma_specialPortMappingArr = ALDRIN_DMA_specialPortMappingArr;

    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort,7);
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.stgId,10); /*1K*/
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.tunnelstartPtr , 14);/*16K*/

    SET_IF_ZERO_MAC(devObjPtr->limitedResources.eVid,4*1024 + 512); /*4.5k*/
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort,6*1024);/*6k*/
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort,128);
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2Ecmp,4*1024);
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.nextHop,8*1024);
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.mllPairs,4*1024);
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.l2LttMll,8*1024);
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.l3LttMll,4*1024);


    devObjPtr->fdbMaxNumEntries = SMEM_MAC_TABLE_SIZE_32KB;
    devObjPtr->lpmRam.perRamNumEntries = 2*1024;

    SET_IF_ZERO_MAC(devObjPtr->policerSupport.eplrTableSize   , (2 * _1K));
    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrTableSize   , (2 * _1K));

    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[0] , 1024 + 512);
    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[1] , 256);
    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplrMemoriesSize[2] , 256);

    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplr0TableSize ,
                    devObjPtr->policerSupport.iplrMemoriesSize[1] +
                    devObjPtr->policerSupport.iplrMemoriesSize[2]) ;
    SET_IF_ZERO_MAC(devObjPtr->policerSupport.iplr1TableSize ,
                devObjPtr->policerSupport.iplrTableSize -
                devObjPtr->policerSupport.iplr0TableSize);

    /* function will be called from inside smemLion2AllocSpecMemory(...) */
    if(devObjPtr->devMemSpecificDeviceUnitAlloc == NULL)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc = smemBobkAldrinSpecificDeviceUnitAlloc;
    }

    devObjPtr->registersDefaultsPtr = &aldrin_linkListElements_RegistersDefaults;
    devObjPtr->registersDfxDefaultsPtr = &aldrin_fix_DfxDefault_RegistersDefaults;

    smemBobkInit(devObjPtr);

    if(simulationCheck_onEmulator())
    {
        /* remove units/memories that not exists */
        onEmulator_smemBobkAldrinInit(devObjPtr);
    }
}

/**
* @internal smemBobkInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemBobcat2Init2(devObjPtr);
}

/**
* @internal smemBobkAldrinInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemBobkAldrinInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    static int my_dummy = 0;

    smemBobkInit2(devObjPtr);


    if(my_dummy)
    {
        GT_U32  ii;
        GT_U32  regAddress;
        GT_U32  globalPortNum;
        GT_U32  dpUnitInPipe;
        GT_U32  portInPipe = 0;
        GT_U32  offset;
        GT_U32  value;

        /* <DeviceEn>*/
        smemDfxRegFldSet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, 1);

        globalPortNum = 0;

        for(dpUnitInPipe = 0 ; dpUnitInPipe < devObjPtr->multiDataPath.maxDp; dpUnitInPipe ++)
        {
            offset = 0;

            portInPipe = PORTS_PER_DP_CNS*dpUnitInPipe;
            regAddress = SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,portInPipe);

            /* port per register */
            for(ii = 0 ; ii < (PORTS_PER_DP_CNS+1) ; ii++ , offset+=4)
            {
                if(ii == PORTS_PER_DP_CNS)
                {
                    value = SNET_CHT_CPU_PORT_CNS;
                }
                else
                {
                    value = globalPortNum;

                    globalPortNum ++;
                }

                /*perform RXDMA mapping from local port to 'virual' port on the field of:
                  localDevSrcPort */
                smemRegFldSet(devObjPtr,regAddress + offset,
                    0, 9, value);
            }
        }

        regAddress = SMEM_BOBCAT2_BMA_PORT_MAPPING_TBL_MEM(devObjPtr,0);

        /* set mapping 1:1 */

        /* port per register */
        for(ii = 0 ; ii < 256 ; ii++ , regAddress+=4)
        {
            value = ii & 0x7f;/* 7 bits */
            smemRegSet(devObjPtr,regAddress,value);
        }

        regAddress = SMEM_LION3_TXQ_DQ_TXQ_PORT_TO_TX_DMA_MAP_REG(devObjPtr, 0, 0);

        /* set mapping 1:1 */

        /* port per register */
        for(ii = 0 ; ii < 72 ; ii++ , regAddress+=4)
        {
            value = ii;
            smemRegSet(devObjPtr,regAddress,value);
        }
    }
}


#define onEmulator_ALDRIN_FIRST_PORT          24 /* first 24 ports are not valid */
#define onEmulator_ALDRIN_NUM_GOP_PORTS_GIG   (ALDRIN_NUM_GOP_PORTS_GIG - onEmulator_ALDRIN_FIRST_PORT)
#define onEmulator_ALDRIN_NUM_GOP_PORTS_XLG   (ALDRIN_NUM_GOP_PORTS_XLG - onEmulator_ALDRIN_FIRST_PORT)
#define onEmulator_ALDRIN_GOP_START_ADDR      (BOBCAT2_PORT_OFFSET_CNS * onEmulator_ALDRIN_FIRST_PORT)

#define onEmulator_BOBK_ALDRIN_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr + onEmulator_ALDRIN_GOP_START_ADDR ,numBytes)},                 \
        FORMULA_TWO_PARAMETERS(onEmulator_ALDRIN_NUM_GOP_PORTS_GIG, BOBCAT2_PORT_OFFSET_CNS,numSteps1, stepSize1)}

#define onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    onEmulator_BOBK_ALDRIN_PORTS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)

#define onEmulator_BOBK_ALDRIN_XLG_PORTS__BY_NUM_BYTES(startAddr,numBytes,numSteps1,stepSize1) \
    {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(startAddr + onEmulator_ALDRIN_GOP_START_ADDR  ,numBytes)},                 \
        FORMULA_TWO_PARAMETERS(onEmulator_ALDRIN_NUM_GOP_PORTS_XLG, BOBCAT2_PORT_OFFSET_CNS,numSteps1, stepSize1)}

#define onEmulator_BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(startAddr,endAddr,numSteps1,stepSize1) \
    onEmulator_BOBK_ALDRIN_XLG_PORTS__BY_NUM_BYTES(startAddr,(((endAddr)-(startAddr)) + 4),numSteps1,stepSize1)


/**
* @internal onEmulator_smemBobkAldrinUnitGop function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the bobk-aldrin GOP unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
*/
static void onEmulator_smemBobkAldrinUnitGop
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* Port<n> Auto-Negotiation Configuration Register */
    {0x0000000C+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},
    /* Port MAC Control Register2 */
    {0x00000008+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteMacGigControl2, 0},

    /* Port MAC Control Register0 */
    {0x000C0000+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},

    /* Port MAC Control Register3 */
    {0x000C001C+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemXcatActiveWriteMacModeSelect, 0},

    /* Port MAC Control Register4 : SMEM_XCAT_XG_MAC_CONTROL4_REG */
    {0x000C0084+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemBobKActiveWriteMacXlgCtrl4, 0},


    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x000C0024+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0 , smemChtActiveWritePortInterruptsMaskReg, 0},

    /* XG Port<n> Interrupt Cause Register  */
    {0x000C0014+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS,  smemChtActiveReadIntrCauseReg, 29, smemChtActiveWriteIntrCauseReg, 0},

    /* XG Port<n> Interrupt Mask Register */
    {0x000C0018+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* stack gig ports - Port<n> Interrupt Cause Register  */
    {0x00000020+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS,smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},

    /* stack gig ports - Tri-Speed Port<n> Interrupt Mask Register */
    {0x00000024+onEmulator_ALDRIN_GOP_START_ADDR, BOBCAT2_GOP_64_PORTS_MASK_CNS, NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},


    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    {
        /* ports 24..33 */
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
            /* PTP */
             onEmulator_BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180800,0,31, 0x4)

            /* Mac-TG Generator */
            ,onEmulator_BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180C00,0,52, 0x4)

            /* GIG */
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00000000, 0x00000094, 0, 0)
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x000000A0, 0x000000A4, 0, 0)
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x000000C0, 0x000000D8, 0, 0)

            /* MPCS */
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180008, 0x00180014, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180030, 0x00180030, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x0018003C, 0x001800C8, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x001800D0, 0x00180120, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180128, 0x0018014C, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x0018015C, 0x0018017C, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS__BY_NUM_BYTES(0x00180200, 256, 0, 0 )

            /* XPCS IP */
            /* ports 24..32 in steps of 2 */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00180400+onEmulator_ALDRIN_GOP_START_ADDR, 0x00180424+onEmulator_ALDRIN_GOP_START_ADDR)}, FORMULA_SINGLE_PARAMETER(onEmulator_ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x0018042C+onEmulator_ALDRIN_GOP_START_ADDR, 0x0018044C+onEmulator_ALDRIN_GOP_START_ADDR)}, FORMULA_SINGLE_PARAMETER(onEmulator_ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}
            /* 6 lanes */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (0x00180450+onEmulator_ALDRIN_GOP_START_ADDR, 0x0044)}, FORMULA_TWO_PARAMETERS(6 , 0x44  , onEmulator_ALDRIN_NUM_GOP_PORTS_XLG/2  , 0x1000*2)}

            /* XLG */
            ,onEmulator_BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0000,0x000c0024, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c002C,0x000c0060, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0068,0x000c0088, 0, 0 )
            ,onEmulator_BOBK_ALDRIN_PORTS_XLG_PORTS__BY_END_ADDR(0x000c0090,0x000c0098, 0, 0 )

            /*FCA*/
            ,onEmulator_BOBK_ALDRIN_PORTS_PORTS__BY_END_ADDR(0x00180600, 0x00180718,0, 0)

        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }

    {
        static SMEM_REGISTER_DEFAULT_VALUE_STC myUnit_registersDefaultValueArr[] =
        {
             /*{DUMMY_NAME_PTR_CNS,            0x000a1000,         0x0000ffff,      1,    0x0         }*/
            /* PTP */
             {DUMMY_NAME_PTR_CNS,           0x00180808+onEmulator_ALDRIN_GOP_START_ADDR,         0x00004D94,      onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180870+onEmulator_ALDRIN_GOP_START_ADDR,         0x000083aa,      onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180874+onEmulator_ALDRIN_GOP_START_ADDR,         0x00007e5d,      onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }
            ,{DUMMY_NAME_PTR_CNS,           0x00180878+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000040,      onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000 }

        /*Giga*/
            ,{DUMMY_NAME_PTR_CNS,            0x00000000+onEmulator_ALDRIN_GOP_START_ADDR,         0x00008be5,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00018004,         0x00000C03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00019004,         0x00000C83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001A004,         0x00000D03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001B004,         0x00000D83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001C004,         0x00000E03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001D004,         0x00000E83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001E004,         0x00000F03,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x0001F004,         0x00000F83,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00020004,         0x00001003,     1,     0x1000    }
            ,{DUMMY_NAME_PTR_CNS,            0x00000008+onEmulator_ALDRIN_GOP_START_ADDR,         0x0000c048,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000000c+onEmulator_ALDRIN_GOP_START_ADDR,         0x0000b8e8,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000001c+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000052,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0000002c+onEmulator_ALDRIN_GOP_START_ADDR,         0x0000000c,      2,    0x18,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000     }
            ,{DUMMY_NAME_PTR_CNS,            0x00000030+onEmulator_ALDRIN_GOP_START_ADDR,         0x0000c815,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000048+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000300,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000094+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000001,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c0+onEmulator_ALDRIN_GOP_START_ADDR,         0x00001004,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c4+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000100,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000c8+onEmulator_ALDRIN_GOP_START_ADDR,         0x000001fd,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

            ,{DUMMY_NAME_PTR_CNS,            0x00000090+onEmulator_ALDRIN_GOP_START_ADDR,         0x0000FD9A,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000018+onEmulator_ALDRIN_GOP_START_ADDR,         0x00004b4d,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00000014+onEmulator_ALDRIN_GOP_START_ADDR,         0x000008c4,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000000d4+onEmulator_ALDRIN_GOP_START_ADDR,         0x000000ff,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

        /*XLG*/
                                        /* set ports as XLG */
            ,{DUMMY_NAME_PTR_CNS,            0x000c0008+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000400,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0010+onEmulator_ALDRIN_GOP_START_ADDR,         0x00007BE3,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c001c+onEmulator_ALDRIN_GOP_START_ADDR,         0x00006000,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0030+onEmulator_ALDRIN_GOP_START_ADDR,         0x000007ec,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0080+onEmulator_ALDRIN_GOP_START_ADDR,         0x00001000,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000c0084+onEmulator_ALDRIN_GOP_START_ADDR,         0x00005210,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000C0000+onEmulator_ALDRIN_GOP_START_ADDR,         0x00000200,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x000C0004+onEmulator_ALDRIN_GOP_START_ADDR,         0x000002F9,     onEmulator_ALDRIN_NUM_GOP_PORTS_XLG,    0x1000                      }
        /*FCA*/
            ,{DUMMY_NAME_PTR_CNS,            0x00180000+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00000011,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180004+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00002003,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180054+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00000001,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180058+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x0000c200,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0018005c+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00000180,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x0018006c+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00008808,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180070+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x00000001,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
            ,{DUMMY_NAME_PTR_CNS,            0x00180104+0x600+onEmulator_ALDRIN_GOP_START_ADDR,   0x0000ff00,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }
        /* MPCS */
            ,{DUMMY_NAME_PTR_CNS,            0x00180014+onEmulator_ALDRIN_GOP_START_ADDR,         0x00003c20,     onEmulator_ALDRIN_NUM_GOP_PORTS_GIG,    0x1000                      }

            ,{NULL,            0,         0x00000000,      0,    0x0      }
        };
        static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC list = {myUnit_registersDefaultValueArr,NULL};
        unitPtr->unitDefaultRegistersPtr = &list;
    }
}
/**
* @internal onEmulator_smemBobkAldrinUnitXGPortMib function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the XGPortMib unit
*/
static void onEmulator_smemBobkAldrinUnitXGPortMib
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr
)
{
    START_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* XG port MAC MIB Counters */
    {0x00000000, SMEM_BOBCAT2_XG_MIB_COUNT_MSK_CNS, smemLion2ActiveReadMsmMibCounters, 0, NULL,0},

    END_BIND_UNIT_ACTIVE_MEM_MAC(devObjPtr,unitPtr)

    /* steps between each port */
    devObjPtr->xgCountersStepPerPort   = BOBCAT2_MIB_OFFSET_CNS;
    devObjPtr->xgCountersStepPerPort_1 = 0;/* not valid */
    /* offset of table xgPortMibCounters_1 */
    devObjPtr->offsetToXgCounters_1 = 0;/* not valid */
    devObjPtr->startPortNumInXgCounters_1 = 0;/* not valid */

    /* chunks with formulas */
    {
        SMEM_CHUNK_EXTENDED_STC  chunksMem[]=
        {
             {{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000,4)}}/* this line only as WA to fatal error on table 'xgPortMibCounters' */
            /* base of 0x00000000 */
            /* ports 24..32 - each port has a total of 30 counters: two 64-bit counters and 28 32-bit counters */
            ,{{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC(0x00000000+(BOBCAT2_MIB_OFFSET_CNS * onEmulator_ALDRIN_FIRST_PORT) ,0)}, FORMULA_TWO_PARAMETERS(128/4, 0x4, onEmulator_ALDRIN_NUM_GOP_PORTS_GIG, BOBCAT2_MIB_OFFSET_CNS)}
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_EXTENDED_STC);

        smemInitMemChunkExt(devObjPtr,chunksMem, numOfChunks, unitPtr);
    }
}

static void onEmulator_UnitMemoryBindToChunk
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * currUnitChunkPtr
)
{
    /* build addresses of the units according to base address */
    smemGenericUnitAddressesAlignToBaseAddress1(devObjPtr,
        currUnitChunkPtr,
        currUnitChunkPtr->hugeUnitSupportPtr ?
            GT_TRUE : GT_FALSE);
}



static GT_U32  skipUnitStartNumReg = sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32);
/**
* @internal resetRegDb function
* @endinternal
*
* @brief   Get DP local port number according to the global port number.
*
* @param[in] startUnitPtr             - pointer to memory of start unit
* @param[in] numRegisters             - the number of registers from startUnitPtr
* @param[in] stepOverStartUnit        - indication to skip start of unit that hold 'delemiter'
*                                      of SMEM_CHT_PP_REGS_UNIT_START_INFO_STC
*                                       None
*/
void resetRegDb(
    IN GT_U32                *startUnitPtr,
    IN GT_U32                numRegisters,
    IN GT_BOOL               stepOverStartUnit
)
{
    GT_U32  ii;

    if(stepOverStartUnit == GT_TRUE)
    {
        startUnitPtr += skipUnitStartNumReg;
        numRegisters -= skipUnitStartNumReg;
    }

    /* Init structure by SMAIN_NOT_VALID_CNS values */
    for(ii = 0;  ii < numRegisters; ii++)
    {
        startUnitPtr[ii] = SMAIN_NOT_VALID_CNS;
    }
}
#define SET_UNIT_AND_SIZE_MAC(unitName)  (void*)&unitName, ((sizeof(unitName) / sizeof(GT_U32)))

/*
PLLs - avoid writing/reading to/from DFX Server PLL registers
CPLL - cancel CPLL configuration
DRO - not accessed by CPSS
AVS - not accessed by CPSS
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/SERDES And SBC Controller/<Ports_Temp_Sensor> Avago_temp_sense
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/SERDES And SBC Controller/<Serdes> sd28C_txke_rxd6e - all SERDES
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/SERDES And SBC Controller/<SBC_IP> SBC Units
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/SERDES And SBC Controller/<Sbus_Master> SBUS_MASTER
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/TCAM
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<Gige MAC IP> Gige MAC IP Units%g  - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<XLG MAC IP> XLG MAC IP Units%p -  ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<MPCS IP> MPCS_IP Units%k - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<AP_IP> AP_IP Units%k - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<XPCS IP> XPCS Units%x -  ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<FCA IP> FCA IP Units%f - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<PTP IP> PTP IP Units%t - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<PacketGen IP> Packet Gen IP Units%p - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<MSM MIB IP> MSM MIB IP Units%m - ports 0 to 23
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/GOP/Units/GOP/<SMI IP> SMI%s - both SMI units
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/RXDMA - units 0 and 1
/Cider/EBU/Aldrin/Aldrin {Current}/Switching Core/TX_FIFO - units 0 and 1
/Cider/EBU/Aldrin/Aldrin {Current}/XBAR Units/IHB
/Cider/EBU/Aldrin/Aldrin {Current}/XBAR Units/PEX/<PEX_PHY>COMPHY_M_PIPE_rev3.4.2
*/

static void onEmulator_smemBobkAldrinInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex,ii;
    GT_U32  sizePerPort;
    static SMEM_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_registersDefaultValueArr[] =
    {
         {NULL,            0,         0x00000000,      0,    0x0      }
    };
    static SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC nonExistsUnit_defaults = {nonExistsUnit_registersDefaultValueArr,NULL};

    /*****************************************/
    /* remote units/memories that not exists */
    /*****************************************/
    if(0 == simulationCheck_onEmulator_isAldrinFull())
    {
        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TCAM);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        devObjPtr->tablesInfo.tcamMemory.commonInfo.baseAddress = 0;
        devObjPtr->tablesInfo.globalActionTable.commonInfo.baseAddress = 0;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_RX_DMA_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_DMA_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_TX_FIFO_1);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        currUnitChunkPtr->numOfChunks = 0;
        currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;
    }

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_0);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_1);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_2);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP_SMI_3);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;

    if(0 == simulationCheck_onEmulator_isAldrinFull())
    {
        for(ii = 0 ; ii < 2 ; ii++)
        {
            resetRegDb(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[ii]),GT_TRUE);
            resetRegDb(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[ii]),GT_TRUE);
            resetRegDb(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO[ii]),GT_TRUE);
        }
        resetRegDb(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM),GT_TRUE);

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_GOP);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        onEmulator_smemBobkAldrinUnitGop(devObjPtr,currUnitChunkPtr);
        onEmulator_UnitMemoryBindToChunk(devObjPtr,currUnitChunkPtr);

        unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_XG_PORT_MIB);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        onEmulator_smemBobkAldrinUnitXGPortMib(devObjPtr,currUnitChunkPtr);
        onEmulator_UnitMemoryBindToChunk(devObjPtr,currUnitChunkPtr);
    }

#if 0 /*allow to access : GOP/<SD_WRAPPER> SD_IP Units<<%s>>*/
    unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(devObjPtr,UNIT_SERDES);
    currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;
    currUnitChunkPtr->numOfChunks = 0;
    currUnitChunkPtr->unitDefaultRegistersPtr = &nonExistsUnit_defaults;
    onEmulator_UnitMemoryBindToChunk(devObjPtr,currUnitChunkPtr);

    sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]);
    sizePerPort /= 4;
    /* reset ALL ports */
    resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.SERDESes.SERDES[0]),
        sizePerPort*ALDRIN_NUM_GOP_PORTS_GIG , GT_FALSE);
#endif/*0*/

    if(0 == simulationCheck_onEmulator_isAldrinFull())
    {
        sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.gigPort[0]);
        sizePerPort /= 4;
        /* reset first 24 ports */
        resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.gigPort[0]),
            sizePerPort*onEmulator_ALDRIN_FIRST_PORT , GT_FALSE);

        sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.XLGIP[0]);
        sizePerPort /= 4;
        /* reset first 24 ports */
        resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.XLGIP[0]),
            sizePerPort*onEmulator_ALDRIN_FIRST_PORT , GT_FALSE);

        sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.MPCSIP[0]);
        sizePerPort /= 4;
        /* reset first 24 ports */
        resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.MPCSIP[0]),
            sizePerPort*onEmulator_ALDRIN_FIRST_PORT , GT_FALSE);

        sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.packGenConfig.packGenConfig[0]);
        sizePerPort /= 4;
        /* reset first 24 ports */
        resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.packGenConfig.packGenConfig[0]),
            sizePerPort*onEmulator_ALDRIN_FIRST_PORT , GT_FALSE);

        sizePerPort = sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0]);
        sizePerPort /= 4;
        /* reset first 24 ports */
        resetRegDb((GT_U32*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0]),
            sizePerPort*onEmulator_ALDRIN_FIRST_PORT , GT_FALSE);
    }

}



