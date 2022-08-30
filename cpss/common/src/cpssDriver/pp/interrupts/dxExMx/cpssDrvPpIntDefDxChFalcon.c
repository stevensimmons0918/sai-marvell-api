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
* @file cpssDrvPpIntDefDxChFalcon.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Falcon devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsFalcon.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_U32   falconOffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
);

/* fake function to support 5 interrupts registers on the same bit in the 'father' */
 /* d2d_serdes_mac_error_NODE_MAC(16,_tileId, _pipeId, _index),    */
 /* d2d_mac_rx_NODE_MAC          (17,_tileId, _pipeId, _index),    */
 /* d2d_pcs_event_NODE_MAC       (18,_tileId, _pipeId, _index),    */
 /* d2d_pma_event_NODE_MAC       (19,_tileId, _pipeId, _index),    */
static GT_STATUS prvCpssDrvHwPpPortGroupIsrRead_eagle_d2d_mac_tx_SUB_TREE_MAC
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc;
    /* read 'orig' addr : 0x00004600 of '<D2D> Qnm_Die2Die/D2D/MAC_TX/MAC TX Error Cause' */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* fake into this register also bits : 16..19 */
    *dataPtr |= 0x000f0000;
    return GT_OK;
}


/* fake function to support 5 interrupts registers on the same bit in the 'father' */
 /* d2d_serdes_mac_error_NODE_MAC(16,_tileId, _pipeId, _index),    */
 /* d2d_mac_rx_NODE_MAC          (17,_tileId, _pipeId, _index),    */
 /* d2d_pcs_event_NODE_MAC       (18,_tileId, _pipeId, _index),    */
 /* d2d_pma_event_NODE_MAC       (19,_tileId, _pipeId, _index),    */
static GT_STATUS prvCpssDrvHwPpPortGroupIsrRead_raven_d2d_mac_tx_SUB_TREE_MAC
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_STATUS rc;
    /* read 'orig' addr : 0x00004600 of '<D2D> Qnm_Die2Die/D2D/MAC_TX/MAC TX Error Cause' */
    rc = prvCpssDrvHwPpPortGroupIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* fake into this register also bits : 16..19 */
    *dataPtr |= 0x000f0000;
    return GT_OK;
}

/* offset of start of 'CERNTRAL' memory 0x18000000 .. 0x20000000 */
#define CENTRAL_OFFSET              0x18000000
/* offset of start of 'pipe0' memory 0x00000000 .. 0x01000000 */
#define PIPE_0_START_ADDR           0x08000000
/* offset of start of 'pipe1' memory 0x10000000 .. 0x18000000 */
#define PIPE_1_START_ADDR           0x10000000

/* offset between pipe 0 to pipe 1 in tile */
#define PIPE_OFFSET          0x08000000
/* offset between tile 0 to tile 1 to tile 2 to tile 3 */
#define TILE_OFFSET          0x20000000
/* offset between ravens in tile*/
#define RAVEN_OFFSET         0x01000000

/* Register address  per pipe */
#define _PIPE(_cause, _mask, _pipeId)     ((_cause) + ((GT_U32)PIPE_OFFSET * (_pipeId))), \
                                           ((_mask) + ((GT_U32)PIPE_OFFSET * (_pipeId)))

#define  PRV_CPSS_DXCH_UNIT_TAI0_E     PRV_CPSS_DXCH_UNIT_TAI_E
#define  PRV_CPSS_DXCH_UNIT_RXDMA0_E   PRV_CPSS_DXCH_UNIT_RXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TXDMA0_E   PRV_CPSS_DXCH_UNIT_TXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TX_FIFO0_E PRV_CPSS_DXCH_UNIT_TX_FIFO_E

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS(_tileId)        \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AU_PROC_COMPLETED_E)))

/* unit of type PRV_CPSS_DXCH_UNIT_ENT */
#define UNIT_IN_TILE(unit,_tileId)                                              \
    ((unit) + (PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS*(_tileId)))

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT_IN_TILE(unit,_tileId,_mgIndex) \
    ((PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS*(_tileId)) +  \
        (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E))


/* unit of type PRV_CPSS_DXCH_UNIT_ENT for Raven device */
#define UNIT_IN_RAVEN(unit,_tileId, _ravenId)                                                       \
    ((PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + (_ravenId)) << 16) | UNIT_IN_TILE(unit,_tileId)

#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)
#define RAVEN_ID_GET(unit) ((unit) >> 16)

#define FALCON_SBC_SD_INTERRUPT_SUM_CAUSE               0x00600060
#define FALCON_SBC_SD_INTERRUPT_SUM_MASK                0x00600064
#define FALCON_SBC_SD_INTERRUPT_1_CAUSE                 0x00600040
#define FALCON_SBC_SD_INTERRUPT_1_MASK                  0x00600044

#define FALCON_SD_PORT_BASE                             0x00340000

#define FALCON_MTI_CPU_PORT_INTERRUPT_CAUSE             0x0051C008
#define FALCON_MTI_CPU_PORT_INTERRUPT_MASK              0x0051C00C

#define FALCON_SD_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(port, unit, _tileId, _ravenId)     \
        (FALCON_SD_PORT_BASE + 0x1000*(port)+ 0x40)

#define FALCON_SD_PORT_INTERRUPT_SUMMARY_MASK_MAC(port, unit, _tileId, _ravenId)     \
        (FALCON_SD_PORT_BASE + 0x1000*(port)+ 0x44)

#define FALCON_SD_PORT_INTERRUPT_0_CAUSE_MAC(port, unit, _tileId, _ravenId)     \
        (FALCON_SD_PORT_BASE + 0x1000*(port)+ 0x28)

#define FALCON_SD_PORT_INTERRUPT_0_MASK_MAC(port, unit, _tileId, _ravenId)     \
        (FALCON_SD_PORT_BASE + 0x1000*(port)+ 0x2c)

#define FALCON_MTI_PORT_BASE_MAC(unit, _tileId, _ravenId)                   \
        (0x00478000 + 0x80000*(unit))

/* MSDB unit */
#define FALCON_MSDB_INTERRUPT_SUMMARY_CAUSE_MAC(unit)                        \
        (0x00508404 + 0x1000*(unit))

#define FALCON_MSDB_INTERRUPT_SUMMARY_MASK_MAC(unit)                         \
        (0x00508408 + 0x1000*(unit))

/* LMU unit */
#define FALCON_LMU_INTERRUPT_SUMMARY_CAUSE_MAC(unit)                        \
        (0x00438230 + 0x80000*(unit))

#define FALCON_LMU_INTERRUPT_SUMMARY_MASK_MAC(unit)                         \
        (0x00438234 + 0x80000*(unit))

#define FALCON_LMU_LATENCY_INTERRUPT_CAUSE_MAC(unit, _threshold)            \
        ((0x00438100 + 0x80000*(unit)) + (0x4 * (_threshold)))

#define FALCON_LMU_LATENCY_INTERRUPT_MASK_MAC(unit, _threshold)             \
        ((0x00438180 + 0x80000*(unit)) + (0x4 * (_threshold)))


/* MTI100 - cause */
#define FALCON_MTI100_PORT_INTERRUPT_CAUSE_MAC(port, unit,_tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x18*(port)+0x94)

/* MTI100 - mask */
#define FALCON_MTI100_PORT_INTERRUPT_MASK_MAC(port, unit,_tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x18*(port)+0x98)

/* MTI400 - cause */
#define FALCON_MTI400_PORT_INTERRUPT_CAUSE_MAC(port, unit,_tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+((port == 4)? 0x14 : 0)+0x68)

/* MTI400 - mask */
#define FALCON_MTI400_PORT_INTERRUPT_MASK_MAC(port, unit,_tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+((port == 4)? 0x14 : 0)+0x6c)

/* MTI Global - cause */
#define FALCON_MTI_GLOBAL_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x4c)

/* MTI Global - mask */
#define FALCON_MTI_GLOBAL_INTERRUPT_MASK_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x50)

/* Global FEC CE - cause */
#define FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x3c)

/* Global FEC CE - mask */
#define FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_MASK_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x40)

/* Global FEC NCE - cause */
#define FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x44)

/* Global FEC NCE - mask */
#define FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_MASK_MAC(unit, _tileId, _ravenId) \
        (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId)+0x48)

/* Cause/Mask address pairs per tree index.
   Tree index is used to connect dual tile to MG0 bit 17:
   0 - cause/mask registers for global cause 0 tree (relevant for single/dual tiles)
   1 - cause/mask registers for global cause 1 tree (relevant for 4 tiles) */

#define GLOBAL_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00000030 : 0x00000610), ((tree == 0) ? 0x00000034 : 0x00000614)

#define FUNC_UNITS_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x000003F8 : 0x0000064C), ((tree == 0) ? 0x000003FC : 0x00000650)

#define FUNC1_UNITS_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x000003F4 : 0x00000648), ((tree == 0) ? 0x000003F0 : 0x00000644)

#define DATA_PATH_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x000000A4 : 0x00000628), ((tree == 0) ? 0x000000A8 : 0x0000062C)

#define PORTS_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00000080 : 0x00000620), ((tree == 0) ? 0x00000084 : 0x00000624)

#define DFX_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x000000AC : 0x00000630), ((tree == 0) ? 0x000000B0 : 0x00000634)

#define DFX1_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x000000B8 : 0x00000638), ((tree == 0) ? 0x000000BC : 0x0000063C)

#define MG_INTERNAL_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00000038 : 0x00000618), ((tree == 0) ? 0x0000003C : 0x0000061C)

#define TX_DMA_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00002810 : 0x00002898), ((tree == 0) ? 0x00002818 : 0x000028A8)

#define RX_DMA_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x0000280C : 0x00002890), ((tree == 0) ? 0x00002814 : 0x000028A0)

#define PORTS1_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00000150 : 0x00000160), ((tree == 0) ? 0x00000154 : 0x00000164)

#define PORTS2_CAUSE_MASK(tree) \
        ((tree == 0) ? 0x00000158 : 0x00000168), ((tree == 0) ? 0x0000015C : 0x0000016C)

#define   mg0InternalIntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                          \
    /* Miscellaneous */                                                                             \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E,_tileId,_mgIndex), NULL,                \
        MG_INTERNAL_CAUSE_MASK(_treeId),                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_I2C_TIME_OUT_INTERRUPT_E,                   \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */ \
        {24, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,           \
            0x00000518, 0x0000051c,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define   mt_SUB_TREE_MAC(bit,_tileId)                                                  \
    /* MT FDBIntSum - FDB Interrupt Cause */                                            \
    {bit, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_FDB_E,_tileId), NULL,                \
        0x000001B0, 0x000001B4,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_NUM_OF_HOP_EX_P_E,                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_ADDRESS_OUT_OF_RANGE_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS(_tileId),                          \
             0, NULL, NULL}

#define   em_SUB_TREE_MAC(bit,_tileId)                                                  \
    /* EM FDBIntSum - FDB Interrupt Cause */ \
    {bit, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EM_E,_tileId), NULL,                 \
        0x000001B0, 0x000001B4,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_EM_FDB_NUM_OF_HOP_EX_P_E,                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_EM_FDB_ADDRESS_OUT_OF_RANGE_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS(_tileId),                          \
             0, NULL, NULL}

#define msdb_SUB_TREE_MAC(bit, _unit, _tileId, _ravenId)                                             \
    /* MSDB Cause Summary */                                                                         \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,                \
        FALCON_MSDB_INTERRUPT_SUMMARY_CAUSE_MAC(_unit),                                              \
        FALCON_MSDB_INTERRUPT_SUMMARY_MASK_MAC(_unit),                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MSDB##_unit##_ILLEGAL_ADDRESS_ACCESS_E,  \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MSDB##_unit##_FCU_TX_GROUP_ERROR_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(bit, _threshold, _unit, _tileId, _ravenId, _fromPort, _toPort)    \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,   \
        FALCON_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_unit, _threshold),                      \
        FALCON_LMU_LATENCY_INTERRUPT_MASK_MAC(_unit, _threshold),                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_LMU##_unit##_LATENCY_OVER_THRESHOLD_##_fromPort##_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_LMU##_unit##_LATENCY_OVER_THRESHOLD_##_toPort##_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_SUB_TREE_MAC(bit, _unit, _tileId, _ravenId)                                 \
    /* LMU Cause Summary */                                                             \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,   \
        FALCON_LMU_INTERRUPT_SUMMARY_CAUSE_MAC(_unit),                                  \
        FALCON_LMU_INTERRUPT_SUMMARY_MASK_MAC(_unit),                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_LMU##_unit##_LATENCY_OVER_THRESHOLD_SUM0_E,  \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_LMU##_unit##_LATENCY_FIFO_FULL_SUM0_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                         \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 1,  0, _unit, _tileId, _ravenId,   0,  30),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 2,  1, _unit, _tileId, _ravenId,  31,  61),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 3,  2, _unit, _tileId, _ravenId,  62,  92),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 4,  3, _unit, _tileId, _ravenId,  93, 123),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 5,  4, _unit, _tileId, _ravenId, 124, 154),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 6,  5, _unit, _tileId, _ravenId, 155, 185),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 7,  6, _unit, _tileId, _ravenId, 186, 216),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 8,  7, _unit, _tileId, _ravenId, 217, 247),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC( 9,  8, _unit, _tileId, _ravenId, 248, 278),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(10,  9, _unit, _tileId, _ravenId, 279, 309),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(11, 10, _unit, _tileId, _ravenId, 310, 340),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(12, 11, _unit, _tileId, _ravenId, 341, 371),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(13, 12, _unit, _tileId, _ravenId, 372, 402),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(14, 13, _unit, _tileId, _ravenId, 403, 433),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(15, 14, _unit, _tileId, _ravenId, 434, 464),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(16, 15, _unit, _tileId, _ravenId, 465, 495),  \
        FALCON_LMU_LATENCY_OVER_THRESHOLD_INT_SUB_TREE_MAC(17, 16, _unit, _tileId, _ravenId, 496, 526)

GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);
#define AP_Doorbell_SUB_TREE_MAC(bit, _tileId, _ravenId) \
    /* AP_DOORBELL_MAIN */ \
    {bit, GT_FALSE ,UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL, 0x00000010, 0x00000011,  \
        prvCpssGenericSrvCpuRegisterRead,                                                                     \
        prvCpssGenericSrvCpuRegisterWrite,                                                                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_MAIN_IPC_E,                           \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_MAIN_RESERVED31_E,                    \
        FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0xFFFFF880, 3,NULL, NULL},                                         \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                                                         \
        {4, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL, 0x00000012, 0x00000013,\
            prvCpssGenericSrvCpuRegisterRead,                                                                 \
            prvCpssGenericSrvCpuRegisterWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,     \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},                                            \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                                                   \
        {8, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL, 0x00000018, 0x00000019,\
            prvCpssGenericSrvCpuRegisterRead,                                                                 \
            prvCpssGenericSrvCpuRegisterWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_0_802_3_AP_E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_31_802_3_AP_E,               \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},                                            \
        /* AP_DOORBELL_PORT0_31_AP_DISABLE_AP */                                                               \
        {12, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL, 0x0000001E, 0x0000001F,\
            prvCpssGenericSrvCpuRegisterRead,                                                                 \
            prvCpssGenericSrvCpuRegisterWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_0_AP_DISABLE_E,               \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_PORT_31_AP_DISABLE_E,              \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL}

#define raven_side_d2d_cp_SUB_TREE_MAC(bit, _tileId, _ravenId)                                                               \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E, _tileId, _ravenId), NULL,         \
        0x00000050, 0x00000054,                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                           \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                          \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_CP_D2X_REQ_TIMEOUT_CNT_EXPIRED_E,                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_CP_TAP_TIMEOUT_TRIGGERED_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_side_d2d_serdes_mac_error_NODE_MAC(bit, _tileId, _ravenId, _d2dId)                                     \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _d2dId, _tileId, _ravenId), NULL,         \
        0x0000603C, 0x00006038,                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_SERDES_MAC_ERR_BAD_ADDRESS_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_SERDES_MAC_ERR_TX_CHILD_ERROR_7_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_side_d2d_mac_rx_NODE_MAC(bit, _tileId, _ravenId, _d2dId)                                                   \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _d2dId, _tileId, _ravenId), NULL,         \
        0x00004F08, 0x00004F04,                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_MAC_RX_ERR_TX_FIFO_OVERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_MAC_RX_ERR_MAC_DEBUG_CHECK_RF_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_side_d2d_pcs_event_NODE_MAC(bit, _tileId, _ravenId, _d2dId)                                                   \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _d2dId, _tileId, _ravenId), NULL,         \
        0x00007040, 0x00007044,                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_side_d2d_pma_event_NODE_MAC(bit, _tileId, _ravenId, _d2dId)                                                   \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _d2dId, _tileId, _ravenId), NULL,         \
        0x00008060, 0x00008064,                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_PMA_ERR_RX_CDC_OVERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_PMA_ERR_RX_FEC_UNCOR_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_side_d2d_SUB_TREE_MAC(bit, _tileId, _ravenId , _d2dId)      \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _d2dId, _tileId, _ravenId), NULL,         \
        0x00004600, 0x000045FC,                                                               \
        prvCpssDrvHwPpPortGroupIsrRead_raven_d2d_mac_tx_SUB_TREE_MAC,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_MAC_TX_ERR_MAC_TX_RF_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_D2D_##_d2dId##_MAC_TX_ERR_RESERVED31_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                                      \
/*fake!*/  raven_side_d2d_serdes_mac_error_NODE_MAC(16,_tileId, _ravenId, _d2dId),                                   \
/*fake!*/  raven_side_d2d_mac_rx_NODE_MAC(17, _tileId, _ravenId, _d2dId),                                            \
/*fake!*/  raven_side_d2d_pcs_event_NODE_MAC(18, _tileId, _ravenId, _d2dId),                                         \
/*fake!*/  raven_side_d2d_pma_event_NODE_MAC(19, _tileId, _ravenId, _d2dId)

#define raven_dfx_server_int_SUB_TREE_MAC(bit, _tileId, _ravenId)                                                      \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, _tileId, _ravenId), NULL,                           \
        0x000F8108, 0x000F810C,                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                               \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,          \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_DFX_SERVER_INTERRUPT_EXTERNAL_LOW_TEMPERATURE_THRESHOLD_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define raven_dfx_server_int_sum_SUB_TREE_MAC(bit, _tileId, _ravenId)                                            \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, _tileId, _ravenId), NULL,                     \
        0x000F8100, 0x000F8104,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                                  \
        raven_dfx_server_int_SUB_TREE_MAC(1, _tileId, _ravenId)

