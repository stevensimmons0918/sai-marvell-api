/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfDiagDataIntegrityErrorInjection.c
*
* @brief Tests of error injection in various memories
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>

#define VLANID_CNS           (5)
#define SENDER_PORT          (0)
#define WAIT_EVENTS_TIME_CNS (10000)

typedef enum
{
    PACKET_COUNTERS_EMPTY_E = 1,
    PACKET_COUNTERS_GOOD_RECEIVER_PORT_E,
    PACKET_COUNTERS_GOOD_SENDER_PORT_E
} PACKET_COUNTERS_STATE_ENT;

/* Memories to test */
typedef struct
{
    const char                      *name;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType;
    GT_U32                          burstCount;
    GT_U32                          expectedErrors;
    PACKET_COUNTERS_STATE_ENT       expectedCounters[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32                          expectedValidPackets;
    GT_U32                          gotErrors;
    GT_U32                          gotValidPackets;
    CPSS_DXCH_TABLE_ENT             expectedHwTable;
} MEMORY_DESC_STC;

#define MEMORY_ENTRY_CUSTOM_COUNTERS_MAC(__memType, __burstCount, __expEventNum, __sender, __receiver) \
    {#__memType, __memType, __burstCount, __expEventNum, {__sender, __receiver, __receiver, \
     __receiver, 0, }, (__burstCount) * 4, 0, 0, 0}

#define MEMORY_ENTRY_PASSED_MAC(__memType, __burstCount, __expEventNum)  \
    MEMORY_ENTRY_CUSTOM_COUNTERS_MAC(__memType, __burstCount, __expEventNum, \
    PACKET_COUNTERS_GOOD_SENDER_PORT_E, PACKET_COUNTERS_GOOD_RECEIVER_PORT_E)

#define MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(__memType, __burstCount, __expEventNum) \
    {#__memType, __memType, __burstCount, __expEventNum, {0, 0, 0, 0, 0, }, (__burstCount) * 4, 0, 0, 0}

static MEMORY_DESC_STC memoriesArr[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,
        1, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_PTR_E,
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E,
        2, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E,
        2, 1),

#if 0  /* need to understand how to get the RAM accessed */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E, 2, 1),
#endif

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 1),
};


/* memories for bobcat3 */
static MEMORY_DESC_STC memoriesArrBc3[] =
{

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 6),

    /* WRR occurs (>= 4 times) on every packet transmitting
       except first one. So send two packets. */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 4 /* Can be more than 4 */),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 4),

    /* one of QHEAD0...QHEAD3 occurs 4 times on every iteration */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD0_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD1_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD2_E,
        4, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD3_E,
        4, 4),

   /* one of QLAST0, ..., QLAST3 occurs on every packet transmitting
       expect first 4 ones. So send 8 packets to be sure event occurs
       for every of QLAST0, ..., QLAST3 */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST0_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST1_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST2_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST3_E,
        8, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E,
        1, 16 /* can be more that 16 */)
};


/* memories for aldrin2 */
static MEMORY_DESC_STC memoriesArrAldrin2[] =
{

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E,
        1, 4),

    /* WRR occurs (>= 4 times) on every packet transmitting
       except first one. So send two packets. */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E,
        2, 4 /* Can be more than 4 */),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_BC_UPD_E,
        1, 4),

    /* one of QHEAD0...QHEAD6 occurs 4 times on every iteration */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD0_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD1_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD2_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD3_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD4_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD5_E,
        8, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD6_E,
        8, 4),

   /* one of QLAST0, ..., QLAST6 occurs on every packet transmitting
       except first 7 ones. So send 24 packets to be sure event occurs
       for every of QLAST0, ..., QLAST6 */
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST0_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST1_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST2_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST3_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST4_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST5_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QLAST6_E,
        24, 4),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E,
        1, 8 /* can be more that 8 */)
};

/* memories for falcon error injection */
static MEMORY_DESC_STC memoriesArrFalconErrInjection[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_PFC_CNTRS_E,                                /* QFC_PFC_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_PFC_CNTRS_E,                                /* QFC_PFC_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_PFC_CNTRS_E,                                /* QFC_PFC_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_HEADER_BANK_E,            /* SIP_PHA_BYPASS_BUFFER_HDR - PARITY - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_HEADER_BANK_E,            /* SIP_PHA_BYPASS_BUFFER_HDR - PARITY - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_HEADER_BANK_E,            /* SIP_PHA_BYPASS_BUFFER_HDR - PARITY - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - RAM_INDEX */
        1, 1)
};

/* memories for AC5X error injection */
static MEMORY_DESC_STC memoriesArrAc5xErrInjection[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - HW_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - LOGICAL_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - RAM_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - HW_INDEX */
        1, 3),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - LOGICAL_INDEX */
        1, 3),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - RAM_INDEX */
        1, 3)
};

/* memories for AC5P error injection */
static MEMORY_DESC_STC memoriesArrAc5pErrInjection[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - HW_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - LOGICAL_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E,                                    /* IA_OS - SINGLE_ECC - RAM_INDEX */
        1, 6),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E,                         /* BMA_MC_CLEAR_SHIFTER - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E,                           /* PDS_DATA_STORAGE_0 - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E,                                  /* PDS_PID_TBL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E,                                 /* QFC_QL_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E,                                /* PDS_TAIL_TAIL - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - LOGICAL_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E,                                /* PDS_HEAD_HEAD - SINGLE_ECC - RAM_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - HW_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - LOGICAL_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E,                                    /* PDS_CNTRS - SINGLE_ECC - RAM_INDEX */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - HW_INDEX */
        1, 3),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - LOGICAL_INDEX */
        1, 3),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_BYPASS_BUFFER_DESC_E,                   /* SIP_PHA_BYPASS_BUFFER_DESC - PARITY - RAM_INDEX */
        1, 3)
};

