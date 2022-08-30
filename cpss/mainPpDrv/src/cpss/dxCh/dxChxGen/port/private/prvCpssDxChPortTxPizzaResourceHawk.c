
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
* @file prvCpssDxChPortTxPizzaResourceHawk; Phoenix.c
*
* @brief CPSS implementation for Tx pizza and resources configuration.
*        PB GPC Packet Read, TX DMA, TX FIFO, PCA Arbiter.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_SHARED_PORT_PRV_DB_VAR(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPrvSrc._var)

/* global options can be set before init system */
/* the defaults as HW defaults                  */

/* global enable statistics in SFF unit */
#define PRV_CPSS_DXCH_PIZZA_ARBITER_DEBUG_SFF_STATISTICS 1

/* array address and size */
#define ARRAY_ADDR_AND_SIZE(_arr) _arr, (sizeof(_arr) / sizeof(_arr[0]))

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDebugOptionsSet function
* @endinternal
*
* @brief    Sets debug options.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] debugOptions  - debug options.
*
* @retval aways returns GT_OK
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDebugOptionsSet
(
    IN GT_U32 debugOptions
)
{
    PRV_SHARED_PORT_PRV_DB_VAR(dpHawkDebugOptions) = debugOptions;
    return GT_OK;
}

#define NOT_VALID_CNS 0xFFFFFFFF

/* device initialization units sequence */
static PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT unitsInitSequenceArr[] =
{
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E
};
static GT_U32 unitsInitSequenceArrSize =
    (sizeof(unitsInitSequenceArr) / sizeof(unitsInitSequenceArr[0]));

/* channel configuration units sequence */
static PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT unitsConfigurationSequenceArr[] =
{
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E
};
static GT_U32 unitsConfigurationSequenceArrSize =
    (sizeof(unitsConfigurationSequenceArr) / sizeof(unitsConfigurationSequenceArr[0]));

/* channel power down units sequence */
static PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT unitsPowerDownSequenceArr[] =
{
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E,
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E
};

static GT_U32 unitsPowerDownSequenceArrSize =
    (sizeof(unitsPowerDownSequenceArr) / sizeof(unitsPowerDownSequenceArr[0]));