#define raven_SUB_TREE_MAC(bit, _tileId, _ravenId) \
    /* Raven Global Interrupt Cause */ \
    {bit, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL,    \
        0x00000030, 0x00000034,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_GEN_PCIE_INTERRUPT_SUMMARY_E,                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_GEN_MG_INT01_INTERNAL_INTERRUPT_SUMMARY_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                         \
        /* Functional Interrupts Summary Cause */                                       \
        {3, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL,  \
            0x000003F8, 0x000003FC,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_FUNC_INTR_IN_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_FUNC_I2C_INTR_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                     \
        /* Ports Interrupts Summary Cause */                                            \
        {5, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL,  \
            0x00000080, 0x00000084,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_PORTS_SERDES_INTERRUPT_CAUSE_SUM_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_PORTS_MTIP_INT_SUM_1_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                     \
            FALCON_SEREDES_GLOBAL_SUMMARY_INT_SUB_TREE_MAC(1, _tileId,_ravenId),        \
            FALCON_MTI_CPU_SUMMARY_INT_NODE_MAC(24, _tileId,_ravenId),                  \
            FALCON_MTI_GLOBAL_SUMMARY_INT_SUB_TREE_MAC(25, 0, _tileId,_ravenId),        \
            FALCON_MTI_GLOBAL_SUMMARY_INT_SUB_TREE_MAC(26, 1, _tileId,_ravenId),        \
        /*  DFX Interrupts Summary Cause */                                             \
        {6, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL,  \
            0x000000AC, 0x000000B0,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_DFX_SERVER_INT_SUM_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_DFX_CLIENT_MTIP_CPU_MACRO_APP_CLK_INT_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                     \
            raven_dfx_server_int_sum_SUB_TREE_MAC(1, _tileId, _ravenId),                      \
        /*  MG internal Interrupt Cause */                                              \
        {7, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL,  \
            0x00000038, 0x0000003C,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_INTERNAL_ILLEGAL_ADDRESS_INTERRUPT_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_INTERNAL_SERDES_ADDRESS_OUT_OF_RANGE_VIOLATION_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                     \
            /* HostCpuDoorbellIntSum */ \
            {24, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL, \
                0x00000518, 0x0000051c,                                                     \
                prvCpssDrvHwPpPortGroupIsrRead,                                             \
                prvCpssDrvHwPpPortGroupIsrWrite,                                            \
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,\
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1,NULL, NULL},                  \
                AP_Doorbell_SUB_TREE_MAC(4, _tileId, _ravenId),\
        /* Functional 1 Interrupts Summary Cause */                                     \
        {11, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL, \
            0x000003F4, 0x000003F0,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_FUNC_1_QNM_D2D_GW_500G_INTERRUPT0_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_FUNC_1_GOP_SD0_GENX_MG_TERM_TIMEOUT_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF,  3, NULL, NULL},                     \
            raven_side_d2d_SUB_TREE_MAC(1, _tileId, _ravenId, 0/*d2d0*/),               \
            raven_side_d2d_SUB_TREE_MAC(2, _tileId, _ravenId, 1/*d2d1*/),               \
            raven_side_d2d_cp_SUB_TREE_MAC(3, _tileId, _ravenId),                       \
        /*  Ports 1 Interrupts Summary Cause */                                         \
        {16, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _ravenId), NULL, \
            0x00000150, 0x00000154,                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_PORTS1_TAI_INT0_E,        \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_PORTS1_LMU_INTERRUPT_1_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                      \
            msdb_SUB_TREE_MAC(4, 0, _tileId, _ravenId),                                  \
            msdb_SUB_TREE_MAC(5, 1, _tileId, _ravenId),                                  \
            lmu_SUB_TREE_MAC(6, 0, _tileId, _ravenId),                                   \
            lmu_SUB_TREE_MAC(7, 1, _tileId, _ravenId)


#define FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller, port, unit, _tileId, _ravenId)    \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI100_PORT_INTERRUPT_CAUSE_MAC(port,unit,_tileId, _ravenId),                        \
            FALCON_MTI100_PORT_INTERRUPT_MASK_MAC(port,unit,_tileId, _ravenId),                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_PORT##port##_LINK_OK_CHANGE_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_PORT##port##_FF_RX_RDY_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_MTI400_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller, port, unit, _tileId, _ravenId)    \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI400_PORT_INTERRUPT_CAUSE_MAC(port, unit,_tileId, _ravenId),       \
            FALCON_MTI400_PORT_INTERRUPT_MASK_MAC(port, unit,_tileId, _ravenId),        \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_SEG_PORT##port##_LINK_OK_CHANGE_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_SEG_PORT##port##_FF_RX_RDY_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_GLOBAL_FEC_CE_SUMMARY_INT_NODE_MAC(bitIndexInCaller, unit, _tileId, _ravenId)    \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_CAUSE_MAC(unit,_tileId, _ravenId),                       \
            FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_MASK_MAC(unit,_tileId, _ravenId),                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_FEC_CE_0_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_FEC_CE_15_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_GLOBAL_FEC_NCE_SUMMARY_INT_NODE_MAC(bitIndexInCaller, unit, _tileId, _ravenId)    \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_CAUSE_MAC(unit,_tileId, _ravenId),                       \
            FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_MASK_MAC(unit,_tileId, _ravenId),                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_FEC_NCE_0_E,\
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_FEC_NCE_15_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_MTI_PORT_SUMMARY_INT_SUB_TREE_MAC(unit, _tileId, _ravenId)                       \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(1,   0,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(2,   1,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(3,   2,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(4,   3,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(5,   4,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(6,   5,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(7,   6,   unit, _tileId, _ravenId),     \
            FALCON_MTI100_PORT_SUMMARY_INT_SUB_TREE_MAC(8,   7,   unit, _tileId, _ravenId),     \
            FALCON_MTI400_PORT_SUMMARY_INT_SUB_TREE_MAC(9,   0,   unit, _tileId, _ravenId),     \
            FALCON_MTI400_PORT_SUMMARY_INT_SUB_TREE_MAC(10,  4,   unit, _tileId, _ravenId),     \
            FALCON_GLOBAL_FEC_CE_SUMMARY_INT_NODE_MAC  (12,       unit, _tileId, _ravenId),     \
            FALCON_GLOBAL_FEC_NCE_SUMMARY_INT_NODE_MAC (13,       unit, _tileId, _ravenId)

#define FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller, port, unit, _tileId, _ravenId)    \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_SD_PORT_INTERRUPT_SUMMARY_CAUSE_MAC(port, unit,_tileId, _ravenId),               \
            FALCON_SD_PORT_INTERRUPT_SUMMARY_MASK_MAC(port, unit,_tileId, _ravenId),                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SD1_INT_PORT##port##_SUM_0_E,       \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SD1_INT_PORT##port##_RESERVED_30_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                 \
            {1, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,         \
                FALCON_SD_PORT_INTERRUPT_0_CAUSE_MAC(port, unit,_tileId, _ravenId),                 \
                FALCON_SD_PORT_INTERRUPT_0_MASK_MAC(port, unit,_tileId, _ravenId),                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SD1_INT0_PORT##port##_INT_BIT_0_E, \
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SD1_INT0_PORT##port##_RESERVED_30_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define FALCON_SEREDES_GLOBAL_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller, _tileId, _ravenId)     \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL, \
            FALCON_SBC_SD_INTERRUPT_SUM_CAUSE,                                                  \
            FALCON_SBC_SD_INTERRUPT_SUM_MASK,                                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SBC_SD_INT_GLOBAL_SD_INT_SUM_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SBC_SD_INT_RESERVED_31_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                             \
            {1, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,     \
                FALCON_SBC_SD_INTERRUPT_1_CAUSE,                                                \
                FALCON_SBC_SD_INTERRUPT_1_MASK,                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SBC_SD_INT1_SUM_E,          \
                PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_SBC_SD_INT1_INT_1_30_E,     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                        \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(1,    0,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(2,    1,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(3,    2,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(4,    3,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(5,    4,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(6,    5,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(7,    6,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(8,    7,   0, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(10,   8,    1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(11,   9,    1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(12,   10,   1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(13,   11,   1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(14,   12,   1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(15,   13,   1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(16,   14,   1, _tileId, _ravenId),     \
                FALCON_SERDES_PORT_SUMMARY_INT_SUB_TREE_MAC(17,   15,   1, _tileId,_ravenId)
               /* FALCON_SERDES_CPU_SUMMARY_INT_SUB_TREE_MAC(9,     0,      _tileId, _ravenId),     \*/

#define FALCON_MTI_GLOBAL_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller, unit, _tileId, _ravenId)           \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI_GLOBAL_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId),                             \
            FALCON_MTI_GLOBAL_INTERRUPT_MASK_MAC(unit, _tileId, _ravenId),                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_GLOBAL_SUM_P0_INT_SUM_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTI_EXT##unit##_GLOBAL_SUM_FEC_NCE_INT_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 12, NULL, NULL},                    \
            FALCON_MTI_PORT_SUMMARY_INT_SUB_TREE_MAC(unit,_tileId,_ravenId)

#define FALCON_MTI_CPU_SUMMARY_INT_NODE_MAC(bitIndexInCaller, _tileId, _ravenId)        \
        {bitIndexInCaller, GT_FALSE, UNIT_IN_RAVEN(PRV_CPSS_DXCH_UNIT_GOP_E, _tileId, _ravenId), NULL,  \
            FALCON_MTI_CPU_PORT_INTERRUPT_CAUSE,                                        \
            FALCON_MTI_CPU_PORT_INTERRUPT_MASK,                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTIIP_CPU_LINK_OK_CHANGED_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MTIIP_CPU_FF_RX_RDY_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   packer0_SUB_TREE_MAC(bit,_tileId)                                             \
    /* PDX Packer0 Interrupt Cause */ \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E, _tileId), NULL,    \
        0x00001F10, 0x00001F14,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_PAC0_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_PAC0_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   packer1_SUB_TREE_MAC(bit,_tileId)                                             \
    /* PDX Packer1 Interrupt Cause */ \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E, _tileId), NULL,    \
        0x00001F10, 0x00001F14,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_PAC1_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_PAC1_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pdx_SUB_TREE_MAC(bit,_tileId)                                                   \
    /* PDX Interrupt Summary */                                                         \
    {27, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PDX_E,_tileId), NULL,            \
        0x0001F010, 0x0001F014,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_ILLEGAL_ADDRESS_ACCESS_E,                  \
        PRV_CPSS_FALCON_TILE_##_tileId##_PDX_BURST_FIFO_DROP_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_SUB_TREE_MAC(bit,_tileId)                                              \
    /* PFCC Interrupt Summary */                                                    \
    {28, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E,_tileId), NULL,        \
        0x00000100, 0x00000104,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_PFCC_ILLEGAL_ADDRESS_ACCESS_E,             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PFCC_SOURCE_PORT_COUNTER_OVERFLOW_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_SUB_TREE_MAC(bit,_tileId)                                               \
    /* PSI Interrupt Summary */                                                     \
    {29, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PSI_E,_tileId), NULL,         \
        0x00000134, 0x00000138,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_PSI_SCHEDULER_INTERRUPT_E,                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PSI_BAD_ADDRESS_INT_E,                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define shm_SUB_TREE_MAC(bit,_tileId)                                               \
    /* SHM Interrupt Summary */                                                     \
    {30, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_SHM_E,_tileId), NULL,             \
        0x000000E0, 0x000000E4,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_SHM_RF_ERR_CAUSE_SUM_E,                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_SHM_ENGINE_ERR_CAUSE_SUM_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define npm_SUB_TREE_MAC(bit,_tileId, _instance)                                                            \
    /* NPM-0  */                                                                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL,   \
        0x00000100, 0x00000104,                                                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_SUM_GENERIC_E,                              \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_SUM_LIST_ERR_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                            \
                                                                                                            \
        /* GenericInterruptSum  */                                                                          \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000108, 0x0000010C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_GEN_ADDR_ERR_E,                         \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_GEN_RAND_ICRD_E,                        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UintErrorInterruptSum  */                                                                        \
        {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000110, 0x00000114,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_ERR_CAUSE_0_0_E,                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_ERR_CAUSE_3_4_E,                        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertInterruptSum  */                                                                         \
        {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000118, 0x0000011C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REINSERT_CAUSE_0_0_E,                   \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REINSERT_CAUSE_3_4_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ReinsertfailInterruptSum  */                                                                     \
        {4, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000120, 0x00000124,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REINFAIL_CAUSE_0_0_E,                   \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REINFAIL_CAUSE_3_4_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitRefsInterruptSum  */                                                                         \
        {5, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000128, 0x0000012C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_UNIT_REFS_CAUSE_0_0_E,                  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_UNIT_REFS_CAUSE_3_4_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* UnitDataInterruptSum  */                                                                         \
        {6, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000130, 0x00000134,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_UNIT_DATA_CAUSE_0_0_E,                  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_UNIT_DATA_CAUSE_3_4_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsErrorInterruptSum  */                                                                        \
        {7, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000138, 0x0000013C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REFS_ERR_CAUSE_0_0_E,                   \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REFS_ERR_CAUSE_3_3_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* AgingErrorInterruptSum  */                                                                       \
        {8, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000140, 0x00000144,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_AGING_ERR_CAUSE_0_0_E,                  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_AGING_ERR_CAUSE_3_3_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* RefsFifoInterruptSum  */                                                                         \
        {9, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL, \
            0x00000148, 0x0000014C,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REFS_FIFO_CAUSE_0_0_E,                  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_REFS_FIFO_CAUSE_3_3_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                          \
        /* ListErrorInterruptSum  */                                                                        \
        {10, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_##_instance##_E, _tileId), NULL,\
            0x00000150, 0x00000154,                                                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_LIST_ERR_CAUSE_0_E,                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_MC##_instance##_LIST_ERR_CAUSE_3_E,                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define smb_SUB_TREE_MAC(bit,_tileId, _instance, _index)                                                               \
    /* SMB-MC */                                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL,   \
        0x00000150, 0x00000154,                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                               \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_SUM_INTERRUPT_SUM_MISC_E,                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_SUM_INTERRUPT_SUM_FIFO_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                                         \
                                                                                                                       \
        /* MiscInterruptSum  */                                                                                        \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000100, 0x00000104,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MISC_RPW_NO_TRUST_E,                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* AgeInterruptSum  */                                                                                         \
        {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000110, 0x00000114,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_AGE_0_E,                                 \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_AGE_15_E,                                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* RbwInterruptSum  */                                                                                         \
        {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000120, 0x00000124,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_RBW_ERR_0_E,                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_RBW_ERR_15_E,                            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* MemoriesInterruptSum  */                                                                                    \
        {4, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000130, 0x00000134,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MEM_PAYLOAD_SER_ERROR_0_E,               \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MEM_PAYLOAD_SER_ERROR_15_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* Memories2InterruptSum  */                                                                                   \
        {5, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000140, 0x00000144,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MEM2_REFCNT_SER_ERROR_0_E,               \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_MEM2_LIST_SER_ERROR_3_E,                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                     \
        /* FifoInterruptSum  */                                                                                        \
        {6, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF##_##_instance##_##_index##_E, _tileId), NULL, \
            0x00000118, 0x0000011C,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_INTERRUPT_FIFO_ERR_0_E,                  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB##_instance##_MC##_index##_INTERRUPT_FIFO_ERR_15_E,                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   tcam_SUB_TREE_MAC(bit,_tileId)                                            \
    /* TCAM Interrupt Summary */                                                    \
    {bit, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TCAM_E,_tileId), NULL,           \
        0x00501004, 0x00501000,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,             \
        PRV_CPSS_FALCON_TILE_##_tileId##_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                     \
        /* TCAM Interrupt */                                                        \
        {1, GT_FALSE,UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TCAM_E,_tileId), NULL,         \
            0x0050100C, 0x00501008,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_TCAM_CPU_ADDRESS_OUT_OF_RANGE_E,       \
            PRV_CPSS_FALCON_TILE_##_tileId##_TCAM_TABLE_OVERLAPPING_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   bma_SUB_TREE_MAC(bit, _tileId, _pipe)                                                 \
        /* bma  */                                                                              \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_BMA_E, _tileId), NULL,                  \
            _PIPE(0x00059000, 0x00059004, _pipe),                                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_BMA_SUM_MC_CNT_RMW_CLEAR_FIFO_FULL_E,\
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_BMA_SUM_FINAL_CLEAR_UDB_FIFO0_FULL_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eft_SUB_TREE_MAC(bit, _tileId, _pipe)                                       \
        /* eft  */                                                                    \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EGF_EFT_E, _tileId), NULL,    \
            _PIPE(0x000010A0, 0x000010B0, _pipe),                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                          \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EFT_SUM_EGRESS_WRONG_ADDR_E,       \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EFT_SUM_INC_BUS_IS_TOO_SMALL_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eoam_SUB_TREE_MAC(bit, _tileId, _pipe)                                    \
        /* egress OAM  */                                                           \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EOAM_E, _tileId), NULL,     \
            _PIPE(0x000000F0, 0x000000F4, _pipe),                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EGR_OAM_KEEP_ALIVE_AGING_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EGR_OAM_TX_PERIOD_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_SUB_TREE_MAC(bit, _tileId, _pipe)                                        \
        /* EPCL  */                                                                     \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EPCL_E, _tileId), NULL,         \
            _PIPE(0x00000010, 0x00000014, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EPCL_REGFILE_ADDRESS_ERROR_E,        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EPCL_TABLE_OVERLAP_ACCESS_ERROR_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_SUB_TREE_MAC(bit, _tileId, _pipe)                                        \
        /* eplr  */                                                                     \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EPLR_E, _tileId), NULL,         \
            _PIPE(0x00000200, 0x00000204, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EPLR_ILLEGAL_OVERLAPPED_ACCESS_E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EPLR_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ermrk_SUB_TREE_MAC(bit, _tileId, _pipe)                                       \
    /* ERMRK  */                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_ERMRK_E, _tileId), NULL,        \
            _PIPE(0x00000004, 0x00000008, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_ERMRK_SUM_REGFILE_ADDRESS_ERROR_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_ERMRK_SUM_TABLE_OVERLAP_ACCESS_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_SUB_TREE_MAC(bit, _tileId, _pipe)                                             \
        /* L2 Bridge  */                                                                    \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_L2I_E, _tileId),  NULL,             \
            _PIPE(0x00002100, 0x00002104, _pipe),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_BRIDGE_ADDRESS_OUT_OF_RANGE_E,   \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_BRIDGE_UPDATE_SECURITY_BREACH_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index, _tileId, _pipe)                    \
        {(index+1), GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EQ_E, _tileId), NULL,       \
            _PIPE((0x00010020 + ((index) * 0x10)), (0x00010024 + ((index) * 0x10)),  _pipe),\
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_0_PKT_DROPED_E   + ((index) * 32),    \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_15_PKT_DROPED_E  + ((index) * 32),    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit, _tileId, _pipe)                           \
        /* SCT Rate Limiters */                                                         \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EQ_E, _tileId), NULL,           \
            _PIPE(0x00010000, 0x00010004, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_SCT_RATE_LIMITERS_REG0_CPU_CODE_RATE_LIMITER_E,         \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_SCT_RATE_LIMITERS_REG15_CPU_CODE_RATE_LIMITER_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                    \
                                                                                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9,  _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10, _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11, _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12, _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13, _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14, _tileId, _pipe),                \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15, _tileId, _pipe)

#define   eq_SUB_TREE_MAC(bit, _tileId, _pipe)                                          \
        /* Pre-Egress Interrupt Summary Cause */                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EQ_E, _tileId), NULL,           \
            _PIPE(0x00000058, 0x0000005C, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EQ_SUM_INGRESS_STC_E,        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EQ_SUM_EPORT_DLB_INTERRUPT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                     \
            eqSctRateLimiters_SUB_TREE_MAC(2, _tileId, _pipe)


#define  ha_SUB_TREE_MAC(bit, _tileId, _pipe)                                           \
        /* HA Rate Limiters */                                                          \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_HA_E, _tileId), NULL,             \
            _PIPE(0x00000300, 0x00000304, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_HA_SUM_REGFILE_ADDRESS_ERROR_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_HA_SUM_OVERSIZE_TUNNEL_HEADER_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ia_SUB_TREE_MAC(bit, _tileId, _pipe)                                          \
        /* IA */                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_IA_E, _tileId), NULL,           \
            _PIPE(0x00000500, 0x00000504, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IA_RF_ERR_E,                 \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IA_RF_ERR_E,                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ioam_SUB_TREE_MAC(bit, _tileId, _pipe)                                        \
        /* ingress OAM  */                                                              \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_IOAM_E, _tileId), NULL,         \
            _PIPE(0x000000F0, 0x000000F4, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_INGR_OAM_KEEP_ALIVE_AGING_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_INGR_OAM_TX_PERIOD_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr0_SUB_TREE_MAC(bit, _tileId, _pipe)                                       \
        /* iplr0 */                                                                     \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_IPLR_E, _tileId), NULL,         \
            _PIPE(0x00000200, 0x00000204, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPLR0_ILLEGAL_OVERLAPPED_ACCESS_E,                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPLR0_IPFIX_AGING_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr1_SUB_TREE_MAC(bit, _tileId, _pipe)                                   \
        /* iplr1 */                                                                 \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_IPLR_1_E, _tileId), NULL,   \
            _PIPE(0x00000200, 0x00000204, _pipe),                                   \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPLR1_ILLEGAL_OVERLAPPED_ACCESS_E,                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPLR1_IPFIX_AGING_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   router_SUB_TREE_MAC(bit, _tileId, _pipe)                                         \
        /* router */                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_IPVX_E, _tileId), NULL,            \
            _PIPE(0x00000970, 0x00000974, _pipe),                                          \
            prvCpssDrvHwPpPortGroupIsrRead,                                                \
            prvCpssDrvHwPpPortGroupIsrWrite,                                               \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_ROUTER_I_PV_X_BAD_ADDR_E,       \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_ROUTER_ILLEGAL_OVERLAP_ACCESS_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   lpm_SUB_TREE_MAC(bit, _tileId, _pipe)                                         \
        /* LPM */                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_LPM_E, _tileId), NULL,          \
            _PIPE(0x00F00120, 0x00F00130, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_LPM_WRONG_ADDRESS_E,         \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_LPM_MG_TO_LPM_FORBIDDEN_WRITE_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mll_SUB_TREE_MAC(bit, _tileId, _pipe)                                         \
        /* MLL */                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MLL_E, _tileId), NULL,          \
            _PIPE(0x00000030, 0x00000034, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_MLL_INVALID_IP_MLL_ACCESS_E,         \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_MLL_ILLEGAL_TABLE_OVERLAP_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pcl_SUB_TREE_MAC(bit, _tileId, _pipe)                                                \
        /* PCL */                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PCL_E, _tileId), NULL,          \
            _PIPE(0x00000004, 0x00000008, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPCL_MG_ADDR_OUT_OF_RANGE_E,         \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_IPCL_PCL_ILLEGAL_TABLE_OVERLAP_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   preq_SUB_TREE_MAC(bit, _tileId, _pipe)                                                 \
        /* PREQ */                                                                               \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PREQ_E, _tileId), NULL,                  \
                _PIPE(0x00000600, 0x00000604, _pipe),                                            \
                prvCpssDrvHwPpPortGroupIsrRead,                                                  \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                 \
                PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PREQ_REGFILE_ADDRESS_ERROR_E,    \
                PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PREQ_TABLE_ACCESS_OVERLAP_ERROR_E,       \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   qag_SUB_TREE_MAC(bit, _tileId, _pipe)                                         \
        /* qag */                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EGF_QAG_E, _tileId), NULL,      \
            _PIPE(0x000B0000, 0x000B0010, _pipe),                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_QAG_QAG_WRONG_ADDR_E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_QAG_TABLE_OVERLAPPING_INT_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_SUB_TREE_MAC(bit, _tileId, _pipe)                                              \
        /* SHT */                                                                     \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EGF_SHT_E, _tileId), NULL,    \
            _PIPE(0x000B0010, 0x000B0020, _pipe),                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                           \
            prvCpssDrvHwPpPortGroupIsrWrite,                                          \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_SHT_SHT_WRONG_ADDR_E,              \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_SHT_TABLE_OVERLAPPING_INTERRUPT_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PHA_PPN_OFFSET(ppg, ppn)   (PPG_OFFSET(ppg) + 0x4000 * (ppn))

#define PHA_PPN_NODE_MAC(bit, ppg, ppn , _tileId, _pipe)                        \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,      \
        _PIPE(0x00003040 + PHA_PPN_OFFSET(ppg, ppn),0x00003044 + PHA_PPN_OFFSET(ppg, ppn), _pipe) ,\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##ppg##_PPN##ppn##_HOST_UNMAPPED_ACCESS_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##ppg##_PPN##ppn##_DOORBELL_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(bit, ppg , _tileId, _pipe)                          \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,      \
        _PIPE(0x0007FF00 + PPG_OFFSET(ppg),0x0007FF04 + PPG_OFFSET(ppg) , _pipe), \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG##ppg##_FUNCTIONAL_SUMMARY_INTERRUPT_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG##ppg##_PPN_8_SUMMARY_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF,10, NULL, NULL},                 \
        /*  PPG Internal Error Cause */                                         \
            {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,\
            _PIPE(0x0007FF10 + PPG_OFFSET(ppg), 0x0007FF14 + PPG_OFFSET(ppg) , _pipe),  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG##ppg##_ERROR_BAD_ADDRESS_ERROR_E,          \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG##ppg##_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        PHA_PPN_NODE_MAC(2, ppg, 0, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(3, ppg, 1, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(4, ppg, 2, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(5, ppg, 3, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(6, ppg, 4, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(7, ppg, 5, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(8, ppg, 6, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(9, ppg, 7, _tileId, _pipe),                            \
        PHA_PPN_NODE_MAC(10,ppg, 8, _tileId, _pipe)

#define   phaPpa_SUB_TREE_MAC(bit, _tileId, _pipe)                              \
    /* PHA PPA */                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,      \
        _PIPE(0x007EFF10, 0x007EFF14, _pipe),                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPA_PPG_2_SUMMARY_INTERRUPT_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                 \
        /* PPA Internal Error Cause */                                          \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,    \
            _PIPE(0x007EFF20, 0x007EFF24, _pipe),                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPA_ERROR_BAD_ADDRESS_ERRROR_E,\
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPA_ERROR_BAD_ADDRESS_ERRROR_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        phaPpg_SUB_TREE_MAC(2, 0, _tileId, _pipe),                              \
        phaPpg_SUB_TREE_MAC(3, 1, _tileId, _pipe),                              \
        phaPpg_SUB_TREE_MAC(4, 2, _tileId, _pipe)


#define   phaInternal_NODE_MAC(bit, _tileId, _pipe)                             \
    /* PHA Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,      \
        _PIPE(0x007FFF80, 0x007FFF84, _pipe),                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_BAD_ADDRESS_ERROR_E,           \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_PPA_CLOCK_DOWN_VIOLATION_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_SUB_TREE_MAC(bit, _tileId, _pipe)                                                          \
        /* PHA */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PHA_E, _tileId), NULL,                           \
            _PIPE(0x007FFF88, 0x007FFF8C, _pipe),                                                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_SUM_INTERNAL_FUNCTIONAL_E,               \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_SUM_PPA_E,                               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                      \
            phaInternal_NODE_MAC(1, _tileId, _pipe),                                                     \
            phaPpa_SUB_TREE_MAC(2, _tileId, _pipe)


#define   tti_SUB_TREE_MAC(bit, _tileId, _pipe)                                                          \
        /* TTI */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TTI_E, _tileId), NULL,                           \
            _PIPE(0x00000004, 0x00000008, _pipe),                                                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,              \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_TTI_TABLE_OVERLAP_ERROR_E,                   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   hbu_SUB_TREE_MAC(bit, _tileId, _pipe)                                                          \
        /* HBU */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_HBU_E, _tileId), NULL,                           \
            _PIPE(0x00000000, 0x00000004, _pipe),                                                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_HBU_ERROR_FHF_MEM_E,                          \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_HBU_HEADER_FIFO_EMPTY_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txDma_SUB_TREE_MAC(bit, _tileId, _pipe, _unit, _index)                                         \
        /* TXD */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXDMA##_index##_E, _tileId), NULL,               \
            0x00006000, 0x00006004,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_DMA_TXD##_unit##_TXD_RF_ERR_E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_DMA_TXD##_unit##_NO_PB_DESC_CREDITS_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_SUB_TREE_MAC(bit, _tileId, _pipe, _unit, _index)                                        \
        /* TXF */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TX_FIFO##_index##_E, _tileId), NULL,             \
            0x00004000, 0x00004004,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_TXFIFIO_TXF##_unit##_TXFIFO_RF_ERR__E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_TXFIFIO_TXF##_unit##_WB_DISABLE_EXTENDED_GENERATION_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int0_SUB_TREE_MAC(bit, _tileId, _pipe, _unit, _index)                                     \
        /* RXD INT0 */                                                                                    \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, _tileId), NULL,                \
            0x00001C80, 0x00001C84,                                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_RXDMA_RF_ERR_E,                \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_OS_STATUS_L_DROP_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int1_SUB_TREE_MAC(bit, _tileId, _pipe, _unit, _index)                                     \
        /* RXD INT1 */                                                                                    \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, _tileId), NULL,                \
            0x00001C88, 0x00001C8C,                                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_PB_TAIL_ID_MEM_SER_E,          \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_RX_LOCAL_IDS_FIFO_MEM_RD_SER_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_SUB_TREE_MAC(bit, _tileId, _pipe, _unit, _index)                                                 \
        /* RXD */                                                                                                \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E, _tileId), NULL,                       \
            0x00001C90, 0x00001C94,                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_INTERRUPT_SUMMARY_RXDMA_INTERRUPT_0_E,\
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_RXDMA_##_unit##_INTERRUPT_SUMMARY_RXDMA_INTERRUPT_1_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                              \
            rxDma_int0_SUB_TREE_MAC(1, _tileId, _pipe, _unit, _index),                                           \
            rxDma_int1_SUB_TREE_MAC(2, _tileId, _pipe, _unit, _index)

#define   tai_slave_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                        \
        /* TAI */                                                                                                      \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE##_pipeId##_TAI##_index##_E, _tileId), NULL,     \
            0x00000000, 0x00000004,                                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                            \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                           \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TAI##_index##_SLAVE_GENERATION_INT_E,                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TAI##_index##_SLAVE_SER_RX_LINK_FAULT_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   tai_SUB_TREE_MAC(bit, _tileId, index)                                                          \
        /* TAI */                                                                                        \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TAI##index##_E, _tileId), NULL,                  \
            0x00000000, 0x00000004,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_TAI##index##_GENERATION_INT_E,                              \
            PRV_CPSS_FALCON_TILE_##_tileId##_TAI##index##_SER_RX_LINK_FAULT_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_chan_SUB_TREE_MAC(bit, _tileId, _gpcPr, _gpcInst, _chan)                                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                                       \
        0x00000C00 + (bit-1)*4, 0x00000D00 + (bit-1)*4,                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                       \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_CHANNEL##_chan##_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E,\
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_CHANNEL##_chan##_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_pr_pc_SUB_TREE_MAC(bit, _tileId, _gpcPr, _gpcInst)                                                                             \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                                      \
        0x00000E00, 0x00000E04,                                                                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                       \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_COUNT_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_READ_COUNT_E,\
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_COUNT_NULL_ADDRESS_RECEIVED_PACKET_COUNT_CELL_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_misc_SUB_TREE_MAC(bit, _tileId, _gpcPr, _gpcInst)                                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                           \
        0x00000E08, 0x00000E0C,                                                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                            \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                           \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_MISC_NULL_ADDRESS_RECEIVED_PACKET_READ_CELL_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_pr_cc_SUB_TREE_MAC(bit, _tileId, _gpcPr, _gpcInst)                                                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                                                              \
        0x00000E10, 0x00000E14,                                                                                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                                               \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                                              \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_pr_pc_cc_SUB_TREE_MAC(bit, _tileId, _gpcPr, _gpcInst)                                                                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                                                            \
        0x00000E18, 0x00000E1C,                                                                                                                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                                            \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E,\
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_gpcInst##_GRP##_gpcPr##_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_gr_pr_SUB_TREE_MAC(bit, _tileId, _gpcPr)                                                                             \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                         \
        0x00002008, 0x000200C,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_GRP##_gpcPr##_SUM_INTERRUPT_SUMMARY_PACKET_READ_GROUP_E,                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_GRP##_gpcPr##_SUM_INTERRUPT_SUMMARY_PACKET_READ_1_E,                                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                                                  \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                       \
            0x00002000, 0x00002004,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_GRP##_gpcPr##_ILLEGAL_ADDRESS_ACCESS_E,                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_GRP##_gpcPr##_ILLEGAL_ADDRESS_ACCESS_E,                                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                              \
        {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                       \
            0x00000E20, 0x00000E24,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_GRP##_gpcPr##_PACKET_READ_SUM_INT_CHANNEL_SUMMARY_0_E,                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_GRP##_gpcPr##_PACKET_READ_SUM_INT_PACKET_COUNT_CREDIT_COUNTER_SUMMARY_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 14, NULL, NULL},                                                             \
        gpc_pr_chan_SUB_TREE_MAC (1, _tileId,  _gpcPr, 0, 0),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (2, _tileId,  _gpcPr, 0, 1),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (3, _tileId,  _gpcPr, 0, 2),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (4, _tileId,  _gpcPr, 0, 3),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (5, _tileId,  _gpcPr, 0, 4),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (6, _tileId,  _gpcPr, 0, 5),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (7, _tileId,  _gpcPr, 0, 6),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (8, _tileId,  _gpcPr, 0, 7),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (9, _tileId,  _gpcPr, 0, 8),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (10, _tileId,  _gpcPr, 0, 9),                                                     \
        gpc_pr_pr_pc_SUB_TREE_MAC(11, _tileId,  _gpcPr, 0),                                                        \
        gpc_pr_misc_SUB_TREE_MAC (12, _tileId,  _gpcPr, 0),                                                        \
        gpc_pr_pr_cc_SUB_TREE_MAC(13, _tileId,  _gpcPr, 0),                                                        \
        gpc_pr_pc_cc_SUB_TREE_MAC(14, _tileId,  _gpcPr, 0),                                                        \
        {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_##_gpcPr##_E, _tileId), NULL,                       \
            0x00001E20, 0x00001E24,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_GRP##_gpcPr##_PACKET_READ_SUM_INT_CHANNEL_SUMMARY_0_E,                        \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_GRP##_gpcPr##_PACKET_READ_SUM_INT_PACKET_COUNT_CREDIT_COUNTER_SUMMARY_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 14, NULL, NULL},                                                             \
        gpc_pr_chan_SUB_TREE_MAC (1, _tileId,  _gpcPr, 1, 0),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (2, _tileId,  _gpcPr, 1, 1),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (3, _tileId,  _gpcPr, 1, 2),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (4, _tileId,  _gpcPr, 1, 3),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (5, _tileId,  _gpcPr, 1, 4),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (6, _tileId,  _gpcPr, 1, 5),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (7, _tileId,  _gpcPr, 1, 6),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (8, _tileId,  _gpcPr, 1, 7),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (9, _tileId,  _gpcPr, 1, 8),                                                      \
        gpc_pr_chan_SUB_TREE_MAC (10, _tileId,  _gpcPr, 1, 9),                                                     \
        gpc_pr_pr_pc_SUB_TREE_MAC(11, _tileId,  _gpcPr, 1),                                                        \
        gpc_pr_misc_SUB_TREE_MAC (12, _tileId,  _gpcPr, 1),                                                        \
        gpc_pr_pr_cc_SUB_TREE_MAC(13, _tileId,  _gpcPr, 1),                                                        \
        gpc_pr_pc_cc_SUB_TREE_MAC(14, _tileId,  _gpcPr, 1)

#define gpc_gr_cr_SUB_TREE_MAC(bit, _tileId, _gpcCr)                                                                             \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_gpcCr##_E, _tileId), NULL,                           \
        0x00000208, 0x000020C,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_GPC_CR##_gpcCr##_INT_SUM_CELL_READ_GROUP_SUMMARY_E,                                     \
        PRV_CPSS_FALCON_TILE_##_tileId##_GPC_CR##_gpcCr##_INT_SUM_CELL_READ_1_SUMMARY_E,                                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                                                  \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_gpcCr##_E, _tileId), NULL,                         \
            0x00000200, 0x00000204,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC_CR##_gpcCr##_ILLEGAL_ADDRESS_ACCESS_E,                                          \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC_CR##_gpcCr##_ILLEGAL_ADDRESS_ACCESS_E,                                          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                              \
        {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_gpcCr##_E, _tileId), NULL,                         \
            0x000000A0, 0x000000A4,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC0_CR##_gpcCr##_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E,                   \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC0_CR##_gpcCr##_ILLEGAL_ADDRESS_ACCESS_E,                                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                              \
        {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_##_gpcCr##_E, _tileId), NULL,                         \
            0x000001A0, 0x000001A4,                                                                                              \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                                      \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                                     \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC1_CR##_gpcCr##_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E,                   \
            PRV_CPSS_FALCON_TILE_##_tileId##_GPC1_CR##_gpcCr##_ILLEGAL_ADDRESS_ACCESS_E,                                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define smb_write_arbiter_SUB_TREE_MAC(bit, _tileId)                                                                             \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E, _tileId), NULL,                                      \
        0x00000100, 0x0000104,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_WRITE_ARBITER_MISC_INTERRUPT_ILLEGAL_ADDRESS_E,                                  \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_WRITE_ARBITER_MISC_INTERRUPT_MC_CA_FIFO_OVERFLOW_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_gr_pw_gr_SUB_TREE_MAC(bit, _tileId, _gpcPw)                                                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_##_gpcPw##_E, _tileId), NULL,                        \
        0x00001000, 0x00001004,                                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_ILLEGAL_ADDRESS_ACCESS_E,                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_ILLEGAL_ADDRESS_ACCESS_E,                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gpc_gr_pw_inst_SUB_TREE_MAC(bit, _tileId, _gpcPw, _pwInst)                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_##_gpcPw##_E, _tileId), NULL,                        \
        0x00000200, 0x00000204,                                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_##_pwInst##_INT_CAUSE_RSVD_0_E,                           \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_##_pwInst##_INT_CAUSE_ADDR_ERR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define gpc_gr_pw_SUB_TREE_MAC(bit, _tileId, _gpcPw)                                                                             \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_##_gpcPw##_E, _tileId), NULL,                        \
        0x00001008, 0x000100C,                                                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_SUM_PACKET_WRITE_0_E,                                     \
        PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC##_gpcPw##_PACKET_WRITE_SUM_PACKET_WRITE_GROUP_E,                                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                                                  \
        gpc_gr_pw_inst_SUB_TREE_MAC(1, _tileId, _gpcPw, 0),                                                                      \
        gpc_gr_pw_inst_SUB_TREE_MAC(2, _tileId, _gpcPw, 1),                                                                      \
        gpc_gr_pw_inst_SUB_TREE_MAC(3, _tileId, _gpcPw, 2),                                                                      \
        gpc_gr_pw_inst_SUB_TREE_MAC(4, _tileId, _gpcPw, 3),                                                                      \
        gpc_gr_pw_gr_SUB_TREE_MAC(5, _tileId, _gpcPw)

#define   packetBuffer_SUB_TREE_MAC(bit, _tileId)                                                        \
        /* Packet Buffer */                                                                              \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,                 \
            0x00001120, 0x00001124,                                                                      \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTERS_E,                 \
            PRV_CPSS_FALCON_TILE_##_tileId##_PB_INTERRUPT_SUMMARY_PB_CENTER_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                      \
            {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00001108, 0x0000110C,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_SUM_MEMORY_CLUSTER_0_E,                          \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_NPM_SUM_MEMORY_CLUSTER_2_E,                          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                  \
            npm_SUB_TREE_MAC(1,_tileId, 0),                                                              \
            npm_SUB_TREE_MAC(2,_tileId, 1),                                                              \
            npm_SUB_TREE_MAC(3,_tileId, 2),                                                              \
            {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00001110, 0x00001114,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC_SUM_PACKET_WRITE_GPC_0_E,                        \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_GPC_SUM_CELL_READ_GPC_3_E,                           \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                 \
            gpc_gr_pw_SUB_TREE_MAC(1, _tileId,  0),                                                      \
            gpc_gr_pw_SUB_TREE_MAC(2, _tileId,  1),                                                      \
            gpc_gr_pr_SUB_TREE_MAC(3, _tileId,  0),                                                      \
            gpc_gr_pr_SUB_TREE_MAC(4, _tileId,  1),                                                      \
            gpc_gr_pr_SUB_TREE_MAC(5, _tileId,  2),                                                      \
            gpc_gr_pr_SUB_TREE_MAC(6, _tileId,  3),                                                      \
            gpc_gr_cr_SUB_TREE_MAC(7, _tileId,  0),                                                      \
            gpc_gr_cr_SUB_TREE_MAC(8, _tileId,  1),                                                      \
            gpc_gr_cr_SUB_TREE_MAC(9, _tileId,  2),                                                      \
            gpc_gr_cr_SUB_TREE_MAC(10, _tileId, 3),                                                      \
            {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00001118, 0x0000111C,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_SUM_MEMORY_CLUSTER_0_E,                          \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_SUM_MEMORY_CLUSTER_5_E,                          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                  \
            smb_SUB_TREE_MAC(1,_tileId, 0, 0),                                                           \
            smb_SUB_TREE_MAC(2,_tileId, 0, 1),                                                           \
            smb_SUB_TREE_MAC(3,_tileId, 1, 0),                                                           \
            smb_SUB_TREE_MAC(4,_tileId, 1, 1),                                                           \
            smb_SUB_TREE_MAC(5,_tileId, 2, 0),                                                           \
            smb_SUB_TREE_MAC(6,_tileId, 2, 1),                                                           \
            {4, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00000110, 0x00000114,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_WRITE_ARBITER_SUM_INTERRUPT_SUM_MISC_E,          \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_SMB_WRITE_ARBITER_SUM_INTERRUPT_SUM_MISC_E,          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                  \
            smb_write_arbiter_SUB_TREE_MAC(1,_tileId),                                                   \
            {5, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00000190, 0x00000194,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_COUNTER_ILLEGAL_ADDRESS_ACCESS_E,                    \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_COUNTER_ILLEGAL_ADDRESS_ACCESS_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                  \
            {6, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, _tileId), NULL,               \
                0x00001100, 0x00001104,                                                                  \
                prvCpssDrvHwPpPortGroupIsrRead,                                                          \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_CENTER_ILLEGAL_ADDRESS_ACCESS_E,                     \
                PRV_CPSS_FALCON_TILE_##_tileId##_PB_CENTER_ILLEGAL_ADDRESS_ACCESS_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define tx_0_3_units(_tileId, _pipe)                                                                     \
         txDma_SUB_TREE_MAC(23,_tileId, _pipe, 0, 0),                                                    \
         txFifo_SUB_TREE_MAC(24,_tileId, _pipe, 0, 0),                                                   \
         txDma_SUB_TREE_MAC(25,_tileId, _pipe, 1, 1),                                                    \
         txFifo_SUB_TREE_MAC(26,_tileId, _pipe, 1, 1),                                                   \
         txDma_SUB_TREE_MAC(27,_tileId, _pipe, 2, 2),                                                    \
         txFifo_SUB_TREE_MAC(28,_tileId, _pipe, 2, 2),                                                   \
         txDma_SUB_TREE_MAC(29,_tileId, _pipe, 3, 3),                                                    \
         txFifo_SUB_TREE_MAC(30,_tileId, _pipe, 3, 3),                                                   \
         tai_slave_SUB_TREE_MAC(31,_tileId, _pipe, 0)

#define tx_4_7_units(_tileId, _pipe)                                                                     \
         txDma_SUB_TREE_MAC(23,_tileId, _pipe, 0, 4),                                                    \
         txFifo_SUB_TREE_MAC(24,_tileId, _pipe, 0, 4),                                                   \
         txDma_SUB_TREE_MAC(25,_tileId, _pipe, 1, 5),                                                    \
         txFifo_SUB_TREE_MAC(26,_tileId, _pipe, 1, 5),                                                   \
         txDma_SUB_TREE_MAC(27,_tileId, _pipe, 2, 6),                                                    \
         txFifo_SUB_TREE_MAC(28,_tileId, _pipe, 2, 6),                                                   \
         txDma_SUB_TREE_MAC(29,_tileId, _pipe, 3, 7),                                                    \
         txFifo_SUB_TREE_MAC(30,_tileId, _pipe, 3, 7),                                                   \
         tai_slave_SUB_TREE_MAC(31,_tileId, _pipe, 1)

#define rx_0_3_units(_tileId)                                                                            \
         rxDma_SUB_TREE_MAC((1+((_tileId%2)*8)),_tileId, 0, 0, 0),                                       \
         rxDma_SUB_TREE_MAC((2+((_tileId%2)*8)),_tileId, 0, 1, 1),                                       \
         rxDma_SUB_TREE_MAC((3+((_tileId%2)*8)),_tileId, 0, 2, 2),                                       \
         rxDma_SUB_TREE_MAC((4+((_tileId%2)*8)),_tileId, 0, 3, 3)

#define rx_4_7_units(_tileId)                                                                            \
         rxDma_SUB_TREE_MAC((5+((_tileId%2)*8)),_tileId, 1, 0, 4),                                       \
         rxDma_SUB_TREE_MAC((6+((_tileId%2)*8)),_tileId, 1, 1, 5),                                       \
         rxDma_SUB_TREE_MAC((7+((_tileId%2)*8)),_tileId, 1, 2, 6),                                       \
         rxDma_SUB_TREE_MAC((8+((_tileId%2)*8)),_tileId, 1, 3, 7)

#define   erep_SUB_TREE_MAC(bit, _tileId, _pipe)                                                         \
        /* EREP Rate Limiters */                                                                         \
        {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EREP_E, _tileId), NULL,                          \
            _PIPE(0x00003000, 0x00003004, _pipe),                                                        \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,              \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_EREP_MIRROR_REPLICATION_NOT_PERFORMED_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


/*21 elements */
#define cp_units(_tileId, _pipe)                                                                         \
        bma_SUB_TREE_MAC(1, _tileId, _pipe),                                                             \
        eft_SUB_TREE_MAC(2, _tileId, _pipe),                                                             \
        eoam_SUB_TREE_MAC(3, _tileId, _pipe),                                                            \
        epcl_SUB_TREE_MAC(4, _tileId, _pipe),                                                            \
        eplr_SUB_TREE_MAC(5,_tileId, _pipe),                                                             \
        eq_SUB_TREE_MAC(6,_tileId, _pipe),                                                               \
        erep_SUB_TREE_MAC(7,_tileId, _pipe),                                                             \
        ha_SUB_TREE_MAC(8, _tileId, _pipe),                                                              \
        hbu_SUB_TREE_MAC(9,_tileId, _pipe),                                                              \
        ia_SUB_TREE_MAC(10, _tileId, _pipe),                                                             \
        ioam_SUB_TREE_MAC(11, _tileId, _pipe),                                                           \
        iplr0_SUB_TREE_MAC(12, _tileId, _pipe),                                                          \
        iplr1_SUB_TREE_MAC(13, _tileId, _pipe),                                                          \
        router_SUB_TREE_MAC(14, _tileId, _pipe),                                                         \
        lpm_SUB_TREE_MAC(15, _tileId, _pipe),                                                            \
        mll_SUB_TREE_MAC(16, _tileId, _pipe),                                                            \
        pcl_SUB_TREE_MAC(17, _tileId, _pipe),                                                            \
        preq_SUB_TREE_MAC(18,_tileId, _pipe),                                                            \
        qag_SUB_TREE_MAC(19,_tileId, _pipe),                                                             \
        sht_SUB_TREE_MAC(20,_tileId, _pipe),                                                             \
        pha_SUB_TREE_MAC(21,_tileId, _pipe),                                                             \
        tti_SUB_TREE_MAC(22,_tileId, _pipe)

/*10 elements */
#define cnm_func_units(_tileId)                                                                          \
        packetBuffer_SUB_TREE_MAC(22, _tileId),                                                          \
        em_SUB_TREE_MAC(23, _tileId),                                                                    \
        mt_SUB_TREE_MAC(24, _tileId),                                                                    \
        packer0_SUB_TREE_MAC(25, _tileId),                                                               \
        packer1_SUB_TREE_MAC(26, _tileId),                                                               \
        pdx_SUB_TREE_MAC(27, _tileId),                                                                   \
        pfcc_SUB_TREE_MAC(28, _tileId),                                                                  \
        psi_SUB_TREE_MAC(29, _tileId),                                                                   \
        shm_SUB_TREE_MAC(30, _tileId),                                                                   \
        tcam_SUB_TREE_MAC(31,_tileId)

#define   cnc_SUB_TREE_MAC(bit,_tileId, pipe, instance)                                                  \
    /* CNC-0  */                                                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, _tileId), NULL,                  \
        _PIPE(0x00000100, 0x00000104 , pipe),                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_SUM_WRAPAROUND_FUNC_SUM_E,         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_SUM_MISC_FUNC_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                                           \
                                                                                                         \
        /* WraparoundFuncInterruptSum  */                                                                \
        {1, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, _tileId), NULL,                \
            _PIPE(0x00000190, 0x000001A4 ,  pipe),                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                       \
        /* RateLimitFuncInterruptSum  */                                                                 \
        {2, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, _tileId), NULL,                \
            _PIPE(0x000001B8, 0x000001CC ,  pipe),                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_RATE_LIMIT_SUM_BLOCK0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_RATE_LIMIT_SUM_BLOCK15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                       \
        /* MiscFuncInterruptSum  */                                                                      \
        {3, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, _tileId), NULL,                \
            _PIPE(0x000001E0, 0x000001E4 ,  pipe),                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                              \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_MISC_SUM_DUMP_FINISHED_E,      \
            PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##pipe##_CNC##instance##_MISC_SUM_CNC_UPDATE_LOST_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId,_nodes,_mgIndex)         \
        /* FuncUnitsIntsSum Interrupt Cause */                                              \
        {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E,_tileId,_mgIndex), NULL,     \
            FUNC_UNITS_CAUSE_MASK(_treeId),                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                \
            PRV_CPSS_FALCON_TILE_##_tileId##_##MG##_mgIndex##_GPIO_1_0_FROM_RAVEN_0_0_E,    \
            PRV_CPSS_FALCON_TILE_##_tileId##_##MG##_mgIndex##_CENTRAL_TOP_TCAM_INT_SUM_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, _nodes, NULL, NULL},                             \
            raven_SUB_TREE_MAC(1, _tileId,    0),                                          \
            raven_SUB_TREE_MAC(2, _tileId,    1),                                          \
            raven_SUB_TREE_MAC(3, _tileId,    2),                                          \
            raven_SUB_TREE_MAC(4, _tileId,    3)

#define cnm_grp_0_0_primary_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)     \
        cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, 18, _mgIndex),                \
                raven_SUB_TREE_MAC(5, _tile1,    3),                                          \
                raven_SUB_TREE_MAC(6, _tile1,    2),                                          \
                raven_SUB_TREE_MAC(7, _tile1,    1),                                          \
                raven_SUB_TREE_MAC(8, _tile1,    0),                                          \
                cnm_func_units(_tileId)

/* FuncUnitsIntsSum Interrupt Cause (that supports tiles 2,3) */
#define cnm_grp_0_0_primary_for_4_tiles_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)      \
        cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, 19, _mgIndex),                  \
        raven_SUB_TREE_MAC(5, _tile1,    3),                                          \
        raven_SUB_TREE_MAC(6, _tile1,    2),                                          \
        raven_SUB_TREE_MAC(7, _tile1,    1),                                          \
        raven_SUB_TREE_MAC(8, _tile1,    0),                                          \
        /* pointer to the second dual tile */                                         \
        FALCON_MG_INTERRUPTS_DUAL_TILE_TREE1_MAC(falcon_other_dual_int_sum/*bitIndexInCaller*/, 1/*tree*/, 2/*primary tile*/, 11/*numChild*/, 3/*secondary tile*/),\
        cnm_func_units(_tileId)


#define cnm_grp_0_0_single_tile_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)          \
        cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, 14, _mgIndex),                 \
        cnm_func_units(_tileId)

#define pds_DBG_INT_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                                                        \
    /* PDS Interrupt Summary */                                                                                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_index##_E,_tileId), NULL,                                                          \
        _PIPE(0x00032010, 0x00032014, _pipeId),                                                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                                               \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_DEBUG_PB_WRITE_REPLY_FIFO_FULL_E,                                           \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_DEBUG_LENGTH_ADJUST_CONSTANT_BIGGER_THAN_THE_PACKETS_BYTE_COUNT_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pds_FUNC_INT_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                \
    /* PDS Interrupt Summary */                                                                                 \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_index##_E,_tileId), NULL,                   \
        _PIPE(0x00032008, 0x0003200C, _pipeId),                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_FUNC_BAD_ADDRESS_INT_E,              \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_FUNC_PB_ECC_DOUBLE_ERROR_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pds_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                 \
    /* PDS Interrupt Summary */                                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_index##_E,_tileId), NULL,           \
        _PIPE(0x00032000, 0x00032004, _pipeId),                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_INT_SUM_FUNCTIONAL_INTSUM_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_PDS##_index##_INT_SUM_DEBUG_INTSUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        pds_FUNC_INT_SUB_TREE_MAC(1,_tileId,_pipeId,_index),                                            \
        pds_DBG_INT_SUB_TREE_MAC(2,_tileId,_pipeId,_index)

/* 8 elements */
#define pds_units(_tileId)                                                                                   \
        pds_SUB_TREE_MAC(1, _tileId, 0, 0),                                                                  \
        pds_SUB_TREE_MAC(2, _tileId, 0, 1),                                                                  \
        pds_SUB_TREE_MAC(3, _tileId, 0, 2),                                                                  \
        pds_SUB_TREE_MAC(4, _tileId, 0, 3),                                                                  \
        pds_SUB_TREE_MAC(5, _tileId, 1, 0),                                                                  \
        pds_SUB_TREE_MAC(6, _tileId, 1, 1),                                                                  \
        pds_SUB_TREE_MAC(7, _tileId, 1, 2),                                                                  \
        pds_SUB_TREE_MAC(8, _tileId, 1, 3)

#define qfc_FUNC_INT_NODE_MAC(bit, _tileId, _pipeId, _index)                                                 \
    /* QFC Functional Interrupt Summary */                                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_index##_E,_tileId), NULL,                \
        _PIPE(0x00000100, 0x00000104, _pipeId),                                                              \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                      \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                     \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_FUNC_UBURST_EVENT_INT_E,          \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_FUNC_AGG_BUFFER_COUNT_OVERFLOW_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_DBG_INT_NODE_MAC(bit, _tileId, _pipeId, _index)                                                                      \
    /* QFC Debug Interrupt Summary */                                                                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_index##_E,_tileId), NULL,                                    \
        _PIPE(0x00000108, 0x0000010C, _pipeId),                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                         \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_DEBUG_PFC_INT_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_DEBUG_PFC_INT_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_HR_INT_NODE_MAC(bit, _tileId, _pipeId, _index,_regIndex)                                                      \
    /* QFC HR Crossed Threshold Interrupt Summary */                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_index##_E,_tileId), NULL,                             \
        _PIPE((0x0000011C+(_regIndex*8)), (0x00000120+(_regIndex*8)), _pipeId),                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                   \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                  \
        PRV_CPSS_FALCON_TILE_##_tileId##_##_pipeId##_TXQ_##_index##_INT_##_regIndex##_PORT_0_TC_0_CROSSED_HR_THRESHOLD_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_##_pipeId##_TXQ_##_index##_INT_##_regIndex##_PORT_2_TC_7_CROSSED_HR_THRESHOLD_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define qfc_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                                  \
    /* QFC Interrupt Summary */                                                                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_index##_E,_tileId), NULL,                            \
        _PIPE(0x00000110, 0x00000114, _pipeId),                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_INT_SUM_FUNCTIONAL_INTSUM_E,                  \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_QFC##_index##_INT_SUM_HR_CROSSED_THRESHOLD_REG2_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                                          \
        qfc_FUNC_INT_NODE_MAC(1,_tileId,_pipeId,_index),                                                                 \
        qfc_DBG_INT_NODE_MAC(2,_tileId,_pipeId,_index),                                                                  \
        qfc_HR_INT_NODE_MAC(3,_tileId,_pipeId,_index,0),                                                                 \
        qfc_HR_INT_NODE_MAC(4,_tileId,_pipeId,_index,1),                                                                 \
        qfc_HR_INT_NODE_MAC(5,_tileId,_pipeId,_index,2)

/* 8 elements */
#define qfc_units(_tileId)                                                                                   \
        qfc_SUB_TREE_MAC( 9, _tileId, 0, 0),                                                                 \
        qfc_SUB_TREE_MAC(10, _tileId, 0, 1),                                                                 \
        qfc_SUB_TREE_MAC(11, _tileId, 0, 2),                                                                 \
        qfc_SUB_TREE_MAC(12, _tileId, 0, 3),                                                                 \
        qfc_SUB_TREE_MAC(13, _tileId, 1, 0),                                                                 \
        qfc_SUB_TREE_MAC(14, _tileId, 1, 1),                                                                 \
        qfc_SUB_TREE_MAC(15, _tileId, 1, 2),                                                                 \
        qfc_SUB_TREE_MAC(16, _tileId, 1, 3)

#define sdq_DBG_INT_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                \
    /* SDQ Interrupt Summary */                                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_index##_E,_tileId), NULL,                  \
        _PIPE(0x00000210, 0x00000214, _pipeId),                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_DEBUG_QCN_FIFO_OVERRUN_E,           \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_DEBUG_QCN_FIFO_OVERRUN_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sdq_FUNC_INT_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                               \
    /* SDQ Interrupt Summary */                                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_index##_E,_tileId), NULL,                  \
        _PIPE(0x00000208, 0x0000020C, _pipeId),                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_FUNC_ILLEGAL_ADDRESS_ACCESS_E,      \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_FUNC_SEL_PORT_OUTOFRANGE_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sdq_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                        \
    /* SDQ Interrupt Summary */                                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_index##_E,_tileId), NULL,                  \
        _PIPE(0x00000200, 0x00000204, _pipeId),                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_SUM_FUNCTIONAL_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_SDQ##_index##_SUM_DEBUG_INTERRUPT_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                                \
        sdq_FUNC_INT_SUB_TREE_MAC(1,_tileId,_pipeId,_index),                                                   \
        sdq_DBG_INT_SUB_TREE_MAC(2,_tileId,_pipeId,_index)

/* 8 elements */
#define sdq_units(_tileId)                                                                                   \
        sdq_SUB_TREE_MAC(17, _tileId, 0, 0),                                                                 \
        sdq_SUB_TREE_MAC(18, _tileId, 0, 1),                                                                 \
        sdq_SUB_TREE_MAC(19, _tileId, 0, 2),                                                                 \
        sdq_SUB_TREE_MAC(20, _tileId, 0, 3),                                                                 \
        sdq_SUB_TREE_MAC(21, _tileId, 1, 0),                                                                 \
        sdq_SUB_TREE_MAC(22, _tileId, 1, 1),                                                                 \
        sdq_SUB_TREE_MAC(23, _tileId, 1, 2),                                                                 \
        sdq_SUB_TREE_MAC(24, _tileId, 1, 3)

#define txq_tai_slave_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                               \
    /* TAI slave Interrupt Summary */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI##_index##_E, _tileId), NULL,     \
        _PIPE(0x00000000, 0x00000004, _pipeId),                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_TAI##_index##_SLAVE_GENERATION_INT_E,          \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_TXQ_TAI##_index##_SLAVE_SER_RX_LINK_FAULT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* 4 elements */
#define txq_tai_slave_units(_tileId)                                                                           \
        txq_tai_slave_SUB_TREE_MAC(25, _tileId, 0, 0),                                                         \
        txq_tai_slave_SUB_TREE_MAC(26, _tileId, 0, 1),                                                         \
        txq_tai_slave_SUB_TREE_MAC(27, _tileId, 1, 0),                                                         \
        txq_tai_slave_SUB_TREE_MAC(28, _tileId, 1, 1)

#define eagle_side_d2d_cp_NODE_MAC(bit, _tileId)                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E, _tileId), NULL,         \
        0x00000050, 0x00000054,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_D2D_CP_D2X_REQ_TIMEOUT_CNT_EXPIRED_E,                       \
        PRV_CPSS_FALCON_TILE_##_tileId##_D2D_CP_TAP_TIMEOUT_TRIGGERED_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_SUB_TREE_MAC(bit, _tileId)                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, _tileId), NULL,                    \
        0x000F8108, 0x000F810C,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,           \
        PRV_CPSS_FALCON_TILE_##_tileId##_DFX_SERVER_INTERRUPT_READ_FROM_MC_BC_ADDRESS_FROM_PIPE_7_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_sum_SUB_TREE_MAC(bit, _tileId)                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, _tileId), NULL,                    \
        0x000F8100, 0x000F8104,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,        \
        PRV_CPSS_FALCON_TILE_##_tileId##_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                      \
        dfx_server_SUB_TREE_MAC(1, _tileId)

#define d2d_serdes_mac_error_NODE_MAC(bit, _tileId, _pipeId, _index)                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _index, _tileId), NULL,  \
        _PIPE(0x0000603C, 0x00006038, _pipeId),                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_SERDES_MAC_ERR_BAD_ADDRESS_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_SERDES_MAC_ERR_TX_CHILD_ERROR_7_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define d2d_mac_rx_NODE_MAC(bit, _tileId, _pipeId, _index)                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _index, _tileId), NULL,  \
        _PIPE(0x00004F08, 0x00004F04, _pipeId),                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_MAC_RX_ERR_TX_FIFO_OVERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_MAC_RX_ERR_MAC_DEBUG_CHECK_RF_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define d2d_pcs_event_NODE_MAC(bit, _tileId, _pipeId, _index)                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _index, _tileId), NULL,  \
        _PIPE(0x00007040, 0x00007044, _pipeId),                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define d2d_pma_event_NODE_MAC(bit, _tileId, _pipeId, _index)                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E +  _index, _tileId), NULL,  \
        _PIPE(0x00008060, 0x00008064, _pipeId),                                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_PMA_ERR_RX_CDC_OVERFLOW_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_PMA_ERR_RX_FEC_UNCOR_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define d2d_mac_tx_SUB_TREE_MAC(bit, _tileId, _pipeId, _index)                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + _index, _tileId), NULL,  \
        _PIPE(0x00004600, 0x000045FC, _pipeId),                                                               \
        prvCpssDrvHwPpPortGroupIsrRead_eagle_d2d_mac_tx_SUB_TREE_MAC,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_MAC_TX_ERR_MAC_TX_RF_CAUSE_E, \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_index##_MAC_TX_ERR_RESERVED31_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                                      \
 /*fake!*/       d2d_serdes_mac_error_NODE_MAC(16,_tileId, _pipeId, _index),                                   \
 /*fake!*/       d2d_mac_rx_NODE_MAC(17, _tileId, _pipeId, _index),                                            \
 /*fake!*/       d2d_pcs_event_NODE_MAC(18, _tileId, _pipeId, _index),                                         \
 /*fake!*/       d2d_pma_event_NODE_MAC(19, _tileId, _pipeId, _index)

/* 8 elements */
#define d2d_mac_tx_units(_tileId)                                                                     \
        d2d_mac_tx_SUB_TREE_MAC(1, _tileId, 0, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(2, _tileId, 0, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(3, _tileId, 0, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(4, _tileId, 0, 3),                              \
        d2d_mac_tx_SUB_TREE_MAC(5, _tileId, 1, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(6, _tileId, 1, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(7, _tileId, 1, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(8, _tileId, 1, 3)

/* 16 elements */
#define d2d_mac_tx_units_dual(_tileId, tile1)                                                                     \
        d2d_mac_tx_SUB_TREE_MAC(1,  _tileId, 0, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(2,  _tileId, 0, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(3,  _tileId, 0, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(4,  _tileId, 0, 3),                              \
        d2d_mac_tx_SUB_TREE_MAC(5,  _tileId, 1, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(6,  _tileId, 1, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(7,  _tileId, 1, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(8,  _tileId, 1, 3),                              \
        d2d_mac_tx_SUB_TREE_MAC(9,    tile1, 0, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(10,   tile1, 0, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(11,   tile1, 0, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(12,   tile1, 0, 3),                              \
        d2d_mac_tx_SUB_TREE_MAC(13,   tile1, 1, 0),                              \
        d2d_mac_tx_SUB_TREE_MAC(14,   tile1, 1, 1),                              \
        d2d_mac_tx_SUB_TREE_MAC(15,   tile1, 1, 2),                              \
        d2d_mac_tx_SUB_TREE_MAC(16,   tile1, 1, 3)

#define cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* DataPathIntsSum Interrupt Cause */                                                                \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                   \
        DATA_PATH_CAUSE_MASK(_treeId),                                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TOP_BMA_INT_SUM_E,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TOP_TAI_INT_SUM_0_E,                                      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                  \
        cp_units(_tileId,0),                                                                             \
        tx_0_3_units(_tileId, 0)



#define cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* PortsIntsSum Interrupt Cause */                                                                   \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        PORTS_CAUSE_MASK(_treeId),                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TOP_BMA_INT_SUM_E,                                        \
        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TOP_TAI_INT_SUM_0_E,                                      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                  \
        cp_units(_tileId, 1),                                                                            \
        tx_4_7_units(_tileId, 1)

#define cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex , _tile1)                       \
    /* DFX */                                                                                            \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        DFX_CAUSE_MASK(_treeId),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_for_tile_##_tile1##_PIPE0_TOP_BMA_INT_SUM_E,                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_for_tile_##_tile1##_PIPE0_TOP_TAI_INT_SUM_0_E,                  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                  \
        cp_units(_tile1, 0),                                                                             \
        tx_0_3_units(_tile1, 0)

#define cnm_grp_0_4_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex , _tile1)                       \
    /* DFX1 */                                                                                           \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        DFX1_CAUSE_MASK(_treeId),                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_for_tile_##_tile1##_PIPE1_TOP_BMA_INT_SUM_E,                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_for_tile_##_tile1##_PIPE1_TOP_TAI_INT_SUM_0_E,                  \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 31, NULL, NULL},                                                  \
        cp_units(_tile1, 1),                                                                             \
        tx_4_7_units(_tile1, 1)

#define cnm_grp_0_5_IntsSum_single_tile_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                    \
    /* FuncUnits1IntsSum */                                                                              \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        FUNC1_UNITS_CAUSE_MASK(_treeId),                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_5_TILE0_DMA_TOP_RX0_RXDMA_CAUSE_INT_SUM,\
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_5_TILE0_TOP_PIPE1_TAI_INT_SUM_1_E,     \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 8, NULL, NULL},                                                   \
        rx_0_3_units(_tileId),                                                                           \
        rx_4_7_units(_tileId)
        /*txTai_SUB_TREE_MAC(17,_tileId, 0),*/
        /*txTai_SUB_TREE_MAC(18,_tileId, 1) */


#define cnm_grp_0_5_primary_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)                         \
    /* FuncUnits1IntsSum */                                                                              \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        FUNC1_UNITS_CAUSE_MASK(_treeId),                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_5_TILE0_DMA_TOP_RX0_RXDMA_CAUSE_INT_SUM,\
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_5_CENTRAL_TOP_TCAM_INT_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 26, NULL, NULL},                                                  \
        rx_0_3_units(_tileId),                                                                           \
        rx_4_7_units(_tileId),                                                                           \
        rx_0_3_units(_tile1),                                                                            \
        rx_4_7_units(_tile1),                                                                            \
/*      txTai_SUB_TREE_MAC(17,_tileId, 0),*/                                                             \
/*      txTai_SUB_TREE_MAC(18,_tileId, 1),*/                                                             \
/*      txTai_SUB_TREE_MAC(19,_tile1 , 0),*/                                                             \
/*      txTai_SUB_TREE_MAC(20,_tile1 , 1),*/                                                             \
        cnm_func_units(_tile1)


#define cnm_grp_0_6_primary_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)                          \
        cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex,_tile1)

/* support for MG4 (empty on this register -- 0 nodes) */
/* NOTE: this macro not used because : 0 nodes ! */
#define cnm_grp_0_6_secondary_IntsSum_SUB_TREE_MAC(bit, _treeId,  _tileId, _mgIndex)                                \
        cnm_grp_0_6_IntsSum_____SUB_TREE_MAC(bit, _treeId, _tileId, 0, _mgIndex )

/* hold 12 'native' sons */
#define cnm_grp_0_7_primary_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)                 \
        cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, 12, _mgIndex,_tile1),                     \
        FALCON_MG_INTERRUPTS_SECONDARY_TILE_MAC(29/*bit*/, _tile1)

/* hold 3 'native' sons */
/* support for MG4 (that connects to MG5,6,7) */
#define cnm_grp_0_7_secondary_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                       \
        cnm_grp_0_7_IntsSum_____SUB_TREE_MAC(bit, _treeId, _tileId, 3, _mgIndex )

#define cnm_grp_1_0_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* FuncUnitsIntsSum Interrupt Cause */                                                               \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                   \
        FUNC_UNITS_CAUSE_MASK(_treeId),                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_PDS0_E,                                                     \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_TAI0_E,                                                     \
        FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 31, NULL, NULL},                                           \
        /* 8 elements */                                                                                 \
        pds_units(_tileId),                                                                              \
        /* 8 elements */                                                                                 \
        qfc_units(_tileId),                                                                              \
        /* 8 elements */                                                                                 \
        sdq_units(_tileId),                                                                              \
        /* 4 elements */                                                                                 \
        txq_tai_slave_units(_tileId),                                                                    \
        eagle_side_d2d_cp_NODE_MAC(29, _tileId),                                                         \
        dfx_server_sum_SUB_TREE_MAC(30, _tileId),                                                        \
        tai_SUB_TREE_MAC(31, _tileId, 0)

#define cnm_grp_1_1_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, _tile1)                       \
    /* DataPathIntsSum Interrupt Cause */                                                               \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                  \
        DATA_PATH_CAUSE_MASK(_treeId),                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_FALCON_TILE_##_tile1##_MG1_PDS0_E,                                                     \
        PRV_CPSS_FALCON_TILE_##_tile1##_MG1_TAI0_E,                                                     \
        FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 31, NULL, NULL},                                          \
        /* 8 elements */                                                                                \
        pds_units(_tile1),                                                                              \
        /* 8 elements */                                                                                \
        qfc_units(_tile1),                                                                              \
        /* 8 elements */                                                                                \
        sdq_units(_tile1),                                                                              \
        /* 4 elements */                                                                                \
        txq_tai_slave_units(_tile1),                                                                    \
        eagle_side_d2d_cp_NODE_MAC(29, _tile1),                                                         \
        dfx_server_sum_SUB_TREE_MAC(30, _tile1),                                                        \
        tai_SUB_TREE_MAC(31, _tile1, 1)

#define cnm_grp_1_2_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* Ports Interrupt Cause */                                                                          \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                   \
        PORTS_CAUSE_MASK(_treeId),                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PORTS_TILE0_D2D_0_INT_SUM,                                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_PORTS_TILE0_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_1_MACRO_CP_X2_CLK_1_INT_SUM, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 8, NULL, NULL},                                                  \
        d2d_mac_tx_units(_tileId)

#define cnm_grp_1_2_dual_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex, tile1)                    \
    /* Ports Interrupt Cause */                                                                          \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                   \
        PORTS_CAUSE_MASK(_treeId),                                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_PORTS_TILE0_D2D_0_INT_SUM,                                      \
        PRV_CPSS_FALCON_TILE_##_tileId##_PORTS_TILE0_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_1_MACRO_CP_X2_CLK_1_INT_SUM, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                                  \
        d2d_mac_tx_units_dual(_tileId, tile1)

#define cnm_grp_1_3_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* DFXIntsSum Interrupt Cause */                                                                     \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,               \
        DFX_CAUSE_MASK(_treeId),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_CP_0_TOP_DFX_CLIENT_EAGLE_EGF_MACRO_CP_CLK_0_INT_SUM_E,         \
        PRV_CPSS_FALCON_TILE_##_tileId##_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_1_MACRO_CP_X2_CLK_0_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_grp_1_4_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* DFX1IntsSum Interrupt Cause */                                                                    \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,               \
        DFX1_CAUSE_MASK(_treeId),                                                                        \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_CP_0_TOP_DFX_CLIENT_EAGLE_EGF_MACRO_CP_CLK_0_INT_SUM_E,         \
        PRV_CPSS_FALCON_TILE_##_tileId##_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_1_MACRO_CP_X2_CLK_0_INT_SUM_E,\
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_grp_1_5_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* FuncUnits1IntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,               \
        FUNC1_UNITS_CAUSE_MASK(_treeId),                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_FUNC_UNITS1_DMA_TOP_RXM0_DFX_CLIENT_EAGLE_RX_MACRO_PB_CLK_INT_SUM_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_FUNC_UNITS1_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_0_MACRO_CP_X2_CLK_0_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_grp_1_6_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex)                                \
    /* Ports1IntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,               \
        PORTS1_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_FUNC_UNITS1_DMA_TOP_RXM0_DFX_CLIENT_EAGLE_RX_MACRO_PB_CLK_INT_SUM_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_FUNC_UNITS1_CENTRAL_TOP_DFX_CLIENT_EAGLE_UBM4_0_MACRO_CP_X2_CLK_0_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_grp_1_7_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId , _mgIndex)                               \
    /* Ports2IntsSum Interrupt Cause */                                                                  \
    {bit, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                   \
        PORTS2_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_TILE0_PORT2_PB_TOP_EAGLE_PB_CENTER_MACRO_DFX_CLIENT_EAGLE_PB_CENTER_MACRO_PB_CLK_INT_SUM_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG1_TILE1_PORT2_CENTRAL_TOP_DFX_CLIENT_EAGLE_PSI_MACRO_PB_CLK_1_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

/* MG4 'start of tree' */
#define  FALCON_MG_INTERRUPTS_SECONDARY_TILE_MAC(bitIndexInCaller, _tile1)          \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 0, _tile1/*tileId*/, 4, 0 /*_mgIndex*/), \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, 0, _tile1, 0 /*_mgIndex*/),                 \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, 0, _tile1, 0 /*_mgIndex*/),                   \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, 0, _tile1, 0 /*_mgIndex*/),                   \
        /* Ports 2 Interrupt Summary */                                             \
        cnm_grp_0_7_secondary_IntsSum_SUB_TREE_MAC(17, 0, _tile1, 0 /*_mgIndex*/)


#define cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _mgIndex , _tile1)                       \
    /* Ports1IntSum - First Cider releases has bugs. CNC bits are 22..29  */                             \
    cnm_grp_0_6_IntsSum_single_tile_SUB_TREE_MAC(bit, _treeId, _tileId, 8, _mgIndex),                    \
        cnc_SUB_TREE_MAC(28, _tile1, 0/*Pipe*/, 0/*Instance*/),                                          \
        cnc_SUB_TREE_MAC(29, _tile1, 0/*Pipe*/, 1/*Instance*/),                                          \
        cnc_SUB_TREE_MAC(30, _tile1, 1/*Pipe*/, 0/*Instance*/),                                          \
        cnc_SUB_TREE_MAC(31, _tile1, 1/*Pipe*/, 1/*Instance*/)

/*   cnm_grp_0_6 : without support for 'tile issues' */
#define cnm_grp_0_6_IntsSum_____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex)                    \
    /* Ports1IntSum */                                                                                   \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        PORTS1_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_6_GPIO_7_0_INT_SUM_E,                  \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_6_PIN_MCU_WDT1_INT_SUM_E,              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, _nodes, NULL, NULL}

/*  cnm_grp_0_6_IntsSum_SUB_TREE_MAC with support for 'single tile' */
#define cnm_grp_0_6_IntsSum_single_tile_SUB_TREE_MAC(bit, _treeId, _tileId, _nodes , _mgIndex)                     \
    cnm_grp_0_6_IntsSum_____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex) ,                                \
        cnc_SUB_TREE_MAC(24, _tileId, 0/*Pipe*/, 0/*Instance*/),                                         \
        cnc_SUB_TREE_MAC(25, _tileId, 0/*Pipe*/, 1/*Instance*/),                                         \
        cnc_SUB_TREE_MAC(26, _tileId, 1/*Pipe*/, 0/*Instance*/),                                         \
        cnc_SUB_TREE_MAC(27, _tileId, 1/*Pipe*/, 1/*Instance*/)

/* hold 11 'native' sons */
#define cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex , _tile1)               \
    cnm_grp_0_7_IntsSum_dual_tile_SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex, _tile1),         \
        ermrk_SUB_TREE_MAC(23, _tile1/*tileId*/, 0/*pipe*/),                                             \
        l2i_SUB_TREE_MAC(  24, _tile1/*tileId*/, 0/*pipe*/),                                             \
        ermrk_SUB_TREE_MAC(25, _tile1/*tileId*/, 1/*pipe*/),                                             \
        l2i_SUB_TREE_MAC(  26, _tile1/*tileId*/, 1/*pipe*/)

/* hold 3 'native' sons */
/*   cnm_grp_0_7 : without support for 'tile issues' */
#define cnm_grp_0_7_IntsSum_____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex)                    \
    /* Ports2ntSum */                                                                                    \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        PORTS2_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_MG_CLUSTER0_MG1_INT_OUT0_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_OTHER_MG_CLUSTER_MG0_INT_OUT2_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, _nodes, NULL, NULL},                                              \
        /*MG1*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG1_int_sum + ((_tileId & 1) * 9), 0/*tree*/, _tileId/*tileId*/,1/*mgUnit*/),          \
        /*MG2*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG2_int_sum + ((_tileId & 1) * 9), 0/*tree*/, _tileId/*tileId*/,2/*mgUnit*/),          \
        /*MG3*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG3_int_sum + ((_tileId & 1) * 9), 0/*tree*/, _tileId/*tileId*/,3/*mgUnit*/)

/* hold 3 'native' sons */
/*   cnm_grp_0_7 single tile: without support for 'tile issues' */
#define cnm_grp_0_7_IntsSum_single____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex)              \
    /* Ports2ntSum */                                                                                    \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        PORTS2_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_MG_CLUSTER0_MG1_INT_OUT0_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_OTHER_MG_CLUSTER_MG0_INT_OUT2_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, _nodes, NULL, NULL},                                              \
        /*MG1*/                                                                                          \
        FALCON_MG_INTERRUPTS_SINGLE_TILE_MG_1_MAC(falcon_MG1_int_sum, 0/*tree*/, _tileId/*tileId*/,1/*mgUnit*/),     \
        /*MG2*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG2_int_sum, 0/*tree*/, _tileId/*tileId*/,2/*mgUnit*/),          \
        /*MG3*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG3_int_sum, 0/*tree*/, _tileId/*tileId*/,3/*mgUnit*/)

/* hold 3 'native' sons */
/*   cnm_grp_0_7 dual tiles: without support for 'tile issues' */
#define cnm_grp_0_7_IntsSum_dual____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex, _tile1)        \
    /* Ports2ntSum */                                                                                    \
    {bit, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E, _tileId, _mgIndex), NULL,                    \
        PORTS2_CAUSE_MASK(_treeId),                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_MG_CLUSTER0_MG1_INT_OUT0_E,    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CNM_GRP_0_7_PORT2_OTHER_MG_CLUSTER_MG0_INT_OUT2_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, _nodes, NULL, NULL},                                              \
        /*MG1*/                                                                                          \
        FALCON_MG_INTERRUPTS_DUAL_TILE_MG_1_MAC(falcon_MG1_int_sum, 0/*tree*/, _tileId/*tileId*/,1/*mgUnit*/, _tile1),\
        /*MG2*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG2_int_sum, 0/*tree*/, _tileId/*tileId*/,2/*mgUnit*/),           \
        /*MG3*/                                                                                          \
        FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(falcon_MG3_int_sum, 0/*tree*/, _tileId/*tileId*/,3/*mgUnit*/)

/* hold 7 'native' sons  */
/*  cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC with support for 'single tile' */
#define cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex)            \
    cnm_grp_0_7_IntsSum_single____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex) ,                \
        ermrk_SUB_TREE_MAC(19, _tileId/*tileId*/, 0/*pipe*/),                                            \
        l2i_SUB_TREE_MAC(  20, _tileId/*tileId*/, 0/*pipe*/),                                            \
        ermrk_SUB_TREE_MAC(21, _tileId/*tileId*/, 1/*pipe*/), /* unlike the excel file */                \
        l2i_SUB_TREE_MAC(  22, _tileId/*tileId*/, 1/*pipe*/)

#define cnm_grp_0_7_IntsSum_dual_tile_SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex, _tile1)      \
    cnm_grp_0_7_IntsSum_dual____SUB_TREE_MAC(bit, _treeId, _tileId, _nodes, _mgIndex, _tile1),           \
        ermrk_SUB_TREE_MAC(19, _tileId/*tileId*/, 0/*pipe*/),                                            \
        l2i_SUB_TREE_MAC(  20, _tileId/*tileId*/, 0/*pipe*/),                                            \
        ermrk_SUB_TREE_MAC(21, _tileId/*tileId*/, 1/*pipe*/), /* unlike the excel file */                \
        l2i_SUB_TREE_MAC(  22, _tileId/*tileId*/, 1/*pipe*/)

#define FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, _treeId, _tileId, numChild, _mgIndex)  \
    /* Eagle Global Interrupt CaRY_use */                                            \
    {bitIndexInCaller, GT_FALSE, MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E,_tileId,_mgIndex), NULL,     \
        GLOBAL_CAUSE_MASK(_treeId),                                             \
        prvCpssDrvHwPpPortGroupIsrRead,                                     \
        prvCpssDrvHwPpPortGroupIsrWrite,                                    \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_GLOBAL_SUMMARY_FUNC_UNITS_SUM_E,   \
        PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_GLOBAL_SUMMARY_MG1_INTERNAL_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define FALCON_MG_Tx_SDMA_SUMMARY_MAC(bitIndexInCaller, _treeId, _tileId, _mgIndex)          \
        /* Tx SDMA  */                                                    \
        {bitIndexInCaller, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E,_tileId,_mgIndex), NULL, \
            TX_DMA_CAUSE_MASK(_treeId),                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                              \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_TX_SDMA_TX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_TX_SDMA_TX_REJECT_0_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define FALCON_MG_Rx_SDMA_SUMMARY_MAC(bitIndexInCaller, _treeId, _tileId, _mgIndex) \
        /* Rx SDMA  */                                                    \
        {bitIndexInCaller, GT_FALSE,MG_UNIT_IN_TILE(PRV_CPSS_DXCH_UNIT_MG_E,_tileId,_mgIndex), NULL, \
            RX_DMA_CAUSE_MASK(_treeId),                                         \
            prvCpssDrvHwPpPortGroupIsrRead,                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                              \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_RX_SDMA_RX_BUFFER_QUEUE_0_E, \
            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_RX_SDMA_PACKET_CNT_OF_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define  FALCON_MG_INTERRUPTS_SINGLE_TILE_MG_1_MAC(bitIndexInCaller, tree, tile, _mgId)       \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, 5 /*numChild*/,_mgId/*mgId*/), \
        /* Functional Units Interrupt Summary */                                    \
        cnm_grp_1_0_IntsSum_SUB_TREE_MAC(3, tree, tile, _mgId /*_mgIndex*/),               \
        /* Ports Interrupt Summary */                                               \
        cnm_grp_1_2_IntsSum_SUB_TREE_MAC(5, tree, tile, _mgId /*_mgIndex*/),               \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, _mgId /*_mgIndex*/),                \
        /* Tx SDMA  */                                                               \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, _mgId /*_mgIndex*/),                  \
        /* Rx SDMA  */                                                               \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, _mgId /*_mgIndex*/)


#define  FALCON_MG_INTERRUPTS_DUAL_TILE_MG_1_MAC(bitIndexInCaller, tree, tile, _mgId, _tile1)       \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, 6/*numChild*/,_mgId/*mgId*/), \
        /* Functional Units Interrupt Summary */                                    \
        cnm_grp_1_0_IntsSum_SUB_TREE_MAC(3, tree, tile, _mgId /*_mgIndex*/),               \
        /* DataPathIntsSum Interrupt Cause */                                       \
        cnm_grp_1_1_IntsSum_SUB_TREE_MAC(4, tree, tile, _mgId /*_mgIndex*/, _tile1),        \
        /* Ports Interrupt Summary */                                               \
        cnm_grp_1_2_dual_IntsSum_SUB_TREE_MAC(5, tree, tile, _mgId /*_mgIndex*/, _tile1),  \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, _mgId /*_mgIndex*/),               \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, _mgId /*_mgIndex*/),                 \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, _mgId /*_mgIndex*/)

#define  FALCON_MG_INTERRUPTS_TILE_x_MG_y_MAC(bitIndexInCaller, tree, tile, _mgId)       \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, 3 /*numChild*/,_mgId/*mgId*/), \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, _mgId /*_mgIndex*/),               \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, _mgId /*_mgIndex*/),                 \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, _mgId /*_mgIndex*/)

#define  FALCON_MG_INTERRUPTS_SINGLE_TILE_MAC(bitIndexInCaller, tree, tile, globalNumChild)   \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, globalNumChild/*numChild*/, 0 /*_mgIndex*/), \
        /* FuncUnitsIntsSum Interrupt Cause */                                      \
        cnm_grp_0_0_single_tile_IntsSum_SUB_TREE_MAC(3, tree, tile, 0 /*_mgIndex*/),      \
        /* Data Path Interrupt Summay */                                            \
        cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, tree, tile, 0 /*_mgIndex*/),                  \
        /* ports0SumIntSum */                                                       \
        cnm_grp_0_2_IntsSum_SUB_TREE_MAC(5, tree, tile, 0 /*_mgIndex*/),                  \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, 0 /*_mgIndex*/),                   \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, 0 /*_mgIndex*/),                     \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, 0 /*_mgIndex*/),                     \
        /* FuncUnitsIntsSum */                                                      \
        cnm_grp_0_5_IntsSum_single_tile_SUB_TREE_MAC(11, tree, tile, 0 /*_mgIndex*/),     \
        /* Ports 1 Interrupt Summary */                                             \
        cnm_grp_0_6_IntsSum_single_tile_SUB_TREE_MAC(16, tree, tile, 4/*nodes*/, 0 /*_mgIndex*/),     \
        /* Ports 2 Interrupt Summary */                                             \
        cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC(17, tree, tile, 7/*nodes*/, 0 /*_mgIndex*/)


#define FALCON_MG_INTERRUPTS_DUAL_TILE_TREE0_MAC(bitIndexInCaller, tree, tile, globalNumChild, tile1)   \
        FALCON_MG_INTERRUPTS_DUAL_TILE_MAC(bitIndexInCaller, tree, tile, globalNumChild,tile1)

#define FALCON_MG_INTERRUPTS_DUAL_TILE_TREE1_MAC(bitIndexInCaller, tree, tile, globalNumChild, tile1)   \
        FALCON_MG_INTERRUPTS_DUAL_TILE_MAC(bitIndexInCaller, tree, tile, globalNumChild,tile1)

#define FALCON_MG_INTERRUPTS_DUAL_TILE_MAC(bitIndexInCaller, tree, tile, globalNumChild, tile1)   \
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, globalNumChild/*numChild*/, 0 /*_mgIndex*/), \
        /* FuncUnitsIntsSum Interrupt Cause */                                      \
        cnm_grp_0_0_primary_IntsSum_SUB_TREE_MAC(3, tree, tile, 0 /*_mgIndex*/, tile1),   \
        /* Data Path Interrupt Summay */                                            \
        cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, tree, tile, 0 /*_mgIndex*/),            \
        /* ports0SumIntSum */                                                       \
        cnm_grp_0_2_IntsSum_SUB_TREE_MAC(5, tree, tile, 0 /*_mgIndex*/),            \
        /* DFX Interrupt Summay */                                                  \
        cnm_grp_0_3_IntsSum_SUB_TREE_MAC(6, tree, tile, 0 /*_mgIndex*/,tile1),      \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, 0 /*_mgIndex*/),             \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, 0 /*_mgIndex*/),                     \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, 0 /*_mgIndex*/),                     \
        /* DFX1 Interrupt Summay */                                                 \
        cnm_grp_0_4_IntsSum_SUB_TREE_MAC(10, tree, tile, 0 /*_mgIndex*/,tile1),     \
        /* FuncUnitsIntsSum */                                                      \
        cnm_grp_0_5_primary_IntsSum_SUB_TREE_MAC(11, tree, tile, 0 /*_mgIndex*/, tile1),  \
        /* Ports 1 Interrupt Summary */                                             \
        cnm_grp_0_6_primary_IntsSum_SUB_TREE_MAC(16, tree, tile, 0 /*_mgIndex*/, tile1),  \
        /* Ports 2 Interrupt Summary */                                             \
        cnm_grp_0_7_primary_IntsSum_SUB_TREE_MAC(17, tree, tile, 0 /*_mgIndex*/, tile1)

#define FALCON_MG_INTERRUPTS_4_TILES_MAC(bitIndexInCaller, tree, tile, globalNumChild, tile1)\
    /* Global Interrupt Cause */                                                    \
    FALCON_MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, tree, tile/*tileId*/, globalNumChild/*numChild*/, 0 /*_mgIndex*/), \
        /* FuncUnitsIntsSum Interrupt Cause (that supports tiles 2,3) */                        \
        cnm_grp_0_0_primary_for_4_tiles_IntsSum_SUB_TREE_MAC(3, tree, tile, 0 /*_mgIndex*/, tile1),   \
        /* Data Path Interrupt Summay */                                            \
        cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, tree, tile, 0 /*_mgIndex*/),            \
        /* ports0SumIntSum */                                                       \
        cnm_grp_0_2_IntsSum_SUB_TREE_MAC(5, tree, tile, 0 /*_mgIndex*/),            \
        /* DFX Interrupt Summay */                                                  \
        cnm_grp_0_3_IntsSum_SUB_TREE_MAC(6, tree, tile, 0 /*_mgIndex*/,tile1),      \
        /* MG internal Interrupt Cause */                                           \
        mg0InternalIntsSum_SUB_TREE_MAC(7, tree, tile, 0 /*_mgIndex*/),             \
        /* Tx SDMA  */                                                              \
        FALCON_MG_Tx_SDMA_SUMMARY_MAC(8, tree, tile, 0 /*_mgIndex*/),                     \
        /* Rx SDMA  */                                                              \
        FALCON_MG_Rx_SDMA_SUMMARY_MAC(9, tree, tile, 0 /*_mgIndex*/),                     \
        /* DFX1 Interrupt Summay */                                                 \
        cnm_grp_0_4_IntsSum_SUB_TREE_MAC(10, tree, tile, 0 /*_mgIndex*/, tile1),    \
        /* FuncUnitsIntsSum */                                                      \
        cnm_grp_0_5_primary_IntsSum_SUB_TREE_MAC(11, tree, tile, 0 /*_mgIndex*/, tile1),  \
        /* Ports 1 Interrupt Summary */                                             \
        cnm_grp_0_6_primary_IntsSum_SUB_TREE_MAC(16, tree, tile, 0 /*_mgIndex*/, tile1),  \
        /* Ports 2 Interrupt Summary */                                             \
        cnm_grp_0_7_primary_IntsSum_SUB_TREE_MAC(17, tree, tile, 0 /*_mgIndex*/, tile1)


#define falcon_MG1_int_sum 1
#define falcon_MG2_int_sum 4
#define falcon_MG3_int_sum 7
#define falcon_other_int_sum 29
#define falcon_other_dual_int_sum 17


static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC falconIntrScanArr_1_tile[] =
{
    FALCON_MG_INTERRUPTS_SINGLE_TILE_MAC(0/*bitIndexInCaller*/, 0/*tree*/, 0/*tile*/, 9/*numChild*/)
};
static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC falconIntrScanArr_2_tiles[] =
{
    FALCON_MG_INTERRUPTS_DUAL_TILE_TREE0_MAC(0/*bitIndexInCaller*/, 0/*tree*/, 0/*primary tile*/, 11/*numChild*/, 1/*secondary tile*/)
};
static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC falconIntrScanArr_4_tiles[] =
{
    /* NOTE: the knowledge about tiles : 2,3 comes from
        cnm_grp_0_0_primary_for_4_tiles_IntsSum_SUB_TREE_MAC !!! */
    FALCON_MG_INTERRUPTS_4_TILES_MAC(0/*bitIndexInCaller*/, 0/*tree*/, 0/*primary tile*/, 11/*numChild*/, 1/*secondary tile*/)
};

/* number of elements in the array of falconIntrScanArr[] */
#define FALCON_1_TILE_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(falconIntrScanArr_1_tile)

#define FALCON_2_TILES_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(falconIntrScanArr_2_tiles)

#define FALCON_4_TILES_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(falconIntrScanArr_4_tiles)


#define FALCON_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_FALCON_LAST_INT_E / 32)

/* mark extData as per pipe per tile port interrupt */
/* _tile    --> the tileId 0, 2  */
/* _raven   --> local raven 0..3 */
/* _port    --> local port in the pipe 0..15 */
#define TILE_0_2___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile, _raven, _port, index) \
    ((((_tile) * 64) + (16 * (_raven)) + (_port + (index * 8))) | MARK_PER_PORT_INT_CNS)

/* _tile    --> the tileId 1, 3  */
/* _raven   --> local raven 0..3 */
/* _port    --> local port in the pipe 0..15 */
#define TILE_1_3___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile, _raven, _port, index) \
    ((((_tile) * 64) + (16 * (3-(_raven))) + (_port + (index * 8))) | MARK_PER_PORT_INT_CNS)

/*  */
#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##PORT##_portNum##_##_postFix,       TILE_0_2___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, index)

#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##SEG_PORT##_portNum##_##_postFix,       TILE_0_2___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, index)

#define TILE_SET_EVENT_PER_RAVEN_PER_TILE_CPU_PORT_MAC(index,_postFix, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTIIP_CPU_##_postFix,                             index

/* _postFix - must include the "_E" */
#define SET_EVENT_WITH_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, _ravenId, _tileId, _index)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix + _index,  _index


/* _tile    --> the tileId 0..3  */
/* _raven   --> local raven 0..3 */
/* _lmu     --> the LMU unit in raven 0..1 */
/* index    --> the latency profile 0..511 */
#define TILE___MARK_PER_RAVEN_PER_TILE_0_2_LMU_INT_MAC(_tile, _raven, _lmu, index) \
    ((((_lmu) | ((_raven) << 1) | ((_tile) << 3)) << 16) + index)

#define TILE___MARK_PER_RAVEN_PER_TILE_1_3_LMU_INT_MAC(_tile, _raven, _lmu, index) \
    ((((_lmu) | ((3-(_raven)) << 1) | ((_tile) << 3)) << 16) + index)

/* _postFix - must include the "_E" */
#define SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_0_2_MAC(_postFix, _lmu, _ravenId, _tileId, _reg, _index)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix + _reg,  TILE___MARK_PER_RAVEN_PER_TILE_0_2_LMU_INT_MAC(_tileId, _ravenId, _lmu, _index)

/* _postFix - must include the "_E" */
#define SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_1_3_MAC(_postFix, _lmu, _ravenId, _tileId, _reg, _index)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix + _reg,  TILE___MARK_PER_RAVEN_PER_TILE_1_3_LMU_INT_MAC(_tileId, _ravenId, _lmu, _index)

#define SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_0_2_MAC(_postFix, _lmu, _tile, _reg, _index)                     \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_0_2_MAC(_postFix, _lmu, 0, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_0_2_MAC(_postFix, _lmu, 1, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_0_2_MAC(_postFix, _lmu, 2, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_0_2_MAC(_postFix, _lmu, 3, _tile, _reg, _index)

#define SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_1_3_MAC(_postFix, _lmu, _tile, _reg, _index)                     \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_1_3_MAC(_postFix, _lmu, 0, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_1_3_MAC(_postFix, _lmu, 1, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_1_3_MAC(_postFix, _lmu, 2, _tile, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_RAVEN_PER_TILE_1_3_MAC(_postFix, _lmu, 3, _tile, _reg, _index)

#define SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(_postFix, _lmu, _reg, _index)             \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_0_2_MAC(_postFix, _lmu, 0, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_1_3_MAC(_postFix, _lmu, 1, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_0_2_MAC(_postFix, _lmu, 2, _reg, _index),           \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_IN_TILE_1_3_MAC(_postFix, _lmu, 3, _reg, _index)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_RAVEN_PER_TILE_MAC(_postFix, _ravenId, _tileId, _index)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix,       _index

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_ALL_RAVEN_IN_TILE_MAC(_postFix, _tile, _index)                     \
    SET_EVENT_PER_RAVEN_PER_TILE_MAC(_postFix, 0, _tile, _index),           \
    SET_EVENT_PER_RAVEN_PER_TILE_MAC(_postFix, 1, _tile, _index),           \
    SET_EVENT_PER_RAVEN_PER_TILE_MAC(_postFix, 2, _tile, _index),           \
    SET_EVENT_PER_RAVEN_PER_TILE_MAC(_postFix, 3, _tile, _index)

#define SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(_postFix, _index)             \
    SET_EVENT_PER_ALL_RAVEN_IN_TILE_MAC(_postFix, 0, _index),               \
    SET_EVENT_PER_ALL_RAVEN_IN_TILE_MAC(_postFix, 1, _index),               \
    SET_EVENT_PER_ALL_RAVEN_IN_TILE_MAC(_postFix, 2, _index),               \
    SET_EVENT_PER_ALL_RAVEN_IN_TILE_MAC(_postFix, 3, _index)

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, _ravenId, _tileId)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix,                     \
    PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_##_postFix