/* memories for AC5 error injection */
static MEMORY_DESC_STC memoriesArrAc5ErrInjection[] =
{
    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,                   /* TXQ_LL_LINK_LIST    - PARITY/ECC - RAM_INFO */
        1, 1),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E,                /* TXQ_DQ_TB_GIGA_PRIO - PARITY/ECC - RAM_INFO */
        1, 1),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E,            /* MLL_PCL2MLL_UNUSED_FIFO - PARITY/ECC - RAM_INFO */
        1, 1),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E,                /* IP_UNUSED_DATA_FIFO - PARITY/ECC - RAM_INFO */
        1, 2),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,                   /* TXQ_LL_LINK_LIST    - PARITY/ECC - HW_INDEX */
        1, 0),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E,                /* TXQ_DQ_TB_GIGA_PRIO - PARITY/ECC - HW_INDEX */
        1, 1),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E,            /* MLL_PCL2MLL_UNUSED_FIFO - PARITY/ECC - HW_INDEX */
        1, 0),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E,                /* IP_UNUSED_DATA_FIFO - PARITY/ECC - HW_INDEX */
        1, 0),

    MEMORY_ENTRY_PASSED_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E,                   /* TXQ_LL_LINK_LIST    - PARITY/ECC - LOGICAL_INFO */
        1, 0),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E,                /* TXQ_DQ_TB_GIGA_PRIO - PARITY/ECC - LOGICAL_INFO */
        1, 1),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E,            /* MLL_PCL2MLL_UNUSED_FIFO - PARITY/ECC - LOGICAL_INFO */
        1, 0),

    MEMORY_ENTRY_PASSED_NO_PKT_CHECK_MAC(
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E,                /* IP_UNUSED_DATA_FIFO - PARITY/ECC - LOGICAL_INFO */
        1, 0)
};

/* memories for falcon */
static MEMORY_DESC_STC memoriesArrFalcon[] =
{
    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - HW_INDEX         */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - LOGICAL_INDEX    */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - RAM_INDEX        */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - HW_INDEX           */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - LOGICAL_INDEX      */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - RAM_INDEX          */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - HW_INDEX       */
            1, 2),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - LOGICAL_INDEX  */
            1, 2),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - RAM_INDEX      */
            1, 2),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E,       /* MG_SDMA_TX_FIFO - SINGLE_ECC - HW_INDEX       */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E,       /* MG_SDMA_TX_FIFO - SINGLE_ECC - LOGICAL_INDEX  */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E,       /* MG_SDMA_TX_FIFO - SINGLE_ECC - RAM_INDEX      */
            1, 1)
};

/* memories for AC5X */
static MEMORY_DESC_STC memoriesArrAc5x[] =
{
    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - HW_INDEX         */
            1, 4),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - LOGICAL_INDEX    */
            1, 4),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXF_WORD_BUFFER_MEM_E,   /* TXF_WORD - PARITY     - RAM_INDEX        */
            1, 4),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - HW_INDEX           */
            1, 6),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - LOGICAL_INDEX      */
            1, 6),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QNM_PB_SMB_MC_PM_E,      /* QNM_PB_SMB - PARITY - RAM_INDEX          */
            1, 6),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - HW_INDEX       */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - LOGICAL_INDEX  */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E,     /* HBU_HEADER - SINGLE_ECC - RAM_INDEX      */
            1, 1),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCA_SFF_RAM_E,       /*  PCA Shared FIFO - SINGLE_ECC - HW_INDEX       */
            1, 5),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCA_SFF_RAM_E,       /*  PCA Shared FIFO - SINGLE_ECC - LOGICAL_INDEX  */
            1, 5),

    MEMORY_ENTRY_PASSED_MAC(
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCA_SFF_RAM_E,       /*  PCA Shared FIFO - SINGLE_ECC - RAM_INDEX      */
            1, 5)
};



static MEMORY_DESC_STC *currentMemEntryPtr;

/* Event counter and semaphore signaling we have all needed events */
static GT_U32          evErrorCtr = 0;
static CPSS_OS_SIG_SEM eventErrorsCame = CPSS_OS_SEMB_EMPTY_E;

/* Callback function prototype for event counting */
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *savedDataIntegrityErrorCb;

/*
 * Packets info
 */

/* L2 part of unknown UC packet */
#define SEND_PACKET_SIZE_CNS (64)

static TGF_PACKET_L2_STC ucPacketL2Part =
{
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}  /* srcMac */
};
static TGF_PACKET_VLAN_TAG_STC packetVlanTagPart =
{
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
        0,  0,  VLANID_CNS               /* pri, cfi, VlanId */
};
static GT_U8 payloadDataArr[SEND_PACKET_SIZE_CNS] =
{
        0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};
static TGF_PACKET_PAYLOAD_STC packetPayloadPart =
{
        sizeof(payloadDataArr),   /* dataLength */
        payloadDataArr            /* dataPtr */
};
static TGF_PACKET_PART_STC ucPcktArray[] =
{
        {TGF_PACKET_PART_L2_E,       &ucPacketL2Part}, /* type, partPtr */
        {TGF_PACKET_PART_VLAN_TAG_E, &packetVlanTagPart},
        {TGF_PACKET_PART_PAYLOAD_E,  &packetPayloadPart}
};
#define PACKET_LEN_CNS \
        TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
        sizeof(payloadDataArr)

#define PACKET_CRC_LEN_CNS (PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

#define PACKET_CRC_LEN_WO_TAG_CNS (TGF_L2_HEADER_SIZE_CNS + \
    sizeof(payloadDataArr) + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC packetInfo = {
        PACKET_LEN_CNS,
        sizeof(ucPcktArray) / sizeof(TGF_PACKET_PART_STC),
        ucPcktArray
};

static GT_U32 memToDebug = 0xFFFFFFFF;
void memToDebugSet(GT_U32 newMemToDebug)
{
    memToDebug = newMemToDebug;
}