/* PB_GPC                   33 resources, 37 slots */
/* TX_DMA and TX_FIFO       32 resources, 33 slots */
/* PCA Arbiters RX and TX   32 resources, 33 slots */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_hawk_default[] =
{
    { 0, 214001, 428000, {0xFFFFFFFF,0,0,0}}, /*0-31*/
    { 0, 110001, 214000, {0x0000FFFF,0,0,0}}, /*0-15*/
    {10, 110001, 214000, {0xFFFF0000,0,0,0}}, /*16-31*/
    { 0,  50001, 110000, {0x000000FF,0,0,0}}, /*0-3*/
    { 2,  50001, 110000, {0x0000FF00,0,0,0}}, /*8-15*/
    { 5,  50001, 110000, {0x0000FF00,0,0,0}}, /*8-15*/
    {10,  50001, 110000, {0x00FF0000,0,0,0}}, /*16-23*/
    {11,  50001, 110000, {0xFF000000,0,0,0}}, /*24-31*/
    {18,  50001, 110000, {0xFF000000,0,0,0}}, /*24-31*/
    { 0,  25001,  50000, {0x0000000F,0,0,0}}, /*0-3*/
    { 1,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 5,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 2,  25001,  50000, {0x00000F00,0,0,0}}, /*8-11*/
    { 3,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    { 6,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    {10,  25001,  50000, {0x000F0000,0,0,0}}, /*16-19*/
    {11,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {14,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {18,  25001,  50000, {0x0F000000,0,0,0}}, /*24-27*/
    {19,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    {22,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    {27,      1,  50000, {0x00000000,2,0,0}}, /*33*/
    { 0,   5001,  25000, {0x00000003,0,0,0}}, /*0-1*/
    { 5,   5001,  25000, {0x0000000C,0,0,0}}, /*2-3*/
    { 1,   5001,  25000, {0x00000030,0,0,0}}, /*4-5*/
    { 9,   5001,  25000, {0x000000C0,0,0,0}}, /*6-7*/
    { 2,   5001,  25000, {0x00000300,0,0,0}}, /*8-9*/
    { 3,   5001,  25000, {0x00000C00,0,0,0}}, /*10-11*/
    { 4,   5001,  25000, {0x00000C00,0,0,0}}, /*10-11*/
    { 6,   5001,  25000, {0x00003000,0,0,0}}, /*12-13*/
    { 7,   5001,  25000, {0x0000C000,0,0,0}}, /*14-15*/
    { 8,   5001,  25000, {0x0000C000,0,0,0}}, /*14-15*/
    {10,   5001,  25000, {0x00030000,0,0,0}}, /*16-17*/
    {11,   5001,  25000, {0x000C0000,0,0,0}}, /*18-19*/
    {12,   5001,  25000, {0x000C0000,0,0,0}}, /*18-19*/
    {14,   5001,  25000, {0x00300000,0,0,0}}, /*20-21*/
    {15,   5001,  25000, {0x00C00000,0,0,0}}, /*22-23*/
    {16,   5001,  25000, {0x00C00000,0,0,0}}, /*22-23*/
    {18,   5001,  25000, {0x03000000,0,0,0}}, /*24-25*/
    {19,   5001,  25000, {0x0C000000,0,0,0}}, /*26-27*/
    {20,   5001,  25000, {0x0C000000,0,0,0}}, /*26-27*/
    {22,   5001,  25000, {0x30000000,0,0,0}}, /*28-29*/
    {23,   5001,  25000, {0xC0000000,0,0,0}}, /*30-31*/
    {24,   5001,  25000, {0xC0000000,0,0,0}}, /*30-31*/
    {26,      1,  25000, {0x00000000,1,0,0}}, /*32*/
    { 0,      1,   5000, {0x00000001,0,0,0}}, /*0*/
    { 1,      1,   5000, {0x00000010,0,0,0}}, /*4*/
    { 2,      1,   5000, {0x00000100,0,0,0}}, /*8*/
    { 3,      1,   5000, {0x00000200,0,0,0}}, /*9*/
    { 4,      1,   5000, {0x00000400,0,0,0}}, /*10*/
    { 5,      1,   5000, {0x00000800,0,0,0}}, /*11*/
    { 6,      1,   5000, {0x00001000,0,0,0}}, /*12*/
    { 7,      1,   5000, {0x00002000,0,0,0}}, /*13*/
    { 8,      1,   5000, {0x00004000,0,0,0}}, /*14*/
    { 9,      1,   5000, {0x00008000,0,0,0}}, /*15*/
    {10,      1,   5000, {0x00010000,0,0,0}}, /*16*/
    {11,      1,   5000, {0x00020000,0,0,0}}, /*7*/
    {12,      1,   5000, {0x00040000,0,0,0}}, /*18*/
    {13,      1,   5000, {0x00080000,0,0,0}}, /*19*/
    {14,      1,   5000, {0x00100000,0,0,0}}, /*20*/
    {15,      1,   5000, {0x00200000,0,0,0}}, /*21*/
    {16,      1,   5000, {0x00400000,0,0,0}}, /*22*/
    {17,      1,   5000, {0x00800000,0,0,0}}, /*23*/
    {18,      1,   5000, {0x01000000,0,0,0}}, /*24*/
    {19,      1,   5000, {0x02000000,0,0,0}}, /*25*/
    {20,      1,   5000, {0x04000000,0,0,0}}, /*26*/
    {21,      1,   5000, {0x08000000,0,0,0}}, /*27*/
    {22,      1,   5000, {0x10000000,0,0,0}}, /*28*/
    {23,      1,   5000, {0x20000000,0,0,0}}, /*29*/
    {24,      1,   5000, {0x40000000,0,0,0}}, /*30*/
    {25,      1,   5000, {0x80000000,0,0,0}}, /*31*/
    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

static const GT_U32 arbiter_slot_to_resource_id_tx_dma_hawk_default[] =
{
    /*0-15*/     0,16, 8,24, 4,20,12,28, 2,18,10,26, 6,22,14,30,
    /*16-31*/    1,17, 9,25, 5,21,13,29, 3,19,11,27, 7,23,15,31,
    /*32*/       32
};

/* PB GPC Hawk default */
static const GT_U32 arbiter_slot_to_resource_id_pb_gpc_hawk_default[] =
{
    /*0-14*/       0,16, 8, 24,  4,20,32,12, 28, 2,18, 10,26,33, 6,
    /*15-29*/     22,14,30, 1,  17,33, 9,25,  5,21,13, 29, 3,33,19,
    /*30-36*/     11,27, 7, 23, 15,31,33
};

/* Speed profiles */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_TO_PROFILE_STC speedId_to_profile_tx_dma_hawk_default[] =
{
    /* speed up profiles */
    {106000, 106001,  11},
    {107000, 107001,  11},
    {212000, 212001,  11},
    {424000, 424001,  11},
    /* profiles with HW default values */
    {214001, 428001,   0},
    {107001, 214000,   1},
    { 50001, 107000,   2},
    { 40001,  50000,   3},
    { 25001,  40000,   4},
    { 20001,  25000,   5},
    { 10001,  20000,   6},
    {  5001,  10000,   7},
    {  2501,   5000,   8},
    {  1001,   2500,   9},
    {     1,   1000,  10},
    {NOT_VALID_CNS, 0, 0} /*end of table*/
};

/* Speed profiles - use reggular profiles for speed ups */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_TO_PROFILE_STC speedId_to_profile_tx_fifo_hawk_default[] =
{
    /* profiles with HW default values */
    {214001, 428001,   0},
    {107001, 214000,   1},
    { 50001, 107000,   2},
    { 40001,  50000,   3},
    { 25001,  40000,   4},
    { 20001,  25000,   5},
    { 10001,  20000,   6},
    {  5001,  10000,   7},
    {  2501,   5000,   8},
    {  1001,   2500,   9},
    {     1,   1000,  10},
    {NOT_VALID_CNS, 0, 0} /*end of table*/
};

/* speed up configuration */
/* the same profile used for 424G, 212G, 107G and 106G */
/* 424G uses almost all DP bandwidth - other speed up will not configured on the same DP */
/* assumed that 107G and 106G will not configured on tha same device */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_UP_PROFILE_CFG_STC speed_up_config_hawk_default[] =
{
    {106000, 11, 2, 0x1043, 0x40},
    {107000, 11, 2, 0x106A, 0x41},
    {212000, 11, 1, 0x2085, 0x80},
    {424000, 11, 0, 0x4109, 0xFF},
    {NOT_VALID_CNS, 0, 0, 0, 0} /*end of table*/
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_dma_hawk_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_fifo_hawk_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_fifo_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pb_gpc_hawk_default =
{
    /** number of slots in arbiter */
    37,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    28,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_pb_gpc_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_rx_hawk_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_hawk_dp01_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    11,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    24,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_hawk_dp23_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    1,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_default,
    /** speedId to Profile Table */
    NULL
};

/* SFF                   33 resources, ((32 * 4) + 8) = 136 LLs         */
/* LLs 0-10G:4, 10G+-25G:8, 25G+-50G:16, 50G+-110G:32, 110+ and more 63 */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_hawk_sff_default[] =
{
    { 0, 110001, 428000, {0x0000FFFF,0,0,0}}, /*0-15*/
    {10, 110001, 214000, {0xFFFF0000,0,0,0}}, /*16-31*/
    { 0,  50001, 110000, {0x000000FF,0,0,0}}, /*0-3*/
    { 2,  50001, 110000, {0x0000FF00,0,0,0}}, /*8-15*/
    { 5,  50001, 110000, {0x0000FF00,0,0,0}}, /*8-15*/
    {10,  50001, 110000, {0x00FF0000,0,0,0}}, /*16-23*/
    {11,  50001, 110000, {0xFF000000,0,0,0}}, /*24-31*/
    {18,  50001, 110000, {0xFF000000,0,0,0}}, /*24-31*/
    { 0,  25001,  50000, {0x0000000F,0,0,0}}, /*0-3*/
    { 1,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 5,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 2,  25001,  50000, {0x00000F00,0,0,0}}, /*8-11*/
    { 3,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    { 6,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    {10,  25001,  50000, {0x000F0000,0,0,0}}, /*16-19*/
    {11,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {14,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {18,  25001,  50000, {0x0F000000,0,0,0}}, /*24-27*/
    {19,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    {22,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    { 0,   5001,  25000, {0x00000003,0,0,0}}, /*0-1*/
    { 5,   5001,  25000, {0x0000000C,0,0,0}}, /*2-3*/
    { 1,   5001,  25000, {0x00000030,0,0,0}}, /*4-5*/
    { 9,   5001,  25000, {0x000000C0,0,0,0}}, /*6-7*/
    { 2,   5001,  25000, {0x00000300,0,0,0}}, /*8-9*/
    { 3,   5001,  25000, {0x00000C00,0,0,0}}, /*10-11*/
    { 4,   5001,  25000, {0x00000C00,0,0,0}}, /*10-11*/
    { 6,   5001,  25000, {0x00003000,0,0,0}}, /*12-13*/
    { 7,   5001,  25000, {0x0000C000,0,0,0}}, /*14-15*/
    { 8,   5001,  25000, {0x0000C000,0,0,0}}, /*14-15*/
    {10,   5001,  25000, {0x00030000,0,0,0}}, /*16-17*/
    {11,   5001,  25000, {0x000C0000,0,0,0}}, /*18-19*/
    {12,   5001,  25000, {0x000C0000,0,0,0}}, /*18-19*/
    {14,   5001,  25000, {0x00300000,0,0,0}}, /*20-21*/
    {15,   5001,  25000, {0x00C00000,0,0,0}}, /*22-23*/
    {16,   5001,  25000, {0x00C00000,0,0,0}}, /*22-23*/
    {18,   5001,  25000, {0x03000000,0,0,0}}, /*24-25*/
    {19,   5001,  25000, {0x0C000000,0,0,0}}, /*26-27*/
    {20,   5001,  25000, {0x0C000000,0,0,0}}, /*26-27*/
    {22,   5001,  25000, {0x30000000,0,0,0}}, /*28-29*/
    {23,   5001,  25000, {0xC0000000,0,0,0}}, /*30-31*/
    {24,   5001,  25000, {0xC0000000,0,0,0}}, /*30-31*/
    {26,      1,  25000, {0x00000000,1,0,0}}, /*32*/
    { 0,      1,   5000, {0x00000001,0,0,0}}, /*0*/
    { 1,      1,   5000, {0x00000010,0,0,0}}, /*4*/
    { 2,      1,   5000, {0x00000100,0,0,0}}, /*8*/
    { 3,      1,   5000, {0x00000200,0,0,0}}, /*9*/
    { 4,      1,   5000, {0x00000400,0,0,0}}, /*10*/
    { 5,      1,   5000, {0x00000800,0,0,0}}, /*11*/
    { 6,      1,   5000, {0x00001000,0,0,0}}, /*12*/
    { 7,      1,   5000, {0x00002000,0,0,0}}, /*13*/
    { 8,      1,   5000, {0x00004000,0,0,0}}, /*14*/
    { 9,      1,   5000, {0x00008000,0,0,0}}, /*15*/
    {10,      1,   5000, {0x00010000,0,0,0}}, /*16*/
    {11,      1,   5000, {0x00020000,0,0,0}}, /*7*/
    {12,      1,   5000, {0x00040000,0,0,0}}, /*18*/
    {13,      1,   5000, {0x00080000,0,0,0}}, /*19*/
    {14,      1,   5000, {0x00100000,0,0,0}}, /*20*/
    {15,      1,   5000, {0x00200000,0,0,0}}, /*21*/
    {16,      1,   5000, {0x00400000,0,0,0}}, /*22*/
    {17,      1,   5000, {0x00800000,0,0,0}}, /*23*/
    {18,      1,   5000, {0x01000000,0,0,0}}, /*24*/
    {19,      1,   5000, {0x02000000,0,0,0}}, /*25*/
    {20,      1,   5000, {0x04000000,0,0,0}}, /*26*/
    {21,      1,   5000, {0x08000000,0,0,0}}, /*27*/
    {22,      1,   5000, {0x10000000,0,0,0}}, /*28*/
    {23,      1,   5000, {0x20000000,0,0,0}}, /*29*/
    {24,      1,   5000, {0x40000000,0,0,0}}, /*30*/
    {25,      1,   5000, {0x80000000,0,0,0}}, /*31*/
    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

/* PCA SFF Fifo elements resources Hawk default*/
static const GT_U32 arbiter_slot_to_resource_id_pca_sff_hawk1[] =
{
    /*0-15*/     0, 0, 0, 0,  1, 1, 1, 1,   2, 2, 2, 2,  3, 3, 3, 3,
    /*16-31*/    4, 4, 4, 4,  5, 5, 5, 5,   6, 6, 6, 6,  7, 7, 7, 7,
    /*32-47*/    8, 8, 8, 8,  9, 9, 9, 9,  10,10,10,10, 11,11,11,11,
    /*48-63*/   12,12,12,12, 13,13,13,13,  14,14,14,14, 15,15,15,15,
    /*64-79*/   16,16,16,16, 17,17,17,17,  18,18,18,18, 19,19,19,19,
    /*80-95*/   20,20,20,20, 21,21,21,21,  22,22,22,22, 23,23,23,23,
    /*96-111*/  24,24,24,24, 25,25,25,25,  26,26,26,26, 27,27,27,27,
    /*112-127*/ 28,28,28,28, 29,29,29,29, 30,30,30,30,  31,31,31,31,
    /*128-136*/ 32,32,32,32,  32,32,32,32
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_sff_hawk_default =
{
    /** number of slots in arbiter */
    136,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_sff_hawk1,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_hawk_sff_default,
    /** speedId to Profile Table */
    NULL
};


static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC sff_fifo_width_phoenix[] =
{/*only groups of channels for any speed */
    {{0xFFFFFFFF, 0x0003FFFF},     0, 0}, /* channels 0-49  Depth-8B*/
    {{0x00000000, 0x003C0000},     0, 1}, /* channels 50-53 Depth-32B*/
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC sff_max_of_tokens_hawk[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0, 120},
    {{0, 0},NOT_VALID_CNS, 0},
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC sff_max_of_tokens_phoenix[] =
{/*only groups of channels for any speed */
    {{0x01010101, 0x00000101},     0, 176},/* channels 0,8,16,24,32,40  value 176 */
    {{0xFEFEFEFE, 0x0000FEFE},     0, 112},/* channels 1-7,9-15,17-23,25-31,33-39,41-47  value 112 */
    {{0x00000000, 0x00030000},     0, 176},/* channels 48-49  value 176 */
    {{0x00000000, 0x00040000},     0, 128},/* channels 50  value 128 */
    {{0x00000000, 0x00100000},     0,  80},/* channels 52  value 80 */
    {{0x00000000, 0x00280000},     0,  72},/* channels 51,53  value 72 */
    {{0, 0},NOT_VALID_CNS, 0},
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC brg_tx_credits_alloc_hawk[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   9},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 10000,  12},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 20000,  16},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 40000,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF},100000,  64},
    {{0xFFFFFFFF, 0xFFFFFFFF},200000, 100},
    {{0xFFFFFFFF, 0xFFFFFFFF},400000, 180},
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC brg_tx_credits_alloc_phoenix[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},      0,  9}, /* 0 - 5G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},   5000, 12}, /* 5G - 10G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  10000, 15}, /* 10G - 20G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  20000, 22}, /* 20G - 40G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  40000, 44}, /* 40G - 100G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF}, 100000, 88}, /* 100G and more */
    {{0, 0},NOT_VALID_CNS, 0}
};

 /* TF_200830 aligned */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC brg_tx_credits_alloc_harrier[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,  20},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 25001,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 50001,  64},
    {{0xFFFFFFFF, 0xFFFFFFFF},110001, 112},
    {{0xFFFFFFFF, 0xFFFFFFFF},214001, 224},
    {{0, 0},NOT_VALID_CNS, 0}
};

/* PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E */

static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_phoenix_default[] =
{
    { 0,      0,    2500, {0x00000001,0x00000000,0,0}}, /*0*/
    { 1,      0,    2500, {0x00000002,0x00000000,0,0}}, /*1*/
    { 2,      0,    2500, {0x00000004,0x00000000,0,0}}, /*2*/
    { 3,      0,    2500, {0x00000008,0x00000000,0,0}}, /*3*/
    { 4,      0,    2500, {0x00000010,0x00000000,0,0}}, /*4*/
    { 5,      0,    2500, {0x00000020,0x00000000,0,0}}, /*5*/
    { 6,      0,    2500, {0x00000040,0x00000000,0,0}}, /*6*/
    { 7,      0,    2500, {0x00000080,0x00000000,0,0}}, /*7*/
    { 8,      0,    2500, {0x00000100,0x00000000,0,0}}, /*8*/
    { 9,      0,    2500, {0x00000200,0x00000000,0,0}}, /*9*/
    {10,      0,    2500, {0x00000400,0x00000000,0,0}}, /*10*/
    {11,      0,    2500, {0x00000800,0x00000000,0,0}}, /*11*/
    {12,      0,    2500, {0x00001000,0x00000000,0,0}}, /*12*/
    {13,      0,    2500, {0x00002000,0x00000000,0,0}}, /*13*/
    {14,      0,    2500, {0x00004000,0x00000000,0,0}}, /*14*/
    {15,      0,    2500, {0x00008000,0x00000000,0,0}}, /*15*/
    {16,      0,    2500, {0x00010000,0x00000000,0,0}}, /*16*/
    {17,      0,    2500, {0x00020000,0x00000000,0,0}}, /*17*/
    {18,      0,    2500, {0x00040000,0x00000000,0,0}}, /*18*/
    {19,      0,    2500, {0x00080000,0x00000000,0,0}}, /*19*/
    {20,      0,    2500, {0x00100000,0x00000000,0,0}}, /*20*/
    {21,      0,    2500, {0x00200000,0x00000000,0,0}}, /*21*/
    {22,      0,    2500, {0x00400000,0x00000000,0,0}}, /*22*/
    {23,      0,    2500, {0x00800000,0x00000000,0,0}}, /*23*/
    {24,      0,    2500, {0x01000000,0x00000000,0,0}}, /*24*/
    {25,      0,    2500, {0x02000000,0x00000000,0,0}}, /*25*/
    {26,      0,    2500, {0x04000000,0x00000000,0,0}}, /*26*/
    {27,      0,    2500, {0x08000000,0x00000000,0,0}}, /*27*/
    {28,      0,    2500, {0x10000000,0x00000000,0,0}}, /*28*/
    {29,      0,    2500, {0x20000000,0x00000000,0,0}}, /*29*/
    {30,      0,    2500, {0x40000000,0x00000000,0,0}}, /*30*/
    {31,      0,    2500, {0x80000000,0x00000000,0,0}}, /*31*/
    {32,      0,    2500, {0x00000000,0x00000001,0,0}}, /*32*/
    {33,      0,    2500, {0x00000000,0x00000002,0,0}}, /*33*/
    {34,      0,    2500, {0x00000000,0x00000004,0,0}}, /*34*/
    {35,      0,    2500, {0x00000000,0x00000008,0,0}}, /*35*/
    {36,      0,    2500, {0x00000000,0x00000010,0,0}}, /*36*/
    {37,      0,    2500, {0x00000000,0x00000020,0,0}}, /*37*/
    {38,      0,    2500, {0x00000000,0x00000040,0,0}}, /*38*/
    {39,      0,    2500, {0x00000000,0x00000080,0,0}}, /*39*/
    {40,      0,    2500, {0x00000000,0x00000100,0,0}}, /*40*/
    {41,      0,    2500, {0x00000000,0x00000200,0,0}}, /*41*/
    {42,      0,    2500, {0x00000000,0x00000400,0,0}}, /*42*/
    {43,      0,    2500, {0x00000000,0x00000800,0,0}}, /*43*/
    {44,      0,    2500, {0x00000000,0x00001000,0,0}}, /*44*/
    {45,      0,    2500, {0x00000000,0x00002000,0,0}}, /*45*/
    {46,      0,    2500, {0x00000000,0x00004000,0,0}}, /*46*/
    {47,      0,    2500, {0x00000000,0x00008000,0,0}}, /*47*/
    { 0,   2501,    5000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   2501,    5000, {0x0000000C,0x00000000,0,0}}, /*2,3*/
    { 2,   2501,    5000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 3,   2501,    5000, {0x000000C0,0x00000000,0,0}}, /*6,7*/
    { 8,   2501,    5000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   2501,    5000, {0x00000C00,0x00000000,0,0}}, /*10,11*/
    {10,   2501,    5000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {11,   2501,    5000, {0x0000C000,0x00000000,0,0}}, /*14,15*/
    {16,   2501,    5000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   2501,    5000, {0x000C0000,0x00000000,0,0}}, /*18,19*/
    {18,   2501,    5000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {19,   2501,    5000, {0x00C00000,0x00000000,0,0}}, /*22,23*/
    {24,   2501,    5000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   2501,    5000, {0x0C000000,0x00000000,0,0}}, /*26,27*/
    {26,   2501,    5000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {27,   2501,    5000, {0xC0000000,0x00000000,0,0}}, /*30,31*/
    {32,   2501,    5000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   2501,    5000, {0x00000000,0x0000000C,0,0}}, /*34,35,*/
    {34,   2501,    5000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {35,   2501,    5000, {0x00000000,0x000000C0,0,0}}, /*38,39*/
    {40,   2501,    5000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   2501,    5000, {0x00000000,0x00000C00,0,0}}, /*42,43*/
    {42,   2501,    5000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    {43,   2501,    5000, {0x00000000,0x0000C000,0,0}}, /*46,47*/
    { 0,   5001,   10000, {0x0000000F,0x00000000,0,0}}, /*0-3*/
    { 1,   5001,   10000, {0x000000F0,0x00000000,0,0}}, /*4-7*/
    { 8,   5001,   10000, {0x00000F00,0x00000000,0,0}}, /*8-11*/
    { 9,   5001,   10000, {0x0000F000,0x00000000,0,0}}, /*12-15*/
    {16,   5001,   10000, {0x000F0000,0x00000000,0,0}}, /*16-19*/
    {17,   5001,   10000, {0x00F00000,0x00000000,0,0}}, /*20-23*/
    {24,   5001,   10000, {0x0F000000,0x00000000,0,0}}, /*24-27*/
    {25,   5001,   10000, {0xF0000000,0x00000000,0,0}}, /*28-31*/
    {32,   5001,   10000, {0x00000000,0x0000000F,0,0}}, /*32-35*/
    {33,   5001,   10000, {0x00000000,0x000000F0,0,0}}, /*36-39*/
    {40,   5001,   10000, {0x00000000,0x00000F00,0,0}}, /*40-43*/
    {41,   5001,   10000, {0x00000000,0x0000F000,0,0}}, /*44-47*/
    { 0,  10001,   25000, {0x000000FF,0x00000000,0,0}}, /*0-7*/
    { 8,  10001,   25000, {0x0000FF00,0x00000000,0,0}}, /*8-15*/
    {16,  10001,   25000, {0x00FF0000,0x00000000,0,0}}, /*16-23*/
    {24,  10001,   25000, {0xFF000000,0x00000000,0,0}}, /*24-31*/
    {32,  10001,   25000, {0x00000000,0x000000FF,0,0}}, /*32-39*/
    {40,  10001,   25000, {0x00000000,0x0000FF00,0,0}}, /*40-47*/
    {48,      0,   25000, {0x00000000,0x00010000,0,0}}, /*48*/
    {49,      0,   25000, {0x00000000,0x00020000,0,0}}, /*49*/
    {50,      0,   25000, {0x00000000,0x00040000,0,0}}, /*50*/
    {51,      0,   25000, {0x00000000,0x00080000,0,0}}, /*51*/
    {52,      0,   25000, {0x00000000,0x00100000,0,0}}, /*52*/
    {53,      0,   25000, {0x00000000,0x00200000,0,0}}, /*53*/
    {50,  25001,   50000, {0x00000000,0x000C0000,0,0}}, /*50-51*/
    {52,  25001,   50000, {0x00000000,0x00300000,0,0}}, /*52-53*/
    {50,  50001,  100000, {0x00000000,0x003C0000,0,0}}, /*50-53*/
    {54,      0,    5000, {0x00000000,0x00400000,0,0}}, /*54*/
    {55,      0,    5000, {0x00000000,0x00800000,0,0}}, /*55*/
    {54,   5001,   10000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {55,   5001,   10000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {56,      0,   50000, {0x00000000,0x01000000,0,0}}, /*56*/

    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

static const GT_U32 arbiter_slot_to_resource_id_tx_dma_phoenix_default[] =
{
    /*0-11*/    0, 32, 50, 16, 48, 52,  8, 40, 51, 24, 49, 53,
    /*12-24*/   4, 36, 50, 20, 48, 52, 12, 44, 51, 28, 49, 53, 54,
    /*25-36*/   2, 34, 50, 18, 48, 52, 10, 42, 51, 26, 49, 53,
    /*37-49*/   6, 38, 50, 22, 48, 52, 14, 46, 51, 30, 49, 53, 55,
    /*50-61*/   1, 33, 50, 17, 48, 52,  9, 41, 51, 25, 49, 53,
    /*62-74*/   5, 37, 50, 21, 48, 52, 13, 45, 51, 29, 49, 53, 54,
    /*75-86*/   3, 35, 50, 19, 48, 52, 11, 43, 51, 27, 49, 53,
    /*87-99*/   7, 39, 50, 23, 48, 52, 15, 47, 51, 31, 49, 53, 55
};

static const GT_U32 arbiter_slot_to_resource_id_pb_gpc_phoenix_default[] =
{
    /*0-13*/     0, 32, 50, 56, 16, 48, 52,  8, 40, 51, 56, 24, 49, 53,
    /*14-28*/    4, 36, 50, 56, 20, 48, 52, 12, 44, 51, 56, 28, 49, 53, 54,
    /*29-42*/    2, 34, 50, 56, 18, 48, 52, 10, 42, 51, 56, 26, 49, 53,
    /*43-57*/    6, 38, 50, 56, 22, 48, 52, 14, 46, 51, 56, 30, 49, 53, 55,
    /*58-71*/    1, 33, 50, 56, 17, 48, 52,  9, 41, 51, 56, 25, 49, 53,
    /*72-86*/    5, 37, 50, 56, 21, 48, 52, 13, 45, 51, 56, 29, 49, 53, 54,
    /*87-100*/   3, 35, 50, 56, 19, 48, 52, 11, 43, 51, 56, 27, 49, 53,
    /*101-115*/  7, 39, 50, 56, 23, 48, 52, 15, 47, 51, 56, 31, 49, 53, 55
};

static const GT_U32 arbiter_slot_to_resource_id_pca_arbiter_phoenix_default[] =
{
    /*0-11*/    0, 32, 50, 16, 48, 52,  8, 40, 51, 24, 49, 53,
    /*12-23*/   4, 36, 50, 20, 48, 52, 12, 44, 51, 28, 49, 53,
    /*24-35*/   2, 34, 50, 18, 48, 52, 10, 42, 51, 26, 49, 53,
    /*36-47*/   6, 38, 50, 22, 48, 52, 14, 46, 51, 30, 49, 53,
    /*48-59*/   1, 33, 50, 17, 48, 52,  9, 41, 51, 25, 49, 53,
    /*60-71*/   5, 37, 50, 21, 48, 52, 13, 45, 51, 29, 49, 53,
    /*72-83*/   3, 35, 50, 19, 48, 52, 11, 43, 51, 27, 49, 53,
    /*84-95*/   7, 39, 50, 23, 48, 52, 15, 47, 51, 31, 49, 53
};

/* only for phoenix SFF rings to match the SFF designers tests      */
/* resource here is just 4 LLs: resource-N id LLs (4*N) - ((4*N)+3) */
/* LLs ring sizes will also be reduced accorging to limits table    */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_sff_phoenix_default[] =
{
    { 0,      0,    2500, {0x00000001,0x00000000,0,0}}, /*0*/
    { 1,      0,    2500, {0x00000002,0x00000000,0,0}}, /*1*/
    { 2,      0,    2500, {0x00000004,0x00000000,0,0}}, /*2*/
    { 3,      0,    2500, {0x00000008,0x00000000,0,0}}, /*3*/
    { 4,      0,    2500, {0x00000010,0x00000000,0,0}}, /*4*/
    { 5,      0,    2500, {0x00000020,0x00000000,0,0}}, /*5*/
    { 6,      0,    2500, {0x00000040,0x00000000,0,0}}, /*6*/
    { 7,      0,    2500, {0x00000080,0x00000000,0,0}}, /*7*/
    { 8,      0,    2500, {0x00000100,0x00000000,0,0}}, /*8*/
    { 9,      0,    2500, {0x00000200,0x00000000,0,0}}, /*9*/
    {10,      0,    2500, {0x00000400,0x00000000,0,0}}, /*10*/
    {11,      0,    2500, {0x00000800,0x00000000,0,0}}, /*11*/
    {12,      0,    2500, {0x00001000,0x00000000,0,0}}, /*12*/
    {13,      0,    2500, {0x00002000,0x00000000,0,0}}, /*13*/
    {14,      0,    2500, {0x00004000,0x00000000,0,0}}, /*14*/
    {15,      0,    2500, {0x00008000,0x00000000,0,0}}, /*15*/
    {16,      0,    2500, {0x00010000,0x00000000,0,0}}, /*16*/
    {17,      0,    2500, {0x00020000,0x00000000,0,0}}, /*17*/
    {18,      0,    2500, {0x00040000,0x00000000,0,0}}, /*18*/
    {19,      0,    2500, {0x00080000,0x00000000,0,0}}, /*19*/
    {20,      0,    2500, {0x00100000,0x00000000,0,0}}, /*20*/
    {21,      0,    2500, {0x00200000,0x00000000,0,0}}, /*21*/
    {22,      0,    2500, {0x00400000,0x00000000,0,0}}, /*22*/
    {23,      0,    2500, {0x00800000,0x00000000,0,0}}, /*23*/
    {24,      0,    2500, {0x01000000,0x00000000,0,0}}, /*24*/
    {25,      0,    2500, {0x02000000,0x00000000,0,0}}, /*25*/
    {26,      0,    2500, {0x04000000,0x00000000,0,0}}, /*26*/
    {27,      0,    2500, {0x08000000,0x00000000,0,0}}, /*27*/
    {28,      0,    2500, {0x10000000,0x00000000,0,0}}, /*28*/
    {29,      0,    2500, {0x20000000,0x00000000,0,0}}, /*29*/
    {30,      0,    2500, {0x40000000,0x00000000,0,0}}, /*30*/
    {31,      0,    2500, {0x80000000,0x00000000,0,0}}, /*31*/
    {32,      0,    2500, {0x00000000,0x00000001,0,0}}, /*32*/
    {33,      0,    2500, {0x00000000,0x00000002,0,0}}, /*33*/
    {34,      0,    2500, {0x00000000,0x00000004,0,0}}, /*34*/
    {35,      0,    2500, {0x00000000,0x00000008,0,0}}, /*35*/
    {36,      0,    2500, {0x00000000,0x00000010,0,0}}, /*36*/
    {37,      0,    2500, {0x00000000,0x00000020,0,0}}, /*37*/
    {38,      0,    2500, {0x00000000,0x00000040,0,0}}, /*38*/
    {39,      0,    2500, {0x00000000,0x00000080,0,0}}, /*39*/
    {40,      0,    2500, {0x00000000,0x00000100,0,0}}, /*40*/
    {41,      0,    2500, {0x00000000,0x00000200,0,0}}, /*41*/
    {42,      0,    2500, {0x00000000,0x00000400,0,0}}, /*42*/
    {43,      0,    2500, {0x00000000,0x00000800,0,0}}, /*43*/
    {44,      0,    2500, {0x00000000,0x00001000,0,0}}, /*44*/
    {45,      0,    2500, {0x00000000,0x00002000,0,0}}, /*45*/
    {46,      0,    2500, {0x00000000,0x00004000,0,0}}, /*46*/
    {47,      0,    2500, {0x00000000,0x00008000,0,0}}, /*47*/
    { 0,   2501,    5000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   2501,    5000, {0x0000000C,0x00000000,0,0}}, /*2,3*/
    { 2,   2501,    5000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 3,   2501,    5000, {0x000000C0,0x00000000,0,0}}, /*6,7*/
    { 8,   2501,    5000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   2501,    5000, {0x00000C00,0x00000000,0,0}}, /*10,11*/
    {10,   2501,    5000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {11,   2501,    5000, {0x0000C000,0x00000000,0,0}}, /*14,15*/
    {16,   2501,    5000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   2501,    5000, {0x000C0000,0x00000000,0,0}}, /*18,19*/
    {18,   2501,    5000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {19,   2501,    5000, {0x00C00000,0x00000000,0,0}}, /*22,23*/
    {24,   2501,    5000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   2501,    5000, {0x0C000000,0x00000000,0,0}}, /*26,27*/
    {26,   2501,    5000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {27,   2501,    5000, {0xC0000000,0x00000000,0,0}}, /*30,31*/
    {32,   2501,    5000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   2501,    5000, {0x00000000,0x0000000C,0,0}}, /*34,35,*/
    {34,   2501,    5000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {35,   2501,    5000, {0x00000000,0x000000C0,0,0}}, /*38,39*/
    {40,   2501,    5000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   2501,    5000, {0x00000000,0x00000C00,0,0}}, /*42,43*/
    {42,   2501,    5000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    {43,   2501,    5000, {0x00000000,0x0000C000,0,0}}, /*46,47*/
    { 0,   5001,   10000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   5001,   10000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 8,   5001,   10000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   5001,   10000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {16,   5001,   10000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   5001,   10000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {24,   5001,   10000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   5001,   10000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {32,   5001,   10000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   5001,   10000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {40,   5001,   10000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   5001,   10000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    { 0,  10001,   25000, {0x00000003,0x00000000,0,0}}, /*0-1*/
    { 8,  10001,   25000, {0x00000300,0x00000000,0,0}}, /*8-9*/
    {16,  10001,   25000, {0x00030000,0x00000000,0,0}}, /*16-17*/
    {24,  10001,   25000, {0x03000000,0x00000000,0,0}}, /*24-25*/
    {32,  10001,   25000, {0x00000000,0x00000003,0,0}}, /*32-33*/
    {40,  10001,   25000, {0x00000000,0x00000300,0,0}}, /*40-41*/
    {48,      0,   25000, {0x00000000,0x00030000,0,0}}, /*48-49*/
    {49,      0,   25000, {0x00000000,0x000C0000,0,0}}, /*50-51*/
    {50,      0,   25000, {0x00000000,0x00300000,0,0}}, /*52-53*/
    {51,      0,   25000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {52,      0,   25000, {0x00000000,0x03000000,0,0}}, /*56-57*/
    {53,      0,   25000, {0x00000000,0x0C000000,0,0}}, /*58-59*/
    {50,  25001,   50000, {0x00000000,0x00F00000,0,0}}, /*52-55*/
    {52,  25001,   50000, {0x00000000,0x0F000000,0,0}}, /*56-59*/
    {50,  50001,  110000, {0x00000000,0x0FF00000,0,0}}, /*52-59*/
    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

/* PCA SFF Fifo elements resources Phoenix default*/
static const GT_U32 arbiter_slot_to_resource_id_pca_sff_phoenix_default[] =
{
    /*0-15*/      0, 0, 0, 0,  1, 1, 1, 1,  2, 2, 2, 2,  3, 3, 3, 3,
    /*16-31*/     4, 4, 4, 4,  5, 5, 5, 5,  6, 6, 6, 6,  7, 7, 7, 7,
    /*32-47*/     8, 8, 8, 8,  9, 9, 9, 9, 10,10,10,10, 11,11,11,11,
    /*48-63*/    12,12,12,12, 13,13,13,13, 14,14,14,14, 15,15,15,15,
    /*64-79*/    16,16,16,16, 17,17,17,17, 18,18,18,18, 19,19,19,19,
    /*80-95*/    20,20,20,20, 21,21,21,21, 22,22,22,22, 23,23,23,23,
    /*96-111*/   24,24,24,24, 25,25,25,25, 26,26,26,26, 27,27,27,27,
    /*112-127*/  28,28,28,28, 29,29,29,29, 30,30,30,30, 31,31,31,31,
    /*128-143*/  32,32,32,32, 33,33,33,33, 34,34,34,34, 35,35,35,35,
    /*144-150*/  36,36,36,36, 37,37,37,37, 38,38,38,38, 39,39,39,39,
    /*160-175*/  40,40,40,40, 41,41,41,41, 42,42,42,42, 43,43,43,43,
    /*176-191*/  44,44,44,44, 45,45,45,45, 46,46,46,46, 47,47,47,47,
    /*192-207*/  48,48,48,48, 49,49,49,49, 50,50,50,50, 51,51,51,51,
    /*208-223*/  52,52,52,52, 53,53,53,53, 54,54,54,54, 55,55,55,55,
    /*224-239*/  56,56,56,56, 57,57,57,57, 58,58,58,58, 59,59,59,59
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_dma_phoenix_default =
{
    /** number of slots in arbiter */
    100,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    56,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_phoenix_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_fifo_phoenix_default =
{
    /** number of slots in arbiter */
    100,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    56,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_phoenix_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_fifo_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pb_gpc_phoenix_default =
{
    /** number of slots in arbiter */
    116,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    57,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_pb_gpc_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_phoenix_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_phoenix_default =
{
    /** number of slots in arbiter */
    96,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    14,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    90,
    /** number of channels in unit */
    54,
    /** default number of slot in disabled arbiter slot register in unit */
    0x3F,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_arbiter_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_phoenix_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_rx_phoenix_default =
{
    /** number of slots in arbiter */
    96,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    54,
    /** default number of slot in disabled arbiter slot register in unit */
    0x3F,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_arbiter_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_phoenix_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_sff_phoenix_default =
{
    /** number of slots in arbiter */
    240,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    54,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_sff_phoenix_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_sff_phoenix_default,
    /** speedId to Profile Table */
    NULL
};


/* Maximal num of slots - if resourced mapped to channel have more - not all used.  */
/* Additional limitation of amount of slots or LL elewments needed for cases when   */
/* the same resources allocated to different speeds of the same channel and desired */
/* to use part of slots and leave the rest unused for slow speeds.                  */

/* LLs 0-10G:4, 10G+-25G:8, 25G+-50G:16, 50G+-110G:32, 110+ and more 63 */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC hawk_sff_max_num_of_slots[] =
{
    {{0x04000000, 0x00000000},     0,   8}, /* channel 26 always has 8 LLs */
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   4},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 20000,   8},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 40000,  16},
    {{0xFFFFFFFF, 0xFFFFFFFF},100000,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF},200000,  63},
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC phoenix_sff_max_num_of_slots[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   4},
    {{0xFFFFFFFF, 0xFFFFFFFF},  5000,   6},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 10000,   8},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 40000,  16},
    {{0xFFFFFFFF, 0xFFFFFFFF},100000,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF},200000,  63},
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC harrier_sff_max_num_of_slots[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   8},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 25001,  16},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 50001,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF},110001,  63},
    {{0, 0},NOT_VALID_CNS, 0}
};

/* Common HARRIER Channel+Speed => resources table */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_harrier_default[] =
{
    { 0, 214001, 428000, {0xFFFFFFFF,0,0,0}}, /*0-32*/
    { 0, 107001, 214000, {0x0000FFFF,0,0,0}}, /*0-15*/
    { 8, 107001, 214000, {0xFFFF0000,0,0,0}}, /*16-32*/
    { 0,  50001, 107000, {0x000000FF,0,0,0}}, /*0-7*/
    { 1,  50001, 107000, {0x0000FF00,0,0,0}}, /*8-15*/
    { 4,  50001, 107000, {0x0000FF00,0,0,0}}, /*8-15*/
    { 8,  50001, 107000, {0x00FF0000,0,0,0}}, /*16-23*/
    { 9,  50001, 107000, {0xFF000000,0,0,0}}, /*24-31*/
    {12,  50001, 107000, {0xFF000000,0,0,0}}, /*24-31*/
    { 0,  25001,  50000, {0x0000000F,0,0,0}}, /*0-3*/
    { 1,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 2,  25001,  50000, {0x000000F0,0,0,0}}, /*4-7*/
    { 4,  25001,  50000, {0x00000F00,0,0,0}}, /*8-13*/
    { 5,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    { 6,  25001,  50000, {0x0000F000,0,0,0}}, /*12-15*/
    { 8,  25001,  50000, {0x000F0000,0,0,0}}, /*16-19*/
    { 9,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {10,  25001,  50000, {0x00F00000,0,0,0}}, /*20-23*/
    {12,  25001,  50000, {0x0F000000,0,0,0}}, /*24-27*/
    {13,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    {14,  25001,  50000, {0xF0000000,0,0,0}}, /*28-31*/
    { 0,   5001,  25000, {0x00000003,0,0,0}}, /*0-1*/
    { 1,   5001,  25000, {0x0000000C,0,0,0}}, /*2-3*/
    { 2,   5001,  25000, {0x00000030,0,0,0}}, /*4-5*/
    { 3,   5001,  25000, {0x000000C0,0,0,0}}, /*6-7*/
    { 4,   5001,  25000, {0x00000300,0,0,0}}, /*8-9*/
    { 5,   5001,  25000, {0x00000C00,0,0,0}}, /*10-11*/
    { 6,   5001,  25000, {0x00003000,0,0,0}}, /*12-13*/
    { 7,   5001,  25000, {0x0000C000,0,0,0}}, /*14-15*/
    { 8,   5001,  25000, {0x00030000,0,0,0}}, /*16-17*/
    { 9,   5001,  25000, {0x000C0000,0,0,0}}, /*18-19*/
    {10,   5001,  25000, {0x00300000,0,0,0}}, /*20-21*/
    {11,   5001,  25000, {0x00C00000,0,0,0}}, /*22-23*/
    {12,   5001,  25000, {0x03000000,0,0,0}}, /*24-25*/
    {13,   5001,  25000, {0x0C000000,0,0,0}}, /*26-27*/
    {14,   5001,  25000, {0x30000000,0,0,0}}, /*28-29*/
    {15,   5001,  25000, {0xC0000000,0,0,0}}, /*30-31*/
    { 0,      1,   5000, {0x00000001,0,0,0}}, /*0-0*/
    { 1,      1,   5000, {0x00000004,0,0,0}}, /*2-2*/
    { 2,      1,   5000, {0x00000010,0,0,0}}, /*4-4*/
    { 3,      1,   5000, {0x00000040,0,0,0}}, /*6-6*/
    { 4,      1,   5000, {0x00000100,0,0,0}}, /*8-8*/
    { 5,      1,   5000, {0x00000400,0,0,0}}, /*10-10*/
    { 6,      1,   5000, {0x00001000,0,0,0}}, /*12-12*/
    { 7,      1,   5000, {0x00004000,0,0,0}}, /*14-14*/
    { 8,      1,   5000, {0x00010000,0,0,0}}, /*16-16*/
    { 9,      1,   5000, {0x00040000,0,0,0}}, /*18-18*/
    {10,      1,   5000, {0x00100000,0,0,0}}, /*20-20*/
    {11,      1,   5000, {0x00400000,0,0,0}}, /*22-22*/
    {12,      1,   5000, {0x01000000,0,0,0}}, /*24-24*/
    {13,      1,   5000, {0x04000000,0,0,0}}, /*26-26*/
    {14,      1,   5000, {0x10000000,0,0,0}}, /*28-28*/
    {15,      1,   5000, {0x40000000,0,0,0}}, /*30-30*/
    {26,      1,  25000, {0x00000000,1,0,0}}, /*32*/
    {27,      1,  54000, {0x00000000,2,0,0}}, /*33*/
};

/* Harrier SFF port => resource ids table */
/* 17 resources 8 LLs each                */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_harrier_sff_default[] =
{
    { 0, 214001, 428000, {0x000000FF,0,0,0}}, /*0-7*/
    { 0, 107001, 214000, {0x000000FF,0,0,0}}, /*0-7*/
    { 8, 107001, 214000, {0x0000FF00,0,0,0}}, /*8-15*/
    { 0,  50001, 110000, {0x0000000F,0,0,0}}, /*0-3*/
    { 1,  50001, 110000, {0x000000F0,0,0,0}}, /*4-7*/
    { 4,  50001, 110000, {0x000000F0,0,0,0}}, /*4-7*/
    { 8,  50001, 110000, {0x00000F00,0,0,0}}, /*8-11*/
    { 9,  50001, 110000, {0x0000F000,0,0,0}}, /*12-15*/
    {12,  50001, 110000, {0x0000F000,0,0,0}}, /*12-15*/
    { 0,  25001,  50000, {0x00000003,0,0,0}}, /*0-1*/
    { 1,  25001,  50000, {0x0000000C,0,0,0}}, /*2-3*/
    { 2,  25001,  50000, {0x0000000C,0,0,0}}, /*2-3*/
    { 4,  25001,  50000, {0x00000030,0,0,0}}, /*4-5*/
    { 5,  25001,  50000, {0x000000C0,0,0,0}}, /*6-7*/
    { 6,  25001,  50000, {0x000000C0,0,0,0}}, /*6-7*/
    { 8,  25001,  50000, {0x00000300,0,0,0}}, /*8-9*/
    { 9,  25001,  50000, {0x00000C00,0,0,0}}, /*10-11*/
    {10,  25001,  50000, {0x00000C00,0,0,0}}, /*10-11*/
    {12,  25001,  50000, {0x00003000,0,0,0}}, /*12-13*/
    {13,  25001,  50000, {0x0000C000,0,0,0}}, /*14-15*/
    {14,  25001,  50000, {0x0000C000,0,0,0}}, /*14-15*/
    { 0,      1,  25000, {0x00000001,0,0,0}}, /*0*/
    { 1,      1,  25000, {0x00000002,0,0,0}}, /*1*/
    { 2,      1,  25000, {0x00000004,0,0,0}}, /*2*/
    { 3,      1,  25000, {0x00000008,0,0,0}}, /*3*/
    { 4,      1,  25000, {0x00000010,0,0,0}}, /*4*/
    { 5,      1,  25000, {0x00000020,0,0,0}}, /*5*/
    { 6,      1,  25000, {0x00000040,0,0,0}}, /*6*/
    { 7,      1,  25000, {0x00000080,0,0,0}}, /*7*/
    { 8,      1,  25000, {0x00000100,0,0,0}}, /*8*/
    { 9,      1,  25000, {0x00000200,0,0,0}}, /*9*/
    {10,      1,  25000, {0x00000400,0,0,0}}, /*10*/
    {11,      1,  25000, {0x00000800,0,0,0}}, /*11*/
    {12,      1,  25000, {0x00001000,0,0,0}}, /*12*/
    {13,      1,  25000, {0x00002000,0,0,0}}, /*13*/
    {14,      1,  25000, {0x00004000,0,0,0}}, /*14*/
    {15,      1,  25000, {0x00008000,0,0,0}}, /*15*/
    {26,      1,  25000, {0x00010000,0,0,0}}, /*16*/
};

/* PCA SFF Fifo elements resources Phoenix default*/
static const GT_U32 arbiter_slot_to_resource_id_pca_sff_harrier_default[] =
{
    /*0-15*/     0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1,
    /*16-31*/    2, 2, 2, 2,  2, 2, 2, 2,  3, 3, 3, 3,  3, 3, 3, 3,
    /*32-47*/    4, 4, 4, 4,  4, 4, 4, 4,  5, 5, 5, 5,  5, 5, 5, 5,
    /*48-63*/    6, 6, 6, 6,  6, 6, 6, 6,  7, 7, 7, 7,  7, 7, 7, 7,
    /*64-79*/    8, 8, 8, 8,  8, 8, 8, 8,  9, 9, 9, 9,  9, 9, 9, 9,
    /*80-95*/   10,10,10,10, 10,10,10,10, 11,11,11,11, 11,11,11,11,
    /*96-111*/  12,12,12,12, 12,12,12,12, 13,13,13,13, 13,13,13,13,
    /*112-127*/ 14,14,14,14, 14,14,14,14, 15,15,15,15, 15,15,15,15,
    /*128-135*/ 16,16,16,16, 16,16,16,16
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_dma_harrier_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_fifo_harrier_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter */
    NOT_VALID_CNS,
    /** number of channels in unit */
    27,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_fifo_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pb_gpc_harrier_default =
{
    /** number of slots in arbiter */
    37,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter */
    NOT_VALID_CNS,
    /** number of channels in unit */
    28,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_pb_gpc_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_harrier_dp01_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    11,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    24,
    /** number of channels in unit */
    16,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_harrier_dp2_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    1,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    16,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_rx_harrier_default =
{
    /** number of slots in arbiter */
    33,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    16,
    /** default number of slot in disabled arbiter slot register in unit */
    0x1F,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_hawk_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_sff_harrier_default =
{
    /** number of slots in arbiter */
    136,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    16,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_sff_harrier_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_harrier_sff_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC ironman_sff_max_num_of_slots[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   4},
    {{0xFFFFFFFF, 0xFFFFFFFF},  5000,   6},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 10000,   8},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 40000,  16},
    {{0xFFFFFFFF, 0xFFFFFFFF},100000,  32},
    {{0xFFFFFFFF, 0xFFFFFFFF},200000,  63},
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_tx_dma_ironman_default[] =
{
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC initPortTypeSpeeds_pca_sff_ironman_default[] =
{
    {NOT_VALID_CNS, PRV_CPSS_PORT_NOT_EXISTS_E, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_pb_gpc_ironman_default[] =
{
/*    {57,         CPSS_PORT_SPEED_50000_E},*/
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC sff_fifo_width_ironman[] =
{/*only groups of channels for any speed */
    {{0xFFFFFFFF, 0x01FFFFFF},     0, 0}, /* channels 0-54  Depth-8B*/
    {{0, 0},NOT_VALID_CNS, 0}
};


static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC sff_max_of_tokens_ironman[] =
{/*only groups of channels for any speed */
    {{0x01010101, 0x00000101},     0, 176},/* channels 0,8,16,24,32,40  value 176 */
    {{0xFEFEFEFE, 0x0000FEFE},     0, 112},/* channels 1-7,9-15,17-23,25-31,33-39,41-47  value 112 */
    {{0x00000000, 0x00030000},     0, 176},/* channels 48-49  value 176 */
    {{0x00000000, 0x00040000},     0, 128},/* channels 50  value 128 */
    {{0x00000000, 0x00100000},     0,  80},/* channels 52  value 80 */
    {{0x00000000, 0x00280000},     0,  72},/* channels 51,53  value 72 */
    {{0, 0},NOT_VALID_CNS, 0},
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC brg_tx_credits_alloc_ironman[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},      0,  9}, /* 0 - 5G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},   5000, 12}, /* 5G - 10G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  10000, 15}, /* 10G - 20G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  20000, 22}, /* 20G - 40G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF},  40000, 44}, /* 40G - 100G-1 */
    {{0xFFFFFFFF, 0xFFFFFFFF}, 100000, 88}, /* 100G and more */
    {{0, 0},NOT_VALID_CNS, 0}
};

/* PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E */

static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_ironman_default[] =
{
    { 0,      0,    2500, {0x00000001,0x00000000,0,0}}, /*0*/
    { 1,      0,    2500, {0x00000002,0x00000000,0,0}}, /*1*/
    { 2,      0,    2500, {0x00000004,0x00000000,0,0}}, /*2*/
    { 3,      0,    2500, {0x00000008,0x00000000,0,0}}, /*3*/
    { 4,      0,    2500, {0x00000010,0x00000000,0,0}}, /*4*/
    { 5,      0,    2500, {0x00000020,0x00000000,0,0}}, /*5*/
    { 6,      0,    2500, {0x00000040,0x00000000,0,0}}, /*6*/
    { 7,      0,    2500, {0x00000080,0x00000000,0,0}}, /*7*/
    { 8,      0,    2500, {0x00000100,0x00000000,0,0}}, /*8*/
    { 9,      0,    2500, {0x00000200,0x00000000,0,0}}, /*9*/
    {10,      0,    2500, {0x00000400,0x00000000,0,0}}, /*10*/
    {11,      0,    2500, {0x00000800,0x00000000,0,0}}, /*11*/
    {12,      0,    2500, {0x00001000,0x00000000,0,0}}, /*12*/
    {13,      0,    2500, {0x00002000,0x00000000,0,0}}, /*13*/
    {14,      0,    2500, {0x00004000,0x00000000,0,0}}, /*14*/
    {15,      0,    2500, {0x00008000,0x00000000,0,0}}, /*15*/
    {16,      0,    2500, {0x00010000,0x00000000,0,0}}, /*16*/
    {17,      0,    2500, {0x00020000,0x00000000,0,0}}, /*17*/
    {18,      0,    2500, {0x00040000,0x00000000,0,0}}, /*18*/
    {19,      0,    2500, {0x00080000,0x00000000,0,0}}, /*19*/
    {20,      0,    2500, {0x00100000,0x00000000,0,0}}, /*20*/
    {21,      0,    2500, {0x00200000,0x00000000,0,0}}, /*21*/
    {22,      0,    2500, {0x00400000,0x00000000,0,0}}, /*22*/
    {23,      0,    2500, {0x00800000,0x00000000,0,0}}, /*23*/
    {24,      0,    2500, {0x01000000,0x00000000,0,0}}, /*24*/
    {25,      0,    2500, {0x02000000,0x00000000,0,0}}, /*25*/
    {26,      0,    2500, {0x04000000,0x00000000,0,0}}, /*26*/
    {27,      0,    2500, {0x08000000,0x00000000,0,0}}, /*27*/
    {28,      0,    2500, {0x10000000,0x00000000,0,0}}, /*28*/
    {29,      0,    2500, {0x20000000,0x00000000,0,0}}, /*29*/
    {30,      0,    2500, {0x40000000,0x00000000,0,0}}, /*30*/
    {31,      0,    2500, {0x80000000,0x00000000,0,0}}, /*31*/
    {32,      0,    2500, {0x00000000,0x00000001,0,0}}, /*32*/
    {33,      0,    2500, {0x00000000,0x00000002,0,0}}, /*33*/
    {34,      0,    2500, {0x00000000,0x00000004,0,0}}, /*34*/
    {35,      0,    2500, {0x00000000,0x00000008,0,0}}, /*35*/
    {36,      0,    2500, {0x00000000,0x00000010,0,0}}, /*36*/
    {37,      0,    2500, {0x00000000,0x00000020,0,0}}, /*37*/
    {38,      0,    2500, {0x00000000,0x00000040,0,0}}, /*38*/
    {39,      0,    2500, {0x00000000,0x00000080,0,0}}, /*39*/
    {40,      0,    2500, {0x00000000,0x00000100,0,0}}, /*40*/
    {41,      0,    2500, {0x00000000,0x00000200,0,0}}, /*41*/
    {42,      0,    2500, {0x00000000,0x00000400,0,0}}, /*42*/
    {43,      0,    2500, {0x00000000,0x00000800,0,0}}, /*43*/
    {44,      0,    2500, {0x00000000,0x00001000,0,0}}, /*44*/
    {45,      0,    2500, {0x00000000,0x00002000,0,0}}, /*45*/
    {46,      0,    2500, {0x00000000,0x00004000,0,0}}, /*46*/
    {47,      0,    2500, {0x00000000,0x00008000,0,0}}, /*47*/
    { 0,   2501,    5000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   2501,    5000, {0x0000000C,0x00000000,0,0}}, /*2,3*/
    { 2,   2501,    5000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 3,   2501,    5000, {0x000000C0,0x00000000,0,0}}, /*6,7*/
    { 8,   2501,    5000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   2501,    5000, {0x00000C00,0x00000000,0,0}}, /*10,11*/
    {10,   2501,    5000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {11,   2501,    5000, {0x0000C000,0x00000000,0,0}}, /*14,15*/
    {16,   2501,    5000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   2501,    5000, {0x000C0000,0x00000000,0,0}}, /*18,19*/
    {18,   2501,    5000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {19,   2501,    5000, {0x00C00000,0x00000000,0,0}}, /*22,23*/
    {24,   2501,    5000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   2501,    5000, {0x0C000000,0x00000000,0,0}}, /*26,27*/
    {26,   2501,    5000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {27,   2501,    5000, {0xC0000000,0x00000000,0,0}}, /*30,31*/
    {32,   2501,    5000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   2501,    5000, {0x00000000,0x0000000C,0,0}}, /*34,35,*/
    {34,   2501,    5000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {35,   2501,    5000, {0x00000000,0x000000C0,0,0}}, /*38,39*/
    {40,   2501,    5000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   2501,    5000, {0x00000000,0x00000C00,0,0}}, /*42,43*/
    {42,   2501,    5000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    {43,   2501,    5000, {0x00000000,0x0000C000,0,0}}, /*46,47*/
    { 0,   5001,   10000, {0x0000000F,0x00000000,0,0}}, /*0-3*/
    { 1,   5001,   10000, {0x000000F0,0x00000000,0,0}}, /*4-7*/
    { 8,   5001,   10000, {0x00000F00,0x00000000,0,0}}, /*8-11*/
    { 9,   5001,   10000, {0x0000F000,0x00000000,0,0}}, /*12-15*/
    {16,   5001,   10000, {0x000F0000,0x00000000,0,0}}, /*16-19*/
    {17,   5001,   10000, {0x00F00000,0x00000000,0,0}}, /*20-23*/
    {24,   5001,   10000, {0x0F000000,0x00000000,0,0}}, /*24-27*/
    {25,   5001,   10000, {0xF0000000,0x00000000,0,0}}, /*28-31*/
    {32,   5001,   10000, {0x00000000,0x0000000F,0,0}}, /*32-35*/
    {33,   5001,   10000, {0x00000000,0x000000F0,0,0}}, /*36-39*/
    {40,   5001,   10000, {0x00000000,0x00000F00,0,0}}, /*40-43*/
    {41,   5001,   10000, {0x00000000,0x0000F000,0,0}}, /*44-47*/
    { 0,  10001,   25000, {0x000000FF,0x00000000,0,0}}, /*0-7*/
    { 8,  10001,   25000, {0x0000FF00,0x00000000,0,0}}, /*8-15*/
    {16,  10001,   25000, {0x00FF0000,0x00000000,0,0}}, /*16-23*/
    {24,  10001,   25000, {0xFF000000,0x00000000,0,0}}, /*24-31*/
    {32,  10001,   25000, {0x00000000,0x000000FF,0,0}}, /*32-39*/
    {40,  10001,   25000, {0x00000000,0x0000FF00,0,0}}, /*40-47*/
    {48,      0,   25000, {0x00000000,0x00010000,0,0}}, /*48*/
    {49,      0,   25000, {0x00000000,0x00020000,0,0}}, /*49*/
    {50,      0,   25000, {0x00000000,0x00040000,0,0}}, /*50*/
    {51,      0,   25000, {0x00000000,0x00080000,0,0}}, /*51*/
    {52,      0,   25000, {0x00000000,0x00100000,0,0}}, /*52*/
    {53,      0,   25000, {0x00000000,0x00200000,0,0}}, /*53*/
    {50,  25001,   50000, {0x00000000,0x000C0000,0,0}}, /*50-51*/
    {52,  25001,   50000, {0x00000000,0x00300000,0,0}}, /*52-53*/
    {50,  50001,  100000, {0x00000000,0x003C0000,0,0}}, /*50-53*/
    {54,      0,    5000, {0x00000000,0x00400000,0,0}}, /*54*/
    {55,      0,    5000, {0x00000000,0x00800000,0,0}}, /*55*/
    {54,   5001,   10000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {55,   5001,   10000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {56,      0,   10000, {0x00000000,0x01000000,0,0}}, /*56*/
    {57,      0,   50000, {0x00000000,0x01000000,0,0}}, /*57*/

    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

static const GT_U32 arbiter_slot_to_resource_id_tx_dma_ironman_default[] =
{
    /*0-11*/    0, 32, 50, 16, 48, 52,  8, 40, 51, 24, 49, 53,
    /*12-24*/   4, 36, 50, 20, 48, 52, 12, 44, 51, 28, 49, 53, 54,
    /*25-36*/   2, 34, 50, 18, 48, 52, 10, 42, 51, 26, 49, 53,
    /*37-49*/   6, 38, 50, 22, 48, 52, 14, 46, 51, 30, 49, 53, 55,
    /*50-61*/   1, 33, 50, 17, 48, 52,  9, 41, 51, 25, 49, 53,
    /*62-74*/   5, 37, 50, 21, 48, 52, 13, 45, 51, 29, 49, 53, 54,
    /*75-86*/   3, 35, 50, 19, 48, 52, 11, 43, 51, 27, 49, 53,
    /*87-99*/   7, 39, 50, 23, 48, 52, 15, 47, 51, 31, 49, 53, 55
};

static const GT_U32 arbiter_slot_to_resource_id_pb_gpc_ironman_default[] =
{
    /*0-13*/     0, 32, 50, 56, 16, 48, 52,  8, 40, 51, 56, 24, 49, 53,
    /*14-28*/    4, 36, 50, 56, 20, 48, 52, 12, 44, 51, 56, 28, 49, 53, 54,
    /*29-42*/    2, 34, 50, 56, 18, 48, 52, 10, 42, 51, 56, 26, 49, 53,
    /*43-57*/    6, 38, 50, 56, 22, 48, 52, 14, 46, 51, 56, 30, 49, 53, 55,
    /*58-71*/    1, 33, 50, 56, 17, 48, 52,  9, 41, 51, 56, 25, 49, 53,
    /*72-86*/    5, 37, 50, 56, 21, 48, 52, 13, 45, 51, 56, 29, 49, 53, 54,
    /*87-100*/   3, 35, 50, 56, 19, 48, 52, 11, 43, 51, 56, 27, 49, 53,
    /*101-115*/  7, 39, 50, 56, 23, 48, 52, 15, 47, 51, 56, 31, 49, 53, 55
};

static const GT_U32 arbiter_slot_to_resource_id_pca_arbiter_ironman_default[] =
{
    /*0-11*/    0, 32, 50, 16, 48, 52,  8, 40, 51, 24, 49, 53,
    /*12-23*/   4, 36, 50, 20, 48, 52, 12, 44, 51, 28, 49, 53,
    /*24-35*/   2, 34, 50, 18, 48, 52, 10, 42, 51, 26, 49, 53,
    /*36-47*/   6, 38, 50, 22, 48, 52, 14, 46, 51, 30, 49, 53,
    /*48-59*/   1, 33, 50, 17, 48, 52,  9, 41, 51, 25, 49, 53,
    /*60-71*/   5, 37, 50, 21, 48, 52, 13, 45, 51, 29, 49, 53,
    /*72-83*/   3, 35, 50, 19, 48, 52, 11, 43, 51, 27, 49, 53,
    /*84-95*/   7, 39, 50, 23, 48, 52, 15, 47, 51, 31, 49, 53
};

/* only for ironman SFF rings to match the SFF designers tests      */
/* resource here is just 4 LLs: resource-N id LLs (4*N) - ((4*N)+3) */
/* LLs ring sizes will also be reduced accorging to limits table    */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC channel_resources_sff_ironman_default[] =
{
    { 0,      0,    2500, {0x00000001,0x00000000,0,0}}, /*0*/
    { 1,      0,    2500, {0x00000002,0x00000000,0,0}}, /*1*/
    { 2,      0,    2500, {0x00000004,0x00000000,0,0}}, /*2*/
    { 3,      0,    2500, {0x00000008,0x00000000,0,0}}, /*3*/
    { 4,      0,    2500, {0x00000010,0x00000000,0,0}}, /*4*/
    { 5,      0,    2500, {0x00000020,0x00000000,0,0}}, /*5*/
    { 6,      0,    2500, {0x00000040,0x00000000,0,0}}, /*6*/
    { 7,      0,    2500, {0x00000080,0x00000000,0,0}}, /*7*/
    { 8,      0,    2500, {0x00000100,0x00000000,0,0}}, /*8*/
    { 9,      0,    2500, {0x00000200,0x00000000,0,0}}, /*9*/
    {10,      0,    2500, {0x00000400,0x00000000,0,0}}, /*10*/
    {11,      0,    2500, {0x00000800,0x00000000,0,0}}, /*11*/
    {12,      0,    2500, {0x00001000,0x00000000,0,0}}, /*12*/
    {13,      0,    2500, {0x00002000,0x00000000,0,0}}, /*13*/
    {14,      0,    2500, {0x00004000,0x00000000,0,0}}, /*14*/
    {15,      0,    2500, {0x00008000,0x00000000,0,0}}, /*15*/
    {16,      0,    2500, {0x00010000,0x00000000,0,0}}, /*16*/
    {17,      0,    2500, {0x00020000,0x00000000,0,0}}, /*17*/
    {18,      0,    2500, {0x00040000,0x00000000,0,0}}, /*18*/
    {19,      0,    2500, {0x00080000,0x00000000,0,0}}, /*19*/
    {20,      0,    2500, {0x00100000,0x00000000,0,0}}, /*20*/
    {21,      0,    2500, {0x00200000,0x00000000,0,0}}, /*21*/
    {22,      0,    2500, {0x00400000,0x00000000,0,0}}, /*22*/
    {23,      0,    2500, {0x00800000,0x00000000,0,0}}, /*23*/
    {24,      0,    2500, {0x01000000,0x00000000,0,0}}, /*24*/
    {25,      0,    2500, {0x02000000,0x00000000,0,0}}, /*25*/
    {26,      0,    2500, {0x04000000,0x00000000,0,0}}, /*26*/
    {27,      0,    2500, {0x08000000,0x00000000,0,0}}, /*27*/
    {28,      0,    2500, {0x10000000,0x00000000,0,0}}, /*28*/
    {29,      0,    2500, {0x20000000,0x00000000,0,0}}, /*29*/
    {30,      0,    2500, {0x40000000,0x00000000,0,0}}, /*30*/
    {31,      0,    2500, {0x80000000,0x00000000,0,0}}, /*31*/
    {32,      0,    2500, {0x00000000,0x00000001,0,0}}, /*32*/
    {33,      0,    2500, {0x00000000,0x00000002,0,0}}, /*33*/
    {34,      0,    2500, {0x00000000,0x00000004,0,0}}, /*34*/
    {35,      0,    2500, {0x00000000,0x00000008,0,0}}, /*35*/
    {36,      0,    2500, {0x00000000,0x00000010,0,0}}, /*36*/
    {37,      0,    2500, {0x00000000,0x00000020,0,0}}, /*37*/
    {38,      0,    2500, {0x00000000,0x00000040,0,0}}, /*38*/
    {39,      0,    2500, {0x00000000,0x00000080,0,0}}, /*39*/
    {40,      0,    2500, {0x00000000,0x00000100,0,0}}, /*40*/
    {41,      0,    2500, {0x00000000,0x00000200,0,0}}, /*41*/
    {42,      0,    2500, {0x00000000,0x00000400,0,0}}, /*42*/
    {43,      0,    2500, {0x00000000,0x00000800,0,0}}, /*43*/
    {44,      0,    2500, {0x00000000,0x00001000,0,0}}, /*44*/
    {45,      0,    2500, {0x00000000,0x00002000,0,0}}, /*45*/
    {46,      0,    2500, {0x00000000,0x00004000,0,0}}, /*46*/
    {47,      0,    2500, {0x00000000,0x00008000,0,0}}, /*47*/
    { 0,   2501,    5000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   2501,    5000, {0x0000000C,0x00000000,0,0}}, /*2,3*/
    { 2,   2501,    5000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 3,   2501,    5000, {0x000000C0,0x00000000,0,0}}, /*6,7*/
    { 8,   2501,    5000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   2501,    5000, {0x00000C00,0x00000000,0,0}}, /*10,11*/
    {10,   2501,    5000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {11,   2501,    5000, {0x0000C000,0x00000000,0,0}}, /*14,15*/
    {16,   2501,    5000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   2501,    5000, {0x000C0000,0x00000000,0,0}}, /*18,19*/
    {18,   2501,    5000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {19,   2501,    5000, {0x00C00000,0x00000000,0,0}}, /*22,23*/
    {24,   2501,    5000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   2501,    5000, {0x0C000000,0x00000000,0,0}}, /*26,27*/
    {26,   2501,    5000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {27,   2501,    5000, {0xC0000000,0x00000000,0,0}}, /*30,31*/
    {32,   2501,    5000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   2501,    5000, {0x00000000,0x0000000C,0,0}}, /*34,35,*/
    {34,   2501,    5000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {35,   2501,    5000, {0x00000000,0x000000C0,0,0}}, /*38,39*/
    {40,   2501,    5000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   2501,    5000, {0x00000000,0x00000C00,0,0}}, /*42,43*/
    {42,   2501,    5000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    {43,   2501,    5000, {0x00000000,0x0000C000,0,0}}, /*46,47*/
    { 0,   5001,   10000, {0x00000003,0x00000000,0,0}}, /*0,1*/
    { 1,   5001,   10000, {0x00000030,0x00000000,0,0}}, /*4,5*/
    { 8,   5001,   10000, {0x00000300,0x00000000,0,0}}, /*8,9*/
    { 9,   5001,   10000, {0x00003000,0x00000000,0,0}}, /*12,13*/
    {16,   5001,   10000, {0x00030000,0x00000000,0,0}}, /*16,17*/
    {17,   5001,   10000, {0x00300000,0x00000000,0,0}}, /*20,21*/
    {24,   5001,   10000, {0x03000000,0x00000000,0,0}}, /*24,25*/
    {25,   5001,   10000, {0x30000000,0x00000000,0,0}}, /*28,29*/
    {32,   5001,   10000, {0x00000000,0x00000003,0,0}}, /*32,33*/
    {33,   5001,   10000, {0x00000000,0x00000030,0,0}}, /*36,37*/
    {40,   5001,   10000, {0x00000000,0x00000300,0,0}}, /*40,41*/
    {41,   5001,   10000, {0x00000000,0x00003000,0,0}}, /*44,45*/
    { 0,  10001,   25000, {0x00000003,0x00000000,0,0}}, /*0-1*/
    { 8,  10001,   25000, {0x00000300,0x00000000,0,0}}, /*8-9*/
    {16,  10001,   25000, {0x00030000,0x00000000,0,0}}, /*16-17*/
    {24,  10001,   25000, {0x03000000,0x00000000,0,0}}, /*24-25*/
    {32,  10001,   25000, {0x00000000,0x00000003,0,0}}, /*32-33*/
    {40,  10001,   25000, {0x00000000,0x00000300,0,0}}, /*40-41*/
    {48,      0,   25000, {0x00000000,0x00030000,0,0}}, /*48-49*/
    {49,      0,   25000, {0x00000000,0x000C0000,0,0}}, /*50-51*/
    {50,      0,   25000, {0x00000000,0x00300000,0,0}}, /*52-53*/
    {51,      0,   25000, {0x00000000,0x00C00000,0,0}}, /*54-55*/
    {52,      0,   25000, {0x00000000,0x03000000,0,0}}, /*56-57*/
    {53,      0,   25000, {0x00000000,0x0C000000,0,0}}, /*58-59*/
    {50,  25001,   50000, {0x00000000,0x00F00000,0,0}}, /*52-55*/
    {52,  25001,   50000, {0x00000000,0x0F000000,0,0}}, /*56-59*/
    {50,  50001,  110000, {0x00000000,0x0FF00000,0,0}}, /*52-59*/
    { NOT_VALID_CNS, 0, 0, {0,0,0,0}} /* end of table*/
};

/* PCA SFF Fifo elements resources ironman default*/
static const GT_U32 arbiter_slot_to_resource_id_pca_sff_ironman_default[] =
{
    /*0-15*/      0, 0, 0, 0,  1, 1, 1, 1,  2, 2, 2, 2,  3, 3, 3, 3,
    /*16-31*/     4, 4, 4, 4,  5, 5, 5, 5,  6, 6, 6, 6,  7, 7, 7, 7,
    /*32-47*/     8, 8, 8, 8,  9, 9, 9, 9, 10,10,10,10, 11,11,11,11,
    /*48-63*/    12,12,12,12, 13,13,13,13, 14,14,14,14, 15,15,15,15,
    /*64-79*/    16,16,16,16, 17,17,17,17, 18,18,18,18, 19,19,19,19,
    /*80-95*/    20,20,20,20, 21,21,21,21, 22,22,22,22, 23,23,23,23,
    /*96-111*/   24,24,24,24, 25,25,25,25, 26,26,26,26, 27,27,27,27,
    /*112-127*/  28,28,28,28, 29,29,29,29, 30,30,30,30, 31,31,31,31,
    /*128-143*/  32,32,32,32, 33,33,33,33, 34,34,34,34, 35,35,35,35,
    /*144-150*/  36,36,36,36, 37,37,37,37, 38,38,38,38, 39,39,39,39,
    /*160-175*/  40,40,40,40, 41,41,41,41, 42,42,42,42, 43,43,43,43,
    /*176-191*/  44,44,44,44, 45,45,45,45, 46,46,46,46, 47,47,47,47,
    /*192-207*/  48,48,48,48, 49,49,49,49, 50,50,50,50, 51,51,51,51,
    /*208-223*/  52,52,52,52, 53,53,53,53, 54,54,54,54, 55,55,55,55,
    /*224-239*/  56,56,56,56, 57,57,57,57, 58,58,58,58, 59,59,59,59
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_dma_ironman_default =
{
    /** number of slots in arbiter */
    100,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    57,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_ironman_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_tx_fifo_ironman_default =
{
    /** number of slots in arbiter */
    100,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    57,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_tx_dma_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_ironman_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_fifo_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pb_gpc_ironman_default =
{
    /** number of slots in arbiter */
    116,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    58,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the slot per slot */
    arbiter_slot_to_resource_id_pb_gpc_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_ironman_default,
    /** speedId to Profile Table - same as Hawk */
    speedId_to_profile_tx_dma_hawk_default
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_tx_ironman_default =
{
    /** number of slots in arbiter */
    96,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    14,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    90,
    /** number of channels in unit */
    55,
    /** default number of slot in disabled arbiter slot register in unit */
    0x3F,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_arbiter_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_ironman_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_pca_arbiter_rx_ironman_default =
{
    /** number of slots in arbiter */
    96,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    55,
    /** default number of slot in disabled arbiter slot register in unit */
    0x3F,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_arbiter_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_ironman_default,
    /** speedId to Profile Table */
    NULL
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC pizzaPattern_sff_ironman_default =
{
    /** number of slots in arbiter */
    240,
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    NOT_VALID_CNS,
    /** Pizza offset for PCA arbiter  for active MACSEC */
    NOT_VALID_CNS,
    /** number of channels in unit */
    55,
    /** default number of slot in disabled arbiter slot register in unit */
    0,
    /** table of ID of resource containing the fifo-element per fifo-element */
    arbiter_slot_to_resource_id_pca_sff_ironman_default,
    /** table of resources per all supported pairs channel and speed */
    channel_resources_sff_ironman_default,
    /** speedId to Profile Table */
    NULL
};


/**
* @internal isNotSupportedUnit function
* @endinternal
*
* @brief    Checks that the unit is supported.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - device number.
* @param[in] unitType               - id of unit
*
* @retval Not supported unit - 1, supported unit - 0
*/
static GT_U32 isNotSupportedUnit
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
)
{
    if (PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* The GM not supports the DP related units except RX_DMA */
        switch (unitType)
        {
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                return 1;
            default: break;
        }
    }
    return 0;
}

/**
* @internal isPcaUnit function
* @endinternal
*
* @brief    Checks that the unit is PCA unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] unitType               - id of unit
*
* @retval PCA unit - 1, not PCA unit - 0
*/
static GT_U32 isPcaUnit
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
)
{
    switch (unitType)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:
            return 1;
        default: break;
    }
    return 0;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Phoenix.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @param[out] txDmaIndexPtr       - (pointer to)  Tx global
*       index (0..3)
* @param[out] channelIndexPtr     - (pointer to)  channel local
*      index in DP (0..26)
* @param[out] isPca               - (pointer to)  1 - PCA, 0 - SDMA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *txDmaIndexPtr,
    OUT GT_U32                       *channelIndexPtr,
    OUT GT_U32                       *isPca
)
{
    GT_STATUS rc;
    CPSS_DXCH_SHADOW_PORT_MAP_STC detailedPortMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapGet(
        devNum, portNum,/*OUT*/&detailedPortMap);
    if (GT_OK != rc )
    {
        return rc;
    }

    *isPca =
        (detailedPortMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            ? 0 : 1;

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
        devNum, detailedPortMap.txDmaNum, txDmaIndexPtr, channelIndexPtr);
    if (GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesUnitIdToName function
* @endinternal
*
* @brief    Convert unit id to unit printable name.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] unitType               - id of unit
*
* @retval unit name or UNKNOWN
*/
const char* prvCpssDxChTxPortSpeedPizzaResourcesUnitIdToName
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
)
{
    switch (unitType)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:          return "PCA_RX";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:          return "PCA_TX";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:         return "PCA_SFF";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:      return "PCA_BRG_RX";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:      return "PCA_BRG_TX";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:          return "TX_DMA";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:         return "TX_FIFO";
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E: return "PB_GPC_PKT_READ";
        default: return "UNKNOWN";
    }
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaProjectGet function
* @endinternal
*
* @brief    Get device projecty Id.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - physical device number
* @param[out] projectPtr           - (pointer to)project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaProjectGet
(
    IN  GT_U8                                          devNum,
    OUT  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT       *projectPtr
)
{
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        *projectPtr = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        *projectPtr = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E;
    }
    else
    if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        *projectPtr = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E;
    }
    else
    {
        *projectPtr = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet function
* @endinternal
*
* @brief    Get get the most exact value found by speed id.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] valueTablePtr          - table of pairs <speedIdBound, value>.
* @param[in] valueTableSize         - size of table
* @param[in] channelId              - channel Id for search
* @param[in] speedId                - speed Id for search
*
* @retval the most exact value found by speed id, NOT_VALID_CNS if not found
*/
GT_U32 prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet
(
    IN  const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC   *valueTablePtr,
    IN  const GT_U32                                            valueTableSize,
    IN  GT_U32                                                  channelId,
    IN  GT_U32                                                  speedId
)
{
    GT_U32 index;
    GT_U32 lastIndex;
    GT_U32 bmpSize;

    bmpSize = sizeof(valueTablePtr->channelBitmapArr)
        / sizeof(valueTablePtr->channelBitmapArr[0]);
    if ((channelId / 32) >= bmpSize)
    {
        return NOT_VALID_CNS;
    }

    lastIndex = NOT_VALID_CNS;
    for (index = 0;
          ((index < valueTableSize) && (valueTablePtr[index].speedIdLowest != NOT_VALID_CNS)); index++)
    {
        if ((valueTablePtr[index].channelBitmapArr[channelId / 32] & (1 << (channelId % 32))) == 0)
        {
            continue;
        }
        if (valueTablePtr[index].speedIdLowest > speedId) continue;
        if (valueTablePtr[index].speedIdLowest == speedId) return valueTablePtr[index].value;

        /* assumed valueTablePtr[index].speedIdLowest < speedId */
        /* save index of the nearest low bound */
        if (lastIndex == NOT_VALID_CNS)
        {
            lastIndex = index;
        }
        else if (valueTablePtr[lastIndex].speedIdLowest
                 < valueTablePtr[index].speedIdLowest)
        {
            lastIndex = index;
        }
    }
    return ((lastIndex == NOT_VALID_CNS)
                ? NOT_VALID_CNS /* not found - all the bounds greater than speedId */
                : valueTablePtr[lastIndex].value); /* the nearest lower bound pair */
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPatternGet function
* @endinternal
*
* @brief    Get pattern of pizza arbiter.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in]  unitIndex             - unit index
* @retval pointer to pizza arbiter pattern on success or NULL
*/
const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* prvCpssDxChTxPortSpeedPizzaResourcesPatternGet
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN  GT_U32                                    unitIndex
)
{
    /* parameter temporary reserved                                                      */
    /* will be needed again making difference between DPs having and not having CPU port */
    unitIndex = unitIndex;

    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_dma_hawk_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_fifo_hawk_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                    if (unitIndex < 2)
                    {
                        return
                            (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                                (&pizzaPattern_pca_arbiter_tx_hawk_dp01_default);
                    }
                    else
                    {
                        return
                            (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                                (&pizzaPattern_pca_arbiter_tx_hawk_dp23_default);
                    }
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_rx_hawk_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pb_gpc_hawk_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_sff_hawk_default);
                default: return NULL;
            }
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_dma_phoenix_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_fifo_phoenix_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_tx_phoenix_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_rx_phoenix_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pb_gpc_phoenix_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_sff_phoenix_default);
                default: return NULL;
            }
            break;

        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_dma_harrier_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_fifo_harrier_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                    if (unitIndex < 2)
                    {
                        return
                            (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                                (&pizzaPattern_pca_arbiter_tx_harrier_dp01_default);
                    }
                    else
                    {
                        return
                            (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                                (&pizzaPattern_pca_arbiter_tx_harrier_dp2_default);
                    }
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_rx_harrier_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pb_gpc_harrier_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_sff_harrier_default);
                default: return NULL;
            }
            break;

        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_dma_ironman_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_tx_fifo_ironman_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_tx_ironman_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pca_arbiter_rx_ironman_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_pb_gpc_ironman_default);
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* const)
                            (&pizzaPattern_sff_ironman_default);
                default: return NULL;
            }
            break;
        default: return NULL;
    }
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet function
* @endinternal
*
* @brief    Get bitmap of pizza arbiter slots for given Tx channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - unut index
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
* @param[in] slotBmpPtr             - (pointer to)bitmap should be cleared
* @param[out] slotBmpPtr            - (pointer to)bitmap of arbiter slots for channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    INOUT GT_U32                                    *slotBmpPtr
)
{
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    GT_U32 idx;
    GT_U32 speedId;
    const GT_U32 *resourcesBmpPtr;
    GT_U32 resourceId;
    GT_U32 slot;

    CPSS_NULL_PTR_CHECK_MAC(slotBmpPtr);

    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(project, unitType, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search resources bitmap */
    for (idx = 0; (1); idx++)
    {
        if (resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].channelId == NOT_VALID_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].channelId
            >= resourcePatternPtr->numOfChannels)
        {
            continue;
        }
        if ((resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].channelId == channel) &&
            (resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].speedIdLow <= speedId) &&
            (resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].speedIdHigh >= speedId))
        {
            resourcesBmpPtr = resourcePatternPtr->resourcesPerChannelSpeedPtr[idx].resourceIdsBmp;
            break;
        }
    }

    /* build slot bitmap */
    for (slot = 0; (slot < resourcePatternPtr->numOfSlots); slot++)
    {
        resourceId = resourcePatternPtr->resourceIdPerSlotPtr[slot];
        if (resourcesBmpPtr[resourceId / 32] & (1 << (resourceId % 32)))
        {
            slotBmpPtr[slot / 32] |= (1 << (slot % 32));
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet function
* @endinternal
*
* @brief    Get pizza arbiter HW info.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - physical device number
* @param[in]  unitType             - type of unit containig pizza arbiter
* @param[in]  unitIndex            - index of unit containig pizza arbiter
* @param[out] hwInfoPtr            - (pointer to) HW info of pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT           unitType,
    IN  GT_U32                                         unitIndex,
    OUT PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC        *hwInfoPtr
)
{
    GT_STATUS                           rc;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(hwInfoPtr);
    if (unitIndex >= MAX_PCA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* should never occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    cpssOsMemSet(hwInfoPtr, 0, sizeof(PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC));

    hwInfoPtr->ctrlRegPizzaOffsetFldLen   = NOT_VALID_CNS; /* default */
    hwInfoPtr->ctrlRegPizzaOffsetFldPos   = NOT_VALID_CNS; /* default */
    hwInfoPtr->channelSpeedProfileFldPos  = NOT_VALID_CNS;
    hwInfoPtr->channelSpeedProfileFldLen  = NOT_VALID_CNS;

    /* TX DMA and FIFO DP0 per port arrays uses CPU port place as  */
    /* origin of next unit channel addresses. The CPU port channel */
    /* accessable by global index only.                            */
    hwInfoPtr->cpuChannelLocalIndex  = NOT_VALID_CNS; /* default */
    hwInfoPtr->cpuChannelGlobalIndex = NOT_VALID_CNS; /* default */
    if (unitIndex == 0)
    {
        switch (unitType)
        {
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
            case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                hwInfoPtr->cpuChannelLocalIndex =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[unitIndex].cpuPortDmaNum;
                rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
                    devNum, unitIndex, hwInfoPtr->cpuChannelLocalIndex,
                    &(hwInfoPtr->cpuChannelGlobalIndex));
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
            default:
                break;
        }
    }

    switch (unitType)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
            hwInfoPtr->ctrlRegAddr          =
                regsAddrPtr->sip6_txDMA[unitIndex].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            hwInfoPtr->ctrlRegEnableBit     = 31;
            hwInfoPtr->ctrlRegSizeFldPos    =  0;
            hwInfoPtr->ctrlRegSizeFldLen    =  8;
            hwInfoPtr->slotRegAddrArrPtr    =
                &(regsAddrPtr->sip6_txDMA[unitIndex].configs.globalConfigs.pizzaArbiter.
                  pizzaArbiterConfigReg[0]);
            hwInfoPtr->slotRegValidBit      = 31;
            hwInfoPtr->slotRegChannelFldPos =  0;
            hwInfoPtr->slotRegChannelFldLen =  6;
            hwInfoPtr->channelSpeedProfileRegAddrArrPtr =
                &(regsAddrPtr->sip6_txDMA[unitIndex].configs.channelConfigs.speedProfile[0]);
            hwInfoPtr->channelSpeedProfileFldPos        = 0;
            hwInfoPtr->channelSpeedProfileFldLen        = 4;
            hwInfoPtr->channelResetRegAddrArrPtr        =
                &(regsAddrPtr->sip6_txDMA[unitIndex].configs.channelConfigs.channelReset[0]);
            hwInfoPtr->channelResetNumOfWrites          = 3;
            hwInfoPtr->channelResetBitsToWriteWrites    = 0x02;
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
            hwInfoPtr->ctrlRegAddr          =
                regsAddrPtr->sip6_txFIFO[unitIndex].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            hwInfoPtr->ctrlRegEnableBit     = 31;
            hwInfoPtr->ctrlRegSizeFldPos    =  0;
            hwInfoPtr->ctrlRegSizeFldLen    =  8;
            hwInfoPtr->slotRegAddrArrPtr    =
                &(regsAddrPtr->sip6_txFIFO[unitIndex].configs.globalConfigs.pizzaArbiter.
                  pizzaArbiterConfigReg[0]);
            hwInfoPtr->slotRegValidBit      = 31;
            hwInfoPtr->slotRegChannelFldPos =  0;
            hwInfoPtr->slotRegChannelFldLen =  6;
            hwInfoPtr->channelSpeedProfileRegAddrArrPtr =
                &(regsAddrPtr->sip6_txFIFO[unitIndex].configs.channelConfigs.speedProfile[0]);
            hwInfoPtr->channelSpeedProfileFldPos        = 0;
            hwInfoPtr->channelSpeedProfileFldLen        = 4;
            hwInfoPtr->channelResetRegAddrArrPtr        = NULL;
            hwInfoPtr->channelResetNumOfWrites          = 0;
            hwInfoPtr->channelResetBitsToWriteWrites    = 0;
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
            hwInfoPtr->ctrlRegAddr          =
                regsAddrPtr->PCA_PZ_ARBITER[unitIndex][0].control;
            hwInfoPtr->ctrlRegEnableBit     = 24;
            hwInfoPtr->ctrlRegSizeFldPos    =  0;
            hwInfoPtr->ctrlRegSizeFldLen    =  8;
            hwInfoPtr->slotRegAddrArrPtr    =
                &(regsAddrPtr->PCA_PZ_ARBITER[unitIndex][0].configuration[0]);
            hwInfoPtr->slotRegValidBit      = 28;
            hwInfoPtr->slotRegChannelFldPos =  0;
            hwInfoPtr->slotRegChannelFldLen =  6;
            hwInfoPtr->ctrlRegPizzaOffsetFldLen  = 8;
            hwInfoPtr->ctrlRegPizzaOffsetFldPos   = 9;
            hwInfoPtr->channelSpeedProfileRegAddrArrPtr = NULL;
            hwInfoPtr->channelResetRegAddrArrPtr        = NULL;
            hwInfoPtr->channelResetNumOfWrites          = 0;
            hwInfoPtr->channelResetBitsToWriteWrites    = 0;
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
            hwInfoPtr->ctrlRegAddr          =
                regsAddrPtr->PCA_PZ_ARBITER[unitIndex][1].control;
            hwInfoPtr->ctrlRegEnableBit     = 24;
            hwInfoPtr->ctrlRegSizeFldPos    =  0;
            hwInfoPtr->ctrlRegSizeFldLen    =  8;
            hwInfoPtr->slotRegAddrArrPtr    =
                &(regsAddrPtr->PCA_PZ_ARBITER[unitIndex][1].configuration[0]);
            hwInfoPtr->slotRegValidBit      = 28;
            hwInfoPtr->slotRegChannelFldPos =  0;
            hwInfoPtr->slotRegChannelFldLen =  6;
            hwInfoPtr->ctrlRegPizzaOffsetFldLen  = 8;
            hwInfoPtr->ctrlRegPizzaOffsetFldPos   = 9;
            hwInfoPtr->channelSpeedProfileRegAddrArrPtr = NULL;
            hwInfoPtr->channelResetRegAddrArrPtr        = NULL;
            hwInfoPtr->channelResetNumOfWrites          = 0;
            hwInfoPtr->channelResetBitsToWriteWrites    = 0;
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
            hwInfoPtr->ctrlRegAddr          =
                regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].arbiterCalendarMax;
            hwInfoPtr->ctrlRegEnableBit     = 31;
            hwInfoPtr->ctrlRegSizeFldPos    =  0;
            hwInfoPtr->ctrlRegSizeFldLen    =  8;
            hwInfoPtr->slotRegAddrArrPtr    =
                &(regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].arbiterCalendarSlot[0]);
            hwInfoPtr->slotRegValidBit      = 31;
            hwInfoPtr->slotRegChannelFldPos =  0;
            hwInfoPtr->slotRegChannelFldLen =  5;
            if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {/* Phoenix */
                hwInfoPtr->slotRegChannelFldLen =  6;
            }
            hwInfoPtr->channelSpeedProfileRegAddrArrPtr =
                &(regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].channelProfile[0]);
            hwInfoPtr->channelSpeedProfileFldPos        = 0;
            hwInfoPtr->channelSpeedProfileFldLen        = 4;
            hwInfoPtr->channelResetRegAddrArrPtr       =
                &(regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].channelReset[0]);
            hwInfoPtr->channelResetNumOfWrites         = 1;
            hwInfoPtr->channelResetBitsToWriteWrites   = 0x01;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/*{26,         CPSS_PORT_SPEED_10000_E}, CPU ports configured later */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_tx_dma_hawk_default[] =
{
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC initPortTypeSpeeds_pca_sff_hawk_default[] =
{
    {NOT_VALID_CNS, PRV_CPSS_PORT_NOT_EXISTS_E, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_pb_gpc_hawk_default[] =
{
    {27,         CPSS_PORT_SPEED_50000_E},
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_tx_dma_phoenix_default[] =
{
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC initPortTypeSpeeds_pca_sff_phoenix_default[] =
{
    {NOT_VALID_CNS, PRV_CPSS_PORT_NOT_EXISTS_E, CPSS_PORT_SPEED_NA_E}
};
static const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC initSpeeds_pb_gpc_phoenix_default[] =
{
    {56,         CPSS_PORT_SPEED_50000_E},
    {NOT_VALID_CNS, CPSS_PORT_SPEED_NA_E}
};

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet function
* @endinternal
*
* @brief    Get pattern of initial speeds of arbiter channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
*
* @retval pointer to pizza arbiter pattern on success or NULL
*/
const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
)
{
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_tx_dma_hawk_default;
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_pb_gpc_hawk_default;
                default: return NULL;
            }
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_tx_dma_phoenix_default;
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_pb_gpc_phoenix_default;
                default: return NULL;
            }
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E:
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_tx_dma_ironman_default;
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* const)
                            initSpeeds_pb_gpc_ironman_default;
                default: return NULL;
            }
            break;
        default: return NULL;
    }
}

const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC* prvCpssDxChTxPortTypeAndSpeedPizzaResourcesInitSpeedsGet
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
)
{
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC* const)
                            initPortTypeSpeeds_pca_sff_hawk_default;
                default: return NULL;
            }
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC* const)
                            initPortTypeSpeeds_pca_sff_phoenix_default;
                default: return NULL;
            }
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            switch (unitType)
            {
                case PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E:
                    return
                        (PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC* const)
                            initPortTypeSpeeds_pca_sff_ironman_default;
                default: return NULL;
            }
            break;
        default: return NULL;
    }
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure function
* @endinternal
*
* @brief    Configure arbiter for given channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
)
{
    GT_STATUS                                  rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    GT_U32  slotBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS];
    GT_U32                                     slot;
    GT_U32                                     localBmp;
    GT_U32                                     regAddr;
    GT_U32                                     data;
    GT_U32                                     chanMask;

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (hwInfo.slotRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(slotBmp, 0, sizeof(slotBmp));
    rc = prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet(
        project, unitType, unitIndex, channel, portSpeed, &(slotBmp[0]));
    if (rc != GT_OK)
    {
        return rc;
    }
    chanMask = (1 << hwInfo.slotRegChannelFldLen) - 1;
    if (channel & (~ chanMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (slot = 0; (slot < PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_SLOTS_CNS); slot++)
    {
        localBmp = slotBmp[slot / 32];
        if (localBmp == 0)
        {
            slot += 31;
            continue;
        }
        if ((localBmp & (1 << (slot % 32))) == 0) continue;

        regAddr = hwInfo.slotRegAddrArrPtr[slot];

        /* read arbiter slot configuration */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (data & (1 << hwInfo.slotRegValidBit))
        {
            /* slot already valid */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* update channel field and valid bit and write back */
        data |= (1 << hwInfo.slotRegValidBit);
        data &= (~ (chanMask << hwInfo.slotRegChannelFldPos));
        data |= (channel << hwInfo.slotRegChannelFldPos);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterInvalidate function
* @endinternal
*
* @brief    Invalidate arbiter for given channel.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterInvalidate
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS                                  rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC    *resourcePatternPtr;
    GT_U32                                     slot;
    GT_U32                                     regAddr;
    GT_U32                                     data;
    GT_U32                                     chanMask;

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(project, unitType, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwInfo.slotRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    chanMask = (1 << hwInfo.slotRegChannelFldLen) - 1;
    if (channel & (~ chanMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (slot = 0; (slot < resourcePatternPtr->numOfSlots); slot++)
    {
        regAddr = hwInfo.slotRegAddrArrPtr[slot];

        /* read arbiter slot configuration */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((data & (1 << hwInfo.slotRegValidBit)) == 0)
        {
            /* slot not valid */
            continue;
        }
        if (((data >> hwInfo.ctrlRegSizeFldPos) & chanMask) != channel)
        {
            /* slot allocated to another channel */
            continue;
        }
        /* update valid bit and write back */
        data &= (~ (1 << hwInfo.slotRegValidBit));
        data &= (~ (chanMask << hwInfo.slotRegChannelFldPos));
        data |= (resourcePatternPtr->defaultSlotNumber << hwInfo.slotRegChannelFldPos);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet function
* @endinternal
*
* @brief    Set profile for given channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
)
{
    GT_STATUS                                  rc;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC    *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    GT_U32                                     idx;
    GT_U32                                     speedId;
    GT_U32                                     data;
    GT_U32                                     regIndex;

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(project, unitType, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel >= resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwInfo.channelSpeedProfileRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search profile Id */
    for (idx = 0; (1); idx++)
    {
        if (resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdLow == NOT_VALID_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdLow <= speedId) &&
            (resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdHigh >= speedId))
        {
            data = resourcePatternPtr->speedIdToProfileTablePtr[idx].profileId;
            break;
        }
    }

    regIndex = (channel != hwInfo.cpuChannelLocalIndex) ? channel : hwInfo.cpuChannelGlobalIndex;
    return prvCpssDrvHwPpSetRegField(
        devNum, hwInfo.channelSpeedProfileRegAddrArrPtr[regIndex],
        hwInfo.channelSpeedProfileFldPos,
        hwInfo.channelSpeedProfileFldLen, data);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxDmaSpeedUpProfileConfigure function
* @endinternal
*
* @brief    Configure TX_DMA Speed Up profile for as modified pattern profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of configured profile
* @param[in] patternProfileId       - Id of pattern profile
* @param[in] txDmaIdleRateIncrement - value of modified field Idle Rate Increment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxDmaSpeedUpProfileConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId,
    IN    GT_U32                                    patternProfileId,
    IN    GT_U32                                    txDmaIdleRateIncrement
)
{
    GT_U32     srcRegAddr;
    GT_U32     dstRegAddr;
    GT_U32     i;
    GT_U32     data;
    GT_STATUS  rc;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* copy profile configuration from pattern to configured */
    for (i = 0; (i < 10); i++)
    {
        switch (i)
        {
            case 0:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.descFIFODepth[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.descFIFODepth[profileId];
                break;
            case 1:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqMaxCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqMaxCredits[profileId];
                break;
            case 2:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqThresholdBytes[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqThresholdBytes[profileId];
                break;
            case 3:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxWordCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxWordCredits[profileId];
                break;
            case 4:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxCellsCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxCellsCredits[profileId];
                break;
            case 5:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxDescCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxDescCredits[profileId];
                break;
            case 6:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateLimiter[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateLimiter[profileId];
                break;
            case 7:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interCellRateLimiter[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interCellRateLimiter[profileId];
                break;
            case 8:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateMaxTokens[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateMaxTokens[profileId];
                break;
            case 9:
                srcRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.rateLimitSpeedUp[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.rateLimitSpeedUp[profileId];
                break;
            default: continue; /*never reached */
        }
        if ((srcRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) || (dstRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED))
        {
            continue;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, srcRegAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, dstRegAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* modify  Idle Rate Increment */
    dstRegAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateLimiter[profileId];
    return prvCpssDrvHwPpSetRegField(devNum, dstRegAddr, 16, 16, txDmaIdleRateIncrement);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPbGprSpeedUpProfileConfigure function
* @endinternal
*
* @brief    Configure PB_GPR Speed Up profile for as modified pattern profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of configured profile
* @param[in] patternProfileId       - Id of pattern profile
* @param[in] pbGprShaperLeakValue  - value of modified field Shaper Leak Value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPbGprSpeedUpProfileConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId,
    IN    GT_U32                                    patternProfileId,
    IN    GT_U32                                    pbGprShaperLeakValue
)
{
    GT_U32     srcRegAddr;
    GT_U32     dstRegAddr;
    GT_U32     i;
    GT_U32     data;
    GT_STATUS  rc;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* copy profile configuration from pattern to configured */
    for (i = 0; (i < 7); i++)
    {
        switch (i)
        {
            case 0:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeCellCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeCellCredits[profileId];
                break;
            case 1:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeLevel012Credits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeLevel012Credits[profileId];
                break;
            case 2:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderCellCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderCellCredits[profileId];
                break;
            case 3:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderH2tCredits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderH2tCredits[profileId];
                break;
            case 4:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl01Credits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl01Credits[profileId];
                break;
            case 5:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl2Credits[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl2Credits[profileId];
                break;
            case 6:
                srcRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileShaper[patternProfileId];
                dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileShaper[profileId];
                break;
            default: continue; /*never reached */
        }
        if ((srcRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) || (dstRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED))
        {
            continue;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, srcRegAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, dstRegAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* modify  Idle Rate Increment */
    dstRegAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileShaper[profileId];
    return prvCpssDrvHwPpSetRegField(devNum, dstRegAddr, 0, 8, pbGprShaperLeakValue);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSpeedUpConfigure function
* @endinternal
*
* @brief    Configure speed up profiles for given speeed in DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] speedUpCfgTablePtr     - (pointer to) table of speed up parameters per speed
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSpeedUpConfigure
(
    IN    GT_U8                                                         devNum,
    IN    GT_U32                                                        unitIndex,
    IN    const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_UP_PROFILE_CFG_STC    *speedUpCfgTablePtr,
    IN    CPSS_PORT_SPEED_ENT                                           portSpeed
)
{
    GT_STATUS            rc;
    GT_U32               i;
    GT_U32               speedId;

    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (speedUpCfgTablePtr[i].speedId != NOT_VALID_CNS); i++)
    {
        if (speedUpCfgTablePtr[i].speedId == speedId) break;
    }

    if (speedUpCfgTablePtr[i].speedId != speedId)
    {
        /* the given speed is not speed-up */
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesTxDmaSpeedUpProfileConfigure(
        devNum, unitIndex,
        speedUpCfgTablePtr[i].profileId,
        speedUpCfgTablePtr[i].patternProfileId,
        speedUpCfgTablePtr[i].txDmaIdleRateIncrement);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPbGprSpeedUpProfileConfigure(
        devNum, unitIndex,
        speedUpCfgTablePtr[i].profileId,
        speedUpCfgTablePtr[i].patternProfileId,
        speedUpCfgTablePtr[i].pbGprShaperLeakValue);
    return rc;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxDmaProfileDump function
* @endinternal
*
* @brief    Dump TX_DMA profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxDmaProfileDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId
)
{
    GT_U32     regAddr;
    char*      name;
    GT_U32     i;
    GT_U32     data;
    GT_STATUS  rc;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    cpssOsPrintf("device %d TX_DMA %d profile %d\n", devNum, unitIndex, profileId);
    for (i = 0; (i < 10); i++)
    {
        switch (i)
        {
            case 0:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.descFIFODepth[profileId];
                name = "descFIFODepth";
                break;
            case 1:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqMaxCredits[profileId];
                name = "sdqMaxCredits\t\t";
                break;
            case 2:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.sdqThresholdBytes[profileId];
                name = "sdqThresholdBytes\t";
                break;
            case 3:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxWordCredits[profileId];
                name = "maxWordCredits\t\t";
                break;
            case 4:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxCellsCredits[profileId];
                name = "maxCellsCredits\t\t";
                break;
            case 5:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.maxDescCredits[profileId];
                name = "maxDescCredits\t\t";
                break;
            case 6:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateLimiter[profileId];
                name = "interPacketRateLimiter\t";
                break;
            case 7:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interCellRateLimiter[profileId];
                name = "interCellRateLimiter\t";
                break;
            case 8:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.interPacketRateMaxTokens[profileId];
                name = "interPacketRateMaxTokens";
                break;
            case 9:
                regAddr = regsAddrPtr->sip6_txDMA[unitIndex].configs.speedProfileConfigs.rateLimitSpeedUp[profileId];
                name = "rateLimitSpeedUp\t";
                break;
            default: continue; /*never reached */
        }
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            continue;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("%s \t 0x%08x\n", name, data);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPbGprProfileDump function
* @endinternal
*
* @brief    Dump PB_GPR profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPbGprProfileDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId
)
{
    GT_U32     regAddr;
    char*      name;
    GT_U32     i;
    GT_U32     data;
    GT_STATUS  rc;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    cpssOsPrintf("device %d PB_GPR_UNIT %d profile %d\n", devNum, unitIndex, profileId);
    for (i = 0; (i < 7); i++)
    {
        switch (i)
        {
            case 0:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeCellCredits[profileId];
                name = "profileMergeCellCredits";
                break;
            case 1:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileMergeLevel012Credits[profileId];
                name = "profileMergeLevel012Credits";
                break;
            case 2:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderCellCredits[profileId];
                name = "profileReorderCellCredits";
                break;
            case 3:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderH2tCredits[profileId];
                name = "profileReorderH2tCredits";
                break;
            case 4:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl01Credits[profileId];
                name = "profileReorderLvl01Credits";
                break;
            case 5:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileReorderLvl2Credits[profileId];
                name = "profileReorderLvl2Credits";
                break;
            case 6:
                regAddr = regsAddrPtr->sip6_packetBuffer.gpcPacketRead[unitIndex].profileShaper[profileId];
                name = "profileShaper\t\t";
                break;
            default: continue; /*never reached */
        }
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            continue;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("%s \t 0x%08x\n", name, data);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelReset function
* @endinternal
*
* @brief    Reset given channel.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelReset
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS                                  rc;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC    *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    GT_U32                                     idx;
    GT_U32                                     regIndex;

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(project, unitType, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel >= resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwInfo.channelResetRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regIndex = (channel != hwInfo.cpuChannelLocalIndex) ? channel : hwInfo.cpuChannelGlobalIndex;
    for (idx = 0; (idx < hwInfo.channelResetNumOfWrites); idx++)
    {
        rc = prvCpssDrvHwPpSetRegField(
            devNum, hwInfo.channelResetRegAddrArrPtr[regIndex],
            0, 1, ((hwInfo.channelResetBitsToWriteWrites >> idx) & 1));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesInit function
* @endinternal
*
* @brief    Initialyze pizza arbiter and profiles of default channels and resets them.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC             hwInfo;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC             *resourcePatternPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC  *initChanSpeedsPtr;
    GT_U32                                              slot;
    GT_U32                                              regAddr;
    GT_U32                                              data;
    GT_U32                                              fieldMask;
    GT_U32                                              chanIdx;
    GT_U32                                              pizzaOffset;

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(project, unitType, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pizzaOffset =
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp & (1 << unitIndex))
            ? resourcePatternPtr->pizzaOffsetMacSecOn
            : resourcePatternPtr->pizzaOffsetMacSecOff;

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    initChanSpeedsPtr = prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet(
        project, unitType);
    if (initChanSpeedsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (hwInfo.slotRegAddrArrPtr != NULL)
    {
        /* update pizza arbiter control register */
        regAddr = hwInfo.ctrlRegAddr;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* disable pizza arbiter writting the given amount of slots             */
        data &= (~ (1 << hwInfo.ctrlRegEnableBit));

        /* the register contains index of the last slot = (amount_of_slots - 1) */
        fieldMask = ((1 << hwInfo.ctrlRegSizeFldLen) - 1);
        if (resourcePatternPtr->numOfSlots & (~ fieldMask))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        data &= (~ (fieldMask << hwInfo.ctrlRegSizeFldPos));
        data |= ((resourcePatternPtr->numOfSlots - 1) << hwInfo.ctrlRegSizeFldPos);

        if ((pizzaOffset != NOT_VALID_CNS)
            && (hwInfo.ctrlRegPizzaOffsetFldPos != NOT_VALID_CNS))
        {
            fieldMask = ((1 << hwInfo.ctrlRegPizzaOffsetFldLen) - 1);
            if (pizzaOffset & (~fieldMask))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* write pizza offset */
            data &= (~ (fieldMask << hwInfo.ctrlRegPizzaOffsetFldPos));
            data |= (pizzaOffset << hwInfo.ctrlRegPizzaOffsetFldPos);
        }
        /* write with enable bit == 0 */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write with enable bit == 1 */
        data |= (1 << hwInfo.ctrlRegEnableBit);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* invalidate all slots */
        for (slot = 0; (slot < resourcePatternPtr->numOfSlots); slot++)
        {
            regAddr = hwInfo.slotRegAddrArrPtr[slot];

            /* read arbiter slot configuration */
            rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
            if (rc != GT_OK)
            {
                return rc;
            }
            if ((data & (1 << hwInfo.slotRegValidBit)) == 0)
            {
                /* slot not valid */
                continue;
            }
            /* update valid bit and write back */
            data &= (~ (1 << hwInfo.slotRegValidBit));
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* configure default channels */
    for (chanIdx = 0; (initChanSpeedsPtr[chanIdx].channelId != NOT_VALID_CNS); chanIdx++)
    {
        if (hwInfo.slotRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure(
                devNum, project, unitType, unitIndex,
                initChanSpeedsPtr[chanIdx].channelId,
                initChanSpeedsPtr[chanIdx].speedEnum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (hwInfo.channelSpeedProfileRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet(
                devNum, project, unitType, unitIndex,
                initChanSpeedsPtr[chanIdx].channelId,
                initChanSpeedsPtr[chanIdx].speedEnum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (hwInfo.channelResetRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelReset(
                devNum, project, unitType, unitIndex,
                initChanSpeedsPtr[chanIdx].channelId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDump function
* @endinternal
*
* @brief    Dump pizza arbiter and profiles of all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDump
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC             hwInfo;
    GT_U32                                              slot;
    GT_U32                                              regAddr;
    GT_U32                                              data;
    GT_U32                                              fieldMask;
    GT_U32                                              usedSlots;
    GT_U32                                              channelBmp = 0;
    GT_U32                                              channel;
    GT_U32                                              regIndex;

    cpssOsPrintf(
        "***** Unit type %s index %d *****\n",
        prvCpssDxChTxPortSpeedPizzaResourcesUnitIdToName(unitType),
        unitIndex);

    if (isNotSupportedUnit(devNum, unitType))
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E)
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump(
            devNum, unitIndex);
    }

    if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E)
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxDump(
            devNum, unitIndex);
    }

    if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E)
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxDump(
            devNum, unitIndex);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, unitType, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet failed\n");
        return rc;
    }

    if (hwInfo.slotRegAddrArrPtr != NULL)
    {
        regAddr = hwInfo.ctrlRegAddr;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        channelBmp = 0;
        fieldMask = ((1 << hwInfo.ctrlRegSizeFldLen) - 1);
        usedSlots = ((data >> hwInfo.ctrlRegSizeFldPos) & fieldMask) + 1;
        cpssOsPrintf(
            "PizzaArbiter enable %d used slots %d\n",
            ((data >> hwInfo.ctrlRegEnableBit) & 1), usedSlots);

        if (hwInfo.ctrlRegPizzaOffsetFldPos != NOT_VALID_CNS)
        {
            fieldMask = ((1 << hwInfo.ctrlRegPizzaOffsetFldLen) - 1);
            cpssOsPrintf(
                "PizzaArbiter Pizza Offset %d\n",
                ((data >> hwInfo.ctrlRegPizzaOffsetFldPos) & fieldMask));
        }

        cpssOsPrintf("    | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 |\n");
        cpssOsPrintf("----|-------------------------------------------------------------");
        for (slot = 0; (slot < usedSlots); slot++)
        {
            if ((slot % 20) == 0)
            {
                cpssOsPrintf("|\n%03d | ", (slot - (slot % 20)));
            }

            regAddr = hwInfo.slotRegAddrArrPtr[slot];
            /* read arbiter slot configuration */
            rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
                return rc;
            }
            if ((data & (1 << hwInfo.slotRegValidBit)) == 0)
            {
                /* slot not valid */
                cpssOsPrintf("** ");
                continue;
            }
            fieldMask = (1 << hwInfo.slotRegChannelFldLen) - 1;
            channel = (data >> hwInfo.slotRegChannelFldPos) & fieldMask;
            cpssOsPrintf("%02d ", channel);
            channelBmp |= (1 << channel);
        }
        cpssOsPrintf("\n");
        cpssOsPrintf("------------------------------------------------------------------");
        cpssOsPrintf("\n");
    }
    if (hwInfo.channelSpeedProfileRegAddrArrPtr != NULL)
    {
        cpssOsPrintf("Channel profilers:\n");
        for (channel = 0; (channel < 32); channel++)
        {
            if (((1 << channel) & channelBmp) == 0) continue;
            regIndex =
                (channel != hwInfo.cpuChannelLocalIndex) ? channel : hwInfo.cpuChannelGlobalIndex;
            regAddr = hwInfo.channelSpeedProfileRegAddrArrPtr[regIndex];
            rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
                return rc;
            }
            fieldMask = (1 << hwInfo.channelSpeedProfileFldLen) - 1;
            cpssOsPrintf("%d->%d ", channel, ((data >> hwInfo.channelSpeedProfileFldPos) & fieldMask));
        }
        cpssOsPrintf("\n");
    }

    if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E)
    {
        prvCpssDxChTxPortSpeedPizzaResourcesTxDmaProfileDump(
            devNum, unitIndex, 11/*profileId*/);
    }

    if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E)
    {
        prvCpssDxChTxPortSpeedPizzaResourcesPbGprProfileDump(
            devNum, unitIndex, 11/*profileId*/);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet function
* @endinternal
*
* @brief   Set source physical port for the given RX Channel.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X  Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portGlobalRxNum          - global Rx DMA port number including CPU port.
* @param[in] sorcePortNum             - source physical port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on wrong minCtByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet
(
    IN GT_U8                 devNum,
    IN GT_U32                portGlobalRxNum,
    IN GT_PHYSICAL_PORT_NUM  sorcePortNum
)
{
    GT_STATUS rc;                /* return code */
    GT_U32    portGroupId;       /* the port group Id - support multi port group device */
    GT_U32    localPort;         /* local port - support multi-port-groups device */
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    mask;              /* Mask for selecting the written bits */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
         CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if (sorcePortNum >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    data = sorcePortNum;
    mask = 0x3FF;

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portGlobalRxNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portGlobalRxNum);

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.channelConfig.channelToLocalDevSourcePort[localPort];

    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, portGroupId, regAddr, mask, data);

    return rc;
}

/* Minimal Byte Count of Cut Through packets - for Phoenix */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC rxMinCutThroughBc_phoenix[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,  512},
    {{0xFFFFFFFF, 0xFFFFFFFF},  1001,  384},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 10001,  256},
    {{0, 0},NOT_VALID_CNS, 0}
};

/* Minimal Byte Count of Cut Through packets - for Hawk and Harrier */
static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC rxMinCutThroughBc_hawk[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,  513},
    {{0xFFFFFFFF, 0xFFFFFFFF},  1001,  385},
    {{0xFFFFFFFF, 0xFFFFFFFF},  2501,  257},
    {{0xFFFFFFFF, 0xFFFFFFFF}, 10001,  256},
    {{0, 0},NOT_VALID_CNS, 0}
};

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesRxMinCutThroughBcGet function
* @endinternal
*
* @brief    Get RX Cut Through Minimal Byte Count.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - RX DMA unit index.
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
* @param[out] rxMinCutThroughBcPtr  - (pointer to) RX Cut Through Minimal Byte Count
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*
* @note - for all already supported projects unitIndex and channel parameters are redundant
*         but the conversion mechanism makes easy to support special cases for different units (separate tables)
*         and for special port (port bitmaps in table entries)
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesRxMinCutThroughBcGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *rxMinCutThroughBcPtr
)
{
    GT_U32 speedId;

    unitIndex = unitIndex; /* prevent compiler warning */

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            *rxMinCutThroughBcPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(rxMinCutThroughBc_phoenix), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            *rxMinCutThroughBcPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(rxMinCutThroughBc_hawk), channel, speedId);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return (*rxMinCutThroughBcPtr == NOT_VALID_CNS) ? GT_BAD_PARAM : GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortMiscConfigure function
* @endinternal
*
* @brief    Configure miscelanous features for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortMiscConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
)
{
    GT_STATUS  rc;
    GT_U32     speedInMbPerSec;
    GT_U32     minCtByteCount;
    GT_U32     unitIndex;
    GT_U32     channel;
    GT_U32     globalDmaNum;
    GT_U32     isPca;
    GT_BOOL    preemptiveModeActivated = GT_FALSE;
    GT_U32     preChannel = 0;/*extra channel for preemption*/
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT project;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(
        devNum, portNum, &unitIndex, &channel, &isPca);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* configure minimal Cut Through byte count */
    speedInMbPerSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);

    /*check if preemption was enabled for the port .
            The assumption is that Port manger enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum,speedInMbPerSec,&preemptiveModeActivated);
    if(rc != GT_OK)
    {
         return rc;
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum,speedInMbPerSec,channel,&preChannel);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
        devNum, unitIndex, channel, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesRxMinCutThroughBcGet(
        project, unitIndex, channel, portSpeed, &minCtByteCount);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChCutThroughPortMinCtByteCountSet(
        devNum, globalDmaNum, minCtByteCount);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChCutThroughPortMinCtByteCountSet globalDmaNum %d, minCtByteCount %d\n",
            globalDmaNum, minCtByteCount);
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, unitIndex, preChannel, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChCutThroughPortMinCtByteCountSet(
            devNum, globalDmaNum, minCtByteCount);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChCutThroughPortMinCtByteCountSet globalDmaNum %d, minCtByteCount %d\n",
                globalDmaNum, minCtByteCount);
        }

        rc = prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet(
            devNum, globalDmaNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet globalDmaNum %d, portNum %d\n",
                globalDmaNum, portNum);
        }
    }

    rc = prvCpssDxChFalconPortCutThroughSpeedSet(
        devNum, portNum, portSpeed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChFalconPortCutThroughSpeedSet portNum %d, speedInMbPerSec %d\n",
            portNum, speedInMbPerSec);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortMiscDown function
* @endinternal
*
* @brief    Power down miscelanous features for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortMiscDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_STATUS  rc;
    GT_U32     unitIndex;
    GT_U32     channel;
    GT_U32     isPca;
    GT_U32     globalDmaNum;
    GT_BOOL    preemptiveModeActivated = GT_FALSE;
    GT_U32     preChannel = 0;/*extra channel for preemption*/
    GT_PHYSICAL_PORT_NUM  preChannelDbPortNum;

    rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(
        devNum, portNum, &unitIndex, &channel, &isPca);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*check if preemption was enabled for the port .
            The assumption is that Port manger enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum, 0, &preemptiveModeActivated);
    if (rc != GT_OK)
    {
         return rc;
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, 0, channel, &preChannel);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, unitIndex, preChannel, &globalDmaNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert preChannel %d, globalDmaNum %d\n",
                preChannel, globalDmaNum);
        }

        /* restore source physical port number for the channel by DB */
        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(
            devNum, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, globalDmaNum, &preChannelDbPortNum);
        if (rc == GT_OK)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet(
                devNum, globalDmaNum, preChannelDbPortNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet globalDmaNum %d, portNum %d\n",
                    globalDmaNum, preChannelDbPortNum);
            }
        }

    }

    return GT_OK;
}

/* Full Units Init, DP channel Power up and Power down functions */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceGlobalInit function
* @endinternal
*
* @brief    Initialyze global resources of device.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceGlobalInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if (project == PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E)
    {
        data = 30; /*   Config Fill Threshold needed 30, default - 40 */
        regAddr = regsAddrPtr->sip6_packetBuffer.pbCounter.cfgFillThreshold;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceDpInit function
* @endinternal
*
* @brief    Initialyze pizza arbiters and profiles for default channels for all DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceDpInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
)
{
    GT_STATUS                            rc;
    GT_U32                               numOfDp;
    GT_U32                               ii;
    GT_U32                               validDataPathBmp;
    GT_U32                               unitIndex;
    GT_U32                               unitTypeIndex;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType;

    validDataPathBmp = 0;
    numOfDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
    for(ii = 0 ; (ii < numOfDp); ii++)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts == 0)
        {
            /* not valid DP (data path) */
            continue;
        }
        validDataPathBmp |= (1 << ii);
    }

    for (unitIndex = 0; (unitIndex < numOfDp); unitIndex++)
    {
        if ((validDataPathBmp & (1 << unitIndex)) == 0) continue;

        for (unitTypeIndex = 0; (unitTypeIndex < unitsInitSequenceArrSize); unitTypeIndex++)
        {
            unitType = unitsInitSequenceArr[unitTypeIndex];

            if (isNotSupportedUnit(devNum, unitType))
            {
                /* The GM not supports the DP related units except RX DMA */
                continue;
            }
            if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffInit(
                    devNum, project, unitIndex);
            }
            else if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxInit(
                    devNum, project, unitIndex);
            }
            else if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxInit(
                    devNum, project, unitIndex);
            }
            else
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesInit(
                    devNum, project, unitType, unitIndex);
            }
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure function
* @endinternal
*
* @brief    Configure pizzas and profile and reset channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] channel                - channel local number in Datapath
* @param[in] isPca                  - 0 - PCA units not used (SDMA), other PCA units used (network port)
* @param[in] portType               - port MAC type, relevant only for PCA ports (network ports)
* @param[in] preempionType          - channel preemption type
* @param[in] project                - project Id.
* @param[in] portSpeed              - port speed
* @param[in] usxPortMaxSpeed        - speed to configure all Pizzas and registers in PCA.
*                                     Maximal supported for USX port, for others same as portSpeed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure
(
    IN    GT_U8                                      devNum,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    GT_U32                                     isPca,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed,
    IN    CPSS_PORT_SPEED_ENT                        usxPortMaxSpeed
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC hwInfo;
    GT_U32                                  unitTypeIndex;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT    unitType;

    for (unitTypeIndex = 0; (unitTypeIndex < unitsConfigurationSequenceArrSize); unitTypeIndex++)
    {
        unitType = unitsConfigurationSequenceArr[unitTypeIndex];
        if (isNotSupportedUnit(devNum, unitType))
        {
            /* The GM not supports the DP related units except RX DMA */
            continue;
        }
        if ((! isPca) && isPcaUnit(unitType))
        {
            /* unit not relevant for SDMA mapped port */
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure(
                devNum, project, unitIndex,
                channel, portType, preempionType, usxPortMaxSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure(
                devNum, project, unitIndex,
                channel, usxPortMaxSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure(
                devNum, project, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }

        rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
            devNum, unitType, unitIndex, &hwInfo);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (hwInfo.slotRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure(
                devNum, project, unitType, unitIndex,
                channel, usxPortMaxSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* PB_GPR, TX_DMA, TX_FIFO profiles configured always to actual speed */
        if (hwInfo.channelSpeedProfileRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet(
                devNum, project, unitType, unitIndex,
                channel, portSpeed);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (hwInfo.channelResetRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelReset(
                devNum, project, unitType, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure function
* @endinternal
*
* @brief    Configure pizzas and profile and reset channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] project                - project Id.
* @param[in] ifMode                 - Interface mode
*                                     not relevant SDMA, PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_PHYSICAL_PORT_NUM                      portNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    CPSS_PORT_INTERFACE_MODE_ENT              ifMode,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
)
{
    GT_STATUS                               rc;
    GT_U32                                  unitIndex;
    GT_U32                                  channel;
    GT_U32                                  isPca;
    PRV_CPSS_PORT_TYPE_ENT                  portType;
    GT_BOOL                                 preemptiveModeActivated = GT_FALSE;
    GT_U32                                  preChannel = 0;/*extra channel for preemption*/
    GT_U32                                  speedInMbPerSec;
    CPSS_PORT_SPEED_ENT                     usxPortMaxSpeed;
    CPSS_SYSTEM_RECOVERY_INFO_STC           tempSystemRecovery_Info; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E) ) &&
         (tempSystemRecovery_Info.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))
    {
        /*only update event state and return */
        rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
        }
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(
        devNum, portNum, &unitIndex, &channel, &isPca);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* port type used only for PCA_SFF units configuration, ignored for other units */
    portType = PRV_CPSS_PORT_NOT_EXISTS_E;
    if (isPca)
    {
        rc = prvCpssDxChPortModeToPortTypeConvert(
            devNum, portNum, ifMode, portSpeed, &portType);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc, "error in prvCpssDxChPortModeToPortTypeConvert\n");
        }
    }

    speedInMbPerSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);

    usxPortMaxSpeed = portSpeed;
    if (project == PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E ||
        project == PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E)
    {
        /* for USX mode configure TX DP units for maximal speed supported by USX mode */
        usxPortMaxSpeed = prvCpssCommonPortUsxModeMaxSpeedGet(ifMode);
        if (usxPortMaxSpeed == CPSS_PORT_SPEED_NA_E)
        {
            usxPortMaxSpeed = portSpeed;
        }
    }


    /*check if preemption was enabled for the port .
            The assumption is that Port manger enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum, speedInMbPerSec, &preemptiveModeActivated);
    if(rc != GT_OK)
    {
         return rc;
    }

    if (GT_TRUE == preemptiveModeActivated )
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, speedInMbPerSec, channel,&preChannel);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesSpeedUpConfigure(
        devNum, unitIndex, speed_up_config_hawk_default, portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure(
        devNum, unitIndex, channel, isPca, portType,
        ((GT_TRUE == preemptiveModeActivated)
            ? PRV_CPSS_DXCH_PIZZA_ARBITER_EXPRESS_E
            : PRV_CPSS_DXCH_PIZZA_ARBITER_NOPREEMTION_E),
        project, portSpeed, usxPortMaxSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET,GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
    }

    if (GT_TRUE == preemptiveModeActivated )
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure(
            devNum, unitIndex, preChannel, isPca, portType,
            PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMPTIVE_E, project, portSpeed, usxPortMaxSpeed);
        if (rc != GT_OK)
        {
            /* rollback express channel */
            prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown(
                devNum, unitIndex, channel, isPca, project);
            return rc;
        }

        rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown function
* @endinternal
*
* @brief    Free pizzas slots of channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] channel                - channel local number in Datapath
* @param[in] isPca                  - 0 - PCA units not used (SDMA), other PCA units used (network port)
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    GT_U32                                    isPca,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC hwInfo;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT    unitType;
    GT_U32                                  unitTypeIndex;

    for (unitTypeIndex = 0; (unitTypeIndex < unitsPowerDownSequenceArrSize); unitTypeIndex++)
    {
        unitType = unitsPowerDownSequenceArr[unitTypeIndex];
        if (isNotSupportedUnit(devNum, unitType))
        {
            /* The GM not supports the DP related units except RX DMA */
            continue;
        }
        if ((! isPca) && isPcaUnit(unitType))
        {
            /* unit not relevant for SDMA mapped port */
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelDown(
                devNum, project, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelDown(
                devNum, project, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }
        if (unitType == PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelDown(
                devNum, project, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
            continue;
        }
        rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
            devNum, unitType, unitIndex, &hwInfo);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (hwInfo.slotRegAddrArrPtr != NULL)
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterInvalidate(
                devNum, project, unitType, unitIndex, channel);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown function
* @endinternal
*
* @brief    Free pizzas slots of channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown
(
    IN    GT_U8                                     devNum,
    IN    GT_PHYSICAL_PORT_NUM                      portNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
)
{
    GT_STATUS  rc, rc1;
    GT_U32     unitIndex;
    GT_U32     channel;
    GT_U32     isPca;
    GT_BOOL    preemptiveModeActivated = GT_FALSE;
    GT_U32     preChannel = 0;/*extra channel for preemption*/

    rc = prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum(
        devNum, portNum, &unitIndex, &channel, &isPca);
    if (rc != GT_OK)
    {
        return rc;
    }


    /*check if preemption was enabled for the port .
            The assumption is that Port manger enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum, 0, &preemptiveModeActivated);
    if(rc != GT_OK)
    {
         return rc;
    }

    rc = GT_OK;
    rc1 = prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown(
        devNum, unitIndex, channel, isPca, project);
    if (rc1 != GT_OK)
    {
        rc = rc1;
    }

    rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_CLEAR,GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum, 0, channel,&preChannel);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc1 = prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown(
            devNum, unitIndex, preChannel, isPca, project);
        if (rc1 != GT_OK)
        {
            rc = rc1;
        }

        rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_CLEAR,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffFifoWidthGet function
* @endinternal
*
* @brief    Get FIFO width of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in]  preempionType         - channel preemption type
* @param[in]  portSpeed             - port speed
* @param[out] fifo_widthPtr         - (pointer to) FIFO width
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffFifoWidthGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed,
    OUT   GT_U32                                     *fifo_widthPtr
)
{
    GT_U32  speedId;
    unitIndex = unitIndex; /* prevent compiler warning */

    /* this parameter used in PCA verification code  */
    /* probably this code will be partially restored */
    /* so the param yet not removed                  */
    preempionType = preempionType;

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            switch (portType)
            {
                case PRV_CPSS_PORT_MTI_100_E: *fifo_widthPtr = 1; /*32 bytes*/ break;
                case PRV_CPSS_PORT_MTI_400_E: *fifo_widthPtr = 3; /*128 bytes*/ break;
                case PRV_CPSS_PORT_MTI_CPU_E: *fifo_widthPtr = 0; /*8 bytes*/ break;
                case PRV_CPSS_PORT_MTI_USX_E: *fifo_widthPtr = 0; /*8 bytes*/ break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            };
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            *fifo_widthPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(sff_fifo_width_phoenix), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            *fifo_widthPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(sff_fifo_width_ironman), channel, speedId);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return (*fifo_widthPtr == NOT_VALID_CNS) ? GT_BAD_PARAM : GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxOfTokensGet function
* @endinternal
*
* @brief    Get Max of Tokens of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in]  portSpeed             - port speed
* @param[out] max_of_tokensPtr      - (pointer to) Max of Tokens
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxOfTokensGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                    portType,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *max_of_tokensPtr
)
{
    GT_U32  speedId;

    unitIndex = unitIndex; /* prevent compiler warning */
    portType = portType;

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            *max_of_tokensPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(sff_max_of_tokens_hawk), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            *max_of_tokensPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(sff_max_of_tokens_phoenix), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            *max_of_tokensPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(sff_max_of_tokens_ironman), channel, speedId);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return (*max_of_tokensPtr == NOT_VALID_CNS) ? GT_BAD_PARAM : GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxSlotsGet function
* @endinternal
*
* @brief    Get maximum slots for channel of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
* @param[out] maxSlotsPtr           - (pointer to)Max slots for channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxSlotsGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *maxSlotsPtr
)
{
    GT_U32  speedId;
    unitIndex = unitIndex; /* prevent compiler warning */

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            *maxSlotsPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(hawk_sff_max_num_of_slots), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            *maxSlotsPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(phoenix_sff_max_num_of_slots), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
            *maxSlotsPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(harrier_sff_max_num_of_slots), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_S_E:*/
            *maxSlotsPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(ironman_sff_max_num_of_slots), channel, speedId);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return (*maxSlotsPtr == NOT_VALID_CNS) ? GT_BAD_PARAM : GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxCreditsAllocGet function
* @endinternal
*
* @brief    Get TX Credits Alloc PCA BRG unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
* @param[out] brg_tx_creditsPtr      - (pointer to) TX Credits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxCreditsAllocGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *brg_tx_credits_allocPtr
)
{
    GT_U32 speedId;

    unitIndex = unitIndex; /* prevent compiler warning */

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E:
            *brg_tx_credits_allocPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(brg_tx_credits_alloc_hawk), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E:
            *brg_tx_credits_allocPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(brg_tx_credits_alloc_phoenix), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E:
            *brg_tx_credits_allocPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(brg_tx_credits_alloc_harrier), channel, speedId);
            break;
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
        /*case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:*/
            *brg_tx_credits_allocPtr =
                prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
                    ARRAY_ADDR_AND_SIZE(brg_tx_credits_alloc_ironman), channel, speedId);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return (*brg_tx_credits_allocPtr == NOT_VALID_CNS) ? GT_BAD_PARAM : GT_OK;
}

/* PCA SFF unit */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in] preempionType          - channel preemption type
* @param[in]  portSpeed             - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure
(
    IN    GT_U8                                      devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    index;
    GT_U32    prev_index;
    GT_U32    first_index;
    GT_U32    last_index;
    GT_U32    write_data;
    GT_U32    num_of_elements;
    GT_U32    max_num_of_elements;
    GT_U32    max_of_tokens;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    speedId;
    GT_U32    fifo_width;
    GT_U32    wait_slots;
    GT_U32    fifoElementsBmp[256/32];
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC           *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxSlotsGet(
        project, unitIndex, channel, portSpeed, &max_num_of_elements);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsMemSet(fifoElementsBmp, 0, sizeof(fifoElementsBmp));
    rc = prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex,
        channel, portSpeed, fifoElementsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* unused FIFO elements - each element next poiter poinits to itself */
    /* check and write ring                                              */
    /* the loop have additional execution with index == 256              */
    /* to write the last ring element pointing to the first              */
    num_of_elements = 0;
    first_index     = NOT_VALID_CNS;
    for (index = 0; (index < 256); index++)
    {
        if (index >= resourcePatternPtr->numOfSlots) break;
        if (fifoElementsBmp[index / 32] == 0)
        {
            index += 31; /* skip all word of bitmap */
            continue;
        }
        if ((fifoElementsBmp[index / 32] & (1 << (index % 32))) == 0)
        {
            continue;
        }
        last_index = index;
        if (first_index == NOT_VALID_CNS)
        {
            first_index = index;
        }
        num_of_elements ++;
        if (num_of_elements >= max_num_of_elements) break;
    }
    if (first_index == NOT_VALID_CNS)
    {
        /* empty bitmap */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prev_index      = NOT_VALID_CNS;
    /* each loop the previous element written   */
    /* additional loop for writing last element */
    for (index = first_index; (index <= (last_index + 1)); index++)
    {
        if (index <= last_index)
        {
            if (fifoElementsBmp[index / 32] == 0)
            {
                index += 31; /* skip all word of bitmap */
                continue;
            }
            if ((fifoElementsBmp[index / 32] & (1 << (index % 32))) == 0)
            {
                continue;
            }
            if (first_index == index)
            {
                prev_index = index;
                continue;
            }
            write_data = index;
        }
        else
        {
            write_data = first_index;
        }
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].llNextPointer[prev_index];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (data != prev_index)
        {
            /* found that FIFO element not free */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, write_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        prev_index = index;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffFifoWidthGet(
        project, unitIndex, channel, portType, preempionType, portSpeed, &fifo_width);
    if (rc != GT_OK)
    {
        return rc;
    }

    wait_slots = 4 * num_of_elements;

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxOfTokensGet(
        project, unitIndex, channel, portType, portSpeed, &max_of_tokens);
    if (rc != GT_OK)
    {
        return rc;
    }

    mask = (GT_U32)((0xFF << 24)/*ReadPtr*/ | (0xFF << 16)/*WritePtr*/
        | (3 << 14)/*FifoWidth*/ | (0x3F << 8)/*MaxOccup*/ | 0xFF/*WaitSlots*/);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (first_index << 24)/*ReadPtr*/ | (first_index << 16)/*WritePtr*/
        | (fifo_width << 14)/*FifoWidth*/ | (num_of_elements << 8)/*MaxOccup*/
        | wait_slots/*WaitSlots*/;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    mask = (GT_U32)((0xFF << 24)/*TokenMax*/ | (1 << 3)/*Token1Up*/
        | (1 << 2)/*Token0Up*/ | (1 << 7)/*StatEnable*/ | 1/*ChannelEnable*/);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (GT_U32)((max_of_tokens << 24)/*TokenMax*/ | (0 << 3)/*Token1Up*/
        | (1 << 2)/*Token0Up*/ | (0 << 7)/*StatEnable*/ | 1/*ChannelEnable*/);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelDown function
* @endinternal
*
* @brief    Disable Channel of Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    index;
    GT_U32    next_index;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    i;
    GT_U32    ring_passed;

    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC     *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    mask = (GT_U32)((0xFF << 24)/*TokenMax*/ | (1 << 3)/*Token1Up*/
        | (1 << 2)/*Token0Up*/ | (1 << 7)/*StatEnable*/ | 1/*ChannelEnable*/);
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* unused FIFO elements - each element next poiter poinits to itself  */
    /* start index retrieved from channelControlConfig register           */
    /* loop marks elements unused and ends finding already unused element */
    /* Loop header must prevent endless loop for any garbage in registers */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    index = (data >> 24) & 0xFF; /* start index in ring */
    ring_passed = 0;
    for (i = 0; (i < resourcePatternPtr->numOfSlots); i++)
    {
        if (index >= resourcePatternPtr->numOfSlots)
        {
            /* should never occur */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].llNextPointer[index];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (data == index)
        {
            /* found that FIFO element is free */
            ring_passed = 1;
            break;
        }
        next_index = data; /* next element in ring */
        data = index; /* mark current FIFO element free */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
        index = next_index;
    }
    if (ring_passed == 0)
    {
        /* should never occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    mask = (0xFF << 24)/*ReadPtr*/ | (0xFF << 16)/*WritePtr*/
        | (3 << 14)/*FifoWidth*/ | (0x3F << 8)/*MaxOccup*/ | 0xFF/*WaitSlots*/;
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (3 << 14)/*FifoWidth*/;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffInit function
* @endinternal
*
* @brief    Initialyze Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    index;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    orig_data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC  *initChanSpeedsPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC                       *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC                           *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    initChanSpeedsPtr = prvCpssDxChTxPortTypeAndSpeedPizzaResourcesInitSpeedsGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E);
    if (initChanSpeedsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_SHARED_PORT_PRV_DB_VAR(dpHawkDebugOptions) 
        & PRV_CPSS_DXCH_PIZZA_ARBITER_DEBUG_SFF_STATISTICS)
    {
        data = orig_data | (1 << 24) /*staisics enable*/;
    }
    else
    {
        data = orig_data & (~ (1 << 24)) /*staisics disable*/;
    }

    if (orig_data != data)
    {
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* unused FIFO - each element next poiter poinits to itself */
    for (index = 0; (index < resourcePatternPtr->numOfSlots); index++)
    {
        data = index;
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].llNextPointer[index];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (orig_data != data)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        mask = (GT_U32)((0xFF << 24)/*ReadPtr*/ | (0xFF << 16)/*WritePtr*/
            | (3 << 14)/*FifoWidth*/ | (0x3F << 8)/*MaxOccup*/ | 0xFF/*WaitSlots*/);
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        data = (~ mask) & orig_data;
        data |= (3 << 14)/*FifoWidth*/;
        if (orig_data != data)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        mask = (GT_U32)((0xFF << 24)/*TokenMax*/ | (1 << 3)/*Token1Up*/
            | (1 << 2)/*Token0Up*/ | (1 << 7)/*StatEnable*/ | 1/*ChannelEnable*/);
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        data = (~ mask) & orig_data;
        if (orig_data != data)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (channel = 0; (initChanSpeedsPtr[channel].channelId != NOT_VALID_CNS); channel++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure(
            devNum, project, unitIndex,
            initChanSpeedsPtr[channel].channelId,
            initChanSpeedsPtr[channel].portType,
            PRV_CPSS_DXCH_PIZZA_ARBITER_NOPREEMTION_E,
            initChanSpeedsPtr[channel].speedEnum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump function
* @endinternal
*
* @brief    Dump SFF Fifo of all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U8     llNextPtrArr[256];
    GT_U32    maxLlNextIndex;
    GT_U32    channelAmount;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    llStart;
    GT_U32    llNext;
    GT_U32    index;
    GT_U32    data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT        project;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChTxPortSpeedPizzaProjectGet failed\n");
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(llNextPtrArr, 0, sizeof(llNextPtrArr));

    channelAmount  = resourcePatternPtr->numOfChannels;
    maxLlNextIndex = resourcePatternPtr->numOfSlots;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* Global configuration */
    regAddr = regsAddrPtr->PCA_SFF[unitIndex].globalConfig;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
        return rc;
    }
    cpssOsPrintf(
        "global configuration: statistics collection enable %d statistics for SOP (not EOP) %d\n",
        ((data >> 24) & 1), ((data >> 23) & 1));
    cpssOsPrintf(
        "===================: statistics channel select %d token timeout interval %d\n",
        ((data >> 16) & 0x3F), ((data >> 8) & 0xFF));
    cpssOsPrintf(
        "===================: FW command %d Credit Sync Threshold %d\n",
        ((data >> 4) & 0x3), (data & 0x7));


    /* load aff llNextPointer registers to llNextPtrArr */
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].llNextPointer[index];
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            maxLlNextIndex = index;
            break;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        llNextPtrArr[index] = (GT_U8)(data & 0xFF);
    }

    /* unused FIFO - element next poiter poinits to itself */
    cpssOsPrintf("    | 000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 |\n");
    cpssOsPrintf("----|---------------------------------------------------------------------------------");
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        if ((index % 20) == 0)
        {
            cpssOsPrintf("|\n%03d | ", index);
        }
        if ((GT_U32)llNextPtrArr[index] == index)
        {
            /* free FIFO element */
            cpssOsPrintf("*** ");
            continue;
        }
        cpssOsPrintf("%03d ", llNextPtrArr[index]);
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("--------------------------------------------------------------------------------------");
    cpssOsPrintf("\n");

    /* per channel registers */
    for (channel = 0; (channel < channelAmount); channel++)
    {
        regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        if ((data & 1) == 0)
        {
            /* channel disable */
            continue;
        }
        cpssOsPrintf("channel %02d \n", channel);
        cpssOsPrintf(
            "ENABLE CfgTokenMax %d Token1RateThd %d Token0RateThd %d StatEn %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF), ((data >> 8) & 0xFF),
            ((data >> 7) & 1));
        cpssOsPrintf(
            "------ FwCmdGo %d PtpControlEnable %d Token1Up %d Token0Up %d Stop %d Enable %d\n",
            ((data >> 6) & 1), ((data >> 4) & 1), ((data >> 3) & 1), ((data >> 2) & 1),
            ((data >> 1) & 1), (data & 1));

        regAddr = regsAddrPtr->PCA_SFF[unitIndex].channelControlConfig[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        cpssOsPrintf(
            "CONFIG rdPtr %d wrPtr %d txFifoWidth %d MaxOccup %d WaitSlots %d\n",
            ((data >> 24) & 0xFF), ((data >> 16) & 0xFF), ((data >> 14) & 3),
            ((data >> 8) & 0x3F), (data & 0xFF));
        llStart = (data >> 24) & 0xFF;
        cpssOsPrintf("FIFO ring:\n");
        for (llNext = NOT_VALID_CNS; (llNext != llStart); llNext = llNextPtrArr[llNext])
        {
            if (llNext == NOT_VALID_CNS) llNext = llStart;
            cpssOsPrintf("%d ", llNext);
        }
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk TX PCA BRG unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    speedId;
    GT_U32    tx_credits_alloc;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* search speed Id */
    speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    if (speedId == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxCreditsAllocGet(
        project, unitIndex, channel, portSpeed, &tx_credits_alloc);
    if (rc != GT_OK)
    {
        return rc;
    }

    mask = (GT_U32)((0xFF << 16)/*TxCreditsAllocated*/ | (1 << 2)/*IgnoreCredits*/
        | (1 << 1)/*PCH_enable*/ | 1/*enable*/);
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (tx_credits_alloc << 16)/*TxCreditsAllocated*/ | (0 << 2)/*IgnoreCredits*/
        | (1 << 1)/*PCH_enable*/ | 1/*enable*/;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    mask;
    GT_U32    data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = (GT_U32)((1 << 1)/*PCH_enable*/ | 1/*enable*/);
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].rxChannel[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (GT_U32)((1 << 1)/*PCH_enable*/ | 1/*enable*/);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelDown function
* @endinternal
*
* @brief    Disable Channel of PCA BRG TX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    mask;
    GT_U32    data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = (GT_U32)((0xFF << 16)/*TxCreditsAllocated*/ | (1 << 2)/*IgnoreCredits*/
        | (1 << 1)/*PCH_enable*/ | 1/*enable*/);
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (0 << 16)/*TxCreditsAllocated*/ | (0 << 2)/*IgnoreCredits*/
        | (0 << 1)/*PCH_enable*/ | 0/*enable*/;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelDown function
* @endinternal
*
* @brief    Disable Channel of PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    mask;
    GT_U32    data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel > resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    mask = (GT_U32)((1 << 1)/*PCH_enable*/ | 1/*enable*/);
    regAddr = regsAddrPtr->PCA_BRG[unitIndex].rxChannel[channel];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }
    data &= (~ mask);
    data |= (GT_U32)((0 << 1)/*PCH_enable*/ | 0/*enable*/);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* HAWK specific */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxInit function
* @endinternal
*
* @brief    Initialyze PCA BRG TX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    orig_data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC  *initChanSpeedsPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    initChanSpeedsPtr = prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E);
    if (initChanSpeedsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        mask = (GT_U32)((0xFF << 16)/*TxCreditsAllocated*/ | (1 << 2)/*IgnoreCredits*/
            | (1 << 1)/*PCH_enable*/ | 1/*enable*/);
        regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        data = (~ mask) & orig_data;
        data |= (0 << 16)/*TxCreditsAllocated*/ | (0 << 2)/*IgnoreCredits*/
            | (0 << 1)/*PCH_enable*/ | 0/*enable*/;
        if (orig_data != data)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (channel = 0; (initChanSpeedsPtr[channel].channelId != NOT_VALID_CNS); channel++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure(
            devNum, project, unitIndex,
            initChanSpeedsPtr[channel].channelId,
            initChanSpeedsPtr[channel].speedEnum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxInit function
* @endinternal
*
* @brief    Initialyze PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    mask;
    GT_U32    data;
    GT_U32    orig_data;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC  *initChanSpeedsPtr;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    initChanSpeedsPtr = prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E);
    if (initChanSpeedsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        mask = (GT_U32)((1 << 1)/*PCH_enable*/ | 1/*enable*/);
        regAddr = regsAddrPtr->PCA_BRG[unitIndex].rxChannel[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &orig_data);
        if (rc != GT_OK)
        {
            return rc;
        }
        data = (~ mask) & orig_data;
        data |= (GT_U32)((0 << 1)/*PCH_enable*/ | 0/*enable*/);
        if (orig_data != data)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for (channel = 0; (initChanSpeedsPtr[channel].channelId != NOT_VALID_CNS); channel++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure(
            devNum, project, unitIndex,
            initChanSpeedsPtr[channel].channelId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxDump function
* @endinternal
*
* @brief    Dump BRG TX all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT        project;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChTxPortSpeedPizzaProjectGet failed\n");
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        regAddr = regsAddrPtr->PCA_BRG[unitIndex].txChannel[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        if ((data & 1) == 0)
        {
            /* channel disable */
            continue;
        }
        cpssOsPrintf(
            "channel %02d tx_credits_allocated %d drain %d tx_pch_en %d tx_en %d",
            channel, ((data >> 16) & 0xFF),
            ((data >> 2) & 1), ((data >> 1) & 1), (data & 1));

        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxDump function
* @endinternal
*
* @brief    Dump BRG RX all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT        project;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChTxPortSpeedPizzaProjectGet failed\n");
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    resourcePatternPtr = prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
        project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        regAddr = regsAddrPtr->PCA_BRG[unitIndex].rxChannel[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        if ((data & 1) == 0)
        {
            /* channel disable */
            continue;
        }
        cpssOsPrintf(
            "channel %02d rx_pch_en %d rx_en %d",
            channel, ((data >> 1) & 1), (data & 1));
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceInit function
* @endinternal
*
* @brief    Initialyze Hawk DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceInit
(
    IN    GT_U8                                devNum
)
{
    GT_STATUS            rc;
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32               numOfCpuPorts;
    CPSS_PORT_SPEED_ENT  cpuSpeed;
    GT_U32               i;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT project = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* holds system recovery information */

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)||
          (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
          (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) )
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesDeviceGlobalInit(devNum, project);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChTxPortSpeedPizzaResourcesDeviceDpInit(devNum, project);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChPortMappingCPUPortGet(
        devNum, /*OUT*/cpuPortNumArr, &numOfCpuPorts);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpuSpeed = CPSS_PORT_SPEED_10000_E;
    if ((PRV_CPSS_SIP_6_15_CHECK_MAC(devNum)) &&
        (numOfCpuPorts > 1))
    {
        /* Phoenix - 2 SDMA ports of 5G each */
        cpuSpeed = CPSS_PORT_SPEED_5000_E;
    }
    for (i = 0; (i < numOfCpuPorts); i++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure(
            devNum, cpuPortNumArr[i], CPSS_PORT_INTERFACE_MODE_NA_E, cpuSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure function
* @endinternal
*
* @brief    Configure all DP units for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant SDMA, PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
)
{
    GT_STATUS                 rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* holds system recovery information */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT project = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure(
        devNum, portNum, project, ifMode, portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E) ) &&
        (tempSystemRecovery_Info.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E) )
    {
        return GT_OK;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPortMiscConfigure(
        devNum, portNum, portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDown function
* @endinternal
*
* @brief    Free pizzas slots of port in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT project = PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* patch ironman up to correct update */
    switch (project)
    {
        case PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E:
            return GT_OK;
        default: break;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown(
        devNum, portNum, project);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChTxPortSpeedPizzaResourcesPortMiscDown(
        devNum, portNum);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelSpeedByTxDmaProfileGet function
* @endinternal
*
* @brief    Get channel and speed by Tx DMA profile.
*           Not exact - can be used only for speed save and restore in tests.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[out] portSpeed             - pointer to port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - convertion profile id to speed failed
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelSpeedByTxDmaProfileGet
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    OUT   CPSS_PORT_SPEED_ENT                       *portSpeedPtr
)
{
    GT_STATUS                                  rc;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC    *resourcePatternPtr;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    GT_U32                                     idx;
    GT_U32                                     speedId;
    GT_U32                                     speedIdLow;
    GT_U32                                     speedIdHigh;
    GT_U32                                     slot;
    GT_U32                                     chanMask;
    GT_U32                                     slotFound;
    GT_U32                                     data;
    GT_U32                                     regIndex;
    GT_U32                                     regAddr;
    CPSS_PORT_SPEED_ENT                        portSpeed;

    CPSS_NULL_PTR_CHECK_MAC(portSpeedPtr);
    if (isNotSupportedUnit(devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E))
    {
        /* The GM not supports the DP related units except RX DMA */
        return GT_OK;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
            project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (channel >= resourcePatternPtr->numOfChannels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwInfo.channelSpeedProfileRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check than the channel has pizza slots */
    slotFound = 0;
    chanMask = (1 << hwInfo.slotRegChannelFldLen) - 1;
    if (channel & (~ chanMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (slot = 0; (slot < resourcePatternPtr->numOfSlots); slot++)
    {
        regAddr = hwInfo.slotRegAddrArrPtr[slot];

        /* read arbiter slot configuration */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((data & (1 << hwInfo.slotRegValidBit)) == 0)
        {
            /* slot not valid */
            continue;
        }
        if (((data >> hwInfo.ctrlRegSizeFldPos) & chanMask) == channel)
        {
            /* slot allocated to another channel */
            slotFound ++;
            break;
        }
    }
    if (slotFound == 0)
    {
        /* no slots - channel disable */
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;
        return GT_OK;
    }

    regIndex = (channel != hwInfo.cpuChannelLocalIndex) ? channel : hwInfo.cpuChannelGlobalIndex;
    rc = prvCpssDrvHwPpGetRegField(
        devNum, hwInfo.channelSpeedProfileRegAddrArrPtr[regIndex],
        hwInfo.channelSpeedProfileFldPos,
        hwInfo.channelSpeedProfileFldLen, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* search range for profile Id */
    speedId     = NOT_VALID_CNS;
    speedIdLow  = NOT_VALID_CNS;
    speedIdHigh = NOT_VALID_CNS;
    for (idx = 0; (1); idx++)
    {
        if (resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdLow == NOT_VALID_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (data == resourcePatternPtr->speedIdToProfileTablePtr[idx].profileId)
        {
            speedIdLow  = resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdLow;
            speedIdHigh = resourcePatternPtr->speedIdToProfileTablePtr[idx].speedIdHigh;
            break;
        }
    }

    /* search speed Id */
    for (portSpeed = (CPSS_PORT_SPEED_ENT)0; (portSpeed < CPSS_PORT_SPEED_NA_E); portSpeed++)
    {
        speedId = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
        if (speedId == 0) continue;
        if ((speedIdLow <= speedId) && (speedIdHigh >= speedId))
        {
            /* one of mathing speeds found */
            break;
        }
    }
    if (portSpeed >= CPSS_PORT_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *portSpeedPtr = portSpeed;
    return GT_OK;
}

/*****************************************************************************/

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterReinitEmpty function
* @endinternal
*
* @brief    Reinitialize PCA TX pizza arbiter - all slots invalid.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX Arbier unit
* @param[in] usedSlots              - amount of slots used by PCA TX pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterReinitEmpty
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    usedSlots
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT             project;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC             hwInfo;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC       *resourcePatternPtr;
    GT_U32                                              slot;
    GT_U32                                              regAddr;
    GT_U32                                              data;
    GT_U32                                              fieldMask;
    GT_U32                                              pizzaOffset;
    GT_U32                                              numOfSlots;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
            project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pizzaOffset =
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp & (1 << unitIndex))
            ? resourcePatternPtr->pizzaOffsetMacSecOn
            : resourcePatternPtr->pizzaOffsetMacSecOff;

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    numOfSlots = usedSlots;

    /* update pizza arbiter control register */
    regAddr = hwInfo.ctrlRegAddr;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* disable pizza arbiter writting the given amount of slots             */
    data &= (~ (1 << hwInfo.ctrlRegEnableBit));

    /* the register contains index of the last slot = (amount_of_slots - 1) */
    fieldMask = ((1 << hwInfo.ctrlRegSizeFldLen) - 1);
    if (numOfSlots & (~ fieldMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    data &= (~ (fieldMask << hwInfo.ctrlRegSizeFldPos));
    data |= ((numOfSlots - 1) << hwInfo.ctrlRegSizeFldPos);

    if ((pizzaOffset != NOT_VALID_CNS)
        && (hwInfo.ctrlRegPizzaOffsetFldPos != NOT_VALID_CNS))
    {
        fieldMask = ((1 << hwInfo.ctrlRegPizzaOffsetFldLen) - 1);
        if (pizzaOffset & (~fieldMask))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* write pizza offset */
        data &= (~ (fieldMask << hwInfo.ctrlRegPizzaOffsetFldPos));
        data |= (pizzaOffset << hwInfo.ctrlRegPizzaOffsetFldPos);
    }
    /* write with enable bit == 0 */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write with enable bit == 1 */
    data |= (1 << hwInfo.ctrlRegEnableBit);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* invalidate all slots */
    for (slot = 0; (slot < numOfSlots); slot++)
    {
        regAddr = hwInfo.slotRegAddrArrPtr[slot];

        /* read arbiter slot configuration */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((data & (1 << hwInfo.slotRegValidBit)) == 0)
        {
            /* slot not valid */
            continue;
        }
        /* update valid bit and write back */
        data &= (~ (1 << hwInfo.slotRegValidBit));
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterRestore function
* @endinternal
*
* @brief    Restore PCA TX pizza arbiter - all channel-mapped slots restored.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX Arbier unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterRestore
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT             project;
    const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC       *resourcePatternPtr;
    GT_U32                                              channel;
    CPSS_PORT_SPEED_ENT                                 portSpeed;
    GT_U32                                              globalMacNum;
    GT_PHYSICAL_PORT_NUM                                physicalPortNum;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    resourcePatternPtr =
        prvCpssDxChTxPortSpeedPizzaResourcesPatternGet(
            project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E, unitIndex);
    if (resourcePatternPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =  prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterReinitEmpty(
        devNum, unitIndex, resourcePatternPtr->numOfSlots);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (channel = 0; (channel < resourcePatternPtr->numOfChannels); channel++)
    {
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, unitIndex, channel, &globalMacNum);
        if (rc != GT_OK)
        {
            /* not used channel */
            continue;
        }

        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(
            devNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, globalMacNum,
            &physicalPortNum);
        if (rc != GT_OK)
        {
            /* no MAC - probably SDMA */
            continue;
        }

        /* CPSS DB per global Mac - not per Physical port */
        portSpeed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, globalMacNum);
        if (CPSS_PORT_SPEED_NA_E <= portSpeed)
        {
            /* not configured port */
            continue;
        }

        rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure(
            devNum, project, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E,
            unitIndex, channel, portSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and slots bitmap.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] slotBmp                - bitmap of slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet
(
    IN    GT_U8   devNum,
    IN    GT_U32  unitIndex,
    IN    GT_U32  channel,
    IN    GT_U32  slotBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS]
)
{
    GT_STATUS                                  rc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project;
    PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC    hwInfo;
    GT_U32                                     slot;
    GT_U32                                     localBmp;
    GT_U32                                     regAddr;
    GT_U32                                     data;
    GT_U32                                     chanMask;

    rc = prvCpssDxChTxPortSpeedPizzaProjectGet(
        devNum, &project);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet(
        devNum, PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E, unitIndex, &hwInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (hwInfo.slotRegAddrArrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    chanMask = (1 << hwInfo.slotRegChannelFldLen) - 1;
    if (channel & (~ chanMask))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (slot = 0; (slot < PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_SLOTS_CNS); slot++)
    {
        localBmp = slotBmp[slot / 32];
        if (localBmp == 0)
        {
            slot += 31;
            continue;
        }
        if ((localBmp & (1 << (slot % 32))) == 0) continue;

        regAddr = hwInfo.slotRegAddrArrPtr[slot];

        /* read arbiter slot configuration */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (data & (1 << hwInfo.slotRegValidBit))
        {
            /* slot already valid */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* update channel field and valid bit and write back */
        data |= (1 << hwInfo.slotRegValidBit);
        data &= (~ (chanMask << hwInfo.slotRegChannelFldPos));
        data |= (channel << hwInfo.slotRegChannelFldPos);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetGen function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and slots bitmap words.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - physical device number
* @param[in] unitIndex           - index of PCA TX pizza arbiter
* @param[in] channel             - local number of channel
* @param[in] slotsBmp31_0        - bitmap of slots 0-31    of supported 256 slots
* @param[in] slotsBmp63_32       - bitmap of slots 32-63   of supported 256 slots
* @param[in] slotsBmp95_64       - bitmap of slots 64-95   of supported 256 slots
* @param[in] slotsBmp127_96      - bitmap of slots 96-127  of supported 256 slots
* @param[in] slotsBmp159_128     - bitmap of slots 128-159 of supported 256 slots
* @param[in] slotsBmp191_160     - bitmap of slots 160-191 of supported 256 slots
* @param[in] slotsBmp223_192     - bitmap of slots 192-223 of supported 256 slots
* @param[in] slotsBmp255_224     - bitmap of slots 224-255 of supported 256 slots
*
* @retval none
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetGen
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    GT_U32                                    slotsBmp31_0,
    IN    GT_U32                                    slotsBmp63_32,
    IN    GT_U32                                    slotsBmp95_64,
    IN    GT_U32                                    slotsBmp127_96,
    IN    GT_U32                                    slotsBmp159_128,
    IN    GT_U32                                    slotsBmp191_160,
    IN    GT_U32                                    slotsBmp223_192,
    IN    GT_U32                                    slotsBmp255_224
)
{
    GT_U32  slotBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS];

    cpssOsMemSet(slotBmp, 0, sizeof(slotBmp));
    slotBmp[0] = slotsBmp31_0;
    slotBmp[1] = slotsBmp63_32;
    slotBmp[2] = slotsBmp95_64;
    slotBmp[3] = slotsBmp127_96;
    slotBmp[4] = slotsBmp159_128;
    slotBmp[5] = slotsBmp191_160;
    slotBmp[6] = slotsBmp223_192;
    slotBmp[7] = slotsBmp255_224;

    return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
        devNum, unitIndex, channel, slotBmp);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetCyclic function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and slots cyclic sequence.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - physical device number
* @param[in] unitIndex           - index of PCA TX pizza arbiter
* @param[in] channel             - local number of channel
* @param[in] startSlot           - start slot of the sequence
* @param[in] slotsStep           - step of the sequence in slots
* @param[in] slotsAmount         - amount of slots of the sequence
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetCyclic
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    GT_U32                                    startSlot,
    IN    GT_U32                                    slotsStep,
    IN    GT_U32                                    slotsAmount
)
{
    GT_U32    slot;
    GT_U32    i;
    GT_U32    slotBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS];

    if ((startSlot + (slotsStep * (slotsAmount - 1))) >= PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_SLOTS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(slotBmp, 0, sizeof(slotBmp));
    for (i = 0; (i < slotsAmount); i++)
    {
        slot = startSlot + (slotsStep * i);
        slotBmp[slot / 32] |= (1 << (slot % 32));
    }

    return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
        devNum, unitIndex, channel, slotBmp);
}

/************************************************************************************************************/
/* Resources for Harrier PCA TX Pizza */

/* speed(G)  1  2.5  5  10  25  40  50  100  200  400 */
/* slots     1    1  2   4  10  16  20   40   80  160 */ 

/* Amount of slots in all the pizza for Harrier 208 */

/* bits of resource R0 (100G) */
/* 0 5 10 15 20 26 31 36 41 46 52 57 62 67 72 78 83 88 93 98 104 109 114 119 */
/* 124 130 135 140 145 150 156 161 166 171 176 182 187 192 197 202 */
static const GT_U32 harrierPcaTxPizzaR0[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS] =
{ 
    0x84108421, /*0 5 10 15 20 26 31*/
    0x42104210, /*36 41 46 52 57 62*/
    0x21084108, /*67 72 78 83 88 93*/
    0x10842104, /*98 104 109 114 119 124*/
    0x10421084, /*130 135 140 145 150 156*/
    0x08410842, /*161 166 171 176 182 187*/
    0x00000421, /*192 197 202*/
    0x00000000  /**/
};

/* bits of resource R1 (100G) */
/* 1 6 11 17 22 27 32 37 43 48 53 58 63 69 74 79 84 89 95 100 105 110 115 121 */
/* 126 131 136 141 147 152 157 162 167 173 178 183 188 193 199 204 */
static const GT_U32 harrierPcaTxPizzaR1[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS] =
{
    0x08420842, /*1 6 11 17 22 27*/
    0x84210821, /*32 37 43 48 53 58 63*/
    0x82108420, /*69 74 79 84 89 95*/
    0x42084210, /*100 105 110 115 121 126*/
    0x21082108, /*131 136 141 147 152 157*/
    0x10842084, /*162 167 173 178 183 188*/
    0x00001082, /*193 199 204*/
    0x00000000  /**/
};

/* bits of resource R2 (100G) */
/* 2 7 13 18 23 28 33 39 44 49 54 59 65 70 75 80 85 91 96 101 106 111 117 122 */
/* 127 132 137 143 148 153 158 163 169 174 179 184 189 195 200 205 */
static const GT_U32 harrierPcaTxPizzaR2[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS] =
{
    0x10842084, /*2 7 13 18 23 28*/
    0x08421082, /*33 39 44 49 54 59*/
    0x08210842, /*65 70 75 80 85 91*/
    0x84208421, /*96 101 106 111 117 122 127*/
    0x42108210, /*132 137 143 148 153 158*/
    0x21084208, /*163 169 174 179 184 189*/
    0x00002108, /*195 200 205*/
    0x00000000  /**/
};

/* bits of resource R3 (100G) */
/* 4 9 14 19 24 30 35 40 45 50 56 61 66 71 76 82 87 92 97 102 108 113 118 123 */
/* 128 134 139 144 149 154 160 165 170 175 180 186 191 196 201 206 */
static const GT_U32 harrierPcaTxPizzaR3[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS] =
{
    0x41084210, /*4 9 14 19 24 30*/
    0x21042108, /*35 40 45 50 56 61*/
    0x10841084, /*66 71 76 82 87 92*/
    0x08421042, /*97 102 108 113 118 123*/
    0x04210841, /*128 134 139 144 149 154*/
    0x84108421, /*160 165 170 175 180 186 191*/
    0x00004210, /*196 201 206*/
    0x00000000  /**/
};

/* 400G chan0 R0+R1+R2+R3 */
/* 200G chan0 R0+R2 chan8 R1+R3  */
/* 100G chan0 R0 chan4 R2 chan8 R1 chan12 R3 */

/* Partitions hierarchy speed <= 100G */
/* R0 0(0(0,1),2(2,3))*/
/* R2 4(4(4,5),6(6,7))*/
/* R1 8(8(8,9),10(10,11))*/
/* R3 12(12(12,13),14(14,15))*/

/* speed(G)      1  2.5   5  10  25  40  50  100  */
/* partition    10   10  10  10  10  20  20   40  */ 
/* used          1    1   2   4  10  16  20   40  */ 

/* service functions */
/*  Clear */
static void slotBitmapClear(OUT GT_U32 *bpmPtr)
{
    cpssOsMemSet(
        bpmPtr, 0, 
        (sizeof(GT_U32) * PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS));
}
/* bitwise OR */
static void slotBitmapAdd(INOUT GT_U32 *bpmPtr, const GT_U32 *addedBpmPtr)
{
    GT_U32 i;
    for (i = 0; (i < PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS); i++)
    {
        bpmPtr[i] |= addedBpmPtr[i];
    }
}

/* select from one-holding bits using cyclic filtering by selectBmp */
/* the selectBmp is a pattern applyed to one-holding bit sequence   */
/* not selected bits cleared                                        */
/* sample: source bmp 0xC317 - 8 one-holding bits, selectBmp 0x9, selectBmp size 4, cycles 2 */
/* after first cycle 0xC311, after second - 0x8111                                           */
static void slotBitmapSelectByScalarBmp(
    INOUT GT_U32 *bpmPtr, 
    IN    GT_U32 numOfCycles,
    IN    GT_U32 selectBmp,
    IN    GT_U32 numOfBitsInSelectBmp)
{
    GT_U32 i, j;
    GT_U32 word;
    GT_U32 cycleNum = 0;
    GT_U32 patternBit;


    cycleNum = 0;
    patternBit = 0;
    for (i = 0; (i < PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS); i++)
    {
        word = bpmPtr[i];
        for (j = 0; (j < 32); j++)
        {
            /* skip bits not seted in source without counting them */
            if ((word & (1 << j)) == 0) continue;

            if ((selectBmp & (1 << patternBit)) == 0)
            {
                /* zero bit in selectBmp - bit not selected and removed */
                word &= (~(1 << j));
            }
            patternBit ++;
            if (patternBit >= numOfBitsInSelectBmp)
            {
                patternBit = 0;
                cycleNum ++;
                if (cycleNum >= numOfCycles) break;
            }
        }
        bpmPtr[i] = word;
        if (cycleNum >= numOfCycles) break;
    }
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterHarrierChannelSlotsBySpeedSet function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and speed.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] speedInMbps            - speed in mega-bits per second.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterHarrierChannelSlotsBySpeedSet
(
    IN    GT_U8   devNum,
    IN    GT_U32  unitIndex,
    IN    GT_U32  channel,
    IN    GT_U32  speedInMbps
)
{
    GT_U32  slotBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS];
    GT_U32  selectBmp;

    slotBitmapClear(slotBmp);

    if (speedInMbps > 200000) /* case for 400G - any more than 200G */
    {
        /* 160 slots */
        if (channel != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        slotBitmapAdd(slotBmp, harrierPcaTxPizzaR0);
        slotBitmapAdd(slotBmp, harrierPcaTxPizzaR1);
        slotBitmapAdd(slotBmp, harrierPcaTxPizzaR2);
        slotBitmapAdd(slotBmp, harrierPcaTxPizzaR3);
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
            devNum, unitIndex, channel, slotBmp);
    }
    if (speedInMbps > 100000) /* case for 200G - any more than 100G*/
    {
        /* 80 slots */
        switch (channel)
        {
            case 0:
                slotBitmapAdd(slotBmp, harrierPcaTxPizzaR0);
                slotBitmapAdd(slotBmp, harrierPcaTxPizzaR2);
                break;
            case 8:
                slotBitmapAdd(slotBmp, harrierPcaTxPizzaR1);
                slotBitmapAdd(slotBmp, harrierPcaTxPizzaR3);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
            devNum, unitIndex, channel, slotBmp);
    }

    /* prepare partition in pizza to select subset */
    /* 40 slots */
    switch (channel)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            slotBitmapAdd(slotBmp, harrierPcaTxPizzaR0);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            slotBitmapAdd(slotBmp, harrierPcaTxPizzaR2);
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            slotBitmapAdd(slotBmp, harrierPcaTxPizzaR1);
            break;
        case 12:
        case 13:
        case 14:
        case 15:
            slotBitmapAdd(slotBmp, harrierPcaTxPizzaR3);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (speedInMbps > 50000) /* case for 100G - any more than 50G */
    {
        /* check channel and configure */
        if (channel % 4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
            devNum, unitIndex, channel, slotBmp);
    }

    /* partition for 50G */
    /* 20 slots - select evens or odds from 40 */
    slotBitmapSelectByScalarBmp(
        slotBmp, 20/*numOfCycles*/,
        (1 << ((channel >> 1) % 2)) /*selectBmp*/,
        2/*numOfBitsInSelectBmp*/);

    if (speedInMbps > 25000) /* case for 50G and 40G - any more than 25G */
    {/* 40G configured exactly as 50G */
        /* check channel and configure */
        if (channel % 2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
            devNum, unitIndex, channel, slotBmp);
    }

    if (speedInMbps > 10000)
    {
        /* partition for 25G - not compatible with 10G neigbour */
        /* 10 slots - select evens or odds from 20 */
        slotBitmapSelectByScalarBmp(
            slotBmp, 10/*numOfCycles*/,
            (1 << (channel % 2)) /*selectBmp*/,
            2/*numOfBitsInSelectBmp*/);
    }
    else
    {
        /* partition for 10G - not compatible with 25G neigbour */
        /* 4 slots - from 20 */
        slotBitmapSelectByScalarBmp(
            slotBmp, 4/*numOfCycles*/,
            (1 << (channel % 4)) /*selectBmp*/,
            5/*numOfBitsInSelectBmp*/);
    }


    if (speedInMbps > 5000) /* case for 25G and for 10G - any more than 5G */
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
            devNum, unitIndex, channel, slotBmp);
    }

    if (speedInMbps > 2500) /* case for 5G - any more than 2.5G */
    {
        /* select 2 from 4 slots */
        selectBmp      = 0x05;
    }
    else /* case for 2.5G and 1G */
    {
        /* select 1 from 4 slots */
        selectBmp      = 0x01;
    }

    slotBitmapSelectByScalarBmp(
        slotBmp, 1/*numOfCycles*/, selectBmp, 4/*selectBmpSize*/);
    return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSet(
        devNum, unitIndex, channel, slotBmp);
}

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsBySpeedSet function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and speed.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] speedInMbps            - speed in mega-bits per second.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsBySpeedSet
(
    IN    GT_U8   devNum,
    IN    GT_U32  unitIndex,
    IN    GT_U32  channel,
    IN    GT_U32  speedInMbps
)
{
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        return prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterHarrierChannelSlotsBySpeedSet(
            devNum, unitIndex, channel, speedInMbps);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
}