/* _postFix - must include the "_E" */
#define SET_EVENT_ALL_RAVEN_IN_TILE_MAC(_postFix, _tile)                  \
    SET_EVENT_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, 0, _tile),           \
    SET_EVENT_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, 1, _tile),           \
    SET_EVENT_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, 2, _tile),           \
    SET_EVENT_INDEX_PER_RAVEN_PER_TILE_MAC(_postFix, 3, _tile)

#define SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(_postFix)             \
    SET_EVENT_ALL_RAVEN_IN_TILE_MAC(_postFix, 0),               \
    SET_EVENT_ALL_RAVEN_IN_TILE_MAC(_postFix, 1),               \
    SET_EVENT_ALL_RAVEN_IN_TILE_MAC(_postFix, 2),               \
    SET_EVENT_ALL_RAVEN_IN_TILE_MAC(_postFix, 3)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, _pipeId, _tileId, _index)             \
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_##_postFix,       _index
/* _postFix - must include the "_E" */
#define SET_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, _tile, _index)        \
    SET_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, 0, _tile, _index),            \
    SET_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, 1, _tile, _index)
/* _postFix - must include the "_E" */
#define SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(_postFix, _index)             \
    SET_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, 0, _index),               \
    SET_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, 1, _index),               \
    SET_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, 2, _index),               \
    SET_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, 3, _index)