/**
* @internal dataIntegrityPacketCheck function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*
* @param[in] packetCountersPtr        - packet counters.
* @param[in] expectedState            - expected state of the packet counters
*/
static GT_BOOL dataIntegrityPacketCheck
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC *packetCountersPtr,
    IN PACKET_COUNTERS_STATE_ENT     expectedState
)
{
    GT_BOOL retVal;
    GT_U32  ii;
    GT_U8   *ptr;

    switch(expectedState)
    {
        case PACKET_COUNTERS_EMPTY_E:
            ptr = (GT_U8 *)packetCountersPtr;
            retVal = GT_TRUE;
            for(ii = 0; ii < sizeof(CPSS_PORT_MAC_COUNTER_SET_STC); ii++)
            {
                if(ptr[ii] != 0)
                {
                    retVal = GT_FALSE;
                }
            }
            break;
        case PACKET_COUNTERS_GOOD_RECEIVER_PORT_E:
            if((packetCountersPtr->goodOctetsSent.l[0] == PACKET_CRC_LEN_WO_TAG_CNS) &&
               (packetCountersPtr->goodPktsSent.l[0]   == 1) &&
               (packetCountersPtr->ucPktsSent.l[0]     == 1)
              )
            {
                retVal = GT_TRUE;
            }
            else
            {
                retVal = GT_FALSE;
            }
            break;
        case PACKET_COUNTERS_GOOD_SENDER_PORT_E:
            if((packetCountersPtr->goodOctetsRcv.l[0]  == PACKET_CRC_LEN_CNS) &&
               (packetCountersPtr->goodPktsRcv.l[0]    == 1) &&
               (packetCountersPtr->goodOctetsSent.l[0] == PACKET_CRC_LEN_CNS) &&
               (packetCountersPtr->goodPktsSent.l[0]   == 1) &&
               (packetCountersPtr->ucPktsRcv.l[0]      == 1) &&
               (packetCountersPtr->ucPktsSent.l[0]     == 1)
              )
            {
                retVal = GT_TRUE;
            }
            else
            {
                retVal = GT_FALSE;
            }
            break;
        default:
            retVal = GT_FALSE;
    }
    return retVal;
}

/**
* @internal dataIntegrityErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS dataIntegrityErrorHandler
(
 IN GT_U8                                     dev,
 IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
 )
{
    /* Print debug info */
    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG2_MAC("Device     : %d - %d\n", dev, prvTgfDevNum);
    PRV_UTF_LOG1_MAC("Event type : %d\n", eventPtr->eventsType);
    PRV_UTF_LOG6_MAC("Memory     : %d {%d, %d, %d}, line # %d, hwTableType[%d]\n",
                     eventPtr->location.ramEntryInfo.memType,
                     eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId,
                     eventPtr->location.ramEntryInfo.ramRow,
                     eventPtr->location.hwEntryInfo.hwTableType);

    if((dev == prvTgfDevNum) &&
        (eventPtr->location.ramEntryInfo.memType == currentMemEntryPtr->memType)
      )
    {
        evErrorCtr++;
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(currentMemEntryPtr->expectedHwTable,
                    eventPtr->location.hwEntryInfo.hwTableType,
                    "hwTableType mismatch for memType[%d]", eventPtr->location.ramEntryInfo.memType);
        }
        if(evErrorCtr == currentMemEntryPtr->expectedErrors)
        {
            cpssOsSigSemSignal(eventErrorsCame);
        }
    }
    return GT_OK;
}

/**
* @internal dataIntegrityInjectError function
* @endinternal
*
* @brief   Inject error in specified memory or disable injection.
*
* @param[in] memType                  - memory type
* @param[in] enable                   - if GT_TRUE --  error injection. In case of GT_FALSE --
*                                      disable error injection.
*                                       None
*/
static GT_VOID dataIntegrityInjectError
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType,
    IN CPSS_DXCH_LOCATION_ENT                       locationType,
    IN CPSS_DXCH_LOGICAL_TABLE_ENT                  logicalTableType,
    IN CPSS_DXCH_TABLE_ENT                          hwTableType,
    IN GT_BOOL                                      enable
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    cpssOsMemSet(&memoryLocation, 0, sizeof(memoryLocation));
    memoryLocation.portGroupsBmp             = 0xFFFFFFFF;
    memoryLocation.type                      = locationType;
    switch(memoryLocation.type)
    {
        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memoryLocation.info.hwEntryInfo.hwTableType = hwTableType;
            break;
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            memoryLocation.info.logicalEntryInfo.numOfLogicalTables = 1;
            memoryLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = logicalTableType;
            break;
        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            memoryLocation.info.ramEntryInfo.memType = memType;
            break;
        default:
            return;
    }

    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
        prvTgfDevNum,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        enable
    );
    UTF_VERIFY_EQUAL6_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityErrorInjectionConfigSet: "
        "dev=[%d], "
        "injectEn=[%d], "
        "memType=[%d], "
        "locationType=[%d], "
        "logicalTableType=[%d], "
        "hwTableType=[%d]",
        prvTgfDevNum,
        enable,
        memType,
        locationType,
        logicalTableType,
        hwTableType
    );
}

/**
* @internal dataIntegrityInterruptConfig function
* @endinternal
*
* @brief   Mask or unmask interrupt
*
* @param[in] memType                  - memory type
*                                      enable  - if GT_TRUE -- enable error injection. In case of GT_FALSE --
*                                      disable error injection.
*                                       None
*/
static GT_VOID dataIntegrityInterruptConfig
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT       memType,
    IN CPSS_EVENT_MASK_SET_ENT                          operation,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT    errorType,
    IN CPSS_DXCH_LOCATION_ENT                           locationType,
    IN CPSS_DXCH_LOGICAL_TABLE_ENT                      logicalTableType,
    IN CPSS_DXCH_TABLE_ENT                              hwTableType
)
{
    GT_STATUS                            rc, expRc = GT_OK;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    cpssOsMemSet(&memoryLocation, 0, sizeof(memoryLocation));
    memoryLocation.portGroupsBmp = 0xFFFFFFFF;
    memoryLocation.type = locationType;
    switch(locationType)
    {
        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memoryLocation.info.hwEntryInfo.hwTableType = hwTableType;
            break;
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            memoryLocation.info.logicalEntryInfo.numOfLogicalTables = 1;
            memoryLocation.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType = logicalTableType;
            break;
        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            memoryLocation.info.ramEntryInfo.memType = memType;
            break;
        default:
            return;
    }

    rc = cpssDxChDiagDataIntegrityEventMaskSet(
        prvTgfDevNum,
        &memoryLocation,
        errorType,
        operation
    );

    /* For RAMs which have ECC based DFX-protection type, parity based error
     * injection is an invalid operation.
     */
    if((locationType == CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE) &&
       (errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E) &&
       ((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_CNTRS_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IA_OS_MEM_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_DATA_STORAGE_0_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_PID_TBL_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_QL_CNTRS_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_QFC_PFC_CNTRS_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_HEAD_HEAD_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PDS_TAIL_TAIL_E)))
    {
        expRc = GT_BAD_PARAM;
    }

    UTF_VERIFY_EQUAL7_STRING_MAC(expRc, rc,
        "cpssDxChDiagDataIntegrityEventMaskSet: "
        "dev=[%d], "
        "mode=[%d], "
        "operation=[%d], "
        "locationType=[%d], "
        "logicalTableType=[%d], "
        "hwTableType=[%d], "
        "memType=[%d], ",
        prvTgfDevNum,
        errorType,
        operation,
        locationType,
        logicalTableType,
        hwTableType,
        memType
    );
}