/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, _pipeId, _tileId)             \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_##_postFix)
/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix, _tile)                     \
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, 0, _tile),                         \
    SET_INDEX_EQUAL_EVENT_PER_PIPE_PER_TILE_MAC(_postFix, 1, _tile)
/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(_postFix)                         \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix,  0),                          \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix,  1),                          \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix,  2),                          \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_IN_TILE_MAC(_postFix,  3)

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_TILE_MAC(_postFix, _tileId)             \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_##_postFix)

#define SET_EVENT_ALL_TILES_MAC(_postFix)                    \
    SET_EVENT_INDEX_PER_TILE_MAC(_postFix, 0),               \
    SET_EVENT_INDEX_PER_TILE_MAC(_postFix, 1),               \
    SET_EVENT_INDEX_PER_TILE_MAC(_postFix, 2),               \
    SET_EVENT_INDEX_PER_TILE_MAC(_postFix, 3)

/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_D2D_PER_PIPE_PER_TILE_MAC(_postFix, _d2dId,_pipeId, _tileId)             \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_D2D##_d2dId##_##_postFix)

#define SET_INDEX_EQUAL_EVENT_ALL_D2D_PER_PIPE_PER_TILE_MAC(_postFix, _pipeId, _tileId)        \
    SET_INDEX_EQUAL_EVENT_PER_D2D_PER_PIPE_PER_TILE_MAC(_postFix,0/*d2d0*/, _pipeId, _tileId) ,\
    SET_INDEX_EQUAL_EVENT_PER_D2D_PER_PIPE_PER_TILE_MAC(_postFix,1/*d2d1*/, _pipeId, _tileId) ,\
    SET_INDEX_EQUAL_EVENT_PER_D2D_PER_PIPE_PER_TILE_MAC(_postFix,2/*d2d2*/, _pipeId, _tileId) ,\
    SET_INDEX_EQUAL_EVENT_PER_D2D_PER_PIPE_PER_TILE_MAC(_postFix,3/*d2d3*/, _pipeId, _tileId)

/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_IN_TILE_MAC(_postFix, _tile)                     \
    SET_INDEX_EQUAL_EVENT_ALL_D2D_PER_PIPE_PER_TILE_MAC(_postFix, 0, _tile),                     \
    SET_INDEX_EQUAL_EVENT_ALL_D2D_PER_PIPE_PER_TILE_MAC(_postFix, 1, _tile)

#define SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(_postFix) \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_IN_TILE_MAC(_postFix,  0),                          \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_IN_TILE_MAC(_postFix,  1),                          \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_IN_TILE_MAC(_postFix,  2),                          \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_IN_TILE_MAC(_postFix,  3)

/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_IN_TILE_MAC(_postFix, _tileId)                                \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_##_tileId##_##_postFix)

/* _postFix - must include the "_E" */
#define SET_INDEX_EQUAL_EVENT_PER_TILE_MAC(_postFix)                                        \
    SET_INDEX_EQUAL_EVENT_IN_TILE_MAC(_postFix,  0),                                        \
    SET_INDEX_EQUAL_EVENT_IN_TILE_MAC(_postFix,  1),                                        \
    SET_INDEX_EQUAL_EVENT_IN_TILE_MAC(_postFix,  2),                                        \
    SET_INDEX_EQUAL_EVENT_IN_TILE_MAC(_postFix,  3)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_INDEX_PER_PIPE_PER_TILE_MAC(_preFix, _postFix, _pipeId, _tileId, _index)     \
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipeId##_##_preFix##_##_index##_##_postFix,    _index

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, _index, _postFix, _tile)                \
    SET_EVENT_PER_INDEX_PER_PIPE_PER_TILE_MAC(_preFix, _postFix, 0, _tile, _index),           \
    SET_EVENT_PER_INDEX_PER_PIPE_PER_TILE_MAC(_preFix, _postFix, 1, _tile, _index)


#define SET_EVENT_PER_256_INDEX_IN_PIPE_MAC(_preFix, _postFix, _tile)           \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   0,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   1,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   2,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   3,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   4,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   5,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   6,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   7,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   8,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,   9,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  10,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  11,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  12,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  13,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  14,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  15,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  16,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  17,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  18,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  19,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  20,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  21,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  22,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  23,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  24,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  25,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  26,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  27,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  28,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  29,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  30,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  31,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  32,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  33,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  34,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  35,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  36,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  37,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  38,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  39,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  40,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  41,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  42,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  43,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  44,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  45,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  46,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  47,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  48,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  49,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  50,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  51,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  52,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  53,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  54,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  55,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  56,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  57,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  58,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  59,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  60,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  61,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  62,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  63,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  64,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  65,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  66,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  67,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  68,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  69,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  70,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  71,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  72,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  73,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  74,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  75,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  76,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  77,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  78,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  79,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  80,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  81,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  82,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  83,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  84,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  85,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  86,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  87,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  88,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  89,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  90,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  91,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  92,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  93,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  94,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  95,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  96,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  97,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  98,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix,  99,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 100,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 101,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 102,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 103,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 104,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 105,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 106,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 107,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 108,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 109,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 110,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 111,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 112,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 113,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 114,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 115,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 116,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 117,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 118,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 119,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 120,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 121,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 122,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 123,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 124,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 125,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 126,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 127,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 128,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 129,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 130,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 131,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 132,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 133,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 134,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 135,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 136,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 137,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 138,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 139,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 140,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 141,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 142,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 143,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 144,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 145,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 146,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 147,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 148,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 149,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 150,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 151,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 152,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 153,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 154,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 155,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 156,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 157,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 158,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 159,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 160,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 161,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 162,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 163,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 164,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 165,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 166,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 167,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 168,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 169,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 170,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 171,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 172,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 173,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 174,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 175,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 176,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 177,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 178,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 179,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 180,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 181,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 182,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 183,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 184,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 185,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 186,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 187,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 188,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 189,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 190,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 191,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 192,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 193,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 194,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 195,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 196,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 197,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 198,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 199,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 200,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 201,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 202,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 203,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 204,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 205,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 206,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 207,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 208,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 209,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 210,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 211,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 212,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 213,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 214,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 215,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 216,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 217,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 218,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 219,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 220,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 221,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 222,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 223,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 224,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 225,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 226,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 227,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 228,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 229,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 230,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 231,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 232,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 233,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 234,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 235,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 236,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 237,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 238,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 239,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 240,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 241,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 242,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 243,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 244,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 245,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 246,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 247,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 248,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 249,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 250,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 251,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 252,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 253,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 254,  _postFix, _tile),        \
    SET_EVENT_PER_ALL_INDEX_IN_PIPE_MAC(_preFix, 255,  _postFix, _tile)

/* state for a port that event hold extra raven and port number */
/* _prefix - is between 'device' and "_PORT" */
/* _postFix - must include the "_E" */
#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##PORT##_portNum##_##_postFix,       TILE_1_3___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, index)

#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##SEG_PORT##_portNum##_##_postFix,       TILE_1_3___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, index)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      7,      _raven, _tile)

#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_PORT_MAC(index,_postFix,      7,      _raven, _tile)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,  0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,  4,      _raven, _tile)


#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,  0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SEG_PORT_MAC(index,_postFix,  4,      _raven, _tile)

#define TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_SET_EVENT_PER_RAVEN_PER_TILE_CPU_PORT_MAC(index,_postFix,  _raven, _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(index,_postFix)                \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_TILE_MAC(index,_postFix, 3)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(index,_postFix)            \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_TILE_MAC(index,_postFix, 3)

/* state for CPU port that event holds extra index equal to event */
#define TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORTS___ON_ALL_TILES_MAC(index,_postFix)               \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_TILE_MAC(index,_postFix, 0),          \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_TILE_MAC(index,_postFix, 1),          \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_TILE_MAC(index,_postFix, 2),          \
    TILE_FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORT___ON_TILE_MAC(index,_postFix, 3)

/*  */
#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_SD1_INT0_PORT##_portNum##_##_postFix,       TILE_0_2___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, 0)

/* state for a port that event hold extra raven and port number */
/* _prefix - is between 'device' and "_PORT" */
/* _postFix - must include the "_E" */
#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_SD1_INT0_PORT##_portNum##_##_postFix,       TILE_1_3___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, 0)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      1,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      2,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      3,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      4,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      5,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      6,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      7,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      8,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      9,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      10,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      11,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      12,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      13,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      14,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      15,     _raven, _tile)

#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      1,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      2,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      3,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      4,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      5,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      6,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      7,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      8,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      9,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      10,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      11,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      12,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      13,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      14,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_SERDES_MAC(_postFix,      15,     _raven, _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   3,   _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_RAVEN_MAC(_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_ALL_TILES_MAC(_postFix)                \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_SERDES___ON_TILE_MAC(_postFix, 3)


/*  */
#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_AP_DOORBELL_PORT_##_portNum##_##_postFix,       TILE_0_2___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, 0)

/* state for a port that event hold extra raven and port number */
/* _prefix - is between 'device' and "_PORT" */
/* _postFix - must include the "_E" */
#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,_portNum, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_AP_DOORBELL_PORT_##_portNum##_##_postFix,       TILE_1_3___MARK_PER_RAVEN_PER_TILE_PORT_INT_MAC(_tile , _raven, _portNum, 0)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      1,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      2,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      3,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      4,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      5,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      6,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      7,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      8,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      9,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      10,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      11,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      12,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      13,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      14,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      15,     _raven, _tile)

#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      1,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      2,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      3,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      4,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      5,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      6,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      7,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      8,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      9,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      10,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      11,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      12,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      13,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      14,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_AP_DOORBELL_MAC(_postFix,      15,     _raven, _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   3,   _tile)

/* state for ports 0..32 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_RAVEN_MAC(_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_ALL_TILES_MAC(_postFix)                \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_TILE_MAC(_postFix, 3)

#define FEC_CE_MARK_BIT                 0x00000
/* Bit[16] in extended data */
#define FEC_NCE_MARK_BIT                0x10000

/* _tile    --> the tileId 1, 3  */
/* _raven   --> local raven 0..3 */
/* _lane    --> local lane 0..15 */
/* _index   --> pipe index 0, 1  */
#define TILE_1_3___MARK_PER_RAVEN_PER_TILE_FEC_CE_LANE_INT_MAC(_tile, _raven, _lane, index) \
    ((((_tile) * 128) + (32 * (3-(_raven))) + (_lane + (index * 16))))

#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix, _lane, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##_postFix + _lane,    FEC_CE_MARK_BIT | TILE_1_3___MARK_PER_RAVEN_PER_TILE_FEC_CE_LANE_INT_MAC(_tile, _raven, _lane, index)

/* mark extData as per pipe per tile port interrupt */
/* _tile    --> the tileId 0, 2   */
/* _raven   --> local raven 0..3 */
/* _lane    --> local lane 0..15 */
/* _index   --> pipe index 0, 1  */
#define TILE_0_2___MARK_PER_RAVEN_PER_TILE_FEC_CE_LANE_INT_MAC(_tile, _raven, _lane, index) \
    ((((_tile) * 128) + (32 * (_raven)) + (_lane + (index * 16))))

#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,_lane, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##_postFix + _lane,    FEC_CE_MARK_BIT | TILE_0_2___MARK_PER_RAVEN_PER_TILE_FEC_CE_LANE_INT_MAC(_tile, _raven, _lane, index)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      7,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      8,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      9,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      10,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      11,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      12,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      13,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      14,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      15,     _raven, _tile)


#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      7,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      8,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      9,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      10,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      11,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      12,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      13,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      14,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_CE_LANE_MAC(index,_postFix,      15,     _raven, _tile)

/* state for FEC lanes 0..63 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

/* state for FEC lanes 0..63 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_ALL_TILES_MAC(index,_postFix)                \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_TILE_MAC(index,_postFix, 3)


/* _tile    --> the tileId 1, 3  */
/* _raven   --> local raven 0..3 */
/* _lane    --> local lane 0..15 */
/* _index   --> pipe index 0, 1  */
#define TILE_1_3___MARK_PER_RAVEN_PER_TILE_FEC_NCE_LANE_INT_MAC(_tile, _raven, _lane, index) \
    ((((_tile) * 128) + (32 * (3-(_raven))) + (_lane + (index * 16))))

#define TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix, _lane, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##_postFix + _lane,    FEC_NCE_MARK_BIT | TILE_1_3___MARK_PER_RAVEN_PER_TILE_FEC_NCE_LANE_INT_MAC(_tile, _raven, _lane, index)

/* mark extData as per pipe per tile port interrupt */
/* _tile    --> the tileId 0, 2   */
/* _raven   --> local raven 0..3 */
/* _lane    --> local lane 0..15 */
/* _index   --> pipe index 0, 1  */
#define TILE_0_2___MARK_PER_RAVEN_PER_TILE_FEC_NCE_LANE_INT_MAC(_tile, _raven, _lane, index) \
    ((((_tile) * 128) + (32 * (_raven)) + (_lane + (index * 16))))

#define TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,_lane, _raven, _tile)   \
    PRV_CPSS_FALCON_TILE_##_tile##_RAVEN_##_raven##_MTI_EXT##index##_##_postFix + _lane,    FEC_NCE_MARK_BIT | TILE_0_2___MARK_PER_RAVEN_PER_TILE_FEC_NCE_LANE_INT_MAC(_tile, _raven, _lane, index)

#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      7,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      8,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      9,      _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      10,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      11,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      12,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      13,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      14,     _raven, _tile),    \
    TILE_0_2___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      15,     _raven, _tile)


#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix, _raven, _tile)   \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      0,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      1,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      2,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      3,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      4,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      5,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      6,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      7,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      8,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      9,      _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      10,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      11,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      12,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      13,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      14,     _raven, _tile),    \
    TILE_1_3___SET_EVENT_PER_RAVEN_PER_TILE_FEC_NCE_LANE_MAC(index,_postFix,      15,     _raven, _tile)

/* state for FEC lanes 0..63 that event holds extra index equal to event */
#define TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

/* state for FEC lanes 0..63 that event holds extra index equal to event */
#define TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, _tile)   \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   0,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   1,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   2,   _tile),    \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_RAVEN_MAC(index,_postFix,   3,   _tile)

#define FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_ALL_TILES_MAC(index,_postFix)                \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, 0),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, 1),          \
    TILE_0_2___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, 2),          \
    TILE_1_3___FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_TILE_MAC(index,_postFix, 3)

#define SINGLE_MG_ALL_QUEUES(tile,_mgIndex,type)              \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_0_E,  0 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_1_E,  1 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_2_E,  2 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_3_E,  3 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_4_E,  4 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_5_E,  5 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_6_E,  6 + ((_mgIndex)*8) + ((tile) * 32),     \
    PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_7_E,  7 + ((_mgIndex)*8) + ((tile) * 32)

#define SINGLE_TILE_ALL_MG_ALL_QUEUES(tile,type) \
    SINGLE_MG_ALL_QUEUES(tile,0,type),\
    SINGLE_MG_ALL_QUEUES(tile,1,type),\
    SINGLE_MG_ALL_QUEUES(tile,2,type),\
    SINGLE_MG_ALL_QUEUES(tile,3,type)

#define ALL_TILE_ALL_MG_ALL_QUEUES(type)     \
    SINGLE_TILE_ALL_MG_ALL_QUEUES(0,type),\
    SINGLE_TILE_ALL_MG_ALL_QUEUES(1,type),\
    SINGLE_TILE_ALL_MG_ALL_QUEUES(2,type),\
    SINGLE_TILE_ALL_MG_ALL_QUEUES(3,type)


/* connect unified event of 'RX SDMA' to their interrupt */
/* specific unified event to specific interrupt */
#define SINGLE_RX_SDMA_INT_MAC(type,tile,local_mgUnitId,globalQueue,localQueue) \
    CPSS_PP_##type##_QUEUE##globalQueue##_E,                              \
        PRV_CPSS_FALCON_TILE_##tile##_MG##local_mgUnitId##_RX_SDMA_##type##_QUEUE_##localQueue##_E, globalQueue , \
    MARK_END_OF_UNI_EV_CNS



#define SINGLE_TILE_SINGLE_MG_RX_SDMA_MAC(tile, mgIndex) \
    PRV_CPSS_FALCON_TILE_##tile##_MG##mgIndex##_RX_SDMA_RESOURCE_ERROR_CNT_OF_E,  ((tile)*24)+((mgIndex)*6)+0, \
    PRV_CPSS_FALCON_TILE_##tile##_MG##mgIndex##_RX_SDMA_BYTE_CNT_OF_E,            ((tile)*24)+((mgIndex)*6)+1, \
    PRV_CPSS_FALCON_TILE_##tile##_MG##mgIndex##_RX_SDMA_PACKET_CNT_OF_E,          ((tile)*24)+((mgIndex)*6)+2

#define SINGLE_TILE_ALL_MG_RX_SDMA_MAC(tile) \
    SINGLE_TILE_SINGLE_MG_RX_SDMA_MAC(tile, 0), \
    SINGLE_TILE_SINGLE_MG_RX_SDMA_MAC(tile, 1), \
    SINGLE_TILE_SINGLE_MG_RX_SDMA_MAC(tile, 2), \
    SINGLE_TILE_SINGLE_MG_RX_SDMA_MAC(tile, 3)


/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 0 */
#define MG0_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  0/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  1/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  2/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  3/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  4/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  5/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  6/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  0/*local mg*/,  7/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 1 */
#define MG1_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/,  8/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/,  9/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 10/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 11/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 12/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 13/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 14/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  1/*local mg*/, 15/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 2 */
#define MG2_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 16/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 17/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 18/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 19/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 20/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 21/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 22/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  2/*local mg*/, 23/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 3 */
#define MG3_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 24/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 25/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 26/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 27/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 28/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 29/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 30/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,0/*tile*/,  3/*local mg*/, 31/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[4..7] are connected to DP[11..8] (reverse order) therefore need to give MG4 to use Q56..63 */
/* global MG 4 */
#define MG4_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 56/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 57/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 58/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 59/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 60/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 61/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 62/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  0/*local mg*/, 63/*globalQueue*/,7/*localQueue*/)
/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[4..7] are connected to DP[11..8] (reverse order) therefore need to give MG5 to use Q48..55 */
/* global MG 5 */
#define MG5_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 48/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 49/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 50/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 51/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 52/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 53/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 54/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  1/*local mg*/, 55/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[4..7] are connected to DP[11..8] (reverse order) therefore need to give MG6 to use Q40..47 */
/* global MG 6 */
#define MG6_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 40/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 41/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 42/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 43/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 44/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 45/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 46/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  2/*local mg*/, 47/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[4..7] are connected to DP[11..8] (reverse order) therefore need to give MG7 to use Q32..39 */
/* global MG 7 */
#define MG7_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 32/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 33/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 34/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 35/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 36/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 37/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 38/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,1/*tile*/,  3/*local mg*/, 39/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 8 */
#define MG8_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 64/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 65/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 66/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 67/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 68/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 69/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 70/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  0/*local mg*/, 71/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 9 */
#define MG9_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 72/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 73/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 74/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 75/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 76/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 77/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 78/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  1/*local mg*/, 79/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 10 */
#define MG10_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 80/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 81/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 82/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 83/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 84/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 85/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 86/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  2/*local mg*/, 87/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 11 */
#define MG11_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 88/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 89/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 90/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 91/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 92/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 93/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 94/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,2/*tile*/,  3/*local mg*/, 95/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[12..15] are connected to DP[23..20] (reverse order) therefore need to give MG12 to use Q120..127 */
/* global MG 12 */
#define MG12_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,120/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,121/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,122/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,123/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,124/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,125/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,126/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  0/*local mg*/,127/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[12..15] are connected to DP[23..20] (reverse order) therefore need to give MG13 to use Q112..119 */
/* global MG 13 */
#define MG13_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,112/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,113/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,114/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,115/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,116/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,117/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,118/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  1/*local mg*/,119/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[12..15] are connected to DP[23..20] (reverse order) therefore need to give MG14 to use Q104..111 */
/* global MG 14 */
#define MG14_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,104/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,105/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,106/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,107/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,108/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,109/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,110/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  2/*local mg*/,111/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
    /* NOTE MG[12..15] are connected to DP[23..20] (reverse order) therefore need to give MG15 to use Q96..103 */
/* global MG 15 */
#define MG15_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/, 96/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/, 97/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/, 98/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/, 99/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/,100/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/,101/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/,102/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,3/*tile*/,  3/*local mg*/,103/*globalQueue*/,7/*localQueue*/)

/* connect unified event of 'RX SDMA' to their interrupt */
/* ALL MGs [0..15] */
#define ALL_MGS_RX_SDMA_INT_MAC(type)    \
    MG0_RX_SDMA_INT_MAC(type),           \
    MG1_RX_SDMA_INT_MAC(type),           \
    MG2_RX_SDMA_INT_MAC(type),           \
    MG3_RX_SDMA_INT_MAC(type),           \
    MG4_RX_SDMA_INT_MAC(type),           \
    MG5_RX_SDMA_INT_MAC(type),           \
    MG6_RX_SDMA_INT_MAC(type),           \
    MG7_RX_SDMA_INT_MAC(type),           \
    MG8_RX_SDMA_INT_MAC(type),           \
    MG9_RX_SDMA_INT_MAC(type),           \
    MG10_RX_SDMA_INT_MAC(type),          \
    MG11_RX_SDMA_INT_MAC(type),          \
    MG12_RX_SDMA_INT_MAC(type),          \
    MG13_RX_SDMA_INT_MAC(type),          \
    MG14_RX_SDMA_INT_MAC(type),          \
    MG15_RX_SDMA_INT_MAC(type)

/* support for 'multi PHA units' (we have one per pipe = 2 per tile)
    add the 'portGroupId' as : (portGroupId) << 16 */
#define PHA_INSTANCE_OFFSET(_tileId,_pipe) (((_tileId*2)+_pipe) << 16)

/* Maximal PPN events (compliment with PIPE device !!! that hold 8*4 = 32)  */
#define PPN_EVENT_TOTAL    32

#define SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, _ppn)   \
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##_ppg##_PPN##_ppn##_HOST_UNMAPPED_ACCESS_E     , ((8 + (PPN_EVENT_TOTAL * 0) + (_ppg * 9)) + (_ppn)) + PHA_INSTANCE_OFFSET(_tileId,_pipe),\
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##_ppg##_PPN##_ppn##_CORE_UNMAPPED_ACCESS_E     , ((8 + (PPN_EVENT_TOTAL * 1) + (_ppg * 9)) + (_ppn)) + PHA_INSTANCE_OFFSET(_tileId,_pipe),\
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##_ppg##_PPN##_ppn##_NEAR_EDGE_IMEM_ACCESS_E    , ((8 + (PPN_EVENT_TOTAL * 2) + (_ppg * 9)) + (_ppn)) + PHA_INSTANCE_OFFSET(_tileId,_pipe),\
    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE_##_pipe##_PPG_##_ppg##_PPN##_ppn##_DOORBELL_INTERRUPT_E       , ((8 + (PPN_EVENT_TOTAL * 3) + (_ppg * 9)) + (_ppn)) + PHA_INSTANCE_OFFSET(_tileId,_pipe)

#define SET_PPG_EVENT_PER_9_PPN_MAC(_tileId,_pipe,_ppg)\
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 0),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 1),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 2),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 3),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 4),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 5),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 6),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 7),          \
    SET_PPN_EVENT_MAC(_tileId,_pipe,_ppg, 8)


#define SET_PHA_EVENTS_MAC(_tileId,_pipe)\
   /* PHA/PPA */                         \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_BAD_ADDRESS_ERROR_E,                   0 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,        512 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_HEADER_SIZE_VIOLATION_E,             513 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PHA_INTERNAL_ERROR_PPA_CLOCK_DOWN_VIOLATION_E,          514 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   /* PPGs */                                                                                                                                       \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG0_ERROR_BAD_ADDRESS_ERROR_E,                           4 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG1_ERROR_BAD_ADDRESS_ERROR_E,                           5 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG2_ERROR_BAD_ADDRESS_ERROR_E,                           6 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
                                                                                                                                                    \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG0_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,                515 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG1_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,                516 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   PRV_CPSS_FALCON_TILE_##_tileId##_PIPE##_pipe##_PPG2_ERROR_TABLE_ACCESS_OVERLAP_ERROR_E,                517 + PHA_INSTANCE_OFFSET(_tileId,_pipe), \
   /* PPNs */                                                                                                                                        \
   SET_PPG_EVENT_PER_9_PPN_MAC(_tileId,_pipe,0),                                                                                                     \
   SET_PPG_EVENT_PER_9_PPN_MAC(_tileId,_pipe,1),                                                                                                     \
   SET_PPG_EVENT_PER_9_PPN_MAC(_tileId,_pipe,2)