/**
* @internal prvTgfDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Sets configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Create semaphore to wait all DI errors found */
    rc = cpssOsSigSemBinCreate("eventErrorsCame", CPSS_OS_SEMB_EMPTY_E,
        &eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = dataIntegrityErrorHandler;

    /* AUTODOC: Enable HW errors ignoring */
    prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E);

    /* Mask critical events caused by RAMs corruption due to error injection */
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssEventDeviceMaskSet(prvTgfDevNum,
                                    CPSS_PP_CRITICAL_HW_ERROR_E,
                                    CPSS_EVENT_MASK_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
    }

    /* AUTODOC: Create VLAN from available ports */
    rc = prvTgfBrgDefVlanEntryWrite(VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWrite: vlanId=[%d]", VLANID_CNS);
}

GT_VOID prvTgfDiagDataIntegrityErrorInjection_pktSendAndCheck
(
    CPSS_DXCH_LOCATION_ENT                          locationType,
    GT_U32                                          memArrIndex,
    GT_U32                                          memArrEndIndex
)
{
    GT_STATUS                                       rc;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType = 1;
    CPSS_DXCH_TABLE_ENT                             hwTableType = 0;
    CPSS_DXCH_LOGICAL_TABLE_ENT                     logicalTableType = 0;
    MEMORY_DESC_STC                                 *memArr;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    MEMORY_DESC_STC               *memEntryPtr = NULL;
    GT_U32                        pktCnt;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_BOOL                       validCntrs;
    GT_U32                        portIter;
    GT_U32                        expectedPacketCounter;

    memArr = memoriesArrAc5ErrInjection;

    for(; memArrIndex < memArrEndIndex; memArrIndex++)
    {
        if ((memToDebug != 0xFFFFFFFF) && (memToDebug != memArrIndex))
        {
            continue;
        }

        if(locationType != CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE)
        {
            memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
            memEntry.info.ramEntryInfo.memType = memArr[memArrIndex].memType;

            rc = cpssDxChDiagDataIntegrityProtectionTypeGet(prvTgfDevNum, &memEntry, &protectionType);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChDiagDataIntegrityProtectionTypeGet");

            if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
            {
                errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                hwTableType      = CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_ECC_E;
                logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_ECC_E;
            }
            else
            {
                errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                hwTableType      = CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_PARITY_E;
                logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_PARITY_E;
            }
        }

        memEntryPtr                  = &memArr[memArrIndex];
        currentMemEntryPtr           = memEntryPtr;
        currentMemEntryPtr->expectedHwTable = hwTableType;
        expectedPacketCounter        = 0;

        /* AUTODOC: Inject error */
        dataIntegrityInjectError(memEntryPtr->memType,
                locationType,
                logicalTableType,
                hwTableType,
                GT_TRUE);

        /* AUTODOC: Unmask event */
        dataIntegrityInterruptConfig(memEntryPtr->memType,
                CPSS_EVENT_UNMASK_E,
                errorType,
                locationType,
                logicalTableType,
                hwTableType);

        for(pktCnt = 0; pktCnt < memEntryPtr->burstCount; pktCnt++ ) /* Packet counter loop */
        {
            /* AUTODOC: Reset counters */
            rc = prvTgfEthCountersReset(prvTgfDevNum);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

            /* Do not print packets */
            tgfTrafficTracePacketByteSet(GT_FALSE);

            /* AUTODOC: Send packet */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                    prvTgfPortsArray[SENDER_PORT]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfStartTransmitingEth: dev=[%d], port=[%d]\n",
                    prvTgfDevNum, prvTgfPortsArray[SENDER_PORT]);

            /* AUTODOC: Get packet counters */
            for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                if(memEntryPtr->expectedCounters[portIter] == 0)
                {
                    break;
                }

                rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                        prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "prvTgfReadPortCountersEth: dev=[%d], enablePrint[%d]\n",
                        prvTgfDevNum, prvTgfPortsArray[portIter]);

                validCntrs = dataIntegrityPacketCheck(&portCntrs,
                        memEntryPtr->expectedCounters[portIter]);
                if(validCntrs)
                {
                    expectedPacketCounter++;
                }
            }
            memEntryPtr->gotValidPackets = expectedPacketCounter;
        }

        /* AUTODOC: Wait for events */
        rc = cpssOsSigSemWait(eventErrorsCame, WAIT_EVENTS_TIME_CNS);

        if(rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("Not all events came. "
                    "We have %d, but expected %d\n",
                    evErrorCtr, memEntryPtr->expectedErrors);

            /* Send signal to free semaphore */
            cpssOsSigSemSignal(eventErrorsCame);
        }
        else
        {
            /* AUTODOC: Maybe some unexpected signals will come */
            cpssOsTimerWkAfter(1000);
        }

        /* allow number or errors to be more than expected. */
        if(memEntryPtr->expectedErrors >= evErrorCtr)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(memEntryPtr->expectedErrors, evErrorCtr,
                    "Unexpected event count for memType=[%d], expected %d, got %d",
                    memArrIndex, memEntryPtr->expectedErrors, evErrorCtr);
        }

        memEntryPtr->gotErrors = evErrorCtr;
        evErrorCtr = 0;

        dataIntegrityInjectError(memEntryPtr->memType,
                locationType,
                logicalTableType,
                hwTableType,
                GT_FALSE);

        /* AUTODOC: mask event */
        dataIntegrityInterruptConfig(memEntryPtr->memType,
                CPSS_EVENT_MASK_E,
                errorType,
                locationType,
                logicalTableType,
                hwTableType);
    }

}

GT_VOID prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate_AC5
(
    GT_VOID
)
{
    GT_STATUS                     rc;
    MEMORY_DESC_STC               *memEntryPtr = NULL;
    CPSS_INTERFACE_INFO_STC       iface;
    GT_U32                        ii;
    CPSS_DXCH_LOCATION_ENT                          locationTypeArr[3] =
        {CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE,
         CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE,
         CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE};

    GT_U32                                          memArrIndex = 0;
    GT_U32                                          memArrLen;


    cpssOsMemSet(&iface, 0, sizeof(iface));
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: dev=[%d]",
            prvTgfDevNum);

    memArrLen = sizeof(memoriesArrAc5ErrInjection)/sizeof(memoriesArrAc5ErrInjection[0]);
    for(ii = 0; ii < 3; ii++)
    {
       prvTgfDiagDataIntegrityErrorInjection_pktSendAndCheck(locationTypeArr[ii],
            memArrIndex,                    /* Start Index */
            memArrIndex + (memArrLen/3));   /* End Index */
        memArrIndex += (memArrLen/3);
    }

    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* Print overall results */
    PRV_UTF_LOG0_MAC("\n\n\n======TEST RESULTS======\n");
    for(memArrIndex = 0; memArrIndex < memArrLen; memArrIndex++)
    {
        memEntryPtr = &memoriesArrAc5ErrInjection[memArrIndex];

        PRV_UTF_LOG3_MAC("%s expected errors: %d, got errors %d\n",
                memEntryPtr->name,
                memEntryPtr->expectedErrors,
                memEntryPtr->gotErrors
                );
        PRV_UTF_LOG3_MAC("%s expected packets: %d, got %d\n",
                memEntryPtr->name,
                memEntryPtr->expectedValidPackets,
                memEntryPtr->gotValidPackets
                );
    }
    PRV_UTF_LOG0_MAC("\n\n");

}

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                        ii;
    GT_U32                        jj;
    GT_STATUS                     rc;
    GT_U32                        portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_BOOL                       validCntrs;
    GT_U32                        expectedPacketCounter;
    MEMORY_DESC_STC               *memEntryPtr = NULL;
    CPSS_INTERFACE_INFO_STC       iface;
    MEMORY_DESC_STC               *memArr;
    GT_U32                        memArrLen;
    GT_U32                        doSkip;
    GT_U32                        memCnt, pktCnt;
    CPSS_DXCH_LOCATION_ENT                          locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    CPSS_DXCH_TABLE_ENT                             hwTableType;
    CPSS_DXCH_LOGICAL_TABLE_ENT                     logicalTableType;
    GT_U32                                          memArrIndex = 0;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;


    cpssOsMemSet(&iface, 0, sizeof(iface));
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;


    if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrBc3;
        memArrLen = sizeof(memoriesArrBc3)/sizeof(memoriesArrBc3[0]);
    }
    else if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrAldrin2;
        memArrLen = sizeof(memoriesArrAldrin2)/sizeof(memoriesArrAldrin2[0]);
    }
    else if (PRV_CPSS_DXCH_FALCON_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrFalconErrInjection;
        memArrLen = sizeof(memoriesArrFalconErrInjection)/sizeof(memoriesArrFalconErrInjection[0]);
    }
    else if (PRV_CPSS_DXCH_AC5X_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrAc5xErrInjection;
        memArrLen = sizeof(memoriesArrAc5xErrInjection)/sizeof(memoriesArrAc5xErrInjection[0]);
    }
    else if (PRV_CPSS_DXCH_AC5_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfDiagDataIntegrityErrorInjectionTrafficGenerate_AC5();
        return;
    }
    else if (PRV_CPSS_DXCH_AC5P_CHECK_MAC(prvTgfDevNum))
    {
        memArr = memoriesArrAc5pErrInjection;
        memArrLen = sizeof(memoriesArrAc5pErrInjection)/sizeof(memoriesArrAc5pErrInjection[0]);
    }
    else
    {
        memArr = memoriesArr;
        memArrLen = sizeof(memoriesArr)/sizeof(memoriesArr[0]);
    }

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: dev=[%d]",
        prvTgfDevNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        for(ii = 0; ii < memArrLen/3 /* per RAM Info Types events*/; ii++)   /* Falcon RAMs Loop */
        {
            memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
            memEntry.info.ramEntryInfo.memType = memArr[memArrIndex].memType;

            rc = cpssDxChDiagDataIntegrityProtectionTypeGet(prvTgfDevNum, &memEntry, &protectionType);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChDiagDataIntegrityProtectionTypeGet");

            if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
            {
                errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                hwTableType      = CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_ECC_E;
                logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_ECC_E;
            }
            else
            {
                errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                hwTableType      = CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_PARITY_E;
                logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_PARITY_E;
            }

            for(locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE; locationType <= CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE; locationType++)
            {
                memEntryPtr                  = &memArr[memArrIndex];
                currentMemEntryPtr           = memEntryPtr;
                currentMemEntryPtr->expectedHwTable = hwTableType;

                expectedPacketCounter        = 0;

                /* AUTODOC: Inject error */
                dataIntegrityInjectError(memEntryPtr->memType,
                    locationType,
                    logicalTableType,
                    hwTableType,
                    GT_TRUE);

                /* AUTODOC: Unmask event */
                dataIntegrityInterruptConfig(memEntryPtr->memType,
                    CPSS_EVENT_UNMASK_E,
                    errorType,
                    locationType,
                    logicalTableType,
                    hwTableType);

                for(pktCnt = 0; pktCnt < memEntryPtr->burstCount; pktCnt++ ) /* Packet counter loop */
                {
                    /* AUTODOC: Reset counters */
                    rc = prvTgfEthCountersReset(prvTgfDevNum);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

                    /* Do not print packets */
                    tgfTrafficTracePacketByteSet(GT_FALSE);

                    /* AUTODOC: Send packet */
                    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                        prvTgfPortsArray[SENDER_PORT]);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "prvTgfStartTransmitingEth: dev=[%d], port=[%d]\n",
                        prvTgfDevNum, prvTgfPortsArray[SENDER_PORT]);

                    /* AUTODOC: Get packet counters */
                    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
                    {
                        if(memEntryPtr->expectedCounters[portIter] == 0)
                        {
                            break;
                        }

                        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                            prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                            "prvTgfReadPortCountersEth: dev=[%d], enablePrint[%d]\n",
                            prvTgfDevNum, prvTgfPortsArray[portIter]);

                        validCntrs = dataIntegrityPacketCheck(&portCntrs,
                            memEntryPtr->expectedCounters[portIter]);
                        /* TODO ?? UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, validCntrs,
                            "Got unexpected counters"); */
                        if(validCntrs)
                        {
                            expectedPacketCounter++;
                        }
                    }
                    memEntryPtr->gotValidPackets = expectedPacketCounter;
                }

                /* AUTODOC: Wait for events */
                rc = cpssOsSigSemWait(eventErrorsCame, WAIT_EVENTS_TIME_CNS);

                if(rc != GT_OK)
                {
                    PRV_UTF_LOG2_MAC("Not all events came. "
                        "We have %d, but expected %d\n",
                        evErrorCtr, memEntryPtr->expectedErrors);

                    /* Send signal to free semaphore */
                    cpssOsSigSemSignal(eventErrorsCame);
                }
                else
                {
                    /* AUTODOC: Maybe some unexpected signals will come */
                    cpssOsTimerWkAfter(1000);
                }

                /* allow number or errors to be more than expected. */
                if(memEntryPtr->expectedErrors >= evErrorCtr)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(memEntryPtr->expectedErrors, evErrorCtr,
                        "Unexpected event count for memType=[%d], expected %d, got %d",
                        memArrIndex, memEntryPtr->expectedErrors, evErrorCtr);
                }

                memEntryPtr->gotErrors = evErrorCtr;
                evErrorCtr = 0;

                dataIntegrityInjectError(memEntryPtr->memType,
                    locationType,
                    logicalTableType,
                    hwTableType,
                    GT_FALSE);

                /* AUTODOC: mask event */
                dataIntegrityInterruptConfig(memEntryPtr->memType,
                    CPSS_EVENT_MASK_E,
                    errorType,
                    locationType,
                    logicalTableType,
                    hwTableType);

                memArrIndex += 1;
            }
        }
        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* Print overall results */
        PRV_UTF_LOG0_MAC("\n\n\n======TEST RESULTS======\n");
        memArrIndex = 0;
        for(memCnt = 0; memCnt < memArrLen/3; memCnt++)
        {
            for(locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE; locationType<=CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE; locationType++)
            {
                memEntryPtr = &memArr[memArrIndex];;
                PRV_UTF_LOG3_MAC("%s expected errors: %d, got errors %d\n",
                    memEntryPtr->name,
                    memEntryPtr->expectedErrors,
                    memEntryPtr->gotErrors
                );
                PRV_UTF_LOG3_MAC("%s expected packets: %d, got %d\n",
                    memEntryPtr->name,
                    memEntryPtr->expectedValidPackets,
                    memEntryPtr->gotValidPackets
                );

                memArrIndex += 1;
            }
        }
        PRV_UTF_LOG0_MAC("\n\n");
    }
    else
    {
        for(ii = 0; ii < memArrLen; ii++)
        {
            if ((memToDebug != 0xFFFFFFFF) && (memToDebug != ii))
            {
                continue;
            }

            memEntryPtr = &memArr[ii];

            switch (memEntryPtr->memType)
            {
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E:
                    /* The memory is accessed during the test but there are
                     no interrupts. Maybe several writings happen before reading
                     and reset injected bit. Skip. */
                    doSkip = IS_BOBK_DEV_MAC(prvTgfDevNum);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E:
                    /* events doesn't occur on bobk with enabled TM. */
                    doSkip = (IS_BOBK_DEV_MAC(prvTgfDevNum) &&
                              prvUtfIsTrafficManagerUsed());
                    break;

                default:
                    doSkip = 0;
                    break;
            }
            if (doSkip)
            {
                continue;
            }

            PRV_UTF_LOG1_MAC("=== Start test for %s ===\n",memEntryPtr->name);

            currentMemEntryPtr = memEntryPtr;

            expectedPacketCounter = 0;

            /* AUTODOC: Unmask event */
            dataIntegrityInterruptConfig(memEntryPtr->memType,
                CPSS_EVENT_UNMASK_E,
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
                CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE, 0, 0 /* NOT used for LOCATION_RAM */);

            for(jj = 0; jj < memEntryPtr->burstCount; jj++)
            {
                /* AUTODOC: Reset counters */
                rc = prvTgfEthCountersReset(prvTgfDevNum);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

                /* AUTODOC: Inject error */
                dataIntegrityInjectError(memEntryPtr->memType,
                    CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE,
                    0, 0, /* Not used for LOCATION_RAM */
                    GT_TRUE);

                /* Do not print packets */
                tgfTrafficTracePacketByteSet(GT_FALSE);

                /* WA for 100G ports. To make 100G link up CPSS API sets
                   fields RX_ENA, TX_ENA to 1 in "COMMAND_CONFIG Register".
                   RX_ENA=1 causes loopback when packet goes on 100G tx port.
                   tgfTrafficGeneratorPortLoopbackModeEnableSet(GT_FALSE)
                   sets RX_ENA to 0. Should be placed after prvTgfEthCountersReset,
                   because this functions makes link up. */
                for (portIter=0; portIter< prvTgfPortsNum; portIter++)
                {
                    iface.devPort.portNum = prvTgfPortsArray[portIter];
                    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&iface, GT_FALSE);
                    UTF_VERIFY_EQUAL0_STRING_MAC(
                        GT_OK, rc, "tgfTrafficGeneratorPortLoopbackModeEnableSet");
                }


                /* AUTODOC: Send packet */
                rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                    prvTgfPortsArray[SENDER_PORT]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfStartTransmitingEth: dev=[%d], port=[%d]\n",
                    prvTgfDevNum, prvTgfPortsArray[SENDER_PORT]);

                for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
                {
                    if(memEntryPtr->expectedCounters[portIter] == 0)
                    {
                        break;
                    }

                    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                        prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "prvTgfReadPortCountersEth: dev=[%d], enablePrint[%d]\n",
                        prvTgfDevNum, prvTgfPortsArray[portIter]);

                    validCntrs = dataIntegrityPacketCheck(&portCntrs,
                                 memEntryPtr->expectedCounters[portIter]);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, validCntrs,
                        "Got unexpected counters");
                    if(validCntrs)
                    {
                        expectedPacketCounter++;
                    }
                }
                memEntryPtr->gotValidPackets = expectedPacketCounter;
            }

            /* AUTODOC: Wait for events */
            rc = cpssOsSigSemWait(eventErrorsCame, WAIT_EVENTS_TIME_CNS);

            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("Not all events came. "
                    "We have %d, but expected %d\n",
                    evErrorCtr, memEntryPtr->expectedErrors);

                /* Send signal to free semaphore */
                cpssOsSigSemSignal(eventErrorsCame);
            }
            else
            {
                /* AUTODOC: Maybe some unexpected signals will come */
                cpssOsTimerWkAfter(1000);
            }

            /* allow number or errors to be more than expected. */
            if(memEntryPtr->expectedErrors >= evErrorCtr)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(memEntryPtr->expectedErrors, evErrorCtr,
                    "Unexpected event count for memType=[%d], expected %d, got %d",
                    ii, memEntryPtr->expectedErrors, evErrorCtr);
            }

            memEntryPtr->gotErrors = evErrorCtr;
            evErrorCtr = 0;
            dataIntegrityInjectError(memEntryPtr->memType,
                CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE,
                0, 0, /* Not used for LOCATION_RAM */
                GT_FALSE);

            /* AUTODOC: mask event */
            dataIntegrityInterruptConfig(memEntryPtr->memType,
                CPSS_EVENT_MASK_E,
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E,
                CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE, 0, 0 /* NOT used for LOCATION_RAM */);