#define LMU_0_LATENCY_OVER_THRESHOLD_MAC(offset)   \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  0),   ((31*offset) +  0)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  1),   ((31*offset) +  1)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  2),   ((31*offset) +  2)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  3),   ((31*offset) +  3)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  4),   ((31*offset) +  4)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  5),   ((31*offset) +  5)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  6),   ((31*offset) +  6)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  7),   ((31*offset) +  7)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  8),   ((31*offset) +  8)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) +  9),   ((31*offset) +  9)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 10),   ((31*offset) + 10)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 11),   ((31*offset) + 11)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 12),   ((31*offset) + 12)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 13),   ((31*offset) + 13)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 14),   ((31*offset) + 14)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 15),   ((31*offset) + 15)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 16),   ((31*offset) + 16)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 17),   ((31*offset) + 17)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 18),   ((31*offset) + 18)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 19),   ((31*offset) + 19)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 20),   ((31*offset) + 20)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 21),   ((31*offset) + 21)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 22),   ((31*offset) + 22)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 23),   ((31*offset) + 23)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 24),   ((31*offset) + 24)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 25),   ((31*offset) + 25)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 26),   ((31*offset) + 26)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 27),   ((31*offset) + 27)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 28),   ((31*offset) + 28)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 29),   ((31*offset) + 29)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU0_LATENCY_OVER_THRESHOLD_0_E,  0,  ((32*offset) + 30),   ((31*offset) + 30))

#define LMU_1_LATENCY_OVER_THRESHOLD_MAC(offset)   \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  0),   ((31*offset) +  0)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  1),   ((31*offset) +  1)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  2),   ((31*offset) +  2)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  3),   ((31*offset) +  3)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  4),   ((31*offset) +  4)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  5),   ((31*offset) +  5)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  6),   ((31*offset) +  6)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  7),   ((31*offset) +  7)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  8),   ((31*offset) +  8)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) +  9),   ((31*offset) +  9)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 10),   ((31*offset) + 10)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 11),   ((31*offset) + 11)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 12),   ((31*offset) + 12)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 13),   ((31*offset) + 13)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 14),   ((31*offset) + 14)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 15),   ((31*offset) + 15)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 16),   ((31*offset) + 16)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 17),   ((31*offset) + 17)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 18),   ((31*offset) + 18)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 19),   ((31*offset) + 19)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 20),   ((31*offset) + 20)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 21),   ((31*offset) + 21)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 22),   ((31*offset) + 22)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 23),   ((31*offset) + 23)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 24),   ((31*offset) + 24)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 25),   ((31*offset) + 25)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 26),   ((31*offset) + 26)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 27),   ((31*offset) + 27)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 28),   ((31*offset) + 28)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 29),   ((31*offset) + 29)), \
    SET_EVENT_WITH_INDEX_PER_LMU_PER_ALL_RAVEN_ALL_TILES_MAC(LMU1_LATENCY_OVER_THRESHOLD_0_E,  1,  ((32*offset) + 30),   ((31*offset) + 30))


#define SET_HR_TRSH_EVENT_MAC(_tileId, _pipeId,_qfcId,_registerId,_port,_tc)\
    PRV_CPSS_FALCON_TILE_##_tileId##_##_pipeId##_TXQ_##_qfcId##_INT_##_registerId##_PORT_##_port##_TC_##_tc##_CROSSED_HR_THRESHOLD_E,\
        (GT_U32)(((_tileId)<<30)+((_pipeId)<<28)+((_qfcId)<<24)+((_registerId)<<16)+((_port)<<8)+(_tc))

#define SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId, _pipeId,_qfcId,_port,_tc)\
    SET_HR_TRSH_EVENT_MAC(_tileId,_pipeId,_qfcId,0,_port,_tc),\
    SET_HR_TRSH_EVENT_MAC(_tileId,_pipeId,_qfcId,1,_port,_tc),\
    SET_HR_TRSH_EVENT_MAC(_tileId,_pipeId,_qfcId,2,_port,_tc)

#define SET_HR_TRSH_EVENT_ALL_REG_AND_TC_MAC(_tileId, _pipeId,_qfcId,_port)\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,0),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,1),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,2),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,3),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,4),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,5),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,6),\
    SET_HR_TRSH_EVENT_ALL_REG_MAC(_tileId,_pipeId,_qfcId,_port,7)

#define SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId, _pipeId,_qfcId)\
    SET_HR_TRSH_EVENT_ALL_REG_AND_TC_MAC(_tileId,_pipeId,_qfcId,0),\
    SET_HR_TRSH_EVENT_ALL_REG_AND_TC_MAC(_tileId,_pipeId,_qfcId,1),\
    SET_HR_TRSH_EVENT_ALL_REG_AND_TC_MAC(_tileId,_pipeId,_qfcId,2)

#define SET_HR_TRSH_EVENT_ALL_QFC_MAC(_tileId)\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,0,0),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,0,1),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,0,2),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,0,3),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,1,0),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,1,1),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,1,2),\
    SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tileId,1,3)


#define FALCON_COMMON_EVENTS                                                                                            \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_BUFFER_QUEUE_E,                                                                                             \
     ALL_TILE_ALL_MG_ALL_QUEUES(TX_SDMA_TX_BUFFER_QUEUE),                                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_ERR_QUEUE_E,                                                                                                \
     ALL_TILE_ALL_MG_ALL_QUEUES(TX_SDMA_TX_ERROR_QUEUE),                                                                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_END_E,                                                                                                      \
     ALL_TILE_ALL_MG_ALL_QUEUES(TX_SDMA_TX_END_QUEUE),                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_BUFFER),                                                                                    \
 /* 128 uniEvents for 128 queues , extData = queueId */                                                                 \
 ALL_MGS_RX_SDMA_INT_MAC(RX_ERR),                                                                                       \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0, MAC_TX_OVR_ERR_E),                                     \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1, MAC_TX_OVR_ERR_E),                                     \
/* seg ports*/                                                                                                          \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(0,MAC_TX_OVR_ERR_E),                                  \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(1,MAC_TX_OVR_ERR_E),                                  \
/* CPU ports */                                                                                                         \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORTS___ON_ALL_TILES_MAC(0, TX_OVR_ERR_E),                                     \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORTS___ON_ALL_TILES_MAC(1, TX_OVR_ERR_E),                                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0, RX_OVERRUN_E),                                         \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1, RX_OVERRUN_E),                                         \
/* seg ports*/                                                                                                          \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(0,RX_OVERRUN_E),                                      \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(1,RX_OVERRUN_E),                                      \
/* CPU ports */                                                                                                         \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORTS___ON_ALL_TILES_MAC(0, RX_OVERRUN_E),                                     \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_CPU_PORTS___ON_ALL_TILES_MAC(1, RX_OVERRUN_E),                                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE0_FULL_E,0),                                 \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE1_FULL_E,1),                                 \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE0_FULL_E,2),                                  \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,3),                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q0_E,0),                                      \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q1_E,1),                                      \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q0_E ,2),                                      \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q1_E,3),                                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
  /* Per Index events */                                                                                                \
 CPSS_PP_GPP_E,                                                                                                         \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP0_INTERRUPT_CAUSE_E, 0),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP1_INTERRUPT_CAUSE_E, 1),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP2_INTERRUPT_CAUSE_E, 2),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP3_INTERRUPT_CAUSE_E, 3),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP4_INTERRUPT_CAUSE_E, 4),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP5_INTERRUPT_CAUSE_E, 5),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP6_INTERRUPT_CAUSE_E, 6),                                  \
    SET_EVENT_PER_ALL_RAVEN_ALL_TILES_MAC(MG_INT0_INTERNAL_GPP7_INTERRUPT_CAUSE_E, 7),                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(0),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(1),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(2),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(3),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(4),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(5),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(6),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(7),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(8),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(9),                                                                                \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(10),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(11),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(12),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(13),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(14),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(15),                                                                               \
    LMU_0_LATENCY_OVER_THRESHOLD_MAC(16),                                                                               \
                                                                                                                        \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(0),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(1),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(2),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(3),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(4),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(5),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(6),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(7),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(8),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(9),                                                                                \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(10),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(11),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(12),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(13),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(14),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(15),                                                                               \
    LMU_1_LATENCY_OVER_THRESHOLD_MAC(16),                                                                               \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,  0 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,  1 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,  2 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,  3 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,  4 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,  5 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,  6 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,  7 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,  8 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,  9 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E, 10),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E, 11),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E, 12),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E, 13),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E, 14),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC0_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E, 15),                                \
                                                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,  0 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,  1 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,  2 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,  3 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,  4 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,  5 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,  6 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,  7 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,  8 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,  9 ),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E, 10),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E, 11),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E, 12),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E, 13),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E, 14),                                \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(CNC1_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E, 15),                                \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_RX_CNTR_OVERFLOW_E,                                                                                            \
    SINGLE_TILE_ALL_MG_RX_SDMA_MAC(0),                                                                                  \
    SINGLE_TILE_ALL_MG_RX_SDMA_MAC(1),                                                                                  \
    SINGLE_TILE_ALL_MG_RX_SDMA_MAC(2),                                                                                  \
    SINGLE_TILE_ALL_MG_RX_SDMA_MAC(3),                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                            \
    /* tile 0 */                                                                                                        \
    SET_EVENT_PER_256_INDEX_IN_PIPE_MAC(CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER, PKT_DROPED_E,  0/*_tile*/),            \
    /* tile 1 */                                                                                                        \
    SET_EVENT_PER_256_INDEX_IN_PIPE_MAC(CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER, PKT_DROPED_E,  1/*_tile*/),            \
    /* tile 2 */                                                                                                        \
    SET_EVENT_PER_256_INDEX_IN_PIPE_MAC(CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER, PKT_DROPED_E,  2/*_tile*/),            \
    /* tile 3 */                                                                                                        \
    SET_EVENT_PER_256_INDEX_IN_PIPE_MAC(CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER, PKT_DROPED_E,  3/*_tile*/),            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                                  \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR0_ILLEGAL_ACCESS_E,              0 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR1_ILLEGAL_ACCESS_E,              1 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(EPLR_ILLEGAL_ACCESS_E,               2 ),                                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                                   \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR0_IPFIX_WRAPAROUND_E,            0 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR1_IPFIX_WRAPAROUND_E,            1 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(EPLR_IPFIX_WRAPAROUND_E,             2 ),                                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                         \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR0_IPFIX_ALARM_E,                 0 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR1_IPFIX_ALARM_E,                 1 ),                                     \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(EPLR_IPFIX_ALARM_E,                  2 ),                                     \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                              \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0 ),                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPLR1_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1 ),                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(EPLR_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2 ),                        \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                       \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),                            \
                                                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),                            \
                                                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                        \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_LOOKUP0_FIFO_FULL_E,                      0),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_LOOKUP1_FIFO_FULL_E,                      1),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_LOOKUP2_FIFO_FULL_E,                      2),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                          \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         0),                            \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(IPCL_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         1),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                             \
    /* tile 0 */                                                                                                        \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL0_E    , 0,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL1_E    , 1,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL2_E    , 2,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL3_E    , 3,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL4_E    , 4,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL5_E    , 5,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL6_E    , 6,                                                               \
    PRV_CPSS_FALCON_TILE_0_MT_FDB_NA_FIFO_FULL7_E    , 7,                                                               \
                                                                                                                        \
    /* tile 1 */                                                                                                        \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL0_E    , 0,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL1_E    , 1,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL2_E    , 2,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL3_E    , 3,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL4_E    , 4,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL5_E    , 5,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL6_E    , 6,                                                               \
    PRV_CPSS_FALCON_TILE_1_MT_FDB_NA_FIFO_FULL7_E    , 7,                                                               \
                                                                                                                        \
    /* tile 2 */                                                                                                        \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL0_E    , 0,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL1_E    , 1,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL2_E    , 2,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL3_E    , 3,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL4_E    , 4,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL5_E    , 5,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL6_E    , 6,                                                               \
    PRV_CPSS_FALCON_TILE_2_MT_FDB_NA_FIFO_FULL7_E    , 7,                                                               \
                                                                                                                        \
    /* tile 3 */                                                                                                        \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL0_E    , 0,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL1_E    , 1,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL2_E    , 2,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL3_E    , 3,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL4_E    , 4,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL5_E    , 5,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL6_E    , 6,                                                               \
    PRV_CPSS_FALCON_TILE_3_MT_FDB_NA_FIFO_FULL7_E    , 7,                                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                       \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(TTI_DATA_ERROR_INT0_E, 0),                                                    \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(TTI_DATA_ERROR_INT1_E, 1),                                                    \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(TTI_DATA_ERROR_INT2_E, 2),                                                    \
    SET_EVENT_PER_ALL_PIPES_ALL_TILES_MAC(TTI_DATA_ERROR_INT3_E, 3),                                                    \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                    \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),  \
                                                                                                                        \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(BMA_SUM_MC_CNT_COUNTER_FLOW_E                                 ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(BMA_SUM_MC_CNT_PARITY_ERROR_E                                 ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(BMA_SUM_SHIFTER_LATENCY_FIFO_OVERFLOW_E                       ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(BMA_SUM_SHIFTER_LATENCY_FIFO_UNDERFLOW_E                      ),  \
    /*d2d_cp raven side*/                                                                                               \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_D2X_REQ_TIMEOUT_CNT_EXPIRED_E  /*bit1*/                                ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_MBUS_SEQ_MISMATCH_E            /*bit2*/                                ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_CPU_RX_SEQ_MISMATCH_E          /*bit3*/                                ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_CPU_FRAME_FORMAT_ERR_E         /*bit4*/                                ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_RTN_FIFO_TIMEOUT_E             /*bit5*/                                ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_CP_TAP_TIMEOUT_TRIGGERED_E        /*bit8*/                                ),  \
    /*d2d_cp eagle side*/                                                                                               \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_D2X_REQ_TIMEOUT_CNT_EXPIRED_E  /*bit1*/                                          ),  \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_MBUS_SEQ_MISMATCH_E            /*bit2*/                                          ),  \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_CPU_RX_SEQ_MISMATCH_E          /*bit3*/                                          ),  \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_CPU_FRAME_FORMAT_ERR_E         /*bit4*/                                          ),  \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_RTN_FIFO_TIMEOUT_E             /*bit5*/                                          ),  \
    SET_EVENT_ALL_TILES_MAC(D2D_CP_TAP_TIMEOUT_TRIGGERED_E        /*bit8*/                                          ),  \
\
    /*PSI_REG*/                                                                                                         \
    SET_EVENT_ALL_TILES_MAC(PSI_SCHEDULER_INTERRUPT_E   /*bit1*/                                                    ),  \
    SET_EVENT_ALL_TILES_MAC(PSI_MG2SNAKE_INTERRUPT_E    /*bit2*/                                                    ),  \
\
    /*D2D-QNM eagle side*/                                                                                              \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E  /*bit1*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E   /*bit2*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E  /*bit3*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E   /*bit4*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_LANE_ALIGN_CAUSE_E        /*bit5*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RXCAL_ALIGN_CAUSE_E       /*bit6*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RF_ERR_CAUSE_E            /*bit7*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_CAL_LOCK_CAUSE_E       /*bit8*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_REMOTE_CAL_LOST_EVENT_E   /*bit9*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_REMOTE_CAL_LOCK_EVENT_E   /*bit10*/                  ),  \
\
    /*QNM-D2D Raven side : d2d0 */                                                                                         \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E                /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E                 /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E                /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E                 /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_LANE_ALIGN_CAUSE_E                      /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RXCAL_ALIGN_CAUSE_E                     /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_CAL_LOCK_CAUSE_E                     /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOST_EVENT_E                 /*bit9*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E                 /*bit10*/                  ),  \
    /*QNM-D2D Raven side : d2d1 */                                                                                         \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E                /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E                 /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E                /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E                 /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_LANE_ALIGN_CAUSE_E                      /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RXCAL_ALIGN_CAUSE_E                     /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_CAL_LOCK_CAUSE_E                     /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOST_EVENT_E                 /*bit9*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E                 /*bit10*/                  ),  \
\
    /*D2D-QNM-PMA eagle side*/                                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_RX_CDC_OVERFLOW_CAUSE_E   /*bit1*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_RX_CDC_UNDERFLOW_CAUSE_E  /*bit2*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_TX_CDC_OVERFLOW_CAUSE_E   /*bit3*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_TX_CDC_UNDERFLOW_CAUSE_E  /*bit4*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_FE_LBK_CDC_OVERFLOW_CAUSE_E  /*bit5*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_FE_LBK_CDC_UNDERFLOW_CAUSE_E /*bit6*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_RF_ERR_CAUSE_E            /*bit7*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_RX_BLOCK_LOCK_LOSS_CAUSE_E   /*bit8*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PMA_ERR_RX_FEC_UNCOR_CAUSE_E      /*bit10*/                  ),  \
\
    /*QNM-D2D-PMA Raven side : d2d0 */                                                                                     \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_RX_CDC_OVERFLOW_CAUSE_E                 /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_RX_CDC_UNDERFLOW_CAUSE_E                /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_TX_CDC_OVERFLOW_CAUSE_E                 /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_TX_CDC_UNDERFLOW_CAUSE_E                /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_FE_LBK_CDC_OVERFLOW_CAUSE_E             /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_FE_LBK_CDC_UNDERFLOW_CAUSE_E            /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_RX_BLOCK_LOCK_LOSS_CAUSE_E              /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PMA_ERR_RX_FEC_UNCOR_CAUSE_E                    /*bit10*/                  ),  \
    /*QNM-D2D-PMA Raven side : d2d1 */                                                                                     \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_RX_CDC_OVERFLOW_CAUSE_E                 /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_RX_CDC_UNDERFLOW_CAUSE_E                /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_TX_CDC_OVERFLOW_CAUSE_E                 /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_TX_CDC_UNDERFLOW_CAUSE_E                /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_FE_LBK_CDC_OVERFLOW_CAUSE_E             /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_FE_LBK_CDC_UNDERFLOW_CAUSE_E            /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_RX_BLOCK_LOCK_LOSS_CAUSE_E              /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PMA_ERR_RX_FEC_UNCOR_CAUSE_E                    /*bit10*/                  ),  \
\
    /*D2D-QNM-PCS eagle side*/                                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E  /*bit1*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E   /*bit2*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E  /*bit3*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E   /*bit4*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_LANE_ALIGN_CAUSE_E        /*bit5*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RXCAL_ALIGN_CAUSE_E       /*bit6*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RF_ERR_CAUSE_E            /*bit7*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_RX_CAL_LOCK_CAUSE_E       /*bit8*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_REMOTE_CAL_LOST_EVENT_E   /*bit9*/                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_D2D_ALL_PIPES_PER_TILES_MAC(PCS_ERR_REMOTE_CAL_LOCK_EVENT_E   /*bit10*/                  ),  \
\
    /*QNM-D2D-PCS Raven side : d2d0 */                                                                                     \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E                /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E                 /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E                /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E                 /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_LANE_ALIGN_CAUSE_E                      /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RXCAL_ALIGN_CAUSE_E                     /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_RX_CAL_LOCK_CAUSE_E                     /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOST_EVENT_E                 /*bit9*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_0_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E                 /*bit10*/                  ),  \
    /*QNM-D2D-PCS Raven side : d2d1 */                                                                                     \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RX_PFC_UNDERFLOW_CAUSE_E                /*bit1*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RX_PFC_OVERFLOW_CAUSE_E                 /*bit2*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_TX_PFC_UNDERFLOW_CAUSE_E                /*bit3*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_TX_PFC_OVERFLOW_CAUSE_E                 /*bit4*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_LANE_ALIGN_CAUSE_E                      /*bit5*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RXCAL_ALIGN_CAUSE_E                     /*bit6*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RF_ERR_CAUSE_E                          /*bit7*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_RX_CAL_LOCK_CAUSE_E                     /*bit8*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_REMOTE_CAL_LOST_EVENT_E                 /*bit9*/                   ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(D2D_1_PCS_ERR_REMOTE_CAL_LOCK_EVENT_E                 /*bit10*/                  ),  \
\
    /*TXQ_QFC*/                                                                                                            \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_E           /*bit3*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_E           /*bit4*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_E     /*bit5*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_E     /*bit6*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_QUEUE_COUNTER_OVERFLOW_E                     /*bit9*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_QUEUE_COUNTER_UNDERFLOW_E                    /*bit10*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PORT_COUNTER_OVERFLOW_E                      /*bit11*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PORT_COUNTER_UNDERFLOW_E                     /*bit12*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PFC_COUNTER_OVERFLOW_E                       /*bit13*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PFC_COUNTER_UNDERFLOW_E                      /*bit14*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_GLOBAL_TC_COUNTER_OVERFLOW_E                 /*bit15*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_GLOBAL_TC_COUNTER_UNDERFLOW_E                /*bit16*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PB_SIZE_LESS_THAN_PB_USED_INT_E              /*bit18*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E   /*bit19*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E /*bit20*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC0_FUNC_AGG_BUFFER_COUNT_OVERFLOW_E                  /*bit21*/ ),\
                                                                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_E           /*bit3*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_E           /*bit4*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_E     /*bit5*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_E     /*bit6*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_QUEUE_COUNTER_OVERFLOW_E                     /*bit9*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_QUEUE_COUNTER_UNDERFLOW_E                    /*bit10*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PORT_COUNTER_OVERFLOW_E                      /*bit11*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PORT_COUNTER_UNDERFLOW_E                     /*bit12*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PFC_COUNTER_OVERFLOW_E                       /*bit13*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PFC_COUNTER_UNDERFLOW_E                      /*bit14*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_GLOBAL_TC_COUNTER_OVERFLOW_E                 /*bit15*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_GLOBAL_TC_COUNTER_UNDERFLOW_E                /*bit16*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PB_SIZE_LESS_THAN_PB_USED_INT_E              /*bit18*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E   /*bit19*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E /*bit20*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC1_FUNC_AGG_BUFFER_COUNT_OVERFLOW_E                  /*bit21*/ ),\
                                                                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_E           /*bit3*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_E           /*bit4*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_E     /*bit5*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_E     /*bit6*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_QUEUE_COUNTER_OVERFLOW_E                     /*bit9*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_QUEUE_COUNTER_UNDERFLOW_E                    /*bit10*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PORT_COUNTER_OVERFLOW_E                      /*bit11*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PORT_COUNTER_UNDERFLOW_E                     /*bit12*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PFC_COUNTER_OVERFLOW_E                       /*bit13*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PFC_COUNTER_UNDERFLOW_E                      /*bit14*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_GLOBAL_TC_COUNTER_OVERFLOW_E                 /*bit15*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_GLOBAL_TC_COUNTER_UNDERFLOW_E                /*bit16*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PB_SIZE_LESS_THAN_PB_USED_INT_E              /*bit18*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E   /*bit19*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E /*bit20*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC2_FUNC_AGG_BUFFER_COUNT_OVERFLOW_E                  /*bit21*/ ),\
                                                                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_QUEUE_FIELD_ENQ_OUT_OF_RANGE_INT_E           /*bit3*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_QUEUE_FIELD_DEQ_OUT_OF_RANGE_INT_E           /*bit4*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_TARGET_PORT_FIELD_ENQ_OUT_OF_RANGE_INT_E     /*bit5*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_TARGET_PORT_FIELD_DEQ_OUT_OF_RANGE_INT_E     /*bit6*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_QUEUE_COUNTER_OVERFLOW_E                     /*bit9*/  ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_QUEUE_COUNTER_UNDERFLOW_E                    /*bit10*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PORT_COUNTER_OVERFLOW_E                      /*bit11*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PORT_COUNTER_UNDERFLOW_E                     /*bit12*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PFC_COUNTER_OVERFLOW_E                       /*bit13*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PFC_COUNTER_UNDERFLOW_E                      /*bit14*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_GLOBAL_TC_COUNTER_OVERFLOW_E                 /*bit15*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_GLOBAL_TC_COUNTER_UNDERFLOW_E                /*bit16*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PB_SIZE_LESS_THAN_PB_USED_INT_E              /*bit18*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_PORT_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E   /*bit19*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E /*bit20*/ ),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_QFC3_FUNC_AGG_BUFFER_COUNT_OVERFLOW_E                  /*bit21*/ ),\
\
    /*TXQ_PFCC*/                                                                                                             \
    SET_EVENT_ALL_TILES_MAC(PFCC_TC_POOL_0_COUNTER_OVERFLOW_E      /*bit3*/                                                ),\
    SET_EVENT_ALL_TILES_MAC(PFCC_SOURCE_PORT_COUNTER_OVERFLOW_E    /*bit7*/                                                ),\
\
     /*HBU*/                                                                                                            \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(HBU_ERROR_FHF_MEM_E     /*bit1*/                              ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(HBU_ERROR_CMT1_MEM_E    /*bit2*/                              ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(HBU_ERROR_CMT2_MEM_E    /*bit3*/                              ),  \
\
    /*TXQ_SDQ*/                                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_QUEUE_CREDIT_E    /*bit3*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_QUEUE_ENQ_E       /*bit4*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_QUEUE_DEQ_E       /*bit5*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_QUEUE_QCN_E       /*bit6*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_PORT_CREDIT_E     /*bit7*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_PORT_ENQ_E        /*bit8*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_PORT_DEQ_E        /*bit9*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_PORT_QCN_E        /*bit10*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_UNMAP_PORT_PFC_E        /*bit11*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ0_FUNC_SEL_PORT_OUTOFRANGE_E   /*bit13*/               ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_QUEUE_CREDIT_E    /*bit3*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_QUEUE_ENQ_E       /*bit4*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_QUEUE_DEQ_E       /*bit5*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_QUEUE_QCN_E       /*bit6*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_PORT_CREDIT_E     /*bit7*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_PORT_ENQ_E        /*bit8*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_PORT_DEQ_E        /*bit9*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_PORT_QCN_E        /*bit10*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_UNMAP_PORT_PFC_E        /*bit11*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ1_FUNC_SEL_PORT_OUTOFRANGE_E   /*bit13*/               ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_QUEUE_CREDIT_E    /*bit3*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_QUEUE_ENQ_E       /*bit4*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_QUEUE_DEQ_E       /*bit5*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_QUEUE_QCN_E       /*bit6*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_PORT_CREDIT_E     /*bit7*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_PORT_ENQ_E        /*bit8*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_PORT_DEQ_E        /*bit9*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_PORT_QCN_E        /*bit10*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_UNMAP_PORT_PFC_E        /*bit11*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ2_FUNC_SEL_PORT_OUTOFRANGE_E   /*bit13*/               ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_QUEUE_CREDIT_E    /*bit3*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_QUEUE_ENQ_E       /*bit4*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_QUEUE_DEQ_E       /*bit5*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_QUEUE_QCN_E       /*bit6*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_PORT_CREDIT_E     /*bit7*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_PORT_ENQ_E        /*bit8*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_PORT_DEQ_E        /*bit9*/                ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_PORT_QCN_E        /*bit10*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_UNMAP_PORT_PFC_E        /*bit11*/               ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_SDQ3_FUNC_SEL_PORT_OUTOFRANGE_E   /*bit13*/               ),  \
\
    /*TXQ_PDS*/                                                                                                          \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_PB_WRITE_REPLY_FIFO_FULL_E          /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_PB_FIFO_STATE_ORDER_FIFO_FULL_E     /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_PB_FIFO_DATA_ORDER_FIFO_FULL_E      /*bit3*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_FRAG_ON_THE_AIR_CNTR_FULL_INT_E     /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E   /*bit5*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_DATAS_BMX_SAME_WR_ADDR_INT_E        /*bit6*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_TOTAL_TXQ_PDS_CNTR_OVRRUN_INT_E     /*bit7*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_DEBUG_CACHE_CNTR_OVRRUN_INT_E             /*bit8*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_PB_WRITE_REPLY_FIFO_FULL_E          /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_PB_FIFO_STATE_ORDER_FIFO_FULL_E     /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_PB_FIFO_DATA_ORDER_FIFO_FULL_E      /*bit3*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_FRAG_ON_THE_AIR_CNTR_FULL_INT_E     /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E   /*bit5*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_DATAS_BMX_SAME_WR_ADDR_INT_E        /*bit6*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_TOTAL_TXQ_PDS_CNTR_OVRRUN_INT_E     /*bit7*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_DEBUG_CACHE_CNTR_OVRRUN_INT_E             /*bit8*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_PB_WRITE_REPLY_FIFO_FULL_E          /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_PB_FIFO_STATE_ORDER_FIFO_FULL_E     /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_PB_FIFO_DATA_ORDER_FIFO_FULL_E      /*bit3*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_FRAG_ON_THE_AIR_CNTR_FULL_INT_E     /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E   /*bit5*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_DATAS_BMX_SAME_WR_ADDR_INT_E        /*bit6*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_TOTAL_TXQ_PDS_CNTR_OVRRUN_INT_E     /*bit7*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_DEBUG_CACHE_CNTR_OVRRUN_INT_E             /*bit8*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_PB_WRITE_REPLY_FIFO_FULL_E          /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_PB_FIFO_STATE_ORDER_FIFO_FULL_E     /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_PB_FIFO_DATA_ORDER_FIFO_FULL_E      /*bit3*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_FRAG_ON_THE_AIR_CNTR_FULL_INT_E     /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E   /*bit5*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_DATAS_BMX_SAME_WR_ADDR_INT_E        /*bit6*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_TOTAL_TXQ_PDS_CNTR_OVRRUN_INT_E     /*bit7*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_DEBUG_CACHE_CNTR_OVRRUN_INT_E             /*bit8*/   ),  \
\
    /*TXQ_PDS*/                                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS0_FUNC_PB_ECC_DOUBLE_ERROR_INT_E            /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS1_FUNC_PB_ECC_DOUBLE_ERROR_INT_E            /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS2_FUNC_PB_ECC_DOUBLE_ERROR_INT_E            /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(TXQ_PDS3_FUNC_PB_ECC_DOUBLE_ERROR_INT_E            /*bit4*/   ),  \
\
    /*GPC_CELL_READ*/                                                                                                   \
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR0_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR1_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR2_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC0_CR3_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR0_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR1_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR2_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_0_E/*bit1*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E         /*bit5*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E          /*bit6*/ ),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_CAM_MULTIPLE_MATCHES_E                        /*bit14*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E          /*bit15*/),\
    SET_EVENT_ALL_TILES_MAC(GPC1_CR3_CELL_REFERENCE_FREE_LIST_OVERFLOW_E           /*bit16*/),\
\
    /*NPM_MC*/                                                                                                             \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_UNIT_ERR_E    /*bit2*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_REINFAIL_E    /*bit4*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_UNIT_REFS_E   /*bit5*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_UNIT_DATA_E   /*bit6*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_REFS_ERR_E    /*bit7*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_AGING_ERR_E   /*bit8*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_REFS_FIFO_E   /*bit9*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC0_SUM_LIST_ERR_E    /*bit10*/                                                     ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_UNIT_ERR_E    /*bit2*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_REINFAIL_E    /*bit4*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_UNIT_REFS_E   /*bit5*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_UNIT_DATA_E   /*bit6*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_REFS_ERR_E    /*bit7*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_AGING_ERR_E   /*bit8*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_REFS_FIFO_E   /*bit9*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC1_SUM_LIST_ERR_E    /*bit10*/                                                     ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_UNIT_ERR_E    /*bit2*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_REINFAIL_E    /*bit4*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_UNIT_REFS_E   /*bit5*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_UNIT_DATA_E   /*bit6*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_REFS_ERR_E    /*bit7*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_AGING_ERR_E   /*bit8*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_REFS_FIFO_E   /*bit9*/                                                      ),  \
    SET_EVENT_ALL_TILES_MAC(PB_NPM_MC2_SUM_LIST_ERR_E    /*bit10*/                                                     ),  \
\
    /*SMB_MC[0x19130100]*/                                                                                                 \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC0_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC1_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC0_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC1_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC0_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC1_MISC_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E     /*bit3*/                               ),  \
\
    /*SMB_MC[0x19130150]*/                                                                                                 \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC0_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC0_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC0_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC1_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC1_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB0_MC1_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC0_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC0_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC0_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC1_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC1_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB1_MC1_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC0_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC0_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC0_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
                                                                                                                           \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC1_SUM_INTERRUPT_SUM_AGE_E                 /*bit2*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC1_SUM_INTERRUPT_SUM_RBW_E                 /*bit3*/                               ),  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB2_MC1_SUM_INTERRUPT_SUM_MC_MEMORIES2_E        /*bit5*/                               ),  \
\
    /*SMB_WRITE_ARBITER*/                                                                                                  \
    SET_EVENT_ALL_TILES_MAC(PB_SMB_WRITE_ARBITER_MISC_INTERRUPT_MC_CA_FIFO_OVERFLOW_E    /*bit2*/                       ), \
\
    /*GPC_PACKET_WRITE[0x19040200]*/                                                                                       \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_0_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_1_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_2_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC0_PACKET_WRITE_3_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_0_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_1_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_2_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_FIFO_0_E      /*bit8*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_FIFO_1_E      /*bit9*/                                     ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_FIFO_2_E      /*bit10*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_FIFO_3_E      /*bit11*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_CREDIT_0_E    /*bit12*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_CREDIT_1_E    /*bit13*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_LENGTH_0_E    /*bit18*/                                    ), \
    SET_EVENT_ALL_TILES_MAC(PB_GPC1_PACKET_WRITE_3_INT_CAUSE_LENGTH_1_E    /*bit19*/                                    ), \
\
    /*GPC_PACKET_READ[0x19080C00]*/                                                                                        \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL0_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL1_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL2_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL3_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL4_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL5_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL6_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL7_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL8_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL9_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_CHANNEL9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL0_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL1_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL2_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL3_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL4_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL5_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL6_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL7_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL8_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL9_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_CHANNEL9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL0_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL1_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL2_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL3_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL4_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL5_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL6_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL7_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL8_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL9_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_CHANNEL9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL0_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL1_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL2_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL3_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL4_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL5_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL6_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL7_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL8_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E  /*bit1*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL9_NPM_END_OF_PACKET_MISMATCH_CHANNEL_E       /*bit2*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E /*bit4*/   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_CHANNEL9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E           /*bit5*/   ),  \
\
    /*GPC_PACKET_READ_INTERRUPT_MISC[0x19080E08]*/                                                                                        \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E  /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_MULTICAST_COUNTER_OVERFLOW_E              /*bit2*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_MULTICAST_COUNTER_UNDERFLOW_E             /*bit3*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_CAM_MULTIPLE_MATCHES_E                    /*bit5*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_DESCRIPTOR_HEADER_SIZE_ERROR_E            /*bit7*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_DESCRIPTOR_TAIL_SIZE_ERROR_E              /*bit8*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_MISC_PACKET_COUNT_PACKET_SIZE_ERROR_E          /*bit9*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E  /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_MULTICAST_COUNTER_OVERFLOW_E              /*bit2*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_MULTICAST_COUNTER_UNDERFLOW_E             /*bit3*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_CAM_MULTIPLE_MATCHES_E                    /*bit5*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_DESCRIPTOR_HEADER_SIZE_ERROR_E            /*bit7*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_DESCRIPTOR_TAIL_SIZE_ERROR_E              /*bit8*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_MISC_PACKET_COUNT_PACKET_SIZE_ERROR_E          /*bit9*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E  /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_MULTICAST_COUNTER_OVERFLOW_E              /*bit2*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_MULTICAST_COUNTER_UNDERFLOW_E             /*bit3*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_CAM_MULTIPLE_MATCHES_E                    /*bit5*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_DESCRIPTOR_HEADER_SIZE_ERROR_E            /*bit7*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_DESCRIPTOR_TAIL_SIZE_ERROR_E              /*bit8*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_MISC_PACKET_COUNT_PACKET_SIZE_ERROR_E          /*bit9*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E  /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_MULTICAST_COUNTER_OVERFLOW_E              /*bit2*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_MULTICAST_COUNTER_UNDERFLOW_E             /*bit3*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_CAM_MULTIPLE_MATCHES_E                    /*bit5*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_DESCRIPTOR_HEADER_SIZE_ERROR_E            /*bit7*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_DESCRIPTOR_TAIL_SIZE_ERROR_E              /*bit8*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_MISC_PACKET_COUNT_PACKET_SIZE_ERROR_E          /*bit9*/        ),  \
\
    /*GPC_PACKET_READ_INTERRUPT_PACKET_COUNT[0x19080E00]*/                                                                                \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_COUNT_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_READ_COUNT_E    /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_COUNT_NPM_END_OF_PACKET_MISMATCH_CHANNEL_PACKET_READ_COUNT_E /*bit2*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_COUNT_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_READ_COUNT_E    /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_COUNT_NPM_END_OF_PACKET_MISMATCH_CHANNEL_PACKET_READ_COUNT_E /*bit2*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_COUNT_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_READ_COUNT_E    /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_COUNT_NPM_END_OF_PACKET_MISMATCH_CHANNEL_PACKET_READ_COUNT_E /*bit2*/        ),  \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_COUNT_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_READ_COUNT_E    /*bit1*/        ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_COUNT_NPM_END_OF_PACKET_MISMATCH_CHANNEL_PACKET_READ_COUNT_E /*bit2*/        ),  \
\
    /*GPC_PACKET_READ_INTERRUPT_PACKET_READ_CREDIT_COUNTER[0x19080E10]*/                                                                                \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW     /*bit5*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit6*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit7*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit8*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit9*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E/*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E  /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E  /*bit16*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit17*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit18*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit19*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E               /*bit20*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW     /*bit5*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit6*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit7*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit8*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit9*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E/*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E  /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E  /*bit16*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit17*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit18*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit19*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E               /*bit20*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW     /*bit5*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit6*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit7*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit8*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit9*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E/*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E  /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E  /*bit16*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit17*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit18*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit19*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E               /*bit20*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW     /*bit5*/),       \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit6*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit7*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit8*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit9*/),\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E/*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E/*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E/*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E/*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E/*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E  /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E  /*bit16*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit17*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit18*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MERGE_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit19*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_READ_CREDIT_COUNTER_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E               /*bit20*/), \
\
    /*GPC_PACKET_READ_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER[0x19080E18]*/                                                                                \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E   /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E    /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E    /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E    /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP0_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E     /*bit16*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E   /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E    /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E    /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E    /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP1_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E     /*bit16*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E   /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E    /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E    /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E    /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP2_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E     /*bit16*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E   /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E  /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E  /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E  /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E    /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E    /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_MERGE_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E    /*bit15*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(GRP3_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E     /*bit16*/), \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x007F8108]*/                                                                                \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E                                                    ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E                               ),  \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x1BEF8108]*/                                                                                \
    SET_EVENT_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E                                                              ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                  \
\
    /*RXDMA_INTERRUPT_1_CAUSE[0x0D521C88]*/                                                                                   \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PB_TAIL_ID_MEM_SER_E                          /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_FINAL_TAIL_BC_MEM_SER_E                       /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_REPLY_TAIL_BC_MEM_SER_E                       /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_REPLY_TAIL_PB_ID_MEM_SER_E                    /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PACM_MEM_0_PB_RD_DATA_ERR_E                   /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PACM_MEM_1_PB_RD_DATA_ERR_E_E                 /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PCM_MEM_0_PB_RD_DATA_ERR_E_E                  /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PCM_MEM_1_PB_RD_DATA_ERR_E_E                  /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_HEAD_REQ_FIFO_RD_SER_E                        /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_TAIL_REQ_FIFO_RD_SER_E                        /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PACKET_HEAD_FIFO_0_RD_SER_E                   /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_PACKET_HEAD_FIFO_1_RD_SER_E                   /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_E     /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_E      /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_0_RX_LOCAL_IDS_FIFO_MEM_RD_SER_E                /*bit15*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PB_TAIL_ID_MEM_SER_E                          /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_FINAL_TAIL_BC_MEM_SER_E                       /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_REPLY_TAIL_BC_MEM_SER_E                       /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_REPLY_TAIL_PB_ID_MEM_SER_E                    /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PACM_MEM_0_PB_RD_DATA_ERR_E                   /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PACM_MEM_1_PB_RD_DATA_ERR_E_E                 /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PCM_MEM_0_PB_RD_DATA_ERR_E_E                  /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PCM_MEM_1_PB_RD_DATA_ERR_E_E                  /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_HEAD_REQ_FIFO_RD_SER_E                        /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_TAIL_REQ_FIFO_RD_SER_E                        /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PACKET_HEAD_FIFO_0_RD_SER_E                   /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_PACKET_HEAD_FIFO_1_RD_SER_E                   /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_E     /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_E      /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_1_RX_LOCAL_IDS_FIFO_MEM_RD_SER_E                /*bit15*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PB_TAIL_ID_MEM_SER_E                          /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_FINAL_TAIL_BC_MEM_SER_E                       /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_REPLY_TAIL_BC_MEM_SER_E                       /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_REPLY_TAIL_PB_ID_MEM_SER_E                    /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PACM_MEM_0_PB_RD_DATA_ERR_E                   /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PACM_MEM_1_PB_RD_DATA_ERR_E_E                 /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PCM_MEM_0_PB_RD_DATA_ERR_E_E                  /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PCM_MEM_1_PB_RD_DATA_ERR_E_E                  /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_HEAD_REQ_FIFO_RD_SER_E                        /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_TAIL_REQ_FIFO_RD_SER_E                        /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PACKET_HEAD_FIFO_0_RD_SER_E                   /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_PACKET_HEAD_FIFO_1_RD_SER_E                   /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_E     /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_E      /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_2_RX_LOCAL_IDS_FIFO_MEM_RD_SER_E                /*bit15*/), \
\
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PB_TAIL_ID_MEM_SER_E                          /*bit1*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_FINAL_TAIL_BC_MEM_SER_E                       /*bit2*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_REPLY_TAIL_BC_MEM_SER_E                       /*bit3*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_REPLY_TAIL_PB_ID_MEM_SER_E                    /*bit4*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PACM_MEM_0_PB_RD_DATA_ERR_E                   /*bit5*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PACM_MEM_1_PB_RD_DATA_ERR_E_E                 /*bit6*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PCM_MEM_0_PB_RD_DATA_ERR_E_E                  /*bit7*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PCM_MEM_1_PB_RD_DATA_ERR_E_E                  /*bit8*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_HEAD_REQ_FIFO_RD_SER_E                        /*bit9*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_TAIL_REQ_FIFO_RD_SER_E                        /*bit10*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PACKET_HEAD_FIFO_0_RD_SER_E                   /*bit11*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_PACKET_HEAD_FIFO_1_RD_SER_E                   /*bit12*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_E     /*bit13*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_E      /*bit14*/), \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(RXDMA_3_RX_LOCAL_IDS_FIFO_MEM_RD_SER_E                /*bit15*/), \
\
    /*MSDB_INTERRUPT_CAUSE[0x00508404]*/                                                                                \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(MSDB0_MSDB_TX_CHANNEL_QUEUE_OVERFLOW_E                                /*bit6*/),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(MSDB0_MSDB_TX_SEGMENTED_CHANNEL_QUEUE_OVERFLOW_E                      /*bit7*/),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(MSDB1_MSDB_TX_CHANNEL_QUEUE_OVERFLOW_E                                /*bit6*/),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(MSDB1_MSDB_TX_SEGMENTED_CHANNEL_QUEUE_OVERFLOW_E                      /*bit7*/),  \
\
    /*PORTS1_INTERRUPT_CAUSE[0x1D000038]*/                                                                              \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_MG0_CNM_GRP_0_6_PIN_MEM_ECC_ERR_INT_SUM_E              /*bit18*/), \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(HA_SUM_ECC_SINGLE_ERROR_E                                     ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(HA_SUM_ECC_DOUBLE_ERROR_E                                     ),  \
                                                                                                                        \
    /* -- _PARITY_ -- */                                                                                                \
    SET_INDEX_EQUAL_EVENT_PER_TILE_MAC(TCAM_TCAM_ARRAY_PARITY_ERROR_E                                               ),  \
\
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E            ),  \
\
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E            ),  \
\
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E            ),  \
\
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E            ),  \
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E            ),  \
\
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E                           ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E                           ),  \
    SET_EVENT_ALL_RAVEN_ALL_TILES_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E                           ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_KEEP_ALIVE_AGING_E                                   ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_KEEP_ALIVE_AGING_E                                    ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_EXCESS_KEEPALIVE_E                                    ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_EXCESS_KEEPALIVE_E                                   ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_INVALID_KEEPALIVE_HASH_E                              ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_INVALID_KEEPALIVE_HASH_E                             ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_RDI_STATUS_E                                          ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_RDI_STATUS_E                                         ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_TX_PERIOD_E                                           ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_TX_PERIOD_E                                          ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_MEG_LEVEL_EXCEPTION_E                                 ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_MEG_LEVEL_EXCEPTION_E                                ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_SOURCE_ERFACE_EXCEPTION_E                             ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_SOURCE_ERFACE_EXCEPTION_E                            ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(EGR_OAM_ILLEGAL_OAM_ENTRY_INDEX_E                             ),  \
    SET_INDEX_EQUAL_EVENT_PER_ALL_PIPES_PER_TILES_MAC(INGR_OAM_ILLEGAL_OAM_ENTRY_INDEX_E                            ),  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_PHA_E,                                                                                                         \
    /* tile 0 pipe 0 */                                                                                                 \
    SET_PHA_EVENTS_MAC(0,0),                                                                                            \
    /* tile 0 pipe 1 */                                                                                                 \
    SET_PHA_EVENTS_MAC(0,1),                                                                                            \
    /* tile 1 pipe 0 */                                                                                                 \
    SET_PHA_EVENTS_MAC(1,0),                                                                                            \
    /* tile 1 pipe 1 */                                                                                                 \
    SET_PHA_EVENTS_MAC(1,1),                                                                                            \
    /* tile 2 pipe 0 */                                                                                                 \
    SET_PHA_EVENTS_MAC(2,0),                                                                                            \
    /* tile 2 pipe 1 */                                                                                                 \
    SET_PHA_EVENTS_MAC(2,1),                                                                                            \
    /* tile 3 pipe 0 */                                                                                                 \
    SET_PHA_EVENTS_MAC(3,0),                                                                                            \
    /* tile 3 pipe 1 */                                                                                                 \
    SET_PHA_EVENTS_MAC(3,1),                                                                                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E,                                                                                \
    /* tile 0 */                                                                                                        \
    SET_HR_TRSH_EVENT_ALL_QFC_MAC(0),                                                                                   \
    /* tile 1 */                                                                                                        \
    SET_HR_TRSH_EVENT_ALL_QFC_MAC(1),                                                                                   \
    /* tile 2 */                                                                                                        \
    SET_HR_TRSH_EVENT_ALL_QFC_MAC(2),                                                                                   \
    /* tile 3 */                                                                                                        \
    SET_HR_TRSH_EVENT_ALL_QFC_MAC(3),                                                                                   \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,                                                                                \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_ALL_TILES_MAC(LINK_STATUS_CHANGE_E),                              \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_SRVCPU_PORT_802_3_AP_E,                                                                                           \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_ALL_TILES_MAC(802_3_AP_E),                                        \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_SRVCPU_PORT_AP_DISABLE_E,                                                                                          \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_AP_DOORBELL___ON_ALL_TILES_MAC(AP_DISABLE_E),                                       \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS

#define PORTS_LINK_STATUS_CHANGE_EVENTS \
    /* MTI total ports 0..255 */                                                            \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0,LINK_STATUS_CHANGE_E),      \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1,LINK_STATUS_CHANGE_E),      \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0,LPCS_LINK_STATUS_CHANGE_E), \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1,LPCS_LINK_STATUS_CHANGE_E), \
/* seg ports*/                                                                              \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(0,LINK_STATUS_CHANGE_E),  \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(1,LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS \
    /* MTI total ports 0..255 */                                                            \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0, LINK_OK_CHANGE_E),         \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1, LINK_OK_CHANGE_E),         \
/* seg ports*/                                                                              \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(0, LINK_OK_CHANGE_E),     \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(1, LINK_OK_CHANGE_E)

#define PORTS_TX_FIFO_UNDERFLOW_EVENTS                                                      \
    /* MTI total ports 0..255 */                                                            \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(0, MAC_TX_UNDERFLOW_E),       \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_PORTS___ON_ALL_TILES_MAC(1, MAC_TX_UNDERFLOW_E),       \
/* seg ports*/                                                                              \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(0, MAC_TX_UNDERFLOW_E),   \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS___ON_ALL_TILES_MAC(1, MAC_TX_UNDERFLOW_E)

#define FEC_LANE_LOCKED_EVENTS \
    /* FEC CE total virtual lanes 0..511 */                                                 \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_ALL_TILES_MAC(0, FEC_CE_0_E),        \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_CE_LANES___ON_ALL_TILES_MAC(1, FEC_CE_0_E),        \
    /* FEC NCE total virtual lanes 0..511 */                                                \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_ALL_TILES_MAC(0, FEC_NCE_0_E),      \
    FALCON_SET_INDEX_EQUAL_EVENT_PER_FEC_NCE_LANES___ON_ALL_TILES_MAC(1, FEC_NCE_0_E)


#define CPU_PORT_EVENT_TILES_0_2(hwEvent,tileId,startPort)                        \
    /* CPU ports */                                                               \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_0##hwEvent,        startPort+0,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_1##hwEvent,        startPort+1,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_2##hwEvent,        startPort+2,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_3##hwEvent,        startPort+3

#define CPU_PORT_EVENT_TILES_1_3(hwEvent,tileId,startPort)                        \
    /* CPU ports- reverse order of Ravens */                                      \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_3##hwEvent,        startPort+0,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_2##hwEvent,        startPort+1,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_1##hwEvent,        startPort+2,         \
    PRV_CPSS_FALCON_TILE_##tileId##_RAVEN_0##hwEvent,        startPort+3

#define FALCON_1_TILE_CPU_PORT_EVENTS(postfix) \
    /* CPU ports */                                                                         \
    CPU_PORT_EVENT_TILES_0_2(postfix,0/*tileId*/,64/* start port*/)

#define FALCON_2_TILE_CPU_PORT_EVENTS(postfix) \
    /* CPU ports */                                                                           \
    CPU_PORT_EVENT_TILES_0_2(postfix,0/*tileId*/,128/* start port*/),\
    CPU_PORT_EVENT_TILES_1_3(postfix,1/*tileId*/,132/* start port*/)

#define FALCON_4_TILE_CPU_PORT_EVENTS(postfix) \
    /* CPU ports */                                                                           \
    CPU_PORT_EVENT_TILES_0_2(postfix,0/*tileId*/,256/* start port*/),\
    CPU_PORT_EVENT_TILES_1_3(postfix,1/*tileId*/,260/* start port*/),\
    CPU_PORT_EVENT_TILES_0_2(postfix,2/*tileId*/,264/* start port*/),\
    CPU_PORT_EVENT_TILES_1_3(postfix,3/*tileId*/,268/* start port*/)

/* Interrupt cause to unified event map for falcon
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 falcon_3_2_UniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
        FALCON_1_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_STATUS_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
        FALCON_1_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_OK_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
        PORTS_TX_FIFO_UNDERFLOW_EVENTS,
        FALCON_1_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_TX_UNDERFLOW_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
       FALCON_1_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_LINK_STATUS_CHANGE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_802_3_AP_E,
       FALCON_1_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_802_3_AP_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_AP_DISABLE_E,
       FALCON_1_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_AP_DISABLE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LANE_FEC_ERROR_E,
    FEC_LANE_LOCKED_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    FALCON_COMMON_EVENTS,
};

static const GT_U32 falcon_6_4_UniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
        FALCON_2_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_STATUS_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
        FALCON_2_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_OK_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
        PORTS_TX_FIFO_UNDERFLOW_EVENTS,
        FALCON_2_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_TX_UNDERFLOW_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
       FALCON_2_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_LINK_STATUS_CHANGE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_802_3_AP_E,
       FALCON_2_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_802_3_AP_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_AP_DISABLE_E,
       FALCON_2_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_AP_DISABLE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LANE_FEC_ERROR_E,
       FEC_LANE_LOCKED_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    FALCON_COMMON_EVENTS,
};

static const GT_U32 falcon_12_8_UniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
        FALCON_4_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_STATUS_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
        FALCON_4_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_LINK_OK_CHANGED_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
        PORTS_TX_FIFO_UNDERFLOW_EVENTS,
        FALCON_4_TILE_CPU_PORT_EVENTS(_MTIIP_CPU_TX_UNDERFLOW_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
       FALCON_4_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_LINK_STATUS_CHANGE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_802_3_AP_E,
       FALCON_4_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_802_3_AP_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_SRVCPU_PORT_AP_DISABLE_E,
       FALCON_4_TILE_CPU_PORT_EVENTS(_AP_DOORBELL_PORT_16_AP_DISABLE_E),
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LANE_FEC_ERROR_E,
       FEC_LANE_LOCKED_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

   FALCON_COMMON_EVENTS

};

#define falconUniEvMapTable_tile_x_raven_y_main_ipc(_tileId,_ravenId)  \
    {CPSS_SRVCPU_IPC_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_AP_DOORBELL_MAIN_IPC_E}

#define falconUniEvMapTable_tile_x_uburst(_tileId)  \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TXQ_QFC0_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TXQ_QFC1_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TXQ_QFC2_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TXQ_QFC3_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TXQ_QFC0_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TXQ_QFC1_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TXQ_QFC2_FUNC_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TXQ_QFC3_FUNC_UBURST_EVENT_INT_E}


#define falconUniEvMapTable_tile_x(_tileId)  \
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_BRIDGE_UPDATE_SECURITY_BREACH_E               },\
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_BRIDGE_UPDATE_SECURITY_BREACH_E               },\
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_NUM_OF_HOP_EX_P_E                            },\
    {CPSS_PP_MAC_NA_LEARNED_E,               PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_NA_LEARNT_E                                  },\
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_NA_NOT_LEARNT_E                              },\
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AGE_VIA_TRIGGER_ENDED_E                      },\
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AU_PROC_COMPLETED_E                          },\
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AU_MSG_TOCPU_READY_E                         },\
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_NA_SELF_LEARNED_E                            },\
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_N_AFROM_CPU_LEARNED_E                        },\
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_N_AFROM_CPU_DROPPED_E                        },\
    {CPSS_PP_MAC_AGED_OUT_E,                 PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AGED_OUT_E                                   },\
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_AU_FIFO_TO_CPU_IS_FULL_E                     },\
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_BRIDGE_ADDRESS_OUT_OF_RANGE_E                 },\
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_BRIDGE_ADDRESS_OUT_OF_RANGE_E                 },\
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_TTI_CPU_ADDRESS_OUT_OF_RANGE_E               },\
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_TTI_CPU_ADDRESS_OUT_OF_RANGE_E               },\
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_FALCON_TILE_##_tileId##_PIPE0_IPCL_MG_ADDR_OUT_OF_RANGE_E                   },\
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_FALCON_TILE_##_tileId##_PIPE1_IPCL_MG_ADDR_OUT_OF_RANGE_E                   },\
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_FALCON_TILE_##_tileId##_MT_FDB_BLC_OVERFLOW_E                           },\
    falconUniEvMapTable_tile_x_mg_y(_tileId,0)                                                                                     ,\
    falconUniEvMapTable_tile_x_mg_y(_tileId,1)                                                                                     ,\
    falconUniEvMapTable_tile_x_mg_y(_tileId,2)                                                                                     ,\
    falconUniEvMapTable_tile_x_mg_y(_tileId,3)                                                                                     ,\
    falconUniEvMapTable_tile_x_raven_y_main_ipc(_tileId,0)                                                                         ,\
    falconUniEvMapTable_tile_x_raven_y_main_ipc(_tileId,1)                                                                         ,\
    falconUniEvMapTable_tile_x_raven_y_main_ipc(_tileId,2)                                                                         ,\
    falconUniEvMapTable_tile_x_raven_y_main_ipc(_tileId,3)                                                                         ,\
    falconUniEvMapTable_tile_x_uburst(_tileId)

#define falconUniEvMapTable_tile_x_mg_y(_tileId,_mgIndex)  \
    {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_AU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_AU_QUEUE_FULL_E},                     \
    {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_AU_QUEUE_OVERRUN_E},                  \
    {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_AU_QUEUE_ALMOST_FULL_E},              \
    {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_FU_QUEUE_PENDING_E},                  \
    {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_FU_QUEUE_FULL_E},                     \
    {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_MG_READ_DMA_DONE_E},                  \
    {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_PCIE_ADDRESS_UNMAPPED_E},             \
    {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_I2C_TIME_OUT_INTERRUPT_E},            \
    {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_I2C_STATUS_INTERRUPT_E},              \
    {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_ILLEGAL_ADDRESS_INTERRUPT_E},         \
    {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_Z80_INTERRUPT_E},                     \
    {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_FALCON_TILE_##_tileId##_MG##_mgIndex##_CM3_TO_HOST_DOORBELL_INTERRUPT_E}    \


/* Interrupt cause to unified event map for falcon
 This Table is for unified event without extended data
*/
static const GT_U32 falconUniEvMapTable[][2] =
{
     falconUniEvMapTable_tile_x(0)
    ,falconUniEvMapTable_tile_x(1)
    ,falconUniEvMapTable_tile_x(2)
    ,falconUniEvMapTable_tile_x(3)
};