#if 0
        /* AUTODOC: Do soft reset for clean test */
        rc = prvTgfResetAndInitSystem();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetAndInitSystem");
#endif
        }
        /* AUTODOC: Reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

        /* Print overall results */
        PRV_UTF_LOG0_MAC("\n\n\n======TEST RESULTS======\n");
        for(ii = 0; ii < memArrLen; ii++)
        {
            memEntryPtr = &memArr[ii];

            PRV_UTF_LOG3_MAC("%s expected errors: %d, got errors %d\n",
                memEntryPtr->name,
                memEntryPtr->expectedErrors,
                memEntryPtr->gotErrors
            );
            PRV_UTF_LOG3_MAC("%s expected packets: %d, got %d\n",
                memEntryPtr->name,
                memEntryPtr->expectedValidPackets,
                memEntryPtr->gotValidPackets
            );
        }
        PRV_UTF_LOG0_MAC("\n\n");
    }
}

/**
* @internal prvTgfDiagDataIntegrityErrorInjectionRestore function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagDataIntegrityErrorInjectionRestore
(
    GT_VOID
)
{
    GT_STATUS                     rc;

    /* AUTODOC: Disable HW errors ignoring */
    prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E);

    /* AUTODOC: Restore data integrity error handler */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, VLANID_CNS);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) || PRV_CPSS_DXCH_AC5_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfResetAndInitSystem();
    }
}