/* Interrupt cause to unified event map for Falcon without extended data size */
static const GT_U32 falconUniEvMapTableSize = (sizeof(falconUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal setFalconDedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Falcon devices.
*/
static void setFalconDedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for Falcon with extended data size */
    GT_U32 falconUniEvMapTableWithExtDataSize;

    switch (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles)
    {
        case 1:
            falconUniEvMapTableWithExtDataSize = (sizeof(falcon_3_2_UniEvMapTableWithExtData)/(sizeof(GT_U32)));
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = falcon_3_2_UniEvMapTableWithExtData;
            break;
        case 2:
            falconUniEvMapTableWithExtDataSize = (sizeof(falcon_6_4_UniEvMapTableWithExtData)/(sizeof(GT_U32)));
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = falcon_6_4_UniEvMapTableWithExtData;
            break;
        case 4:
            falconUniEvMapTableWithExtDataSize = (sizeof(falcon_12_8_UniEvMapTableWithExtData)/(sizeof(GT_U32)));
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = falcon_12_8_UniEvMapTableWithExtData;
            break;
        default:
            CPSS_LOG_ERROR_MAC("Bad number of tiles[%d] (supported 1/2/4)", PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles);
            return;
    }
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = falconUniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr              = falconUniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize             = falconUniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts              = 0;

    return;
}

/**
* @internal prvCpssRavenHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of Raven unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - the PP's device number to init the struct for.
* @param[in] tileId                - the ID of the Raven's tile.
* @param[in] gppId                  - the ID of the Raven base address space unit and unit ID.
*
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
static GT_U32 prvCpssRavenHwUnitBaseAddrGet
(
    IN GT_U32                   devNum,
    IN GT_U32                   tileId,
    IN GT_U32                   gppId
)
{
    GT_U32 ravenUnitBaseAddr;
    GT_U32 tileOffset;

    PRV_CPSS_DXCH_UNIT_ENT unitId       = UNIT_ID_GET(gppId);
    PRV_CPSS_DXCH_UNIT_ENT ravenUnitId  = RAVEN_ID_GET(gppId);

    GT_U32  baseUnitAddr = 1;   /* default to cause fatal error on simulation */

    unitId %= PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS;    /* Tile 0 unit Id */

    if(ravenUnitId >= PRV_CPSS_DXCH_UNIT_LAST_E)
    {
        /* error */
        return baseUnitAddr;
    }

    baseUnitAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, ravenUnitId, NULL);
    tileOffset   = falconOffsetFromFirstInstanceGet(devNum, tileId, ravenUnitId);

    switch (unitId)
    {
        case PRV_CPSS_DXCH_UNIT_MG_E:
            ravenUnitBaseAddr = 0x00300000;
            break;
        case PRV_CPSS_DXCH_UNIT_GOP_E:
            ravenUnitBaseAddr = 0;/* the caller need only the tile and raven offset */
            break;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_IN_RAVEN_E:
            ravenUnitBaseAddr = 0x002F0000;
            break;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E:  /* name is 'EAGLE_D2D_0' but used here as 'Raven D2D_0' */
            ravenUnitBaseAddr = 0x00680000;
            break;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E:  /* name is 'EAGLE_D2D_1' but used here as 'Raven D2D_1' */
            ravenUnitBaseAddr = 0x00690000;
            break;
        case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E:
            ravenUnitBaseAddr = 0x00700000;
            break;
        default:
            /* error */
            return baseUnitAddr;
    }

    return tileOffset + baseUnitAddr + ravenUnitBaseAddr;
}

static void falconUpdateIntScanTreeInfo(
    IN GT_U8   devNum,
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intrScanArr,
    IN GT_U32  numOfElements
)
{
    GT_U32  ii;
    GT_U32  baseAddr;
    PRV_CPSS_DXCH_UNIT_ENT  unitId, ravenUnitId;
    PRV_CPSS_DXCH_UNIT_ENT  tile0_unitId;
    GT_U32  tileId;
    GT_BOOL didError;
    GT_U32  offset;
    GT_U32  needCheck;

    for(ii = 0 ; ii < numOfElements ; ii++)
    {
        unitId       = UNIT_ID_GET((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);
        ravenUnitId  = RAVEN_ID_GET((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);
        tile0_unitId = unitId % PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS;
        tileId       = unitId / PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS;

        didError = GT_FALSE;

        if (ravenUnitId)
        {
            /* Raven unit */
            /* the Raven ravenUnitId includes the tile offset */
            baseAddr = prvCpssRavenHwUnitBaseAddrGet(devNum, tileId, intrScanArr[ii].gppId);
        }
        else
        {
            switch(tile0_unitId)
            {
                case PRV_CPSS_DXCH_UNIT_MG_E:
                case PRV_CPSS_DXCH_UNIT_MG_0_1_E:
                case PRV_CPSS_DXCH_UNIT_MG_0_2_E:
                case PRV_CPSS_DXCH_UNIT_MG_0_3_E:
                    /* get base of MG[0/1/2/3/4] */
                    baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,tile0_unitId,&didError);
                    /* get offset of '4 MGs' per tile */
                    offset = falconOffsetFromFirstInstanceGet(devNum, (tileId * 4),PRV_CPSS_DXCH_UNIT_MG_E);
                    break;
                default:
                    /* the unitId includes the tile offset */
                    baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&didError);
                    /* for pipe 1 : the intrScanArr[ii].causeRegAddr already hold the pipe offset */
                    offset = 0;
                    break;
            }

            baseAddr += offset;
        }

        if (didError == GT_TRUE)
        {
            /*support MG units */
            intrScanArr[ii].maskRegAddr  = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
        }
        else
        {
            if((baseAddr % TILE_OFFSET) == 0)
            {
                needCheck = 0;
            }
            else
            if((intrScanArr[ii].causeRegAddr % TILE_OFFSET) >= CENTRAL_OFFSET)
            {
                needCheck = 1;
            }
            else
            if((intrScanArr[ii].causeRegAddr % TILE_OFFSET) >= PIPE_0_START_ADDR &&
               (intrScanArr[ii].causeRegAddr % TILE_OFFSET) < CENTRAL_OFFSET)
            {
                if((intrScanArr[ii].causeRegAddr% TILE_OFFSET) < PIPE_1_START_ADDR) /* address in Pipe 0 but meaning to pipe 1 !!! */
                {
                    /* the cause is in pipe1 , but need to check that the 'basic address' is 0 based !!! */
                    if(((intrScanArr[ii].causeRegAddr % TILE_OFFSET) - PIPE_OFFSET) >= (baseAddr % TILE_OFFSET))
                    {
                        /* the check failed ! ... we need 'print' of the error */
                        needCheck = 1;
                    }
                    else
                    {
                        needCheck = 0;/* passed the check ! */
                    }
                }
                else  /* error : not expecting address in Pipe 1 */
                {
                    /* the check failed ! ... we need 'print' of the error */
                    needCheck = 1;
                }
            }
            else /* in raven memory space */
            {
                needCheck = 1;
            }

            if(needCheck &&
               ((intrScanArr[ii].causeRegAddr % TILE_OFFSET) > (baseAddr % TILE_OFFSET)))
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : Falcon interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
                intrScanArr[ii].causeRegAddr,
                baseAddr);

                continue;/* do not ADD the base --- ERROR */
            }

            intrScanArr[ii].causeRegAddr += baseAddr;
            intrScanArr[ii].maskRegAddr  += baseAddr;
        }
    }
}

extern GT_STATUS prvCpssDxChUnitBaseTableInit(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);

/**
* @internal prvCpssDrvInterruptFalconMaskSet function
* @endinternal
*
* @brief   masks/unmasks a given interrupt bit in the relevant
*         interrupt mask register.
*         Makes the same as prvCpssDrvInterruptMaskSet but in case of
*         interrupt PRV_CPSS_FALCON_TILE_##_tileId##_MG1_SERVER_E/
*         PRV_CPSS_FALCON_TILE_##_tileId##_RAVEN_##_ravenId##_MG_INT0_DFX_SERVER_INT_SUM_E
*         (reflects state of summary bit of DFX Server interrupt summary register) additionally
*         masks/unmasks DFX Server, DFX Clients interrupts summary registers bits
*         responsible for DFX RAMs Data Integrity events.
* @param[in] evNode                   - The interrupt node representing the interrupt to be
*                                      unmasked.
* @param[in] operation                - The  to perform, mask or unmask the interrupt
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvCpssDrvInterruptFalconMaskSet
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC *evNode,
    IN CPSS_EVENT_MASK_SET_ENT      operation
)
{
    GT_STATUS                                 rc;

    if (evNode->intMaskReg == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
    {
        /* skip the event nodes for not supported Ravens */
        return GT_OK;
    }

    rc = prvCpssDrvInterruptMaskSet(evNode, operation);
    return rc;
}

GT_VOID prvCpssDrvPpIntMaskSetFptrFalconInit
(
    IN GT_U8   devNum
)
{
    PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr = prvCpssDrvInterruptFalconMaskSet;
}



/**
* @internal prvCpssDrvPpIntDefDxChFalconInit function
* @endinternal
*
* @brief   Interrupts initialization for the Falcon devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChFalconInit
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */
    GT_U32  numOfTiles = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles;
    GT_U32  ii;
    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfoArr[] =
    {
        {/*single tile*/
            NULL,
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_1_TILE_E,
            FALCON_1_TILE_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            falconIntrScanArr_1_tile,
            PRV_CPSS_FALCON_TILE_0____LAST__E/32,
            NULL, NULL, NULL
        }
        ,{/*2 tiles*/
            NULL,
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_2_TILES_E,
            FALCON_2_TILES_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            falconIntrScanArr_2_tiles,
            PRV_CPSS_FALCON_TILE_1____LAST__E/32,
            NULL, NULL, NULL
        }
        ,{/*4 tiles*/
            NULL,
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_4_TILES_E,
            FALCON_4_TILES_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
            falconIntrScanArr_4_tiles,
            PRV_CPSS_FALCON_TILE_3____LAST__E/32,
            NULL, NULL, NULL
        }
    };

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setFalconDedicatedEventsConvertInfo(devNum);

    /* Falcon info */
    switch(numOfTiles)
    {
        case 1:
            ii = 0;
            break;
        case 2:
            ii = 1;
            break;
        case 4:
            ii = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Bad number of tiles[%d] (supported 1/2/4)",
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles);
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(falconInitDone[ii]) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(devFamilyInterrupstInfoArr, 3);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    (void)prvCpssDxChUnitBaseTableInit(CPSS_PP_FAMILY_DXCH_FALCON_E);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */

    /* 1. Align base addresses for mask registers */
    falconUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfoArr[ii].intrScanOutArr ,
        devFamilyInterrupstInfoArr[ii].numScanElements);
    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_FALCON_E         ,
            devFamilyInterrupstInfoArr[ii].numScanElements          ,
            devFamilyInterrupstInfoArr[ii].intrScanOutArr              ,
            devFamilyInterrupstInfoArr[ii].numMaskRegisters         ,
            devFamilyInterrupstInfoArr[ii].maskRegDefaultSummaryArr ,
            devFamilyInterrupstInfoArr[ii].maskRegMapArr            );
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, devFamilyInterrupstInfoArr[ii].maskRegMapArr, devFamilyInterrupstInfoArr[ii].numMaskRegisters);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[devFamilyInterrupstInfoArr[ii].intFamily]);
    devInterruptInfoPtr->numOfScanElements               = devFamilyInterrupstInfoArr[ii].numScanElements         ;
    devInterruptInfoPtr->interruptsScanArray             = devFamilyInterrupstInfoArr[ii].intrScanOutArr             ;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = devFamilyInterrupstInfoArr[ii].maskRegDefaultSummaryArr;
    devInterruptInfoPtr->maskRegistersAddressesArray     = devFamilyInterrupstInfoArr[ii].maskRegMapArr           ;

    devInterruptInfoPtr->numOfInterrupts = devFamilyInterrupstInfoArr[ii].numMaskRegisters * 32;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    prvCpssDrvPpIntMaskSetFptrFalconInit(devNum);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(falconInitDone[ii], GT_TRUE);

    return GT_OK;
}


#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefFalconPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Falcon devices
*/
void  prvCpssDrvPpIntDefFalconPrint(
    IN GT_U32   numOfTiles
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("Falcon [%d] tiles - start : \n",numOfTiles);
    switch(numOfTiles)
    {
        case 1:
            devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_1_TILE_E]);
            break;
        case 2:
            devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_2_TILES_E]);
            break;
        case 4:
            devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_4_TILES_E]);
            break;
        default:
            cpssOsPrintf("Bad number of tiles[%d] (supported 1/2/4) \n",
                numOfTiles);
            return;
    }

    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            FALCON_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("Falcon - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefFalconPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Falcon devices the register info according to value in
*         PRV_CPSS_BOBCAT3_INT_CAUSE_ENT (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefFalconPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_FALCON_INT_CAUSE_ENT   interruptId
)
{
    GT_U32  ii;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_FAMILY_ENT intFamily;

    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfoArr[] =
    {
        {/*single tile*/
            "Falcon single tile interrupts",
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_1_TILE_E,
            0,
            NULL,
            PRV_CPSS_FALCON_TILE_0____LAST__E/32,
            NULL, NULL, NULL
        }
        ,{/*2 tiles*/
            "Falcon 2 tiles interrupts",
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_2_TILES_E,
            0,
            NULL,
            PRV_CPSS_FALCON_TILE_1____LAST__E/32,
            NULL, NULL, NULL
        }
        ,{/*4 tiles*/
            "Falcon 4 tiles interrupts",
            PRV_CPSS_DRV_FAMILY_DXCH_FALCON_4_TILES_E,
            0,
            NULL,
            PRV_CPSS_FALCON_TILE_3____LAST__E/32,
            NULL, NULL, NULL
        }
    };

    cpssOsPrintf("Falcon - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_FALCON_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_FALCON_LAST_INT_E);
    }
    else
    {
        for(ii = 0 ; ii < 3 ; ii++)
        {
            if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(falconInitDone[ii]) == GT_TRUE)
            {
                intFamily = devFamilyInterrupstInfoArr[ii].intFamily;

                devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[intFamily]);

                if((GT_U32)interruptId >= (devFamilyInterrupstInfoArr[ii].numMaskRegisters << 5))
                {
                    cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                        devFamilyInterrupstInfoArr[ii].deviceNamePtr , interruptId, (devFamilyInterrupstInfoArr[ii].numMaskRegisters << 5));
                    continue;
                }

                cpssOsPrintf("start [%s]: \n",
                    devFamilyInterrupstInfoArr[ii].deviceNamePtr);

                prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_TRUE);
                cpssOsPrintf("ended [%s] \n\n",
                    devFamilyInterrupstInfoArr[ii].deviceNamePtr);
            }
        }
    }
    cpssOsPrintf("Falcon - End regInfoByInterruptIndex : \n");
}

extern GT_STATUS prvCpssPpDrvIntDefDxChHawkSdmaInterruptsArraysGet
(
    OUT const GT_U32 ** interruptRxBufQueArrPtr,
    OUT const GT_U32 ** interruptRxErrQueArrPtr,
    OUT const GT_U32 ** interruptTxBufQueArrPtr,
    OUT const GT_U32 ** interruptTxErrQueArrPtr,
    OUT const GT_U32 ** interruptTxEndQueArrPtr
);

/**
* @internal prvCpssPpDrvIntDefDxChFalconBindSdmaInterruptsToPortMappingDependedEvents function
* @endinternal
*
* @brief   Bind CpuPort SDMA Interrupts to Port Mapping Depended Events
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.

* @param[in] devNum             - device number.
* @param[in] cpuPortIndex       - index of CPU port related unified event ids.
* @param[in] globalSdmaIndex    - global index MG unit and SDMA connected to it.
*
*/
GT_STATUS prvCpssPpDrvIntDefDxChFalconBindSdmaInterruptsToPortMappingDependedEvents
(
    IN  GT_U8  devNum,
    IN  GT_U32 cpuPortIndex,
    IN  GT_U32 globalSdmaIndex
)
{
    GT_STATUS rc;
    GT_U32 numOfSdma;
    GT_U32 interruptIndex;
    CPSS_UNI_EV_CAUSE_ENT    uniEvent;
    GT_U32 uniEventExt;
    GT_U32 i;
    const GT_U32 * interruptRxBufQueArrPtr;   /* HW interrupt IDs RX BUF_QUEUE tile.mg.queue ordered */
    const GT_U32 * interruptRxErrQueArrPtr;   /* HW interrupt IDs RX ERR tile.mg.queue ordered */
    const GT_U32 * interruptTxBufQueArrPtr;   /* HW interrupt IDs TX BUFFER tile.mg ordered */
    const GT_U32 * interruptTxErrQueArrPtr;   /* HW interrupt IDs TX ERROR tile.mg ordered */
    const GT_U32 * interruptTxEndQueArrPtr;   /* HW interrupt IDs TX_END tile.mg ordered */

    /*
    Application dedicated - remapped
    Multiple ID for multiple interrupts in each MG unit.
        CPSS_PP_RX_BUFFER_QUEUE0_E 0-127
        CPSS_PP_RX_ERR_QUEUE0_E  0-127
    Single ID for multiple interrupts in each MG unit.
        CPSS_PP_RX_CNTR_OVERFLOW_E
        CPSS_PP_TX_BUFFER_QUEUE_E
        CPSS_PP_TX_ERR_QUEUE_E
        CPSS_PP_TX_END_E
    */
    #define LIST_4(_n1, _n2, _i0, _i1, _i2, _i3) \
        _n1##_i0##_n2, _n1##_i1##_n2, _n1##_i2##_n2, _n1##_i3##_n2
    #define LIST_8(_n1, _n2, _i0, _i1, _i2, _i3, _i4, _i5, _i6, _i7) \
        LIST_4(_n1, _n2, _i0, _i1, _i2, _i3), LIST_4(_n1, _n2, _i4, _i5, _i6, _i7)
    #define LIST_0_127(_n1,_n2) \
        LIST_8(_n1, _n2, 0, 1, 2, 3, 4, 5, 6, 7), \
        LIST_8(_n1, _n2, 8, 9, 10, 11, 12, 13, 14, 15), \
        LIST_8(_n1, _n2, 16, 17, 18, 19, 20, 21, 22, 23), \
        LIST_8(_n1, _n2, 24, 25, 26, 27, 28, 29, 30, 31), \
        LIST_8(_n1, _n2, 32, 33, 34, 35, 36, 37, 38, 39), \
        LIST_8(_n1, _n2, 40, 41, 42, 43, 44, 45, 46, 47), \
        LIST_8(_n1, _n2, 48, 49, 50, 51, 52, 53, 54, 55), \
        LIST_8(_n1, _n2, 56, 57, 58, 59, 60, 61, 62, 63), \
        LIST_8(_n1, _n2, 64, 65, 66, 67, 68, 69, 70, 71), \
        LIST_8(_n1, _n2, 72, 73, 74, 75, 76, 77, 78, 79), \
        LIST_8(_n1, _n2, 80, 81, 82, 83, 84, 85, 86, 87), \
        LIST_8(_n1, _n2, 88, 89, 90, 91, 92, 93, 94, 95), \
        LIST_8(_n1, _n2, 96, 97, 98, 99, 100, 101, 102, 103), \
        LIST_8(_n1, _n2, 104, 105, 106, 107, 108, 109, 110, 111), \
        LIST_8(_n1, _n2, 112, 113, 114, 115, 116, 117, 118, 119), \
        LIST_8(_n1, _n2, 120, 121, 122, 123, 124, 125, 126, 127)

    /* uniEvent queue IDs - 0-127 */
    static const CPSS_UNI_EV_CAUSE_ENT         uniEventRxBufQueueArr[128] =
    {
        LIST_0_127(CPSS_PP_RX_BUFFER_QUEUE, _E)
    };

    /* uniEvent error IDs - 0-127 */
    static const CPSS_UNI_EV_CAUSE_ENT         uniEventRxErrQueueArr[128] =
    {
        LIST_0_127(CPSS_PP_RX_ERR_QUEUE, _E)
    };

    #undef LIST_4
    #undef LIST_8
    #undef LIST_0_127

    #define FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, localQueue) \
        PRV_CPSS_FALCON_TILE_##tile##_MG##local_mgUnitId##_RX_SDMA_##type##_QUEUE_##localQueue##_E

    #define FALCON_INT_NAME_RX_SDMA_MG(type, tile, local_mgUnitId) \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 0), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 1), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 2), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 3), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 4), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 5), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 6), \
        FALCON_INT_NAME_RX_SDMA_QUEUE(type, tile, local_mgUnitId, 7)

    #define FALCON_INT_NAME_RX_SDMA_TILE(type, tile) \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 0), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 1), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 2), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 3)

    #define FALCON_INT_NAME_RX_SDMA_MIRRORED_TILE(type, tile) \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 3), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 2), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 1), \
        FALCON_INT_NAME_RX_SDMA_MG(type, tile, 0)

    #define FALCON_INT_NAME_RX_SDMA(type) \
        FALCON_INT_NAME_RX_SDMA_TILE(type, 0), \
        FALCON_INT_NAME_RX_SDMA_MIRRORED_TILE(type, 1), \
        FALCON_INT_NAME_RX_SDMA_TILE(type, 2), \
        FALCON_INT_NAME_RX_SDMA_MIRRORED_TILE(type, 3)

    #define FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,_mgIndex,type)      \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_0_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_1_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_2_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_3_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_4_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_5_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_6_E,  \
        PRV_CPSS_FALCON_TILE_##tile##_MG##_mgIndex##_##type##_7_E

    #define FALCON_INT_NAME_EVEN_TILE_ALL_MG_SINGLE_TYPE(tile,type)        \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,0,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,1,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,2,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,3,type)

    #define FALCON_INT_NAME_ODD_TILE_ALL_MG_SINGLE_TYPE(tile,type)         \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,3,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,2,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,1,type),                \
        FALCON_INT_NAME_SINGLE_MG_SINGLE_TYPE(tile,0,type)

    #define FALCON_INT_NAME_TX_SDMA(type)                                  \
        FALCON_INT_NAME_EVEN_TILE_ALL_MG_SINGLE_TYPE(0,type),              \
        FALCON_INT_NAME_ODD_TILE_ALL_MG_SINGLE_TYPE(1,type),               \
        FALCON_INT_NAME_EVEN_TILE_ALL_MG_SINGLE_TYPE(2,type),              \
        FALCON_INT_NAME_ODD_TILE_ALL_MG_SINGLE_TYPE(3,type)

    /* HW interrupt IDs RX BUF_QUEUE tile.mg.queue ordered */
    static const GT_U32 falconInterruptRxBufQueArr[] =
    {
        FALCON_INT_NAME_RX_SDMA(RX_BUFFER)
    };

    /* HW interrupt IDs RX ERR tile.mg.queue ordered */
    static const GT_U32 falconInterruptRxErrQueArr[] =
    {
        FALCON_INT_NAME_RX_SDMA(RX_ERR)
    };

    /* HW interrupt IDs TX BUFFER tile.mg ordered */
    static const GT_U32 falconInterruptTxBufQueArr[] =
    {
        FALCON_INT_NAME_TX_SDMA(TX_SDMA_TX_BUFFER_QUEUE)
    };

    /* HW interrupt IDs TX ERROR tile.mg ordered */
    static const GT_U32 falconInterruptTxErrQueArr[] =
    {
        FALCON_INT_NAME_TX_SDMA(TX_SDMA_TX_ERROR_QUEUE)
    };

    /* HW interrupt IDs TX_END tile.mg ordered */
    static const GT_U32 falconInterruptTxEndQueArr[] =
    {
        FALCON_INT_NAME_TX_SDMA(TX_SDMA_TX_END_QUEUE)
    };


    #undef FALCON_INT_NAME_RX_SDMA_QUEUE
    #undef FALCON_INT_NAME_RX_SDMA_MG
    #undef FALCON_INT_NAME_RX_SDMA_TILE
    #undef FALCON_INT_NAME_RX_SDMA_MIRRORED_TILE
    #undef FALCON_INT_NAME_RX_SDMA
    #undef FALCON_INT_NAME_TX_SDMA

    if ((CPSS_PP_FAMILY_DXCH_FALCON_E != PRV_CPSS_PP_MAC(devNum)->devFamily) &&
        (CPSS_PP_FAMILY_DXCH_AC5P_E   != PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        return GT_OK;
    }

    if (CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        interruptRxBufQueArrPtr  = falconInterruptRxBufQueArr;
        interruptRxErrQueArrPtr  = falconInterruptRxErrQueArr;
        interruptTxBufQueArrPtr  = falconInterruptTxBufQueArr;
        interruptTxErrQueArrPtr  = falconInterruptTxErrQueArr;
        interruptTxEndQueArrPtr  = falconInterruptTxEndQueArr;
        numOfSdma = 4 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    }
    else if (CPSS_PP_FAMILY_DXCH_AC5P_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        prvCpssPpDrvIntDefDxChHawkSdmaInterruptsArraysGet(
                                 &interruptRxBufQueArrPtr, &interruptRxErrQueArrPtr,
                                 &interruptTxBufQueArrPtr, &interruptTxErrQueArrPtr,
                                 &interruptTxEndQueArrPtr);
        numOfSdma = 4;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (cpuPortIndex >= numOfSdma)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (globalSdmaIndex >= numOfSdma)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*CPSS_PP_RX_BUFFER_QUEUE0_E - CPSS_PP_RX_BUFFER_QUEUE127_E*/
    for (i = 0; (i < 8); i++)
    {
        interruptIndex = interruptRxBufQueArrPtr[(globalSdmaIndex * 8) + i];
        uniEvent = uniEventRxBufQueueArr[(cpuPortIndex * 8) + i];
        /* queue number according to updated uniEvent */
        uniEventExt = ((cpuPortIndex * 8) + i);
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*CPSS_PP_RX_ERR_QUEUE0_E - CPSS_PP_RX_ERR_QUEUE127_E*/
    for (i = 0; (i < 8); i++)
    {
        interruptIndex = interruptRxErrQueArrPtr[(globalSdmaIndex * 8) + i];
        uniEvent = uniEventRxErrQueueArr[(cpuPortIndex * 8) + i];
        /* queue number according to updated uniEvent */
        uniEventExt = ((cpuPortIndex * 8) + i);
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* CPSS_PP_TX_BUFFER_QUEUE_E - eight interrupts per MG */
    uniEvent = CPSS_PP_TX_BUFFER_QUEUE_E;
    for (i = 0; (i < 8); i++)
    {
        interruptIndex = interruptTxBufQueArrPtr[(globalSdmaIndex * 8) + i];
        /* event index according to updated uniEvent */
        uniEventExt = ((cpuPortIndex * 8) + i);
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* CPSS_PP_TX_ERR_QUEUE_E - eight interrupts per MG */
    uniEvent = CPSS_PP_TX_ERR_QUEUE_E;
    for (i = 0; (i < 8); i++)
    {
        interruptIndex = interruptTxErrQueArrPtr[(globalSdmaIndex * 8) + i];
        /* event index according to updated uniEvent */
        uniEventExt = ((cpuPortIndex * 8) + i);
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* CPSS_PP_TX_END_E - eight interrupts per MG */
    uniEvent = CPSS_PP_TX_END_E;
    for (i = 0; (i < 8); i++)
    {
        interruptIndex = interruptTxEndQueArrPtr[(globalSdmaIndex * 8) + i];
        /* event index according to updated uniEvent */
        uniEventExt = ((cpuPortIndex * 8) + i);
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