/**
* @internal prvTgfDiagPacketDataProtectionConfigurationSet function
* @endinternal
*
* @brief   Sets configuration
*/
GT_VOID prvTgfDiagPacketDataProtectionConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Create semaphore to wait all DI errors found */
    rc = cpssOsSigSemBinCreate("eventErrorsCame", CPSS_OS_SEMB_EMPTY_E,
        &eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemBinCreate");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = dataIntegrityErrorHandler;

    /* AUTODOC: Enable HW errors ignoring */
    prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E);

    /* AUTODOC: Create VLAN from available ports */
    rc = prvTgfBrgDefVlanEntryWrite(VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryWrite: vlanId=[%d]", VLANID_CNS);
}

/**
* @internal prvTgfDiagPacketDataProtectionTrafficGenerate function
* @endinternal
*
* @brief   Traffic generate and error verification
*/
GT_VOID prvTgfDiagPacketDataProtectionTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                                       rc;
    GT_U32                                          memCnt, pktCnt;
    GT_U32                                          expectedPacketCounter;
    MEMORY_DESC_STC                                *memEntryPtr = NULL;
    GT_U32                                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC                   portCntrs;
    GT_BOOL                                         validCntrs;
    CPSS_DXCH_LOCATION_ENT                          locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    GT_U32                                          memArrLen;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    CPSS_DXCH_TABLE_ENT                             hwTableType;
    CPSS_DXCH_LOGICAL_TABLE_ENT                     logicalTableType;
    GT_U32                                          memArrIndex = 0;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    MEMORY_DESC_STC                                *memArrayPtr;

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: dev=[%d]",
            prvTgfDevNum);

    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    if (PRV_CPSS_DXCH_FALCON_CHECK_MAC(prvTgfDevNum))
    {
        memArrayPtr = memoriesArrFalcon;
        memArrLen = sizeof(memoriesArrFalcon) / sizeof(memoriesArrFalcon[0]);
    }
    else
    {
        memArrayPtr = memoriesArrAc5x;
        memArrLen = sizeof(memoriesArrAc5x) / sizeof(memoriesArrAc5x[0]);
    }

    /* each RAM iterated for each RAM index type */
    memArrLen /= (CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE + 1);

    for(memCnt = 0; memCnt < memArrLen; memCnt++)   /* Memory Loop */
    {
        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        memEntry.info.ramEntryInfo.memType = memArrayPtr[memArrIndex].memType;

        rc = cpssDxChDiagDataIntegrityProtectionTypeGet(prvTgfDevNum, &memEntry, &protectionType);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChDiagDataIntegrityProtectionTypeGet");

        if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
        {
            /* ECC protected RAMs */
            errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            hwTableType      = CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_ECC_E;
            logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E;
        }
        else
        {
            /* Parity protected RAMs */
            errorType        = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            hwTableType      = CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_PARITY_E;
            logicalTableType = CPSS_DXCH_LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E;
        }

        for(locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE; locationType <= CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE; locationType++)
        {
            memEntryPtr                  = &memArrayPtr[memArrIndex];
            currentMemEntryPtr           = memEntryPtr;
            memEntryPtr->expectedHwTable = hwTableType;
            expectedPacketCounter        = 0;

            /* AUTODOC: Inject error */
            dataIntegrityInjectError(memEntryPtr->memType,
                    locationType,
                    logicalTableType,
                    hwTableType,
                    GT_TRUE);

            /* AUTODOC: Unmask event */
            dataIntegrityInterruptConfig(memEntryPtr->memType,
                    CPSS_EVENT_UNMASK_E,
                    errorType,
                    locationType,
                    logicalTableType,
                    hwTableType);

            for(pktCnt = 0; pktCnt < memEntryPtr->burstCount; pktCnt++ ) /* Packet counter loop */
            {
                /* AUTODOC: Reset counters */
                rc = prvTgfEthCountersReset(prvTgfDevNum);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

                /* Do not print packets */
                tgfTrafficTracePacketByteSet(GT_FALSE);

                /* AUTODOC: Send packet */
                rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                        prvTgfPortsArray[SENDER_PORT]);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                        "prvTgfStartTransmitingEth: dev=[%d], port=[%d]\n",
                        prvTgfDevNum, prvTgfPortsArray[SENDER_PORT]);

                /* AUTODOC: Get packet counters */
                for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
                {
                    if(memEntryPtr->expectedCounters[portIter] == 0)
                    {
                        break;
                    }

                    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                            prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                            "prvTgfReadPortCountersEth: dev=[%d], enablePrint[%d]\n",
                            prvTgfDevNum, prvTgfPortsArray[portIter]);

                    validCntrs = dataIntegrityPacketCheck(&portCntrs,
                            memEntryPtr->expectedCounters[portIter]);
                    /* TODO ?? UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, validCntrs,
                       "Got unexpected counters"); */
                    if(validCntrs)
                    {
                        expectedPacketCounter++;
                    }
                }
                memEntryPtr->gotValidPackets = expectedPacketCounter;
            }

            /* AUTODOC: Wait for events */
            rc = cpssOsSigSemWait(eventErrorsCame, WAIT_EVENTS_TIME_CNS);

            if(rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("Not all events came. "
                        "We have %d, but expected %d\n",
                        evErrorCtr, memEntryPtr->expectedErrors);

                /* Send signal to free semaphore */
                cpssOsSigSemSignal(eventErrorsCame);
            }
            else
            {
                /* AUTODOC: Maybe some unexpected signals will come */
                cpssOsTimerWkAfter(1000);
            }

            /* allow number or errors to be more than expected. */
            if(memEntryPtr->expectedErrors >= evErrorCtr)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(memEntryPtr->expectedErrors, evErrorCtr,
                        "Unexpected event count for memType=[%d], expected %d, got %d",
                        memArrIndex, memEntryPtr->expectedErrors, evErrorCtr);
            }

            memEntryPtr->gotErrors = evErrorCtr;
            evErrorCtr = 0;
            dataIntegrityInjectError(memEntryPtr->memType,
                    locationType,
                    logicalTableType,
                    hwTableType,
                    GT_FALSE);

            /* AUTODOC: mask event */
            dataIntegrityInterruptConfig(memEntryPtr->memType,
                    CPSS_EVENT_MASK_E,
                    errorType,
                    locationType,
                    logicalTableType,
                    hwTableType);
            memArrIndex+=1;
        }
    }

    /* AUTODOC: Reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* Print overall results */
    PRV_UTF_LOG0_MAC("\n\n\n======TEST RESULTS======\n");
    memArrIndex = 0;
    for(memCnt = 0; memCnt < memArrLen; memCnt++)
    {
        for(locationType = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE; locationType <= CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE; locationType++)
        {
            memEntryPtr = &memArrayPtr[memArrIndex];
            PRV_UTF_LOG3_MAC("%s expected errors: %d, got errors %d\n",
                    memEntryPtr->name,
                    memEntryPtr->expectedErrors,
                    memEntryPtr->gotErrors
                    );
            PRV_UTF_LOG3_MAC("%s expected packets: %d, got %d\n",
                    memEntryPtr->name,
                    memEntryPtr->expectedValidPackets,
                    memEntryPtr->gotValidPackets
                    );
            memArrIndex+=1;
        }
    }
    PRV_UTF_LOG0_MAC("\n\n");
}

/**
* @internal prvTgfDiagPacketDataProtectionRestore function
* @endinternal
*
* @brief   Restores configuration
*/
GT_VOID prvTgfDiagPacketDataProtectionRestore
(
    GT_VOID
)
{
    GT_STATUS                     rc;

    /* AUTODOC: Disable HW errors ignoring */
    prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E);

    /* AUTODOC: Restore data integrity error handler */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;

    /* AUTODOC: Delete semaphore */
    rc = cpssOsSigSemDelete(eventErrorsCame);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssOsSigSemDelete");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, VLANID_CNS);
}
